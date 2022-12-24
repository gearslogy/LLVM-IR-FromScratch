#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"



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

llvm::Function *createFunction(llvm::IRBuilder<> &builder, const std::string &funcName, llvm::Module &m){
    std::vector<llvm::Type*> args(2);
    std::fill(args.begin(), args.end(),builder.getInt32Ty());
    auto *funcType = llvm::FunctionType::get(builder.getInt32Ty(), args, false);
    auto *ret = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, m);
    setArgName(ret,"a",0);
    setArgName(ret,"end_condition",1);
    return ret;
}


/*

int foo(int a, int end_condition){
    int ret = a;
    for(int i=1; i< end_condition; i++){
        ret += 1;
    }
    return ret;
}

 */


int foo(int a, int end_condition){
    int ret = a;
    for(int i=1; i< end_condition; i++){
        ret += 1;
    }
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

    // create ret variable
    builder.SetInsertPoint(entry_block);
    auto *ret_alloc = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "ret");
    builder.CreateStore(getArg(func,0), ret_alloc);
    builder.CreateBr(loop_block);


    // loop block
    builder.SetInsertPoint(loop_block);
    auto *i_phi = builder.CreatePHI(builder.getInt32Ty(),2 , "i");
    i_phi->addIncoming(builder.getInt32(1), entry_block); // i = 1
    auto *end_cmp = builder.CreateICmpULT(i_phi, getArg(func, 1), "end_cmp");    // i < end_condition
    builder.CreateCondBr(end_cmp, loop_body_block, after_loop_block);


    // loop_body block
    builder.SetInsertPoint(loop_body_block);
    auto *next_val = builder.CreateAdd(i_phi, builder.getInt32(1), "next_val" );
    i_phi->addIncoming(next_val, loop_body_block);

    auto *load_ret = builder.CreateLoad(builder.getInt32Ty(), ret_alloc, "load_ret");
    auto *add_temp = builder.CreateAdd(load_ret, builder.getInt32(1), "add_tmp");
    builder.CreateStore(add_temp, ret_alloc);
    builder.CreateBr(loop_block);


    // after_loop block
    builder.SetInsertPoint(after_loop_block);
    builder.CreateRet(builder.CreateLoad(builder.getInt32Ty(), ret_alloc, "final_load_ret"));


    mod->dump();


    // create execution engine
    auto J = ExitOnErr(llvm::orc::LLJITBuilder().create());
    auto threadSafeMod = llvm::orc::ThreadSafeModule(std::move(mod), std::move(context));
    J->addIRModule(std::move(threadSafeMod));

    // Look up the JIT'd function, cast it to a function pointer, then call it.
    auto addSymbol = ExitOnErr(J->lookup("foo"));
    auto fooFunc= (int (*)(int,int))addSymbol.getAddress();
    int Result1 = fooFunc(1,10);
    llvm::outs() << "foo(1,10)= " << Result1 << "\n";
    std::cout << foo(1,10) << std::endl;

    return 0;
}

