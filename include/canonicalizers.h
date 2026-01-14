#pragma once

#include "canonicalizer.h"
#include "expr.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include <string>

namespace minotaur::canonicalizer {

class UnusedArgumentStep : public CanonicalizationStep {
public:
  ChangeSet canonicalize(llvm::Function *F, llvm::Instruction *I) override;
  Rewrite decanonicalize(const Rewrite &R, const ChangeSet &cs) override;
  std::string getName() const override {
    return "UnusedArgumentStep";
  }
};

class ArgumentOrderStep : public CanonicalizationStep {
public:
  ChangeSet canonicalize(llvm::Function *F, llvm::Instruction *I) override;
  Rewrite decanonicalize(const Rewrite &R, const ChangeSet &cs) override;
  std::string getName() const override {
    return "ArgumentOrderStep";
  }
};

class ArgumentRenamingStep: public CanonicalizationStep {
public:
  ChangeSet canonicalize(llvm::Function *F, llvm::Instruction *I) override;
  Rewrite decanonicalize(const Rewrite &R, const ChangeSet &cs) override;
  std::string getName() const override {
    return "ArgumentRenamingStep";
  }
};

class LeqLtComparisonStep: public CanonicalizationStep {
public:
  ChangeSet canonicalize(llvm::Function *F, llvm::Instruction *I) override;
  Rewrite decanonicalize(const Rewrite &R, const ChangeSet &cs) override;
  std::string getName() const override {
    return "LeqLtComparisonStep";
  }
};

class StrictComparisonStep : public CanonicalizationStep {
  std::tuple<llvm::APInt, llvm::CmpInst::Predicate>
  adjustInt(const llvm::APInt &Val, llvm::CmpInst::Predicate Pred,
            unsigned OpIdx);
  std::tuple<llvm::APFloat, llvm::CmpInst::Predicate>
  adjustFloat(const llvm::APFloat &Val, llvm::CmpInst::Predicate Pred,
              unsigned OpIdx);

public:
  ChangeSet canonicalize(llvm::Function *F, llvm::Instruction *I) override;
  Rewrite decanonicalize(const Rewrite &R, const ChangeSet &cs) override;
  std::string getName() const override {
    return "StrictComparisonStep";
  }
};

} // namespace minotaur::canonicalizer
