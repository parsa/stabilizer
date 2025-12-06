#pragma once
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"

struct LowerIntrinsicsPass : public llvm::PassInfoMixin<LowerIntrinsicsPass> {
    llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);
};

