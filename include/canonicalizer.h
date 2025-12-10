#pragma once

#include <string>
#include <vector>
#include <memory>
#include "llvm/IR/Function.h"
namespace minotaur {

class CanonicalizationStep {
public:
  virtual ~CanonicalizationStep() = default;
  
  virtual void canonicalize(llvm::Function &F) = 0;

  // Reverse our transformations
  virtual void decanonicalize(llvm::Function &F) = 0;
  
  virtual std::string getName() const = 0;
  
  // Use global flags probaly - elegant solution needed
  virtual bool shouldRun(const llvm::Function &F) const { return true; }
};


class Canonicalizer {
  std::vector<std::unique_ptr<CanonicalizationStep>> steps;

public:
  void addStep(std::unique_ptr<CanonicalizationStep> step);
  void canonicalize(llvm::Function &F);
  void decanonicalize(llvm::Function &F);

};
}