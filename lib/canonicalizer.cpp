#include "canonicalizer.h"
using namespace minotaur;
void Canonicalizer::addStep(std::unique_ptr<CanonicalizationStep> step) {
    steps.push_back(std::move(step));
};

void Canonicalizer::canonicalize(llvm::Function &F) {
    for (const auto& step : steps) {
        if (step->shouldRun(F)) {
            step->canonicalize(F);
        }
    }
}

void Canonicalizer::decanonicalize(llvm::Function &F) {
    // Apply decanonicalization in reverse order
    for (auto it = steps.rbegin(); it != steps.rend(); ++it) {
        if ((*it)->shouldRun(F)) {
            (*it)->decanonicalize(F);
        }
    }
}