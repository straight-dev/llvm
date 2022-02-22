//===-- StraightRegisterInfo.cpp - Straight Register Information ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Straight implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "StraightRegisterInfo.h"
#include "Straight.h"
#include "StraightSubtarget.h"
#include "StraightOpTraits.h"
#include "StraightMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/Register.h"

#define GET_REGINFO_TARGET_DESC
#include "StraightGenRegisterInfo.inc"
using namespace llvm;

StraightRegisterInfo::StraightRegisterInfo()
  : StraightGenRegisterInfo(Straight::R0) {}

const MCPhysReg *
StraightRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

BitVector StraightRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  return Reserved;
}
namespace llvm {

class OffsetInfo {
  unsigned reg;
  int64_t constant;
  OffsetInfo(unsigned r, int64_t x) : reg(r), constant(x) {}
public:
  static constexpr struct _reg {} reg_tag {};
  static constexpr struct _imm {} imm_tag {};
  OffsetInfo(_reg, unsigned r) : reg(r), constant(0) {}
  OffsetInfo(int64_t x) : reg(Straight::ZeroReg), constant(x) {}
  OffsetInfo operator+ (OffsetInfo rhs) const {
    assert(isOnlyImm() || rhs.isOnlyImm());
    if (isOnlyImm()) { return OffsetInfo(rhs.reg, constant + rhs.constant); }
    return OffsetInfo( reg, constant + rhs.constant );
  }
  bool isOnlyImm() const { return reg == Straight::ZeroReg; }
  std::tuple<unsigned, int64_t, int64_t> getRegAndImmHiLo() const {
    assert(isInt<32>(constant) && "Too large frame index offset!");
    uint64_t Hi20 = SignExtend64<20>((constant + 0x800) >> 12);
    uint64_t Lo12 = SignExtend64<12>(constant);
    return { reg, Hi20, Lo12 };
  }
};

// レジスタregに入っているものが定数の可能性を再帰的に試し、
// Reg+Immの形に還元する
OffsetInfo interpretRegValue(MachineBasicBlock& MBB, unsigned reg) {
  const auto iter = std::find_if(MBB.instr_begin(), MBB.instr_end(),
    [reg](const auto& it) { return it.getNumOperands() > 0 && it.getOperand(0).isReg() && it.getOperand(0).getReg() == reg; });
  if (iter == MBB.instr_end()) {
    // このBasicBlock外からの数であり、おそらく変数
    // 本当は詳しく調べたほうがいいかもしれない
    // ZeroRegっていうこともある
    return OffsetInfo(OffsetInfo::reg_tag, reg);
  }
  switch ((*iter).getOpcode()) {
  case Straight::ADDi_64: return interpretRegValue(MBB, (*iter).getOperand(1).getReg()) + (*iter).getOperand(2).getImm();
  case Straight::LUi: return (*iter).getOperand(1).getImm() * (1ull << 12); // 回りくどい書き方なのは左シフトでのオーバーフロー（未定義動作）を防ぐため
  case Straight::ADD_64: {
    OffsetInfo src1 = interpretRegValue(MBB, (*iter).getOperand(1).getReg());
    OffsetInfo src2 = interpretRegValue(MBB, (*iter).getOperand(2).getReg());
    if (src1.isOnlyImm() || src2.isOnlyImm()) {
      return src1 + src2;
    } else {
      return OffsetInfo(OffsetInfo::reg_tag, reg);
    }
  }
  default: return OffsetInfo(OffsetInfo::reg_tag, reg);
  }
}

} // namespace llvm

void StraightRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
  int SPAdj, unsigned FIOperandNum,
  RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");

  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();

  if (MI.getOpcode() == Straight::LIFETIME_START) {
    MBB.remove(&*II);
    return;
  }
  if (MI.getOpcode() == Straight::LIFETIME_END) {
    MBB.remove(&*II);
    return;
  }

  // ここに来るのは、LD/ST命令か、スタック上のアドレスを計算する命令である
  unsigned opcode = II->getOpcode();
  assert((Straight::is_store(*II) || Straight::is_load(*II) || opcode == Straight::ADDi_64) && "Unexpected Instruction");
  auto *const STFI = MF.getInfo<StraightMachineFunctionInfo>();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  int64_t callerStackPointerOffset =
      STFI->getCallerStackPointerOffset(FrameIndex);
  int64_t FI_Offset = MF.getFrameInfo().getObjectOffset(FrameIndex) + getFramePointerOffset(MBB) - callerStackPointerOffset;

  OffsetInfo Offset = 0;
  if (MI.getOperand(FIOperandNum+1).isReg()) {
    Offset = interpretRegValue(MBB, MI.getOperand(FIOperandNum+1).getReg()) + FI_Offset;
  } else {
    Offset = MI.getOperand(FIOperandNum+1).getImm() + FI_Offset;
  }

  if (opcode == Straight::ADDi_64) {
    if (Offset.isOnlyImm()) {
      // SP+constant の生成
      auto [_, Hi20, Lo12] = Offset.getRegAndImmHiLo();

      // AUiSP immHi
      // ADDi [1], immLo    ← immLo==0の時は無駄
      unsigned newReg = getAUiSP(II, Hi20);
      MI.getOperand(FIOperandNum).ChangeToRegister(newReg, false);
      MI.getOperand(FIOperandNum+1).ChangeToImmediate(Lo12);
    } else {
      // SP+reg+constant の生成
      auto [reg, Hi20, Lo12] = Offset.getRegAndImmHiLo();

      // AUiSP ImmHi      ← ImmHi==0でも必要
      // ADD [1], [reg]
      // ADDi [1], ImmLo  ← 元あるADDi命令で値を生成する必要がある、immLo==0の時は無駄
      MachineRegisterInfo& MRI = MF.getRegInfo();
      unsigned newReg1 = getAUiSP(II, Hi20);
      unsigned newReg2 = MRI.createVirtualRegister(&Straight::GPRRegClass);
      MI.getOperand(FIOperandNum).ChangeToRegister(newReg2, false);
      MI.getOperand(FIOperandNum+1).ChangeToImmediate(Lo12);
      BuildMI(MBB, std::next(II), DL, TII.get(Straight::ADD_64), newReg2)
        .addReg(newReg1)
        .addReg(reg);
    }
  } else {
    if (Offset.isOnlyImm()) {
      // SP+constant へのメモリアクセス
      auto [_, Hi20, Lo12] = Offset.getRegAndImmHiLo();
      if (Hi20 == 0) {
        // SPLD imm
        MI.getOperand(FIOperandNum).ChangeToRegister(Straight::SP, false);
        MI.getOperand(FIOperandNum+1).ChangeToImmediate(Lo12);
      } else {
        // AUiSP ImmHi
        // LD [1], ImmLo
        unsigned newReg = getAUiSP(II, Hi20);
        MI.getOperand(FIOperandNum).ChangeToRegister(newReg, false);
        MI.getOperand(FIOperandNum+1).ChangeToImmediate(Lo12);
      }
    } else {
      // SP+reg+constant へのメモリアクセス
      auto [reg, Hi20, Lo12] = Offset.getRegAndImmHiLo();

      // AUiSP ImmHi     ← ImmHi==0でも必要
      // ADD [1], [reg]
      // LD [1], immLo
      MachineRegisterInfo& MRI = MF.getRegInfo();
      unsigned newReg1 = getAUiSP(II, Hi20);
      unsigned newReg2 = MRI.createVirtualRegister(&Straight::GPRRegClass);
      BuildMI(MBB, II, DL, TII.get(Straight::ADD_64), newReg2)
        .addReg(newReg1)
        .addReg(reg);
      MI.getOperand(FIOperandNum).ChangeToRegister(newReg2, false);
      MI.getOperand(FIOperandNum+1).ChangeToImmediate(Lo12);
    }
  }
}

Register StraightRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return Straight::R10;
}

// SPADDiの結果とFramePointer(関数に入ってきた瞬間のSP)のずれを計算する
int64_t StraightRegisterInfo::getFramePointerOffset(const MachineBasicBlock &MBB) const {
  const int64_t stackSize = MBB.getParent()->getSubtarget<StraightSubtarget>().getFrameLowering()->stackSize(*MBB.getParent());
  for (MachineBasicBlock::const_instr_iterator iter = MBB.instr_begin(); iter != MBB.instr_end(); ++iter) {
    if ((*iter).getOpcode() == Straight::SPADDi) {
      if ((*iter).getOperand(0).getImm() > 0) {
        // エピローグのSPADDiの結果は元の位置に戻っているので、オフセットは0
        return 0;
      }
      // その他の位置ではFPはSPとスタックサイズ分だけずれてる
      return stackSize;
    }
  }
  return stackSize;
}

unsigned StraightRegisterInfo::getAUiSP(MachineBasicBlock::iterator II, int64_t offset) const {
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  DebugLoc DL = MI.getDebugLoc();

  if (auto it = std::find_if(MBB.instr_begin(), MBB.instr_end(), [offset](const MachineInstr& instr) { return instr.getOpcode() == Straight::AUiSP && instr.getOperand(1).getImm() == offset; }); it != MBB.instr_end()) {
    return (*it).getOperand(0).getReg();
  }
  unsigned newReg = MRI.createVirtualRegister(&Straight::GPRRegClass);
  BuildMI(MBB, II, DL, TII.get(Straight::AUiSP), newReg)
    .addImm(offset);
  return newReg;
}
