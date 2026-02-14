ROOT = ..

TARGETS = $(ROOT)/LLVMStabilizer.$(SHLIB_SUFFIX)
LIBS = 

include $(ROOT)/common.mk

# Build the pass against the active LLVM toolchain (via `llvm-config`).
# Ensure the `llvm-config` in PATH matches the LLVM you intend to use.
CXXFLAGS += $(shell llvm-config --cxxflags)
CXXLIB += $(shell llvm-config --ldflags --libs --system-libs)

CXXFLAGS += -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -fno-exceptions -fno-rtti -fno-common
