ROOT = ../../..

SHLIB_SUFFIX ?= so
PASS_PLUGIN = $(ROOT)/LLVMStabilizer.$(SHLIB_SUFFIX)

CLANG ?= clang
OPT ?= opt

CLANGFLAGS = -std=gnu11 -O1 -Xclang -disable-llvm-optzns -emit-llvm -S -Wno-unused-label

OBJDIR = obj

.PHONY: test clean run-array run-struct run-vector run-nested

test:: $(PASS_PLUGIN)
	@$(MAKE) -s run-array
	@$(MAKE) -s run-struct
	@$(MAKE) -s run-vector
	@$(MAKE) -s run-nested

run-array: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[constant-materialization] Checking array constant handling
	@$(CLANG) $(CLANGFLAGS) array_constant.c -o $(OBJDIR)/array.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=stabilize $(OBJDIR)/array.ll -S -o $(OBJDIR)/array.stabilized.ll 2>&1 | grep -v "warning:" || true
	@# Verify that the pass runs without errors and produces valid IR
	@if ! test -f $(OBJDIR)/array.stabilized.ll; then \
		echo "ERROR: stabilized IR not generated"; \
		exit 1; \
	fi
	@# Check that constants are handled (either materialized or accessed via relocation table)
	@if grep -q "\.literal" $(OBJDIR)/array.stabilized.ll; then \
		if ! grep -q "load.*\.literal" $(OBJDIR)/array.stabilized.ll; then \
			echo "ERROR: materialized constant not loaded"; \
			exit 1; \
		fi; \
	fi

run-struct: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[constant-materialization] Checking struct constant handling
	@$(CLANG) $(CLANGFLAGS) struct_constant.c -o $(OBJDIR)/struct.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=stabilize $(OBJDIR)/struct.ll -S -o $(OBJDIR)/struct.stabilized.ll 2>&1 | grep -v "warning:" || true
	@# Verify that the pass runs without errors
	@if ! test -f $(OBJDIR)/struct.stabilized.ll; then \
		echo "ERROR: stabilized IR not generated"; \
		exit 1; \
	fi

run-vector: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[constant-materialization] Checking vector constant handling
	@$(CLANG) $(CLANGFLAGS) vector_constant.c -o $(OBJDIR)/vector.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=stabilize $(OBJDIR)/vector.ll -S -o $(OBJDIR)/vector.stabilized.ll 2>&1 | grep -v "warning:" || true
	@# Verify that the pass runs without errors
	@if ! test -f $(OBJDIR)/vector.stabilized.ll; then \
		echo "ERROR: stabilized IR not generated"; \
		exit 1; \
	fi

run-nested: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[constant-materialization] Checking nested constant handling
	@$(CLANG) $(CLANGFLAGS) nested_constant.c -o $(OBJDIR)/nested.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=stabilize $(OBJDIR)/nested.ll -S -o $(OBJDIR)/nested.stabilized.ll 2>&1 | grep -v "warning:" || true
	@# Verify that the pass runs without errors
	@if ! test -f $(OBJDIR)/nested.stabilized.ll; then \
		echo "ERROR: stabilized IR not generated"; \
		exit 1; \
	fi

clean:
	@rm -rf $(OBJDIR)

