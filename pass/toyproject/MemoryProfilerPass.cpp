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
    SmallVector<Type*, 2> ArgTys;
    ArgTys.push_back(llvm::PointerType::get(Type::getInt8Ty(Context), 0)); // void* (load 주소)
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
    FunctionCallee traceLoad = M.getOrInsertFunction("traceLoad", 
        traceFuncTy
        // PointerType::get(Type::getInt8Ty(Context), 0),
        // M.getDataLayout().getIntPtrType(Context),
        // false
    );
    FunctionCallee traceStore = M.getOrInsertFunction("traceStore", 
        traceFuncTy
        // PointerType::get(Type::getInt8Ty(Context), 0),
        // M.getDataLayout().getIntPtrType(Context),
        // false
    );
    for (Function& F : M) {
        for (BasicBlock& BB : F) {
             for (Instruction& I : BB) {
                IRBuilder<> Builder(&I);
                if (AllocaInst* alloc = dyn_cast<AllocaInst>(&I)) {
                    // alloc.getArraysize()
                    TypeSize size = Layout.getTypeSizeInBits(alloc->getAllocatedType());
                    Value* allocatedPtr = Builder.CreatePointerCast(alloc, llvm::PointerType::get(Type::getInt8Ty(Context), 0));
                    Value* allocatedSize = ConstantInt::get(M.getDataLayout().getIntPtrType(Context), size);
                    Builder.CreateCall(traceMalloc, {allocatedPtr, allocatedSize});
                    // Builder.CreateCall(traceMalloc,{alloc->getAddressSpace(), size},"traceMalloc");
                }
                if (LoadInst* load = dyn_cast<LoadInst>(&I)) {
                    TypeSize size = Layout.getTypeSizeInBits(load->getPointerOperandType());
                    Value* loadedPtr = Builder.CreatePointerCast(load->getPointerOperand(), llvm::PointerType::get(Type::getInt8Ty(Context), 0));
                    Value* loadedSize = ConstantInt::get(M.getDataLayout().getIntPtrType(Context), M.getDataLayout().getTypeAllocSizeInBits(load->getPointerOperandType()) / 8); // 바이트 단위로 size 전달
                    Builder.CreateCall(traceLoad, {loadedPtr, loadedSize});
                    // Builder.CreateCall(traceLoad,{load->getPointerAddressSpace(), size},"traceLoad");
                }
                if (StoreInst* store = dyn_cast<StoreInst>(&I)) {
                    TypeSize size = Layout.getTypeSizeInBits(store->getPointerOperandType());
                    Value* storedPtr = Builder.CreatePointerCast(store->getPointerOperand(), llvm::PointerType::get(Type::getInt8Ty(Context), 0));
                    Value* storedSize = ConstantInt::get(M.getDataLayout().getIntPtrType(Context), M.getDataLayout().getTypeAllocSizeInBits(store->getPointerOperandType()) / 8); // 바이트 단위로 size 전달
                    Builder.CreateCall(traceStore, {storedPtr, storedSize});
                    // Builder.CreateCall(traceStore,{store->getPointerAddressSpace(), size},"traceStore");
                }
             }
        }
    }
   
    return PreservedAnalyses::all();
}

extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION,
        "MomoryProfilerPassPlugin",
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
