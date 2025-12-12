#include "canonicalizers.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <vector>
#include <algorithm>
#include <iostream>

using namespace minotaur::canonicalizer;

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