#include "mot-imu-tf.h"

/*
//#include "tensorflow/lite/micro/all_ops_resolver.h"
//#include "output_handler.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
//#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

#include "mot-imu-model.h"

static TfLiteTensor *mot_input = nullptr;
static TfLiteTensor *mot_output = nullptr;
//must match model training hyperparameters (don't change)
static const int g_min = -90;
static const int g_max = 90;
static const int a_min = -1;
static const int a_max = 1;

#define INF_THRESH .60
#define INF_SIZE 10

// Globals, used for compatibility with Arduino-style sketches.
namespace
{
  tflite::ErrorReporter *error_reporter = nullptr;
  const tflite::Model *model = nullptr;
  tflite::MicroInterpreter *mot_interpreter = nullptr;

  // Create an area of memory to use for input, output, and intermediate arrays.
  // Minimum arena size, at the time of writing. After allocating tensors
  // you can retrieve this value by invoking interpreter.arena_used_bytes().
  const int kModelArenaSize = 4156;
  // Extra headroom for model + alignment + future interpreter changes.
  const int kExtraArenaSize = 560 + 16 + 100;
  const int kTensorArenaSize = kModelArenaSize + kExtraArenaSize;
  static uint8_t mot_tensor_arena[kTensorArenaSize];
} // namespace

void init_mot_imu(void)
{
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION)
  {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // This pulls in all the operation implementations we need.
  // NOLINTNEXTLINE(runtime-global-variables)
  //static tflite::AllOpsResolver resolver;
  static tflite::MicroMutableOpResolver<5> resolver;
  resolver.AddConv2D();
  resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
                      tflite::ops::micro::Register_MAX_POOL_2D());
  resolver.AddFullyConnected();
  resolver.AddSoftmax();
  resolver.AddReshape();

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter mot_static_interpreter(
      model, resolver, mot_tensor_arena, kTensorArenaSize, error_reporter);
  mot_interpreter = &mot_static_interpreter;

  // Allocate memory from the mot_tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = mot_interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk)
  {
    TF_LITE_REPORT_ERROR(error_reporter, "AllocateTensors() failed");
    return;
  }
  mot_input = mot_interpreter->input(0);
  mot_output = mot_interpreter->output(0);
}
*/
