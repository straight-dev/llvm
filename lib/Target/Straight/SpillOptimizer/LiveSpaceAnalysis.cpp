#include "SpillOptimizer/LiveSpaceAnalysis.h"

#include <algorithm>

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"

#include "llvm/ADT/STLExtras.h"

#include "SpillOptimizer/LivenessAnalysis.h"

namespace llvm {

namespace StraightSpillOptimizer {
template <class T>
T &LiveSpaceInfo::Map<T>::
operator[](std::pair<LLVMBasicBlockID, LLVMVarReg> p) {
  return map[p];
}
template <class T>
T LiveSpaceInfo::Map<T>::operator()(LLVMBasicBlockID bb,
                                    LLVMVarReg lvreg) const {
  const auto it = map.find(std::make_pair(bb, lvreg));
  if (it != map.end()) {
    return (*it).second;
  } else {
    return T::None;
  }
}

bool operator<(NotInRegister lhs, NotInRegister rhs) {
  return static_cast<int>(lhs) < static_cast<int>(rhs);
}

template <NotInRegister Status>
void LiveSpaceInfo::record(const llvm::MachineBasicBlock &MBB,
                           LLVMVarReg lvreg) {
  static_assert(Status == NotInRegister::All);
  if (LI.isLive(toID(MBB), lvreg) &&
      notInRegister(toID(MBB), lvreg) < NotInRegister::All) {
    if (LI.isUsed(toID(MBB), lvreg)) {
      notInRegister[{toID(MBB), lvreg}] = NotInRegister::Partial;
    } else {
      notInRegister[{toID(MBB), lvreg}] = NotInRegister::All;
      for (const auto *const succ_block : MBB.successors()) {
        record<NotInRegister::All>(*succ_block, lvreg);
      }
    }
  }
}

LiveSpaceInfo::LiveSpaceInfo(const llvm::MachineFunction &MF, const llvm::MachineLoopInfo &MLI) : LI(MF) {
  for (const auto &basic_block : MF) {
    const auto bb = toID(basic_block);
    const bool pred_bb_is_call_bb = basic_block.pred_size() == 1 &&
                                    containsCall(**basic_block.pred_begin());
    for (const auto lvreg : LI.allVirtualRegisters()) {
      if (!LI.isDefined(bb, lvreg) && pred_bb_is_call_bb) {
        record<NotInRegister::All>(basic_block, lvreg);
      }
    }
  }
}

NotInRegister LiveSpaceInfo::getNotInRegister(LLVMBasicBlockID bb,
                                              LLVMVarReg lvreg) const {
  return notInRegister(bb, lvreg);
}

} // namespace StraightSpillOptimizer

} // namespace llvm
