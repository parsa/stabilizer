ROOT = ../../../..
TARGETS = global_ref_smoke

build:: $(TARGETS)

include $(ROOT)/common.mk

CC = $(ROOT)/szc $(SZCFLAGS) -Rcode
CXX = $(CC)
CXXFLAGS += -O0

$(OBJS):: $(ROOT)/szc $(ROOT)/LLVMStabilizer.$(SHLIB_SUFFIX)

test:: global_ref_smoke
	@echo $(INDENT)[test] Running 'global_ref_smoke'
	@$(LD_PATH_VAR)=$(ROOT) ./global_ref_smoke

