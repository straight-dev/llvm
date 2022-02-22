#include "StraightOptimizer2/Optimizer2.h"
#include "StraightOptimizer2/Transformer.hpp"

namespace Optimizer2 {
	void Transformer::shortCutRegOperand( exempt_ptr<RegOperand> regOperand ) {
		const auto& middle = regOperand->getProducer()->regOperands[0];

		// producerを中間命令から真の利用者命令に変更
		regOperand->producer = middle->getProducer();
		// 通過BBの情報は連結すればよい（中間の命令があるBBは被るのでいらない）
		regOperand->transitBasicBlocks.insert( regOperand->transitBasicBlocks.end(), middle->transitBasicBlocks.begin() + 1, middle->transitBasicBlocks.end() );

		// userを中間の命令ではなく真の利用者命令に変更
		middle->getProducer()->users.insert( regOperand );
	}

	void Transformer::shortCutRegOperandsAll( exempt_ptr<Instruction> instr ) {
		// instrを参照しているオペランド全てに対して実行
		for( const auto user_operand : instr->getUsers() ) {
			shortCutRegOperand( user_operand );
		}
		// instrが参照している命令のuserからinstrを消す
		instr->getOnlyOneRegOperand()->getProducer()->users.erase( instr->getOnlyOneRegOperand() );
	}

	InstructionIterator Transformer::insertNOPBefore( InstructionIterator insert_pos, int svreg_hint ) {
		// NOP命令を作成し、挿入
		return BasicBlock::insertBefore( insert_pos, Instruction::createNOP( insert_pos.getBB(), svreg_hint ) );
	}

	InstructionIterator Transformer::insertRelayRMOVBefore( InstructionIterator insert_pos, exempt_ptr<RegOperand> regOperand, char extra ) {
		// リレー元命令をオペランドにとるRMOV命令を作成し、挿入
		return BasicBlock::insertBefore( insert_pos, Instruction::createRMOV( insert_pos.getBB(), regOperand->getProducer(), extra ) );
	}

	void Transformer::changeOperandToRelayRMOV( exempt_ptr<Instruction> rmov, exempt_ptr<RegOperand> regOperand ) {
		assert( rmov->isRMOV() );
		assert( rmov->getOnlyOneRegOperand()->getProducer() == regOperand->getProducer() );

		const auto phi_target_bb = regOperand->getConsumer()->isPhi() ? regOperand->getTransitBasicBlocks()[1] : exempt_ptr<BasicBlock>( nullptr );

		// リレー先命令はuserではなくなったので削除
		regOperand->getProducer()->users.erase( regOperand );
		// リレー先命令のオペランドをRMOVに変更          ※ここpop_backでできるので高速化可能だけどわかりやすく書いた
		*regOperand = RegOperand( rmov, regOperand->getConsumer(), phi_target_bb );
		// RMOVのuserにリレー先命令を追加
		rmov->users.insert( regOperand );
	}

	void Transformer::moveBeforeBasicBlock( exempt_ptr<Instruction> instr ) {
		const auto iter = InstructionIterator( instr );
		const auto bb = instr->getParent();
		const auto pred_bb = bb->getOnlyOnePredBlock();
		const auto pred_branch = pred_bb->getBranchInstructionTo( bb );
		const auto insert_pos = InstructionIterator( pred_branch ).getFixedRegionTop();

		// 直前のBasicBlockに命令を移動
		const auto delete_target = iter.getIterator();
		BasicBlock::insertBefore( insert_pos, std::move( *iter ) );
		bb->eraseInstruction( delete_target );
		instr->parent = pred_bb;

		// instrのオペランド全てに対して、transitBasicBlocksを一つ縮める
		for( const auto operand : instr->getRegOperands() ) {
			operand->pop_consumerBasicBlock();
		}
		// instrを参照しているオペランド全てに対して、transitBasicBlocksを一つ伸ばす
		for( const auto user_operand : instr->getUsers() ) {
			user_operand->push_producerBasicBlock( pred_bb );
		}
	}

	void Transformer::moveBefore( exempt_ptr<Instruction> instr ) {
		const auto iter_instr = InstructionIterator( instr );

		if( iter_instr.isTopOfBasicBlock() ) {
			// このBasicBlockではこれ以上前に移動できないので、直前のBasicBlockに移動する
			moveBeforeBasicBlock( instr );
		} else {
			const auto insert_pos = (iter_instr - 1).getFixedRegionTop();
			//  some1, some2, some3, instr
			// ^insert_pos          ^iter_instr
			//               ↓
			//  instr, some1, some2, some3
			std::rotate( insert_pos.getIterator(), iter_instr.getIterator(), iter_instr.getIterator() + 1 );
		}
	}

}
