//===-- StraightMCTargetDesc.cpp - Straight Target Descriptions ---------------------===//
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

#include "MCTargetDesc/StraightMCTargetDesc.h"
#include "Straight.h"
#include "InstPrinter/StraightInstPrinter.h"
#include "MCTargetDesc/StraightMCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "StraightGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "StraightGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "StraightGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createStraightMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitStraightMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createStraightMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitStraightMCRegisterInfo(X, Straight::R11 /* RAReg doesn't exist */);
  return X;
}

static MCSubtargetInfo *createStraightMCSubtargetInfo(const Triple &TT,
                                                 StringRef CPU, StringRef FS) {
  std::string CPUName = std::string(CPU); // TODO: CPUNameが空文字列だった場合にデフォルト値を指定するべし
  return createStraightMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPUName, FS);
}

static MCStreamer *createStraightMCStreamer(const Triple &T, MCContext &Ctx,
                                       std::unique_ptr<MCAsmBackend> &&MAB,
                                       std::unique_ptr<MCObjectWriter> &&OW,
                                       std::unique_ptr<MCCodeEmitter> &&Emitter,
                                       bool RelaxAll) {
  return createELFStreamer(Ctx, std::move(MAB), std::move(OW), std::move(Emitter),
                           RelaxAll);
}

static MCInstPrinter *createStraightMCInstPrinter(const Triple &T,
                                             unsigned SyntaxVariant,
                                             const MCAsmInfo &MAI,
                                             const MCInstrInfo &MII,
                                             const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new StraightInstPrinter(MAI, MII, MRI);
  return nullptr;
}

extern "C" void LLVMInitializeStraightTargetMC() {
  for (Target *T :
       {&getTheStraightTarget()}) {
    // Register the MC asm info.
    RegisterMCAsmInfo<StraightMCAsmInfo> X(*T);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createStraightMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createStraightMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T,
                                            createStraightMCSubtargetInfo);

    // Register the object streamer
    TargetRegistry::RegisterELFStreamer(*T, createStraightMCStreamer);

    // Register the MCInstPrinter.
    TargetRegistry::RegisterMCInstPrinter(*T, createStraightMCInstPrinter);
  }


  if (sys::IsLittleEndianHost) {
    TargetRegistry::RegisterMCCodeEmitter(getTheStraightTarget(),
                                          createStraightMCCodeEmitter);
    TargetRegistry::RegisterMCAsmBackend(getTheStraightTarget(),
                                         createStraightAsmBackend);
  } else {
    TargetRegistry::RegisterMCCodeEmitter(getTheStraightTarget(),
                                          createStraightbeMCCodeEmitter);
    TargetRegistry::RegisterMCAsmBackend(getTheStraightTarget(),
                                         createStraightbeAsmBackend);
  }
}
