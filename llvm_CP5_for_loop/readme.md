```C
int foo(int a, int end_condition){
    int ret = a;
    for(int i=1; i< end_condition; i++){
        ret += 1;
    }
    return ret;
}
```

```llvm
; ModuleID = 'test foo module'
source_filename = "test foo module"

define i32 @foo(i32 %a, i32 %end_condition) {
entry:
  %ret = alloca i32, align 4
  store i32 %a, i32* %ret, align 4
  br label %loop_body

loop_body:                                        ; preds = %loop_body, %entry
  %i = phi i32 [ 1, %entry ], [ %next_val, %loop_body ]
  %next_val = add i32 %i, 1
  %end_cmp = icmp ult i32 %i, %end_condition
  %load_ret = load i32, i32* %ret, align 4
  %add_tmp = add i32 %load_ret, 1
  store i32 %add_tmp, i32* %ret, align 4
  br i1 %end_cmp, label %loop_body, label %after_loop

after_loop:                                       ; preds = %loop_body
  %final_load_ret = load i32, i32* %ret, align 4
  ret i32 %final_load_ret
}
```