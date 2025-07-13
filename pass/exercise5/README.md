#  (Static) Call Count 

## Notion
- A super class type pointer can point to a subclass type instance

### LLVM IR Classes
- LLVM IR 클래스들의 **"Is-A" 관계(상속 계층 구조)**
- "모든 객체는 Value이다"는 LLVM의 근본적인 개념
- LLVM IR의 거의 모든 중요한 엔티티(명령어, 기본 블록, 인자, 전역 변수, 함수, 상수)는 궁극적으로 llvm::Value 클래스에서 파생됨
- Value (기본 클래스): 많은 IR 엔티티의 뿌리입니다.
    - Value에서 직접 상속받는 클래스:
        - Argument: 함수 매개변수를 나타냅니다.
        - Constant: 상수 값(예: 정수 리터럴, 부동 소수점 리터럴)을 나타냅니다.
        - BasicBlock: 단일 진입점과 단일 종료점을 가진 명령어 시퀀스를 나타냅니다.
        - Instruction: IR에서 단일 연산을 나타냅니다.

### Dynamic Type Casting
- How can we distinguish the type of instructions?
    - 동적 타입 캐스팅: \
        BasicBlock 내의 Instruction 객체를 반복할 때, 보통 Instruction& 참조를 얻게 된다. 그러나 해당 Instruction이 구체적으로 LoadInst, CallInst, AllocaInst 등인지 알고 싶을 수 있다. 이때 동적 타입 캐스팅(C++의 dynamic_cast보다 타입-세이프하고 효율적인 LLVM의 사용자 지정 유틸리티인 isa<T>, cast<T>, dyn_cast<T> 사용)이 필수적이다. 

    - Special operators that support polymorphism
        - ```isa<Type>```
            - 포인터가 가리키는 인스턴스의 타입이 특정 Type인지 확인하는 데 사용
            - 즉, "이 객체가 T 타입인가?"를 묻는 것
            - Input: 포인터 
            - Output: 포인터가 "Type"의 인스턴스를 가리키면 true를 반환 \
            ex)
                ~~~
                    if(isa<CallInst>(&I)) {
                        // I가 CallInst(함수 호출 명령어)이면 이 블록이 실행됨
                    }
                ~~~
            -  이 검사는 런타임에 이루어짐

        - ```dyn_cast<Type>```
            - 포인터를 서브클래스 타입으로 안전하게 캐스팅하는 데 사용
            - "이 객체를 T 타입으로 캐스팅할 수 있는가? 있다면 캐스팅된 포인터를 반환
            - Input: 포인터 
            - Output:
                - 타입 캐스팅이 유효하면 Type의 포인터를 반환
                - 유효하지 않으면 nullptr \
            ex)
                ~~~
                CallInst* CI = dyn_cast<CallInst>(&I);
                // CI가 nullptr이 아니면, I는 CallInst였고, CI는 이제 CallInst*로 안전하게 사용 가능

                if (CallInst* CI = dyn_cast<CallInst>(&I)) {
                    // 이 구문은 위 두 줄의 코드를 한 줄로 줄인 것으로,
                    // 캐스팅이 성공하면 CI 변수가 유효한 CallInst* 포인터가 되고 if 블록이 실행됨
                }
                ~~~


## Each Step Command
1) Open “lib/CallCount/CallCount.cpp”
2) Modify the run function to
    ▪ Count the number of call instructions in a function
    ▪ Print the function name and the number of call instructions
3) Compile and test the pass
 a. compile
    ```clang++ -c -fpic -fno-rtti -stdlib=libc++ `llvm-config --cppflags` CallCount.cpp -o CallCount.o --sysroot=`xcrun --show-sdk-path -std=c++17```

    - error
        ~~~
        error: no template named 'tuple_element_t' in namespace 'std'; did you mean 'tuple_element'?
        error: no template named 'is_enum_v' in namespace 'std'; did you mean 'is_enum'?
        error: no template named 'enable_if_t' in namespace 'std'
        error: no template named 'aligned_union_t' in namespace 'std'
        error: no template named 'is_unsigned_v' in namespace 'std' 
        ~~~
        이러한 에러들은 모두 C++14/17에서 도입된 _t 또는 _v 접미사가 붙은 타입 트레잇 별칭 템플릿(alias template)이나 변수 템플릿(variable template)을 C++11 컴파일러가 인식하지 못하기 때문에 발생 

        컴파일 명령어의 -std=c++11을 LLVM이 사용하는 더 높은 C++ 표준(일반적으로 -std=c++17 이상)으로 변경

    b.  Make a shared library with the LLVM passes
        ~~~
        clang++ -shared CallCount.o -o CallCount.so \
            --sysroot=$(xcrun --show-sdk-path) \
            $(llvm-config --ldflags --libs) \
            -lc++abi -lunwind
        ~~~
    c. Run the LLVM Passes
        ```opt --load-pass-plugin CallCount.so --passes=call-count ../../exercise1/test.bc -o ../../exercise5/test.opt.bc```
    - message result
        ~~~
        [Plugin] Registering CallCount Pass
        [CallCount] 
        add [# of Call Instructions] 0
        [CallCount] 
        main [# of Call Instructions] 1
        ~~~
## results
~~~
define i32 @add(i32 noundef %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = add nsw i32 %1, %0  -> Instruction 1: add (덧셈)
  ret i32 %3 -> Instruction 2: ret (반환)
}
~~~
=> 따라서 Call instruction 0개

~~~
define noundef i32 @main() local_unnamed_addr #1 {
  %1 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, i32 noundef 5) ; -> Instruction 1: call (함수 호출)
  ret i32 0 -> Instruction 2: ret (반환)
}
~~~
=> 따라서 Call instruction 1개 


