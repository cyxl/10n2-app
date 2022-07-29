# Application makefile
TF_SRC_DIR = tf_model
IMU_SRC_DIR = imu
GNSS_SRC_DIR = gnss
CAM_SRC_DIR = cam
AUD_SRC_DIR = aud
BTN_SRC_DIR = btn
TF_PI_SRC_DIR = tf_pi
DP_SRC_DIR = dp
MENU_HDLR_SRC_DIR = menu_handler
UTIL_SRC_DIR = util
REC_SRC_DIR = rec
MODEL_SRC_DIR = model

# Command name (Public function 'int <APPNAME>_main(void)' required)
APPNAME = tenntwo

# Application execute priority (Range: 0 ~ 255, Default: 100)
PRIORITY =

# Application stack memory size (Default: 2048)
STACKSIZE = 

# Main source code
MAINSRC =

# Additional C source files (*.c)
CSRCS =

#CXXSRCS = constants.cxx  hello_world_model_data.cxx  main_functions.cxx  output_handler.cxx

CXXSRS = $(sort $(shell find ./ -name *.cxx -type f))

# Additional assembler source files (*.S)
ASRCS =

# C compiler flags
CFLAGS =
CFLAGS += -I$(TF_SRC_DIR)
CFLAGS += -I$(IMU_SRC_DIR)
CFLAGS += -I$(GNSS_SRC_DIR)
CFLAGS += -I$(CAM_SRC_DIR)
CFLAGS += -I$(AUD_SRC_DIR)
CFLAGS += -I$(DP_SRC_DIR)
CFLAGS += -I$(BTN_SRC_DIR)
CFLAGS += -I$(TF_PI_SRC_DIR)
CFLAGS += -I$(MENU_HDLR_SRC_DIR)
CFLAGS += -I$(UTIL_SRC_DIR)
CFLAGS += -I$(REC_SRC_DIR)
CFLAGS += -I$(MODEL_SRC_DIR)
CFLAGS += -DSPRESENSE_CONFIG_H="\"/home/bstaley/git/spresense/nuttx/include/nuttx/config.h\"" 
CFLAGS += -DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=1

CFLAGS += -I$(TOPDIR)/arch/arm/src/cxd56xx/
CFLAGS += -DNDEBUG					# Disable assert() macro

# C++ compiler flags
CXXFLAGS += -I$(SDKDIR)/../externals/tensorflow/tflite-micro-spresense-examples-1.0.0/third_party/flatbuffers/include/
# TF
CXXFLAGS += -I$(SDKDIR)/../externals/tensorflow/tflite-micro-spresense-examples-1.0.0/tensorflow/lite/micro/spresense/
CXXFLAGS += -DSPRESENSE_CONFIG_H="\"/home/bstaley/git/spresense/nuttx/include/nuttx/config.h\"" 
#CXXFLAGS += -D_U -D_L -D_N -D_X -D_S -D_P -D_B -D_C
CXXFLAGS += -I$(TF_SRC_DIR)
CXXFLAGS += -I$(GNSS_SRC_DIR)
CXXFLAGS += -I$(IMU_SRC_DIR)
CXXFLAGS += -I$(AUD_SRC_DIR)
CXXFLAGS += -I$(CAM_SRC_DIR)
CXXFLAGS += -I$(BTN_SRC_DIR)
CXXFLAGS += -I$(TF_PI_SRC_DIR)
CXXFLAGS += -I$(DP_SRC_DIR)
CXXFLAGS += -I$(UTIL_SRC_DIR)
CXXFLAGS += -I$(MENU_HDLR_SRC_DIR)
CXXFLAGS += -I$(REC_SRC_DIR)
CXXFLAGS += -I$(MODEL_SRC_DIR)
CXXFLAGS += -DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=1

CXXFLAGS += -DNDEBUG					# Disable assert() macro


include $(SPRESENSE_HOME)/.vscode/application.mk
