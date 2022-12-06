#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LLVMContext.h"


llvm::Value *getArg(llvm::Function *func, int idx){
    auto &arg = *(func->arg_begin() + idx); // return llvm::Argument
    return &arg; // class Argument: public Value
}

llvm::Function *createFooFunction(llvm::IRBuilder<> &builder, const std::string &FuncName, llvm::Module&m ){
    auto *funcType = llvm::FunctionType::get(builder.getInt32Ty(), builder.getInt32Ty(),   false);
    auto *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, FuncName, m);
    auto arg0 = func->arg_begin();
    arg0->setName("a");
    return func;
}



llvm::BasicBlock *createBlock(llvm::Function *func, const std::string &blockName){
    return llvm::BasicBlock::Create(   func->getContext(),blockName, func);
}




int main(int argc, char ** argv){
    llvm::ExitOnError ExitOnErr;
    llvm::InitLLVM give_me_a_name(argc,argv);
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();


    auto context = std::make_unique<llvm::LLVMContext>();
    auto mod = std::make_unique<llvm::Module>("my compiler", *context);

    auto &contextRef = *context;
    auto &modRef = *mod;

    llvm::IRBuilder<> builder{contextRef};

    auto *func = createFooFunction(builder,  "foo", modRef);
    auto *entry_bb = createBlock(func, "entry");
    auto *then_bb = createBlock(func, "then_bb");
    auto *else_bb = createBlock(func, "else_bb");

    // if
    builder.SetInsertPoint(entry_bb);
    auto *cmp = builder.CreateICmpSLT(getArg(func,0), builder.getInt32(1), "cmp_tmp");
    builder.CreateCondBr(cmp, then_bb, else_bb);
    // add1
    builder.SetInsertPoint(then_bb);
    auto *add1 = builder.CreateAdd(getArg(func,0), builder.getInt32(1),"add1");
    builder.CreateRet(add1 );
    // add2
    builder.SetInsertPoint(else_bb);
    auto *add2 = builder.CreateAdd(getArg(func,0), builder.getInt32(2),"add2");
    builder.CreateRet(add2 );
    mod->dump();




    // create execution engine
    auto J = ExitOnErr(llvm::orc::LLJITBuilder().create());
    auto threadSafeMod = llvm::orc::ThreadSafeModule(std::move(mod), std::move(context));
    J->addIRModule(std::move(threadSafeMod));

    // Look up the JIT'd function, cast it to a function pointer, then call it.
    auto addSymbol = ExitOnErr(J->lookup("foo"));
    auto fooFunc= (int (*)(int))addSymbol.getAddress();
    int Result1 = fooFunc(-1);
    llvm::outs() << "foo(-1)= " << Result1 << "\n";
    int Result2 = fooFunc(1);
    llvm::outs() << "foo(1)= " << Result2 << "\n";


    return 0;
}
