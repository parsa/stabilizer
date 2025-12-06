ROOT = ../../..
TARGETS = stack_smoke

build:: $(TARGETS)

include $(ROOT)/common.mk

CC = $(ROOT)/szc $(SZCFLAGS) -Rstack
CXX = $(CC)
CXXFLAGS += -O0

$(OBJS):: $(ROOT)/szc $(ROOT)/LLVMStabilizer.$(SHLIB_SUFFIX)

test:: stack_smoke
	@echo $(INDENT)[test] Running 'stack_smoke'
	@$(LD_PATH_VAR)=$(ROOT) ./stack_smoke

