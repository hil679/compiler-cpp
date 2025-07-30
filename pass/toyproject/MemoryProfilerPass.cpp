#include <iostream>

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

#include "llvm/Support/raw_ostream.h" //degs()

#include "header/MemoryProfilerPass.h"

PreservedAnalyses MemoryProfilerPass::run(Module& M, 
    ModuleAnalysisManager & MAM) {
    
    LLVMContext& Context = M.getContext();
    const DataLayout &Layout = M.getDataLayout();
    std::vector<Type*> ArgTys;
    ArgTys.push_back(llvm::PointerType::get(Type::getInt8Ty(Context), 0));
    ArgTys.push_back(M.getDataLayout().getIntPtrType(Context));         // size_t (load 크기)
    FunctionType* traceFuncTy = FunctionType::get(
        Type::getVoidTy(Context), ArgTys, /*isVarArg=*/false);

    FunctionCallee traceMalloc = M.getOrInsertFunction("traceMalloc", 
        traceFuncTy
        // PointerType::get(Type::getInt8Ty(Context), 0),
        // M.getDataLayout().getIntPtrType(Context),
        // Type::getInt8PtrTy(Context),
        //https://llvm.org/doxygen/classllvm_1_1Type.html
        // Type::getTypeSizeInBits(Type::getPrimitiveType(Context, Type::getTypeId())),
        // false
    );
    FunctionCallee traceAllocStack = M.getOrInsertFunction("traceAllocStack", 
        traceFuncTy
    );
    FunctionCallee traceLoad = M.getOrInsertFunction("traceLoad", 
        traceFuncTy
    );
    FunctionCallee traceStore = M.getOrInsertFunction("traceStore", 
        traceFuncTy
    );
    for (Function& F : M) {
        for (BasicBlock& BB : F) {
             for (Instruction& I : BB) {
                // IRBuilder<> Builder(&I);
                if (CallInst* call = dyn_cast<CallInst>(&I)) {
                    if (Function* calledFunc = call->getCalledFunction()) {
                        if (calledFunc->getName() == "malloc") {
                            IRBuilder<> MallocBuilder(call->getNextNonDebugInstruction());
 
                            // malloc의 반환 값 (%1)과 할당 크기를 traceMalloc에 전달
                            Value* allocatedPtr = MallocBuilder.CreatePointerCast(call, llvm::PointerType::get(Type::getInt8Ty(Context), 0));
                            Value* allocatedSize = call->getArgOperand(0); // malloc의 첫 번째 인수가 크기
                            MallocBuilder.CreateCall(traceMalloc, {allocatedPtr, allocatedSize});
                        }
                    }
                }
                if (AllocaInst* alloc = dyn_cast<AllocaInst>(&I)) {
                    // AllocaInst는 항상 Entry Block의 시작 부분에 오고, 그 결과(%1)를 바로 사용합니다.
                    // 따라서 AllocaInst 직후에 traceMalloc을 삽입하는 것이 안전합니다.
                    // IRBuilder의 삽입 지점을 alloc의 다음 인스트럭션으로 설정합니다.
                    // alloc->getNextNonDebugInstruction()을 사용하는 것이 가장 확실합니다.
                    IRBuilder<> AllocaBuilder(alloc->getNextNonDebugInstruction());
                    // alloc.getArraysize()
                    TypeSize size = Layout.getTypeSizeInBits(alloc->getAllocatedType());
                    Value* allocatedPtr = AllocaBuilder.CreatePointerCast(alloc, llvm::PointerType::get(Type::getInt8Ty(Context), 0));
                    Value* allocatedSize = ConstantInt::get(M.getDataLayout().getIntPtrType(Context), size/8);
                    AllocaBuilder.CreateCall(traceAllocStack, {allocatedPtr, allocatedSize});
                }
                if (LoadInst* load = dyn_cast<LoadInst>(&I)) {
                    IRBuilder<> LoadBuilder(load);

                    TypeSize size = Layout.getTypeSizeInBits(load->getPointerOperandType());
                    Value* loadedPtr = LoadBuilder.CreatePointerCast(load->getPointerOperand(), llvm::PointerType::get(Type::getInt8Ty(Context), 0));
                    Value* loadedSize = ConstantInt::get(M.getDataLayout().getIntPtrType(Context), size/8); // 바이트 단위로 size 전달
                    LoadBuilder.CreateCall(traceLoad, {loadedPtr, loadedSize});
                }
                if (StoreInst* store = dyn_cast<StoreInst>(&I)) {
                    IRBuilder<> StoreBuilder(store);

                    TypeSize size = Layout.getTypeSizeInBits(store->getPointerOperandType());
                    Value* storedPtr = StoreBuilder.CreatePointerCast(store->getPointerOperand(), llvm::PointerType::get(Type::getInt8Ty(Context), 0));
                    Value* storedSize = ConstantInt::get(M.getDataLayout().getIntPtrType(Context), size/8); // 바이트 단위로 size 전달
                    StoreBuilder.CreateCall(traceStore, {storedPtr, storedSize});
                }
             }
        }
    }
   
    return PreservedAnalyses::none();
}

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION,
        "MemoryProfilerPassPlugin",
        LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            dbgs() << "[DEBUG] Registering MemoryProfiler Pass";
            PB.registerPipelineParsingCallback(
                [](const StringRef name, ModulePassManager& MPM,
                    ArrayRef<PassBuilder::PipelineElement>) {
                        if (name == "memory-profiler") {
                            MPM.addPass(MemoryProfilerPass());
                            return true;
                        } else {
                            return false;
                        }
                    }
            );
        }
    };
}
