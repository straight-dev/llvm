//===-- StraightISelLowering.h - Straight DAG Lowering Interface ----------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_Straight_StraightISELLOWERING_H
#define LLVM_LIB_TARGET_Straight_StraightISELLOWERING_H

#include "Straight.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
class StraightSubtarget;
namespace StraightISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET_FLAG,
  CALL,
  GLOBAL,
  CONSTANT_FP,
  SELECT_CC,
  BITCAST_FTOI,
  BITCAST_ITOF,
};
}

class StraightTargetLowering : public TargetLowering {
public:
  explicit StraightTargetLowering(const TargetMachine &TM, const StraightSubtarget &STI);

  // Provide custom lowering hooks for some operations.
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  // This method returns the name of a target specific DAG node.
  const char *getTargetNodeName(unsigned Opcode) const override;

  // This method decides whether folding a constant offset
  // with the given GlobalAddress is legal.
  bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const override;

  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr &MI,
                              MachineBasicBlock *BB) const override;

private:
  SDValue lowerSELECT(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerConstantFP(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerDynamicStackAlloc(SDValue Op, SelectionDAG &DAG) const;

  // Lower the result values of a call, copying them out of physregs into vregs
  SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                          CallingConv::ID CallConv, bool IsVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins,
                          const SDLoc &DL, SelectionDAG &DAG,
                          SmallVectorImpl<SDValue> &InVals) const;

  // Lower a call into CALLSEQ_START - StraightISD:CALL - CALLSEQ_END chain
  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  // Lower incoming arguments, copy physregs into vregs
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;

  EVT getOptimalMemOpType(const MemOp &Op,
                          const AttributeList & /*FuncAttributes*/) const override {
    return MVT::i32;
  }

  EVT getSetCCResultType(const DataLayout &DL, LLVMContext &, EVT VT) const override {
    if (!VT.isVector())
      return getPointerTy(DL).SimpleTy;
    return VT.changeVectorElementTypeToInteger();
  }

  // MIPSよりコピペ
  SDValue passArgOnStack(SDValue StackPtr, int64_t Offset, SDValue Chain,
                         SDValue Arg, const SDLoc &DL, bool IsTailCall,
                         SelectionDAG &DAG) const;


  virtual bool isFPImmLegal( const APFloat &/*Imm*/, EVT /*VT*/ , bool /*ForCodeSize*/) const override {
    return true;
  }

  bool shouldConvertConstantLoadToIntImm(const APInt &Imm,
                                         Type *Ty) const override {
    return true;
  }
};
}

#endif
