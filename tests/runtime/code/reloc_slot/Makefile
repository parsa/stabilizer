ROOT = ../../../..
TARGETS = reloc_slot_smoke

build:: $(TARGETS)

include $(ROOT)/common.mk

CC = $(ROOT)/szc $(SZCFLAGS) -Rcode
CXX = $(CC)
CXXFLAGS += -O0

$(OBJS):: $(ROOT)/szc $(ROOT)/LLVMStabilizer.$(SHLIB_SUFFIX)

test:: reloc_slot_smoke
	@echo $(INDENT)[test] Running 'reloc_slot_smoke'
	@$(LD_PATH_VAR)=$(ROOT) ./reloc_slot_smoke

