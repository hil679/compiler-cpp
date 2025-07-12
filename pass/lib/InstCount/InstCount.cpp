#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "header/InstCount.h"

PreservedAnalyses InstCount::run(const Function &F, 
  FunctionAnalysisManager &FAM) {

  for (const BasicBlock &BB : F) {
    dbgs() << "[InstCount] " << F.getName() << " [# of Instructions] "<< BB.size()<<"\n";
  }
  
  return PreservedAnalyses::all();
}

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "InstCountPlugin",
    LLVM_VERSION_STRING,
    [](PassBuilder &PB) {
      dbgs() << "[Plugin] Registering InstCount Pass\n";
      PB.registerPipelineParsingCallback(
        [](const StringRef name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (name == "inst-count") {
            FPM.addPass(InstCount());
            return true; 
          } else {
            return false;
          }
        }
      );  
    }  
  };  
}
