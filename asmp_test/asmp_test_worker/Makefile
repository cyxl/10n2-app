
-include $(TOPDIR)/.config
include $(APPDIR)/Make.defs
include $(SDKDIR)/Make.defs

ifeq ($(WINTOOL),y)
LIB_DIR = "${shell cygpath -w lib}"
else
LIB_DIR = "lib"
endif

LDLIBPATH += -L $(LIB_DIR)

LDLIBS += -lasmpw

WORKER_FOLDER_NAME := $(notdir $(shell pwd))
WORKER_NAME := $(shell echo $(WORKER_FOLDER_NAME) | sed s/_worker$$//)
OUTDIR := $(SPRESENSE_HOME)/out/worker
BIN := $(OUTDIR)/$(WORKER_NAME)

#VPATH = $(SDKDIR)/modules/audio/components/customproc/dsp_framework
VPATH = 
#VPATH += userproc/src

CXXSRCS = main.cpp
#CXXSRCS += 

CXXELFFLAGS += -Os
#CXXELFFLAGS += -Iuserproc/include
CXXELFFLAGS += -I$(APPDIR)
CXXELFFLAGS += -I$(SDKDIR)/modules/asmp/worker

CXXOBJS = $(CXXSRCS:.cpp=$(OBJEXT))

# Build

.PHONY: lib depend clean
all: $(BIN)

# Build ASMP worker library

lib:
	$(Q) $(MAKE) -C lib TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)

# Complile

$(CXXOBJS): %$(OBJEXT):%.cpp
	@echo "CXX: $<"
	$(Q) $(CXX) -c $(CXXELFFLAGS) $< -o $@

# Build workers

$(BIN): lib $(CXXOBJS)
	@echo "LD: $@"
	$(Q) $(LD) $(LDRAWELFFLAGS) $(LDLIBPATH) -o $@ $(ARCHCRT0OBJ) $(CXXOBJS) $(LDLIBS)
	$(Q) $(STRIP) -d $(BIN)

.depend:
	$(Q) $(MAKE) -C lib TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) depend
	@touch $@

depend: .depend

lib_clean:
	$(Q) $(MAKE) -C lib TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) clean

clean: lib_clean
	$(call DELFILE, $(BIN))
	$(call CLEAN)

