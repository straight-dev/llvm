#include "Straight.h"
#include "StraightTargetMachine.h"
#include "StraightCodeGen.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"

extern char forStraightAddPhiAndSpillPass;
extern char forStraightAddJmpAndBasicBlocksPass;
extern char forStraightAddTrampolinePass;

namespace llvm {

	class StraightAddPhiAndSpillPass : public llvm::MachineFunctionPass {
	public:
		explicit StraightAddPhiAndSpillPass() : MachineFunctionPass(forStraightAddPhiAndSpillPass) {}

		StringRef getPassName() const override {
			return "Straight Add Phi and Spill Pass";
		}

		void getAnalysisUsage(AnalysisUsage &AU) const override {
			AU.addRequired<MachineLoopInfo>();
			MachineFunctionPass::getAnalysisUsage(AU);
		}

		bool runOnMachineFunction(MachineFunction &MF) override;
	};

	class StraightAddJmpAndBasicBlocksPass : public llvm::MachineFunctionPass {
	public:
		explicit StraightAddJmpAndBasicBlocksPass() : MachineFunctionPass(forStraightAddJmpAndBasicBlocksPass) {}

		StringRef getPassName() const override {
			return "Straight Add Jump Instructions And BasicBloks Pass";
		}

		bool runOnMachineFunction(MachineFunction &MF) override;
	};

	class StraightAddTrampolinePass : public llvm::MachineFunctionPass {
	public:
		explicit StraightAddTrampolinePass() : MachineFunctionPass(forStraightAddTrampolinePass) {}

		StringRef getPassName() const override {
			return "Straight Add Trampoline Pass";
		}

		bool runOnMachineFunction(MachineFunction &MF) override;
	};

}
