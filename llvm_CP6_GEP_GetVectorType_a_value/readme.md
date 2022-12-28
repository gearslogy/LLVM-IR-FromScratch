```llvm
; ModuleID = 'test foo module'
source_filename = "test foo module"

define i32 @foo(<3 x i32>* %array) {
entry:
  %a_0 = getelementptr <3 x i32>, <3 x i32>* %array, i32 0, i32 2
  %ret = load i32, i32* %a_0, align 4
  ret i32 %ret
}
```

the key is :
```llvm
%a_0 = getelementptr <3 x i32>, <3 x i32>* %array, i32 0, i32 2
```

c:
```c
auto *gep_0 = builder.CreateGEP(base->getType()->getScalarType()->getNonOpaquePointerElementType(), base,  indexList, "a_0" );
```