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
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

using namespace llvm;

namespace {
  struct SkeletonPass : public LoopPass {
    static char ID;
    SkeletonPass() : LoopPass(ID) {}

    // This example modifies the program, but preserves CFG.
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      getLoopAnalysisUsage(AU);
      // Legacy analysis pass to compute loop infomation.  
      AU.addRequired<LoopInfoWrapperPass>();
      // Legacy analysis pass to compute dominator tree.
      AU.addRequired<DominatorTreeWrapperPass>();
    }

    // Loop analysis
    static void LoopAnalysis(Loop *L, BinaryOperator *&Increment, Value *&Bound, BranchInst *&BackBranch, PHINode *&InductionPHI, ScalarEvolution *SE) {
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
      //BackBranch = dyn_cast<BranchInst>(Latch->getTerminator());
      BackBranch = dyn_cast<BranchInst>(L->getExitingBlock()->getTerminator());
      errs() << "C1...\n";
      //if (!BackBranch || !BackBranch->isConditional()) {
      //  errs() << "Could not find back-branch\n";
      //}
      //errs() << "Found back branch: "; BackBranch->dump();

      // Find loop increment and bound
      ICmpInst *Compare = dyn_cast<ICmpInst>(BackBranch->getCondition());
      errs() << "C2...\n";
      Increment = dyn_cast<BinaryOperator>(Compare->getOperand(0));
      Bound = Compare->getOperand(1);
      errs() << "Loop bound: " << *Bound << "\n";
      errs() << "Loop increment: " << Increment << "\n";

      // find induction PHI nodes
      //InductionPHI = L->getCanonicalInductionVariable();
      InductionPHI = L->getInductionVariable(*SE);
      /*
      for (PHINode &PHI : L->getHeader()->phis()) {
        &PHI = L->getCanonicalInductionVariable();
        InductionDescriptor ID;
        if (InductionDescriptor::isInductionPHI(&PHI, L, SE, ID)) {
          InductionPHI = &PHI;
          errs() << "Found induction PHI: "; InductionPHI->dump();
          break;
        }
      }
      */
      if (!InductionPHI) {
        errs() << "Could not find induction PHI\n";
      }
    }

    // PHInodeAnalysis
    static void PHInodeAnalysis(Loop *L, Loop *InnerLoop,
                      SmallPtrSetImpl<PHINode *> &InnerPHIsToTransform,
                      PHINode *InnerInductionPHI, PHINode *OuterInductionPHI) {
      
    }

    bool runOnLoop(Loop *L, LPPassManager &LPM) override {
      errs() << "Default optimizations... \n";
     
      if  (L->getSubLoops().size() != 1) {
        errs() << "Unnested loop, stop flattening it!";
      }
      else {
        errs() << "Find nested loop...\n";
        // Loop analysis pass
        ScalarEvolution *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
        errs() << "1...\n";
        // Loop basic info
        LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
        errs() << "2...\n";
        // Donimatortree
        auto *DTWP = getAnalysisIfAvailable<DominatorTreeWrapperPass>();
        DominatorTree *DT = DTWP ? &DTWP->getDomTree() : nullptr;
        errs() << "3...\n";
        
        Loop *InnerLoop = *L->begin();
        errs() << "Loop flattening running on nested loop: " << L->getHeader()->getName() << "\n";
        //
        BranchInst *InnerBranch, *OuterBranch;
        
        PHINode *InnerInductionPHI, *OuterInductionPHI;
        Value *InnerBound, *OuterBound;
        BinaryOperator *InnerIncrement, *OuterIncrement;

        //-----------------------------------------------------------
        // flatten the nested loop
        //-----------------------------------------------------------
        // Loop analysis
        LoopAnalysis(L, OuterIncrement, OuterBound, OuterBranch, OuterInductionPHI, SE);
        LoopAnalysis(InnerLoop, InnerIncrement, InnerBound, InnerBranch, InnerInductionPHI, SE);

        SmallPtrSet<PHINode *, 4> InnerPHIsToTransform;
        // PHI node analysis
        PHInodeAnalysis(L, InnerLoop, InnerPHIsToTransform, InnerInductionPHI, OuterInductionPHI);

        // remove the PHI nodes related to innerloop backedge
        //InnerInductionPHI->removeIncomingValue(InnerLoop->getLoopLatch());
        //for (PHINode *PHI : InnerPHIsToTransform)
         // PHI->removeIncomingValue(InnerLoop->getLoopLatch());

        // new loop bound
        Value *NewBound = BinaryOperator::CreateMul(InnerBound, OuterBound, "NewBound", L->getLoopPreheader()->getTerminator());
        errs() << *NewBound <<"\n";
        // modify the trip cout of the outer loop
        cast<User>(OuterBranch->getCondition())->setOperand(1, NewBound);
        // replace the inner loop backedge with unconditional exit
        BasicBlock *InnerExitBlock = InnerLoop->getExitBlock();
        BasicBlock *InnerExitingBlock = InnerLoop->getExitingBlock();
        errs() << "==========================\n";
        errs() << *InnerExitBlock << "\n";
        errs() << "==========================\n";
        errs() << *InnerExitingBlock << "\n";
        InnerExitingBlock->getTerminator()->eraseFromParent();
        BranchInst::Create(InnerExitBlock, InnerExitingBlock);
        DT->deleteEdge(InnerExitingBlock, InnerLoop->getHeader());
        errs() << "==========================\n";
        errs() << *InnerExitBlock << "\n";
        errs() << "==========================\n";
        errs() << *InnerExitingBlock << "\n";

        // mark the inner loop as a deleted one
        LPM.markLoopAsDeleted(*InnerLoop);
        SE->forgetLoop(L);
        SE->forgetLoop(InnerLoop);
        //LI->erase(InnerLoop);
        errs() << *L << "\n";
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
