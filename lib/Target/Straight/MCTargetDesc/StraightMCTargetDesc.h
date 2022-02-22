//===-- StraightMCTargetDesc.h - Straight Target Descriptions -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Straight specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Straight_MCTARGETDESC_StraightMCTARGETDESC_H
#define LLVM_LIB_TARGET_Straight_MCTARGETDESC_StraightMCTARGETDESC_H

#include "llvm/Config/config.h"
#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class StringRef;
class Target;
class Triple;
class raw_ostream;
class raw_pwrite_stream;

Target &getTheStraightleTarget();
Target &getTheStraightbeTarget();
Target &getTheStraightTarget();

MCCodeEmitter *createStraightMCCodeEmitter(const MCInstrInfo &MCII,
                                      const MCRegisterInfo &MRI,
                                      MCContext &Ctx);
MCCodeEmitter *createStraightbeMCCodeEmitter(const MCInstrInfo &MCII,
                                        const MCRegisterInfo &MRI,
                                        MCContext &Ctx);

MCAsmBackend *createStraightAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                  const MCRegisterInfo &MRI,
                                  const MCTargetOptions &Options);
MCAsmBackend *createStraightbeAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter> createStraightELFObjectWriter(uint8_t OSABI);
}

// Defines symbolic names for Straight registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "StraightGenRegisterInfo.inc"

// Defines symbolic names for the Straight instructions.
//
#define GET_INSTRINFO_ENUM
#include "StraightGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "StraightGenSubtargetInfo.inc"

#endif
