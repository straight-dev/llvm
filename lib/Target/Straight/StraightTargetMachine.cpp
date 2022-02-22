//===-- StraightTargetMachine.cpp - Define TargetMachine for Straight ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about Straight target spec.
//
//===----------------------------------------------------------------------===//

#include "StraightTargetMachine.h"
#include "Straight.h"
#include "StraightCodeGen.h"
#include "StraightPhiPass.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
using namespace llvm;

extern "C" void LLVMInitializeStraightTarget() {
  // Register the target.
  RegisterTargetMachine<StraightTargetMachine> X(getTheStraightTarget());
}

// DataLayout: little or big endian
static std::string computeDataLayout(const Triple &TT) {
  return "e-m:e-p:64:64-i64:64-n64-S64";
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::PIC_;
  return *RM;
}

[[maybe_unused]]
static CodeModel::Model getEffectiveCodeModel(Optional<CodeModel::Model> CM,
                                              CodeModel::Model Default) {
  if (CM)
    return *CM;
  return CodeModel::Small;
}

StraightTargetMachine::StraightTargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   Optional<Reloc::Model> RM,
                                   Optional<CodeModel::Model> CM,
                                   CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(TT), TT, CPU, FS, Options,
                        getEffectiveRelocModel(RM), getEffectiveCodeModel(CM, CodeModel::Small),
                        OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}
namespace {
// Straight Code Generator Pass Configuration Options.
class StraightPassConfig : public TargetPassConfig {
public:
  StraightPassConfig(StraightTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  StraightTargetMachine &getStraightTargetMachine() const {
    return getTM<StraightTargetMachine>();
  }

  bool addInstSelector() override;

  void addPhiAndSpillPass() override;
  void addCodeGenPass(raw_pwrite_stream& Out) override;
};
}

TargetPassConfig *StraightTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new StraightPassConfig(*this, PM);
}

// Install an instruction selector pass using
// the ISelDag to gen Straight code.
bool StraightPassConfig::addInstSelector() {
  addPass(createStraightISelDag(getStraightTargetMachine()));

  return false;
}

void StraightPassConfig::addPhiAndSpillPass() {
  addPass(new StraightAddJmpAndBasicBlocksPass());
  addPass(new StraightAddPhiAndSpillPass());
  addPass(new StraightAddTrampolinePass());
}

void StraightPassConfig::addCodeGenPass(raw_pwrite_stream& Out) {
  addPass(new StraightCodeGenPass(Out));
  addPass(new StraightEmitGlobalObjectPass(Out));
}
