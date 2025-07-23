#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include "header/InsertIncFunction.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

PreservedAnalyses InsertIncFunction::run(Module &M, 
  ModuleAnalysisManager &MAM) {
  dbgs() << "[InsertIncFunction] Pass Entry\n";
  
  LLVMContext &Context = M.getContext();
  Type *Int32Ty = Type::getInt32Ty(Context);
  FunctionType *IncFuncType = FunctionType::get(Int32Ty, {Int32Ty}, false);

  // Module에 "inc" 함수를 추가합니다. 이미 존재하면 기존 함수를 가져옴.
  Function *IncFunc = Function::Create(IncFuncType, Function::ExternalLinkage, "inc", &M);

  // "inc" 함수를 위한 기본 블록 생성
  BasicBlock *EntryBB = BasicBlock::Create(Context, "entry", IncFunc);

  // 3. Inc 함수에 명령어 채우기
  IRBuilder<> Builder(EntryBB); // EntryBB의 끝에 명령어를 삽입할 IRBuilder 생성

  // 함수의 첫 번째 인자를 가져옵니다.
  Argument *N = &*IncFunc->arg_begin();
  N->setName("n"); // 인자 이름을 "n"으로 설정 (선택 사항)

  // 정수 상수 1을 생성
  Value *One = ConstantInt::get(Int32Ty, 1);
  // n + 1 연산
  Value *Result = Builder.CreateAdd(N, One, "result");

  // 결과 반환
  Builder.CreateRet(Result);

  return PreservedAnalyses::none();
}

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo() {  
  return {  
    LLVM_PLUGIN_API_VERSION,  
    "InsertIncFunctionPlugin",
    LLVM_VERSION_STRING, 
    [](PassBuilder &PB) {
      dbgs() << "[Plugin] Registering InsertIncFunction Pass\n";
      PB.registerPipelineParsingCallback(
        [](const StringRef name, ModulePassManager &MPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (name == "insert-inc-function") {
            MPM.addPass(InsertIncFunction()); 
            return true; 
          } else {  
            return false; 
          }
        }
      ); 
    }  
  };  
}
