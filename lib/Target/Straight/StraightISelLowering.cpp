//===-- StraightISelLowering.cpp - Straight DAG Lowering Implementation  ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Straight uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "StraightISelLowering.h"
#include "Straight.h"
#include "StraightSubtarget.h"
#include "StraightTargetMachine.h"
#include "StraightMachineFunctionInfo.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "straight-lower"

static void fail(const SDLoc &DL, SelectionDAG &DAG, const Twine &Msg) {
  MachineFunction &MF = DAG.getMachineFunction();
  DAG.getContext()->diagnose(
      DiagnosticInfoUnsupported(MF.getFunction(), Msg, DL.getDebugLoc()));
}

[[maybe_unused]]
static void fail(const SDLoc &DL, SelectionDAG &DAG, const char *Msg,
                 SDValue Val) {
  MachineFunction &MF = DAG.getMachineFunction();
  std::string Str;
  raw_string_ostream OS(Str);
  OS << Msg;
  Val->print(OS);
  OS.flush();
  DAG.getContext()->diagnose(
      DiagnosticInfoUnsupported(MF.getFunction(), Str, DL.getDebugLoc()));
}

StraightTargetLowering::StraightTargetLowering(const TargetMachine &TM,
                                     const StraightSubtarget &STI)
    : TargetLowering(TM) {

  // Set up the register classes.
  addRegisterClass(MVT::i64, &Straight::GPRRegClass);
  addRegisterClass(MVT::f32, &Straight::FPRRegClass);  // これがないとネイティブで浮動小数点数演算ができないCPUとみなされてライブラリコールになる
  addRegisterClass(MVT::f64, &Straight::DPRRegClass);  // これがないとネイティブで浮動小数点数演算ができないCPUとみなされてライブラリコールになる


  // Compute derived properties from the register classes
  computeRegisterProperties(STI.getRegisterInfo());

  //setStackPointerRegisterToSaveRestore(Straight::R11);

  // SelectionDAGでとらえきれなかったllvmISDに含まれる標準命令のハンドル方法を指定する

  // BR_CC(比較して分岐)命令は自前(Custom)でハンドリングする
  // こう宣言しておくと、StraightTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG)を
  // 呼び出してくれるので、その中で自分のISAを使ってその機能を実装する
  // 他はllvm側でうまくフォールバックしてくれるみたい


  // 浮動小数点数の即値をConstantPoolにせず、直接積むため。isFPImmLegal(){ return true; }なので適当に処理しないと残ってしまう
  setOperationAction(ISD::ConstantFP, MVT::f32, Custom);
  setOperationAction(ISD::ConstantFP, MVT::f64, Custom);

  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::BRIND, MVT::Other, Expand);

  setOperationAction(ISD::BR_CC, MVT::i8, Expand);
  setOperationAction(ISD::SETCC, MVT::i8, Expand);

  setOperationAction(ISD::BR_CC, MVT::i16, Expand);
  setOperationAction(ISD::SETCC, MVT::i16, Expand);

  setOperationAction(ISD::BR_CC, MVT::i32, Expand);
  setOperationAction(ISD::SETCC, MVT::i32, Expand);

  setOperationAction(ISD::BR_CC, MVT::i64, Expand);

  ISD::CondCode FPCCToExtend[] = {
    ISD::SETOGT, ISD::SETOGE, ISD::SETONE, ISD::SETO,   ISD::SETUEQ,
    ISD::SETUGT, ISD::SETUGE, ISD::SETULT, ISD::SETULE, ISD::SETUNE,
    ISD::SETGT,  ISD::SETGE,  ISD::SETNE};

  setOperationAction(ISD::FMINNUM, MVT::f32, Legal);
  setOperationAction(ISD::FMAXNUM, MVT::f32, Legal);
  for (auto CC : FPCCToExtend)
    setCondCodeAction(CC, MVT::f32, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::f32, Expand);
  setOperationAction(ISD::SELECT, MVT::f32, Custom);
  setOperationAction(ISD::BR_CC, MVT::f32, Expand);

  setOperationAction(ISD::FMINNUM, MVT::f64, Legal);
  setOperationAction(ISD::FMAXNUM, MVT::f64, Legal);
  for (auto CC : FPCCToExtend)
    setCondCodeAction(CC, MVT::f64, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::f64, Expand);
  setOperationAction(ISD::SELECT, MVT::f64, Custom);
  setOperationAction(ISD::BR_CC, MVT::f64, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::f64, MVT::f32, Expand);
  setTruncStoreAction(MVT::f64, MVT::f32, Expand);

  // setOperationAction(ISD::SELECT, MVT::i8, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::i8, Expand);
  // setOperationAction(ISD::SELECT, MVT::i16, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::i16, Expand);
  //setOperationAction(ISD::SELECT, MVT::i32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Expand);
  setOperationAction(ISD::SELECT, MVT::i64, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::i64, Expand);
  setOperationAction(ISD::SELECT, MVT::f32, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::f32, Expand);
  setOperationAction(ISD::SELECT, MVT::f64, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::f64, Expand);


  setOperationAction(ISD::GlobalAddress, MVT::i64, Custom);

  // RISC-Vと同じ
  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction(ISD::VAARG, MVT::Other, Expand);
  setOperationAction(ISD::VACOPY, MVT::Other, Expand);
  setOperationAction(ISD::VAEND, MVT::Other, Expand);

  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i64, Custom);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);

  setOperationAction(ISD::SDIVREM, MVT::i64, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i64, Expand);

  setOperationAction(ISD::UMUL_LOHI, MVT::i64, Expand);
  setOperationAction(ISD::SMUL_LOHI, MVT::i64, Expand);

  setOperationAction(ISD::ADDC, MVT::i64, Expand);
  setOperationAction(ISD::ADDE, MVT::i64, Expand);
  setOperationAction(ISD::SUBC, MVT::i64, Expand);
  setOperationAction(ISD::SUBE, MVT::i64, Expand);

  setOperationAction(ISD::ROTR, MVT::i64, Expand);
  setOperationAction(ISD::ROTL, MVT::i64, Expand);
  setOperationAction(ISD::SHL_PARTS, MVT::i64, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i64, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i64, Expand);

  setOperationAction(ISD::BSWAP,  MVT::i64, Expand);

  setOperationAction(ISD::CTTZ, MVT::i64, Expand);
  setOperationAction(ISD::CTLZ, MVT::i64, Expand);
  setOperationAction(ISD::CTTZ_ZERO_UNDEF, MVT::i64, Expand);
  setOperationAction(ISD::CTLZ_ZERO_UNDEF, MVT::i64, Expand);
  setOperationAction(ISD::CTPOP, MVT::i64, Expand);

  // Extended load operations for i1 types must be promoted
  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1, Promote);

  }
  for (auto VT : {MVT::i1, MVT::i8, MVT::i16, MVT::i32})
    setOperationAction(ISD::SIGN_EXTEND_INREG, VT, Expand);


  setBooleanContents(ZeroOrOneBooleanContent);

  // Function alignments(llvm12ではアライン幅を2のn乗で直接指定することになった(従来まではlog2 アライン幅 で指定))
  setMinFunctionAlignment(Align(8));
  setPrefFunctionAlignment(Align(8));

  // inline memcpy() for kernel to see explicit copy
  MaxStoresPerMemset = MaxStoresPerMemsetOptSize = 128;
  MaxStoresPerMemcpy = MaxStoresPerMemcpyOptSize = 128;
  MaxStoresPerMemmove = MaxStoresPerMemmoveOptSize = 128;

}

bool StraightTargetLowering::isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const {
  return false;
}

SDValue StraightTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::BR_CC:
    llvm_unreachable("unimplemented operand (float? cond branch)");
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::SELECT:
    return lowerSELECT(Op, DAG);
  case ISD::ConstantFP:
    return LowerConstantFP(Op, DAG);
  case ISD::VASTART:
    return LowerVASTART(Op, DAG);
  case ISD::DYNAMIC_STACKALLOC:
    return LowerDynamicStackAlloc(Op, DAG);
  default:
    llvm_unreachable("unimplemented operand");
  }
}

// Calling Convention Implementation
#include "StraightGenCallingConv.inc"

SDValue StraightTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  switch (CallConv) {
  default:
    report_fatal_error("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    break;
  }

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 32> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, IsVarArg ? VA_Straight64 : CC_Straight64);

  if (IsVarArg) {
    MachineFrameInfo &MFI = MF.getFrameInfo();

    // RISC-Vを参考にした
	// 標準では保持できない情報を突っ込むことができる
    auto *STFI = MF.getInfo<StraightMachineFunctionInfo>();

    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
      CCValAssign &VA = ArgLocs[i];
      MVT LocVT = VA.getLocVT();

      // sanity check
      assert(VA.isMemLoc());

      int FI = MFI.CreateFixedObject(LocVT.getSizeInBits() / 8,
                                     VA.getLocMemOffset(), true);

      // Create load nodes to retrieve arguments from the stack
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      SDValue ArgValue = DAG.getLoad(
          LocVT, DL, Chain, FIN,
          MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI));


      InVals.push_back(ArgValue);
    }
    // RISC-Vを参考にしたが特に意味はないと思われる
    constexpr int regWidthInByte = 8;

    int64_t VaArgOffset = CCInfo.getNextStackOffset();
    int FI = MFI.CreateFixedObject(regWidthInByte, VaArgOffset, true);
    STFI->setVarArgStartFrameIndex(FI);

    return Chain;
  }

  for (auto &VA : ArgLocs) {
    if (VA.isRegLoc()) {
      // Arguments passed in registers
      EVT RegVT = VA.getLocVT();
      switch (RegVT.getSimpleVT().SimpleTy) {
      default: {
        errs() << "LowerFormalArguments Unhandled argument type: "
               << RegVT.getEVTString() << '\n';
        llvm_unreachable(0);
      }
	  case MVT::f32:
      case MVT::f64:
      case MVT::i64:
        unsigned VReg = RegInfo.createVirtualRegister(&Straight::GPRRegClass);
        RegInfo.addLiveIn(VA.getLocReg(), VReg);
        SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);

        // If this is an 8/16-bit value, it is really passed promoted to 32
        // bits. Insert an assert[sz]ext to capture this, then truncate to the
        // right size.
        if (VA.getLocInfo() == CCValAssign::SExt)
          ArgValue = DAG.getNode(ISD::AssertSext, DL, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));
        else if (VA.getLocInfo() == CCValAssign::ZExt)
          ArgValue = DAG.getNode(ISD::AssertZext, DL, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));

        if (VA.getLocInfo() != CCValAssign::Full)
          ArgValue = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), ArgValue);

        InVals.push_back(ArgValue);
      }
    } else {
      fail(DL, DAG, "defined with too many args");
      InVals.push_back(DAG.getConstant(0, DL, VA.getLocVT()));
    }
  }

  return Chain;
}

// RISC-Vより
SDValue StraightTargetLowering::LowerVASTART(SDValue Op, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  StraightMachineFunctionInfo *FuncInfo = MF.getInfo<StraightMachineFunctionInfo>();

  SDLoc DL(Op);
  SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgStartFrameIndex(),
                                 getPointerTy(MF.getDataLayout()));

  // vastart just stores the address of the VarArgsFrameIndex slot into the
  // memory location argument.
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  return DAG.getStore(Op.getOperand(0), DL, FI, Op.getOperand(1),
                      MachinePointerInfo(SV));
}

SDValue StraightTargetLowering::LowerDynamicStackAlloc(SDValue Op, SelectionDAG &DAG) const{
  const TargetLowering &TLI = DAG.getTargetLoweringInfo();
  RTLIB::Libcall LC = RTLIB::Libcall::STRAIGHT_DYNAMIC_ALLOCA_FALLBACK;
  SDLoc dl(Op);
  TargetLowering::MakeLibCallOptions CallOptions;
  return TLI.makeLibCall(DAG, LC, MVT::i64, Op.getOperand(1), CallOptions, dl).first;
}

// Mipsより
SDValue StraightTargetLowering::passArgOnStack(SDValue StackPtr, int64_t Offset,
                                           SDValue Chain, SDValue Arg,
                                           const SDLoc &DL, bool IsTailCall,
                                           SelectionDAG &DAG) const {
  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  auto *const STFI =
      DAG.getMachineFunction().getInfo<StraightMachineFunctionInfo>();

  int FI = MFI.CreateFixedObject(Arg.getValueSizeInBits() / 8, Offset, false);

  STFI->setVarArgFrameIndex(FI);
  SDValue FIN = DAG.getFrameIndex(FI, MVT::i64);
  return DAG.getStore(Chain, DL, Arg, FIN, MachinePointerInfo(),
                      /* Alignment = */ 0, MachineMemOperand::MOVolatile);
}

SDValue StraightTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc dl = CLI.DL;

  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins = CLI.Ins;

  SDValue InChain = CLI.Chain;
  SDValue Callee = CLI.Callee;

  bool &isTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool isVarArg = CLI.IsVarArg;

  //末尾呼び出しとは、とりあえずオフ
  isTailCall = false;

  //関数のオペランドを解析してオペランドをレジスタに割り当てる
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, isVarArg ? VA_Straight64 : CC_Straight64);

  SmallVector<std::pair<unsigned, SDValue>, 32> RegsToPass;
  SDValue StackPtr;

  //引数をRegToPassに追加していく
  for (unsigned i = 0, e = ArgLocs.size(); i != e; i++) {
    SDValue Arg = OutVals[i];
    CCValAssign &VA = ArgLocs[i];
    ISD::ArgFlagsTy Flags = Outs[i].Flags;

    //引数が数値
    if (Flags.isByVal()) {
      assert(Flags.getByValSize() &&
        "ByVal args of size 0 should have been egnored by front-end.");
      llvm_unreachable("ByVal arguments are not supported");
      continue;
    }

    //必要に応じてPromoteする
    switch (VA.getLocInfo()) {
    default: llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full:
      break;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    }

    //レジスタ経由の引数はRegsToPassに追加
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    }
    else {
      assert(VA.isMemLoc());
      // MIPSを参考に
	  InChain = passArgOnStack(StackPtr, VA.getLocMemOffset(),
                               InChain, Arg, dl, isTailCall, DAG);
    }
  }

  // 逆順で積む
  std::reverse(RegsToPass.begin(), RegsToPass.end());

  //レジスタをコピーするノードを作成
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; i++) {
    InChain = DAG.getCopyToReg(InChain, dl, RegsToPass[i].first, RegsToPass[i].second, InFlag);
    InFlag = InChain.getValue(1);
  }

  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, MVT::i64);
  }
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i64);
  }

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(InChain);
  Ops.push_back(Callee);

  //引数のレジスタをリストに追加
  for (unsigned i = 0, e = RegsToPass.size(); i != e; i++) {
    Ops.push_back(DAG.getRegister(RegsToPass[i].first, RegsToPass[i].second.getValueType()));
  }

  if (InFlag.getNode()) {
    Ops.push_back(InFlag);
  }

  InChain = DAG.getNode(StraightISD::CALL, dl, NodeTys, Ops);
  InFlag = InChain.getValue(1);

  //戻り値の処理
  return LowerCallResult(InChain, InFlag, CallConv, isVarArg, Ins, dl, DAG, InVals);

 // BasicBlockの分割は、StraightISD::CALL→Straight::JALの時にEmitInstrWithCustomInserterで行われる
}

SDValue
StraightTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               const SDLoc &DL, SelectionDAG &DAG) const {
  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;
  MachineFunction &MF = DAG.getMachineFunction();

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());

  // Analize return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_Straight64);

  // RISC-VではGlueと呼んでいる
  SDValue Flag;
  SmallVector<SDValue, 16> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); i++) {
    CCValAssign &VA = RVLocs[i];

    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }
  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(StraightISD::RET_FLAG, DL, MVT::Other, RetOps);
}

SDValue StraightTargetLowering::LowerCallResult(
    SDValue Chain, SDValue InFlag, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_Straight64);

  //結果レジスタをコピー
  for (unsigned i = 0; i != RVLocs.size(); i++) {
    SDValue Val = DAG.getCopyFromReg(Chain, DL, RVLocs[i].getLocReg(), RVLocs[i].getValVT(), InFlag);
    Chain = Val.getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Val.getValue(0));
  }

  return Chain;
}

// Changes the condition code and swaps operands if necessary, so the SetCC
// operation matches one of the comparisons supported directly in the RISC-V
// ISA.
static void normaliseSetCC(SDValue &LHS, SDValue &RHS, ISD::CondCode &CC) {
  switch (CC) {
  default:
    break;
  case ISD::SETGT:
  case ISD::SETLE:
  case ISD::SETUGT:
  case ISD::SETULE:
    CC = ISD::getSetCCSwappedOperands(CC);
    std::swap(LHS, RHS);
    break;
  }
}

// Return the RISC-V branch opcode that matches the given DAG integer
// condition code. The CondCode must be one of those supported by the RISC-V
// ISA (see normaliseSetCC).
static unsigned getBranchOpcodeForIntCondCode(ISD::CondCode CC) {
  switch (CC) {
  default:
    llvm_unreachable("Unsupported CondCode");
  case ISD::SETEQ:
    return Straight::Beq;
  case ISD::SETNE:
    return Straight::Bne;
  case ISD::SETLT:
    return Straight::Blt;
  case ISD::SETGE:
    return Straight::Bge;
  case ISD::SETULT:
    return Straight::Bltu;
  case ISD::SETUGE:
    return Straight::Bgeu;
  }
}

SDValue StraightTargetLowering::lowerSELECT(SDValue Op, SelectionDAG &DAG) const {
  SDValue CondV = Op.getOperand(0);
  SDValue TrueV = Op.getOperand(1);
  SDValue FalseV = Op.getOperand(2);
  SDLoc DL(Op);
  MVT XLenVT = MVT::i64;

  // If the result type is XLenVT and CondV is the output of a SETCC node
  // which also operated on XLenVT inputs, then merge the SETCC node into the
  // lowered RISCVISD::SELECT_CC to take advantage of the integer
  // compare+branch instructions. i.e.:
  // (select (setcc lhs, rhs, cc), truev, falsev)
  // -> (riscvisd::select_cc lhs, rhs, cc, truev, falsev)
  if (Op.getSimpleValueType() == XLenVT && CondV.getOpcode() == ISD::SETCC &&
      CondV.getOperand(0).getSimpleValueType() == XLenVT) {
    SDValue LHS = CondV.getOperand(0);
    SDValue RHS = CondV.getOperand(1);
    auto CC = cast<CondCodeSDNode>(CondV.getOperand(2));
    ISD::CondCode CCVal = CC->get();

    normaliseSetCC(LHS, RHS, CCVal);

    SDValue TargetCC = DAG.getConstant(CCVal, DL, XLenVT);
    SDVTList VTs = DAG.getVTList(Op.getValueType(), MVT::Glue);
    SDValue Ops[] = {LHS, RHS, TargetCC, TrueV, FalseV};
    return DAG.getNode(StraightISD::SELECT_CC, DL, VTs, Ops);
  }

  // Otherwise:
  // (select condv, truev, falsev)
  // -> (riscvisd::select_cc condv, zero, setne, truev, falsev)
  SDValue Zero = DAG.getConstant(0, DL, XLenVT);
  SDValue SetNE = DAG.getConstant(ISD::SETNE, DL, XLenVT);

  SDVTList VTs = DAG.getVTList(Op.getValueType(), MVT::Glue);
  SDValue Ops[] = {CondV, Zero, SetNE, TrueV, FalseV};

  return DAG.getNode(StraightISD::SELECT_CC, DL, VTs, Ops);
}

const char *StraightTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((StraightISD::NodeType)Opcode) {
  case StraightISD::FIRST_NUMBER:
    break;
  case StraightISD::RET_FLAG:
    return "StraightISD::RET_FLAG";
  case StraightISD::CALL:
    return "StraightISD::CALL";
  case StraightISD::GLOBAL:
    return "StraightISD::GLOBAL";
  case StraightISD::CONSTANT_FP:
    return "StraightISD::CONSTANT_FP";
  case StraightISD::SELECT_CC:
    return "StraightISD::SELECT_CC";
  case StraightISD::BITCAST_FTOI:
    return "StraightISD::BITCAST_FTOI";
  case StraightISD::BITCAST_ITOF:
    return "StraightISD::BITCAST_ITOF";
  }
  return nullptr;
}

SDValue StraightTargetLowering::LowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  auto N = cast<GlobalAddressSDNode>(Op);
  assert( N->getOffset() == 0 && "Invalid offset for global address" );

  SDLoc DL( Op );
  const GlobalValue *GV = N->getGlobal();
  SDValue GA = DAG.getTargetGlobalAddress(GV, DL, MVT::i64);
  return DAG.getNode(StraightISD::GLOBAL, DL, MVT::i64, GA);
}

SDValue StraightTargetLowering::LowerConstantFP(SDValue Op,
                                              SelectionDAG &DAG) const {
  int64_t N = cast<ConstantFPSDNode>(Op)->getValueAPF().bitcastToAPInt().getZExtValue();
  auto imm = DAG.getConstant(N, SDLoc(Op), MVT::i64);
  // ISD::BIT_CASTでは即座にCombineされて元に戻ってしまう
  return DAG.getNode(StraightISD::CONSTANT_FP, SDLoc(Op), Op.getSimpleValueType(), imm);
}

static bool isSelect_XXX_CC_XXX(unsigned int x) {
  switch( x ) {
  case Straight::Select_GPR_Using_CC_GPR:
  case Straight::Select_GPR_Using_CC_FPR:
  case Straight::Select_GPR_Using_CC_DPR:
  case Straight::Select_FPR_Using_CC_GPR:
  case Straight::Select_FPR_Using_CC_FPR:
  case Straight::Select_FPR_Using_CC_DPR:
  case Straight::Select_DPR_Using_CC_GPR:
  case Straight::Select_DPR_Using_CC_FPR:
  case Straight::Select_DPR_Using_CC_DPR: return true;
  default: return false;
  }
}

MachineBasicBlock *
StraightTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                               MachineBasicBlock *BB) const {
  if (isSelect_XXX_CC_XXX(MI.getOpcode())) {
    // To "insert" a SELECT instruction, we actually have to insert the triangle
    // control-flow pattern.  The incoming instruction knows the destination vreg
    // to set, the condition code register to branch on, the true/false values to
    // select between, and the condcode to use to select the appropriate branch.
    //
    // We produce the following control flow:
    //     HeadMBB
    //     |  \
    //     |  IfFalseMBB
    //     | /
    //    TailMBB
    const TargetInstrInfo &TII = *BB->getParent()->getSubtarget().getInstrInfo();
    const BasicBlock *LLVM_BB = BB->getBasicBlock();
    DebugLoc DL = MI.getDebugLoc();
    MachineFunction::iterator I = ++BB->getIterator();

    MachineBasicBlock *HeadMBB = BB;
    MachineFunction *F = BB->getParent();
    MachineBasicBlock *TailMBB = F->CreateMachineBasicBlock(LLVM_BB);
    MachineBasicBlock *IfFalseMBB = F->CreateMachineBasicBlock(LLVM_BB);

    F->insert(I, IfFalseMBB);
    F->insert(I, TailMBB);
    // Move all remaining instructions to TailMBB.
    TailMBB->splice(TailMBB->begin(), HeadMBB,
                    std::next(MachineBasicBlock::iterator(MI)), HeadMBB->end());
    // Update machine-CFG edges by transferring all successors of the current
    // block to the new block which will contain the Phi node for the select.
    TailMBB->transferSuccessorsAndUpdatePHIs(HeadMBB);
    // Set the successors for HeadMBB.
    HeadMBB->addSuccessor(IfFalseMBB);
    HeadMBB->addSuccessor(TailMBB);

    // Insert appropriate branch.
    unsigned LHS = MI.getOperand(1).getReg();
    unsigned RHS = MI.getOperand(2).getReg();
    auto CC = static_cast<ISD::CondCode>(MI.getOperand(3).getImm());
    unsigned Opcode = getBranchOpcodeForIntCondCode(CC);

    BuildMI(HeadMBB, DL, TII.get(Opcode))
      .addReg(LHS)
      .addReg(RHS)
      .addMBB(TailMBB);

    // IfFalseMBB just falls through to TailMBB.
    IfFalseMBB->addSuccessor(TailMBB);

    // %Result = phi [ %TrueValue, HeadMBB ], [ %FalseValue, IfFalseMBB ]
    BuildMI(*TailMBB, TailMBB->begin(), DL, TII.get(Straight::PHI),
            MI.getOperand(0).getReg())
        .addReg(MI.getOperand(4).getReg())
        .addMBB(HeadMBB)
        .addReg(MI.getOperand(5).getReg())
        .addMBB(IfFalseMBB);

    MI.eraseFromParent(); // The pseudo instruction is gone now.
    return TailMBB;
  }
  bool isCallOp = MI.getOpcode() == Straight::JAL || MI.getOpcode() == Straight::JALR;
  assert( isCallOp );

  // 関数コールがあるとき、呼び出し関数先で何命令実行されるかわからない
  // そのため、BasicBlockを分割し、必要があればその前後でスピルする

  // コードはBPFTargetLowering::EmitInstrWithCustomInserterを参考に
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator I = ++BB->getIterator(); //なんで？

  MachineFunction *F = BB->getParent();
  MachineBasicBlock *newMBB = F->CreateMachineBasicBlock(LLVM_BB);

  F->insert(I, newMBB);
  // Update machine-CFG edges by transferring all successors of the current
  // block to the new block which will contain the Phi node for the select.
  newMBB->splice(newMBB->begin(), BB,
                   std::next(MachineBasicBlock::iterator(MI)), BB->end());
  newMBB->transferSuccessorsAndUpdatePHIs(BB);
  // Next, add the true and fallthrough blocks as its successors.
  BB->addSuccessor(newMBB);

  return newMBB;
}
