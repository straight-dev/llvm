; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -O3 -mtriple=x86_64-linux-generic -mattr=avx < %s | FileCheck %s

; Bug 45833:
; The SplitVecRes_MSTORE method should split a extended value type
; according to the halving of the enveloping type to avoid all sorts
; of inconsistencies downstream. For example for a extended value type
; with VL=14 and enveloping type VL=16 that is split 8/8, the extended
; type should be split 8/6 and not 7/7. This also accounts for hi masked
; store that get zero storage size (and are unused).

define void @mstore_split9(<9 x float> %value, <9 x float>* %addr, <9 x i1> %mask) {
; CHECK-LABEL: mstore_split9:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0],xmm5[0],xmm4[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0,1],xmm6[0],xmm4[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0,1,2],xmm7[0]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0],xmm1[0],xmm0[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1],xmm2[0],xmm0[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1,2],xmm3[0]
; CHECK-NEXT:    vinsertf128 $1, %xmm4, %ymm0, %ymm0
; CHECK-NEXT:    vmovss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; CHECK-NEXT:    movzbl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vmovd %eax, %xmm2
; CHECK-NEXT:    vpslld $31, %xmm2, %xmm2
; CHECK-NEXT:    vmaskmovps %ymm1, %ymm2, 32(%rdi)
; CHECK-NEXT:    vmovd %esi, %xmm1
; CHECK-NEXT:    vpinsrw $1, %edx, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrw $2, %ecx, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrw $3, %r8d, %xmm1, %xmm1
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm2 = xmm1[0],zero,xmm1[1],zero,xmm1[2],zero,xmm1[3],zero
; CHECK-NEXT:    vpslld $31, %xmm2, %xmm2
; CHECK-NEXT:    vpinsrw $4, %r9d, %xmm1, %xmm1
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $5, %eax, %xmm1, %xmm1
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $6, %eax, %xmm1, %xmm1
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $7, %eax, %xmm1, %xmm1
; CHECK-NEXT:    vpunpckhwd {{.*#+}} xmm1 = xmm1[4,4,5,5,6,6,7,7]
; CHECK-NEXT:    vpslld $31, %xmm1, %xmm1
; CHECK-NEXT:    vinsertf128 $1, %xmm1, %ymm2, %ymm1
; CHECK-NEXT:    vmaskmovps %ymm0, %ymm1, (%rdi)
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retq
  call void @llvm.masked.store.v9f32.p0v9f32(<9 x float> %value, <9 x float>* %addr, i32 4, <9 x i1>%mask)
  ret void
}

define void @mstore_split13(<13 x float> %value, <13 x float>* %addr, <13 x i1> %mask) {
; CHECK-LABEL: mstore_split13:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0],xmm5[0],xmm4[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0,1],xmm6[0],xmm4[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0,1,2],xmm7[0]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0],xmm1[0],xmm0[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1],xmm2[0],xmm0[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1,2],xmm3[0]
; CHECK-NEXT:    vinsertf128 $1, %xmm4, %ymm0, %ymm0
; CHECK-NEXT:    vmovss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0],mem[0],xmm1[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0,1],mem[0],xmm1[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0,1,2],mem[0]
; CHECK-NEXT:    vmovss {{.*#+}} xmm2 = mem[0],zero,zero,zero
; CHECK-NEXT:    vinsertf128 $1, %xmm2, %ymm1, %ymm1
; CHECK-NEXT:    movzbl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vmovd %eax, %xmm2
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $1, %eax, %xmm2, %xmm2
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $2, %eax, %xmm2, %xmm2
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $3, %eax, %xmm2, %xmm2
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm3 = xmm2[0],zero,xmm2[1],zero,xmm2[2],zero,xmm2[3],zero
; CHECK-NEXT:    vpslld $31, %xmm3, %xmm3
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $4, %eax, %xmm2, %xmm2
; CHECK-NEXT:    vpunpckhwd {{.*#+}} xmm2 = xmm2[4,4,5,5,6,6,7,7]
; CHECK-NEXT:    vpslld $31, %xmm2, %xmm2
; CHECK-NEXT:    vinsertf128 $1, %xmm2, %ymm3, %ymm2
; CHECK-NEXT:    vmaskmovps %ymm1, %ymm2, 32(%rdi)
; CHECK-NEXT:    vmovd %esi, %xmm1
; CHECK-NEXT:    vpinsrw $1, %edx, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrw $2, %ecx, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrw $3, %r8d, %xmm1, %xmm1
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm2 = xmm1[0],zero,xmm1[1],zero,xmm1[2],zero,xmm1[3],zero
; CHECK-NEXT:    vpslld $31, %xmm2, %xmm2
; CHECK-NEXT:    vpinsrw $4, %r9d, %xmm1, %xmm1
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $5, %eax, %xmm1, %xmm1
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $6, %eax, %xmm1, %xmm1
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $7, %eax, %xmm1, %xmm1
; CHECK-NEXT:    vpunpckhwd {{.*#+}} xmm1 = xmm1[4,4,5,5,6,6,7,7]
; CHECK-NEXT:    vpslld $31, %xmm1, %xmm1
; CHECK-NEXT:    vinsertf128 $1, %xmm1, %ymm2, %ymm1
; CHECK-NEXT:    vmaskmovps %ymm0, %ymm1, (%rdi)
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retq
  call void @llvm.masked.store.v13f32.p0v13f32(<13 x float> %value, <13 x float>* %addr, i32 4, <13 x i1>%mask)
  ret void
}

define void @mstore_split14(<14 x float> %value, <14 x float>* %addr, <14 x i1> %mask) {
; CHECK-LABEL: mstore_split14:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0],xmm5[0],xmm4[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0,1],xmm6[0],xmm4[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0,1,2],xmm7[0]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0],xmm1[0],xmm0[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1],xmm2[0],xmm0[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1,2],xmm3[0]
; CHECK-NEXT:    vinsertf128 $1, %xmm4, %ymm0, %ymm0
; CHECK-NEXT:    vmovss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0],mem[0],xmm1[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0,1],mem[0],xmm1[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0,1,2],mem[0]
; CHECK-NEXT:    vmovss {{.*#+}} xmm2 = mem[0],zero,zero,zero
; CHECK-NEXT:    vinsertps {{.*#+}} xmm2 = xmm2[0],mem[0],xmm2[2,3]
; CHECK-NEXT:    vinsertf128 $1, %xmm2, %ymm1, %ymm1
; CHECK-NEXT:    movzbl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vmovd %eax, %xmm2
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $1, %eax, %xmm2, %xmm2
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $2, %eax, %xmm2, %xmm2
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $3, %eax, %xmm2, %xmm2
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm3 = xmm2[0],zero,xmm2[1],zero,xmm2[2],zero,xmm2[3],zero
; CHECK-NEXT:    vpslld $31, %xmm3, %xmm3
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $4, %eax, %xmm2, %xmm2
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $5, %eax, %xmm2, %xmm2
; CHECK-NEXT:    vpunpckhwd {{.*#+}} xmm2 = xmm2[4,4,5,5,6,6,7,7]
; CHECK-NEXT:    vpslld $31, %xmm2, %xmm2
; CHECK-NEXT:    vinsertf128 $1, %xmm2, %ymm3, %ymm2
; CHECK-NEXT:    vmaskmovps %ymm1, %ymm2, 32(%rdi)
; CHECK-NEXT:    vmovd %esi, %xmm1
; CHECK-NEXT:    vpinsrw $1, %edx, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrw $2, %ecx, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrw $3, %r8d, %xmm1, %xmm1
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm2 = xmm1[0],zero,xmm1[1],zero,xmm1[2],zero,xmm1[3],zero
; CHECK-NEXT:    vpslld $31, %xmm2, %xmm2
; CHECK-NEXT:    vpinsrw $4, %r9d, %xmm1, %xmm1
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $5, %eax, %xmm1, %xmm1
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $6, %eax, %xmm1, %xmm1
; CHECK-NEXT:    movl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vpinsrw $7, %eax, %xmm1, %xmm1
; CHECK-NEXT:    vpunpckhwd {{.*#+}} xmm1 = xmm1[4,4,5,5,6,6,7,7]
; CHECK-NEXT:    vpslld $31, %xmm1, %xmm1
; CHECK-NEXT:    vinsertf128 $1, %xmm1, %ymm2, %ymm1
; CHECK-NEXT:    vmaskmovps %ymm0, %ymm1, (%rdi)
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retq
  call void @llvm.masked.store.v14f32.p0v14f32(<14 x float> %value, <14 x float>* %addr, i32 4, <14 x i1>%mask)
  ret void
}

define void @mstore_split17(<17 x float> %value, <17 x float>* %addr, <17 x i1> %mask) {
; CHECK-LABEL: mstore_split17:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0],xmm5[0],xmm4[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0,1],xmm6[0],xmm4[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0,1,2],xmm7[0]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0],xmm1[0],xmm0[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1],xmm2[0],xmm0[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1,2],xmm3[0]
; CHECK-NEXT:    vinsertf128 $1, %xmm4, %ymm0, %ymm0
; CHECK-NEXT:    vmovss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0],mem[0],xmm1[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0,1],mem[0],xmm1[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0,1,2],mem[0]
; CHECK-NEXT:    vmovss {{.*#+}} xmm2 = mem[0],zero,zero,zero
; CHECK-NEXT:    vinsertps {{.*#+}} xmm2 = xmm2[0],mem[0],xmm2[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm2 = xmm2[0,1],mem[0],xmm2[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm2 = xmm2[0,1,2],mem[0]
; CHECK-NEXT:    vinsertf128 $1, %xmm2, %ymm1, %ymm1
; CHECK-NEXT:    vmovss {{.*#+}} xmm2 = mem[0],zero,zero,zero
; CHECK-NEXT:    movzbl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vmovd %eax, %xmm3
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm3 = xmm3[0],zero,xmm3[1],zero,xmm3[2],zero,xmm3[3],zero
; CHECK-NEXT:    vpslld $31, %xmm3, %xmm3
; CHECK-NEXT:    vmaskmovps %ymm2, %ymm3, 64(%rdi)
; CHECK-NEXT:    vmovd {{.*#+}} xmm2 = mem[0],zero,zero,zero
; CHECK-NEXT:    vpinsrb $2, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpinsrb $4, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpinsrb $6, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm3 = xmm2[0],zero,xmm2[1],zero,xmm2[2],zero,xmm2[3],zero
; CHECK-NEXT:    vpslld $31, %xmm3, %xmm3
; CHECK-NEXT:    vpinsrb $8, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpinsrb $10, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpinsrb $12, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpinsrb $14, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpunpckhwd {{.*#+}} xmm2 = xmm2[4,4,5,5,6,6,7,7]
; CHECK-NEXT:    vpslld $31, %xmm2, %xmm2
; CHECK-NEXT:    vinsertf128 $1, %xmm2, %ymm3, %ymm2
; CHECK-NEXT:    vmaskmovps %ymm1, %ymm2, 32(%rdi)
; CHECK-NEXT:    vmovd %esi, %xmm1
; CHECK-NEXT:    vpinsrb $2, %edx, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrb $4, %ecx, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrb $6, %r8d, %xmm1, %xmm1
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm2 = xmm1[0],zero,xmm1[1],zero,xmm1[2],zero,xmm1[3],zero
; CHECK-NEXT:    vpslld $31, %xmm2, %xmm2
; CHECK-NEXT:    vpinsrb $8, %r9d, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrb $10, {{[0-9]+}}(%rsp), %xmm1, %xmm1
; CHECK-NEXT:    vpinsrb $12, {{[0-9]+}}(%rsp), %xmm1, %xmm1
; CHECK-NEXT:    vpinsrb $14, {{[0-9]+}}(%rsp), %xmm1, %xmm1
; CHECK-NEXT:    vpunpckhwd {{.*#+}} xmm1 = xmm1[4,4,5,5,6,6,7,7]
; CHECK-NEXT:    vpslld $31, %xmm1, %xmm1
; CHECK-NEXT:    vinsertf128 $1, %xmm1, %ymm2, %ymm1
; CHECK-NEXT:    vmaskmovps %ymm0, %ymm1, (%rdi)
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retq
  call void @llvm.masked.store.v17f32.p0v17f32(<17 x float> %value, <17 x float>* %addr, i32 4, <17 x i1>%mask)
  ret void
}

define void @mstore_split23(<23 x float> %value, <23 x float>* %addr, <23 x i1> %mask) {
; CHECK-LABEL: mstore_split23:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0],xmm5[0],xmm4[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0,1],xmm6[0],xmm4[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm4 = xmm4[0,1,2],xmm7[0]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0],xmm1[0],xmm0[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1],xmm2[0],xmm0[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm0 = xmm0[0,1,2],xmm3[0]
; CHECK-NEXT:    vinsertf128 $1, %xmm4, %ymm0, %ymm0
; CHECK-NEXT:    vmovss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0],mem[0],xmm1[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0,1],mem[0],xmm1[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm1 = xmm1[0,1,2],mem[0]
; CHECK-NEXT:    vmovss {{.*#+}} xmm2 = mem[0],zero,zero,zero
; CHECK-NEXT:    vinsertps {{.*#+}} xmm2 = xmm2[0],mem[0],xmm2[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm2 = xmm2[0,1],mem[0],xmm2[3]
; CHECK-NEXT:    vinsertf128 $1, %xmm2, %ymm1, %ymm1
; CHECK-NEXT:    vmovss {{.*#+}} xmm2 = mem[0],zero,zero,zero
; CHECK-NEXT:    vinsertps {{.*#+}} xmm2 = xmm2[0],mem[0],xmm2[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm2 = xmm2[0,1],mem[0],xmm2[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm2 = xmm2[0,1,2],mem[0]
; CHECK-NEXT:    vmovss {{.*#+}} xmm3 = mem[0],zero,zero,zero
; CHECK-NEXT:    vinsertps {{.*#+}} xmm3 = xmm3[0],mem[0],xmm3[2,3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm3 = xmm3[0,1],mem[0],xmm3[3]
; CHECK-NEXT:    vinsertps {{.*#+}} xmm3 = xmm3[0,1,2],mem[0]
; CHECK-NEXT:    vinsertf128 $1, %xmm3, %ymm2, %ymm2
; CHECK-NEXT:    movzbl {{[0-9]+}}(%rsp), %eax
; CHECK-NEXT:    vmovd {{.*#+}} xmm3 = mem[0],zero,zero,zero
; CHECK-NEXT:    vpinsrb $2, {{[0-9]+}}(%rsp), %xmm3, %xmm3
; CHECK-NEXT:    vpinsrb $4, {{[0-9]+}}(%rsp), %xmm3, %xmm3
; CHECK-NEXT:    vpinsrb $6, {{[0-9]+}}(%rsp), %xmm3, %xmm3
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm4 = xmm3[0],zero,xmm3[1],zero,xmm3[2],zero,xmm3[3],zero
; CHECK-NEXT:    vpslld $31, %xmm4, %xmm4
; CHECK-NEXT:    vpinsrb $8, {{[0-9]+}}(%rsp), %xmm3, %xmm3
; CHECK-NEXT:    vpinsrb $10, {{[0-9]+}}(%rsp), %xmm3, %xmm3
; CHECK-NEXT:    vpinsrb $12, {{[0-9]+}}(%rsp), %xmm3, %xmm3
; CHECK-NEXT:    vpinsrb $14, {{[0-9]+}}(%rsp), %xmm3, %xmm3
; CHECK-NEXT:    vpunpckhwd {{.*#+}} xmm3 = xmm3[4,4,5,5,6,6,7,7]
; CHECK-NEXT:    vpslld $31, %xmm3, %xmm3
; CHECK-NEXT:    vinsertf128 $1, %xmm3, %ymm4, %ymm3
; CHECK-NEXT:    vmaskmovps %ymm2, %ymm3, 32(%rdi)
; CHECK-NEXT:    vmovd %eax, %xmm2
; CHECK-NEXT:    vpinsrb $2, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpinsrb $4, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpinsrb $6, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm3 = xmm2[0],zero,xmm2[1],zero,xmm2[2],zero,xmm2[3],zero
; CHECK-NEXT:    vpslld $31, %xmm3, %xmm3
; CHECK-NEXT:    vpinsrb $8, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpinsrb $10, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpinsrb $12, {{[0-9]+}}(%rsp), %xmm2, %xmm2
; CHECK-NEXT:    vpunpckhwd {{.*#+}} xmm2 = xmm2[4,4,5,5,6,6,7,7]
; CHECK-NEXT:    vpslld $31, %xmm2, %xmm2
; CHECK-NEXT:    vinsertf128 $1, %xmm2, %ymm3, %ymm2
; CHECK-NEXT:    vmaskmovps %ymm1, %ymm2, 64(%rdi)
; CHECK-NEXT:    vmovd %esi, %xmm1
; CHECK-NEXT:    vpinsrb $2, %edx, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrb $4, %ecx, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrb $6, %r8d, %xmm1, %xmm1
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm2 = xmm1[0],zero,xmm1[1],zero,xmm1[2],zero,xmm1[3],zero
; CHECK-NEXT:    vpslld $31, %xmm2, %xmm2
; CHECK-NEXT:    vpinsrb $8, %r9d, %xmm1, %xmm1
; CHECK-NEXT:    vpinsrb $10, {{[0-9]+}}(%rsp), %xmm1, %xmm1
; CHECK-NEXT:    vpinsrb $12, {{[0-9]+}}(%rsp), %xmm1, %xmm1
; CHECK-NEXT:    vpinsrb $14, {{[0-9]+}}(%rsp), %xmm1, %xmm1
; CHECK-NEXT:    vpunpckhwd {{.*#+}} xmm1 = xmm1[4,4,5,5,6,6,7,7]
; CHECK-NEXT:    vpslld $31, %xmm1, %xmm1
; CHECK-NEXT:    vinsertf128 $1, %xmm1, %ymm2, %ymm1
; CHECK-NEXT:    vmaskmovps %ymm0, %ymm1, (%rdi)
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retq
  call void @llvm.masked.store.v23f32.p0v23f32(<23 x float> %value, <23 x float>* %addr, i32 4, <23 x i1>%mask)
  ret void
}

declare void @llvm.masked.store.v9f32.p0v9f32(<9 x float>, <9 x float>*, i32, <9 x i1>)
declare void @llvm.masked.store.v13f32.p0v13f32(<13 x float>, <13 x float>*, i32, <13 x i1>)
declare void @llvm.masked.store.v14f32.p0v14f32(<14 x float>, <14 x float>*, i32, <14 x i1>)
declare void @llvm.masked.store.v17f32.p0v17f32(<17 x float>, <17 x float>*, i32, <17 x i1>)
declare void @llvm.masked.store.v23f32.p0v23f32(<23 x float>, <23 x float>*, i32, <23 x i1>)
