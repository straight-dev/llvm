//===-- StraightOptimizedInstrSelect.td                               -----===//
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


def simm32hi20            :  ImmLeaf<i64, [{return isShiftedInt<20, 12>(Imm);}]>;
def simm64_sll20          :  ImmLeaf<i64, [{if(auto x = getShamtOfShiftedInt<20>(Imm)) { return *x > 12; } return false; }]>;
def lowermask             :  ImmLeaf<i64, [{return isMask_64(Imm);}]>;
def shifted20plus12       :  ImmLeaf<i64, [{return static_cast<bool>(getShamtOfShiftedInt<20>((Imm+0x800) & 0xFFFFFFFFFFFFF000ull));}]>;
def aloneMask             :  ImmLeaf<i64, [{return !isInt<12>(Imm) && isMask_64(Imm) && Node->hasOneUse();}]>;
def p32imm12              :  ImmLeaf<i64, [{return isShiftedInt<12, 32>(Imm);}]>;
def p48imm12              :  ImmLeaf<i64, [{return isShiftedInt<12, 48>(Imm);}]>;
def p32imm                :  ImmLeaf<i64, [{return isShiftedInt<32, 32>(Imm);}]>;
def p48imm                :  ImmLeaf<i64, [{return isShiftedInt<16, 48>(Imm);}]>;

def uimm12                :  ImmLeaf<i64, [{assert(isUInt<32>(Imm)); return isInt<12>(SignExtend64<32>(Imm));}]>;
def uimm12Negatable       :  ImmLeaf<i64, [{assert(isUInt<32>(Imm)); return isInt<12>(SignExtend64<32>(-Imm));}]>;
def u32imm                :  ImmLeaf<i64, [{assert(isUInt<32>(Imm)); return true;}]>;
def s16imm                :  ImmLeaf<i64, [{assert(isInt<16>(Imm)); return true;}]>;
def u16imm                :  ImmLeaf<i64, [{assert(isUInt<16>(Imm)); return true;}]>;

def shamt1to21            :  ImmLeaf<i64, [{return 1 <= Imm && Imm <= 21;}]>;
def shamt1to31            :  ImmLeaf<i64, [{return 1 <= Imm && Imm <= 31;}]>;
def shamt33to63           :  ImmLeaf<i64, [{return 33 <= Imm && Imm <= 63;}]>;
def zextimm11             :  ImmLeaf<i64, [{return isUInt<11>(Imm);}]>;


// LUiで乗せた20bit即値をシフトするパターンを使う最適化
def MASK20 : SDNodeXForm<imm, [{
  int64_t imm = (N->getZExtValue() + 0x800) & 0xFFFF'FFFF'FFFF'F000;
  auto shamt = getShamtOfShiftedInt<20>(imm);
  assert(shamt && *shamt > 12);
  return CurDAG->getTargetConstant(imm / (1ll<<*shamt), SDLoc(N), MVT::i64);
}]>;
def Shamt : SDNodeXForm<imm, [{
  int64_t imm = (N->getSExtValue() + 0x800) & 0xFFFF'FFFF'FFFF'F000;
  auto shamt = getShamtOfShiftedInt<20>(imm);
  assert(shamt && *shamt > 12);
  return CurDAG->getTargetConstant(*shamt - 12, SDLoc(N), MVT::i64);
}]>;

// 下Nビットだけ1が立っているパターンは-1を右シフトすれば作れる
def LowerMaskShamt : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(countLeadingZeros(N->getZExtValue()), SDLoc(N), N->getValueType(0));
}]>;

def U32toSImm : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(SignExtend64<32>(N->getZExtValue()), SDLoc(N), MVT::i64);
}]>;

def U32toSImmNegative : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(-SignExtend64<32>(N->getZExtValue()), SDLoc(N), MVT::i64);
}]>;

def ShamtPlus32 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getZExtValue()+32, SDLoc(N), MVT::i64);
}]>;

def ShamtComplement32 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(32-N->getZExtValue(), SDLoc(N), MVT::i64);
}]>;

def P4Imm : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getZExtValue() * (1ll<<4), SDLoc(N), MVT::i64);
}]>;

def Q32Imm : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getSExtValue() / (1ll<<32), SDLoc(N), MVT::i64);
}]>;

def Q32ImmHI20 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(SignExtend64<20>((N->getZExtValue() / (1ll<<32)+0x800) >> 12),
                                   SDLoc(N), N->getValueType(0));
}]>;

def Q32ImmLO12Sext : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(SignExtend64<12>(N->getZExtValue() / (1ll<<32)),
                                   SDLoc(N), N->getValueType(0));
}]>;

def Q36Imm : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getSExtValue() / (1ll<<36), SDLoc(N), MVT::i64);
}]>;

def Q48Imm : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(N->getSExtValue() / (1ll<<48), SDLoc(N), MVT::i64);
}]>;


// 即値の生成
def : Pat<(simm32hi20:$imm), (LUi (HI20 imm:$imm))>;
def : Pat<(simm64_sll20:$imm), (SLLi_64 (LUi (MASK20 imm:$imm)), (Shamt imm:$imm))>;
def : Pat<(lowermask:$imm), (SRLi_64 (ADDi_64 ZeroReg, -1), (LowerMaskShamt imm:$imm))>;
def : Pat<(shifted20plus12:$imm), (ADDi_64 (SLLi_64 (LUi (MASK20 imm:$imm)), (Shamt imm:$imm)), (LO12Sext imm:$imm))>;


// 単発 sext32to64 は一命令にできる
def : Pat<(sra (shl GPR:$rs1, 32), 32), (ADDi_32 GPR:$rs1, 0)>;
// ADDi_s32 は一命令にできる（部分演算可能だということに基づく最適化が走り、即値が2^32倍された形で渡される）
def : Pat<(sra (add (shl GPR:$rs1, 32), p32imm12:$imm), 32), (ADDi_32 GPR:$rs1, (Q32Imm p32imm12:$imm))>;
def : Pat<(sra (add (shl GPR:$rs1, 32), p32imm:$imm), 32), (ADDi_32 (ADD_64 GPR:$rs1, (LUi (Q32ImmHI20 p32imm:$imm))), (Q32ImmLO12Sext p32imm:$imm))>;
// ADDi_s16 は三命令になる（これも部分演算可能だということに基づく最適化が走っているので元に戻す）
def : Pat<(sra (add (shl GPR:$rs1, 48), p48imm12:$imm), 48), (SRAi_64 (SLLi_64 (ADDi_64 GPR:$rs1, (Q48Imm p48imm12:$imm)), 48), 48)>;
// 48bit左に寄せるのではなく、16bit左に寄せてsext32to64を掛けるという手もある、こうすると任意の16bit即値をLUiから持ってこれる
def : Pat<(sra (add (shl GPR:$rs1, 48), p48imm:$imm), 48),  (SRAi_64 (ADD_32 (SLLi_64 GPR:$rs1, 16), (LUi (Q36Imm p48imm:$imm))), 16)>;

// u32を取り扱うのはこのISAだと苦手、最適化できるときは最適化する

// u32で計算したやつでインデックスアクセスする場合に発生、マスクをシフトに変えるとシフトが三回連続になってしまうパターン。シフト命令を融合させる
def : Pat<(shl (and GPR:$src1, 0xFFFFFFFF), shamt1to31:$imm), (SRLi_64 (SLLi_64 GPR:$src1, 32), (ShamtComplement32 shamt1to31:$imm))>;
// マスクした後左シフトする無意味なパターン
def : Pat<(shl (and GPR:$src1, 0xFFFFFFFF), 32), (SLLi_64 GPR:$src1, 32)>;
def : Pat<(shl (and GPR:$src1, 0xFFFFFFFF), shamt33to63:$imm), (SLLi_64 GPR:$src1, shamt33to63:$imm)>;
// こっちは頻出ではないが、u32を右シフトするパターン
def : Pat<(srl (and GPR:$src1, 0xFFFFFFFF), shamt6:$imm), (SRLi_64 (SLLi_64 GPR:$src1, 32), (ShamtPlus32 shamt6:$imm))>;
// u32を右シフトするパターンで、マスクに最適化がかかったもの
def : Pat<(srl (and GPR:$src1, 0x80000000), 31), (SRLi_64 (SLLi_64 GPR:$src1, 32), 63)>;
def : Pat<(srl (and GPR:$src1, 0xC0000000), 30), (SRLi_64 (SLLi_64 GPR:$src1, 32), 62)>;
def : Pat<(srl (and GPR:$src1, 0xE0000000), 29), (SRLi_64 (SLLi_64 GPR:$src1, 32), 61)>;
def : Pat<(srl (and GPR:$src1, 0xF0000000), 28), (SRLi_64 (SLLi_64 GPR:$src1, 32), 60)>;
def : Pat<(srl (and GPR:$src1, 0xF8000000), 27), (SRLi_64 (SLLi_64 GPR:$src1, 32), 59)>;
def : Pat<(srl (and GPR:$src1, 0xFC000000), 26), (SRLi_64 (SLLi_64 GPR:$src1, 32), 58)>;
def : Pat<(srl (and GPR:$src1, 0xFE000000), 25), (SRLi_64 (SLLi_64 GPR:$src1, 32), 57)>;
def : Pat<(srl (and GPR:$src1, 0xFF000000), 24), (SRLi_64 (SLLi_64 GPR:$src1, 32), 56)>;
def : Pat<(srl (and GPR:$src1, 0xFF800000), 23), (SRLi_64 (SLLi_64 GPR:$src1, 32), 55)>;
def : Pat<(srl (and GPR:$src1, 0xFFC00000), 22), (SRLi_64 (SLLi_64 GPR:$src1, 32), 54)>;
def : Pat<(srl (and GPR:$src1, 0xFFE00000), 21), (SRLi_64 (SLLi_64 GPR:$src1, 32), 53)>;
def : Pat<(srl (and GPR:$src1, 0xFFF00000), 20), (SRLi_64 (SLLi_64 GPR:$src1, 32), 52)>;
def : Pat<(srl (and GPR:$src1, 0xFFF80000), 19), (SRLi_64 (SLLi_64 GPR:$src1, 32), 51)>;
def : Pat<(srl (and GPR:$src1, 0xFFFC0000), 18), (SRLi_64 (SLLi_64 GPR:$src1, 32), 50)>;
def : Pat<(srl (and GPR:$src1, 0xFFFE0000), 17), (SRLi_64 (SLLi_64 GPR:$src1, 32), 49)>;
def : Pat<(srl (and GPR:$src1, 0xFFFF0000), 16), (SRLi_64 (SLLi_64 GPR:$src1, 32), 48)>;
def : Pat<(srl (and GPR:$src1, 0xFFFF8000), 15), (SRLi_64 (SLLi_64 GPR:$src1, 32), 47)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFC000), 14), (SRLi_64 (SLLi_64 GPR:$src1, 32), 46)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFE000), 13), (SRLi_64 (SLLi_64 GPR:$src1, 32), 45)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFF000), 12), (SRLi_64 (SLLi_64 GPR:$src1, 32), 44)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFF800), 11), (SRLi_64 (SLLi_64 GPR:$src1, 32), 43)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFFC00), 10), (SRLi_64 (SLLi_64 GPR:$src1, 32), 42)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFFE00),  9), (SRLi_64 (SLLi_64 GPR:$src1, 32), 41)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFFF00),  8), (SRLi_64 (SLLi_64 GPR:$src1, 32), 40)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFFF80),  7), (SRLi_64 (SLLi_64 GPR:$src1, 32), 39)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFFFC0),  6), (SRLi_64 (SLLi_64 GPR:$src1, 32), 38)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFFFE0),  5), (SRLi_64 (SLLi_64 GPR:$src1, 32), 37)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFFFF0),  4), (SRLi_64 (SLLi_64 GPR:$src1, 32), 36)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFFFF8),  3), (SRLi_64 (SLLi_64 GPR:$src1, 32), 35)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFFFFC),  2), (SRLi_64 (SLLi_64 GPR:$src1, 32), 34)>;
def : Pat<(srl (and GPR:$src1, 0xFFFFFFFE),  1), (SRLi_64 (SLLi_64 GPR:$src1, 32), 33)>;
// 単発 zextXto64 は二命令にできる、もし複数回行われるならマスクを生成した方が得なので Node->hasOneUse() で確認する
def : Pat<(and GPR:$src1, aloneMask:$imm), (SRLi_64 (SLLi_64 GPR:$src1, (LowerMaskShamt aloneMask:$imm)), (LowerMaskShamt aloneMask:$imm))>;
// シフト量は下5-6ビットしか使わないのでu32にする必要はない
def : Pat<(shl GPR:$src, (and GPR:$shamt, 0xFFFFFFFF)), (SLL_64 GPR:$src, GPR:$shamt)>;
def : Pat<(srl GPR:$src, (and GPR:$shamt, 0xFFFFFFFF)), (SRL_64 GPR:$src, GPR:$shamt)>;
def : Pat<(sra GPR:$src, (and GPR:$shamt, 0xFFFFFFFF)), (SRA_64 GPR:$src, GPR:$shamt)>;

// u32で計算している途中に発生、ANDをとるのは一回でよい
def : Pat<(and (add (and GPR:$src1, 0xFFFFFFFF), (and GPR:$src2, 0xFFFFFFFF)), 0xFFFFFFFF), (SRLi_64 (SLLi_64 (ADD_64 GPR:$src1, GPR:$src2), 32), 32)>;
def : Pat<(and (add      GPR:$src1             , (and GPR:$src2, 0xFFFFFFFF)), 0xFFFFFFFF), (SRLi_64 (SLLi_64 (ADD_64 GPR:$src1, GPR:$src2), 32), 32)>;
def : Pat<(and (add (and GPR:$src1, 0xFFFFFFFF),      GPR:$src2             ), 0xFFFFFFFF), (SRLi_64 (SLLi_64 (ADD_64 GPR:$src1, GPR:$src2), 32), 32)>;
// u16で計算している途中に発生、ANDをとるのは一回でよい
def : Pat<(and (add (and GPR:$src1, 0xFFFF), (and GPR:$src2, 0xFFFF)), 0xFFFF), (SRLi_64 (SLLi_64 (ADD_64 GPR:$src1, GPR:$src2), 48), 48)>;
def : Pat<(and (add (and GPR:$src1, 0xFFFF),      GPR:$src2         ), 0xFFFF), (SRLi_64 (SLLi_64 (ADD_64 GPR:$src1, GPR:$src2), 48), 48)>;
def : Pat<(and (add      GPR:$src1         , (and GPR:$src2, 0xFFFF)), 0xFFFF), (SRLi_64 (SLLi_64 (ADD_64 GPR:$src1, GPR:$src2), 48), 48)>;

// ゼロ拡張後シフトしてからマスクをかけるパターン、最初のゼロ拡張は不要
def : Pat<(and (srl (and GPR:$src1, 0xFFFFFFFF), shamt1to21:$shamt), zextimm11:$imm), (ANDi_64 (SRLi_64 GPR:$src1, shamt1to21:$shamt), zextimm11:$imm)>;


// u32, s16, u16 の分岐命令最適化

// u32の比較、一見ゼロ拡張が必要そうに思えるが、符号拡張でも同等の結果となる
// GPR vs GPR の場合は両者を符号拡張でOK
// GPR vs 32bitImm の場合は LUi+ADDi で普通に即値を積めばOK（即値はすでに符号拡張済みなため GPR vs GPR のパターンを流用すると一命令無駄）
// GPR vs 12bitImm の場合は ADDi で普通に即値を積めばOK（同上）
// GPR vs 0 の場合は普通に零レジスタを使えばOK
multiclass PatBranchCCu32<PatFrag CondOp, BranchCC Inst> {
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFFFFFF), (and GPR:$src2, 0xFFFFFFFF))), bb:$brtarget), (Inst (ADDi_32 GPR:$src1, 0), (ADDi_32 GPR:$src2, 0), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFFFFFF), u32imm:$imm)), bb:$brtarget), (Inst (ADDi_32 GPR:$src1, 0), (ADDi_64 (LUi (HI20 imm:$imm)), (LO12Sext uimm12:$imm)), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFFFFFF), uimm12:$imm)), bb:$brtarget), (Inst (ADDi_32 GPR:$src1, 0), (ADDi_64 ZeroReg, (U32toSImm uimm12:$imm)), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFFFFFF), 0)), bb:$brtarget), (Inst (ADDi_32 GPR:$src1, 0), ZeroReg, bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp u32imm:$imm, (and GPR:$src2, 0xFFFFFFFF))), bb:$brtarget), (Inst (ADDi_64 (LUi (HI20 imm:$imm)), (LO12Sext uimm12:$imm)), (ADDi_32 GPR:$src2, 0), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp uimm12:$imm, (and GPR:$src2, 0xFFFFFFFF))), bb:$brtarget), (Inst (ADDi_64 ZeroReg, (U32toSImm uimm12:$imm)), (ADDi_32 GPR:$src2, 0), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp 0, (and GPR:$src2, 0xFFFFFFFF))), bb:$brtarget), (Inst ZeroReg, (ADDi_32 GPR:$src2, 0), bb:$brtarget)>;
}
multiclass PatSwapBranchCCu32<PatFrag CondOp, BranchCC Inst> {
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFFFFFF), (and GPR:$src2, 0xFFFFFFFF))), bb:$brtarget), (Inst (ADDi_32 GPR:$src2, 0), (ADDi_32 GPR:$src1, 0), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFFFFFF), u32imm:$imm)), bb:$brtarget), (Inst (ADDi_64 (LUi (HI20 imm:$imm)), (LO12Sext uimm12:$imm)), (ADDi_32 GPR:$src1, 0), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFFFFFF), uimm12:$imm)), bb:$brtarget), (Inst (ADDi_64 ZeroReg, (U32toSImm uimm12:$imm)), (ADDi_32 GPR:$src1, 0), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFFFFFF), 0)), bb:$brtarget), (Inst ZeroReg, (ADDi_32 GPR:$src1, 0), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp u32imm:$imm, (and GPR:$src2, 0xFFFFFFFF))), bb:$brtarget), (Inst (ADDi_32 GPR:$src2, 0), (ADDi_64 (LUi (HI20 imm:$imm)), (LO12Sext uimm12:$imm)), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp uimm12:$imm, (and GPR:$src2, 0xFFFFFFFF))), bb:$brtarget), (Inst (ADDi_32 GPR:$src2, 0), (ADDi_64 ZeroReg, (U32toSImm uimm12:$imm)), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp 0, (and GPR:$src2, 0xFFFFFFFF))), bb:$brtarget), (Inst (ADDi_32 GPR:$src2, 0), ZeroReg, bb:$brtarget)>;
}
// u32の等値比較ではさらに進んだ最適化が可能になることもある（ADDi.32やSUB.32の結果がゼロになるかで判断できるパターン）
multiclass PatBranchCCu32eq<PatFrag CondOp, BranchCC Inst> {
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFFFFFF), (and GPR:$src2, 0xFFFFFFFF))), bb:$brtarget), (Inst (SUB_32 GPR:$src1, GPR:$src2), ZeroReg, bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFFFFFF), uimm12Negatable:$imm)), bb:$brtarget), (Inst (ADDi_32 GPR:$src1, (U32toSImmNegative uimm12Negatable:$imm)), ZeroReg, bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp uimm12Negatable:$imm, (and GPR:$src1, 0xFFFFFFFF))), bb:$brtarget), (Inst (ADDi_32 GPR:$src1, (U32toSImmNegative uimm12Negatable:$imm)), ZeroReg, bb:$brtarget)>;
}

// s16の比較、16bit左に寄せて32bit値とした後符号拡張でOK、しかも即値はLUiが使えて完全に好きな値と比較できる
multiclass PatBranchCCs16<PatFrag CondOp, BranchCC Inst> {
       def : Pat<(brcond (i64 (CondOp (sra (shl GPR:$src1, 48), 48), (sra (shl GPR:$src2, 48), 48))), bb:$brtarget), (Inst (SLLi_32 GPR:$src1, 16), (SLLi_32 GPR:$src2, 16), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (sra (shl GPR:$src1, 48), 48), s16imm:$imm)), bb:$brtarget), (Inst (SLLi_32 GPR:$src1, 16), (LUi (P4Imm s16imm:$imm)), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (sra (shl GPR:$src1, 48), 48), 0)), bb:$brtarget), (Inst (SLLi_32 GPR:$src1, 16), ZeroReg, bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp s16imm:$imm, (sra (shl GPR:$src1, 48), 48))), bb:$brtarget), (Inst (LUi (P4Imm s16imm:$imm)), (SLLi_32 GPR:$src1, 16), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp 0, (sra (shl GPR:$src1, 48), 48))), bb:$brtarget), (Inst ZeroReg, (SLLi_32 GPR:$src1, 16), bb:$brtarget)>;
}
multiclass PatSwapBranchCCs16<PatFrag CondOp, BranchCC Inst> {
       def : Pat<(brcond (i64 (CondOp (sra (shl GPR:$src1, 48), 48), (sra (shl GPR:$src2, 48), 48))), bb:$brtarget), (Inst (SLLi_32 GPR:$src2, 16), (SLLi_32 GPR:$src1, 16), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (sra (shl GPR:$src1, 48), 48), s16imm:$imm)), bb:$brtarget), (Inst (LUi (P4Imm s16imm:$imm)), (SLLi_32 GPR:$src1, 16), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (sra (shl GPR:$src1, 48), 48), 0)), bb:$brtarget), (Inst ZeroReg, (SLLi_32 GPR:$src1, 16), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp s16imm:$imm, (sra (shl GPR:$src1, 48), 48))), bb:$brtarget), (Inst (SLLi_32 GPR:$src1, 16), (LUi (P4Imm s16imm:$imm)), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp 0, (sra (shl GPR:$src1, 48), 48))), bb:$brtarget), (Inst (SLLi_32 GPR:$src1, 16), ZeroReg, bb:$brtarget)>;
}


// u16の比較も同様
multiclass PatBranchCCu16<PatFrag CondOp, BranchCC Inst> {
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFF), (and GPR:$src2, 0xFFFF))), bb:$brtarget), (Inst (SLLi_32 GPR:$src1, 16), (SLLi_32 GPR:$src2, 16), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFF), u16imm:$imm)), bb:$brtarget), (Inst (SLLi_32 GPR:$src1, 16), (LUi (P4Imm u16imm:$imm)), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFF), 0)), bb:$brtarget), (Inst (SLLi_32 GPR:$src1, 16), ZeroReg, bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp u16imm:$imm, (and GPR:$src1, 0xFFFF))), bb:$brtarget), (Inst (LUi (P4Imm u16imm:$imm)), (SLLi_32 GPR:$src1, 16), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp 0, (and GPR:$src1, 0xFFFF))), bb:$brtarget), (Inst ZeroReg, (SLLi_32 GPR:$src1, 16), bb:$brtarget)>;
}
multiclass PatSwapBranchCCu16<PatFrag CondOp, BranchCC Inst> {
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFF), (and GPR:$src2, 0xFFFF))), bb:$brtarget), (Inst (SLLi_32 GPR:$src2, 16), (SLLi_32 GPR:$src1, 16), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFF), u16imm:$imm)), bb:$brtarget), (Inst (LUi (P4Imm u16imm:$imm)), (SLLi_32 GPR:$src1, 16), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp (and GPR:$src1, 0xFFFF), 0)), bb:$brtarget), (Inst ZeroReg, (SLLi_32 GPR:$src1, 16), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp u16imm:$imm, (and GPR:$src1, 0xFFFF))), bb:$brtarget), (Inst (SLLi_32 GPR:$src1, 16), (LUi (P4Imm u16imm:$imm)), bb:$brtarget)>;
       def : Pat<(brcond (i64 (CondOp 0, (and GPR:$src1, 0xFFFF))), bb:$brtarget), (Inst (SLLi_32 GPR:$src1, 16), ZeroReg, bb:$brtarget)>;
}



defm : PatBranchCCu32<seteq, Beq>;
defm : PatBranchCCu32<setne, Bne>;
defm : PatBranchCCu32<setult, Bltu>;
defm : PatBranchCCu32<setuge, Bgeu>;
defm : PatSwapBranchCCu32<setugt, Bltu>;
defm : PatSwapBranchCCu32<setule, Bgeu>;
defm : PatBranchCCu32eq<seteq, Beq>;
defm : PatBranchCCu32eq<setne, Bne>;

defm : PatBranchCCs16<seteq, Beq>;
defm : PatBranchCCs16<setne, Bne>;
defm : PatBranchCCs16<setlt, Blt>;
defm : PatBranchCCs16<setge, Bge>;
defm : PatSwapBranchCCs16<setgt, Blt>;
defm : PatSwapBranchCCs16<setle, Bge>;

defm : PatBranchCCu16<seteq, Beq>;
defm : PatBranchCCu16<setne, Bne>;
defm : PatBranchCCu16<setlt, Blt>;
defm : PatBranchCCu16<setge, Bge>;
defm : PatSwapBranchCCu16<setgt, Blt>;
defm : PatSwapBranchCCu16<setle, Bge>;
