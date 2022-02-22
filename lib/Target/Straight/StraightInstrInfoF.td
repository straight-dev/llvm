//===-- StraightInstrInfoF.td - single float instructions --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the STRAIGHT instructions,
// Single-Precision Floating-Point instruction set.
//
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Instruction class templates
//===----------------------------------------------------------------------===//

// 0b111はその時々の丸めモードに従うの意
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class FMA<bits<7> opA, bit opW, bits<3> opX, RegisterClass Ty, string opcodestr>
    : StraightInstFMAForm<opA, opW, opX, (outs Ty:$rd), (ins Ty:$rs1, Ty:$rs2, Ty:$rs3), opcodestr, "$rs1, $rs2, $rs3">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class FloatBinaryOp<bits<7> opA, bit opW, bits<3> opX, bits<2> opY, bits<5> opZ, RegisterClass Ty, string opcodestr>
    : StraightInstFloatForm<opA, opW, opX, opY, opZ, (outs Ty:$rd), (ins Ty:$rs1, Ty:$rs2), opcodestr, "$rs1, $rs2">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class FloatUnaryOp<bits<7> opA, bit opW, bits<3> opX, bits<2> opY, bits<5> opZ, RegisterClass Ty, string opcodestr>
    : StraightInstFloatForm<opA, opW, opX, opY, opZ, (outs Ty:$rd), (ins Ty:$rs1), opcodestr, "$rs1">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class FloatBinaryGPROp<bits<7> opA, bit opW, bits<3> opX, bits<2> opY, bits<5> opZ, RegisterClass Ty, string opcodestr>
    : StraightInstFloatForm<opA, opW, opX, opY, opZ, (outs GPR:$rd), (ins Ty:$rs1, Ty:$rs2), opcodestr, "$rs1, $rs2">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class FloatUnaryGPROp<bits<7> opA, bit opW, bits<3> opX, bits<2> opY, bits<5> opZ, RegisterClass Ty, string opcodestr>
    : StraightInstFloatForm<opA, opW, opX, opY, opZ, (outs GPR:$rd), (ins Ty:$rs1), opcodestr, "$rs1">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class FloatFromGPROp<bits<7> opA, bit opW, bits<3> opX, bits<2> opY, bits<5> opZ, RegisterClass Ty, string opcodestr>
    : StraightInstFloatForm<opA, opW, opX, opY, opZ, (outs Ty:$rd), (ins GPR:$rs1), opcodestr, "$rs1">;


//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//

def FMADD_32  : FMA<0b0011011, 0b0, 0b111, FPR, "FMADD.s">;
def FMSUB_32  : FMA<0b1011011, 0b0, 0b111, FPR, "FMSUB.s">;
def FNMSUB_32 : FMA<0b0111011, 0b0, 0b111, FPR, "FNMSUB.s">;
def FNMADD_32 : FMA<0b1111011, 0b0, 0b111, FPR, "FNMADD.s">;

def FADD_32 : FloatBinaryOp<0b1001111, 0b0, 0b111, 0b00, 0b00000, FPR, "FADD.s">;
def FSUB_32 : FloatBinaryOp<0b1001111, 0b0, 0b111, 0b00, 0b00001, FPR, "FSUB.s">;
def FMUL_32 : FloatBinaryOp<0b1001111, 0b0, 0b111, 0b00, 0b00010, FPR, "FMUL.s">;
def FDIV_32 : FloatBinaryOp<0b1001111, 0b0, 0b111, 0b00, 0b00011, FPR, "FDIV.s">;

def FSQRT_32 : FloatUnaryOp<0b1001111, 0b0, 0b111, 0b00, 0b00100, FPR, "FSQRT.s">;

def FSGNJ_32  : FloatBinaryOp<0b1001111, 0b0, 0b000, 0b00, 0b01000, FPR, "FSGNJ.s">;
def FSGNJN_32 : FloatBinaryOp<0b1001111, 0b0, 0b000, 0b00, 0b01001, FPR, "FSGNJN.s">;
def FSGNJX_32 : FloatBinaryOp<0b1001111, 0b0, 0b000, 0b00, 0b01010, FPR, "FSGNJX.s">;
def FMIN_32   : FloatBinaryOp<0b1001111, 0b0, 0b000, 0b00, 0b01100, FPR, "FMIN.s">;
def FMAX_32   : FloatBinaryOp<0b1001111, 0b0, 0b000, 0b00, 0b01101, FPR, "FMAX.s">;

def FCLASS_32 : FloatUnaryGPROp<0b1001111, 0b0, 0b111, 0b00, 0b10000, FPR, "FCLASS.s">;
def FEQ_32    : FloatBinaryGPROp<0b1001111, 0b0, 0b111, 0b00, 0b10001, FPR, "FEQ.s">;
def FLT_32    : FloatBinaryGPROp<0b1001111, 0b0, 0b111, 0b00, 0b10010, FPR, "FLT.s">;
def FLE_32    : FloatBinaryGPROp<0b1001111, 0b0, 0b111, 0b00, 0b10011, FPR, "FLE.s">;

def FCVT_s32_to_f32 : FloatFromGPROp <0b1001111, 0b0, 0b001, 0b00, 0b11000, FPR, "FCVT.s32.to.f32">;
def FCVT_u32_to_f32 : FloatFromGPROp <0b1001111, 0b0, 0b001, 0b00, 0b11001, FPR, "FCVT.u32.to.f32">;
def FCVT_f32_to_s32 : FloatUnaryGPROp<0b1001111, 0b0, 0b111, 0b00, 0b11010, FPR, "FCVT.f32.to.s32">;
def FCVT_f32_to_u32 : FloatUnaryGPROp<0b1001111, 0b0, 0b111, 0b00, 0b11011, FPR, "FCVT.f32.to.u32">;
def FCVT_s64_to_f32 : FloatFromGPROp <0b1001111, 0b0, 0b001, 0b00, 0b11100, FPR, "FCVT.s64.to.f32">;
def FCVT_u64_to_f32 : FloatFromGPROp <0b1001111, 0b0, 0b001, 0b00, 0b11101, FPR, "FCVT.u64.to.f32">;
def FCVT_f32_to_s64 : FloatUnaryGPROp<0b1001111, 0b0, 0b111, 0b00, 0b11110, FPR, "FCVT.f32.to.s64">;
def FCVT_f32_to_u64 : FloatUnaryGPROp<0b1001111, 0b0, 0b111, 0b00, 0b11111, FPR, "FCVT.f32.to.u64">;

def LD_f32   : Load<0b0001111, 0b110, 0b111, FPR, "LD.f32">;
def SPLD_f32 : Load<0b0001111, 0b100, 0b111, FPR, "SPLD.f32">;


//===----------------------------------------------------------------------===//
// Codegen patterns
//===----------------------------------------------------------------------===//

/// Generic pattern classes
class PatFPR<SDPatternOperator OpNode, StraightInstFloatForm Inst>
	: Pat<(OpNode FPR:$rs1, FPR:$rs2), (Inst FPR:$rs1, FPR:$rs2)>;

def : Pat<(fma       FPR:$rs1 ,       FPR:$rs2 ,       FPR:$rs3 ), (FMADD_32  FPR:$rs1, FPR:$rs2, FPR:$rs3)>;
def : Pat<(fma       FPR:$rs1 , (fneg FPR:$rs2),       FPR:$rs3 ), (FNMSUB_32 FPR:$rs1, FPR:$rs2, FPR:$rs3)>;
def : Pat<(fma (fneg FPR:$rs1),       FPR:$rs2 ,       FPR:$rs3 ), (FNMSUB_32 FPR:$rs1, FPR:$rs2, FPR:$rs3)>;
def : Pat<(fma (fneg FPR:$rs1), (fneg FPR:$rs2),       FPR:$rs3 ), (FMADD_32  FPR:$rs1, FPR:$rs2, FPR:$rs3)>;
def : Pat<(fma       FPR:$rs1 ,       FPR:$rs2 , (fneg FPR:$rs3)), (FMSUB_32  FPR:$rs1, FPR:$rs2, FPR:$rs3)>;
def : Pat<(fma       FPR:$rs1 , (fneg FPR:$rs2), (fneg FPR:$rs3)), (FNMADD_32 FPR:$rs1, FPR:$rs2, FPR:$rs3)>;
def : Pat<(fma (fneg FPR:$rs1),       FPR:$rs2 , (fneg FPR:$rs3)), (FNMADD_32 FPR:$rs1, FPR:$rs2, FPR:$rs3)>;
def : Pat<(fma (fneg FPR:$rs1), (fneg FPR:$rs2), (fneg FPR:$rs3)), (FMSUB_32  FPR:$rs1, FPR:$rs2, FPR:$rs3)>;

def : PatFPR<fadd, FADD_32>;
def : PatFPR<fsub, FSUB_32>;
def : PatFPR<fmul, FMUL_32>;
def : PatFPR<fdiv, FDIV_32>;

def : Pat<(fsqrt FPR:$rs1), (FSQRT_32 FPR:$rs1)>;

def : Pat<(fneg FPR:$rs1), (FSGNJN_32 FPR:$rs1, FPR:$rs1)>;
def : Pat<(fabs FPR:$rs1), (FSGNJX_32 FPR:$rs1, FPR:$rs1)>;
def : PatFPR<fcopysign, FSGNJ_32>;
def : Pat<(fcopysign FPR:$rs1, (fneg FPR:$rs2)), (FSGNJN_32 FPR:$rs1, FPR:$rs2)>;

def : PatFPR<fminnum, FMIN_32>;
def : PatFPR<fmaxnum, FMAX_32>;

def : PatFPR<seteq , FEQ_32>;
def : PatFPR<setoeq, FEQ_32>;
def : PatFPR<setlt , FLT_32>;
def : PatFPR<setolt, FLT_32>;
def : PatFPR<setle , FLE_32>;
def : PatFPR<setole, FLE_32>;

// Define pattern expansions for setcc operations which aren't directly
// handled by a RISC-V instruction and aren't expanded in the SelectionDAG
// Legalizer.

def : Pat<(setuo FPR:$rs1, FPR:$rs2), (SLTiu_64 (AND_64 (FEQ_32 FPR:$rs1, FPR:$rs2), (FEQ_32 FPR:$rs2, FPR:$rs1)), 1)>;

def : Pat<(fp_to_sint FPR:$rs1), (FCVT_f32_to_s64 FPR:$rs1)>;
def : Pat<(fp_to_uint FPR:$rs1), (FCVT_f32_to_u64 FPR:$rs1)>;
def : Pat<(sint_to_fp GPR:$rs1), (FCVT_s64_to_f32 GPR:$rs1)>;
def : Pat<(uint_to_fp GPR:$rs1), (FCVT_u64_to_f32 GPR:$rs1)>;


defm : PatLoad<load, LD_f32>;
defm : PatStore<store, ST_f32, FPR>;