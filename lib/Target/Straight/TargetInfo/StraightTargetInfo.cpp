//===-- StraightTargetInfo.cpp - Straight Target Implementation ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Straight.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

namespace llvm {
Target& getTheStraightTarget() {
  static Target TheStraightTarget;
  return TheStraightTarget;
}
} // namespace llvm

extern "C" void LLVMInitializeStraightTargetInfo() {
  TargetRegistry::RegisterTarget(getTheStraightTarget(), "straight", "Straight (host endian)", "StraightBackendName",
                                 [](Triple::ArchType) { return false; }, true);
}
