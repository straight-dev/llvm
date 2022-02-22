#ifndef LLVMSTRAIGHT_SPILLOPTIMIZER2_LIVESPACEANALYSIS_H
#define LLVMSTRAIGHT_SPILLOPTIMIZER2_LIVESPACEANALYSIS_H

#include "SpillOptimizer/LivenessAnalysis.h"
#include "SpillOptimizer/StraightPhiUtil.h"

#include "llvm/CodeGen/MachineLoopInfo.h"

#include <map>
#include <utility>

namespace llvm {

namespace StraightSpillOptimizer {

// All     : このBasicBlockの全領域で確実に。
// Partial : このBasicBlockの途中からは確実に。
// None    : このBasicBlockで確実な部分はない。
enum class NotInRegister { None, Partial, All };

class LiveSpaceInfo {

  template <class T> class Map {
    std::map<std::pair<LLVMBasicBlockID, LLVMVarReg>, T> map;

  public:
    T &operator[](std::pair<LLVMBasicBlockID, LLVMVarReg>);
    T operator()(LLVMBasicBlockID bb, LLVMVarReg lvreg) const;
  };

  Map<NotInRegister> notInRegister;
  const LivenessInfo LI;

  template <NotInRegister>
  void record(const llvm::MachineBasicBlock &MBB, LLVMVarReg lvreg);

public:
  explicit LiveSpaceInfo(const llvm::MachineFunction &MF, const llvm::MachineLoopInfo &MLI);

  NotInRegister getNotInRegister(LLVMBasicBlockID bb, LLVMVarReg lvreg) const;
};

} // namespace StraightSpillOptimizer

} // namespace llvm

#endif // LLVMSTRAIGHT_SPILLOPTIMIZER2_LIVESPACEANALYSIS_H
