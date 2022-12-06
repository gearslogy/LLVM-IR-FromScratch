IMP:
```c
int foo(int a){
if(a < 0)
    return a+1;
else
    return a+2;
}
```


```
; ModuleID = 'my compiler'
source_filename = "my compiler"

define i32 @foo(i32 %a) {
entry:
  %cmp_tmp = icmp slt i32 %a, 1
  br i1 %cmp_tmp, label %then_bb, label %else_bb

then_bb:                                          ; preds = %entry
  %add1 = add i32 %a, 1
  ret i32 %add1

else_bb:                                          ; preds = %entry
  %add2 = add i32 %a, 2
  ret i32 %add2
}
```
```
foo(-1)= 0
foo(1)= 3
```