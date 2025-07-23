#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "header/DynamicCallCount.h"

PreservedAnalyses DynamicCallCount::run(Module &M, 
  ModuleAnalysisManager &MAM) {
  dbgs() << "[DynamicCallCount] Pass Entry\n";
  LLVMContext& Context = M.getContext();
  FunctionCallee countCall = M.getOrInsertFunction("countCall", Type::getVoidTy(Context), false);
  FunctionCallee printResult = M.getOrInsertFunction("printResult", Type::getVoidTy(Context), false);
  for (Function & F: M) {
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (CallInst* call = dyn_cast<CallInst>(&I)) {
          // ArrayRef<Value*>() -> 빈 배열
          CallInst::Create(countCall, ArrayRef<Value*>(),"", call);
        }
        if (ReturnInst* ret = dyn_cast<ReturnInst>(&I)) {
          CallInst::Create(printResult, ArrayRef<Value*>(),"",ret);
        }
      }
    }
  }
  
  return PreservedAnalyses::all();
}

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo() {  
  return {  
    LLVM_PLUGIN_API_VERSION, 
    "DynamicCallCountPlugin", 
    LLVM_VERSION_STRING, 
    [](PassBuilder &PB) {  
      dbgs() << "[Plugin] Registering DynamicCallCount Pass\n";
      PB.registerPipelineParsingCallback( 
        [](const StringRef name, ModulePassManager &MPM, 
           ArrayRef<PassBuilder::PipelineElement>) { 
          if (name == "dynamic-call-count") {
            MPM.addPass(DynamicCallCount());
            return true;
          } else {
            return false;
          }
        }
      ); 
    }
  };  
}
