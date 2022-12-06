
```c
int foo(int a){
    int ret = 0;
    if(a <0)
        ret = a+1;
    else
        ret = a+2;
    return ret;
}
```

```llvm
; ModuleID = 'my compiler'
source_filename = "my compiler"

define i32 @foo(i32 %val) {
entry:
  %ret = alloca i32, align 4
  store i32 %val, i32* %ret, align 4
  %cmp_tmp = icmp slt i32 %val, 1
  br i1 %cmp_tmp, label %then_bb, label %else_bb

then_bb:                                          ; preds = %entry
  %add1_tmp = add i32 %val, 1
  store i32 %add1_tmp, i32* %ret, align 4
  %ret1 = load i32, i32* %ret, align 4
  ret i32 %ret1

else_bb:                                          ; preds = %entry
  %add2_tmp = add i32 %val, 2
  store i32 %add2_tmp, i32* %ret, align 4
  %ret2 = load i32, i32* %ret, align 4
  ret i32 %ret2
}

```
```
foo(-1)= 0
foo(1)= 3
```