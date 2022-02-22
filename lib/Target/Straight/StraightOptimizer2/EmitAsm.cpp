#include "StraightOptimizer2/Optimizer2.h"
#include "StraightOptimizer2/TransformPasses.h"

#include "llvm/Support/raw_ostream.h"
#include "StraightInstrInfo.h"

namespace Optimizer2 {
	std::unique_ptr<Function> convertLLVMIRtoStraightAsm( llvm::MachineFunction& );
	const llvm::TargetInstrInfo* TII;
}

void NoOptPrintAsm( llvm::raw_pwrite_stream& Out, llvm::MachineFunction& MF ) {
	const auto func = Optimizer2::convertLLVMIRtoStraightAsm( MF );
	const auto*const TII = MF.getSubtarget().getInstrInfo();
	Optimizer2::TII = TII;

	Optimizer2::TransformPasses::eliminateIMPLICIT_DEF( *func );
	Optimizer2::TransformPasses::eliminateCOPY( *func );

	Optimizer2::TransformPasses::eliminatePHI( *func );

	Optimizer2::TransformPasses::distanceLimit( *func );

	Out << func->print();
}
