# Copyright 2021 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================

# Settings for Spresense based platforms
# For spresense, Tensorflow lite Microcontroller is used as a library.

# This setting makefile accept 4 optional parameters on the make command line.
# These options below are needed for build an example of Tensorflow Microcontroller.
# But just build a library, no need to add those options.
#
#   SPRESENSE_DEFS       : This is the file path to Make.defs which includes configuration
#                          parameters of spresense.
#   SPRESENSE_CONFIG_H   : This is the file path to config.h which includes configuration
#                          parameters for source code.
#   SPRESENSE_CURDIR     : This is the directory path of externals/tensorflow in spresense
#                          source repository.
#   SPRESENSE_APP_TFMAKE : This is the file path to makefile.inc for additional source code
#                          in spresense to use tensorflow.

# Define empty variable for add spresense specific settings
SPRESENSE_PLATFORM_FLAGS :=

ifneq ($(SPRESENSE_DEFS),)

# Load Spresense Config
include $(SPRESENSE_DEFS)

SPRESENSE_PLATFORM_FLAGS := \
  -DSPRESENSE_CONFIG_H="\"$(SPRESENSE_CONFIG_H)\"" \
  -I$(SPRESENSE_CURDIR)/wrapper_include

# Load application for Tensorflow lite micro in Spresense
ifneq ($(SPRESENSE_APP_TFMAKE),)
ifeq ($(CONFIG_EXTERNALS_TENSORFLOW_EXAMPLE_NONE),y)
-include $(SPRESENSE_APP_TFMAKE)
endif
endif

endif

TOOLCHAIN_PREFIX := arm-none-eabi-
CXX := $(TOOLCHAIN_PREFIX)g++
CC := $(TOOLCHAIN_PREFIX)gcc
AR := $(TOOLCHAIN_PREFIX)ar
ARFLAGS := -r

GENDIR := gen
OBJDIR := $(GENDIR)/obj
BINDIR := $(GENDIR)/bin

LIBTFLM := $(GENDIR)/libtensorflow-microlite.a

OPTIMIZATION_LEVEL := -O3

CC_WARNINGS := \
  -Werror \
  -Wshadow \
  -Wmissing-field-initializers \
  -Wunused-function \
  -Wswitch \
  -Wall \
  -Wextra \
  -Wstrict-aliasing \
  -Wno-unused-parameter \
  -Wno-sign-compare \

PLATFORM_FLAGS := \
  $(SPRESENSE_PLATFORM_FLAGS) \
  -fmessage-length=0 \
  -fno-exceptions \
  -fno-unwind-tables \
  -ffunction-sections \
  -fdata-sections \
  -funsigned-char \
  -fno-delete-null-pointer-checks \
  -fomit-frame-pointer \
  -MMD \
  -mcpu=cortex-m4 \
  -mabi=aapcs \
  -mthumb \
  -DNDEBUG \
  -mfpu=fpv4-sp-d16 \
  -mfloat-abi=hard \
  -DTF_LITE_DISABLE_X86_NEON=1 \
  -DEI_CLASSIFIER_TFLITE_ENABLE_CMSIS_NN=1

COMMON_FLAGS := \
  -DTF_LITE_STATIC_MEMORY \
  -DTF_LITE_MCU_DEBUG_LOG \
  -DCMSIS_NN \
  $(PLATFORM_FLAGS) \
  $(OPTIMIZATION_LEVEL) \
  $(CC_WARNINGS) \

CXXFLAGS := \
  -std=c++11 \
  -fno-rtti \
  -fno-use-cxa-atexit \
  -fno-threadsafe-statics \
  -D__ARM_FEATURE_DSP=1 \
  $(COMMON_FLAGS)

CCFLAGS := \
	-std=c11 \
  -D__ARM_FEATURE_DSP=1 \
  $(COMMON_FLAGS)

INCLUDES := \
  -I. \
  -Ithird_party/cmsis/ \
  -Ithird_party/cmsis/CMSIS/Core/Include/ \
  -Ithird_party/cmsis/CMSIS/DSP/Include/ \
  -Ithird_party/cmsis/CMSIS/NN/Include/ \
  -Ithird_party/flatbuffers/include \
  -Ithird_party/gemmlowp \
  -Ithird_party/kissfft \
  -I../../cmsis/CMSIS_5 \
  -I../../cmsis/CMSIS_5/CMSIS/DSP/Include \
  -I../../cmsis/CMSIS_5/CMSIS/Core/Include \
  -Ithird_party/ruy

ifeq ($(CONFIG_EXTERNALS_TENSORFLOW_EXAMPLE_MICROSPEECH),y)
INCLUDES += -Iexamples/micro_speech
endif

TFLM_CC_SRCS := $(shell find tensorflow -name "*.cc" -o -name "*.c")

ifeq ($(CONFIG_EXTERNALS_TENSORFLOW_EXAMPLE_HELLOWORLD),y)
TFLM_CC_SRCS += $(wildcard examples/hello_world/*.cc)
endif

ifeq ($(CONFIG_EXTERNALS_TENSORFLOW_EXAMPLE_MICROSPEECH),y)
TFLM_CC_SRCS += $(wildcard examples/micro_speech/*.cc)
TFLM_CC_SRCS += $(wildcard examples/micro_speech/micro_features/*.cc)
endif

ifeq ($(CONFIG_EXTERNALS_TENSORFLOW_EXAMPLE_PERSONDETECTION),y)
TFLM_CC_SRCS += $(wildcard examples/person_detection/*.cc)
endif

THIRD_PARTY_CC_SRCS := \
  $(THIRD_PARTY_CC_SRCS) \
  $(shell find third_party/cmsis/CMSIS/NN/Source -name "*.cc" -o -name "*.c")

ALL_SRCS := \
	$(TFLM_CC_SRCS) \
	$(THIRD_PARTY_CC_SRCS) \

OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.c,%.o,$(patsubst %.cc,%.o,$(ALL_SRCS))))

$(OBJDIR)/%.o: %.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $@

$(LIBTFLM): $(OBJS)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $(LIBTFLM) $(OBJS)

libtflm: $(LIBTFLM)

clean:
	rm -rf $(GENDIR)
