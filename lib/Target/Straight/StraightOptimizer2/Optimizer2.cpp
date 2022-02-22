#include "StraightOptimizer2/Optimizer2.h"

#include <algorithm>
#include <numeric>

namespace Optimizer2 {
	bool BasicBlock::containsCall() const {
		const auto it = std::find_if( instructions.begin(), instructions.end(), []( auto& instr ) { return instr->isCall(); } );
		return it != instructions.end();
	}

	auto BasicBlock::find( exempt_ptr<const Instruction> instr ) const {
		return std::find( instructions.begin(), instructions.end(), instr );
	}

	template<class Pred>
	auto BasicBlock::find_if( Pred pred ) const {
		return std::find_if( instructions.begin(), instructions.end(), pred );
	}

	int BasicBlock::countRP( exempt_ptr<const BasicBlock> next_bb ) const {
		const auto branch_inst = find_if( [next_bb]( const auto& instr ) { return instr->isBranchTo( next_bb ); } );
		assert( branch_inst != instructions.end() );

		return std::accumulate( instructions.begin(), branch_inst+1, 0, []( int a, const auto& instr ) { return a + instr->RPIncNum(); } );
	}

	int BasicBlock::countRP( exempt_ptr<const Instruction> producer, exempt_ptr<const BasicBlock> next_bb ) const {
		const auto producer_inst = find( producer );
		const auto branch_inst = find_if( [next_bb]( const auto& instr ) { return instr->isBranchTo( next_bb ); } );
		assert( producer_inst != instructions.end() );
		assert( branch_inst != instructions.end() );

		const int GlobalAddressAdjust = producer->isGlobalAddress() ? -1 : 0;
		return std::accumulate( producer_inst, branch_inst + 1, GlobalAddressAdjust, []( int a, const auto& instr ) { return a + instr->RPIncNum(); } );
	}

	int BasicBlock::countRP( exempt_ptr<const Instruction> producer, exempt_ptr<const Instruction> consumer ) const {
		const auto producer_inst = find( producer );
		const auto consumer_inst = find( consumer );
		assert( producer_inst != instructions.end() );
		assert( consumer_inst != instructions.end() );
		const int GlobalAddressAdjust = producer->isGlobalAddress() ? -1 : 0;
		return std::accumulate( producer_inst, consumer_inst, GlobalAddressAdjust, []( int a, const auto& instr ) { return a + instr->RPIncNum(); } );
	}

	int BasicBlock::countRP( exempt_ptr<const Instruction> consumer ) const {
		const auto consumer_inst = find( consumer );
		assert( consumer_inst != instructions.end() );

		return std::accumulate( instructions.begin(), consumer_inst, 0, []( int a, const auto& instr ) { return a + instr->RPIncNum(); } );
	}

	int Instruction::RPIncNum() const {
		if( isPhi() ) {
            return 0;
		} else if( opcode == StraightIROpcode::IMPLICIT_FALL_THROUGH ) {
			return 0;
		} else if( opcode == StraightIROpcode::IMPLICIT_DEF ) {
			return 0;
		} else if( opcode == StraightIROpcode::Global ) {
			return 2; // LUi + ADDiに変換されるため
		} else {
			return 1;
		}
	}

	int Instruction::getPhiDistance() const {
		assert( isPhi() );
		return regOperands[0]->getDistanceThroughPhi();
	}


	int RegOperand::getDistance() const {
		assert( !transitBasicBlocks.empty() );
		if( isZeroReg() ) {
			return 0;
		}

		if( transitBasicBlocks.size() == 1 ) {
			return transitBasicBlocks[0]->countRP( producer, consumer );
		} else {
			int distance = transitBasicBlocks[0]->countRP( consumer );
			for( std::size_t i = 1; i < transitBasicBlocks.size() - 1; ++i ) {
				distance += transitBasicBlocks[i]->countRP( transitBasicBlocks[i-1] );
			}
			return distance + transitBasicBlocks.back()->countRP( producer, transitBasicBlocks.end()[-2] );
		}
	}

	int RegOperand::getDistanceThroughPhi() const {
		if( producer->isPhi() ) {
            return producer->getPhiDistance() + getDistance();
		} else {
			return getDistance();
		}
	}

	RegOperand::RegOperand( exempt_ptr<Instruction> producer, exempt_ptr<Instruction> consumer, exempt_ptr<BasicBlock> phi_target_bb ) 
		: producer( producer ), consumer( consumer ) {

		const exempt_ptr<BasicBlock> first = consumer->parent;
		const exempt_ptr<BasicBlock> last = producer->parent;

		// PHI命令の場合は、最初の一回に限りBasicBlockを遡るときに候補が複数ある
		bool phi_first_branch = static_cast<bool>(phi_target_bb);
		for( exempt_ptr<BasicBlock> transit_bb = first; phi_first_branch || transit_bb != last; ) {
			transitBasicBlocks.push_back( transit_bb );
			if( phi_first_branch ) {
				phi_first_branch = false;
				transit_bb = phi_target_bb;
			} else {
				// 一本道で持って来ることを要求
				// この制約は 生存解析を行うAddPhiPassを通していれば満たされるはず
				transit_bb = transit_bb->getOnlyOnePredBlock();
			}
		}
		transitBasicBlocks.push_back( last );
	}

	RegOperand::RegOperand( RegOperand::_zeroReg_tag_t, exempt_ptr<Instruction> consumer )
		: producer( RegOperand::ZeroRegIns ), consumer( consumer ), transitBasicBlocks { consumer->parent } {
		// do nothing
	}

	std::unique_ptr<Instruction> Instruction::createNOP( exempt_ptr<BasicBlock> create_bb, int svreg_hint ) {
		auto nop = std::make_unique<Instruction>( svreg_hint, StraightIROpcode::NOP, create_bb, 'N' );
		return nop;
	}

	std::unique_ptr<Instruction> Instruction::createRMOV( exempt_ptr<BasicBlock> create_bb, exempt_ptr<Instruction> producer, char extra ) {
		std::unique_ptr<Instruction> rmov = std::make_unique<Instruction>( producer->svreg, StraightIROpcode::RMOV, create_bb, extra );
		rmov->regOperands.push_back( std::make_unique<RegOperand>( producer, make_exempt( rmov ) ) );
		producer->users.insert( rmov->getOnlyOneRegOperand() );
		return rmov;
	}

	InstructionIterator BasicBlock::insertBefore( InstructionIterator insert_pos, std::unique_ptr<Instruction>&& instr ) {
		const auto it = insert_pos.getBB()->instructions.insert( insert_pos.getIterator(), std::move( instr ) );
		return InstructionIterator( make_exempt( *it ) );
	}

	void BasicBlock::eraseInstruction( BasicBlock::iterator_t target ) {
		instructions.erase( target );
	}

	exempt_ptr<Instruction> BasicBlock::getBranchInstructionTo( exempt_ptr<BasicBlock> branch_target ) {
		const auto instr = getInstructions().find_if( [branch_target]( const auto& ins ) { return ins->isBranchTo( branch_target ); } );
		assert( instr );
		return instr;
	}

	void RegOperand::pop_consumerBasicBlock() {
		transitBasicBlocks.erase( transitBasicBlocks.begin() ); // [pop_front]
		assert( !transitBasicBlocks.empty() );
	}
	void RegOperand::push_producerBasicBlock( exempt_ptr<BasicBlock> bb ) {
		transitBasicBlocks.push_back( bb );
	}



	BasicBlock::iterator_t InstructionIterator::iterator() const {
		const auto bb = instr->getParent();

		return std::find( bb->instructions.begin(), bb->instructions.end(), instr );
	}

	InstructionIterator InstructionIterator::getFixedRegionTop() const {
		const auto bb = (*iterator())->getParent();
		auto it = iterator();
		while( it != bb->getInstructions().begin() && it[-1]->isInFixedRegion() ) {
			--it;
		}
		return InstructionIterator( make_exempt( *it ) );
	}

	bool InstructionIterator::isTopOfBasicBlock() const {
		return iterator() == (*iterator())->getParent()->getInstructions().begin();
	}

}
