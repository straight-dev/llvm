//===-- StraightFrameLowering.cpp - Straight Frame Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Straight implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "StraightFrameLowering.h"
#include "StraightInstrInfo.h"
#include "StraightSubtarget.h"
#include "StraightMachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

bool StraightFrameLowering::hasFP(const MachineFunction &MF) const { return true; }

void StraightFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  BuildMI(MBB, *MBB.instr_begin(), MBB.instr_begin()->getDebugLoc(), TII.get(Straight::SPADDi))
    .addImm(-stackSize(MF));
}

void StraightFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  if (MBB.getFirstNonPHI()->getOpcode() == Straight::SPADDi) {
    // この関数唯一のBasicBlockなので、SPは動かさない
    MBB.remove(&*MBB.getFirstNonPHI());
    return;
  }

  BuildMI(MBB, MBB.getFirstNonPHI(), MBB.getFirstNonPHI()->getDebugLoc(), TII.get(Straight::SPADDi))
    .addImm(stackSize(MF));
}

void StraightFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                            BitVector &SavedRegs,
                                            RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
}

uint64_t StraightFrameLowering::stackSize(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const llvm::Align MaxAlign = MFI.getMaxAlign();
  const auto *const STFI = MF.getInfo<StraightMachineFunctionInfo>();

  int64_t LocalObjectSlotSize = 0;

  // Iterate over other objects.
  for (unsigned I = 0, E = MFI.getObjectIndexEnd(); I != E; ++I)
    LocalObjectSlotSize = alignTo(LocalObjectSlotSize + MFI.getObjectSize(I), MaxAlign);

  int64_t VarArgSlotSize = 0;
  int64_t PassedArgSlotSize = 0;
  for (int I = -1, E = -MFI.getNumFixedObjects(); I >= E; --I) {
    if (STFI->isVarArgFrameIndex(I))
      VarArgSlotSize = std::max(VarArgSlotSize, MFI.getObjectOffset(I) + MFI.getObjectSize(I));
    else
      PassedArgSlotSize += MFI.getObjectSize(I);
  }

  return alignTo(LocalObjectSlotSize + VarArgSlotSize, getStackAlignment());
}

