//=- StraightMachineFunctionInfo.h - STRAIGHT machine function info -----*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares STRAIGHT-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_STRAIGHT_STRAIGHTMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_STRAIGHT_STRAIGHTMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "StraightSubtarget.h"

#include <vector>

namespace llvm {

/// STRAIGHTMachineFunctionInfo - This class is derived from MachineFunctionInfo
/// and contains private STRAIGHT-specific information for each MachineFunction.
class StraightMachineFunctionInfo : public MachineFunctionInfo {
private:
  MachineFunction& MF;
  std::vector<int> fixedFrameIndex;
  int varArgStartFrameIndex = INT_MIN;

public:
  StraightMachineFunctionInfo(MachineFunction &MF) : MF(MF) {}

  void setVarArgFrameIndex(int Index) { fixedFrameIndex.push_back(Index); }
  void setVarArgStartFrameIndex(int Index) { varArgStartFrameIndex = Index; }
  int getVarArgStartFrameIndex() const { return varArgStartFrameIndex; }

  bool isVarArgFrameIndex(int Index) const { return std::find(fixedFrameIndex.begin(), fixedFrameIndex.end(), Index) != fixedFrameIndex.end(); }
  int64_t getCallerStackPointerOffset(int Index) const {
  // StraightISelLoweringのLowerFormalArgumentsで可変引数だった時、callerのSPからの相対距離になるので、そのオフセットを計算
    return isVarArgFrameIndex(Index) ?
      MF.getSubtarget<StraightSubtarget>().getFrameLowering()->stackSize(MF) : 0;
  }
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_STRAIGHT_RISCVMACHINEFUNCTIONINFO_H
