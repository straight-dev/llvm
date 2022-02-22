//===-- Straight.td - Describe the Straight Target Machine -----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

include "StraightRegisterInfo.td"
include "StraightCallingConv.td"
include "StraightInstrInfo.td"

def StraightInstrInfo : InstrInfo;

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;
def : Proc<"v1", []>;
def : Proc<"v2", []>;
def : Proc<"probe", []>;

def StraightInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def StraightAsmParser : AsmParser {
  bit HasMnemonicFirst = 0;
}

def StraightAsmParserVariant : AsmParserVariant {
  int Variant = 0;
  string Name = "Straight";
  string BreakCharacters = ".";
  string TokenizingCharacters = "#()[]=:.<>!+*";
}

def Straight : Target {
  let InstructionSet = StraightInstrInfo;
  let AssemblyWriters = [StraightInstPrinter];
  let AssemblyParsers = [StraightAsmParser];
  let AssemblyParserVariants = [StraightAsmParserVariant];
}
