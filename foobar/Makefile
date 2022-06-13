# Application makefile

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

CXXEXT = cc
# Additional C++ source files (*.cxx)
CXXSRCS = mot-imu-model.cc mot-imu-tf.cc

# Additional assembler source files (*.S)
ASRCS =

# C compiler flags
CFLAGS =

CCFLAGS = -std=c++11 
# C++ compiler flags
FLATBUFFERS_INCLUDES = $(TOPDIR)/../externals/tensorflow/tflite-micro-spresense-examples-1.0.0/third_party/flatbuffers/include/

CXXFLAGS := \
  -std=c++11 \
  $(COMMON_FLAGS)

CXXFLAGS += -I$(FLATBUFFERS_INCLUDES)

include $(SPRESENSE_HOME)/.vscode/application.mk
