; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -global-isel -mtriple=amdgcn-mesa-mesa3d -mcpu=gfx700 -verify-machineinstrs < %s | FileCheck -check-prefix=GFX7 %s
; RUN: llc -global-isel -mtriple=amdgcn-mesa-mesa3d -mcpu=gfx803 -verify-machineinstrs < %s | FileCheck -check-prefix=GFX8 %s
; RUN: llc -global-isel -mtriple=amdgcn-mesa-mesa3d -mcpu=gfx1010 -verify-machineinstrs < %s | FileCheck -check-prefix=GFX10 %s

define amdgpu_ps float @test_writelane_s_s_s(i32 inreg %data, i32 inreg %lane, i32 inreg %vdst.in) #0 {
; GFX7-LABEL: test_writelane_s_s_s:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    v_mov_b32_e32 v0, s4
; GFX7-NEXT:    s_mov_b32 m0, s3
; GFX7-NEXT:    v_writelane_b32 v0, s2, m0
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_s_s_s:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    v_mov_b32_e32 v0, s4
; GFX8-NEXT:    s_mov_b32 m0, s3
; GFX8-NEXT:    v_writelane_b32 v0, s2, m0
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_s_s_s:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    v_mov_b32_e32 v0, s4
; GFX10-NEXT:    v_writelane_b32 v0, s2, s3
; GFX10-NEXT:    ; return to shader part epilog
  %writelane = call i32 @llvm.amdgcn.writelane(i32 %data, i32 %lane, i32 %vdst.in)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}

define amdgpu_ps float @test_writelane_s_s_imm(i32 inreg %data, i32 inreg %lane) #0 {
; GFX7-LABEL: test_writelane_s_s_imm:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    v_mov_b32_e32 v0, 42
; GFX7-NEXT:    s_mov_b32 m0, s3
; GFX7-NEXT:    v_writelane_b32 v0, s2, m0
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_s_s_imm:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    v_mov_b32_e32 v0, 42
; GFX8-NEXT:    s_mov_b32 m0, s3
; GFX8-NEXT:    v_writelane_b32 v0, s2, m0
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_s_s_imm:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    v_mov_b32_e32 v0, 42
; GFX10-NEXT:    v_writelane_b32 v0, s2, s3
; GFX10-NEXT:    ; return to shader part epilog
  %writelane = call i32 @llvm.amdgcn.writelane(i32 %data, i32 %lane, i32 42)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}

; data is not inline imm
define amdgpu_ps float @test_writelane_k_s_v(i32 inreg %lane, i32 %vdst.in) #0 {
; GFX7-LABEL: test_writelane_k_s_v:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    s_movk_i32 s0, 0x3e7
; GFX7-NEXT:    s_mov_b32 m0, s2
; GFX7-NEXT:    v_writelane_b32 v0, s0, m0
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_k_s_v:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    s_movk_i32 s0, 0x3e7
; GFX8-NEXT:    s_mov_b32 m0, s2
; GFX8-NEXT:    v_writelane_b32 v0, s0, m0
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_k_s_v:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    s_movk_i32 s0, 0x3e7
; GFX10-NEXT:    v_writelane_b32 v0, s0, s2
; GFX10-NEXT:    ; return to shader part epilog
  %writelane = call i32 @llvm.amdgcn.writelane(i32 999, i32 %lane, i32 %vdst.in)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}

; Data is inline imm
define amdgpu_ps float @test_writelane_imm_s_v(i32 inreg %lane, i32 %vdst.in) #0 {
; GFX7-LABEL: test_writelane_imm_s_v:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    v_writelane_b32 v0, 42, s2
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_imm_s_v:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    v_writelane_b32 v0, 42, s2
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_imm_s_v:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    v_writelane_b32 v0, 42, s2
; GFX10-NEXT:    ; return to shader part epilog
  %writelane = call i32 @llvm.amdgcn.writelane(i32 42, i32 %lane, i32 %vdst.in)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}

; Data is subtarget dependent inline imm
define amdgpu_ps float @test_writelane_imminv2pi_s_v(i32 inreg %lane, i32 %vdst.in) #0 {
; GFX7-LABEL: test_writelane_imminv2pi_s_v:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    s_mov_b32 s0, 0x3e22f983
; GFX7-NEXT:    s_mov_b32 m0, s2
; GFX7-NEXT:    v_writelane_b32 v0, s0, m0
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_imminv2pi_s_v:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    v_writelane_b32 v0, 0.15915494, s2
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_imminv2pi_s_v:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    v_writelane_b32 v0, 0.15915494, s2
; GFX10-NEXT:    ; return to shader part epilog
  %writelane = call i32 @llvm.amdgcn.writelane(i32 bitcast (float 0x3FC45F3060000000 to i32), i32 %lane, i32 %vdst.in)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}


; Lane is inline imm
define amdgpu_ps float @test_writelane_s_imm_v(i32 inreg %data, i32 %vdst.in) #0 {
; GFX7-LABEL: test_writelane_s_imm_v:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    v_writelane_b32 v0, s2, 23
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_s_imm_v:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    v_writelane_b32 v0, s2, 23
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_s_imm_v:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    v_writelane_b32 v0, s2, 23
; GFX10-NEXT:    ; return to shader part epilog
  %writelane = call i32 @llvm.amdgcn.writelane(i32 %data, i32 23, i32 %vdst.in)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}

; Lane index is larger than the wavesize
define amdgpu_ps float @test_writelane_s_k0_v(i32 inreg %data, i32 %vdst.in) #0 {
; GFX7-LABEL: test_writelane_s_k0_v:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    v_writelane_b32 v0, s2, 3
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_s_k0_v:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    v_writelane_b32 v0, s2, 3
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_s_k0_v:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    s_movk_i32 s0, 0x43
; GFX10-NEXT:    v_writelane_b32 v0, s2, s0
; GFX10-NEXT:    ; return to shader part epilog
  %writelane = call i32 @llvm.amdgcn.writelane(i32 %data, i32 67, i32 %vdst.in)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}

; Lane index is larger than the wavesize for wave32
define amdgpu_ps float @test_writelane_s_k1_v(i32 inreg %data, i32 %vdst.in) #0 {
; GFX7-LABEL: test_writelane_s_k1_v:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    v_writelane_b32 v0, s2, 32
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_s_k1_v:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    v_writelane_b32 v0, s2, 32
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_s_k1_v:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    v_writelane_b32 v0, s2, 32
; GFX10-NEXT:    ; return to shader part epilog
  %writelane = call i32 @llvm.amdgcn.writelane(i32 %data, i32 32, i32 %vdst.in)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}

define amdgpu_ps float @test_writelane_v_v_v(i32 %data, i32 %lane, i32 %vdst.in) #0 {
; GFX7-LABEL: test_writelane_v_v_v:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    v_readfirstlane_b32 s1, v1
; GFX7-NEXT:    v_readfirstlane_b32 s0, v0
; GFX7-NEXT:    s_mov_b32 m0, s1
; GFX7-NEXT:    v_writelane_b32 v2, s0, m0
; GFX7-NEXT:    v_mov_b32_e32 v0, v2
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_v_v_v:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    v_readfirstlane_b32 s1, v1
; GFX8-NEXT:    v_readfirstlane_b32 s0, v0
; GFX8-NEXT:    s_mov_b32 m0, s1
; GFX8-NEXT:    v_writelane_b32 v2, s0, m0
; GFX8-NEXT:    v_mov_b32_e32 v0, v2
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_v_v_v:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    v_readfirstlane_b32 s0, v0
; GFX10-NEXT:    v_readfirstlane_b32 s1, v1
; GFX10-NEXT:    v_writelane_b32 v2, s0, s1
; GFX10-NEXT:    v_mov_b32_e32 v0, v2
; GFX10-NEXT:    ; return to shader part epilog
  %writelane = call i32 @llvm.amdgcn.writelane(i32 %data, i32 %lane, i32 %vdst.in)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}

define amdgpu_ps float @test_writelane_v_s_v(i32 %data, i32 inreg %lane, i32 %vdst.in) #0 {
; GFX7-LABEL: test_writelane_v_s_v:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    v_readfirstlane_b32 s0, v0
; GFX7-NEXT:    s_mov_b32 m0, s2
; GFX7-NEXT:    v_writelane_b32 v1, s0, m0
; GFX7-NEXT:    v_mov_b32_e32 v0, v1
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_v_s_v:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    v_readfirstlane_b32 s0, v0
; GFX8-NEXT:    s_mov_b32 m0, s2
; GFX8-NEXT:    v_writelane_b32 v1, s0, m0
; GFX8-NEXT:    v_mov_b32_e32 v0, v1
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_v_s_v:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    v_readfirstlane_b32 s0, v0
; GFX10-NEXT:    v_writelane_b32 v1, s0, s2
; GFX10-NEXT:    v_mov_b32_e32 v0, v1
; GFX10-NEXT:    ; return to shader part epilog
  %writelane = call i32 @llvm.amdgcn.writelane(i32 %data, i32 inreg %lane, i32 %vdst.in)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}

; FIXME: This could theoretically use m0 directly as the data source,
; and another SGPR as the lane selector and avoid register swap.
define amdgpu_ps float @test_writelane_m0_s_v(i32 inreg %lane, i32 %vdst.in) #0 {
; GFX7-LABEL: test_writelane_m0_s_v:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    ;;#ASMSTART
; GFX7-NEXT:    s_mov_b32 m0, -1
; GFX7-NEXT:    ;;#ASMEND
; GFX7-NEXT:    s_mov_b32 s0, m0
; GFX7-NEXT:    s_mov_b32 m0, s2
; GFX7-NEXT:    v_writelane_b32 v0, s0, m0
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_m0_s_v:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    ;;#ASMSTART
; GFX8-NEXT:    s_mov_b32 m0, -1
; GFX8-NEXT:    ;;#ASMEND
; GFX8-NEXT:    s_mov_b32 s0, m0
; GFX8-NEXT:    s_mov_b32 m0, s2
; GFX8-NEXT:    v_writelane_b32 v0, s0, m0
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_m0_s_v:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    ;;#ASMSTART
; GFX10-NEXT:    s_mov_b32 m0, -1
; GFX10-NEXT:    ;;#ASMEND
; GFX10-NEXT:    v_writelane_b32 v0, m0, s2
; GFX10-NEXT:    ; return to shader part epilog
  %m0 = call i32 asm "s_mov_b32 m0, -1", "={m0}"()
  %writelane = call i32 @llvm.amdgcn.writelane(i32 %m0, i32 %lane, i32 %vdst.in)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}

define amdgpu_ps float @test_writelane_s_m0_v(i32 inreg %data, i32 %vdst.in) #0 {
; GFX7-LABEL: test_writelane_s_m0_v:
; GFX7:       ; %bb.0:
; GFX7-NEXT:    ;;#ASMSTART
; GFX7-NEXT:    s_mov_b32 m0, -1
; GFX7-NEXT:    ;;#ASMEND
; GFX7-NEXT:    v_writelane_b32 v0, s2, m0
; GFX7-NEXT:    ; return to shader part epilog
;
; GFX8-LABEL: test_writelane_s_m0_v:
; GFX8:       ; %bb.0:
; GFX8-NEXT:    ;;#ASMSTART
; GFX8-NEXT:    s_mov_b32 m0, -1
; GFX8-NEXT:    ;;#ASMEND
; GFX8-NEXT:    v_writelane_b32 v0, s2, m0
; GFX8-NEXT:    ; return to shader part epilog
;
; GFX10-LABEL: test_writelane_s_m0_v:
; GFX10:       ; %bb.0:
; GFX10-NEXT:    ;;#ASMSTART
; GFX10-NEXT:    s_mov_b32 m0, -1
; GFX10-NEXT:    ;;#ASMEND
; GFX10-NEXT:    v_writelane_b32 v0, s2, m0
; GFX10-NEXT:    ; return to shader part epilog
  %m0 = call i32 asm "s_mov_b32 m0, -1", "={m0}"()
  %writelane = call i32 @llvm.amdgcn.writelane(i32 %data, i32 %m0, i32 %vdst.in)
  %writelane.cast = bitcast i32 %writelane to float
  ret float %writelane.cast
}

declare i32 @llvm.amdgcn.writelane(i32, i32, i32) #1
declare i32 @llvm.amdgcn.workitem.id.x() #2

attributes #0 = { nounwind }
attributes #1 = { convergent nounwind readnone willreturn }
attributes #2 = { nounwind readnone speculatable willreturn }
