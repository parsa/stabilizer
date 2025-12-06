ROOT = ../../../..
TARGETS = code_move_smoke

build:: $(TARGETS)

include $(ROOT)/common.mk

CC = $(ROOT)/szc $(SZCFLAGS) -Rcode
CXX = $(CC)
CXXFLAGS += -O0

$(OBJS):: $(ROOT)/szc $(ROOT)/LLVMStabilizer.$(SHLIB_SUFFIX)

test:: code_move_smoke
	@echo $(INDENT)[test] Running 'code_move_smoke'
	@$(LD_PATH_VAR)=$(ROOT) ./code_move_smoke

