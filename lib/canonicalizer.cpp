#include "canonicalizer.h"
#include "canonicalizers.h"
#include "expr.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

using namespace minotaur::canonicalizer;

void Canonicalizer::addStep(std::unique_ptr<CanonicalizationStep> step) {
  steps.push_back(std::move(step));
};

void Canonicalizer::updateVMap(llvm::ValueToValueMapTy &VMap,
                               const ChangeSet &change) {
  std::map<const llvm::Value *, llvm::WeakTrackingVH> update;
  for (auto [key, value] : VMap) {
    // TODO: remove dummy tests
    assert((*change.VMap).count(key));
    update[(*change.VMap)[key]] = value;
  }

  for (auto [key, value] : update) {
    VMap[key] = value;
  }
}

void Canonicalizer::cleanModule(llvm::Function *F) {
  F->setName("canonicalF");
  llvm::Module *workM = F->getParent();

  std::set<llvm::Function *> usedFunctions;
  llvm::SmallVector<llvm::Function *, 16> workSet;
  workSet.push_back(F);
  while (!workSet.empty()) {
    auto current = workSet.back();
    workSet.pop_back();
    usedFunctions.insert(current);

    for (llvm::BasicBlock &BB : *current) {
      for (llvm::Instruction &I : BB) {
        if (auto *callInst = llvm::dyn_cast<llvm::CallBase>(&I)) {
          if (!usedFunctions.contains(callInst->getCalledFunction())) {
            workSet.push_back(callInst->getCalledFunction());
          }
        }
      }
    }
  }

  for (auto &F : make_early_inc_range(*workM)) {
    if (!usedFunctions.contains(&F)) {
      F.dropAllReferences();
      F.eraseFromParent();
    }
  }
}

bool Canonicalizer::shouldRunStep(const CanonicalizationStep &step) {
  return steps_config.empty() ||
         steps_config.find("all") != std::string::npos ||
         steps_config.find(step.getName()) != std::string::npos;
}

std::vector<ChangeSet>
Canonicalizer::canonicalize(llvm::Function *F, llvm::Instruction *I,
                            llvm::ValueToValueMapTy &VMap) {
  llvm::Function *currentF = F;
  llvm::Instruction *currentI = I;
  std::vector<ChangeSet> changeSets;
  for (const auto &step : steps) {
    if (!shouldRunStep(*step)) {
      continue;
    }

    changeSets.push_back(step->canonicalize(currentF, currentI));

    currentF = changeSets.back().stepFunc;
    currentI = changeSets.back().I;
    updateVMap(VMap, changeSets.back());

    // TODO: remove dummy tests
    assert(I == VMap[currentI]);
  }

  cleanModule(currentF);
  return changeSets;
}

minotaur::Rewrite
Canonicalizer::decanonicalize(const Rewrite &R,
                              const std::vector<ChangeSet> &changes) {
  Rewrite ret = R;
  // Apply decanonicalization in reverse order
  int changeIndex = changes.size() - 1;
  for (auto it = steps.rbegin(); it != steps.rend(); ++it) {
    if (!shouldRunStep(**it)) {
      continue;
    }

    ret = (*it)->decanonicalize(ret, changes[changeIndex--]);
  }
  return ret;
}

Canonicalizer::Canonicalizer(std::string_view steps_config)
    : steps_config(steps_config) {
  addStep(std::make_unique<UnusedArgumentStep>());
  addStep(std::make_unique<ArgumentOrderStep>());
  addStep(std::make_unique<ArgumentRenamingStep>());
  addStep(std::make_unique<LeqLtComparisonStep>());
  addStep(std::make_unique<StrictComparisonStep>());
}
