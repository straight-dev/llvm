; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+sse2 | FileCheck %s --check-prefixes=SSE,SSE2
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+ssse3 | FileCheck %s --check-prefixes=SSE,SSSE3
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+sse4.1 | FileCheck %s --check-prefixes=SSE,SSE41
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx | FileCheck %s --check-prefixes=AVX,AVX1
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx2 | FileCheck %s --check-prefixes=AVX,AVX2

;
; Partial Vector Loads - PR16739
;

define <4 x float> @load_float4_float3(<4 x float>* nocapture readonly dereferenceable(16)) {
; SSE-LABEL: load_float4_float3:
; SSE:       # %bb.0:
; SSE-NEXT:    movups (%rdi), %xmm0
; SSE-NEXT:    retq
;
; AVX-LABEL: load_float4_float3:
; AVX:       # %bb.0:
; AVX-NEXT:    vmovups (%rdi), %xmm0
; AVX-NEXT:    retq
  %p0 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 0
  %p1 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 1
  %p2 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 2
  %ld0 = load float, float* %p0, align 4
  %ld1 = load float, float* %p1, align 4
  %ld2 = load float, float* %p2, align 4
  %r0 = insertelement <4 x float> undef, float %ld0, i32 0
  %r1 = insertelement <4 x float> %r0,   float %ld1, i32 1
  %r2 = insertelement <4 x float> %r1,   float %ld2, i32 2
  ret <4 x float> %r2
}

define <4 x float> @load_float4_float3_0122(<4 x float>* nocapture readonly dereferenceable(16)) {
; SSE-LABEL: load_float4_float3_0122:
; SSE:       # %bb.0:
; SSE-NEXT:    movss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; SSE-NEXT:    movups (%rdi), %xmm0
; SSE-NEXT:    shufps {{.*#+}} xmm0 = xmm0[0,1],xmm1[0,0]
; SSE-NEXT:    retq
;
; AVX-LABEL: load_float4_float3_0122:
; AVX:       # %bb.0:
; AVX-NEXT:    vmovss {{.*#+}} xmm0 = mem[0],zero,zero,zero
; AVX-NEXT:    vmovups (%rdi), %xmm1
; AVX-NEXT:    vshufps {{.*#+}} xmm0 = xmm1[0,1],xmm0[0,0]
; AVX-NEXT:    retq
  %p0 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 0
  %p1 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 1
  %p2 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 2
  %ld0 = load float, float* %p0, align 4
  %ld1 = load float, float* %p1, align 4
  %ld2 = load float, float* %p2, align 4
  %r0 = insertelement <4 x float> undef, float %ld0, i32 0
  %r1 = insertelement <4 x float> %r0,   float %ld1, i32 1
  %r2 = insertelement <4 x float> %r1,   float %ld2, i32 2
  %r3 = insertelement <4 x float> %r2,   float %ld2, i32 3
  ret <4 x float> %r3
}

define <8 x float> @load_float8_float3(<4 x float>* nocapture readonly dereferenceable(16)) {
; SSE-LABEL: load_float8_float3:
; SSE:       # %bb.0:
; SSE-NEXT:    movups (%rdi), %xmm0
; SSE-NEXT:    retq
;
; AVX-LABEL: load_float8_float3:
; AVX:       # %bb.0:
; AVX-NEXT:    vmovups (%rdi), %xmm0
; AVX-NEXT:    retq
  %p0 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 0
  %p1 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 1
  %p2 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 2
  %ld0 = load float, float* %p0, align 4
  %ld1 = load float, float* %p1, align 4
  %ld2 = load float, float* %p2, align 4
  %r0 = insertelement <8 x float> undef, float %ld0, i32 0
  %r1 = insertelement <8 x float> %r0,   float %ld1, i32 1
  %r2 = insertelement <8 x float> %r1,   float %ld2, i32 2
  ret <8 x float> %r2
}

define <8 x float> @load_float8_float3_0122(<4 x float>* nocapture readonly dereferenceable(16)) {
; SSE-LABEL: load_float8_float3_0122:
; SSE:       # %bb.0:
; SSE-NEXT:    movss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; SSE-NEXT:    movups (%rdi), %xmm0
; SSE-NEXT:    shufps {{.*#+}} xmm0 = xmm0[0,1],xmm1[0,0]
; SSE-NEXT:    retq
;
; AVX-LABEL: load_float8_float3_0122:
; AVX:       # %bb.0:
; AVX-NEXT:    vmovss {{.*#+}} xmm0 = mem[0],zero,zero,zero
; AVX-NEXT:    vmovups (%rdi), %xmm1
; AVX-NEXT:    vshufps {{.*#+}} xmm0 = xmm1[0,1],xmm0[0,0]
; AVX-NEXT:    retq
  %p0 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 0
  %p1 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 1
  %p2 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 2
  %ld0 = load float, float* %p0, align 4
  %ld1 = load float, float* %p1, align 4
  %ld2 = load float, float* %p2, align 4
  %r0 = insertelement <8 x float> undef, float %ld0, i32 0
  %r1 = insertelement <8 x float> %r0,   float %ld1, i32 1
  %r2 = insertelement <8 x float> %r1,   float %ld2, i32 2
  %r3 = insertelement <8 x float> %r2,   float %ld2, i32 3
  ret <8 x float> %r3
}

define <4 x float> @load_float4_float3_as_float2_float(<4 x float>* nocapture readonly dereferenceable(16)) {
; SSE-LABEL: load_float4_float3_as_float2_float:
; SSE:       # %bb.0:
; SSE-NEXT:    movups (%rdi), %xmm0
; SSE-NEXT:    retq
;
; AVX-LABEL: load_float4_float3_as_float2_float:
; AVX:       # %bb.0:
; AVX-NEXT:    vmovups (%rdi), %xmm0
; AVX-NEXT:    retq
  %2 = bitcast <4 x float>* %0 to <2 x float>*
  %3 = load <2 x float>, <2 x float>* %2, align 4
  %4 = extractelement <2 x float> %3, i32 0
  %5 = insertelement <4 x float> undef, float %4, i32 0
  %6 = extractelement <2 x float> %3, i32 1
  %7 = insertelement <4 x float> %5, float %6, i32 1
  %8 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 2
  %9 = load float, float* %8, align 4
  %10 = insertelement <4 x float> %7, float %9, i32 2
  ret <4 x float> %10
}

define <4 x float> @load_float4_float3_as_float2_float_0122(<4 x float>* nocapture readonly dereferenceable(16)) {
; SSE-LABEL: load_float4_float3_as_float2_float_0122:
; SSE:       # %bb.0:
; SSE-NEXT:    movsd {{.*#+}} xmm0 = mem[0],zero
; SSE-NEXT:    movss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; SSE-NEXT:    shufps {{.*#+}} xmm0 = xmm0[0,1],xmm1[0,0]
; SSE-NEXT:    retq
;
; AVX-LABEL: load_float4_float3_as_float2_float_0122:
; AVX:       # %bb.0:
; AVX-NEXT:    vmovsd {{.*#+}} xmm0 = mem[0],zero
; AVX-NEXT:    vmovss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; AVX-NEXT:    vshufps {{.*#+}} xmm0 = xmm0[0,1],xmm1[0,0]
; AVX-NEXT:    retq
  %2 = bitcast <4 x float>* %0 to <2 x float>*
  %3 = load <2 x float>, <2 x float>* %2, align 4
  %4 = extractelement <2 x float> %3, i32 0
  %5 = insertelement <4 x float> undef, float %4, i32 0
  %6 = extractelement <2 x float> %3, i32 1
  %7 = insertelement <4 x float> %5, float %6, i32 1
  %8 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 2
  %9 = load float, float* %8, align 4
  %10 = insertelement <4 x float> %7, float %9, i32 2
  %11 = insertelement <4 x float> %10, float %9, i32 3
  ret <4 x float> %11
}

define <4 x float> @load_float4_float3_trunc(<4 x float>* nocapture readonly dereferenceable(16)) {
; SSE-LABEL: load_float4_float3_trunc:
; SSE:       # %bb.0:
; SSE-NEXT:    movaps (%rdi), %xmm0
; SSE-NEXT:    retq
;
; AVX-LABEL: load_float4_float3_trunc:
; AVX:       # %bb.0:
; AVX-NEXT:    vmovaps (%rdi), %xmm0
; AVX-NEXT:    retq
  %2 = bitcast <4 x float>* %0 to i64*
  %3 = load i64, i64* %2, align 16
  %4 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 2
  %5 = bitcast float* %4 to i64*
  %6 = load i64, i64* %5, align 8
  %7 = trunc i64 %3 to i32
  %8 = bitcast i32 %7 to float
  %9 = insertelement <4 x float> undef, float %8, i32 0
  %10 = lshr i64 %3, 32
  %11 = trunc i64 %10 to i32
  %12 = bitcast i32 %11 to float
  %13 = insertelement <4 x float> %9, float %12, i32 1
  %14 = trunc i64 %6 to i32
  %15 = bitcast i32 %14 to float
  %16 = insertelement <4 x float> %13, float %15, i32 2
  ret <4 x float> %16
}

define <4 x float> @load_float4_float3_trunc_0122(<4 x float>* nocapture readonly dereferenceable(16)) {
; SSE-LABEL: load_float4_float3_trunc_0122:
; SSE:       # %bb.0:
; SSE-NEXT:    movss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; SSE-NEXT:    movaps (%rdi), %xmm0
; SSE-NEXT:    shufps {{.*#+}} xmm0 = xmm0[0,1],xmm1[0,0]
; SSE-NEXT:    retq
;
; AVX-LABEL: load_float4_float3_trunc_0122:
; AVX:       # %bb.0:
; AVX-NEXT:    vmovss {{.*#+}} xmm0 = mem[0],zero,zero,zero
; AVX-NEXT:    vmovaps (%rdi), %xmm1
; AVX-NEXT:    vshufps {{.*#+}} xmm0 = xmm1[0,1],xmm0[0,0]
; AVX-NEXT:    retq
  %2 = bitcast <4 x float>* %0 to i64*
  %3 = load i64, i64* %2, align 16
  %4 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 2
  %5 = bitcast float* %4 to i64*
  %6 = load i64, i64* %5, align 8
  %7 = trunc i64 %3 to i32
  %8 = bitcast i32 %7 to float
  %9 = insertelement <4 x float> undef, float %8, i32 0
  %10 = lshr i64 %3, 32
  %11 = trunc i64 %10 to i32
  %12 = bitcast i32 %11 to float
  %13 = insertelement <4 x float> %9, float %12, i32 1
  %14 = trunc i64 %6 to i32
  %15 = bitcast i32 %14 to float
  %16 = insertelement <4 x float> %13, float %15, i32 2
  %17 = insertelement <4 x float> %16, float %15, i32 3
  ret <4 x float> %17
}

define <4 x float> @load_float4_float3_trunc_0123(<4 x float>* nocapture readonly dereferenceable(16)) {
; SSE2-LABEL: load_float4_float3_trunc_0123:
; SSE2:       # %bb.0:
; SSE2-NEXT:    movaps (%rdi), %xmm0
; SSE2-NEXT:    movhps {{.*#+}} xmm0 = xmm0[0,1],mem[0,1]
; SSE2-NEXT:    retq
;
; SSSE3-LABEL: load_float4_float3_trunc_0123:
; SSSE3:       # %bb.0:
; SSSE3-NEXT:    movaps (%rdi), %xmm0
; SSSE3-NEXT:    movhps {{.*#+}} xmm0 = xmm0[0,1],mem[0,1]
; SSSE3-NEXT:    retq
;
; SSE41-LABEL: load_float4_float3_trunc_0123:
; SSE41:       # %bb.0:
; SSE41-NEXT:    movaps (%rdi), %xmm0
; SSE41-NEXT:    insertps {{.*#+}} xmm0 = xmm0[0,1],mem[0],xmm0[3]
; SSE41-NEXT:    insertps {{.*#+}} xmm0 = xmm0[0,1,2],mem[0]
; SSE41-NEXT:    retq
;
; AVX-LABEL: load_float4_float3_trunc_0123:
; AVX:       # %bb.0:
; AVX-NEXT:    vmovaps (%rdi), %xmm0
; AVX-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1],mem[0],xmm0[3]
; AVX-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1,2],mem[0]
; AVX-NEXT:    retq
  %2 = bitcast <4 x float>* %0 to i64*
  %3 = load i64, i64* %2, align 16
  %4 = getelementptr inbounds <4 x float>, <4 x float>* %0, i64 0, i64 2
  %5 = bitcast float* %4 to i64*
  %6 = load i64, i64* %5, align 8
  %7 = trunc i64 %3 to i32
  %8 = bitcast i32 %7 to float
  %9 = insertelement <4 x float> undef, float %8, i32 0
  %10 = lshr i64 %3, 32
  %11 = trunc i64 %10 to i32
  %12 = bitcast i32 %11 to float
  %13 = insertelement <4 x float> %9, float %12, i32 1
  %14 = trunc i64 %6 to i32
  %15 = bitcast i32 %14 to float
  %16 = insertelement <4 x float> %13, float %15, i32 2
  %17 = lshr i64 %6, 32
  %18 = trunc i64 %17 to i32
  %19 = bitcast i32 %18 to float
  %20 = insertelement <4 x float> %16, float %19, i32 3
  ret <4 x float> %20
}

; PR21780
define <4 x double> @load_double4_0u2u(double* nocapture readonly dereferenceable(32)) {
; SSE2-LABEL: load_double4_0u2u:
; SSE2:       # %bb.0:
; SSE2-NEXT:    movsd {{.*#+}} xmm0 = mem[0],zero
; SSE2-NEXT:    movsd {{.*#+}} xmm1 = mem[0],zero
; SSE2-NEXT:    movlhps {{.*#+}} xmm0 = xmm0[0,0]
; SSE2-NEXT:    movlhps {{.*#+}} xmm1 = xmm1[0,0]
; SSE2-NEXT:    retq
;
; SSSE3-LABEL: load_double4_0u2u:
; SSSE3:       # %bb.0:
; SSSE3-NEXT:    movddup {{.*#+}} xmm0 = mem[0,0]
; SSSE3-NEXT:    movddup {{.*#+}} xmm1 = mem[0,0]
; SSSE3-NEXT:    retq
;
; SSE41-LABEL: load_double4_0u2u:
; SSE41:       # %bb.0:
; SSE41-NEXT:    movddup {{.*#+}} xmm0 = mem[0,0]
; SSE41-NEXT:    movddup {{.*#+}} xmm1 = mem[0,0]
; SSE41-NEXT:    retq
;
; AVX-LABEL: load_double4_0u2u:
; AVX:       # %bb.0:
; AVX-NEXT:    vmovddup {{.*#+}} ymm0 = mem[0,0,2,2]
; AVX-NEXT:    retq
  %2 = load double, double* %0, align 8
  %3 = insertelement <4 x double> undef, double %2, i32 0
  %4 = getelementptr inbounds double, double* %0, i64 2
  %5 = load double, double* %4, align 8
  %6 = insertelement <4 x double> %3, double %5, i32 2
  %7 = shufflevector <4 x double> %6, <4 x double> undef, <4 x i32> <i32 0, i32 0, i32 2, i32 2>
  ret <4 x double> %7
}

; Test case identified in rL366501
@h = dso_local local_unnamed_addr global i8 0, align 1
define dso_local i32 @load_partial_illegal_type() {
; SSE2-LABEL: load_partial_illegal_type:
; SSE2:       # %bb.0:
; SSE2-NEXT:    movzwl {{.*}}(%rip), %eax
; SSE2-NEXT:    movd %eax, %xmm0
; SSE2-NEXT:    pand {{.*}}(%rip), %xmm0
; SSE2-NEXT:    por {{.*}}(%rip), %xmm0
; SSE2-NEXT:    movd %xmm0, %eax
; SSE2-NEXT:    retq
;
; SSSE3-LABEL: load_partial_illegal_type:
; SSSE3:       # %bb.0:
; SSSE3-NEXT:    movzwl {{.*}}(%rip), %eax
; SSSE3-NEXT:    movd %eax, %xmm0
; SSSE3-NEXT:    pshufb {{.*#+}} xmm0 = xmm0[0,1],zero,xmm0[3,u,u,u,u,u,u,u,u,u,u,u,u]
; SSSE3-NEXT:    por {{.*}}(%rip), %xmm0
; SSSE3-NEXT:    movd %xmm0, %eax
; SSSE3-NEXT:    retq
;
; SSE41-LABEL: load_partial_illegal_type:
; SSE41:       # %bb.0:
; SSE41-NEXT:    movzwl {{.*}}(%rip), %eax
; SSE41-NEXT:    movd %eax, %xmm0
; SSE41-NEXT:    movl $2, %eax
; SSE41-NEXT:    pinsrb $2, %eax, %xmm0
; SSE41-NEXT:    movd %xmm0, %eax
; SSE41-NEXT:    retq
;
; AVX-LABEL: load_partial_illegal_type:
; AVX:       # %bb.0:
; AVX-NEXT:    movzwl {{.*}}(%rip), %eax
; AVX-NEXT:    vmovd %eax, %xmm0
; AVX-NEXT:    movl $2, %eax
; AVX-NEXT:    vpinsrb $2, %eax, %xmm0, %xmm0
; AVX-NEXT:    vmovd %xmm0, %eax
; AVX-NEXT:    retq
  %1 = load <2 x i8>, <2 x i8>* bitcast (i8* @h to <2 x i8>*), align 1
  %2 = shufflevector <2 x i8> %1, <2 x i8> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %3 = insertelement <4 x i8> %2, i8 2, i32 2
  %4 = bitcast <4 x i8> %3 to i32
  ret i32 %4
}

define dso_local void @PR43227(i32* %explicit_0, <8 x i32>* %explicit_1) {
; SSE-LABEL: PR43227:
; SSE:       # %bb.0:
; SSE-NEXT:    movq {{.*#+}} xmm0 = mem[0],zero
; SSE-NEXT:    movd {{.*#+}} xmm1 = mem[0],zero,zero,zero
; SSE-NEXT:    psrlq $32, %xmm0
; SSE-NEXT:    punpcklqdq {{.*#+}} xmm0 = xmm0[0],xmm1[0]
; SSE-NEXT:    pxor %xmm1, %xmm1
; SSE-NEXT:    movdqa %xmm1, 672(%rsi)
; SSE-NEXT:    movdqa %xmm0, 688(%rsi)
; SSE-NEXT:    retq
;
; AVX1-LABEL: PR43227:
; AVX1:       # %bb.0:
; AVX1-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; AVX1-NEXT:    vmovd {{.*#+}} xmm1 = mem[0],zero,zero,zero
; AVX1-NEXT:    vpsrlq $32, %xmm0, %xmm0
; AVX1-NEXT:    vpunpcklqdq {{.*#+}} xmm0 = xmm0[0],xmm1[0]
; AVX1-NEXT:    vpxor %xmm1, %xmm1, %xmm1
; AVX1-NEXT:    vinsertf128 $1, %xmm0, %ymm1, %ymm0
; AVX1-NEXT:    vmovaps %ymm0, 672(%rsi)
; AVX1-NEXT:    vzeroupper
; AVX1-NEXT:    retq
;
; AVX2-LABEL: PR43227:
; AVX2:       # %bb.0:
; AVX2-NEXT:    vmovq {{.*#+}} xmm0 = mem[0],zero
; AVX2-NEXT:    vmovd {{.*#+}} xmm1 = mem[0],zero,zero,zero
; AVX2-NEXT:    vpsrlq $32, %xmm0, %xmm0
; AVX2-NEXT:    vpunpcklqdq {{.*#+}} xmm0 = xmm0[0],xmm1[0]
; AVX2-NEXT:    vpxor %xmm1, %xmm1, %xmm1
; AVX2-NEXT:    vinserti128 $1, %xmm0, %ymm1, %ymm0
; AVX2-NEXT:    vmovdqa %ymm0, 672(%rsi)
; AVX2-NEXT:    vzeroupper
; AVX2-NEXT:    retq
  %1 = getelementptr i32, i32* %explicit_0, i64 63
  %2 = bitcast i32* %1 to <3 x i32>*
  %3 = load <3 x i32>, <3 x i32>* %2, align 1
  %4 = shufflevector <3 x i32> %3, <3 x i32> undef, <2 x i32> <i32 1, i32 2>
  %5 = shufflevector <2 x i32> %4, <2 x i32> undef, <8 x i32> <i32 0, i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %6 = shufflevector <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 undef, i32 0, i32 undef, i32 0>, <8 x i32> %5, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 8, i32 5, i32 9, i32 7>
  %7 = getelementptr inbounds <8 x i32>, <8 x i32>* %explicit_1, i64 21
  store <8 x i32> %6, <8 x i32>* %7, align 32
  ret void
}
