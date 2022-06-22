# Application makefile
TF_SRC_DIR = tf_model
IMU_SRC_DIR = imu
GNSS_SRC_DIR = gnss
CAM_SRC_DIR = cam
AUD_SRC_DIR = aud

# Command name (Public function 'int <APPNAME>_main(void)' required)
APPNAME = 

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

CFLAGS += -I$(TOPDIR)/arch/arm/src/cxd56xx/

# C++ compiler flags
CXXFLAGS =
CXXFLAGS += -I$(SDKDIR)/../externals/tensorflow/tflite-micro-spresense-examples-1.0.0/third_party/flatbuffers/include/
CXXFLAGS += -DSPRESENSE_CONFIG_H="\"/home/bstaley/git/spresense/nuttx/include/nuttx/config.h\"" -D_U -D_L -D_N -D_X -D_S -D_P -D_B -D_C
CXXFLAGS += -I$(TF_SRC_DIR)
CXXFLAGS += -I$(GNSS_SRC_DIR)
CXXFLAGS += -I$(IMU_SRC_DIR)
CXXFLAGS += -I$(AUD_SRC_DIR)
CXXFLAGS += -I$(CAM_SRC_DIR)

include $(SPRESENSE_HOME)/.vscode/application.mk
