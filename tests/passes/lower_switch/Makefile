ROOT = ../../..

SHLIB_SUFFIX ?= so
PASS_PLUGIN = $(ROOT)/LLVMStabilizer.$(SHLIB_SUFFIX)

CLANG ?= clang
OPT ?= opt

CLANGFLAGS = -std=gnu11 -O0 -Xclang -disable-O0-optnone -emit-llvm -S -Wno-unused-label

OBJDIR = obj

.PHONY: test clean run-dense run-sparse run-phi run-callbr run-multi run-i64 run-unreachable

test:: $(PASS_PLUGIN)
	@$(MAKE) -s run-dense
	@$(MAKE) -s run-sparse
	@$(MAKE) -s run-phi
	@$(MAKE) -s run-callbr
	@$(MAKE) -s run-multi
	@$(MAKE) -s run-i64
	@$(MAKE) -s run-unreachable

run-dense: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-switch] Checking dense switch lowering
	@$(CLANG) $(CLANGFLAGS) dense_switch.c -o $(OBJDIR)/dense.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-switch $(OBJDIR)/dense.ll -S -o $(OBJDIR)/dense.lowered.ll
	@if grep -qw "switch" $(OBJDIR)/dense.lowered.ll; then \
		echo "ERROR: switch instruction still present in dense lowering"; \
		exit 1; \
	fi

run-sparse: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-switch] Checking sparse switch lowering
	@$(CLANG) $(CLANGFLAGS) sparse_switch.c -o $(OBJDIR)/sparse.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-switch $(OBJDIR)/sparse.ll -S -o $(OBJDIR)/sparse.lowered.ll
	@if grep -qw "switch" $(OBJDIR)/sparse.lowered.ll; then \
		echo "ERROR: switch instruction still present in sparse lowering"; \
		exit 1; \
	fi

run-phi: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-switch] Checking PHI preservation
	@$(CLANG) $(CLANGFLAGS) phi_switch.c -o $(OBJDIR)/phi.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-switch $(OBJDIR)/phi.ll -S -o $(OBJDIR)/phi.lowered.ll
	@if grep -qw "switch" $(OBJDIR)/phi.lowered.ll; then \
		echo "ERROR: switch instruction still present in PHI test"; \
		exit 1; \
	fi
	@if ! grep -q "phi" $(OBJDIR)/phi.lowered.ll; then \
		echo "ERROR: expected PHI nodes missing after lowering"; \
		exit 1; \
	fi

run-callbr: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-switch] Checking callbr + switch lowering
	@$(CLANG) $(CLANGFLAGS) callbr_switch.c -o $(OBJDIR)/callbr.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-switch $(OBJDIR)/callbr.ll -S -o $(OBJDIR)/callbr.lowered.ll
	@if grep -qw "switch" $(OBJDIR)/callbr.lowered.ll; then \
		echo "ERROR: switch instruction still present in callbr test"; \
		exit 1; \
	fi
	@if ! grep -q "callbr" $(OBJDIR)/callbr.lowered.ll; then \
		echo "ERROR: callbr unexpectedly removed"; \
		exit 1; \
	fi

run-multi: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-switch] Checking multi-function module lowering
	@$(CLANG) $(CLANGFLAGS) multi_module_switch.c -o $(OBJDIR)/multi.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-switch $(OBJDIR)/multi.ll -S -o $(OBJDIR)/multi.lowered.ll
	@if grep -qw "switch" $(OBJDIR)/multi.lowered.ll; then \
		echo "ERROR: switch instruction still present in multi-function module"; \
		exit 1; \
	fi

run-i64: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-switch] Checking 64-bit switch lowering
	@$(CLANG) $(CLANGFLAGS) int64_switch.c -o $(OBJDIR)/int64.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-switch $(OBJDIR)/int64.ll -S -o $(OBJDIR)/int64.lowered.ll
	@if grep -qw "switch" $(OBJDIR)/int64.lowered.ll; then \
		echo "ERROR: switch instruction still present in int64 test"; \
		exit 1; \
	fi

run-unreachable: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-switch] Checking unreachable/default-only lowering
	@$(CLANG) $(CLANGFLAGS) unreachable_switch.c -o $(OBJDIR)/unreachable.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-switch $(OBJDIR)/unreachable.ll -S -o $(OBJDIR)/unreachable.lowered.ll
	@if grep -qw "switch" $(OBJDIR)/unreachable.lowered.ll; then \
		echo "ERROR: switch instruction still present in unreachable test"; \
		exit 1; \
	fi

clean:
	@rm -rf $(OBJDIR)

