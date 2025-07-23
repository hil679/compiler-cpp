#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/PassManager.h"

using namespace llvm;

struct LoopInfoPrinter : public PassInfoMixin<LoopInfoPrinter> {
    public:
        static PreservedAnalyses run(Function& F,
        FunctionAnalysisManager& FAM);
};