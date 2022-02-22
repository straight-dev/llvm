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
// Instructions
//===----------------------------------------------------------------------===//

def FMADD_64  : FMA<0b0011011, 0b1, 0b111, DPR, "FMADD.s">;
def FMSUB_64  : FMA<0b1011011, 0b1, 0b111, DPR, "FMSUB.s">;
def FNMSUB_64 : FMA<0b0111011, 0b1, 0b111, DPR, "FNMSUB.s">;
def FNMADD_64 : FMA<0b1111011, 0b1, 0b111, DPR, "FNMADD.s">;

def FADD_64 : FloatBinaryOp<0b1001111, 0b1, 0b111, 0b00, 0b00000, DPR, "FADD.s">;
def FSUB_64 : FloatBinaryOp<0b1001111, 0b1, 0b111, 0b00, 0b00001, DPR, "FSUB.s">;
def FMUL_64 : FloatBinaryOp<0b1001111, 0b1, 0b111, 0b00, 0b00010, DPR, "FMUL.s">;
def FDIV_64 : FloatBinaryOp<0b1001111, 0b1, 0b111, 0b00, 0b00011, DPR, "FDIV.s">;

def FSQRT_64 : FloatUnaryOp<0b1001111, 0b1, 0b111, 0b00, 0b00100, DPR, "FSQRT.s">;

def FSGNJ_64  : FloatBinaryOp<0b1001111, 0b1, 0b000, 0b00, 0b01000, DPR, "FSGNJ.s">;
def FSGNJN_64 : FloatBinaryOp<0b1001111, 0b1, 0b000, 0b00, 0b01001, DPR, "FSGNJN.s">;
def FSGNJX_64 : FloatBinaryOp<0b1001111, 0b1, 0b000, 0b00, 0b01010, DPR, "FSGNJX.s">;
def FMIN_64   : FloatBinaryOp<0b1001111, 0b1, 0b000, 0b00, 0b01100, DPR, "FMIN.s">;
def FMAX_64   : FloatBinaryOp<0b1001111, 0b1, 0b000, 0b00, 0b01101, DPR, "FMAX.s">;

def FCLASS_64 : FloatUnaryGPROp<0b1001111, 0b1, 0b111, 0b00, 0b10000, DPR, "FCLASS.s">;
def FEQ_64    : FloatBinaryGPROp<0b1001111, 0b1, 0b111, 0b00, 0b10001, DPR, "FEQ.s">;
def FLT_64    : FloatBinaryGPROp<0b1001111, 0b1, 0b111, 0b00, 0b10010, DPR, "FLT.s">;
def FLE_64    : FloatBinaryGPROp<0b1001111, 0b1, 0b111, 0b00, 0b10011, DPR, "FLE.s">;

def FCVT_s32_to_f64 : FloatFromGPROp <0b1001111, 0b1, 0b001, 0b00, 0b11000, DPR, "FCVT.s32.to.f64">;
def FCVT_u32_to_f64 : FloatFromGPROp <0b1001111, 0b1, 0b001, 0b00, 0b11001, DPR, "FCVT.u32.to.f64">;
def FCVT_f64_to_s32 : FloatUnaryGPROp<0b1001111, 0b1, 0b111, 0b00, 0b11010, DPR, "FCVT.f64.to.s32">;
def FCVT_f64_to_u32 : FloatUnaryGPROp<0b1001111, 0b1, 0b111, 0b00, 0b11011, DPR, "FCVT.f64.to.u32">;
def FCVT_s64_to_f64 : FloatFromGPROp <0b1001111, 0b1, 0b001, 0b00, 0b11100, DPR, "FCVT.s64.to.f64">;
def FCVT_u64_to_f64 : FloatFromGPROp <0b1001111, 0b1, 0b001, 0b00, 0b11101, DPR, "FCVT.u64.to.f64">;
def FCVT_f64_to_s64 : FloatUnaryGPROp<0b1001111, 0b1, 0b111, 0b00, 0b11110, DPR, "FCVT.f64.to.s64">;
def FCVT_f64_to_u64 : FloatUnaryGPROp<0b1001111, 0b1, 0b111, 0b00, 0b11111, DPR, "FCVT.f64.to.u64">;

def FCVT_f32_to_f64 : StraightInstFloatForm<0b1001111, 0b0, 0b000, 0b00, 0b10100, (outs DPR:$dst), (ins FPR:$src), "FCVT.f32.to.f64", "$dst, $src">;
def FCVT_f64_to_f32 : StraightInstFloatForm<0b1001111, 0b1, 0b111, 0b00, 0b10100, (outs FPR:$dst), (ins DPR:$src), "FCVT.f64.to.f32", "$dst, $src">;


//===----------------------------------------------------------------------===//
// Codegen patterns
//===----------------------------------------------------------------------===//

/// Generic pattern classes
class PatDPR<SDPatternOperator OpNode, StraightInstFloatForm Inst>
	: Pat<(OpNode DPR:$rs1, DPR:$rs2), (Inst DPR:$rs1, DPR:$rs2)>;

def : Pat<(fma       DPR:$rs1 ,       DPR:$rs2 ,       DPR:$rs3 ), (FMADD_64  DPR:$rs1, DPR:$rs2, DPR:$rs3)>;
def : Pat<(fma       DPR:$rs1 , (fneg DPR:$rs2),       DPR:$rs3 ), (FNMSUB_64 DPR:$rs1, DPR:$rs2, DPR:$rs3)>;
def : Pat<(fma (fneg DPR:$rs1),       DPR:$rs2 ,       DPR:$rs3 ), (FNMSUB_64 DPR:$rs1, DPR:$rs2, DPR:$rs3)>;
def : Pat<(fma (fneg DPR:$rs1), (fneg DPR:$rs2),       DPR:$rs3 ), (FMADD_64  DPR:$rs1, DPR:$rs2, DPR:$rs3)>;
def : Pat<(fma       DPR:$rs1 ,       DPR:$rs2 , (fneg DPR:$rs3)), (FMSUB_64  DPR:$rs1, DPR:$rs2, DPR:$rs3)>;
def : Pat<(fma       DPR:$rs1 , (fneg DPR:$rs2), (fneg DPR:$rs3)), (FNMADD_64 DPR:$rs1, DPR:$rs2, DPR:$rs3)>;
def : Pat<(fma (fneg DPR:$rs1),       DPR:$rs2 , (fneg DPR:$rs3)), (FNMADD_64 DPR:$rs1, DPR:$rs2, DPR:$rs3)>;
def : Pat<(fma (fneg DPR:$rs1), (fneg DPR:$rs2), (fneg DPR:$rs3)), (FMSUB_64  DPR:$rs1, DPR:$rs2, DPR:$rs3)>;

def : PatDPR<fadd, FADD_64>;
def : PatDPR<fsub, FSUB_64>;
def : PatDPR<fmul, FMUL_64>;
def : PatDPR<fdiv, FDIV_64>;

def : Pat<(fsqrt DPR:$rs1), (FSQRT_64 DPR:$rs1)>;

def : Pat<(fneg DPR:$rs1), (FSGNJN_64 DPR:$rs1, DPR:$rs1)>;
def : Pat<(fabs DPR:$rs1), (FSGNJX_64 DPR:$rs1, DPR:$rs1)>;
def : PatDPR<fcopysign, FSGNJ_64>;
def : Pat<(fcopysign DPR:$rs1, (fneg DPR:$rs2)), (FSGNJN_64 DPR:$rs1, DPR:$rs2)>;

def : PatDPR<fminnum, FMIN_64>;
def : PatDPR<fmaxnum, FMAX_64>;

def : PatDPR<seteq , FEQ_64>;
def : PatDPR<setoeq, FEQ_64>;
def : PatDPR<setlt , FLT_64>;
def : PatDPR<setolt, FLT_64>;
def : PatDPR<setle , FLE_64>;
def : PatDPR<setole, FLE_64>;

// Define pattern expansions for setcc operations which aren't directly
// handled by a RISC-V instruction and aren't expanded in the SelectionDAG
// Legalizer.

def : Pat<(setuo DPR:$rs1, DPR:$rs2), (SLTiu_64 (AND_64 (FEQ_64 DPR:$rs1, DPR:$rs2), (FEQ_64 DPR:$rs2, DPR:$rs1)), 1)>;

def : Pat<(fp_to_sint DPR:$rs1), (FCVT_f64_to_s64 DPR:$rs1)>;
def : Pat<(fp_to_uint DPR:$rs1), (FCVT_f64_to_u64 DPR:$rs1)>;
def : Pat<(sint_to_fp GPR:$rs1), (FCVT_s64_to_f64 GPR:$rs1)>;
def : Pat<(uint_to_fp GPR:$rs1), (FCVT_u64_to_f64 GPR:$rs1)>;

def : Pat<(fpextend FPR:$rs1), (FCVT_f32_to_f64 FPR:$rs1)>;
def : Pat<(fpround DPR:$rs1), (FCVT_f64_to_f32 DPR:$rs1)>;

defm : PatLoad<load, LD_f64>;
defm : PatStore<store, ST_f64, DPR>;
