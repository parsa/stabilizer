ROOT = ../../..

SHLIB_SUFFIX ?= so
PASS_PLUGIN = $(ROOT)/LLVMStabilizer.$(SHLIB_SUFFIX)

CLANG ?= clang
OPT ?= opt

OBJDIR = obj

.PHONY: test clean run-memcpy run-math run-lifetime run-unsupported run-opaque-memcpy run-memmove run-warning run-vector run-volatile

test:: $(PASS_PLUGIN)
	@$(MAKE) -s run-memcpy
	@$(MAKE) -s run-math
	@$(MAKE) -s run-memmove
	@$(MAKE) -s run-opaque-memcpy
	@$(MAKE) -s run-lifetime
	@$(MAKE) -s run-unsupported
	@$(MAKE) -s run-warning
	@$(MAKE) -s run-vector
	@$(MAKE) -s run-volatile

run-memcpy: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-intrinsics] Checking memcpy/memset lowering
	@$(CLANG) -O0 -emit-llvm -S memcpy_memset.c -o $(OBJDIR)/memcpy.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-intrinsics $(OBJDIR)/memcpy.ll -S -o $(OBJDIR)/memcpy.lowered.ll
	@if grep -q "llvm\.memcpy" $(OBJDIR)/memcpy.lowered.ll; then \
		echo "ERROR: llvm.memcpy still present after lowering"; \
		exit 1; \
	fi
	@if ! grep -q "@memcpy" $(OBJDIR)/memcpy.lowered.ll; then \
		echo "ERROR: call to memcpy not found after lowering"; \
		exit 1; \
	fi
	@if grep -q "llvm\.memset" $(OBJDIR)/memcpy.lowered.ll; then \
		echo "ERROR: llvm.memset still present after lowering"; \
		exit 1; \
	fi
	@if ! grep -q "memset" $(OBJDIR)/memcpy.lowered.ll; then \
		echo "ERROR: call to memset not found after lowering"; \
		exit 1; \
	fi

run-math: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-intrinsics] Checking math intrinsic lowering
	@$(CLANG) -O0 -emit-llvm -S math_calls.c -o $(OBJDIR)/math.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-intrinsics $(OBJDIR)/math.ll -S -o $(OBJDIR)/math.lowered.ll
	@for intrinsic in llvm.sqrt llvm.log llvm.pow llvm.fabs llvm.log10 llvm.exp; do \
		if grep -q $$intrinsic $(OBJDIR)/math.lowered.ll; then \
			echo "ERROR: $$intrinsic still present after lowering"; \
			exit 1; \
		fi; \
	done
	@for fn in "@sqrtf" "@logf" "@powf" "@fabsf" "@log10f" "@expf"; do \
		if ! grep -q $$fn $(OBJDIR)/math.lowered.ll; then \
			echo "ERROR: expected $$fn call not found after lowering"; \
			exit 1; \
		fi; \
	done

run-memmove: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-intrinsics] Checking memmove lowering
	@$(CLANG) -O0 -emit-llvm -S memmove_use.c -o $(OBJDIR)/memmove.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-intrinsics $(OBJDIR)/memmove.ll -S -o $(OBJDIR)/memmove.lowered.ll
	@if grep -q "llvm\.memmove" $(OBJDIR)/memmove.lowered.ll; then \
		echo "ERROR: llvm.memmove still present after lowering"; \
		exit 1; \
	fi
	@if ! grep -q "@memmove" $(OBJDIR)/memmove.lowered.ll; then \
		echo "ERROR: expected memmove call not found"; \
		exit 1; \
	fi

run-opaque-memcpy: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-intrinsics] Checking opaque memcpy lowering
	@$(CLANG) -O0 -emit-llvm -S opaque_memcpy.c -o $(OBJDIR)/opaque_memcpy.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-intrinsics $(OBJDIR)/opaque_memcpy.ll -S -o $(OBJDIR)/opaque_memcpy.lowered.ll
	@if grep -q "llvm\.memcpy\.p0\.p0\.i32" $(OBJDIR)/opaque_memcpy.lowered.ll; then \
		echo "ERROR: opaque llvm.memcpy still present after lowering"; \
		exit 1; \
	fi
	@if ! grep -q "@memcpy" $(OBJDIR)/opaque_memcpy.lowered.ll; then \
		echo "ERROR: expected memcpy call not found for opaque pointer variant"; \
		exit 1; \
	fi

run-lifetime: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-intrinsics] Checking lifetime intrinsic preservation
	@$(CLANG) -O1 -emit-llvm -S lifetime.c -o $(OBJDIR)/lifetime.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-intrinsics $(OBJDIR)/lifetime.ll -S -o $(OBJDIR)/lifetime.lowered.ll
	@if ! grep -q "llvm\.lifetime.start" $(OBJDIR)/lifetime.lowered.ll; then \
		echo "ERROR: llvm.lifetime.start missing after lowering"; \
		exit 1; \
	fi
	@if ! grep -q "llvm\.lifetime.end" $(OBJDIR)/lifetime.lowered.ll; then \
		echo "ERROR: llvm.lifetime.end missing after lowering"; \
		exit 1; \
	fi

run-unsupported: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-intrinsics] Checking warning-free handling for supported intrinsics
	@$(CLANG) -O0 -emit-llvm -S unsupported_fabs.c -o $(OBJDIR)/unsupported.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-intrinsics $(OBJDIR)/unsupported.ll -S -o $(OBJDIR)/unsupported.lowered.ll 2> $(OBJDIR)/unsupported.log
	@if grep -q "warning: unable to handle intrinsic" $(OBJDIR)/unsupported.log; then \
		echo "ERROR: unexpected warning when lowering fabs"; \
		cat $(OBJDIR)/unsupported.log; \
		exit 1; \
	fi
	@if grep -q "llvm\.fabs" $(OBJDIR)/unsupported.lowered.ll; then \
		echo "ERROR: llvm.fabs should be lowered but remains"; \
		exit 1; \
	fi

run-warning: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-intrinsics] Checking warning emission for unsupported intrinsic
	@$(CLANG) -O0 -emit-llvm -S unsupported_intrinsic.c -o $(OBJDIR)/unsupported_intrinsic.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-intrinsics $(OBJDIR)/unsupported_intrinsic.ll -S -o $(OBJDIR)/unsupported_intrinsic.lowered.ll 2> $(OBJDIR)/unsupported_intrinsic.log || true
	@if ! grep -q "warning: unable to handle intrinsic llvm\.abs\.i32" $(OBJDIR)/unsupported_intrinsic.log; then \
		echo "ERROR: expected warning for llvm.abs.i32 not emitted"; \
		exit 1; \
	fi

run-vector: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-intrinsics] Checking vector intrinsic lowering
	@$(CLANG) -O0 -emit-llvm -S vector_math.c -o $(OBJDIR)/vector.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-intrinsics $(OBJDIR)/vector.ll -S -o $(OBJDIR)/vector.lowered.ll 2> $(OBJDIR)/vector.log
	@if grep -q "warning" $(OBJDIR)/vector.log; then \
		echo "ERROR: unexpected warning while handling vector intrinsics"; \
		cat $(OBJDIR)/vector.log; \
		exit 1; \
	fi
	@if ! grep -q "llvm\.sqrt" $(OBJDIR)/vector.lowered.ll; then \
		echo "ERROR: vector intrinsic unexpectedly removed"; \
		exit 1; \
	fi

run-volatile: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-intrinsics] Checking volatile memcpy lowering
	@$(CLANG) -O0 -emit-llvm -S volatile_memcpy.c -o $(OBJDIR)/volatile.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-intrinsics $(OBJDIR)/volatile.ll -S -o $(OBJDIR)/volatile.lowered.ll
	@if grep -q "llvm\.memcpy" $(OBJDIR)/volatile.lowered.ll; then \
		echo "ERROR: volatile llvm.memcpy still present after lowering"; \
		exit 1; \
	fi

clean:
	@rm -rf $(OBJDIR)

