# First Compilation
- LLVM IR 생성을 위한 명령어
- test.c 파일이 있다고 가정

## Each Step Command
1. frontend compilation \
    ```clang -O1 -c -emit-llvm (--sysroot=`xcrun --show-sdk-path`) test.c -o test.bc```
    - 아직 특정 운영체제나 CPU 아키텍처의 기계어 코드를 생성하지 않음. 
    - 최적화 전 LLVM IR 생성. \

    |option|explanation|
    |---|---|
    |-0숫자|최적화 레벨|
    |-c| 링크 등 없이 컴파일만|
    |-emit-llvm|LLVM IR을 출력 (LLVM Intermediate Representation). 이때 -c와 같이 쓰면 바이너리 형태의 bitcode (.bc) 생성|
    |-o|출력 파일 이름을 test.bc로 지정|
    |--sysroot | 컴파일러에게 "루트 디렉토리를 여기로 가정하고 헤더 파일과 라이브러리를 찾아라", 연결 안 될 때만 넣으면 됨|


2. 최적화 - loop unroll\
    ```opt -passes=loop-unroll test.bc -o test.opt.bc```
3. Human-readable IR 파일(*.ll) 만들기\
    ```llvm-dis test.bc``` \
    ```llvm-dis test.opt.bc```
4. Backend compilation\
    ```clang test.opt.bc -o test.exe (--sysroot=`xcrun --show-sdk-path`)```
    -  컴파일러는 대상 운영체제와 CPU 아키텍처에 맞는 특정 규칙 사용

## results
1. test.ll, test.opt.ll
loop가 없으니까 동일
```
define dso_local i32 @add(i32 noundef %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = add nsw i32 %1, %0
  ret i32 %3
}

; Function Attrs: nofree nounwind uwtable
define dso_local noundef i32 @main() local_unnamed_addr #1 {
  %1 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, i32 noundef 5)
  ret i32 0
}

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #2
```

## Further
- -O0 결과
```
@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @add(i32 noundef %0, i32 noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, ptr %3, align 4
  store i32 %1, ptr %4, align 4
  %5 = load i32, ptr %3, align 4
  %6 = load i32, ptr %4, align 4
  %7 = add nsw i32 %5, %6
  ret i32 %7
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  %3 = call i32 @add(i32 noundef 2, i32 noundef 3)
  store i32 %3, ptr %2, align 4
  %4 = load i32, ptr %2, align 4
  %5 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %4)
  ret i32 0
}

declare i32 @printf(ptr noundef, ...) #1
```
- -O3 결과
```
define i32 @add(i32 noundef %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = add nsw i32 %1, %0
  ret i32 %3
}

; Function Attrs: nofree nounwind ssp uwtable(sync)
define noundef i32 @main() local_unnamed_addr #1 {
  %1 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, i32 noundef 5)
  ret i32 0
}

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #2
```
