#include "SpillOptimizer/StraightPhiSpillAnalysis.h"

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"

#include "llvm/ADT/STLExtras.h"

#include "SpillOptimizer/LiveSpaceAnalysis.h"
#include "SpillOptimizer/LivenessAnalysis.h"

namespace llvm {

namespace StraightSpillOptimizer {

bool PhiSpillAnalysis::needSpill(LLVMVarReg lvreg) {
  for (const auto &basic_block : MF) {
    const auto bb = toID(basic_block);
    // 関数コールをまたいだ参照があるならば、(どこかで)スピルする必要がある
    if (!LI.isDefined(bb, lvreg) && LI.isLive(bb, lvreg)) {
      if (basic_block.pred_size() == 1) {
        const auto &pred_bb = **basic_block.pred_begin();
        if (containsCall(pred_bb)) {
          return true;
        }
      }
    }
  }
  return false;
}

PhiSpillAnalysis::PhiSpillAnalysis(const llvm::MachineFunction &MF, const llvm::MachineLoopInfo& MLI)
    : MF(MF), LI(MF), LSI(MF, MLI) {
  for (const auto lvreg : LI.allVirtualRegisters()) {
    for (const auto &basic_block : MF) {
      const auto bb = toID(basic_block);
      if (LI.isLive(bb, lvreg)) {
        [[maybe_unused]]
        const auto notInReg = LSI.getNotInRegister(bb, lvreg);
        const bool bornInThisBB = LI.isDefined(bb, lvreg);
        if (basic_block.pred_size() > 1 && !bornInThisBB && notInReg == NotInRegister::None) {
          needPhi.emplace(bb, lvreg);
        }
        if (notInReg == NotInRegister::Partial) {
          needSpillIn.emplace(bb, lvreg);
        }
        if (bornInThisBB && needSpill(lvreg)) {
          needSpillOut.emplace(bb, lvreg);
        }
      }
    }
  }
}

bool PhiSpillAnalysis::needsPhi(LLVMBasicBlockID bb, LLVMVarReg lvreg) const {
  return needPhi.find(std::make_pair(bb, lvreg)) != needPhi.end();
}
bool PhiSpillAnalysis::needsSpillIn(LLVMBasicBlockID bb,
                                    LLVMVarReg lvreg) const {
  return needSpillIn.find(std::make_pair(bb, lvreg)) != needSpillIn.end();
}
bool PhiSpillAnalysis::needsSpillOut(LLVMBasicBlockID bb,
                                     LLVMVarReg lvreg) const {
  return needSpillOut.find(std::make_pair(bb, lvreg)) != needSpillOut.end();
}

} // namespace StraightSpillOptimizer

} // namespace llvm
