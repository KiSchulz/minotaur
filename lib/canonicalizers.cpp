#include "canonicalizers.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <vector>
#include <algorithm>
#include <iostream>

using namespace minotaur;

void ArgumentOrderCanonicalizationStep::canonicalize(llvm::Function &F) {
    // Sort arguments by their type names

    // TODO: Determine way to store associated state.
    // First try had returned a result (function pointer, permutation vector) from this
    // AI suggested to store in LLVM Function metadata -> explore that
}

void ArgumentOrderCanonicalizationStep::decanonicalize(llvm::Function &F) {
    // Restore original argument order
}

void DebugPrintCanonicalizationStep::canonicalize(llvm::Function &F) {
    std::cout << "Canonicalizing function: " << F.getName().str() << std::endl;
}

void DebugPrintCanonicalizationStep::decanonicalize(llvm::Function &F) {
    std::cout << "Decanonicalizing function: " << F.getName().str() << std::endl;
}