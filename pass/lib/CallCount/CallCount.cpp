#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "header/CallCount.h"

PreservedAnalyses CallCount::run(const Function &F, 
  FunctionAnalysisManager &FAM) {
  for (const BasicBlock& BB : F) {
    int callInstCnt = 0;
    for (const Instruction& I : BB) {
        if (isa<CallInst>(I)) {
          callInstCnt++;
        }
    }
    dbgs() << "[CallCount] \n" << F.getName() << " [# of Call Instructions] " << callInstCnt << "\n";
  }
  return PreservedAnalyses::all();
}

extern "C" ::PassPluginLibraryInfo llvmGetPassPluginInfo() {  
  return {
    LLVM_PLUGIN_API_VERSION, "CallCountPlugin", LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      dbgs() << "[Plugin] Registering CallCount Pass\n";
      PB.registerPipelineParsingCallback(
        [](const StringRef Name, FunctionPassManager &FPM, 
           ArrayRef<PassBuilder::PipelineElement> Pipeline) {
          if (Name == "call-count") { 
            FPM.addPass(CallCount()); 
            return true; 
          }
          return false;
        }
      );
    } 
  };  
}
