## LLVM 15 API CHANGED!
Before this I used llvm14 and everything looked fine
BUT.
when I use clang command dump LLVM-IR.It changed again

if this code:
```c
extern "C" int foo(int array[]){
    return array[2];
}

int main(){
    int a[3] = {100,200,300};
    foo(a);
    return 0;
}
```
LLVM-IR:
```llvm
@__const.main.a = private unnamed_addr constant [3 x i32] [i32 100, i32 200, i32 300], align 4

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local i32 @foo(ptr noundef %0) #0 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = getelementptr inbounds i32, ptr %3, i64 2   ; <- indices changed to one arg
  %5 = load i32, ptr %4, align 4
  ret i32 %5
}

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #1 {
  %1 = alloca i32, align 4
  %2 = alloca [3 x i32], align 4
  store i32 0, ptr %1, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %2, ptr align 4 @__const.main.a, i64 12, i1 false)
  %3 = getelementptr inbounds [3 x i32], ptr %2, i64 0, i64 0
  %4 = call i32 @foo(ptr noundef %3)
  ret i32 0
}
```

The parameter type of foo has changed! it is not:``` <3 x i32> * %array```

## AND LLVM 15 this APIs
1. always asserted :
```c
base->getType()->getScalarType()->getNonOpaquePointerElementType()->dump();
```
2. JIT function getAddress() 
```c
auto addSymbol = ExitOnErr(J->lookup("foo"));
auto fooFunc= (int (*)(int*))addSymbol.getAddress();
```
llvm15 will be:
```c
auto addSymbol = ExitOnErr(J->lookup("foo"));
auto fooFunc = addSymbol.toPtr<int(int*)>();
```


## final Generated-IR
```llvm
; ModuleID = 'test foo module'
source_filename = "test foo module"

define i32 @foo(ptr %array) {
entry:
  %ret_alloc = alloca i32, align 4
  store i32 0, ptr %ret_alloc, align 4
  br label %loop

loop:                                             ; preds = %loop_body, %entry
  %phi_i = phi i32 [ 0, %entry ], [ %next_val, %loop_body ]
  %cmp = icmp slt i32 %phi_i, 3
  br i1 %cmp, label %loop_body, label %after_loop

loop_body:                                        ; preds = %loop
  %next_val = add i32 %phi_i, 1
  %gep_addr = getelementptr i32, ptr %array, i32 %phi_i
  %load_grep = load i32, ptr %gep_addr, align 4
  %ret_temp_load_value = load i32, ptr %ret_alloc, align 4
  %added = add i32 %ret_temp_load_value, %load_grep
  store i32 %added, ptr %ret_alloc, align 4
  br label %loop

after_loop:                                       ; preds = %loop
  %ret = load i32, ptr %ret_alloc, align 4
  ret i32 %ret
}
```