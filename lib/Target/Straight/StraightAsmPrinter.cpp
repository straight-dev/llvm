//===-- StraightAsmPrinter.cpp - Straight LLVM assembly writer ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the Straight assembly language.
//
//===----------------------------------------------------------------------===//

#include "Straight.h"
#include "StraightInstrInfo.h"
#include "StraightMCInstLower.h"
#include "StraightTargetMachine.h"
#include "InstPrinter/StraightInstPrinter.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
class StraightAsmPrinter : public AsmPrinter {
public:
  explicit StraightAsmPrinter(TargetMachine &TM,
                         std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  StringRef getPassName() const override { return "Straight Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override;
};
} // namespace

void StraightAsmPrinter::emitInstruction(const MachineInstr *MI) {

  StraightMCInstLower MCInstLowering(OutContext, *this);

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  EmitToStreamer(*OutStreamer, TmpInst);
}

// Force static initialization.
extern "C" void LLVMInitializeStraightAsmPrinter() {
  RegisterAsmPrinter<StraightAsmPrinter> X(getTheStraightTarget());
}
