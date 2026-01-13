#include "canonicalizers.h"
#include "expr.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

using namespace minotaur::canonicalizer;

ChangeSet UnusedArgumentStep::canonicalize(llvm::Function *F,
                                           llvm::Instruction *I) {
  std::vector<std::pair<unsigned, llvm::Type *>> removed;
  std::vector<llvm::Type *> argTypes;

  for (const llvm::Argument &arg : F->args()) {
    if (arg.use_empty()) {
      removed.push_back({arg.getArgNo(), arg.getType()});
    } else {
      argTypes.push_back(arg.getType());
    }
  }

  llvm::Function *canonicalizedF = llvm::Function::Create(
      llvm::FunctionType::get(F->getReturnType(), argTypes, F->isVarArg()),
      F->getLinkage(), F->getName() + "." + getName(), F->getParent());
  canonicalizedF->copyAttributesFrom(F);
  canonicalizedF->setCallingConv(F->getCallingConv());

  auto VMap = std::make_unique<llvm::ValueToValueMapTy>();
  llvm::Function::arg_iterator canonArgIt = canonicalizedF->arg_begin();
  for (const llvm::Argument &arg : F->args()) {
    if (arg.use_empty()) {
      (*VMap)[&arg] = llvm::UndefValue::get(arg.getType());
    } else {
      canonArgIt->setName(arg.getName());
      (*VMap)[&arg] = &*canonArgIt++;
    }
  }

  llvm::SmallVector<llvm::ReturnInst *, 8> returns;
  llvm::CloneFunctionInto(canonicalizedF, F, *VMap,
                          llvm::CloneFunctionChangeType::LocalChangesOnly,
                          returns);

  auto newI = llvm::cast<llvm::Instruction>((*VMap)[I]);
  return ChangeSet{canonicalizedF, newI, std::move(VMap)};
}

minotaur::Rewrite UnusedArgumentStep::decanonicalize(const Rewrite &R,
                                                     const ChangeSet &cs) {
  return R;
}

ChangeSet ArgumentOrderStep::canonicalize(llvm::Function *F,
                                          llvm::Instruction *I) {
  auto getTypeKey = [](const llvm::Type *T) {
    std::string s;
    llvm::raw_string_ostream os(s);
    T->print(os);
    os.flush();
    return s;
  };
  std::map<std::string, llvm::Type *> typeNameMap;
  for (const llvm::Argument &arg : F->args()) {
    typeNameMap[getTypeKey(arg.getType())] = arg.getType();
  }

  std::map<std::string, std::vector<llvm::Argument *>> typeToArgs;
  for (const llvm::BasicBlock &BB : *F) {
    for (const llvm::Instruction &I : BB) {
      for (llvm::Value *op : I.operands()) {
        if (llvm::Argument *arg = llvm::dyn_cast<llvm::Argument>(op)) {
          std::vector<llvm::Argument *> &canonArgs =
              typeToArgs[getTypeKey(arg->getType())];
          if (std::find(canonArgs.begin(), canonArgs.end(), arg) ==
              canonArgs.end()) {
            canonArgs.push_back(arg);
          }
        }
      }
    }
  }

  std::vector<unsigned> originalOrder;
  std::vector<llvm::Type *> argTypes;
  for (auto &[type, canonArgs] : typeToArgs) {
    for (llvm::Argument *arg : canonArgs) {
      argTypes.push_back(typeNameMap[type]);
      originalOrder.push_back(arg->getArgNo());
    }
  }

  // Add any arguments not used in the body to the end of the argument list
  for (const llvm::Argument &arg : F->args()) {
    if (std::find(originalOrder.begin(), originalOrder.end(), arg.getArgNo()) ==
        originalOrder.end()) {
      argTypes.push_back(arg.getType());
      originalOrder.push_back(arg.getArgNo());
    }
  }

  llvm::Function *canonicalizedF = llvm::Function::Create(
      llvm::FunctionType::get(F->getReturnType(), argTypes, F->isVarArg()),
      F->getLinkage(), F->getName() + "." + getName(), F->getParent());

  auto VMap = std::make_unique<llvm::ValueToValueMapTy>();
  llvm::Function::arg_iterator canonArgIt = canonicalizedF->arg_begin();
  for (unsigned argNo : originalOrder) {
    llvm::Argument *arg = F->getArg(argNo);
    canonArgIt->setName(arg->getName());
    (*VMap)[arg] = &*canonArgIt++;
  }

  llvm::SmallVector<llvm::ReturnInst *, 8> returns;
  llvm::CloneFunctionInto(canonicalizedF, F, *VMap,
                          llvm::CloneFunctionChangeType::LocalChangesOnly,
                          returns);

  auto newI = llvm::cast<llvm::Instruction>((*VMap)[I]);
  return ChangeSet{canonicalizedF, newI, std::move(VMap)};
}

minotaur::Rewrite ArgumentOrderStep::decanonicalize(const Rewrite &R,
                                                    const ChangeSet &cs) {
  return R;
}

std::tuple<llvm::APInt, llvm::CmpInst::Predicate>
StrictComparisonStep::adjustInt(const llvm::APInt &Val,
                                llvm::CmpInst::Predicate Pred, unsigned OpIdx) {
  using llvmP = llvm::CmpInst::Predicate;
  assert(OpIdx == 0 || OpIdx == 1);

  const llvm::APInt One{Val.getBitWidth(), 1};
  llvm::APInt AdjVal;
  bool Overflowed = false;
  switch (Pred) {
  case llvmP::ICMP_SGE: {
    AdjVal = OpIdx == 0 ? Val.sadd_ov(One, Overflowed)
                        : Val.ssub_ov(One, Overflowed);
    break;
  }
  case llvmP::ICMP_UGE: {
    AdjVal = OpIdx == 0 ? Val.uadd_ov(One, Overflowed)
                        : Val.usub_ov(One, Overflowed);
    break;
  }
  case llvmP::ICMP_SLE: {
    AdjVal = OpIdx == 0 ? Val.ssub_ov(One, Overflowed)
                        : Val.sadd_ov(One, Overflowed);
    break;
  }
  case llvmP::ICMP_ULE: {
    AdjVal = OpIdx == 0 ? Val.usub_ov(One, Overflowed)
                        : Val.uadd_ov(One, Overflowed);
    break;
  }
  default:
    return {Val, Pred};
  }

  llvmP AdjPred = llvm::CmpInst::getStrictPredicate(Pred);
  return Overflowed ? std::tuple{Val, Pred} : std::tuple{AdjVal, AdjPred};
}

std::tuple<llvm::APFloat, llvm::CmpInst::Predicate>
StrictComparisonStep::adjustFloat(const llvm::APFloat &Val,
                                  llvm::CmpInst::Predicate Pred,
                                  unsigned OpIdx) {
  using llvmP = llvm::CmpInst::Predicate;
  assert(OpIdx == 0 || OpIdx == 1);

  if (Val.isNaN()) {
    return {Val, Pred};
  }

  llvm::APFloat AdjVal{Val};
  switch (Pred) {
  case llvmP::FCMP_OGE:
  case llvmP::FCMP_UGE: {
    OpIdx == 0 ? AdjVal.next(false) : AdjVal.next(true);
    break;
  }
  case llvmP::FCMP_OLE:
  case llvmP::FCMP_ULE: {
    OpIdx == 0 ? AdjVal.next(true) : AdjVal.next(false);
    break;
  }
  default:
    return {Val, Pred};
  }

  llvmP AdjPred = llvm::CmpInst::getStrictPredicate(Pred);
  return AdjVal.bitwiseIsEqual(Val) ? std::tuple{Val, Pred}
                                    : std::tuple{AdjVal, AdjPred};
}

ChangeSet StrictComparisonStep::canonicalize(llvm::Function *F,
                                             llvm::Instruction *I) {
  llvm::Function *CanonicalizedF =
      llvm::Function::Create(F->getFunctionType(), F->getLinkage(),
                             F->getName() + "." + getName(), F->getParent());

  auto VMap = std::make_unique<llvm::ValueToValueMapTy>();
  for (auto I = F->arg_begin(), CI = CanonicalizedF->arg_begin();
       I != F->arg_end(); I++, CI++) {
    CI->setName(I->getName());
    (*VMap)[&*I] = &*CI;
  }

  llvm::SmallVector<llvm::ReturnInst *, 8> Returns;
  llvm::CloneFunctionInto(CanonicalizedF, F, *VMap,
                          llvm::CloneFunctionChangeType::LocalChangesOnly,
                          Returns);

  for (auto &BB : *F) {
    for (auto &Inst : BB) {
      if (!llvm::isa<llvm::ICmpInst, llvm::FCmpInst>(&Inst)) {
        continue;
      }

      int ConstOpIdx = -1;
      for (auto &Op : Inst.operands()) {
        if (Op.get()->getType()->isVectorTy()) {
          // TODO: handle vector types
          continue;
        }
        if (llvm::isa<llvm::Constant>(Op.get())) {
          ConstOpIdx = Op.getOperandNo();
          break;
        }
      }
      if (ConstOpIdx == -1) {
        continue;
      }

      if (auto *Cmp = llvm::dyn_cast<llvm::ICmpInst>(&Inst)) {
        auto *Const =
            llvm::dyn_cast<llvm::ConstantInt>(Inst.getOperand(ConstOpIdx));
        if (!Const) {
          continue;
        }
        auto [newVal, newPred] =
            adjustInt(Const->getValue(), Cmp->getPredicate(), ConstOpIdx);

        auto newConst = llvm::ConstantInt::get(Const->getType(), newVal);
        llvm::cast<llvm::CmpInst>((*VMap)[Cmp])->setPredicate(newPred);
        llvm::cast<llvm::CmpInst>((*VMap)[Cmp])->setOperand(ConstOpIdx, newConst);
      } else if (auto *Cmp = llvm::dyn_cast<llvm::FCmpInst>(&Inst)) {
        auto *Const =
            llvm::dyn_cast<llvm::ConstantFP>(Inst.getOperand(ConstOpIdx));
        if (!Const) {
          continue;
        }

        auto [newVal, newPred] =
            adjustFloat(Const->getValue(), Cmp->getPredicate(), ConstOpIdx);

        auto newConst = llvm::ConstantFP::get(Const->getType(), newVal);
        llvm::cast<llvm::CmpInst>((*VMap)[Cmp])->setPredicate(newPred);
        llvm::cast<llvm::CmpInst>((*VMap)[Cmp])->setOperand(ConstOpIdx, newConst);
      }
    }
  }
  std::string buf;
  llvm::raw_string_ostream os{buf};
  CanonicalizedF->getParent()->print(os, nullptr);
  os.flush();

  auto newI = llvm::cast<llvm::Instruction>((*VMap)[I]);
  return ChangeSet{CanonicalizedF, newI, std::move(VMap)};
}

minotaur::Rewrite StrictComparisonStep::decanonicalize(const Rewrite &R,
                                                       const ChangeSet &cs) {
  return R;
}
