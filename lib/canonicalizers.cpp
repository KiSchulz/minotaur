#include "canonicalizers.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Constants.h"
#include <vector>
#include <algorithm>
#include <iostream>

using namespace minotaur::canonicalizer;

ChangeSet UnusedArgumentStep::canonicalize(llvm::Function &F) {
    std::vector<unsigned> removed;
    std::vector<llvm::Type*> argTypes;

    for (llvm::Argument &arg : F.args()) {
        if (arg.use_empty()) {
            removed.push_back(arg.getArgNo());
        } else {
            argTypes.push_back(arg.getType());
        }
    }

    if (removed.empty()) {
        return ChangeSet{};
    }

    llvm::Function* canonicalizedF = llvm::Function::Create(
        llvm::FunctionType::get(F.getReturnType(), argTypes, F.isVarArg()),
        F.getLinkage(),
        F.getName() + ".canonicalized",
        F.getParent()
    );

    llvm::ValueToValueMapTy VMap;
    llvm::Function::arg_iterator canonArgIt = canonicalizedF->arg_begin();
    for (llvm::Argument &arg : F.args()) {
        if (arg.use_empty()) {
            VMap[&arg] = llvm::Constant::getNullValue(arg.getType());
        } else {
            canonArgIt->setName(arg.getName());
            VMap[&arg] = &*canonArgIt++;
        }
    }

    llvm::SmallVector<llvm::ReturnInst*, 8> returns;
    llvm::CloneFunctionInto(canonicalizedF, &F, VMap,
        llvm::CloneFunctionChangeType::LocalChangesOnly, returns);

    return ChangeSet{removed};
}

void UnusedArgumentStep::decanonicalize(llvm::Function &F, const ChangeSet& cs) {

}

ChangeSet ArgumentOrderStep::canonicalize(llvm::Function &F) {
    // Sort arguments by their type names

    // TODO: Determine way to store associated state.
    // First try had returned a result (function pointer, permutation vector) from this
    // AI suggested to store in LLVM Function metadata -> explore that
    return ChangeSet{};
}

void ArgumentOrderStep::decanonicalize(llvm::Function &F, const ChangeSet& cs) {
    // Restore original argument order
}

ChangeSet DebugPrintStep::canonicalize(llvm::Function &F) {
    std::cout << "Canonicalizing function: " << F.getName().str() << std::endl;
    return ChangeSet{};
}

void DebugPrintStep::decanonicalize(llvm::Function &F, const ChangeSet& cs) {
    std::cout << "Decanonicalizing function: " << F.getName().str() << std::endl;
}