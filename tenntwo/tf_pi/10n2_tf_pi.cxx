

// standard includes
#include <stdio.h>
#include <10n2_tf_pi.h>
//#include <10n2_cam.h>
//#include "test_data.h"

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

#include <test_data.h>

// Globals
static TfLiteTensor *input = nullptr;
static TfLiteTensor *output = nullptr;
static tflite::ErrorReporter *er = new tflite::MicroErrorReporter();

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
    //since grayscale, only 1 value needed
    float g = static_cast<float>(pixel_grayscale) / 255.0f;

    // ITU-R 601-2 luma transform
    // see: https://pillow.readthedocs.io/en/stable/reference/Image.html#PIL.Image.Image.convert
    float v = (0.299f * g) + (0.587f * g) + (0.114f * g);
    return static_cast<int8_t>((int8_t)(v / EI_CLASSIFIER_TFLITE_INPUT_SCALE + .5) + EI_CLASSIFIER_TFLITE_INPUT_ZEROPOINT);
}

bool model_init(void)
{

    model = tflite::GetModel(trained_tflite);
    printf("after getmodel %d \n", model->version());

    if (model->version() != 3)
    {
        TF_LITE_REPORT_ERROR(er,
                             "Model provided is schema version %d not equal "
                             "to supported version %d.",
                             model->version(), 3);
        return false;
    }

    static tflite::AllOpsResolver resolver;
    // EI_TFLITE_RESOLVER
    printf("after resolver \n");

    interpreter = new tflite::MicroInterpreter(model, resolver, tnt_tensor_arena, tnt_arena_size, er);

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    printf("after alloc\n");

    if (allocate_status != kTfLiteOk)
    {
        printf("TF alloc failure!!!\n");
        TF_LITE_REPORT_ERROR(er, "AllocateTensors() failed");
        return false;
    }

    // Get information about the memory area to use for the model's input.
    printf("before input \n");
    input = interpreter->input(0);
    output = interpreter->output(0);
    printf("after input  scale %f \n", input->params.scale);
    printf("after input  zero_point %i \n", input->params.zero_point);
    printf("after input  type %i \n", input->type);
    printf("after input  size %i \n", input->bytes);

    int8_t *d = tflite::GetTensorData<int8>(input);
    int8_t *od = tflite::GetTensorData<int8>(output);

    for (int i=0;i<160 * 120;i++)
    {
        d[i] = quantize(none_test_data[i] & 0xff); //We only need first byte, bytes 1-3 all the same for gray 
    }
    // Run the model on this input and make sure it succeeds.
    if (kTfLiteOk != interpreter->Invoke())
    {
        printf("TF invoke failure!!!\n");
        TF_LITE_REPORT_ERROR(er, "Invoke failed.");
        return false;
    }
    printf("done with invoke!\n");

    for (int j=0;j<3;j++)
    {
        float res = (od[j] - EI_CLASSIFIER_TFLITE_OUTPUT_ZEROPOINT) * EI_CLASSIFIER_TFLITE_OUTPUT_SCALE;
        printf ("none res %i : %f\n",j,res);
    }
    for (int i=0;i<160 * 120;i++)
    {
        d[i] = quantize(hands_test_data[i] & 0xff); //We only need first byte, bytes 1-3 all the same for gray 
    }
    // Run the model on this input and make sure it succeeds.
    if (kTfLiteOk != interpreter->Invoke())
    {
        printf("TF invoke failure!!!\n");
        TF_LITE_REPORT_ERROR(er, "Invoke failed.");
        return false;
    }
    printf("done with invoke!\n");

    for (int j=0;j<3;j++)
    {
        float res = (od[j] - EI_CLASSIFIER_TFLITE_OUTPUT_ZEROPOINT) * EI_CLASSIFIER_TFLITE_OUTPUT_SCALE;
        printf ("hands res %i : %f\n",j,res);
    }
    for (int i=0;i<160 * 120;i++)
    {
        d[i] = quantize(cell_test_data[i] & 0xff); //We only need first byte, bytes 1-3 all the same for gray 
    }
    // Run the model on this input and make sure it succeeds.
    if (kTfLiteOk != interpreter->Invoke())
    {
        printf("TF invoke failure!!!\n");
        TF_LITE_REPORT_ERROR(er, "Invoke failed.");
        return false;
    }
    printf("done with invoke!\n");

    for (int j=0;j<3;j++)
    {
        float res = (od[j] - EI_CLASSIFIER_TFLITE_OUTPUT_ZEROPOINT) * EI_CLASSIFIER_TFLITE_OUTPUT_SCALE;
        printf ("cell res %i : %f\n",j,res);
    }

    return true;
}
bool tf_pi_init(void)
{
    printf("tf pi init\n");
    model_init();
    printf("done model init\n");
    return true;
}

bool tf_pi_teardown(void)
{
    printf("tf pi teardown\n");
    return true;
}
