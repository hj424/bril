#include <string>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"
using namespace llvm;

namespace {
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}


    virtual bool runOnFunction(Function &F) {
      // Get the function to call from our runtime library.
      LLVMContext& Ctx = F.getContext();
      auto cnt_mul = F.getParent()->getOrInsertFunction(
        "cnt_mul", Type::getVoidTy(Ctx), Type::getInt32Ty(Ctx)
      );

      errs() << "Function body: \n";
      errs() << F << "\n";
      for (auto& B : F) {
        for (auto& I : B) {
          if (auto* op = dyn_cast<BinaryOperator>(&I)) {
            if (std::strncmp(I.getOpcodeName(),"mul",16)) {
              // Insert operation after each mul
              IRBuilder<> builder(op);
              // update pointer, after the mul
              builder.SetInsertPoint(&B, ++builder.GetInsertPoint());

              // Insert a call to our function.
              builder.CreateCall(cnt_mul);
            }
          }
        }
      }
      errs() << "Adding print statement and counter for every mul operation!" << "\n";
      return true;
    }
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);
