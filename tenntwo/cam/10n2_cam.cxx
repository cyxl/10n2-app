
/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <mqueue.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <arch/board/board.h>
#include <arch/chip/audio.h>

#include <nuttx/arch.h>
#include <10n2_cam.h>
#include <10n2_aud.h>
#include <tnt_image_provider.h>
#include <camera_fileutil.h>

static bool cam_running = true;
#define CAM_QUEUE_NAME "/cam_queue" /* Queue name. */
#define CAM_QUEUE_PERMS ((int)(0644))
#define CAM_QUEUE_MAXMSG 16               /* Maximum number of messages. */
#define CAM_QUEUE_MSGSIZE sizeof(cam_req) /* Length of message. */
#define CAM_QUEUE_ATTR_INITIALIZER ((struct mq_attr){CAM_QUEUE_MAXMSG, CAM_QUEUE_MSGSIZE, 0, 0})

#define IMG_SAVE_DIR "/mnt/sd0/img"
static struct mq_attr cam_attr_mq = CAM_QUEUE_ATTR_INITIALIZER;
static pthread_t cam_th_consumer;

unsigned char *latest_img_buf = nullptr;
pthread_mutex_t cam_mutex;

void *_cam_q_read(void *args)
{

    (void)args; /* Suppress -Wunused-parameter warning. */
    /* Initialize the queue attributes */

//    int cpu = up_cpu_index();
 //   printf("CAM CPU %d\n", cpu);

    /* Create the message queue. The queue reader is NONBLOCK. */
    mqd_t r_mq = mq_open(CAM_QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, CAM_QUEUE_PERMS, &cam_attr_mq);
    futil_initialize();

    if (r_mq < 0)
    {
        fprintf(stderr, "[cam CONSUMER]: Error, cannot open the queue: %s.\n", strerror(errno));
        return NULL;
    }

    printf("[cam CONSUMER]: Queue opened, queue descriptor: %d.\n", r_mq);

    unsigned int prio;
    ssize_t bytes_read;
    char buffer[CAM_QUEUE_MSGSIZE];
    char namebuf[128];
    char dirbuf[128];
    uint16_t current_x0, current_y0, current_x1, current_y1 = 0;
    uint32_t num_pics = 0;

    timespec poll_sleep;

    while (cam_running)
    {
        memset(buffer, 0x00, sizeof(buffer));
        bytes_read = mq_receive(r_mq, buffer, CAM_QUEUE_MSGSIZE, &prio);
        cam_req *r = (cam_req *)buffer;
        if (bytes_read >= 0)
        {
            if (
                current_x0 != r->clip_x0 |
                current_y0 != r->clip_y0 |
                current_x1 != r->clip_x1 |
                current_y1 != r->clip_y1)
            {
                camera_teardown();
                camera_init(r->clip_x0, r->clip_y0, r->clip_x1, r->clip_y1);
            }
            unsigned curr_time = (unsigned)time(NULL);
            uint16_t width = (r->clip_x1 - r->clip_x0);
            uint16_t height = (r->clip_y1 - r->clip_y0);
            uint32_t bufsize = width * height * (r->color ? 2 : 1);
            unsigned char *buf = (unsigned char *)memalign(32, bufsize);
            if (buf == NULL)
            {
                printf("ERROR - unable to malloc buf\n");
                break;
            }
            latest_img_buf = buf;
            for (int i = 0; i < r->num; i++)
            {
                bzero(namebuf, 128);
                bzero(dirbuf, 128);
                cam_wait();
                int rc = getimage(buf, r->clip_x0, r->clip_y0, r->clip_x1, r->clip_y1, r->color);
                cam_release();
                if (rc == OK)
                {
                    num_pics++;
                    if ((num_pics% 10) == 0 ) send_aud_seq(cam_capture);
                }

                if (strlen(r->dir) > 0)
                {
                    printf("writing image\n");
                    snprintf(namebuf, 128, "tnt-%i-%i-%ix%i.%s", curr_time, i, width, height, r->color ? "yuv" : "data");
                    snprintf(dirbuf, 128, "%s/%s", IMG_SAVE_DIR, r->dir);
                    if (futil_writeimage(buf, bufsize, dirbuf, namebuf) < 0)
                    {
                        printf("ERROR creating image \n");
                    }
                }
                usleep(r->delay * 1e3);
            }
            free(buf);
        }
        else
        {
            usleep(10 * 1e3);
        }

        fflush(stdout);
    }
    printf("cam cleaning mq\n");
    mq_close(r_mq);
    // mq_unlink(CAM_QUEUE_NAME);
    return NULL;
}

bool send_cam_req(struct cam_req req)
{
    mqd_t mq = mq_open(CAM_QUEUE_NAME, O_WRONLY | O_NONBLOCK);
    if (mq < 0)
    {
        fprintf(stderr, "[cam sender]: Error, cannot open the queue: %s.\n", strerror(errno));
        return false;
    }
    if (mq_send(mq, (char *)&req, CAM_QUEUE_MSGSIZE, 1) < 0)
    {
        fprintf(stderr, "[cam sender]: Error, cannot send: %i, %s.\n", errno, strerror(errno));
    }

    mq_close(mq);
    return true;
}

bool cam_init(void)
{
    printf("cam init\n");
    cam_running = true;
    pthread_create(&cam_th_consumer, NULL, &_cam_q_read, NULL);
    cpu_set_t cpuset = 1 << 3;
    int rc=0;
    rc = pthread_setaffinity_np(cam_th_consumer, sizeof(cpu_set_t), &cpuset);
    if (rc != 0)
    {
        printf("Unable set CPU affinity : %d", rc);
    }
    if (0 != (errno = pthread_mutex_init(&cam_mutex, NULL)))
    {
        perror("pthread_mutex_init() failed");
        return EXIT_FAILURE;
    }
    return true;
}

bool cam_teardown(void)
{
    printf("cam teardown\n");
    cam_running = false;
    pthread_join(cam_th_consumer, NULL);

    return true;
}

bool cam_wait(void)
{
    return 0 == pthread_mutex_lock(&cam_mutex);
}
bool cam_release(void)
{
    return 0 == pthread_mutex_unlock(&cam_mutex);
}
