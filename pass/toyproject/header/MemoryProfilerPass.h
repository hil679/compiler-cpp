#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/PassManager.h"

using namespace llvm;
struct MemoryProfilerPass : public PassInfoMixin<MemoryProfilerPass> {
public:
    static PreservedAnalyses run(Module &M, 
        ModuleAnalysisManager &MAM);
};