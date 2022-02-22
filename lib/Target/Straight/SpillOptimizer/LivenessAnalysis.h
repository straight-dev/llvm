#ifndef LLVMSTRAIGHT_SPILLOPTIMIZER2_LIVENESSANALYSIS_H
#define LLVMSTRAIGHT_SPILLOPTIMIZER2_LIVENESSANALYSIS_H

#include <map>
#include <utility>
#include <vector>

#include "StraightPhiUtil.h"

namespace llvm {

namespace StraightSpillOptimizer {

class LivenessInfo {
  struct VarRegInfo {
    bool live;
    bool define;
    bool use;
  };
  std::map<std::pair<LLVMBasicBlockID, LLVMVarReg>, VarRegInfo> mat;
  std::vector<LLVMVarReg> all_virtual_registers;

  void setLive(const llvm::MachineBasicBlock &bb,
               const llvm::MachineOperand &operand);
  void setDefine(const llvm::MachineBasicBlock &bb,
                     const llvm::MachineOperand &operand);
  void setUse(const llvm::MachineBasicBlock &bb,
               const llvm::MachineOperand &operand);

  void recursiveSearchDefinition(const llvm::MachineBasicBlock &nowBB,
                                 const llvm::MachineOperand &lvreg);

  void recordDefinitionPosition(const llvm::MachineFunction &MF);
  void livenessAnalysis(const llvm::MachineFunction &MF);

public:
  explicit LivenessInfo(const llvm::MachineFunction &MF);
  bool isLive(LLVMBasicBlockID bb, LLVMVarReg lvreg) const;
  bool isUsed(LLVMBasicBlockID bb, LLVMVarReg lvreg) const;
  bool isDefined(LLVMBasicBlockID bb, LLVMVarReg lvreg) const;
  const auto &allVirtualRegisters() const { return all_virtual_registers; }
};

} // namespace StraightSpillOptimizer

} // namespace llvm
#endif // LLVMSTRAIGHT_SPILLOPTIMIZER2_LIVENESSANALYSIS_H
