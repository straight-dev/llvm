#pragma once
#include "Straight.h"
#include "StraightTargetMachine.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/Support/raw_ostream.h"
extern char forStraightCodeGen;
extern char forStraightEmitGlobalObject;

namespace llvm {

	class StraightCodeGenPass : public llvm::MachineFunctionPass {
		raw_pwrite_stream& out;
	public:
		explicit StraightCodeGenPass(raw_pwrite_stream& Out) : MachineFunctionPass(forStraightCodeGen), out(Out) {}

		StringRef getPassName() const override {
			return "Straight Code Generation from SSA form";
		}

		bool runOnMachineFunction(MachineFunction &MF) override {
			StraightCodeGen_impl(out, MF); 
			return false;
		}

	private:
		void StraightCodeGen_impl(llvm::raw_pwrite_stream& Out, llvm::MachineFunction& MF) const;
	};

	class StraightEmitGlobalObjectPass : public llvm::ModulePass {
		raw_pwrite_stream& out;
	public:
		explicit StraightEmitGlobalObjectPass(raw_pwrite_stream& Out) : ModulePass(forStraightEmitGlobalObject), out(Out) {}

		StringRef getPassName() const override {
			return "Straight Emit Global Object";
		}

		bool runOnModule(Module &M) override {
			out << "# Global objects\n";
			StraightEmitGlobalObject_impl(out, M);
			out << "# Alias List\n";
			StraightEmitAliasList_impl(out, M);
			return false;
		}
	private:
		void StraightEmitGlobalObject_impl(llvm::raw_pwrite_stream& Out, llvm::Module& M) const;
		void StraightEmitAliasList_impl(llvm::raw_pwrite_stream& Out, llvm::Module& M) const;
	};
}
