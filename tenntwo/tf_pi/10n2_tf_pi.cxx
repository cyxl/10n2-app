

// standard includes
#include <stdio.h>
#include <10n2_tf_pi.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/types.h>
#include <fcntl.h>
#include <nuttx/clock.h>
#include <nuttx/sched.h>
#include <nuttx/arch.h>
#include <10n2_cam.h>
#include <10n2_aud.h>

// EI Includes
#include "tflite-resolver.h"
#include "tflite-trained.h"
#include "model_metadata.h"

// TF Includes
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"

static bool tf_running = true;
#define TF_QUEUE_NAME "/tf_queue" /* Queue name. */
#define TF_QUEUE_PERMS ((int)(0644))
#define TF_QUEUE_MAXMSG 16              /* Maximum number of messages. */
#define TF_QUEUE_MSGSIZE sizeof(tf_req) /* Length of message. */
#define TF_QUEUE_ATTR_INITIALIZER ((struct mq_attr){TF_QUEUE_MAXMSG, TF_QUEUE_MSGSIZE, 0, 0})

#define TF_QUEUE_POLL ((struct timespec){0, 10000000})

#define CELL_CONF .7
#define HANDS_CONF .7
#define NONE_CONF .7
#define INF_CONF .4

uint8_t current_inf = TF_UNKNOWN;
float current_conf = 0.;
uint32_t current_time_tf = 0;

struct mq_attr tf_attr_mq = TF_QUEUE_ATTR_INITIALIZER;
static pthread_t tf_th_consumer;

// Globals
static TfLiteTensor *input = nullptr;
static TfLiteTensor *output = nullptr;
static tflite::ErrorReporter *er = new tflite::MicroErrorReporter();

#define CELL_IDX 0
#define HANDS_IDX 1
#define NONE_IDX 2

// Globals, used for compatibility with Arduino-style sketches.
namespace
{
    const tflite::Model *model = nullptr;
    tflite::MicroInterpreter *interpreter = nullptr;
    const uint32_t tnt_arena_size = EI_CLASSIFIER_TFLITE_ARENA_SIZE;
    static uint8_t tnt_tensor_arena[tnt_arena_size];
} // namespace

int8_t quantize(uint8_t pixel_grayscale)
{
    static const int32_t iRedToGray = (int32_t)(0.299f * 65536.0f);
    static const int32_t iGreenToGray = (int32_t)(0.587f * 65536.0f);
    static const int32_t iBlueToGray = (int32_t)(0.114f * 65536.0f);

    // ITU-R 601-2 luma transform
    // see: https://pillow.readthedocs.io/en/stable/reference/Image.html#PIL.Image.Image.convert
    int32_t gray = (iRedToGray * pixel_grayscale) + (iGreenToGray * pixel_grayscale) + (iBlueToGray * pixel_grayscale);
    gray >>= 16; // scale down to int8_t
    gray += EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT;
    if (gray < -128)
        gray = -128;
    else if (gray > 127)
        gray = 127;
    return static_cast<int8_t>(gray);
}

int8_t slow_quantize(uint8_t pixel_grayscale)
{
    // since grayscale, only 1 value needed
    float g = static_cast<float>(pixel_grayscale) / 255.0f;

    // ITU-R 601-2 luma transform
    // see: https://pillow.readthedocs.io/en/stable/reference/Image.html#PIL.Image.Image.convert
    float v = (0.299f * g) + (0.587f * g) + (0.114f * g);
    return static_cast<int8_t>((int8_t)(v / EI_CLASSIFIER_TFLITE_INPUT_SCALE + .5) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
}

bool model_init(void)
{

    model = tflite::GetModel(trained_tflite);

    if (model->version() != 3)
    {
        TF_LITE_REPORT_ERROR(er,
                             "Model provided is schema version %d not equal "
                             "to supported version %d.",
                             model->version(), 3);
        return false;
    }

    static tflite::AllOpsResolver resolver;
    //   EI_TFLITE_RESOLVER

    interpreter = new tflite::MicroInterpreter(model, resolver, tnt_tensor_arena, tnt_arena_size, er);

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();

    if (allocate_status != kTfLiteOk)
    {
        printf("TF alloc failure :%i!!!\n", allocate_status);
        TF_LITE_REPORT_ERROR(er, "AllocateTensors() failed");
        return false;
    }

    // Get information about the memory area to use for the model's input.
    input = interpreter->input(0);
    output = interpreter->output(0);

    return true;
}

void *_tf_thread(void *args)
{
    (void)args; /* Suppress -Wunused-parameter warning. */
    /* Initialize the queue attributes */

    int cpu = up_cpu_index();
    printf("TF CPU %d\n", cpu);

    sigset_t mask;
    sigemptyset(&mask);
    //TOOD 18
    sigaddset(&mask, 18);
    int ret = sigprocmask(SIG_BLOCK, &mask, NULL);
    if (ret != OK)
    {
        printf("TF ERROR sigprocmask failed. %d\n", ret);
    }

    /* Create the message queue. The queue reader is NONBLOCK. */
    mqd_t r_mq = mq_open(TF_QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, TF_QUEUE_PERMS, &tf_attr_mq);
    if (r_mq < 0)
    {
        fprintf(stderr, "[TF CONSUMER]: Error, cannot open the queue: %s.\n", strerror(errno));
        return NULL;
    }

    printf("[TF CONSUMER]: Queue opened, queue descriptor: %d.\n", r_mq);

    unsigned int prio;
    ssize_t bytes_read;
    char buffer[TF_QUEUE_MSGSIZE];
    struct timespec poll_sleep;
    static const int num_classes = 3;

    while (tf_running)
    {
        bytes_read = mq_receive(r_mq, buffer, TF_QUEUE_MSGSIZE, &prio);
        tf_req *r = (tf_req *)buffer;
        if (bytes_read >= 0)
        {
            int8_t *d = tflite::GetTensorData<int8>(input);
            int8_t *od = tflite::GetTensorData<int8>(output);
            struct timespec del_sleep
            {
                r->delay / 1000, (r->delay % 1000) * 1e6
            };
            for (int i = 0; i < r->num && tf_running; i++)
            {

                // printf("quant\n");
                cam_wait();
                for (int img_idx = 0; img_idx < 96 * 96; img_idx++)
                {
                    // d[i] = quantize(cell_test_data[i] & 0xff); // We only need first byte, bytes 1-3 all the same for gray
                    d[img_idx] = quantize(latest_img_buf[img_idx]);
                }
                cam_release();
                // printf("done quant\n");
                //   Run the model on this input and make sure it succeeds.
                printf("=======================invoke\n");
                if (kTfLiteOk != interpreter->Invoke())
                {
                    printf("TF invoke failure!!!\n");
                    TF_LITE_REPORT_ERROR(er, "Invoke failed.");
                    continue;
                }
                printf("=======================done invoke\n");

                // for (int j = 0; j < num_classes; j++)
                // {
                //     float res = (od[j] - EI_CLASSIFIER_TFLITE_OUTPUT_ZEROPOINT) * EI_CLASSIFIER_TFLITE_OUTPUT_SCALE;
                //     printf("inf res %i : %f\n", j, res);
                // }

                float max_conf = 0;
                int max_idx = -1;

                for (int k = 0; k < num_classes; k++)
                {
                    float conf = 0.;
                    conf = (od[k] - EI_CLASSIFIER_TFLITE_OUTPUT_ZEROPOINT) * EI_CLASSIFIER_TFLITE_OUTPUT_SCALE;
                    if (conf > max_conf)
                    {
                        max_conf = conf;
                        max_idx = k;
                    }
                }

                printf("infs %f %d\n", max_conf, max_idx);
                current_conf = max_conf;
                current_time_tf = clock();

                if (max_idx == CELL_IDX)
                {
                    current_inf = TF_CELL;
                }
                else if (max_idx == HANDS_IDX)
                {
                    current_inf = TF_HANDS;
                }
                else if (max_idx == NONE_IDX)
                {
                    current_inf = TF_NOHANDS;
                }

                nanosleep(&del_sleep, NULL);
            }
        }
        else
        {
            poll_sleep = TF_QUEUE_POLL;
            nanosleep(&poll_sleep, NULL);
        }

        fflush(stdout);
    }
    printf("tf cleaning mq\n");
    mq_close(r_mq);
    return NULL;
}

bool send_tf_req(struct tf_req req)
{
    mqd_t mq = mq_open(TF_QUEUE_NAME, O_WRONLY);
    if (mq < 0)
    {
        fprintf(stderr, "[tf sender]: Error, cannot open the queue: %s.\n", strerror(errno));
        return false;
    }
    if (mq_send(mq, (char *)&req, TF_QUEUE_MSGSIZE, 1) < 0)
    {
        fprintf(stderr, "[tf sender]: Error, cannot send: %i, %s.\n", errno, strerror(errno));
    }

    mq_close(mq);
    return true;
}
bool tf_pi_init(void)
{
    printf("tf pi init\n");
    model_init();
    printf("done model init\n");
    tf_running = true;
    cpu_set_t cpuset = 1 << 2;
    int rc = pthread_setaffinity_np(tf_th_consumer, sizeof(cpu_set_t), &cpuset);
    pthread_create(&tf_th_consumer, NULL, &_tf_thread, NULL);
    if (rc != 0)
    {
        printf("Unable set CPU affinity : %d", rc);
    }

    return true;
}

bool tf_pi_teardown(void)
{
    printf("tf pi teardown\n");
    tf_running = false;
    pthread_join(tf_th_consumer, NULL);
    return true;
}
