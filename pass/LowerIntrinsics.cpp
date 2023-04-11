#define DEBUG_TYPE "lower_intrinsics"

#include <iostream>
#include <set>

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"

#include "llvm/Support/raw_ostream.h"

#include "IntrinsicLibcalls.h"

using namespace std;
using namespace llvm;


bool runOnModule(Module &m)
{
    InitLibcalls();

    set<Function*> toDelete;

    for(Module::iterator fun = m.begin(); fun != m.end(); fun++) {
        llvm::Function &f = *fun;
        if(f.isIntrinsic() && !isAlwaysInlined(f.getName())) {
            StringRef r = GetLibcall(f.getName());

            if(!r.empty()) {
                Function *f_extern = m.getFunction(r);
                if(!f_extern) {
                    f_extern = Function::Create(
                        f.getFunctionType(),
                        Function::ExternalLinkage,
                        r,
                        &m
                    );
                }
                f.replaceAllUsesWith(f_extern);
                toDelete.insert(&f);

            } else {
                errs()<<"warning: unable to handle intrinsic "<<f.getName().str()<<"\n";
            }
        }
    }

    for(set<Function*>::iterator iter = toDelete.begin(); iter != toDelete.end(); iter++) {
        (*iter)->eraseFromParent();
    }

    return true;
}

struct LowerIntrinsics: public PassInfoMixin<LowerIntrinsics> {
    PreservedAnalyses run(Module &m, ModuleAnalysisManager &mam) {
        runOnModule(m);
        return PreservedAnalyses::none();
    }

    static bool isRequired() { return true; }
};

struct LowerIntrinsicsLegacy : public ModulePass
{
    static char ID;

    LowerIntrinsicsLegacy()
      : ModulePass(ID)
    {
    }

    virtual bool runOnModule(Module& m)
    {
        return ::runOnModule(m);
    }
};

// -----------------------------------------------------------------------------
// New Pass Manager Registration
// -----------------------------------------------------------------------------
namespace {
    bool registerLowerIntrinsicsCallback(StringRef Name, ModulePassManager& MPM,
        ArrayRef<PassBuilder::PipelineElement>)
    {
        if (Name == "lower-intrinsics")
        {
            MPM.addPass(LowerIntrinsics());
            return true;
        }
        return false;
    }

    void registerPipelineParsingCallback(PassBuilder& PB)
    {
        PB.registerPipelineParsingCallback(registerLowerIntrinsicsCallback);
    }
}    // namespace

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo()
{
    return {LLVM_PLUGIN_API_VERSION, "LowerIntrinsics", LLVM_VERSION_STRING,
        registerPipelineParsingCallback};
}

// -----------------------------------------------------------------------------
// Legacy Pass Manager Registration
// -----------------------------------------------------------------------------
char LowerIntrinsicsLegacy::ID = 0;
static RegisterPass<LowerIntrinsicsLegacy> X(
    "lower-intrinsics", "Replace all intrinsics with direct libcalls");
