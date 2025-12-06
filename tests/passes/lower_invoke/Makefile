ROOT = ../../..

SHLIB_SUFFIX ?= so
PASS_PLUGIN = $(ROOT)/LLVMStabilizer.$(SHLIB_SUFFIX)

CLANG ?= clang
CXX = clang++
OPT ?= opt

CLANGFLAGS = -O0 -Xclang -disable-O0-optnone -emit-llvm -S
CXXFLAGS_LLVM = -std=c++17 -O0 -fexceptions -Xclang -disable-O0-optnone -emit-llvm -S

OBJDIR = obj

.PHONY: test clean run-setjmp run-throw run-nested run-multi run-personality run-cleanup

test:: $(PASS_PLUGIN)
	@$(MAKE) -s run-setjmp
	@$(MAKE) -s run-throw
	@$(MAKE) -s run-nested
	@$(MAKE) -s run-multi
	@$(MAKE) -s run-personality
	@$(MAKE) -s run-cleanup

run-setjmp: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-invoke] Checking setjmp lowering
	@$(CLANG) $(CLANGFLAGS) setjmp_longjmp.c -o $(OBJDIR)/setjmp.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-invoke $(OBJDIR)/setjmp.ll -S -o $(OBJDIR)/setjmp.lowered.ll
	@if grep -q "invoke" $(OBJDIR)/setjmp.lowered.ll; then \
		echo "ERROR: invoke instruction still present after lower-invoke"; \
		exit 1; \
	fi

run-throw: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-invoke] Checking C++ throw lowering
	@$(CXX) $(CXXFLAGS_LLVM) throw.cpp -o $(OBJDIR)/throw.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-invoke $(OBJDIR)/throw.ll -S -o $(OBJDIR)/throw.lowered.ll
	@if grep -q "invoke" $(OBJDIR)/throw.lowered.ll; then \
		echo "ERROR: invoke instruction still present in throw test"; \
		exit 1; \
	fi

run-nested: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-invoke] Checking nested throw lowering
	@$(CXX) $(CXXFLAGS_LLVM) throw_nested.cpp -o $(OBJDIR)/nested.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-invoke $(OBJDIR)/nested.ll -S -o $(OBJDIR)/nested.lowered.ll
	@if grep -q "invoke" $(OBJDIR)/nested.lowered.ll; then \
		echo "ERROR: invoke instruction still present in nested throw test"; \
		exit 1; \
	fi

run-multi: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-invoke] Checking multi-function module lowering
	@$(CXX) $(CXXFLAGS_LLVM) multi_throw.cpp -o $(OBJDIR)/multi.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-invoke $(OBJDIR)/multi.ll -S -o $(OBJDIR)/multi.lowered.ll
	@if grep -q "invoke" $(OBJDIR)/multi.lowered.ll; then \
		echo "ERROR: invoke instruction still present in multi-function test"; \
		exit 1; \
	fi

run-personality: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-invoke] Checking mixed personality lowering
	@$(CXX) $(CXXFLAGS_LLVM) personality_mix.cpp -o $(OBJDIR)/personality.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-invoke $(OBJDIR)/personality.ll -S -o $(OBJDIR)/personality.lowered.ll
	@if grep -q "invoke" $(OBJDIR)/personality.lowered.ll; then \
		echo "ERROR: invoke instruction still present in personality test"; \
		exit 1; \
	fi

run-cleanup: $(PASS_PLUGIN)
	@mkdir -p $(OBJDIR)
	@echo $(INDENT)[lower-invoke] Checking cleanup handler lowering
	@$(CXX) $(CXXFLAGS_LLVM) cleanup_throw.cpp -o $(OBJDIR)/cleanup.ll
	@$(OPT) -load-pass-plugin=$(PASS_PLUGIN) -passes=lower-invoke $(OBJDIR)/cleanup.ll -S -o $(OBJDIR)/cleanup.lowered.ll
	@if grep -q "invoke" $(OBJDIR)/cleanup.lowered.ll; then \
		echo "ERROR: invoke instruction still present in cleanup test"; \
		exit 1; \
	fi

clean:
	@rm -rf $(OBJDIR)

