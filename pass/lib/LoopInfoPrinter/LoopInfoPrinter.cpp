#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "header/LoopInfoPrinter.h"
#include "llvm/Analysis/LoopInfo.h"

using namespace llvm;
void printLoopInfo(LoopInfo &LI, Function& F) {
  for (BasicBlock& BB : F) {
    dbgs() << "BB name: ";
    BB.printAsOperand(dbgs());
    dbgs() << ", # of Loop: " << LI.getLoopDepth(&BB) <<"\n";
  }
}

PreservedAnalyses LoopInfoPrinter::run(Function &F, 
  FunctionAnalysisManager &FAM) {
  dbgs() << "[LoopInfoPrinter] Pass Entry\n";
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  printLoopInfo(LI, F);
  for (const Loop *L : LI) {
    dbgs() << "[LoopInfoPrinter] " << L->getLoopID() << " " << L->getName() << " " << L->getLoopDepth()<< "\n";
    dbgs() << "  - Number of blocks: " << L->getBlocks().size() << "\n";

    llvm::SmallVector<BasicBlock *, 8> LoopLatches;
    L->getLoopLatches(LoopLatches);
    for (BasicBlock *Latch : LoopLatches) {
      dbgs() << "  - Latch block: " << *Latch << "\n";
    }

    llvm::SmallVector<BasicBlock *, 8> ExitBlocks;
    L->getExitBlocks(ExitBlocks);
    for (BasicBlock *Exit : ExitBlocks ) {
      dbgs() << "  - Exit block: " << *Exit << "\n";
    }
  }

  return PreservedAnalyses::all();
}

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo() {  
  return {  
    LLVM_PLUGIN_API_VERSION,  
    "LoopInfoPrinterPlugin",  
    LLVM_VERSION_STRING,  
    [](PassBuilder &PB) {  
      dbgs() << "[Plugin] Registering LoopInfoPrinter Pass\n";
      PB.registerAnalysisRegistrationCallback(
                [](FunctionAnalysisManager &FAM) {
                    FAM.registerPass([&] { return LoopAnalysis(); });
                }
            );
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
