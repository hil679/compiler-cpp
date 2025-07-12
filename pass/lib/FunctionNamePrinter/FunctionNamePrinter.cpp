#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

#include "header/FunctionNamePrinter.h"

PreservedAnalyses FunctionNamePrinter::run(const Function &F, 
  FunctionAnalysisManager &FAM) {
  dbgs() << "[FunctionNamePrinter] " << F.getName() << "\n";
  return PreservedAnalyses::all();
}

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo() {  
  return {  
    LLVM_PLUGIN_API_VERSION,  
    "FunctionNamePrinterPlugin",  
    LLVM_VERSION_STRING,  
    [](PassBuilder &PB) {  
      dbgs() << "[Plugin] Registering FunctionNamePrinter Pass\n";
      PB.registerPipelineParsingCallback(  
        [](const StringRef name, FunctionPassManager &FPM,  
           ArrayRef<PassBuilder::PipelineElement>) {  
          if (name == "function-name-printer") { 
            FPM.addPass(FunctionNamePrinter());  
            return true;  
          } else {  
            return false;  
          }
        }
      );  
    }  
  };  
}
