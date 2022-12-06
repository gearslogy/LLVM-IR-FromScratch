#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LLVMContext.h"



llvm::GlobalVariable *createGlob(llvm::IRBuilder<> &builder, const std::string &name, llvm::Module &m){
    m.getOrInsertGlobal(name, builder.getInt32Ty());
    auto *glob = m.getNamedGlobal(name);
    //glob->setLinkage(llvm::GlobalVariable::CommonLinkage);
    glob->setInitializer(builder.getInt32(-1));  // set default value
    glob->setAlignment(llvm::MaybeAlign(4));
    return glob;
}


llvm::Function *createFooFunction(llvm::IRBuilder<> &builder, const std::string &FuncName, llvm::Module&m ){
    auto *funcType = llvm::FunctionType::get(builder.getInt32Ty(),   false);
    auto *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, FuncName, m);
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

    auto *glob = createGlob(builder,"glob_var", modRef);
    auto *func = createFooFunction(builder,  "foo", modRef);
    auto *entry = createBlock(func, "entry");
    auto *thenBB = createBlock(func, "then");
    auto *elseBB = createBlock(func, "else");

    // if
    builder.SetInsertPoint(entry);
    auto *glob_var = glob->getInitializer();
    auto *cond_var = builder.CreateICmpSLT(glob_var, builder.getInt32(5), "cmptmp");
    builder.CreateCondBr(cond_var, thenBB, elseBB);
    // then
    builder.SetInsertPoint(thenBB);
    builder.CreateRet(builder.getInt32(1));
    // else
    builder.SetInsertPoint(elseBB);
    builder.CreateRet(builder.getInt32(0));

    mod->dump();



    // create execution engine
    auto J = ExitOnErr(llvm::orc::LLJITBuilder().create());
    auto threadSafeMod = llvm::orc::ThreadSafeModule(std::move(mod), std::move(context));
    J->addIRModule(std::move(threadSafeMod));

    // Look up the JIT'd function, cast it to a function pointer, then call it.
    auto addSymbol = ExitOnErr(J->lookup("foo"));
    auto fooFunc= (int (*)())addSymbol.getAddress();
    int Result = fooFunc();
    llvm::outs() << "foo= " << Result << "\n";

    return 0;
}
