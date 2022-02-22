//===- StraightDisassembler.cpp - Disassembler for Straight ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is part of the Straight Disassembler.
//
//===----------------------------------------------------------------------===//

#include "Straight.h"
#include "StraightSubtarget.h"
#include "MCTargetDesc/StraightMCTargetDesc.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCFixedLenDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "bpf-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

/// A disassembler class for Straight.
class StraightDisassembler : public MCDisassembler {
public:
  StraightDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  ~StraightDisassembler() override = default;

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &VStream,
                              raw_ostream &CStream) const override;
};

} // end anonymous namespace

static MCDisassembler *createStraightDisassembler(const Target &T,
                                             const MCSubtargetInfo &STI,
                                             MCContext &Ctx) {
  return new StraightDisassembler(STI, Ctx);
}


extern "C" void LLVMInitializeStraightDisassembler() {
  // Register the disassembler.
  TargetRegistry::RegisterMCDisassembler(getTheStraightTarget(),
                                         createStraightDisassembler);
}

static const unsigned GPRDecoderTable[] = {
    Straight::R0,  Straight::R1,  Straight::R2,  Straight::R3,  Straight::R4,  Straight::R5,
    Straight::R6,  Straight::R7,  Straight::R8,  Straight::R9,  Straight::R10, Straight::R11};

static DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t /*Address*/,
                                           const void * /*Decoder*/) {
  if (RegNo > 11)
    return MCDisassembler::Fail;

  unsigned Reg = GPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus decodeMemoryOpValue(MCInst &Inst, unsigned Insn,
                                        uint64_t Address, const void *Decoder) {
  unsigned Register = (Insn >> 16) & 0xf;
  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[Register]));
  unsigned Offset = (Insn & 0xffff);
  Inst.addOperand(MCOperand::createImm(SignExtend32<16>(Offset)));

  return MCDisassembler::Success;
}

#include "StraightGenDisassemblerTables.inc"
static DecodeStatus readInstruction64(ArrayRef<uint8_t> Bytes, uint64_t Address,
                                      uint64_t &Size, uint64_t &Insn,
                                      bool IsLittleEndian) {
  uint64_t Lo, Hi;

  if (Bytes.size() < 8) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Size = 8;
  if (IsLittleEndian) {
    Hi = (Bytes[0] << 24) | (Bytes[1] << 16) | (Bytes[2] << 0) | (Bytes[3] << 8);
    Lo = (Bytes[4] << 0) | (Bytes[5] << 8) | (Bytes[6] << 16) | (Bytes[7] << 24);
  } else {
    Hi = (Bytes[0] << 24) | ((Bytes[1] & 0x0F) << 20) | ((Bytes[1] & 0xF0) << 12) |
         (Bytes[2] << 8) | (Bytes[3] << 0);
    Lo = (Bytes[4] << 24) | (Bytes[5] << 16) | (Bytes[6] << 8) | (Bytes[7] << 0);
  }
  Insn = Make_64(Hi, Lo);

  return MCDisassembler::Success;
}

DecodeStatus StraightDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                             ArrayRef<uint8_t> Bytes,
                                             uint64_t Address,
                                             raw_ostream &VStream,
                                             raw_ostream &CStream) const {
  bool IsLittleEndian = getContext().getAsmInfo()->isLittleEndian();
  uint64_t Insn, Hi;
  DecodeStatus Result;

  Result = readInstruction64(Bytes, Address, Size, Insn, IsLittleEndian);
  if (Result == MCDisassembler::Fail) return MCDisassembler::Fail;


  switch (Instr.getOpcode()) {
  }

  return Result;
}

typedef DecodeStatus (*DecodeFunc)(MCInst &MI, unsigned insn, uint64_t Address,
                                   const void *Decoder);
