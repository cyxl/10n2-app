
include $(APPDIR)/Make.defs
include $(SDKDIR)/Make.defs

# Audio application info

PROGNAME  = asmp_test
PRIORITY  = 100
STACKSIZE = 2048
#MODULE    = 

# Audio Example

MAINSRC = asmp_test.cxx

# Audio Example paths
#AUDIODIR = $(SDKDIR)$(DELIM)modules$(DELIM)audio
WORKERDIR = ./asmp_test_worker/include

# Audio Example flags

CXXFLAGS += ${shell $(INCDIR) $(INCDIROPT) "$(CC)" "$(WORKERDIR)"}


CXXFLAGS += -D_POSIX
#CXXFLAGS += -DUSE_MEMMGR_FENCE
#CXXFLAGS += -DATTENTION_USE_FILENAME_LINE

include $(APPDIR)/Application.mk

build_worker:
	$(Q) $(MAKE) -C asmp_test_worker TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV)

$(OBJS): build_worker

clean_worker:
	@$(MAKE) -C asmp_test_worker TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" APPDIR="$(APPDIR)" CROSSDEV=$(CROSSDEV) clean

clean:: clean_worker

