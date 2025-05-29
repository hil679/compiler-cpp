#ifndef __PASS_CALL_COUNT_H__
#define __PASS_CALL_COUNT_H__

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/PassManager.h"

using namespace llvm;

struct CallCount : public PassInfoMixin<CallCount> {
public:
  static PreservedAnalyses run(const Function &F, FunctionAnalysisManager &FAM);
};

#endif
