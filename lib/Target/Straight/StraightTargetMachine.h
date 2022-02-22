//===-- StraightTargetMachine.h - Define TargetMachine for Straight --- C++ ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Straight specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Straight_StraightTARGETMACHINE_H
#define LLVM_LIB_TARGET_Straight_StraightTARGETMACHINE_H

#include "StraightSubtarget.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class StraightTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  StraightSubtarget Subtarget;

public:
  StraightTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                   StringRef FS, const TargetOptions &Options,
                   Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                   CodeGenOpt::Level OL, bool JIT);

  const StraightSubtarget *getSubtargetImpl() const { return &Subtarget; }
  const StraightSubtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};
}

#endif
