#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LLVMContext.h"



llvm::Function *createAddFunc(llvm::IRBuilder<> &builder, std::vector<llvm::Type*> args, const std::string &FuncName, llvm::Module&m ){
    auto *funcType = llvm::FunctionType::get(builder.getInt32Ty(), std::move(args),  false);
    auto *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, FuncName, m);
    return func;
}

void setAddFuncArgsName(llvm::Function *func){
    func->arg_begin()->setName("A");
    (func->arg_begin()+1)->setName("B");
}
llvm::Value* createAddExpression(llvm::IRBuilder<> & builder, llvm::Function *func){
    llvm::Argument &argA = *func->arg_begin();
    llvm::Argument &argB = (* (func->arg_begin()+1));
    return builder.CreateAdd(&argA, &argB, "ret");
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
    std::vector<llvm::Type*> args = {builder.getInt32Ty(), builder.getInt32Ty()};

    auto *func = createAddFunc(builder, std::move(args), "foo", modRef);
    setAddFuncArgsName(func);
    auto *add_entry = createBlock(func, "add_entry");
    builder.SetInsertPoint(add_entry);
    auto *ret = createAddExpression(builder, func);
    builder.CreateRet(ret);
    mod->dump();


    // create execution engine
    auto J = ExitOnErr(llvm::orc::LLJITBuilder().create());
    auto threadSafeMod = llvm::orc::ThreadSafeModule(std::move(mod), std::move(context));
    J->addIRModule(std::move(threadSafeMod));

    // Look up the JIT'd function, cast it to a function pointer, then call it.
    auto addSymbol = ExitOnErr(J->lookup("foo"));
    auto addFunction= (int (*)(int,int))addSymbol.getAddress();
    int Result = addFunction(1,2);
    llvm::outs() << "add(1,2) = " << Result << "\n";
    return 0;
}
