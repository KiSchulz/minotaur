#include "canonicalizer.h"
using namespace minotaur::canonicalizer;
void Canonicalizer::addStep(std::unique_ptr<CanonicalizationStep> step) {
    steps.push_back(std::move(step));
};

std::vector<ChangeSet> Canonicalizer::canonicalize(llvm::Function &F) {
    std::vector<ChangeSet> changeSets;
    for (const auto& step : steps) {
        if (step->shouldRun(F)) {
            changeSets.push_back(step->canonicalize(F));
        }
    }
    return changeSets;
}

void Canonicalizer::decanonicalize(llvm::Function &F, const std::vector<ChangeSet>& changes) {
    // Apply decanonicalization in reverse order
    int changeIndex = changes.size() - 1;
    for (auto it = steps.rbegin(); it != steps.rend(); ++it) {
        if ((*it)->shouldRun(F)) {
            (*it)->decanonicalize(F, changes[changeIndex--]);
        }
    }
}