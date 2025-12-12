#pragma once

#include <string>
#include <map>
#include <vector>
#include "llvm/IR/Function.h"
#include "canonicalizer.h"

using namespace minotaur::canonicalizer;

class ArgumentOrderStep : public CanonicalizationStep {

public:
  ChangeSet canonicalize(llvm::Function &F) override;
  
  void decanonicalize(llvm::Function &F, const ChangeSet& cs) override;
  
  std::string getName() const override {
    return "ArgumentOrderCanonicalizationStep";
  }
};

class DebugPrintStep : public CanonicalizationStep {
public:
  ChangeSet canonicalize(llvm::Function &F) override;
  void decanonicalize(llvm::Function &F, const ChangeSet& cs) override;
  std::string getName() const override {
    return "DebugPrintCanonicalizationStep";
  }
};