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

using namespace llvm;


bool lowerInstrinsicsPass(Module &m)
{
    InitLibcalls();

    std::set<Function*> toDelete;

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

    for(std::set<Function*>::iterator iter = toDelete.begin(); iter != toDelete.end(); iter++) {
        (*iter)->eraseFromParent();
    }

    return true;
}

struct LowerIntrinsicsLegacy : public ModulePass
{
    static char ID;

    LowerIntrinsicsLegacy()
      : ModulePass(ID)
    {
    }

    virtual bool runOnModule(Module& m)
    {
        return ::lowerInstrinsicsPass(m);
    }
};

// -----------------------------------------------------------------------------
// Legacy Pass Manager Registration
// -----------------------------------------------------------------------------
char LowerIntrinsicsLegacy::ID = 0;
static RegisterPass<LowerIntrinsicsLegacy> X(
    "lower-intrinsics", "Replace all intrinsics with direct libcalls");
