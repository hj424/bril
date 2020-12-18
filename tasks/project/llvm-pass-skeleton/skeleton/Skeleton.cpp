#include <string>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Dominators.h"

#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"


using namespace llvm;

namespace {
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    // This example modifies the program, but preserves CFG.
    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesCFG();
      // Legacy analysis pass to compute loop infomation.  
      AU.addRequired<LoopInfoWrapperPass>();
      // Legacy analysis pass to compute dominator tree.
      AU.addRequired<DominatorTreeWrapperPass>();
    }

    // Loop analysis
    void LoopAnalysis(Loop *L, BinaryOperator *&Increment, Value *&Bound, BranchInst *&BackBranch) {
      // loop should be in simplified form
      if (!L->isLoopSimplifyForm()) {
        errs() << "Loop is not in normal form\n";
      }
      // There must be exactly one exiting block, and it must be the same at the latch.
      BasicBlock *Latch = L->getLoopLatch();
      if (L->getExitingBlock() != Latch) {
        errs() << "Exiting and latch block are different\n";
      }
      // Latch block must end in a conditional branch.
      BackBranch = dyn_cast<BranchInst>(Latch->getTerminator());
      if (!BackBranch || !BackBranch->isConditional()) {
        errs() << "Could not find back-branch\n";
      }
      //errs() << "Found back branch: "; BackBranch->dump();

      // Find loop increment and bound
      ICmpInst *Compare = dyn_cast<ICmpInst>(BackBranch->getCondition());
      Increment = dyn_cast<BinaryOperator>(Compare->getOperand(0));
      Bound = Compare->getOperand(1);
      errs() << "Loop bound: " << Bound << "\n";
      errs() << "Loop increment: " << Increment << "\n";
    }

    virtual bool runOnFunction(Function &F) {
      errs() << "Default optimizations... \n";
     
      // craete llvm function with
      LLVMContext &Ctx = F.getContext();
      std::vector<Type*> paramTypes = {Type::getInt32Ty(Ctx)};
      Type *retType = Type::getVoidTy(Ctx);
      FunctionType *logFuncType = FunctionType::get(retType, paramTypes, false);
      Module* module = F.getParent();
      //Constant *logFunc = module->getOrInsertFunction("logop", logFuncType);

      // apply useful passes
      legacy::FunctionPassManager FPM(module);
      FPM.add(createConstantPropagationPass());
      FPM.add(createIndVarSimplifyPass());
      FPM.add(createDeadCodeEliminationPass());
      FPM.add(createLoopSimplifyPass());
      FPM.doInitialization();
      bool changed = FPM.run(F);
      FPM.doFinalization();

      // perform loop analysis
      LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

      // iterate all loops
      for (auto* L : LI) {
        if  (L->getSubLoops().size() != 1) {
          errs() << "Unnested loop, stop flattening it!";
          break;
        }
        else {
          errs() << "Find nested loop...\n";
          /*
          ScalarEvolution *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
          LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
          auto *DTWP = getAnalysisIfAvailable<DominatorTreeWrapperPass>();
          DominatorTree *DT = DTWP ? &DTWP->getDomTree() : nullptr;
          auto &TTIP = getAnalysis<TargetTransformInfoWrapperPass>();
          TargetTransformInfo *TTI = &TTIP.getTTI(*L->getHeader()->getParent());
          AssumptionCache *AC = &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(
          *L->getHeader()->getParent());
          */
          Loop *InnerLoop = *L->begin();
          errs() << "Loop flattening running on nested loop: " << L->getHeader()->getName() << "\n";
          //
          BranchInst *InnerBranch, *OuterBranch;
          
          PHINode *InnerInductionPHI, *OuterInductionPHI;
          SmallPtrSet<PHINode *, 4> InnerPHIsToTransform;
          Value *InnerBound, *OuterBound;
          BinaryOperator *InnerIncrement, *OuterIncrement;
          //-----------------------------------------------------------
          // flatten the nested loop
          //-----------------------------------------------------------
          // Loop analysis
          LoopAnalysis(L, OuterIncrement, OuterBound, OuterBranch);
          LoopAnalysis(InnerLoop, InnerIncrement, InnerBound, InnerBranch);
          // new loop bound
          Value *NewBound = BinaryOperator::CreateMul(InnerBound, OuterBound, "NewBound", L->getLoopPreheader()->getTerminator());
          // modify the trip cout of the outer loop
          cast<User>(OuterBranch->getCondition())->setOperand(1, NewBound);
        }
      }

      // change the code if LICM optimization is performaned 
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
