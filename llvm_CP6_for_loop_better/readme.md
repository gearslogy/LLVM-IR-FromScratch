```
; ModuleID = 'test foo module'
source_filename = "test foo module"

define i32 @foo(i32 %a, i32 %end_condition) {
entry:
  %ret = alloca i32, align 4
  store i32 %a, i32* %ret, align 4
  br label %loop

loop:                                             ; preds = %loop_body, %entry
  %i = phi i32 [ 1, %entry ], [ %next_val, %loop_body ]
  %end_cmp = icmp ult i32 %i, %end_condition
  br i1 %end_cmp, label %loop_body, label %after_loop

loop_body:                                        ; preds = %loop
  %next_val = add i32 %i, 1
  %load_ret = load i32, i32* %ret, align 4
  %add_tmp = add i32 %load_ret, 1
  store i32 %add_tmp, i32* %ret, align 4
  br label %loop

after_loop:                                       ; preds = %loop
  %final_load_ret = load i32, i32* %ret, align 4
  ret i32 %final_load_ret
}
```