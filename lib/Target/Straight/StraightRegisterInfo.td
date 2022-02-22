//===-- StraightRegisterInfo.td - Straight Register defs -------------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the Straight register file
//===----------------------------------------------------------------------===//

// Registers are identified with 4-bit ID numbers.
// Ri - 64-bit integer registers
class Ri<bits<16> Enc, string n> : Register<n> {
  let Namespace = "Straight";
  let HWEncoding = Enc;
}

// Integer registers
def R0 : Ri< 0, "r0">, DwarfRegNum<[0]>;
def R1 : Ri< 1, "r1">, DwarfRegNum<[1]>;
def R2 : Ri< 2, "r2">, DwarfRegNum<[2]>;
def R3 : Ri< 3, "r3">, DwarfRegNum<[3]>;
def R4 : Ri< 4, "r4">, DwarfRegNum<[4]>;
def R5 : Ri< 5, "r5">, DwarfRegNum<[5]>;
def R6 : Ri< 6, "r6">, DwarfRegNum<[6]>;
def R7 : Ri< 7, "r7">, DwarfRegNum<[7]>;
def R8 : Ri< 8, "r8">, DwarfRegNum<[8]>;
def R9 : Ri< 9, "r9">, DwarfRegNum<[9]>;
def R10 : Ri<10, "r10">, DwarfRegNum<[10]>;
def R11 : Ri<11, "r11">, DwarfRegNum<[11]>;
def R12 : Ri<12, "r12">, DwarfRegNum<[12]>;
def R13 : Ri<13, "r13">, DwarfRegNum<[13]>;
def R14 : Ri<14, "r14">, DwarfRegNum<[14]>;
def R15 : Ri<15, "r15">, DwarfRegNum<[15]>;
def R16 : Ri<16, "r16">, DwarfRegNum<[16]>;
def R17 : Ri<17, "r17">, DwarfRegNum<[17]>;
def R18 : Ri<18, "r18">, DwarfRegNum<[18]>;
def R19 : Ri<19, "r19">, DwarfRegNum<[19]>;
def F0 : Ri<20, "f0">, DwarfRegNum<[20]>;
def F1 : Ri<21, "f1">, DwarfRegNum<[21]>;
def F2 : Ri<22, "f2">, DwarfRegNum<[22]>;
def F3 : Ri<23, "f3">, DwarfRegNum<[23]>;
def F4 : Ri<24, "f4">, DwarfRegNum<[24]>;
def F5 : Ri<25, "f5">, DwarfRegNum<[25]>;
def F6 : Ri<26, "f6">, DwarfRegNum<[26]>;
def F7 : Ri<27, "f7">, DwarfRegNum<[27]>;
def F8 : Ri<28, "f8">, DwarfRegNum<[28]>;
def F9 : Ri<29, "f9">, DwarfRegNum<[29]>;
def ARG0 : Ri<30, "a0">, DwarfRegNum<[30]>;
def ARG1 : Ri<31, "a1">, DwarfRegNum<[31]>;
def ARG2 : Ri<32, "a2">, DwarfRegNum<[32]>;
def ARG3 : Ri<33, "a3">, DwarfRegNum<[33]>;
def ARG4 : Ri<34, "a4">, DwarfRegNum<[34]>;
def ARG5 : Ri<35, "a5">, DwarfRegNum<[35]>;
def ARG6 : Ri<36, "a6">, DwarfRegNum<[36]>;
def ARG7 : Ri<37, "a7">, DwarfRegNum<[37]>;
def ARG8 : Ri<38, "a8">, DwarfRegNum<[38]>;
def ARG9 : Ri<39, "a9">, DwarfRegNum<[39]>;
def ARG10 : Ri<40, "a10">, DwarfRegNum<[40]>;
def ARG11 : Ri<41, "a11">, DwarfRegNum<[41]>;
def ARG12 : Ri<42, "a12">, DwarfRegNum<[42]>;
def ARG13 : Ri<43, "a13">, DwarfRegNum<[43]>;
def ARG14 : Ri<44, "a14">, DwarfRegNum<[44]>;
def ARG15 : Ri<45, "a15">, DwarfRegNum<[45]>;
def ARG16 : Ri<46, "a16">, DwarfRegNum<[46]>;
def ARG17 : Ri<47, "a17">, DwarfRegNum<[47]>;
def ARG18 : Ri<48, "a18">, DwarfRegNum<[48]>;
def ARG19 : Ri<49, "a19">, DwarfRegNum<[49]>;
def ARG20 : Ri<50, "a20">, DwarfRegNum<[50]>;
def ARG21 : Ri<51, "a21">, DwarfRegNum<[51]>;
def ARG22 : Ri<52, "a22">, DwarfRegNum<[52]>;
def ARG23 : Ri<53, "a23">, DwarfRegNum<[53]>;
def ARG24 : Ri<54, "a24">, DwarfRegNum<[54]>;
def ARG25 : Ri<55, "a25">, DwarfRegNum<[55]>;
def ARG26 : Ri<56, "a26">, DwarfRegNum<[56]>;
def ARG27 : Ri<57, "a27">, DwarfRegNum<[57]>;
def ARG28 : Ri<58, "a28">, DwarfRegNum<[58]>;
def ARG29 : Ri<59, "a29">, DwarfRegNum<[59]>;
def RET0 : Ri<60, "e0">, DwarfRegNum<[60]>;
def RET1 : Ri<61, "e1">, DwarfRegNum<[61]>;
def RET2 : Ri<62, "e2">, DwarfRegNum<[62]>;
def RET3 : Ri<63, "e3">, DwarfRegNum<[63]>;
def RET4 : Ri<64, "e4">, DwarfRegNum<[64]>;
def RET5 : Ri<65, "e5">, DwarfRegNum<[65]>;
def RET6 : Ri<66, "e6">, DwarfRegNum<[66]>;
def RET7 : Ri<67, "e7">, DwarfRegNum<[67]>;
def RET8 : Ri<68, "e8">, DwarfRegNum<[68]>;
def RET9 : Ri<69, "e9">, DwarfRegNum<[69]>; // STRAIGHTでは複数のレジスタの返却も可能！
def RETADDR : Ri<70, "lr">, DwarfRegNum<[70]>;
def SP : Ri<71, "SP">, DwarfRegNum<[71]>;
def ZeroReg : Ri<72, "Z-Reg">, DwarfRegNum<[72]>;



// Register classes.
def GPR : RegisterClass<"Straight", [i64], 64, (add (sequence "R%u", 19, 0), (sequence "ARG%u", 29, 0), (sequence "RET%u", 9, 0), RETADDR, ZeroReg)>;
def FPR : RegisterClass<"Straight", [f32], 64, (add (sequence "F%u", 9, 0), (sequence "ARG%u", 29, 0), (sequence "RET%u", 9, 0), ZeroReg)>;
def DPR : RegisterClass<"Straight", [f64], 64, (add (sequence "F%u", 9, 0), (sequence "ARG%u", 29, 0), (sequence "RET%u", 9, 0), ZeroReg)>;


def Rewritable : RegisterClass<"Straight", [i64], 64, (add SP)>;
