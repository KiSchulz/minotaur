#pragma once

#include <string>
#include <vector>
#include <memory>
#include <any>
#include "llvm/IR/Function.h"

namespace minotaur {
namespace canonicalizer {

struct ChangeSet {
  std::any changes;
};

class CanonicalizationStep {
public:
  virtual ~CanonicalizationStep() = default;
  
  virtual ChangeSet canonicalize(llvm::Function &F) = 0;

  // Reverse our transformations
  virtual void decanonicalize(llvm::Function &F, const ChangeSet& cs) = 0;
  
  virtual std::string getName() const = 0;
  
  // Use global flags probaly - elegant solution needed
  virtual bool shouldRun(const llvm::Function &F) const { return true; }
};


class Canonicalizer {
  std::vector<std::unique_ptr<CanonicalizationStep>> steps;

public:
  void addStep(std::unique_ptr<CanonicalizationStep> step);
  std::vector<ChangeSet> canonicalize(llvm::Function &F);
  void decanonicalize(llvm::Function &F, const std::vector<ChangeSet>& changes) ;

};
}
}