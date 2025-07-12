#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/PassManager.h"
using namespace llvm;
struct FunctionNamePrinter : public PassInfoMixin<FunctionNamePrinter> {
public:
    static PreservedAnalyses run(const Function &M,
    FunctionAnalysisManager &MAM);
};