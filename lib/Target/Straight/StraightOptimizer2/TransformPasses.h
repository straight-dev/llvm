#ifndef LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_TRANSFORMPASSES_H
#define LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_TRANSFORMPASSES_H

#include "StraightOptimizer2/Optimizer2.h"

namespace Optimizer2 {
	struct TransformPasses {
		static void eliminateIMPLICIT_DEF( const Function& function );
		static void eliminateCOPY( const Function& function );

		static std::vector<InstructionIterator> createFixedRegionBase( const BasicBlock& bb );
		static std::vector<exempt_ptr<Instruction>> createPhiList( const BasicBlock& bb );
		static void eliminatePHI( const Function& function );

		static exempt_ptr<Instruction> findNearestRelayRMOV( exempt_ptr<RegOperand> operand );
		static exempt_ptr<RegOperand> findLongDistanceOperand( const Function& funcation );
		static exempt_ptr<Instruction> findLongDistanceLimitRMOV( exempt_ptr<BasicBlock> bb );
		static void distanceLimitOnlyLimitRMOV( const Function& func );
		static void changeOperandToLimitRMOV( exempt_ptr<RegOperand> operand );
		static void distanceLimit( const Function& function );
	};
}

#endif // LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_TRANSFORMPASSES_H
