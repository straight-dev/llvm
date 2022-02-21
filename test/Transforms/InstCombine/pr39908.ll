; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -instcombine -S | FileCheck %s

target datalayout = "p:32:32"

%S = type { [2 x i32] }

define i1 @test([0 x %S]* %p, i32 %n) {
; CHECK-LABEL: @test(
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq i32 [[N:%.*]], 1
; CHECK-NEXT:    ret i1 [[CMP]]
;
  %start.cast = bitcast [0 x %S]* %p to %S*
  %end = getelementptr inbounds [0 x %S], [0 x %S]* %p, i32 0, i32 %n, i32 0, i32 0
  %end.cast = bitcast i32* %end to %S*
  %last = getelementptr inbounds %S, %S* %end.cast, i32 -1
  %cmp = icmp eq %S* %last, %start.cast
  ret i1 %cmp
}

; Same test using 64-bit indices.
define i1 @test64([0 x %S]* %p, i64 %n) {
; CHECK-LABEL: @test64(
; CHECK-NEXT:    [[TMP1:%.*]] = trunc i64 [[N:%.*]] to i32
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq i32 [[TMP1]], 1
; CHECK-NEXT:    ret i1 [[CMP]]
;
  %start.cast = bitcast [0 x %S]* %p to %S*
  %end = getelementptr inbounds [0 x %S], [0 x %S]* %p, i64 0, i64 %n, i32 0, i64 0
  %end.cast = bitcast i32* %end to %S*
  %last = getelementptr inbounds %S, %S* %end.cast, i64 -1
  %cmp = icmp eq %S* %last, %start.cast
  ret i1 %cmp
}

; Here the offset overflows and is treated modulo 2^32. This is UB.
define i1 @test64_overflow([0 x %S]* %p, i64 %n) {
; CHECK-LABEL: @test64_overflow(
; CHECK-NEXT:    [[TMP1:%.*]] = trunc i64 [[N:%.*]] to i32
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq i32 [[TMP1]], 1
; CHECK-NEXT:    ret i1 [[CMP]]
;
  %start.cast = bitcast [0 x %S]* %p to %S*
  %end = getelementptr inbounds [0 x %S], [0 x %S]* %p, i64 0, i64 %n, i32 0, i64 8589934592
  %end.cast = bitcast i32* %end to %S*
  %last = getelementptr inbounds %S, %S* %end.cast, i64 -1
  %cmp = icmp eq %S* %last, %start.cast
  ret i1 %cmp
}
