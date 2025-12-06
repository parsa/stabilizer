ROOT = ../../..
TARGETS = heap_smoke

build:: $(TARGETS)

include $(ROOT)/common.mk

CC = $(ROOT)/szc $(SZCFLAGS) -Rheap
CXX = $(CC)
CXXFLAGS += -O0

$(OBJS):: $(ROOT)/szc $(ROOT)/LLVMStabilizer.$(SHLIB_SUFFIX)

test:: heap_smoke
	@echo $(INDENT)[test] Running 'heap_smoke'
	@$(LD_PATH_VAR)=$(ROOT) ./heap_smoke

