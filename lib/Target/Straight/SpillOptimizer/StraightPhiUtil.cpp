#include "SpillOptimizer/StraightPhiUtil.h"

#include <iostream>
#include <string>

#include "StraightRegisterInfo.h"
#include "MCTargetDesc/StraightMCTargetDesc.h"

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#include "llvm/ADT/STLExtras.h"

namespace llvm {

namespace StraightSpillOptimizer {

LLVMBasicBlockID make_BasicBlockID(const llvm::MachineBasicBlock &basic_block) {
  assert(basic_block.getNumber() >= 0);
  return static_cast<LLVMBasicBlockID>(basic_block.getNumber());
}

const llvm::MachineBasicBlock &
getFirstJoinBasicBlock(const llvm::MachineBasicBlock &basic_block) {
  const auto *bb = &basic_block;
  while (bb->pred_size() == 1) {
    bb = *bb->pred_begin();
  }
  return *bb;
}

const llvm::MachineBasicBlock *
getFirstJoinBasicBlockWithoutFunctionCall(const llvm::MachineBasicBlock &basic_block) {
  const auto *bb = &basic_block;
  while (bb->pred_size() == 1 && !containsCall(**bb->pred_begin())) {
    bb = *bb->pred_begin();
  }
  if (bb->pred_size() == 1) {
    return nullptr;
  }
  else {
    return bb;
  }
}

bool containsCall(const llvm::MachineBasicBlock &basic_block) {
  return llvm::any_of(basic_block.instrs(),
                      [](const auto &instr) { return instr.isCall(); });
}

const llvm::MachineInstr *CreatePhi(llvm::MachineFunction &MF,
                                    llvm::MachineBasicBlock &basic_block,
                                    const llvm::MachineInstr &instr) {
  // std::cout << bbName(basic_block) << "に" <<
  // regName(instr.getOperand(0)) << "用のPHIを追加" << std::endl;

  llvm::MachineRegisterInfo &MRI = MF.getRegInfo();
  const llvm::TargetInstrInfo *const TII = MF.getSubtarget().getInstrInfo();
  const llvm::TargetRegisterClass *const NewRC =
      MRI.getRegClass(instr.getOperand(0).getReg());
  const auto newReg = MRI.createVirtualRegister(NewRC);
  llvm::MachineInstrBuilder MIB = llvm::BuildMI(
      basic_block, basic_block.instr_begin(), DebugLoc(),
      TII->get(llvm::Straight::PHI), newReg);

  for (auto *const incomingBB : basic_block.predecessors()) {
    MIB.addReg(instr.getOperand(0).getReg()).addMBB(incomingBB);
  }
  const auto *const new_instr = MIB.getInstr();
  return new_instr;
}

const llvm::MachineInstr *CreatePhi(llvm::MachineFunction &MF,
                                    llvm::MachineBasicBlock &basic_block,
                                    const unsigned &reg) {
  // std::cout << bbName(basic_block) << "に" <<
  // regName(instr.getOperand(0).getReg()) << "用のPHIを追加" << std::endl;

  llvm::MachineRegisterInfo &MRI = MF.getRegInfo();
  const llvm::TargetInstrInfo *const TII = MF.getSubtarget().getInstrInfo();
  const llvm::TargetRegisterClass *const NewRC =
      MRI.getRegClass(reg);
  const auto newReg = MRI.createVirtualRegister(NewRC);
  llvm::MachineInstrBuilder MIB =
      llvm::BuildMI(basic_block, basic_block.instr_begin(), DebugLoc(),
                    TII->get(llvm::Straight::PHI), newReg);

  for (auto *const incomingBB : basic_block.predecessors()) {
    MIB.addReg(reg).addMBB(incomingBB);
  }
  const auto *const new_instr = MIB.getInstr();
  return new_instr;
}

std::string regName(llvm::Register lvreg) {
  if (isVirtualReg(lvreg)) {
    return "vreg" + std::to_string(static_cast<unsigned int>(lvreg) & 0x7fff'ffff);
  } else if (isArgReg(lvreg)) {
    switch (static_cast<unsigned int>(lvreg)) {
    case llvm::Straight::ARG0:
      return "ARG0";
    case llvm::Straight::ARG1:
      return "ARG1";
    case llvm::Straight::ARG2:
      return "ARG2";
    case llvm::Straight::ARG3:
      return "ARG3";
    case llvm::Straight::ARG4:
      return "ARG4";
    case llvm::Straight::ARG5:
      return "ARG5";
    case llvm::Straight::ARG6:
      return "ARG6";
    case llvm::Straight::ARG7:
      return "ARG7";
    case llvm::Straight::ARG8:
      return "ARG8";
    case llvm::Straight::ARG9:
      return "ARG9";
    case llvm::Straight::RETADDR:
      return "RETADDR";
    default:
      llvm_unreachable("");
    }
  } else {
    return "R" + std::to_string(static_cast<int>(lvreg));
  }
}

std::string regName(const llvm::MachineOperand &operand) {
  return regName(operand.getReg());
}
std::string bbName(const llvm::MachineBasicBlock &MB) {
  return "BB" + std::to_string(MB.getNumber());
}

LLVMBasicBlockID toID(const llvm::MachineBasicBlock &bb) {
  assert(bb.getNumber() != -1);
  const auto bb_id = make_BasicBlockID(bb);
  return bb_id;
}
LLVMVarReg toID(const llvm::MachineOperand &operand) {
  assert(isVarReg(operand));
  const auto lvreg = LLVMVarReg{(unsigned) operand.getReg()};
  return lvreg;
}

bool isZeroReg(const llvm::MachineOperand &operand) {
  return operand.getReg() == llvm::Straight::ZeroReg;
}
bool isVarReg(const llvm::MachineOperand &operand) {
  return operand.isReg() && !isZeroReg(operand);
}
bool isArgReg(const llvm::MachineOperand &operand) {
  return isArgReg(operand.getReg());
}
bool isRetReg(const llvm::MachineOperand &operand) {
  return isRetReg(operand.getReg());
}
bool isVirtualReg(const llvm::MachineOperand &operand){
  return isVirtualReg(operand.getReg());
}
bool isArgReg(const LLVMVarReg lvreg) {
  return isArgReg(static_cast<llvm::Register>(static_cast<unsigned>(lvreg)));
}
bool isRetReg(const LLVMVarReg lvreg) {
  return isRetReg(static_cast<llvm::Register>(static_cast<unsigned>(lvreg)));
}
bool isVirtualReg(const LLVMVarReg lvreg) {
  return isVirtualReg(static_cast<llvm::Register>(static_cast<unsigned>(lvreg)));
}

bool isArgReg(llvm::Register lvreg) {
  switch (static_cast<unsigned int>(lvreg)) {
  case llvm::Straight::ARG0:
  case llvm::Straight::ARG1:
  case llvm::Straight::ARG2:
  case llvm::Straight::ARG3:
  case llvm::Straight::ARG4:
  case llvm::Straight::ARG5:
  case llvm::Straight::ARG6:
  case llvm::Straight::ARG7:
  case llvm::Straight::ARG8:
  case llvm::Straight::ARG9:
  case llvm::Straight::ARG10:
  case llvm::Straight::ARG11:
  case llvm::Straight::ARG12:
  case llvm::Straight::ARG13:
  case llvm::Straight::ARG14:
  case llvm::Straight::ARG15:
  case llvm::Straight::ARG16:
  case llvm::Straight::ARG17:
  case llvm::Straight::ARG18:
  case llvm::Straight::ARG19:
    return true;
  case llvm::Straight::RETADDR:
    return true;
  default:
    return false;
  }
}
bool isRetReg(llvm::Register lvreg){
  switch (static_cast<unsigned int>(lvreg)) {
  case llvm::Straight::RET0:
  case llvm::Straight::RET1:
  case llvm::Straight::RET2:
  case llvm::Straight::RET3:
  case llvm::Straight::RET4:
  case llvm::Straight::RET5:
  case llvm::Straight::RET6:
  case llvm::Straight::RET7:
  case llvm::Straight::RET8:
  case llvm::Straight::RET9:
    return true;
  default:
    return false;
  }
}
bool isVirtualReg(llvm::Register lvreg){
  return llvm::Register::isVirtualRegister(static_cast<unsigned int>(lvreg));
}

unsigned int srcOperandBegin(const llvm::MachineInstr &instr) {
  if(
    instr.isConditionalBranch() ||
    instr.getOpcode() == llvm::Straight::JR ||
    instr.getOpcode() == llvm::Straight::ST_8 ||
    instr.getOpcode() == llvm::Straight::ST_16||
    instr.getOpcode() == llvm::Straight::ST_32||
    instr.getOpcode() == llvm::Straight::ST_f32||
    instr.getOpcode() == llvm::Straight::ST_f64||
    instr.getOpcode() == llvm::Straight::ST_64
  ) {
    return 0;
  } else {
    return 1;
  }
}

} // namespace StraightSpillOptimizer

} // namespace llvm
