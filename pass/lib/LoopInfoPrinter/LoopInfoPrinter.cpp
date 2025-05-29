#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "LoopInfoPrinter/LoopInfoPrinter.h"

PreservedAnalyses LoopInfoPrinter::run(const Function &F, 
  FunctionAnalysisManager &FAM) {
  dbgs() << "[LoopInfoPrinter] Pass Entry\n";
  return PreservedAnalyses::all();
}

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo() {  
  return {  
    LLVM_PLUGIN_API_VERSION,  
    "LoopInfoPrinterPlugin",  
    LLVM_VERSION_STRING,  
    [](PassBuilder &PB) {  
      dbgs() << "[Plugin] Registering LoopInfoPrinter Pass\n";
      PB.registerPipelineParsingCallback(  
        [](const StringRef name, FunctionPassManager &FPM,  
           ArrayRef<PassBuilder::PipelineElement>) {  
          if (name == "loop-info-printer") {  
            FPM.addPass(LoopInfoPrinter());  
            return true;  
          } else { 
            return false;  
          }
        }
      );  
    }  
  };  
}
