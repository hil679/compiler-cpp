#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/IR/PassManager.h"
using namespace llvm;
struct InsertIncFunction : public PassInfoMixin<InsertIncFunction> {
public:
    static PreservedAnalyses run(Module &M,
    ModuleAnalysisManager &MAM);
};