#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"



constexpr int array_size  = 3;

llvm::Value *getArg(llvm::Function *function, int idx){
    return function->getArg(idx);
}
void setArgName(llvm::Function *func, const std::string &name, int idx){
    auto *arg = func->getArg(idx);
    arg->setName(name);
}

llvm::BasicBlock *createBlock(llvm::Function *func, const std::string &blockName){
    return llvm::BasicBlock::Create(   func->getContext(),blockName, func);
}

auto *createFunction(llvm::IRBuilder<> &builder, const std::string &funcName, llvm::Module &m){
    llvm::Function *ret{nullptr};
    auto *array_type= builder.getInt32Ty();
    auto *vec_type = llvm::VectorType::get(array_type, array_size, false);
    auto *ptr = vec_type->getPointerTo(0);

    auto *funcType = llvm::FunctionType::get(builder.getVoidTy(), ptr, false);
    ret = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, m);
    setArgName(ret, "array", 0 );
    return ret;
}

int main(int argc, char **argv) {
    llvm::ExitOnError ExitOnErr;
    llvm::InitLLVM give_me_a_name(argc,argv);
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();


    auto context = std::make_unique<llvm::LLVMContext>();
    auto mod = std::make_unique<llvm::Module>("test foo module", *context);
    auto &contextRef = *context;
    auto &modRef = *mod;

    llvm::IRBuilder<> builder(contextRef);
    auto func = createFunction(builder, "foo", modRef);
    auto *entry_block = createBlock(func, "entry");
    auto *loop_block = createBlock(func, "loop");
    auto *loop_body_block = createBlock(func, "loop_body");
    auto *after_loop_block = createBlock(func, "after_loop");
    builder.SetInsertPoint(entry_block);
    builder.CreateBr(loop_block);

    // loop block
    builder.SetInsertPoint(loop_block);
    auto *phi_i = builder.CreatePHI(builder.getInt32Ty(), 2, "phi_i");
    phi_i->addIncoming(builder.getInt32(0), entry_block);
    auto *cmp = builder.CreateICmpSLT(phi_i, builder.getInt32(array_size ) , "cmp"); // i < 3
    builder.CreateCondBr(cmp, loop_body_block, after_loop_block);

    // loop_body
    builder.SetInsertPoint(loop_body_block);
    auto *next_value = builder.CreateAdd(phi_i, builder.getInt32(1), "next_val");
    phi_i->addIncoming(next_value, loop_body_block);


    auto *base = getArg(func, 0);
    auto *gep = builder.CreateGEP(builder.getInt32Ty(), base, phi_i, "gep_addr" );
    auto *load = builder.CreateLoad(builder.getInt32Ty(), gep, "load_grep");
    auto *opValue = builder.CreateMul(next_value, builder.getInt32(100), "mul_val"); //   (i+1) * 100;
    builder.CreateStore(opValue, gep);
    builder.CreateBr(loop_block);

    // after loop body
    builder.SetInsertPoint(after_loop_block);
    builder.CreateRetVoid();
    mod->dump();


    // create execution engine
    auto J = ExitOnErr(llvm::orc::LLJITBuilder().create());
    auto threadSafeMod = llvm::orc::ThreadSafeModule(std::move(mod), std::move(context));
    J->addIRModule(std::move(threadSafeMod));

    int arr[] = {1, 2, 3};
    // Look up the JIT'd function, cast it to a function pointer, then call it.
    auto addSymbol = ExitOnErr(J->lookup("foo"));
    auto fooFunc = addSymbol.toPtr<void(int*)>();
    fooFunc(arr);
    for(auto v : arr){
        std::cout << v << std::endl;
    }

    std::cin.get();

    return 0;
}
