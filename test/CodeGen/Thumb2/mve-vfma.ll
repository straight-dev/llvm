; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mtriple=thumbv8.1m.main-none-none-eabi -mattr=+mve.fp %s -o - | FileCheck %s

define arm_aapcs_vfpcc <4 x float> @fma_v4f32(<4 x float> %dst, <4 x float> %s1, <4 x float> %s2) {
; CHECK-LABEL: fma_v4f32:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vfma.f32 q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %0 = tail call fast <4 x float> @llvm.fma.v4f32(<4 x float> %s1, <4 x float> %s2, <4 x float> %dst)
  ret <4 x float> %0
}

define arm_aapcs_vfpcc <8 x half> @fma_v8f16(<8 x half> %dst, <8 x half> %s1, <8 x half> %s2) {
; CHECK-LABEL: fma_v8f16:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    vfma.f16 q0, q1, q2
; CHECK-NEXT:    bx lr
entry:
  %0 = tail call fast <8 x half> @llvm.fma.v8f16(<8 x half> %s1, <8 x half> %s2, <8 x half> %dst)
  ret <8 x half> %0
}

declare <4 x float> @llvm.fma.v4f32(<4 x float>, <4 x float>, <4 x float>)
declare <8 x half> @llvm.fma.v8f16(<8 x half>, <8 x half>, <8 x half>)
