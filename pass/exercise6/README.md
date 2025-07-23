# Loop Analysis 

## Notion
### Interact with Other Passes
- pass들은 서로 의존성이 있다.
- ```opt --debug-pass-manager```은 의존관계 보여주는 명령어, 패스 매니저가 어떤 패스들을 어떤 순서로 실행하고, 어떤 분석 패스들의 결과를 사용하는지 알 수 있음 \

#### 예제 1) `opt --debug-pass-manager ../exercise1/test.bc -o test.bc.opt`

실행 결과:

```
Running pass: VerifierPass on [module]
Running analysis: VerifierAnalysis on [module]
Running pass: BitcodeWriterPass on [module]
```

분석: 
  * **`Running pass: VerifierPass on [module]`**
      * 이 패스는 LLVM IR(Intermediate Representation)의 유효성(correctness)을 검증
      * 전체 모듈(`[module]`) 레벨에서 실행
      * 주로 IR 파일을 처리하기 시작할 때와 최종 결과를 출력하기 전에 IR의 상태가 유효한지 확인
  * **`Running analysis: VerifierAnalysis on [module]`**
      * `VerifierPass`가 작동하기 위해 필요한 분석 정보를 제공하는 `VerifierAnalysis`라는 분석 패스가 실행
  * **`Running pass: BitcodeWriterPass on [module]`**
      * 현재 메모리에 있는 LLVM IR 모듈을 바이너리 비트코드 형식(`*.bc` 파일)으로 디스크에 쓰는 역할
      * `opt` 명령어의 `-o` 플래그(`-o test.bc.opt`)에 따라 최종 IR을 파일에 기록하기 위해 실행

결론: \
이 명령어는 특정 최적화 패스를 명시적으로 지정하지 않았기 때문에, 입력된 비트코드의 유효성을 검증하고(VerifierPass), 그 결과를 단순히 새로운 파일로 복사(BitcodeWriterPass)하는 기본 동작만 수행한다. 즉, IR에 어떠한 변환이나 최적화도 적용되지 않은 상태이다.


#### 예제 2) `opt --debug-pass-manager --passes=reg2mem ../exercise1/test.bc -o test.reg2mem.bc.opt`

실행 결과:

```
Running analysis: InnerAnalysisManagerProxy<FunctionAnalysisManager, Module> on [module]
Running pass: RegToMemPass on add (2 instructions)
Running analysis: DominatorTreeAnalysis on add
Running analysis: LoopAnalysis on add
Running pass: RegToMemPass on main (2 instructions)
Running analysis: DominatorTreeAnalysis on main
Running analysis: LoopAnalysis on main
Running pass: VerifierPass on [module]
Running analysis: VerifierAnalysis on [module]
Running pass: BitcodeWriterPass on [module]
```
*분석:

  * **`Running analysis: InnerAnalysisManagerProxy<FunctionAnalysisManager, Module> on [module]`**
      * LLVM의 새로운 패스 매니저에서 함수별 분석을 전체 모듈 레벨에서 관리하기 위한 내부 프록시 분석이 실행
  * **`Running pass: RegToMemPass on add (2 instructions)`**
      * `RegToMemPass`는 LLVM IR에서 레지스터 할당된 변수(SSA 레지스터)를 메모리 할당된 변수(스택의 메모리)로 변환하는 패스
      * `add` 함수에 대해 실행되었으며, 이 함수에는 2개의 명령어가 포함
  * **`Running analysis: DominatorTreeAnalysis on add`**
      * 함수의 CFG에서 Dominator Tree를 계산하는 분석 패스
      * `RegToMemPass`가 작업을 수행하기 위해 이 Dominator Tree 정보에 의존했을 수 있다.
  * **`Running analysis: LoopAnalysis on add`**
      * 함수의 제어 흐름 그래프에서 루프(Loop) 구조를 식별하는 분석 패스.
      * `RegToMemPass`가 `LoopAnalysis`에도 의존성을 가질 수 있거나, 패스 매니저가 특정 패스 조합에 대해 일반적으로 필요한 분석들을 미리 실행한 경우일 수 있다.
**결론:**
이 로그는 `opt`가 `--passes=reg2mem` 옵션에 따라 `RegToMemPass`를 실행했으며, 이 `RegToMemPass`의 실행을 위해 `DominatorTreeAnalysis` 및 `LoopAnalysis`와 같은 필요한 분석 패스들이 자동으로 실행되었다. LLVM의 패스 매니저는 이러한 의존성을 자동으로 관리하여 개발자가 필요한 패스만 지정하더라도 관련된 분석 패스들을 자동으로 실행한다.

### 패스의 분석 결과 가져오기
header포함 및 FunctionAnalysisManager의 getResult 이용
~~~
#include "llvm/Analysis/LoopInfo.h"
llvm::LoopInfo &LI = FAM.getResult<llvm::LoopAnalysis>(F);
~~~
LoopInfo는 LLVM에서 루프 관련 정보를 분석하고 제공하는 분석 패스 (루프 최적화에 필요한 정보 얻을 수 있음) \
LoopInfo는 직접적으로 루프를 최적화하는 "변형(Transform) 패스"가 아니라, 루프에 대한 상세한 정보를 제공 \
<br>
이 LoopInfo를 Include하는 것은 다른 패스(주로 최적화 패스)가 루프 정보를 얻어 자신의 작업에 활용하기 위함이 많음.
예를 들어, 루프 최적화 패스는 LoopInfo를 통해 루프의 구조를 파악하고, 이를 바탕으로 최적화 결정을 내리게 된다.


## Each Step Command
1) Open “lib/LoopInfoPrinter/LoopInfoPrinter.cpp”
    - https://stackoverflow.com/questions/26281823/llvm-how-to-get-the-label-of-basic-blocks 
        - printAsOperand()
    
2) Modify the run function to \
    ▪ Get LoopInfo from LoopAnalysis \
    ▪ Print the information about loops \
    - The number of loops, the depth of a loop, …
3) Compile and test the pass
 a. compile
    - mac
    ```clang++ -c -fpic -fno-rtti -stdlib=libc++ `llvm-config --cppflags` LoopInfoPrinter.cpp -o LoopInfoPrinter.o --sysroot=`xcrun --show-sdk-path` -std=c++17```
    - linux
    clang++ -c -fpic -fno-rtti LoopInfoPrinter.cpp -o LoopInfoPrinter.o -stdlib=libc++ `llvm-config --cppflags`


    b.  Make a shared library with the LLVM passes
        - mac
        ~~~
        clang++ -shared LoopInfoPrinter.o -o LoopInfoPrinter.so \
            --sysroot=$(xcrun --show-sdk-path) \
            $(llvm-config --ldflags --libs) \
            -lc++abi -lunwind
        ~~~
        - linux
        ~~~
            clang++ -shared LoopInfoPrinter.o -o LoopInfoPrinter.so
        ~~~
    c. Run the LLVM Passes
        ```opt --load-pass-plugin LoopInfoPrinter.so --passes=loop-info-printer ../../exercise2/mm.bc -o ../../exercise6/mm.opt.bc```
    - message result
  
## results
<h4 style="color: red;"> 
    분석 필요
</h4>

~~~
opt --load-pass-plugin LoopInfoPrinter.so --passes=loop-info-printer mm.bc -o mm.opt.bc
[Plugin] Registering LoopInfoPrinter Pass
[LoopInfoPrinter] Pass Entry
BB name: 
  %1 = alloca [512 x [512 x i32]], align 4
  %2 = alloca [512 x [512 x i32]], align 4
  %3 = alloca [512 x [512 x i32]], align 4
  call void @llvm.lifetime.start.p0(i64 1048576, ptr nonnull %1) #2
  call void @llvm.lifetime.start.p0(i64 1048576, ptr nonnull %2) #2
  call void @llvm.lifetime.start.p0(i64 1048576, ptr nonnull %3) #2
  br label %4
, # of Loop: 0
BB name: 
4:                                                ; preds = %0, %14
  %5 = phi i64 [ 0, %0 ], [ %15, %14 ]
  %6 = trunc nuw nsw i64 %5 to i32
  br label %7
, # of Loop: 1
BB name: 
7:                                                ; preds = %4, %7
  %8 = phi i64 [ 0, %4 ], [ %12, %7 ]
  %9 = getelementptr inbounds nuw [512 x [512 x i32]], ptr %1, i64 0, i64 %5, i64 %8
  store i32 %6, ptr %9, align 4, !tbaa !6
  %10 = getelementptr inbounds nuw [512 x [512 x i32]], ptr %2, i64 0, i64 %5, i64 %8
  %11 = trunc nuw nsw i64 %8 to i32
  store i32 %11, ptr %10, align 4, !tbaa !6
  %12 = add nuw nsw i64 %8, 1
  %13 = icmp eq i64 %12, 512
  br i1 %13, label %14, label %7, !llvm.loop !10
, # of Loop: 2
BB name: 
14:                                               ; preds = %7
  %15 = add nuw nsw i64 %5, 1
  %16 = icmp eq i64 %15, 512
  br i1 %16, label %17, label %4, !llvm.loop !13
, # of Loop: 1
BB name: 
17:                                               ; preds = %14, %36
  %18 = phi i64 [ %37, %36 ], [ 0, %14 ]
  br label %19
, # of Loop: 1
BB name: 
19:                                               ; preds = %17, %33
  %20 = phi i64 [ 0, %17 ], [ %34, %33 ]
  %21 = getelementptr inbounds nuw [512 x [512 x i32]], ptr %1, i64 0, i64 %18, i64 %20
  %22 = load i32, ptr %21, align 4, !tbaa !6
  br label %23
, # of Loop: 2
BB name: 
23:                                               ; preds = %19, %23
  %24 = phi i64 [ 0, %19 ], [ %31, %23 ]
  %25 = getelementptr inbounds nuw [512 x [512 x i32]], ptr %2, i64 0, i64 %20, i64 %24
  %26 = load i32, ptr %25, align 4, !tbaa !6
  %27 = mul nsw i32 %26, %22
  %28 = getelementptr inbounds nuw [512 x [512 x i32]], ptr %3, i64 0, i64 %18, i64 %24
  %29 = load i32, ptr %28, align 4, !tbaa !6
  %30 = add nsw i32 %29, %27
  store i32 %30, ptr %28, align 4, !tbaa !6
  %31 = add nuw nsw i64 %24, 1
  %32 = icmp eq i64 %31, 512
  br i1 %32, label %33, label %23, !llvm.loop !14
, # of Loop: 3
BB name: 
33:                                               ; preds = %23
  %34 = add nuw nsw i64 %20, 1
  %35 = icmp eq i64 %34, 512
  br i1 %35, label %36, label %19, !llvm.loop !15
, # of Loop: 2
BB name: 
36:                                               ; preds = %33
  %37 = add nuw nsw i64 %18, 1
  %38 = icmp eq i64 %37, 512
  br i1 %38, label %39, label %17, !llvm.loop !16
, # of Loop: 1
BB name: 
39:                                               ; preds = %36
  call void @llvm.lifetime.end.p0(i64 1048576, ptr nonnull %3) #2
  call void @llvm.lifetime.end.p0(i64 1048576, ptr nonnull %2) #2
  call void @llvm.lifetime.end.p0(i64 1048576, ptr nonnull %1) #2
  ret i32 0
, # of Loop: 0
[LoopInfoPrinter] 0xaaaadf731cc8 <unnamed loop> 1
[LoopInfoPrinter] 0xaaaadf731bb8 <unnamed loop> 1
~~~
## etc.
~~~
LoopBase -> Loop (LoopT)
LoopInfoBase -> LoopInfo(LoopInfoT)
vector<LoopT> -> LoopInfo
~~~
- for (Loop : LoopInfo)가 가능한 이유 코드, iter구현
~~~
  typedef typename std::vector<LoopT *>::const_iterator iterator;
  typedef
      typename std::vector<LoopT *>::const_reverse_iterator reverse_iterator;
  iterator begin() const { return getSubLoops().begin(); }
  iterator end() const { return getSubLoops().end(); }
  reverse_iterator rbegin() const { return getSubLoops().rbegin(); }
  reverse_iterator rend() const { return getSubLoops().rend(); }
~~~

- 출력 결과
~~~
 opt --load-pass-plugin LoopInfoPrinter.so --passes=loop-info-printer mm.bc -o mm.opt.bc
[Plugin] Registering LoopInfoPrinter Pass
[LoopInfoPrinter] Pass Entry
BB name: label %0, # of Loop: 0
BB name: label %4, # of Loop: 1
BB name: label %7, # of Loop: 2
BB name: label %14, # of Loop: 1
BB name: label %17, # of Loop: 1
BB name: label %19, # of Loop: 2
BB name: label %23, # of Loop: 3
BB name: label %33, # of Loop: 2
BB name: label %36, # of Loop: 1
BB name: label %39, # of Loop: 0
[LoopInfoPrinter] 0xaaaad255bcc8 <unnamed loop> 1
  - Number of blocks: 5
  - Latch block: 
36:                                               ; preds = %33
  %37 = add nuw nsw i64 %18, 1
  %38 = icmp eq i64 %37, 512
  br i1 %38, label %39, label %17, !llvm.loop !16

  - Exit block: 
39:                                               ; preds = %36
  call void @llvm.lifetime.end.p0(i64 1048576, ptr nonnull %3) #2
  call void @llvm.lifetime.end.p0(i64 1048576, ptr nonnull %2) #2
  call void @llvm.lifetime.end.p0(i64 1048576, ptr nonnull %1) #2
  ret i32 0

[LoopInfoPrinter] 0xaaaad255bbb8 <unnamed loop> 1
  - Number of blocks: 3
  - Latch block: 
14:                                               ; preds = %7
  %15 = add nuw nsw i64 %5, 1
  %16 = icmp eq i64 %15, 512
  br i1 %16, label %17, label %4, !llvm.loop !13

  - Exit block: 
17:                                               ; preds = %14, %36
  %18 = phi i64 [ %37, %36 ], [ 0, %14 ]
  br label %19
~~~

