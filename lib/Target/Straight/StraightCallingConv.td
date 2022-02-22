//===-- StraightCallingConv.td - Calling Conventions Straight --------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This describes the calling conventions for the Straight architecture.
//
//===----------------------------------------------------------------------===//

// Straight C return-value convention.
def RetCC_Straight64 : CallingConv<[CCIfType<[ i64, f32, f64 ], CCAssignToReg<[ RET0, RET1, RET2, RET3, RET4, RET5, RET6, RET7, RET8, RET9 ]>>]>;

// Straight C Calling convention.
def CC_Straight64 : CallingConv<[
  // Promote i8/i16/i32 args to i64
  CCIfType<[ i8, i16, i32 ], CCPromoteToType<i64>>,

  // All arguments get passed in integer registers if there is space.
  CCIfType<[ i64, f32, f64 ], CCAssignToReg<[ ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10, ARG11, ARG12, ARG13, ARG14, ARG15, ARG16, ARG17, ARG18, ARG19, ARG20, ARG21, ARG22, ARG23, ARG24, ARG25, ARG26, ARG27, ARG28, ARG29 ]>>,

  // Could be assigned to the stack in 8-byte aligned units, but unsupported
  CCAssignToStack<8, 8>
]>;

// Straight Calling convention with Variable Arguments
def VA_Straight64 : CallingConv<[
  CCAssignToStack<8, 8>
]>;


def CSR : CalleeSavedRegs<(add (sequence "R%u", 19, 0))>;
