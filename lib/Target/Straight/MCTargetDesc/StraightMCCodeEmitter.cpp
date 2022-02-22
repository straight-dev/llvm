//===-- StraightMCCodeEmitter.cpp - Convert Straight code to machine code -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the StraightMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/StraightMCTargetDesc.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/MC/MCExpr.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

namespace {

class StraightMCCodeEmitter : public MCCodeEmitter {
  const MCInstrInfo &MCII;
  const MCRegisterInfo &MRI;
  bool IsLittleEndian;

public:
  StraightMCCodeEmitter(const MCInstrInfo &mcii, const MCRegisterInfo &mri,
                   bool IsLittleEndian)
      : MCII(mcii), MRI(mri), IsLittleEndian(IsLittleEndian) {}
  StraightMCCodeEmitter(const StraightMCCodeEmitter &) = delete;
  void operator=(const StraightMCCodeEmitter &) = delete;
  ~StraightMCCodeEmitter() override = default;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  // getMachineOpValue - Return binary encoding of operand. If the machin
  // operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  uint64_t getMemoryOpValue(const MCInst &MI, unsigned Op,
                            SmallVectorImpl<MCFixup> &Fixups,
                            const MCSubtargetInfo &STI) const;

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

private:
  FeatureBitset computeAvailableFeatures(const FeatureBitset &FB) const;
  void verifyInstructionPredicates(const MCInst &MI,
                                   const FeatureBitset &AvailableFeatures) const;
};

} // end anonymous namespace

MCCodeEmitter *llvm::createStraightMCCodeEmitter(const MCInstrInfo &MCII,
                                            const MCRegisterInfo &MRI,
                                            MCContext &Ctx) {
  return new StraightMCCodeEmitter(MCII, MRI, true);
}

MCCodeEmitter *llvm::createStraightbeMCCodeEmitter(const MCInstrInfo &MCII,
                                              const MCRegisterInfo &MRI,
                                              MCContext &Ctx) {
  return new StraightMCCodeEmitter(MCII, MRI, false);
}

unsigned StraightMCCodeEmitter::getMachineOpValue(const MCInst &MI,
                                             const MCOperand &MO,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return MRI.getEncodingValue(MO.getReg());
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  assert(MO.isExpr());

  const MCExpr *Expr = MO.getExpr();

  assert(Expr->getKind() == MCExpr::SymbolRef);
#if 0
  if (MI.getOpcode() == Straight::JAL)
    // func call name
    Fixups.push_back(MCFixup::create(0, Expr, FK_SecRel_4));
  else if (MI.getOpcode() == Straight::LD_imm64)
    Fixups.push_back(MCFixup::create(0, Expr, FK_SecRel_8));
  else
    // bb label
    Fixups.push_back(MCFixup::create(0, Expr, FK_PCRel_2));
#endif
  return 0;
}

void StraightMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
}

// Encode Straight Memory Operand
[[maybe_unused]]
uint64_t StraightMCCodeEmitter::getMemoryOpValue(const MCInst &MI, unsigned Op,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  uint64_t Encoding;
  const MCOperand Op1 = MI.getOperand(1);
  assert(Op1.isReg() && "First operand is not register.");
  Encoding = MRI.getEncodingValue(Op1.getReg());
  Encoding <<= 16;
  MCOperand Op2 = MI.getOperand(2);
  assert(Op2.isImm() && "Second operand is not immediate.");
  Encoding |= Op2.getImm() & 0xffff;
  return Encoding;
}

#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "StraightGenMCCodeEmitter.inc"
