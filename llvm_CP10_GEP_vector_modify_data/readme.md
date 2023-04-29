```c++
void foo(int *array){
  for(int i=0;i<3;i++){
    array[i] = (i + 1) * 100;
  }
}

int main(){
  int array[3] = {0,0,0};
  foo(array); // create in llvm-ir, this is LLVM FUNCTION
  return 0;
}
```



IR:

```LLVM
; ModuleID = 'test foo module'
source_filename = "test foo module"

define void @foo(ptr %array) {
entry:
  br label %loop

loop:                                             ; preds = %loop_body, %entry
  %phi_i = phi i32 [ 0, %entry ], [ %next_val, %loop_body ]
  %cmp = icmp slt i32 %phi_i, 3
  br i1 %cmp, label %loop_body, label %after_loop

loop_body:                                        ; preds = %loop
  %next_val = add i32 %phi_i, 1
  %gep_addr = getelementptr i32, ptr %array, i32 %phi_i
  %load_grep = load i32, ptr %gep_addr, align 4
  %mul_val = mul i32 %next_val, 100
  store i32 %mul_val, ptr %gep_addr, align 4
  br label %loop

after_loop:                                       ; preds = %loop
  ret void
}
```

