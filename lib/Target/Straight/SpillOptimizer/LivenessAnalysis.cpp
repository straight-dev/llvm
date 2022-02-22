#include "SpillOptimizer/LivenessAnalysis.h"

#include "MCTargetDesc/StraightMCTargetDesc.h"

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"

namespace llvm {

namespace StraightSpillOptimizer {

void LivenessInfo::setLive(const llvm::MachineBasicBlock &bb,
                           const llvm::MachineOperand &operand) {
  mat[std::make_pair(toID(bb), toID(operand))].live = true;
}
void LivenessInfo::setDefine(const llvm::MachineBasicBlock &bb,
                             const llvm::MachineOperand &operand) {
  if (!isArgReg(operand) && !isRetReg(operand)) {
    all_virtual_registers.push_back(toID(operand));
  }
  mat[std::make_pair(toID(bb), toID(operand))].define = true;
}
void LivenessInfo::setUse(const llvm::MachineBasicBlock &bb,
                          const llvm::MachineOperand &operand) {
  mat[std::make_pair(toID(bb), toID(operand))].use = true;
}
bool LivenessInfo::isLive(LLVMBasicBlockID bb, LLVMVarReg lvreg) const {
  if (const auto it = mat.find(std::make_pair(bb, lvreg)); it != mat.end()) {
    return (*it).second.live;
  } else {
    return false;
  }
}
bool LivenessInfo::isUsed(LLVMBasicBlockID bb, LLVMVarReg lvreg) const {
  if (const auto it = mat.find(std::make_pair(bb, lvreg)); it != mat.end()) {
    return (*it).second.use;
  } else {
    return false;
  }
}
bool LivenessInfo::isDefined(LLVMBasicBlockID bb, LLVMVarReg lvreg) const {
  if (const auto it = mat.find(std::make_pair(bb, lvreg)); it != mat.end()) {
    return (*it).second.define;
  } else {
    return false;
  }
}
void LivenessInfo::recursiveSearchDefinition(
    const llvm::MachineBasicBlock &nowBB, const llvm::MachineOperand &lvreg) {
  if (isLive(toID(nowBB), toID(lvreg))) {
    //もう生存していることがわかっているところに来た
    return;
  } else {
    setLive(nowBB, lvreg);
    for (const auto *const pred_block : nowBB.predecessors()) {
      recursiveSearchDefinition(*pred_block, lvreg);
    }
  }
}

void LivenessInfo::recordDefinitionPosition(const llvm::MachineFunction &MF) {
  for (const auto &basic_block : MF) {
    for (const auto &instruction : basic_block.instrs()) {
      // dest を定義する命令: born
      const auto &dest = instruction.getOperand(0);
      if (srcOperandBegin(instruction) > 0 && isVarReg(dest)) {
        setDefine(basic_block, dest);
        setLive(basic_block, dest);
      }
      // 暗黙に定義される名前付きレジスタをコピーする命令
      if (instruction.getOpcode() == llvm::Straight::COPY) {
        const auto &src = instruction.getOperand(1);
        if (isArgReg(src) || isRetReg(src)) {
          setDefine(basic_block, src);
          setLive(basic_block, src);
        }
      }
    }
  }
}

void LivenessInfo::livenessAnalysis(const llvm::MachineFunction &MF) {
  for (const auto &basic_block : MF) {
    for (const auto &instruction : basic_block.instrs()) {
      // オペランドとして使う命令: use
      if (instruction.isPHI()) {
        for (unsigned int i = 1; i < instruction.getNumOperands(); i += 2) {
          const auto &operand = instruction.getOperand(i);
          if (isVarReg(operand)) {
            // PHI命令の場合は、指定された方向に先に一つ辿っておく
            // (そうしないと全パスで生存していることになる)
            const auto &incoming_bb = *instruction.getOperand(i + 1).getMBB();
            // このBBの先頭のPHI命令で使用したのではなく、
			// 合流直前に実質的な使用命令があると考えると便利
            setUse(incoming_bb, operand);
            recursiveSearchDefinition(incoming_bb, operand);
          }
        }
      } else {
        for (unsigned int i = srcOperandBegin(instruction);
             i < instruction.getNumOperands(); ++i) {
          const auto &operand = instruction.getOperand(i);
          if (isVarReg(operand)) {
            setUse(basic_block, operand);
            recursiveSearchDefinition(basic_block, operand);
          }
        }
      }
    }
  }
}

LivenessInfo::LivenessInfo(const llvm::MachineFunction &MF) {
  recordDefinitionPosition(MF);
  livenessAnalysis(MF);
}

} // namespace StraightSpillOptimizer

} // namespace llvm
