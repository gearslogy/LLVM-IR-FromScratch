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

auto *createFunction(llvm::IRBuilder<> &builder, const std::string &funcName, llvm::Module &m){
    llvm::Function *ret{nullptr};
    auto *array_type= builder.getInt32Ty();
    auto *vec_type = llvm::VectorType::get(array_type, 3,false);
    auto *ptr = vec_type->getPointerTo(0);

    auto *funcType = llvm::FunctionType::get(builder.getInt32Ty(), ptr, false);
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
    builder.SetInsertPoint(entry_block);

    llvm::Value *base = getArg(func, 0);
    base->getType()->dump();
    base->getType()->getScalarType()->dump();
    base->getType()->getScalarType()->getNonOpaquePointerElementType()->dump();

    std::vector<llvm::Value*> indices(2);
    indices[0] = builder.getInt32(0);
    indices[1] = builder.getInt32(0);

    llvm::Value* indexList[2] = {builder.getInt32(0), builder.getInt32(2)};

    //builder.CreateAlloca(builder.getInt32Ty(), builder.getInt32(5), "test_array");


    auto *gep_0 = builder.CreateGEP(base->getType()->getScalarType()->getNonOpaquePointerElementType(), base,  indexList, "a_0" );
    auto *ret = builder.CreateLoad(builder.getInt32Ty(), gep_0, "ret");
    builder.CreateRet(ret);

    mod->dump();


    // create execution engine
    auto J = ExitOnErr(llvm::orc::LLJITBuilder().create());
    auto threadSafeMod = llvm::orc::ThreadSafeModule(std::move(mod), std::move(context));
    J->addIRModule(std::move(threadSafeMod));


    int arr[] = {24, 12, 32};
    // Look up the JIT'd function, cast it to a function pointer, then call it.
    auto addSymbol = ExitOnErr(J->lookup("foo"));
    auto fooFunc= (int (*)(int*))addSymbol.getAddress();
    int Result1 = fooFunc(arr);

    std::cout << Result1 << std::endl;



    return 0;
}
