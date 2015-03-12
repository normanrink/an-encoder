
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"

#include "Coder.h"
#include "UsesVault.h"

using namespace llvm;

namespace {
  struct OperationsEncoder : public BasicBlockPass {
    OperationsEncoder(Coder *c) : BasicBlockPass(ID), C(c) {}

    bool runOnBasicBlock(BasicBlock &BB) override;

    static char ID;
  private:
    Coder *C;
  };
}

char OperationsEncoder::ID = 0;

bool OperationsEncoder::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  LLVMContext &ctx = BB.getContext();
  Module *M = BB.getParent()->getParent();

  auto I = BB.begin(), E = BB.end();
  while( I != E) {
    auto N = std::next(I);
    unsigned Op = I->getOpcode();
    switch (Op) {
    default: break;
    case Instruction::Call: {
      // Deferred to the 'CallHandler' pass.
      break;
    }
    case Instruction::GetElementPtr: {
      // Deferred to the 'GEPHandler' pass.
      break;
    }
    case Instruction::PtrToInt: {
      // Save all current uses of the 'ptrtoint' instruction to the
      // vault. These are the uses that need to be replaced with the
      // encoded integer value: (The 'createEncRegionEntry' method
      // will generate another use of the 'ptrtoint' instruction's
      // result, which msut not be replaced.)
      UsesVault UV(I->uses());

      Value *EncInt = C->createEncRegionEntry(I, std::next(I));

      UV.replaceWith(EncInt);
      modified = true;
      break;
    }
    case Instruction::IntToPtr: {
      Value *Op = I->getOperand(0);
      Value *DecInt = C->createEncRegionExit(Op, Op->getType(), I);
      I->setOperand(0, DecInt);
      modified = true;
      break;
    }
    case Instruction::SDiv:
    case Instruction::UDiv: {
      // FIXME: Handle division instructions properly. (Requires
      // looking into the signed-ness issue.)
      assert(0);
      break;
    }
#define HANDLE_BINOP(OPCODE, NAME)                             \
    case Instruction::OPCODE: {                                \
      Type *int64Ty = Type::getInt64Ty(ctx),                   \
           *origTy = I->getOperand(0)->getType();              \
      \
      SmallVector<Value*, 2> args;                             \
      args.push_back(C->preprocessForEncOp(I->getOperand(0), I)); \
      args.push_back(C->preprocessForEncOp(I->getOperand(1), I)); \
      \
      Value *Res = C->createEncBinop((NAME), args, I);         \
      \
      Res = C->postprocessFromEncOp(Res, origTy, I);           \
      I->replaceAllUsesWith(Res);                              \
      I->eraseFromParent();                                    \
      modified = true;                                         \
      break;                                                   \
    }
    HANDLE_BINOP(Add,  "add_enc")
    HANDLE_BINOP(Sub,  "sub_enc")
    HANDLE_BINOP(Mul,  "mul_enc")
    HANDLE_BINOP(URem, "umod_enc")
    HANDLE_BINOP(SRem, "smod_enc")
    HANDLE_BINOP(And,  "and_enc")
    HANDLE_BINOP(Or,   "or_enc")
    HANDLE_BINOP(Shl,  "shl_enc")
    HANDLE_BINOP(LShr, "shr_enc")
    HANDLE_BINOP(AShr, "ashr_enc")
    HANDLE_BINOP(Xor,  "xor_enc")
    }
    I = N;
  }

  return true;
}

Pass *createOperationsEncoder(Coder *c) {
  return new OperationsEncoder(c);
}
