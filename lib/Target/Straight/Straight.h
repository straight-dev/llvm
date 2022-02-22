//===-- Straight.h - Top-level interface for Straight representation ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Straight_Straight_H
#define LLVM_LIB_TARGET_Straight_Straight_H

#include "MCTargetDesc/StraightMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class StraightTargetMachine;

FunctionPass *createStraightISelDag(StraightTargetMachine &TM);
}

#endif
