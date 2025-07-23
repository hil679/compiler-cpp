# Insert a New Function

## Notion
### IR Code Transformation
Transformation은 코드 수정 삭제 모든 경우가 가능하다. \
source code변경 시, PreservedAnalyses::none()을 run함수에서 반환한다.
~~~
PreservedAnalyses HelloFunction::run(const Function &F,
    ModuleAnalysisManager &FAM) {
    ...
    return PreservedAnalyses::none(); // llvm::PreservedAnalyses::all();
}
~~~

### How to Create Function
- getOrInsertFunction in Module class
    - getOrInsertFunction(Stringref name, ret type, args...)
        - 함수의 반환 타입과 인자 타입을 직접 지정하여 getOrInsertFunction을 사용
        - M.getOrInsertFunction("add", Type::getInt64Ty(Context), ...) -> 직접 ret타입 지정
    - getOrInsertFunction(Stringref name, FunctionType* T)
        - FunctionType : 함수의 전체적인 시그니처(Signature)를 정의하는 LLVM 타입
            - 시그니처는 함수의 반환 타입과 인자 타입들을 포함
        - FunctionType::get() 함수를 사용하여 함수 타입을 생성
        - FunctionType* functype = FunctionType::get(Type::getInt64Ty(Context), -> return type \
            ```vector<Type*>```, -> argument type \
            false -> 가변인자 받을 수 없음)

- Create in the Function class
***Function::Create***
~~~
Function* addFun = Function::Create(
    addFunType,                   // Function Type*
    GlobalValue::InternalLinkage, // Linkage type -> InternalLinkage는 이 함수가 현재 컴파일 단위 내에서만 참조될 수 있음
    "add",                        // Function name (const Twine &)
    &M                            // Module -> 함수를 첨부할 Module 객체의 포인터
);
~~~
* InternalLinkage: 다른 .c 또는 .cpp 파일에서는 이 함수/변수의 이름을 알 수 없으며, 따라서 이들을 직접 호출하거나 참조할 수 없다.
* Twine은 LLVM 라이브러리에서 사용되는 특별한 문자열 클래스,  효율적인 문자열 연결을 위해 설계된 경량 문자열 클래스

### How to Create BasicBlock
- Create(LLVM 컨텍스트 객체, BasicBlock의 선택적인 이름, BasicBlock이 속할 Function 객체)
    - Context: LLVM IR(Intermediate Representation) 객체들을 생성하고 관리하는 데 필요한 핵심 객체
```BasicBlock *entry = BasicBlock::Create(Context, "entry", addFun);```

### How to Create Constant
- constant argument를 삽입하기 위해 constant object를 만든다. \ 
    - value* 등에 삽입될 수 있다.
- 10 -> ConstantInt Object이다. \
- 계층 구조: Constant 객체는 단순히 하나의 클래스가 아니라, 정수, 부동 소수점, null 포인터 등 다양한 종류의 상수를 나타내는 여러 하위 클래스(ConstantInt, ConstantFP, ConstantPointerNull 등)를 가지는 계층 구조
    - Constant - ConstantData - 여러 ConstantClass
**EX)**
- ConstantInt class
    - ```Constant* one = ConstantInt::get(Type::getInt32Ty((Context), 1))```
- ConstantFP class
    - ```Constant* one = ConstantFP::get(Type::getFloatTy((Context), 0.0))```

### LLVM IRBuilder
- Help to create instructions and insert them into a basic block
* **필요한 헤더**: `#include "llvm/IR/IRBuilder.h"`
* **`IRBuilder`의 간단한 선언(Simple Declaration)**:
    * `IRBuilder<> Builder(I)`:
        * 주어진 `Instruction* I` 앞에 명령어를 삽입하도록 Builder를 초기화합니다.
    * `IRBuilder<> Builder(BB)`:
        * 주어진 `BasicBlock* BB`의 끝에 명령어를 삽입하도록 Builder를 초기화합니다. (가장 일반적인 사용법 중 하나입니다.)

* **메모리 명령어 (Memory Instructions)**
    * `LoadInst* CreateLoad (Type *Ty, Value *Ptr, const Twine &Name="")`
        * 메모리에서 값을 로드하는 `load` 명령어
        * `*Ty`: 로드할 값의 타입.
        * `*Ptr`: 로드할 메모리 주소를 가리키는 포인터 `Value`.
    * `StoreInst* CreateStore (Value *Val, Value *Ptr, bool isVolatile=false)`
        * 값을 메모리에 저장하는 `store` 명령어
        * `*Val`: 저장할 `Value`.
        * `*Ptr`: 값을 저장할 메모리 주소를 가리키는 포인터 `Value`.

* **산술 명령어 (Arithmetic Instructions)**
    * `Value* CreateAdd (Value *LHS, Value *RHS, const Twine &Name="", bool HasNUW=false, bool HasNSW=false)`
    * `Value* CreateSub (Value *LHS, Value *RHS, const Twine &Name="", bool HasNUW=false, bool HasNSW=false)`
    * `Value* CreateMul (Value *LHS, Value *RHS, const Twine &Name="", bool HasNUW=false, bool HasNSW=false)`
    * **설명**:
        * `*LHS`, `*RHS`: 연산의 왼쪽/오른쪽 피연산자, 이들은 `Value*` 타입이므로 상수, 변수(로드된 값) 등이 될 수 있다.
        * `NUW` (No Unsigned Wrap): 부호 없는 오버플로우가 발생하지 않음을 보장. (최적화 힌트)
        * `NSW` (No Signed Wrap): 부호 있는 오버플로우가 발생하지 않음을 보장. (최적화 힌트)

**기타 명령어 (Other Instructions)**
    * `CallInst* CreateCall (Value *Callee, ArrayRef<Value *> Args=None, const Twine &Name="", MDNode *FPMathTag=nullptr)`
        * 함수 호출 (`call`) 명령어를 생성.
        * `*Callee`: 호출할 함수(`Function*` 또는 `FunctionCallee`)를 나타내는 `Value*`.
        * `Args`: 호출할 함수에 전달할 인자들의 배열.
    * `ReturnInst* CreateRetVoid ()`
        * `void` 타입을 반환하는 `ret void` 명령어를 생성.
    * `ReturnInst* CreateRet (Value *V)`
        * 특정 `Value`(`V`)를 반환하는 `ret <val>` 명령어를 생성.

**사용 예시 (Example)**
  * 기본 블록에 함수 호출을 삽입
    ```cpp
    IRBuilder<> Builder(BB->getTerminator());
    CallInst *newCallInst = Builder.CreateCall(MarkBBEnd, args, "");
    ```
      * `IRBuilder<> Builder(BB->getTerminator());`
          * `BB->getTerminator()`: `BB`라는 `BasicBlock`의 마지막 명령어(종결자, Terminator)를 가져온다.
          * 이 `IRBuilder`는 `BB`의 종결자 명령어 바로 앞에 새로운 명령어를 삽입하도록 설정한다. (이는 `IRBuilder<>(Instruction* I)` 생성자 사용 예시)
      * `CallInst *newCallInst = Builder.CreateCall(MarkBBEnd, args, "");`
          * `Builder`를 사용하여 함수 호출(`call`) 명령어를 생성한다.
          * `MarkBBEnd`: 호출할 함수(`Callee`)를 나타내는 `Value*`입니다.
          * `args`: 함수에 전달할 인자들을 담은 배열.
          * `""`: 생성될 `call` 명령어의 이름.


## Each Step Command
1) Open “lib/InsertIncFunction/InsertIncFunction.cpp”
2) Modify the run function to
▪ Create a function named “inc”
▪ int inc (int n) { return n + 1; }
▪ Create a basic block for the Inc function
3) 4) Fill the Inc function with instructions Compile and test the pass
 a. compile <br>
    ```clang++ -c -fpic -fno-rtti -stdlib=libc++ `llvm-config --cppflags`  InsertIncFunction.cpp -o  InsertIncFunction.o --sysroot=`xcrun --show-sdk-path` -std=c++17```

    b.  Make a shared library with the LLVM passes <br>
    ~~~
        clang++ -shared LoopInfoPrinter.o -o LoopInfoPrinter.so \
        --sysroot=$(xcrun --show-sdk-path) \
        $(llvm-config --ldflags --libs) \
        -lc++abi -lunwind
    ~~~
   
    c. Run the LLVM Passes <br>
    * --load로 mac에서는 동작 안 함, --passes도 있어야해 (linux도 마찬가지)
        ```opt --load-pass-plugin LoopInfoPrinter.so --passes=loop-info-printer ../../exercise2/mm.bc -o ../../exercise6/mm.opt.bc```
    
  
## results
mm.opt.ll
마지막 39 basic block아래에 inc 생김
```
39:                                               ; preds = %36
  call void @llvm.lifetime.end.p0(i64 1048576, ptr nonnull %3) #2
  call void @llvm.lifetime.end.p0(i64 1048576, ptr nonnull %2) #2
  call void @llvm.lifetime.end.p0(i64 1048576, ptr nonnull %1) #2
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

define i32 @inc(i32 %n) {
entry:
  %result = add i32 %n, 1
  ret i32 %result
}
```


