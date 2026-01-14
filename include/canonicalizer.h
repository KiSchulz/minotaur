#pragma once

#include "expr.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include <memory>
#include <string>
#include <vector>

namespace minotaur::canonicalizer {

struct ChangeSet {
  llvm::Function *stepFunc;
  llvm::Instruction *I;
  std::unique_ptr<llvm::ValueToValueMapTy> VMap;
};

class CanonicalizationStep {
public:
  virtual ~CanonicalizationStep() = default;

  virtual ChangeSet canonicalize(llvm::Function *F, llvm::Instruction *I) = 0;

  // Reverse our transformations
  virtual Rewrite decanonicalize(const Rewrite &R, const ChangeSet &cs) = 0;

  virtual std::string getName() const = 0;
};

class Canonicalizer {
  std::string steps_config;
  std::vector<std::unique_ptr<CanonicalizationStep>> steps;

  void updateVMap(llvm::ValueToValueMapTy &VMap, const ChangeSet &change);
  void cleanModule(llvm::Function *F);
  bool shouldRunStep(const CanonicalizationStep &step);

public:
  explicit Canonicalizer(std::string_view steps_config);
  
  void addStep(std::unique_ptr<CanonicalizationStep> step);
  std::vector<ChangeSet> canonicalize(llvm::Function *F, llvm::Instruction *I,
                                      llvm::ValueToValueMapTy &VMap);
  Rewrite decanonicalize(const Rewrite &R,
                         const std::vector<ChangeSet> &changes);
};
} // namespace minotaur::canonicalizer
