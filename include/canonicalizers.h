#pragma once

#include <string>
#include <map>
#include <vector>
#include "llvm/IR/Function.h"
#include "canonicalizer.h"

class ArgumentOrderCanonicalizationStep : public minotaur::CanonicalizationStep {
private:
  // Maps function to the permutation needed to restore original order
  std::map<llvm::Function*, std::vector<unsigned>> permutationMap;

public:
  void canonicalize(llvm::Function &F) override;
  
  void decanonicalize(llvm::Function &F) override;
  
  std::string getName() const override {
    return "ArgumentOrderCanonicalizationStep";
  }
};

class DebugPrintCanonicalizationStep : public minotaur::CanonicalizationStep {
public:
  void canonicalize(llvm::Function &F) override;
  void decanonicalize(llvm::Function &F) override;
  std::string getName() const override {
    return "DebugPrintCanonicalizationStep";
  }
};