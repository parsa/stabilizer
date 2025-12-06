#include "LowerIntrinsics.h"
#include "IntrinsicLibcalls.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

PreservedAnalyses LowerIntrinsicsPass::run(Module &m, ModuleAnalysisManager &MAM) {
    InitLibcalls();
    
    set<Function*> toDelete;
    
    for(auto &f : m) {
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
    
    for(Function* f : toDelete) {
        f->eraseFromParent();
    }
    
    return PreservedAnalyses::none();
}
