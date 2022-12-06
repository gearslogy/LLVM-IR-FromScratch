IMP:
```c
int foo(int a){
if(a < 0)
    return a+1;
else
    return a+2;
}
```


```LLVM
; ModuleID = 'my compiler'
source_filename = "my compiler"

define i32 @foo(i32 %val) {
entry:
%cmp_tmp = icmp slt i32 %val, 1
br i1 %cmp_tmp, label %then_bb, label %else_bb

then_bb:                                          ; preds = %entry
%add1_tmp = add i32 %val, 1
br label %merge_bb

else_bb:                                          ; preds = %entry
%add2_tmp = add i32 %val, 2
br label %merge_bb

merge_bb:                                         ; preds = %else_bb, %then_bb
%ret = phi i32 [ %add1_tmp, %then_bb ], [ %add2_tmp, %else_bb ]
ret i32 %ret
}

```
```
foo(-1)= 0
foo(1)= 3
```