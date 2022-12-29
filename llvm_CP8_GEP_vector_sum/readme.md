```C
int foo(int *array){
    int ret=0;
    for(int i=0;i < 3;i++){
        ret = ret + array[i];
    }
    return ret;
}
```

```llvm
; ModuleID = 'test foo module'
source_filename = "test foo module"

define i32 @foo(<3 x i32>* %array) {
entry:
  %ret_alloc = alloca i32, align 4
  store i32 0, i32* %ret_alloc, align 4
  br label %loop

loop:                                             ; preds = %loop_body, %entry
  %phi_i = phi i32 [ 0, %entry ], [ %next_val, %loop_body ]
  %cmp = icmp slt i32 %phi_i, 3
  br i1 %cmp, label %loop_body, label %after_loop

loop_body:                                        ; preds = %loop
  %next_val = add i32 %phi_i, 1
  %gep_addr = getelementptr <3 x i32>, <3 x i32>* %array, i32 0, i32 %phi_i
  %gep_load = load i32, i32* %gep_addr, align 4
  %ret_temp_load_value = load i32, i32* %ret_alloc, align 4
  %added = add i32 %ret_temp_load_value, %gep_load
  store i32 %added, i32* %ret_alloc, align 4
  br label %loop

after_loop:                                       ; preds = %loop
  %ret = load i32, i32* %ret_alloc, align 4
  ret i32 %ret
}
```
