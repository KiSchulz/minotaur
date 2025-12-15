#include "canonicalizers.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Constants.h"
#include <vector>
#include <algorithm>
#include <iostream>

using namespace minotaur::canonicalizer;

ChangeSet UnusedArgumentStep::canonicalize(llvm::Function &F) {
    std::vector<std::pair<unsigned, llvm::Type*>> removed;
    std::vector<llvm::Type*> argTypes;

    for (llvm::Argument &arg : F.args()) {
        if (arg.use_empty()) {
            removed.push_back({arg.getArgNo(), arg.getType()});
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
    std::vector<unsigned> originalOrder;
    std::vector<llvm::Type*> argTypes;
    std::map<llvm::Type*, std::vector<llvm::Argument*>> typeToArgs;

    for (llvm::BasicBlock &BB : F) {
        for (llvm::Instruction &I : BB) {
            for (llvm::Value* op : I.operands()) {
                if (llvm::Argument* arg = llvm::dyn_cast<llvm::Argument>(op)) {
                    std::vector<llvm::Argument*>& canonArgs = typeToArgs[arg->getType()];
                    if (std::find(canonArgs.begin(), canonArgs.end(), arg) == canonArgs.end()) {
                        canonArgs.push_back(arg);
                    }
                }
            }
        }
    }

    for (auto& [type, canonArgs] : typeToArgs) {
        for (llvm::Argument* arg : canonArgs) {
            argTypes.push_back(type);
            originalOrder.push_back(arg->getArgNo());
        }
    }

    // Add any arguments not used in the body to the end of the argument list
    for (llvm::Argument &arg : F.args()) {
        if (std::find(originalOrder.begin(), originalOrder.end(), arg.getArgNo()) == originalOrder.end()) {
            argTypes.push_back(arg.getType());
            originalOrder.push_back(arg.getArgNo());
        }
    }

    llvm::Function* canonicalizedF = llvm::Function::Create(
        llvm::FunctionType::get(F.getReturnType(), argTypes, F.isVarArg()),
        F.getLinkage(),
        F.getName() + ".canonicalized",
        F.getParent()
    );

    llvm::ValueToValueMapTy VMap;
    llvm::Function::arg_iterator canonArgIt = canonicalizedF->arg_begin();
    for (unsigned argNo : originalOrder) {
        llvm::Argument* arg = F.getArg(argNo);
        canonArgIt->setName(arg->getName());
        VMap[arg] = &*canonArgIt++;
    }

    llvm::SmallVector<llvm::ReturnInst*, 8> returns;
    llvm::CloneFunctionInto(canonicalizedF, &F, VMap,
        llvm::CloneFunctionChangeType::LocalChangesOnly, returns);
    
    return ChangeSet{originalOrder};
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