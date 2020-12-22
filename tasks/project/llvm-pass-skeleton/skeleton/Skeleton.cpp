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
    static void LoopAnalysis(Loop *L, BinaryOperator *&Increment, Value *&Bound, BranchInst *&BackBranch,  ScalarEvolution *SE) {
      // loop should be in simplified form
      if (!L->isLoopSimplifyForm()) {
        errs() << "Loop is not in a simplified form.\n";
      }
      // There must be exactly one exiting block, and it must be the same at the latch.
      BasicBlock *Latch = L->getLoopLatch();
      if (L->getExitingBlock() != Latch) {
        errs() << "Exiting and latch block are different.\n";
      }
      // Latch block must end in a conditional branch.
      //BackBranch = dyn_cast<BranchInst>(Latch->getTerminator());
      BackBranch = dyn_cast<BranchInst>(L->getExitingBlock()->getTerminator());
      //if (!BackBranch || !BackBranch->isConditional()) {
      //  errs() << "Could not find back-branch\n";
      //}
      //errs() << "Found back branch: "; BackBranch->dump();

      // Find loop increment and bound
      ICmpInst *Compare = dyn_cast<ICmpInst>(BackBranch->getCondition());
      Increment = dyn_cast<BinaryOperator>(Compare->getOperand(0));
      Bound = Compare->getOperand(1);
      errs() << "Loop bound: " << *Bound << "\n";
      //errs() << "Loop increment: " << Increment << "\n";
    }

    bool hasNoMemoryOps(BasicBlock *b)
    {
      for (BasicBlock::iterator I = b->begin(), E = b->end(); I != E; I++)
      {
        switch (I->getOpcode())
        {
        case Instruction::Store:
        case Instruction::Invoke:
        case Instruction::VAArg:
        case Instruction::Call:
        case Instruction::Load:
          errs() << *I << "\n";
          return false;
        }
      }
      return true;
    }

    bool isPerfectNest(Loop *L, BasicBlock*& body, LoopInfo *LI)
    {
      //get induction variable
      PHINode *ivar = L->getCanonicalInductionVariable();
      if (!ivar)
        return false;
      if (L->getBlocks().size() == 1)
      {
        body = *L->block_begin();
        return true;
      }
      else
      {
        //do we have a single subloop?
        if (L->getSubLoops().size() != 1)
          return false;
        //make sure all our non-nested loop blocks are innocuous
        for (Loop::block_iterator b = L->block_begin(), e = L->block_end(); b
            != e; b++)
        {
          BasicBlock *block = *b;
          if (LI->getLoopFor(block) == L)
          {
            if (!hasNoMemoryOps(block))
              return false;				
          }
        }

        //recursively check subloops
        return isPerfectNest(*L->begin(), body, LI);
      }
    }

    bool runOnLoop(Loop *L, LPPassManager &LPM) override {
      errs() << "Default optimizations... \n";
     
      if  (L->getSubLoops().size() != 1) {
        errs() << "Unnested loop, stop flattening. \n";
      }
      else {
        errs() << "Find nested loop...\n";
        // Loop analysis pass
        ScalarEvolution *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
        // Loop basic info
        LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
        // Donimatortree
        auto *DTWP = getAnalysisIfAvailable<DominatorTreeWrapperPass>();
        DominatorTree *DT = DTWP ? &DTWP->getDomTree() : nullptr;
        
        Loop *InnerLoop = *L->begin();
        //errs() << "Loop flattening running on nested loop: " << L->getHeader()->getName() << "\n";
        //errs() << "Loop flattening running on nested loop: " << L->getName() << "\n";


        // local variables 
        BranchInst *InnerBranch, *OuterBranch;
        Value *InnerBound, *OuterBound;
        BinaryOperator *InnerIncrement, *OuterIncrement;
        BasicBlock *body;

        // check whether this loop is a perfect loop
        if (!isPerfectNest(L, body, LI)) {
          errs() << "Not a perfect loop, stop flattening. \n";
          return false;
        }

        //-----------------------------------------------------------
        // flatten the nested loop
        //-----------------------------------------------------------
        // Loop analysis
        LoopAnalysis(L, OuterIncrement, OuterBound, OuterBranch, SE);
        LoopAnalysis(InnerLoop, InnerIncrement, InnerBound, InnerBranch, SE);

        // update loop bound
        /*
        Value *NewBound = BinaryOperator::CreateMul(InnerBound, OuterBound, "NewBound", L->getLoopPreheader()->getTerminator());
        errs() << "NewBound: " << *NewBound <<"\n";
        */
        // modify the trip cout of the outer loop
        //cast<User>(OuterBranch->getCondition())->setOperand(1, NewBound);
        //cast<User>(OuterBranch->getCondition())->getOperand(1)->replaceAllUsesWith(NewBound);
        //cast<User>(InnerBranch->getCondition())->setOperand(1, NewBound);
       
        // outer loop bound -> 1 
        errs() << "=============" << "\n";
        for (auto &I : *(L->getHeader())) {
          errs() << "Outer Loop IR: " << I << "\n";
          // op: icmp == 53
          if (I.getOpcode() == 53) { // Hard Coded for simplicity.
            /*
            errs() << *(cast<User>(I).getOperand(0)) << "\n";
            errs() << *(cast<User>(I).getOperand(1)) << "\n";
            errs() << *(cast<User>(I).getOperand(1))->getType()<< "\n";
            */
            // replace the old loop bound with new values
            Value* oldvalue = (I.getOperand(1));
            Value* newvalue = ConstantInt::get(oldvalue->getType(), 1);
            //I.setOperand(1, newvalue);
            I.getOperand(1)->replaceAllUsesWith(newvalue);
            //I.getOperand(1)->replaceNonMetadataUsesWith(newvalue);
          }
        }
        
        // inner loop bound -> outer loop bound * inner loop bound  
        //errs() << OuterBound->getContext() << "\n";
        errs() << "=============" << "\n";
        for (auto &I : *(InnerLoop->getHeader())) {
          errs() << "Inner Loop IR: " << I << "\n";
          // op: icmp == 53
          if (I.getOpcode() == 53) { // Hard Coded for simplicity.
            // replace the old loop bound with new values
            Value* oldvalue = (I.getOperand(1));
            Value* newvalue = ConstantInt::get(oldvalue->getType(), 220);
            I.getOperand(1)->replaceAllUsesWith(newvalue);
          }
        }
        /* 
        BasicBlock *InnerExitBlock = InnerLoop->getExitBlock();
        BasicBlock *InnerExitingBlock = InnerLoop->getExitingBlock();
        InnerExitingBlock->getTerminator()->eraseFromParent();
        BranchInst::Create(InnerExitBlock, InnerExitingBlock);
        DT->deleteEdge(InnerExitingBlock, InnerLoop->getHeader());
        */
        /*
        errs() << "==========================\n";
        errs() << *InnerExitBlock << "\n";
        errs() << "==========================\n";
        errs() << *InnerExitingBlock << "\n";
        */

        // mark the inner loop as a deleted one
        //LPM.markLoopAsDeleted(*InnerLoop);
        //SE->forgetLoop(L);
        //SE->forgetLoop(InnerLoop);
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
