//===-- StraightRegisterInfo.h - Straight Register Information Impl -------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_Straight_StraightREGISTERINFO_H
#define LLVM_LIB_TARGET_Straight_StraightREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/Register.h"

#define GET_REGINFO_HEADER
#include "StraightGenRegisterInfo.inc"

namespace llvm {

struct StraightRegisterInfo : public StraightGenRegisterInfo {

  StraightRegisterInfo();

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  void eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;

  int64_t getFramePointerOffset(const MachineBasicBlock &MBB) const;

  unsigned getAUiSP(MachineBasicBlock::iterator II, int64_t offset) const;
};
}

#endif
