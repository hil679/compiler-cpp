#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/PassManager.h"
using namespace llvm;
struct InstCount : public PassInfoMixin<InstCount> {
public:
    static PreservedAnalyses run(const Function &M,
    FunctionAnalysisManager &MAM);
};