//===-- StraightSubtarget.cpp - Straight Subtarget Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Straight specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "StraightSubtarget.h"
#include "Straight.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "straight-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "StraightGenSubtargetInfo.inc"

void StraightSubtarget::anchor() {}

StraightSubtarget &StraightSubtarget::initializeSubtargetDependencies(StringRef CPU,
                                                            StringRef FS) {
  initializeEnvironment();
  initSubtargetFeatures(CPU, FS);
  return *this;
}

void StraightSubtarget::initializeEnvironment() {
}

void StraightSubtarget::initSubtargetFeatures(StringRef CPU, StringRef FS) {
  if (CPU == "probe")
    // CPU = sys::detail::getHostCPUNameForStraight();
    return;
  if (CPU == "generic" || CPU == "v1")
    return;
  if (CPU == "v2") {
    return;
  }
}

StraightSubtarget::StraightSubtarget(const Triple &TT, const StringRef &CPU,
                           const StringRef &FS, const TargetMachine &TM)
    : StraightGenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS), InstrInfo(),
      FrameLowering(initializeSubtargetDependencies(CPU, FS)),
      TLInfo(TM, *this) {}
