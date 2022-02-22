#ifndef LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_TRANSFORMER_H
#define LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_TRANSFORMER_H

#include "StraightOptimizer2/Optimizer2.h"

namespace Optimizer2 {
	struct Transformer {
		template<class Pred>
		static void eraseInstructions_if( const Function& function, Pred predicate ) {
			for( const auto basic_block : function.getBasicBlocks() ) {
				basic_block->eraseInstructions_if( predicate );
			}
		}

		static void shortCutRegOperand( exempt_ptr<RegOperand> regOperand );
		static void shortCutRegOperandsAll( exempt_ptr<Instruction> instr );

		static InstructionIterator insertNOPBefore( InstructionIterator insert_pos, int hint_svreg = -1 );
		static InstructionIterator insertRelayRMOVBefore( InstructionIterator insert_pos, exempt_ptr<RegOperand> regOperand, char extra );
		static void changeOperandToRelayRMOV( exempt_ptr<Instruction> rmov, exempt_ptr<RegOperand> regOperand );

		static void moveBeforeBasicBlock( exempt_ptr<Instruction> instr );
		static void moveBefore( exempt_ptr<Instruction> instr );

	};
}

#endif // LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_TRANSFORMER_H
