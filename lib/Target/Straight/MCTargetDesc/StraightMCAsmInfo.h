//===-- StraightMCAsmInfo.h - Straight asm properties -------------------*- C++ -*--====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the StraightMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Straight_MCTARGETDESC_StraightMCASMINFO_H
#define LLVM_LIB_TARGET_Straight_MCTARGETDESC_StraightMCASMINFO_H

#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
class Target;

class StraightMCAsmInfo : public MCAsmInfo {
public:
  explicit StraightMCAsmInfo(const Triple &TT, const MCTargetOptions &Options) {
    if (TT.getArch() == Triple::bpfeb)
      IsLittleEndian = false;

    PrivateGlobalPrefix = ".L";
    WeakRefDirective = "\t.weak\t";

    UsesELFSectionDirectiveForBSS = true;
    HasSingleParameterDotFile = false;
    HasDotTypeDotSizeDirective = false;

    SupportsDebugInformation = true;
    ExceptionsType = ExceptionHandling::DwarfCFI;
    MinInstAlignment = 8;

    // the default is 4 and it only affects dwarf elf output
    // so if not set correctly, the dwarf data will be
    // messed up in random places by 4 bytes. .debug_line
    // section will be parsable, but with odd offsets and
    // line numbers, etc.
    CodePointerSize = 8;
  }
};
}

#endif
