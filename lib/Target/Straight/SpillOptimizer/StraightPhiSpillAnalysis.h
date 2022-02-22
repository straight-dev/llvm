#ifndef LLVMSTRAIGHT_SPILLOPTIMIZER2_STRAIGHTPHISPILLANALYSIS_H
#define LLVMSTRAIGHT_SPILLOPTIMIZER2_STRAIGHTPHISPILLANALYSIS_H

#include "SpillOptimizer/LiveSpaceAnalysis.h"
#include "SpillOptimizer/LivenessAnalysis.h"
#include "SpillOptimizer/StraightPhiUtil.h"

#include "llvm/CodeGen/MachineLoopInfo.h"

#include <set>
#include <utility>

namespace llvm {

namespace StraightSpillOptimizer {

class PhiSpillAnalysis {
  using Set = std::set<std::pair<LLVMBasicBlockID, LLVMVarReg>>;

  Set needPhi;
  Set needSpillIn;
  Set needSpillOut;
  const llvm::MachineFunction &MF;
  const LivenessInfo LI;
  const LiveSpaceInfo LSI;

  bool needSpill(LLVMVarReg lvreg);

public:
  PhiSpillAnalysis(const llvm::MachineFunction &MF, const llvm::MachineLoopInfo &MLI);

  bool needsPhi(LLVMBasicBlockID bb, LLVMVarReg lvreg) const;
  bool needsSpillIn(LLVMBasicBlockID bb, LLVMVarReg lvreg) const;
  bool needsSpillOut(LLVMBasicBlockID bb, LLVMVarReg lvreg) const;

  const Set &eachNeedsSpillIn() const { return needSpillIn; }
};

} // namespace StraightSpillOptimizer

} // namespace llvm

#endif // LLVMSTRAIGHT_SPILLOPTIMIZER2_STRAIGHTPHISPILLANALYSIS_H
