//===-- StraightInstrInfo.td - Target Description for Straight Target -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the Straight instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "StraightInstrFormats.td"


//===----------------------------------------------------------------------===//
// STRAIGHT specific DAG Nodes.
//===----------------------------------------------------------------------===//

// 呼び先関数アドレス×1入力, 出力なし
def StraightCallSDT : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;
// グローバルアドレスのラッパ
def StraightGlobalAddressSDT : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisPtrTy<0>]>;
// 浮動小数点即値、出力は浮動小数点数型、入力はi64
def StraightConstantFPSDT : SDTypeProfile<1, 1, [SDTCisVT<1, i64>]>;

// 5入力1出力、一番目と二番目は比較の入力だから同じ型、
// 零番目は出力で、四番目と五番目のどちらかを選ぶので同じ型、
// 三番目は条件オペレータ
def SDT_RISCVSelectCC     : SDTypeProfile<1, 5, [SDTCisSameAs<1, 2>,
                                                 SDTCisSameAs<0, 4>,
                                                 SDTCisSameAs<4, 5>]>;
// 浮動小数点数を整数値として解釈、出力はi64、入力はf32
def StraightBitCastSDT    : SDTypeProfile<1, 1, [SDTCisVT<1, f32>]>;

// The target-specific pre-isel opcode
// 命令選択の前、DAGで取り扱っている期間はStraightISDを使う必要がある
// ISDとInstructionは別々のenumであって互換性はない

def StraightCall       : SDNode<"StraightISD::CALL", StraightCallSDT, [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;
def StraightRetFlag    : SDNode<"StraightISD::RET_FLAG", SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def StraightGlobal     : SDNode<"StraightISD::GLOBAL", StraightGlobalAddressSDT>;
def StraightConstantFP : SDNode<"StraightISD::CONSTANT_FP", StraightConstantFPSDT>;
def SelectCC           : SDNode<"StraightISD::SELECT_CC", SDT_RISCVSelectCC, [SDNPInGlue]>;
def StraightBitCastFtoI: SDNode<"StraightISD::BITCAST_FTOI", StraightBitCastSDT>;
def StraightBitCastItoF: SDNode<"StraightISD::BITCAST_ITOF", StraightConstantFPSDT>;

//===----------------------------------------------------------------------===//
// Operand and SDNode transformation definitions.
//===----------------------------------------------------------------------===//

def brtarget : Operand<OtherVT>;
def calltarget : Operand<iPTR>;

def shamt5     : Operand<i64>, ImmLeaf<i64, [{return isUInt<5>(Imm);}]>;
def shamt6     : Operand<i64>, ImmLeaf<i64, [{return isUInt<6>(Imm);}]>;
def simm12     : Operand<i64>, ImmLeaf<i64, [{return isInt<12>(Imm);}]>;
def simm32     :               ImmLeaf<i64, [{return isInt<32>(Imm);}]>;
def rv64imm32  :               ImmLeaf<i64, [{return isInt<20>((Imm + 0x800) >> 12);}]>;

// 命令フォーマット上必要な定義
def simm20 : Operand<i64>;

// アドレッシングモードの定義
// 複雑なので、自分で実装するパターンマッチング関数を要求する
// SelectAddrFIという関数をStraightDAGtoDAGISel内に実装する必要がある
def AddrFI  : ComplexPattern<i64, 1, "SelectAddrFI", [frameindex], []>;

// SDNodeXFormの定義
// immを受け取って何かしらのノードを返すということをしている
// ここで複雑なノードを返すこともできなくはないが、
// TableGenのコスト計算を惑わせてしまうためよくない

// LUiと合わせて使うため、即値の下12bitを抽出し、符号拡張する
def LO12Sext : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(SignExtend64<12>(N->getZExtValue()),
                                   SDLoc(N), N->getValueType(0));
}]>;

// ADDi, LD, ST と組み合わせて使う、LUiに与える即値を計算する
// 即値は-2048〜2047なので、単に上位bitを抽出するのではなく、丸めた値を抽出する
def HI20 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(SignExtend64<20>((N->getZExtValue()+0x800) >> 12),
                                   SDLoc(N), N->getValueType(0));
}]>;

// 64bit即値用
def TOP20 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(SignExtend64<20>((((N->getZExtValue()+0x80000000)>>32)+0x800) >> 12),
                                   SDLoc(N), N->getValueType(0));
}]>;
def TOP12 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(SignExtend64<12>((N->getZExtValue()+0x80000000)>>32),
                                   SDLoc(N), N->getValueType(0));
}]>;

//===----------------------------------------------------------------------===//
// Instruction Class Templates
//===----------------------------------------------------------------------===//

// 命令クラスのプログラム中での効果（ストアする、分岐である……）を定義

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class BranchCC<bits<6> opA, string opcodestr>
    : StraightInstRegRegImmForm<opA, (outs),
                               (ins GPR:$rs1, GPR:$rs2, brtarget:$brtarget),
                               opcodestr, "$rs1, $rs2, $brtarget"> {
  let isBranch = 1;
  let isTerminator = 1;
}

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in
class Load<bits<7> opA, bits<3> opB, bits<3>opC, RegisterClass LDTy, string opcodestr>
    : StraightInstOtherIntForm<opA, opB, opC, (outs LDTy:$rd),
	                        (ins GPR:$rs1, simm12:$imm12),
                            opcodestr, "$rd, ${imm12}(${rs1})">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
class Store<bits<6> opA, string opcodestr>
    : StraightInstRegRegImmForm<opA, (outs),
                                (ins GPR:$rs2, GPR:$rs1, simm12:$imm12),
                                opcodestr, "$rs2, ${imm12}(${rs1})">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in
class SPStore<bits<7> opA, bits<3> opB, bits<3> opC, string opcodestr>
    : StraightInstOtherIntForm<opA, opB, opC, (outs),
                                (ins GPR:$rs2, GPR:$rs1, simm12:$imm12),
                                opcodestr, "$rs2, ${imm12}(${rs1})">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class RegImm<bits<7> opA, bit opW, bits<3> opX, bits<2> opY, string opcodestr>
    : StraightInstRegImmForm<opA, opW, opX, opY, (outs GPR:$rd),
                             (ins GPR:$rs1, simm12:$imm12),
                             opcodestr, "$rd, $rs1, $imm12">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class RegReg<bits<7> opA, bit opW, bits<3> opX, bits<2> opY, bits<5> opZ, string opcodestr>
    : StraightInstRegRegForm<opA, opW, opX, opY, opZ, (outs GPR:$rd),
	                         (ins GPR:$rs1, GPR:$rs2),
                             opcodestr, "$rd, $rs1, $rs2">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class RegSh5<bits<7> opA, bit opW, bits<3> opX, bits<2> opY, bits<5> opZ, string opcodestr>
    : StraightInstRegShamt5Form<opA, opW, opX, opY, opZ, (outs GPR:$rd),
                             (ins GPR:$rs1, shamt5:$imm5),
                             opcodestr, "$rd, $rs1, $imm5">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class RegSh6<bits<7> opA, bit opW, bits<3> opX, bits<2> opY, bits<5> opZ, string opcodestr>
    : StraightInstRegShamt5Form<opA, opW, opX, opY, opZ, (outs GPR:$rd),
                             (ins GPR:$rs1, shamt6:$imm6),
                             opcodestr, "$rd, $rs1, $imm6">;

//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//

// isReMaterilizableフラグは、ある値が再生成可能かを示す
// これがないと、生成した値を後で使うからと言ってスピルしてしまったりする

let hasSideEffects = 0, isReMaterializable = 1, mayLoad = 0, mayStore = 0 in {
def LUi : StraightInstImm20Form<0b0001111, 0b011, 0b10, (outs GPR:$rd),
                                (ins simm20:$imm20),
                                "LUi", "$rd, $imm20">;

def AUiPC : StraightInstImm20Form<0b0001111, 0b011, 0b11, (outs GPR:$rd),
                                  (ins simm20:$imm20),
                                  "AUiPC", "$rd, $imm20">;

def AUiSP : StraightInstImm20Form<0b0001111, 0b111, 0b11, (outs GPR:$rd),
                                  (ins simm20:$imm20),
                                  "AUiSP", "$rd, $imm20">;

let isCall = 1, usesCustomInserter = 1 in
def JAL : StraightInstImm20Form<0b0001111, 0b011, 0b01, (outs GPR:$rd), (ins calltarget:$fun),
                  "JAL", "$rd, $fun">;

let isCall = 1, usesCustomInserter = 1 in
def JALR : StraightInstOtherIntForm<0b0001111, 0b001, 0b001, (outs GPR:$rd),
                   (ins GPR:$rs1, simm12:$imm12),
                   "JALR", "$rd, $rs1, $imm12">;
} // hasSideEffects = 0, mayLoad = 0, mayStore = 0

let isBarrier = 1, isBranch = 1, isTerminator = 1 in
def JMP : StraightInstImm20Form<0b0001111, 0b011, 0b00, (outs), (ins brtarget:$brtarget),
                                "J", "$brtarget">;

let isBarrier = 1, isReturn = 1, isTerminator = 1 in
def JR : StraightInstOtherIntForm<0b0001111, 0b001, 0b000, (outs), (ins GPR:$rs1, simm12:$imm12),
                               "JR", "$rs1, simm12:$imm12">;

def Beq  : BranchCC<0b000011, "Beq">;
def Bne  : BranchCC<0b100011, "Bne">;
def Blt  : BranchCC<0b010011, "Blt">;
def Bge  : BranchCC<0b110011, "Bge">;
def Bltu : BranchCC<0b001011, "Bltu">;
def Bgeu : BranchCC<0b101011, "Bgeu">;

def LD_8   : Load<0b0001111, 0b110, 0b000, GPR, "LD.8">;
def LD_16  : Load<0b0001111, 0b110, 0b001, GPR, "LD.16">;
def LD_32  : Load<0b0001111, 0b110, 0b010, GPR, "LD.32">;
def LD_64  : Load<0b0001111, 0b110, 0b011, GPR, "LD.64">;
def LD_8u  : Load<0b0001111, 0b110, 0b100, GPR, "LD.8u">;
def LD_16u : Load<0b0001111, 0b110, 0b101, GPR, "LD.16u">;
def LD_32u : Load<0b0001111, 0b110, 0b110, GPR, "LD.32u">;
def SPLD_8   : Load<0b0001111, 0b100, 0b000, GPR, "SPLD.8">;
def SPLD_16  : Load<0b0001111, 0b100, 0b001, GPR, "SPLD.16">;
def SPLD_32  : Load<0b0001111, 0b100, 0b010, GPR, "SPLD.32">;
def SPLD_64  : Load<0b0001111, 0b100, 0b011, GPR, "SPLD.64">;
def SPLD_8u  : Load<0b0001111, 0b100, 0b100, GPR, "SPLD.8u">;
def SPLD_16u : Load<0b0001111, 0b100, 0b101, GPR, "SPLD.16u">;
def SPLD_32u : Load<0b0001111, 0b100, 0b110, GPR, "SPLD.32u">;

def ST_8  : Store<0b000111, "ST.8">;
def ST_16 : Store<0b100111, "ST.16">;
def ST_32 : Store<0b010111, "ST.32">;
def ST_64 : Store<0b110111, "ST.64">;
def SPST_8  : SPStore<0b0001111, 0b101, 0b000, "SPST.8">;
def SPST_16 : SPStore<0b0001111, 0b101, 0b001, "SPST.16">;
def SPST_32 : SPStore<0b0001111, 0b101, 0b010, "SPST.32">;
def SPST_64 : SPStore<0b0001111, 0b101, 0b011, "SPST.64">;

// ゼロレジスタとのADDiの時だけ、再生成可能。
// 本当に再生成可能かはisReallyTriviallyRematerializableで判定。
// RISC-Vでの議論：https://reviews.llvm.org/D46182
let isReMaterializable = 1 in {
  def ADDi_32 : RegImm<0b1001111, 0b0, 0b000, 0b10, "ADDi.32">;
  def ADDi_64 : RegImm<0b1001111, 0b1, 0b000, 0b10, "ADDi.64">;
}

def SLTi_32  : RegImm<0b1001111, 0b0, 0b010, 0b10,         "SLTi.32">;
def SLTi_64  : RegImm<0b1001111, 0b1, 0b010, 0b10,         "SLTi.64">;
def SLTiu_32 : RegImm<0b1001111, 0b0, 0b011, 0b10,         "SLTiu.32">;
def SLTiu_64 : RegImm<0b1001111, 0b1, 0b011, 0b10,         "SLTiu.64">;
def XORi_32  : RegImm<0b1001111, 0b0, 0b100, 0b10,         "XORi.32">;
def XORi_64  : RegImm<0b1001111, 0b1, 0b100, 0b10,         "XORi.64">;
def ORi_32   : RegImm<0b1001111, 0b0, 0b110, 0b10,         "ORi.32">;
def ORi_64   : RegImm<0b1001111, 0b1, 0b110, 0b10,         "ORi.64">;
def ANDi_32  : RegImm<0b1001111, 0b0, 0b111, 0b10,         "ANDi.32">;
def ANDi_64  : RegImm<0b1001111, 0b1, 0b111, 0b10,         "ANDi.64">;
def SLLi_32  : RegSh5<0b1001111, 0b0, 0b001, 0b10, 0b00000, "SLLi.32">;
def SLLi_64  : RegSh6<0b1001111, 0b1, 0b001, 0b10, 0b00000, "SLLi.64">;
def SRLi_32  : RegSh5<0b1001111, 0b0, 0b101, 0b10, 0b00000, "SRLi.32">;
def SRLi_64  : RegSh6<0b1001111, 0b1, 0b101, 0b10, 0b00000, "SRLi.64">;
def SRAi_32  : RegSh5<0b1001111, 0b0, 0b101, 0b10, 0b01000, "SRAi.32">;
def SRAi_64  : RegSh6<0b1001111, 0b1, 0b101, 0b10, 0b01000, "SRAi.64">;

def ADD_32   : RegReg<0b1001111, 0b0, 0b000, 0b10, 0b00000, "ADD.32">;
def ADD_64   : RegReg<0b1001111, 0b1, 0b000, 0b10, 0b00000, "ADD.64">;
def SUB_32   : RegReg<0b1001111, 0b0, 0b000, 0b10, 0b01000, "SUB.32">;
def SUB_64   : RegReg<0b1001111, 0b1, 0b000, 0b10, 0b01000, "SUB.64">;
def SLL_32   : RegReg<0b1001111, 0b0, 0b001, 0b10, 0b00000, "SLL.32">;
def SLL_64   : RegReg<0b1001111, 0b1, 0b001, 0b10, 0b00000, "SLL.64">;
def SLT_32   : RegReg<0b1001111, 0b0, 0b010, 0b10, 0b00000, "SLT.32">;
def SLT_64   : RegReg<0b1001111, 0b1, 0b010, 0b10, 0b00000, "SLT.64">;
def SLTu_32  : RegReg<0b1001111, 0b0, 0b011, 0b10, 0b00000, "SLTu.32">;
def SLTu_64  : RegReg<0b1001111, 0b1, 0b011, 0b10, 0b00000, "SLTu.64">;
def XOR_32   : RegReg<0b1001111, 0b0, 0b100, 0b10, 0b00000, "XOR.32">;
def XOR_64   : RegReg<0b1001111, 0b1, 0b100, 0b10, 0b00000, "XOR.64">;
def SRL_32   : RegReg<0b1001111, 0b0, 0b101, 0b10, 0b00000, "SRL.32">;
def SRL_64   : RegReg<0b1001111, 0b1, 0b101, 0b10, 0b00000, "SRL.64">;
def SRA_32   : RegReg<0b1001111, 0b0, 0b101, 0b10, 0b01000, "SRA.32">;
def SRA_64   : RegReg<0b1001111, 0b1, 0b101, 0b10, 0b01000, "SRA.64">;
def OR_32    : RegReg<0b1001111, 0b0, 0b110, 0b10, 0b00000, "OR.32">;
def OR_64    : RegReg<0b1001111, 0b1, 0b110, 0b10, 0b00000, "OR.64">;
def AND_32   : RegReg<0b1001111, 0b0, 0b111, 0b10, 0b00000, "AND.32">;
def AND_64   : RegReg<0b1001111, 0b1, 0b111, 0b10, 0b00000, "AND.64">;

def MUL_32   : RegReg<0b1001111, 0b0, 0b000, 0b11, 0b00001, "MUL.32">;
def MUL_64   : RegReg<0b1001111, 0b1, 0b000, 0b11, 0b00001, "MUL.64">;
def MULH_64  : RegReg<0b1001111, 0b1, 0b001, 0b11, 0b00001, "MULH.64">;
def MULHsu_64: RegReg<0b1001111, 0b1, 0b010, 0b11, 0b00001, "MULHsu.64">;
def MULHu_64 : RegReg<0b1001111, 0b1, 0b011, 0b11, 0b00001, "MULHu.64">;
def DIV_32   : RegReg<0b1001111, 0b0, 0b100, 0b11, 0b00001, "DIV.32">;
def DIV_64   : RegReg<0b1001111, 0b1, 0b100, 0b11, 0b00001, "DIV.64">;
def DIVu_32  : RegReg<0b1001111, 0b0, 0b101, 0b11, 0b00001, "DIVu.32">;
def DIVu_64  : RegReg<0b1001111, 0b1, 0b101, 0b11, 0b00001, "DIVu.64">;
def REM_32   : RegReg<0b1001111, 0b0, 0b110, 0b11, 0b00001, "REM.32">;
def REM_64   : RegReg<0b1001111, 0b1, 0b110, 0b11, 0b00001, "REM.64">;
def REMu_32  : RegReg<0b1001111, 0b0, 0b111, 0b11, 0b00001, "REMu.32">;
def REMu_64  : RegReg<0b1001111, 0b1, 0b111, 0b11, 0b00001, "REMu.64">;

//===----------------------------------------------------------------------===//
// Pseudo-instructions and codegen patterns
//===----------------------------------------------------------------------===//

// 即値の生成

def : Pat<(simm12:$imm), (ADDi_64 ZeroReg, simm12:$imm)>;
def : Pat<(rv64imm32:$imm), (ADDi_64 (LUi (HI20 imm:$imm)), (LO12Sext imm:$imm))>;
def : Pat<(simm32:$imm), (ADDi_32 (LUi (HI20 imm:$imm)), (LO12Sext imm:$imm))>;
def : Pat<(imm:$imm), (ADD_64 (SLLi_64 (ADDi_64 (LUi (TOP20 imm:$imm)), (TOP12 imm:$imm)), 32),
                             (ADDi_32 (LUi (HI20 imm:$imm)), (LO12Sext imm:$imm)))>;

// ALU命令

class PatRegReg<SDPatternOperator OpNode, RegReg Inst>
    : Pat<(OpNode GPR:$rs1, GPR:$rs2), (Inst GPR:$rs1, GPR:$rs2)>;

class PatZvsReg<SDPatternOperator OpNode, RegReg Inst>
    : Pat<(OpNode 0, GPR:$rs2), (Inst ZeroReg, GPR:$rs2)>;


class PatRegImm<SDPatternOperator OpNode, RegImm Inst>
    : Pat<(OpNode GPR:$rs1, simm12:$imm12), (Inst GPR:$rs1, simm12:$imm12)>;

class PatRegSh6<SDPatternOperator OpNode, RegSh6 Inst>
    : Pat<(OpNode GPR:$rs1, shamt6:$imm6), (Inst GPR:$rs1, shamt6:$imm6)>;

def : PatRegReg<add   , ADD_64>;
def : PatRegImm<add   , ADDi_64>;
def : PatRegReg<sub   , SUB_64>;
def : PatZvsReg<sub   , SUB_64>;
def : PatRegReg<shl   , SLL_64>;
def : PatRegSh6<shl   , SLLi_64>;
def : PatRegReg<setlt , SLT_64>;
def : PatZvsReg<setlt , SLT_64>;
def : PatRegImm<setlt , SLTi_64>;
def : PatRegReg<setult, SLTu_64>;
def : PatZvsReg<setult, SLTu_64>;
def : PatRegImm<setult, SLTiu_64>;
def : PatRegReg<xor   , XOR_64>;
def : PatRegImm<xor   , XORi_64>;
def : PatRegReg<srl   , SRL_64>;
def : PatRegSh6<srl   , SRLi_64>;
def : PatRegReg<sra   , SRA_64>;
def : PatRegSh6<sra   , SRAi_64>;
def : PatRegReg<or    , OR_64>;
def : PatRegImm<or    , ORi_64>;
def : PatRegReg<and   , AND_64>;
def : PatRegImm<and   , ANDi_64>;

def : PatRegReg<mul   , MUL_64>;
def : PatRegReg<mulhs , MULH_64>;
def : PatRegReg<mulhu , MULHu_64>;
def : PatRegReg<sdiv  , DIV_64>;
def : PatRegReg<udiv  , DIVu_64>;
def : PatRegReg<srem  , REM_64>;
def : PatRegReg<urem  , REMu_64>;

// SLT, SLTUだけでは対応できないパターン
def : Pat<(seteq GPR:$rs1, GPR:$rs2), (SLTiu_64 (XOR_64 GPR:$rs1, GPR:$rs2), 1)>;
def : Pat<(seteq 0, GPR:$rs2), (SLTiu_64 GPR:$rs2, 1)>;
def : Pat<(seteq GPR:$rs1, 0), (SLTiu_64 GPR:$rs1, 1)>;
def : Pat<(setne GPR:$rs1, GPR:$rs2), (SLTu_64 ZeroReg, (XOR_64 GPR:$rs1, GPR:$rs2))>;
def : Pat<(setne 0, GPR:$rs2), (SLTu_64 ZeroReg, GPR:$rs2)>;
def : Pat<(setne GPR:$rs1, 0), (SLTu_64 ZeroReg, GPR:$rs1)>;
// ↑のゼロのパターンはないと(XOR $rs1 (ADDi ZeroReg 0))みたいな冗長ノードができてしまう
def : Pat<(setugt GPR:$rs1, GPR:$rs2), (SLTu_64 GPR:$rs2, GPR:$rs1)>;
def : Pat<(setuge GPR:$rs1, GPR:$rs2), (XORi_64 (SLTu_64 GPR:$rs1, GPR:$rs2), 1)>;
def : Pat<(setule GPR:$rs1, GPR:$rs2), (XORi_64 (SLTu_64 GPR:$rs2, GPR:$rs1), 1)>;
def : Pat<(setgt GPR:$rs1, GPR:$rs2), (SLT_64 GPR:$rs2, GPR:$rs1)>;
def : Pat<(setge GPR:$rs1, GPR:$rs2), (XORi_64 (SLT_64 GPR:$rs1, GPR:$rs2), 1)>;
def : Pat<(setle GPR:$rs1, GPR:$rs2), (XORi_64 (SLT_64 GPR:$rs2, GPR:$rs1), 1)>;

// 32-bit 命令
// 基本的にSExtInRegに変換されているので、それをとらえる

class PatRegReg32<SDPatternOperator OpNode, RegReg Inst>
    : Pat<(sra (shl (OpNode GPR:$rs1, GPR:$rs2), 32), 32), (Inst GPR:$rs1, GPR:$rs2)>;
class PatZvsReg32<SDPatternOperator OpNode, RegReg Inst>
    : Pat<(sra (shl (OpNode 0, GPR:$rs2), 32), 32), (Inst ZeroReg, GPR:$rs2)>;

class PatRegImm32<SDPatternOperator OpNode, RegImm Inst>
    : Pat<(sra (shl (OpNode GPR:$rs1, simm12:$imm12), 32), 32), (Inst GPR:$rs1, simm12:$imm12)>;

def : PatRegReg32<add   , ADD_32>;
def : PatRegImm32<add   , ADDi_32>;
def : PatRegReg32<sub   , SUB_32>;
def : PatZvsReg32<sub   , SUB_32>;
// XOR, AND, OR はあったら便利なことがたまにあるけれど、RISC-Vにはないっぽいので保留

/// Branches and jumps

def : Pat<(br bb:$brtarget), (JMP bb:$brtarget)>;
def : Pat<(StraightRetFlag), (JR RETADDR, 0)>;

multiclass PatBranchCC<PatFrag CondOp, BranchCC Inst> {
    def : Pat<(brcond (i64 (CondOp GPR:$rs1, GPR:$rs2)), bb:$brtarget),
	      (Inst GPR:$rs1, GPR:$rs2, bb:$brtarget)>;
    def : Pat<(brcond (i64 (CondOp 0, GPR:$rs2)), bb:$brtarget),
	      (Inst ZeroReg, GPR:$rs2, bb:$brtarget)>;
    def : Pat<(brcond (i64 (CondOp GPR:$rs1, 0)), bb:$brtarget),
	      (Inst GPR:$rs1, ZeroReg, bb:$brtarget)>;
}

// 条件がLE, GTの場合、レジスタを入れ替えて対応する
multiclass PatSwapBranchCC<PatFrag CondOp, BranchCC Inst> {
    def : Pat<(brcond (i64 (CondOp GPR:$rs1, GPR:$rs2)), bb:$brtarget),
	      (Inst GPR:$rs2, GPR:$rs1, bb:$brtarget)>;
    def : Pat<(brcond (i64 (CondOp 0, GPR:$rs2)), bb:$brtarget),
	      (Inst GPR:$rs2, ZeroReg, bb:$brtarget)>;
    def : Pat<(brcond (i64 (CondOp GPR:$rs1, 0)), bb:$brtarget),
	      (Inst ZeroReg, GPR:$rs1, bb:$brtarget)>;
}


// 条件が他の場所で計算される場合
def : Pat<(brcond GPR:$cond, bb:$brtarget), (Bne GPR:$cond, ZeroReg, bb:$brtarget)>;
def : Pat<(brcond (i64 (seteq (xor GPR:$src1, GPR:$src2), 0)), bb:$brtarget), (Beq GPR:$src1, GPR:$src2, bb:$brtarget)>;
def : Pat<(brcond (i64 (setne (xor GPR:$src1, GPR:$src2), 0)), bb:$brtarget), (Bne GPR:$src1, GPR:$src2, bb:$brtarget)>;


defm : PatBranchCC<seteq, Beq>;
defm : PatBranchCC<setne, Bne>;
defm : PatBranchCC<setlt, Blt>;
defm : PatBranchCC<setge, Bge>;
defm : PatBranchCC<setult, Bltu>;
defm : PatBranchCC<setuge, Bgeu>;

defm : PatSwapBranchCC<setgt, Blt>;
defm : PatSwapBranchCC<setle, Bge>;
defm : PatSwapBranchCC<setugt, Bltu>;
defm : PatSwapBranchCC<setule, Bgeu>;


def : Pat<(brind GPR:$rs1), (JR GPR:$rs1, 0)>;
def : Pat<(brind (add GPR:$rs1, simm12:$imm12)), (JR GPR:$rs1, simm12:$imm12)>;

def : Pat<(StraightCall (iPTR GPR:$rs1)), (JALR GPR:$rs1, 0)>;

def : Pat<(StraightCall tglobaladdr:$func), (JAL tglobaladdr:$func)>;
def : Pat<(StraightCall texternalsym:$func), (JAL texternalsym:$func)>;

/// Predicates

// オブジェクトのアラインメントを認識した場合、足し算がBitORに変換される。
// (lib/CodeGen/SelectionDAG/DAGCombiner.cpp 2056行目)
// そのままだと SP + 即値 OR 即値 になってもったいないので変換
// それ以前になぜかスタック上のオブジェクトのアライメントが勝手に8Byteに変わってしまうので
// 直さないと破滅的なことになる
// そういったOR命令を判別するプレディケート
def IsOrAdd: PatFrag<(ops node:$A, node:$B), (or node:$A, node:$B), [{
  return isOrEquivalentToAdd(N);
}]>;

/// Address generation

def : Pat<(IsOrAdd AddrFI:$rs1, GPR:$rs2), (ADDi_64 AddrFI:$rs1, GPR:$rs2)>;

/// Loads

multiclass PatLoad<PatFrag LoadOp, Load Inst> {
  def : Pat<(LoadOp GPR:$rs1), (Inst GPR:$rs1, 0)>;
  def : Pat<(LoadOp AddrFI:$rs1), (Inst AddrFI:$rs1, 0)>;
  def : Pat<(LoadOp (add GPR:$rs1, simm12:$imm12)),
            (Inst GPR:$rs1, simm12:$imm12)>;
  def : Pat<(LoadOp (add GPR:$rs1, simm32:$imm32)),
            (Inst (ADD_64 GPR:$rs1, (LUi (HI20 simm32:$imm32))), (LO12Sext simm32:$imm32))>;
  def : Pat<(LoadOp (add AddrFI:$rs1, GPR:$rs2)),
            (Inst AddrFI:$rs1, GPR:$rs2)>;
  def : Pat<(LoadOp (IsOrAdd AddrFI:$rs1, GPR:$rs2)),
            (Inst AddrFI:$rs1, GPR:$rs2)>;
}

defm : PatLoad<sextloadi8 , LD_8>;
defm : PatLoad< extloadi8 , LD_8>;
defm : PatLoad<zextloadi8 , LD_8u>;
defm : PatLoad<sextloadi16, LD_16>;
defm : PatLoad <extloadi16, LD_16>;
defm : PatLoad<zextloadi16, LD_16u>;
defm : PatLoad<sextloadi32, LD_32>;
defm : PatLoad< extloadi32, LD_32>;
defm : PatLoad<zextloadi32, LD_32u>;
defm : PatLoad<    load   , LD_64>;

/// Stores

multiclass PatStore<PatFrag StoreOp, Store Inst, RegisterClass StTy> {
  def : Pat<(StoreOp StTy:$rs2, GPR:$rs1), (Inst StTy:$rs2, GPR:$rs1, 0)>;
  def : Pat<(StoreOp 0, GPR:$rs1), (Inst ZeroReg, GPR:$rs1, 0)>;
  def : Pat<(StoreOp StTy:$rs2, AddrFI:$rs1), (Inst StTy:$rs2, AddrFI:$rs1, 0)>;
  def : Pat<(StoreOp 0, AddrFI:$rs1), (Inst ZeroReg, AddrFI:$rs1, 0)>;
  def : Pat<(StoreOp StTy:$rs2, (add GPR:$rs1, simm12:$imm12)),
            (Inst StTy:$rs2, GPR:$rs1, simm12:$imm12)>;
  def : Pat<(StoreOp 0, (add GPR:$rs1, simm12:$imm12)),
            (Inst ZeroReg, GPR:$rs1, simm12:$imm12)>;
  def : Pat<(StoreOp StTy:$rs2, (add GPR:$rs1, simm32:$imm32)),
            (Inst StTy:$rs2, (ADD_64 GPR:$rs1, (LUi (HI20 simm32:$imm32))), (LO12Sext simm32:$imm32))>;
  def : Pat<(StoreOp 0, (add GPR:$rs1, simm32:$imm32)),
            (Inst ZeroReg, (ADD_64 GPR:$rs1, (LUi (HI20 simm32:$imm32))), (LO12Sext simm32:$imm32))>;
// 本当はこんな形式ないけれど、StraightRegisterInfo::eliminateFrameIndex
// 関数内で最適化するためにはこの形式でマッチングさせると楽
// (FrameIndexを取り除くまでLD/STの即値はわからないため、その最適化を行う
// 時点でFrameIndexがLD/STのオペランドであった方がよい)
  def : Pat<(StoreOp StTy:$rs2, (add AddrFI:$rs1, GPR:$rs0)),
            (Inst StTy:$rs2, AddrFI:$rs1, GPR:$rs0)>;
  def : Pat<(StoreOp 0, (add AddrFI:$rs1, GPR:$rs0)),
            (Inst ZeroReg, AddrFI:$rs1, GPR:$rs0)>;
  def : Pat<(StoreOp StTy:$rs2, (IsOrAdd AddrFI:$rs1, GPR:$rs0)),
            (Inst StTy:$rs2, AddrFI:$rs1, GPR:$rs0)>;
  def : Pat<(StoreOp 0, (IsOrAdd AddrFI:$rs1, GPR:$rs0)),
            (Inst ZeroReg, AddrFI:$rs1, GPR:$rs0)>;
}

defm : PatStore<truncstorei8 , ST_8 , GPR>;
defm : PatStore<truncstorei16, ST_16, GPR>;
defm : PatStore<truncstorei32, ST_32, GPR>;
defm : PatStore<     store   , ST_64, GPR>;

// pseudo instructions

def RMOV        : StraightInstExactForm<0b00000000000000000010000000001111, (outs GPR:$dst), (ins GPR:$src), "RMOV $dst, $src", []>;
def BITCASTftoi : StraightInstExactForm<0b00000000000000000100000000001111, (outs GPR:$dst), (ins FPR:$src), "BITCASTftoi $dst, $src", [(set GPR:$dst, (bitconvert FPR:$src))]>;
def BITCASTitof : StraightInstExactForm<0b00000000000000000110000000001111, (outs FPR:$dst), (ins GPR:$src), "BITCASTitof $dst, $src", [(set FPR:$dst, (bitconvert GPR:$src))]>;
def BITCASTdtoi : StraightInstExactForm<0b00000000000000011110000000001111, (outs GPR:$dst), (ins DPR:$src), "BITCASTdtoi $dst, $src", [(set GPR:$dst, (bitconvert DPR:$src))]>;
def BITCASTitod : StraightInstExactForm<0b00000000000000100000000000001111, (outs DPR:$dst), (ins GPR:$src), "BITCASTitod $dst, $src", [(set DPR:$dst, (bitconvert GPR:$src))]>;
def IMPLICIT_RET_VALUE : StraightInstExactForm<0b00000000000000001000000000001111, (outs), (ins), "IMPLICIT_RET_VALUE", []>;
def IMPLICIT_RET_SPACE : StraightInstExactForm<0b00000000000000001010000000001111, (outs), (ins), "IMPLICIT_RET_SPACE", []>;
def IMPLICIT_ARG_VALUE : StraightInstExactForm<0b00000000000000001100000000001111, (outs), (ins), "IMPLICIT_ARG_VALUE", []>;
def IMPLICIT_RET_ADDR  : StraightInstExactForm<0b00000000000000001110000000001111, (outs), (ins), "IMPLICIT_RET_ADDR" , []>;
def Global : StraightInstExactForm<0b00000000000000010000000000001111, (outs GPR:$dst), (ins GPR:$src), "Global $dst, $src", [(set GPR:$dst, (StraightGlobal tglobaladdr:$src))]>;
def IMPLICIT_FALL_THROUGH : StraightInstExactForm<0b00000000000000010010000000001111, (outs), (ins), "IMPLICIT_FALL_THROUGH", []>;
def ST_f32 : StraightInstExactForm<0b00000000000000010100000000001111, (outs), (ins FPR:$rs2, GPR:$rs1, simm12:$imm), "ST_f32 $rs1, $rs2, $imm", []>;
def ST_f64 : StraightInstExactForm<0b00000000000000010110000000001111, (outs), (ins DPR:$rs2, GPR:$rs1, simm12:$imm), "ST_f64 $rs1, $rs2, $imm", []>;
def SPST_f32 : StraightInstExactForm<0b00000000000000011000000000001111, (outs), (ins FPR:$rs2, simm12:$imm), "ST_f32 $rs2, $imm", []>;
def SPST_f64 : StraightInstExactForm<0b00000000000000011010000000001111, (outs), (ins DPR:$rs2, simm12:$imm), "ST_f64 $rs2, $imm", []>;
def LD_f64 : StraightInstExactForm<0b00000000000000011100000000001111, (outs DPR:$dst), (ins GPR:$rs1, simm12:$imm), "LD_f64 $dst, $rs1, $imm", []>;

// not automatically selected instructions

def NOP    : StraightInstExactForm<0b00000000000000000000000000001111, (outs), (ins), "NOP" , []>;
def RPINC  : StraightInstExactForm<0b00000010000000000000000000001111, (outs), (ins simm20:$imm), "RPINC $imm", []>;
def SPADDi : StraightInstImm20Form<0b0001111, 0b111, 0b00, (outs), (ins simm20:$imm), "SPADDi", "$imm">;


/// select命令を変換する

let usesCustomInserter = 1 in
class SelectCC_rrirr<RegisterClass valty, RegisterClass cmpty, bits<32> opA>
    : StraightInstExactForm<opA, (outs valty:$dst),
             (ins cmpty:$lhs, cmpty:$rhs, i64imm:$imm, valty:$truev, valty:$falsev),
			 "SelectCC_rrirr [from RISC-V, pseudo instruction!]",
             [(set valty:$dst, (SelectCC cmpty:$lhs, cmpty:$rhs, (i64 imm:$imm), valty:$truev, valty:$falsev))]>;

def Select_GPR_Using_CC_GPR : SelectCC_rrirr<GPR, GPR, 0b00000000000001011110000000001111>;
def Select_GPR_Using_CC_FPR : SelectCC_rrirr<GPR, FPR, 0b00000000000010011110000000001111>;
def Select_GPR_Using_CC_DPR : SelectCC_rrirr<GPR, DPR, 0b00000000000011011110000000001111>;
def Select_FPR_Using_CC_GPR : SelectCC_rrirr<FPR, GPR, 0b00000000000100011110000000001111>;
def Select_FPR_Using_CC_FPR : SelectCC_rrirr<FPR, FPR, 0b00000000000101011110000000001111>;
def Select_FPR_Using_CC_DPR : SelectCC_rrirr<FPR, DPR, 0b00000000000110011110000000001111>;
def Select_DPR_Using_CC_GPR : SelectCC_rrirr<DPR, GPR, 0b00000000000111011110000000001111>;
def Select_DPR_Using_CC_FPR : SelectCC_rrirr<DPR, FPR, 0b00000000001000011110000000001111>;
def Select_DPR_Using_CC_DPR : SelectCC_rrirr<DPR, DPR, 0b00000000001001011110000000001111>;

/// ConstantFP

def : Pat<(StraightConstantFP GPR:$src), (BITCASTitof $src)>;
def : Pat<(StraightConstantFP GPR:$src), (BITCASTitod $src)>;

def : Pat<(SelectCC FPR:$lhs, (StraightConstantFP GPR:$src), (i64 imm:$imm), GPR:$truev, GPR:$falsev), (Select_GPR_Using_CC_FPR FPR:$lhs, (BITCASTitof GPR:$src), i64imm:$imm, GPR:$truev, GPR:$falsev)>;
def : Pat<(SelectCC DPR:$lhs, (StraightConstantFP GPR:$src), (i64 imm:$imm), GPR:$truev, GPR:$falsev), (Select_GPR_Using_CC_DPR DPR:$lhs, (BITCASTitod GPR:$src), i64imm:$imm, GPR:$truev, GPR:$falsev)>;
def : Pat<(SelectCC (StraightConstantFP GPR:$src), FPR:$rhs, (i64 imm:$imm), GPR:$truev, GPR:$falsev), (Select_GPR_Using_CC_FPR (BITCASTitof GPR:$src), FPR:$rhs, i64imm:$imm, GPR:$truev, GPR:$falsev)>;
def : Pat<(SelectCC (StraightConstantFP GPR:$src), DPR:$rhs, (i64 imm:$imm), GPR:$truev, GPR:$falsev), (Select_GPR_Using_CC_DPR (BITCASTitod GPR:$src), DPR:$rhs, i64imm:$imm, GPR:$truev, GPR:$falsev)>;
def : Pat<(SelectCC FPR:$lhs, (StraightConstantFP GPR:$src), (i64 imm:$imm), FPR:$truev, FPR:$falsev), (Select_FPR_Using_CC_FPR FPR:$lhs, (BITCASTitof GPR:$src), i64imm:$imm, FPR:$truev, FPR:$falsev)>;
def : Pat<(SelectCC DPR:$lhs, (StraightConstantFP GPR:$src), (i64 imm:$imm), FPR:$truev, FPR:$falsev), (Select_FPR_Using_CC_DPR DPR:$lhs, (BITCASTitod GPR:$src), i64imm:$imm, FPR:$truev, FPR:$falsev)>;
def : Pat<(SelectCC (StraightConstantFP GPR:$src), FPR:$rhs, (i64 imm:$imm), FPR:$truev, FPR:$falsev), (Select_FPR_Using_CC_FPR (BITCASTitof GPR:$src), FPR:$rhs, i64imm:$imm, FPR:$truev, FPR:$falsev)>;
def : Pat<(SelectCC (StraightConstantFP GPR:$src), DPR:$rhs, (i64 imm:$imm), FPR:$truev, FPR:$falsev), (Select_FPR_Using_CC_DPR (BITCASTitod GPR:$src), DPR:$rhs, i64imm:$imm, FPR:$truev, FPR:$falsev)>;
def : Pat<(SelectCC FPR:$lhs, (StraightConstantFP GPR:$src), (i64 imm:$imm), DPR:$truev, DPR:$falsev), (Select_DPR_Using_CC_FPR FPR:$lhs, (BITCASTitof GPR:$src), i64imm:$imm, DPR:$truev, DPR:$falsev)>;
def : Pat<(SelectCC DPR:$lhs, (StraightConstantFP GPR:$src), (i64 imm:$imm), DPR:$truev, DPR:$falsev), (Select_DPR_Using_CC_DPR DPR:$lhs, (BITCASTitod GPR:$src), i64imm:$imm, DPR:$truev, DPR:$falsev)>;
def : Pat<(SelectCC (StraightConstantFP GPR:$src), FPR:$rhs, (i64 imm:$imm), DPR:$truev, DPR:$falsev), (Select_DPR_Using_CC_FPR (BITCASTitof GPR:$src), FPR:$rhs, i64imm:$imm, DPR:$truev, DPR:$falsev)>;
def : Pat<(SelectCC (StraightConstantFP GPR:$src), DPR:$rhs, (i64 imm:$imm), DPR:$truev, DPR:$falsev), (Select_DPR_Using_CC_DPR (BITCASTitod GPR:$src), DPR:$rhs, i64imm:$imm, DPR:$truev, DPR:$falsev)>;

/// BITCAST_FTOI / BITCAST_ITOF

def : Pat<(StraightBitCastFtoI FPR:$src), (BITCASTftoi $src)>;
def : Pat<(StraightBitCastItoF GPR:$src), (BITCASTitof $src)>;


include "StraightInstrInfoF.td"
include "StraightInstrInfoD.td"
