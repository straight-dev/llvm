#ifndef LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_OPTIMIZER2_H
#define LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_OPTIMIZER2_H
#include "StraightOptimizer2/Opcode.h"

#include <llvm/ADT/StringRef.h>

#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include <optional>

#include "ptr_util.hpp"
#include <iostream>

#include <set>

namespace llvm {
	class TargetInstrInfo;
	class raw_pwrite_stream;
} // namespace llvm

namespace Optimizer2 {

	class Function;
	class BasicBlock;
	class Instruction;
	class RegOperand;
	class InstructionIterator;
	struct Transformer;

	class LLVMIRtoStraightAsmConverter;

	class Function {
		std::string function_name;

		std::vector<std::unique_ptr<BasicBlock>> basicBlocks;
	public:
		explicit Function( llvm::StringRef function_name ) : function_name( function_name ) {}
		friend LLVMIRtoStraightAsmConverter;
		friend Transformer;


		auto getBasicBlocks() const { return exempt_ptr_Range<std::vector, BasicBlock>( basicBlocks ); }


		std::string print() const;
		std::string to_string() const;
	};

	class BasicBlock {
		int bb_id;
		std::vector<std::unique_ptr<Instruction>> instructions;

		exempt_ptr<Function> parent;
		std::vector<exempt_ptr<BasicBlock>> predBlocks;
		std::vector<exempt_ptr<BasicBlock>> succBlocks;
		// （あれば）succBlocks[0]が無条件分岐先、（あれば）succBlocks[1]が条件分岐先

		template<class Pred>
		void eraseInstructions_if( Pred pred ) { instructions.erase( std::remove_if( instructions.begin(), instructions.end(), pred ), instructions.end() ); }
		auto find( exempt_ptr<const Instruction> instr ) const;
		template<class Pred> auto find_if( Pred pred ) const;

		static InstructionIterator insertBefore( InstructionIterator insert_pos, std::unique_ptr<Instruction>&& instr );
		void eraseInstruction( decltype(instructions.begin()) target );
	public:
		BasicBlock( int bb_id, exempt_ptr<Function> parent ) : bb_id( bb_id ), parent( parent ) {}
		friend LLVMIRtoStraightAsmConverter;
		friend Transformer;
		friend InstructionIterator;

		std::string print() const;
		std::string to_string() const;


		auto getInstructions() const { return exempt_ptr_Range<std::vector, Instruction>( instructions ); }
		const std::vector<exempt_ptr<BasicBlock>>& getPredBlocks() const { return predBlocks; }
		exempt_ptr<BasicBlock> getPredBlock( std::size_t i ) const { return predBlocks[i]; }
		exempt_ptr<BasicBlock> getOnlyOnePredBlock() const { assert( predBlocks.size() == 1 && !predBlocks[0]->containsCall() ); return predBlocks[0]; }
		const std::vector<exempt_ptr<BasicBlock>>& getSuccBlocks() const { return succBlocks; }
		exempt_ptr<BasicBlock> getSuccBlock( std::size_t i ) const { return succBlocks[i]; }


		int countRP( exempt_ptr<const BasicBlock> next_bb ) const;
		int countRP( exempt_ptr<const Instruction> producer, exempt_ptr<const BasicBlock> next_bb ) const;
		int countRP( exempt_ptr<const Instruction> producer, exempt_ptr<const Instruction> consumer ) const;
		int countRP( exempt_ptr<const Instruction> consumer ) const;

		bool containsCall() const;
		exempt_ptr<Instruction> getBranchInstructionTo( exempt_ptr<BasicBlock> branch_target );

		using iterator_t = decltype(instructions)::iterator;
	};

	class Instruction {
		// コードの先頭に付ける人間が読む用の番号
		// StraightVirtualRegister
		int svreg = -1;
		// もともとなかったが最適化によって加えられた命令の区別用
		// 'N' : NOP
		// 'F' : fixed領域のRMOV
		// 'A' : 引数を引き込む用のRMOV
		// 'Z' : 返り値を置くためのRMOV
		// 'L' : 距離を制限するためのRMOV
		char extra;
		StraightIROpcode opcode;

		std::vector<std::unique_ptr<RegOperand>> regOperands;
		std::optional<exempt_ptr<BasicBlock>> bbOperand = std::nullopt;
		std::optional<std::string> linkerTargetOperand = std::nullopt;
		std::optional<uint64_t> immidiateOperand = std::nullopt;


		exempt_ptr<BasicBlock> parent;
		std::set<exempt_ptr<RegOperand>> users;

		// 後で消される用
		bool deleted = false;

		static std::unique_ptr<Instruction> createNOP( exempt_ptr<BasicBlock> create_bb, int svreg_hint = -1 );
		static std::unique_ptr<Instruction> createRMOV( exempt_ptr<BasicBlock> create_bb, exempt_ptr<Instruction> producer, char extra );
	public:
		Instruction( int svreg, StraightIROpcode opcode, exempt_ptr<BasicBlock> parent, char extra = ' ', char extra2 = ' ' ) : svreg( svreg ), extra( extra ), opcode( opcode ), parent( parent ) {}
		friend LLVMIRtoStraightAsmConverter;
		friend Transformer;
		friend RegOperand;

		std::string print() const;
		std::string to_string() const;
		std::string to_string_full() const;

		exempt_ptr<BasicBlock> getParent() const { return parent; }
		int getSvreg() const { return svreg; }
		auto getRegOperands() const { return exempt_ptr_Range<std::vector, RegOperand>( regOperands ); }
		exempt_ptr<RegOperand> getRegOperand( std::size_t i ) const { return make_exempt( regOperands[i] ); }
		exempt_ptr<RegOperand> getOnlyOneRegOperand() const { assert( regOperands.size() == 1 ); return make_exempt( regOperands[0] ); }

		const std::set<exempt_ptr<RegOperand>>& getUsers() const { return users; }

		int RPIncNum() const;
		int getPhiDistance() const;
		bool isCOPY() const { return opcode == StraightIROpcode::COPY || opcode == StraightIROpcode::BITCASTftoi || opcode == StraightIROpcode::BITCASTitof || opcode == StraightIROpcode::BITCASTdtoi || opcode == StraightIROpcode::BITCASTitod; }
		bool isRMOV() const { return opcode == StraightIROpcode::RMOV; }
		bool isPhi() const { return opcode == StraightIROpcode::PHI; }
		bool isBranch() const { return static_cast<bool>(bbOperand); }
		bool isBranchTo( exempt_ptr<const BasicBlock> bb ) const { return bb == bbOperand; }
		bool isImplicitFallthrough() const { return opcode == StraightIROpcode::IMPLICIT_FALL_THROUGH; }
		bool isCall() const { return opcode == StraightIROpcode::JAL || opcode == StraightIROpcode::JALR; }
		bool isImplicitDef() const { return opcode == StraightIROpcode::IMPLICIT_ARG_VALUE || opcode == StraightIROpcode::IMPLICIT_RET_ADDR || opcode == StraightIROpcode::IMPLICIT_RET_VALUE || opcode == StraightIROpcode::IMPLICIT_RET_SPACE; }
		bool isIMPLICIT_DEF() const { return opcode == StraightIROpcode::IMPLICIT_DEF; }
		bool isInFixedRegion() const { return extra == 'F'; }
		bool isLimitRMOV() const { return extra == 'L' && isRMOV(); }
		bool isInArgRegion() const { return extra == 'A'; }
		bool isDeleted() const { return deleted; }
		bool isGlobalAddress() const { return opcode == StraightIROpcode::Global; }

		void changeToNOP() { opcode = StraightIROpcode::NOP; }
	};

	class RegOperand {
		static inline Instruction _zeroReg = Instruction( -1, StraightIROpcode::IMPLICIT_DEF, make_exempt<BasicBlock>( nullptr ), '0' );
		static constexpr exempt_ptr<Instruction> ZeroRegIns = make_exempt( &_zeroReg );
		struct _zeroReg_tag_t {};
		exempt_ptr<Instruction> producer;
		exempt_ptr<Instruction> consumer;
		std::vector<exempt_ptr<BasicBlock>> transitBasicBlocks;

		void pop_consumerBasicBlock();
		void push_producerBasicBlock( exempt_ptr<BasicBlock> bb );
	public:
		static constexpr _zeroReg_tag_t ZeroReg{};
		RegOperand( _zeroReg_tag_t, exempt_ptr<Instruction> consumer );
		RegOperand( exempt_ptr<Instruction> producer, exempt_ptr<Instruction> consumer, exempt_ptr<BasicBlock> phi_target_bb = make_exempt<BasicBlock>( nullptr ) );
		friend Transformer;

		std::string to_string() const;


		exempt_ptr<Instruction> getProducer() const { return producer; }
		exempt_ptr<Instruction> getConsumer() const { return consumer; }
		const std::vector<exempt_ptr<BasicBlock>>& getTransitBasicBlocks() const { return transitBasicBlocks; }
		exempt_ptr<BasicBlock> getPhiIncomingBB() const { assert( getConsumer()->isPhi() && transitBasicBlocks.size() > 1 ); return getTransitBasicBlocks()[1]; }

		int getDistance() const;
		int getDistanceThroughPhi() const;
		bool isZeroReg() const { return producer == ZeroRegIns; }
	};

	class InstructionIterator {
		exempt_ptr<Instruction> instr;
		BasicBlock::iterator_t iterator() const;
	public:
		InstructionIterator() = default;
		explicit InstructionIterator( exempt_ptr<Instruction> instr ) : instr( instr ) {};

		exempt_ptr<BasicBlock> getBB() const { return ( *iterator() )->getParent(); }
		exempt_ptr<Instruction> getIns() const { return make_exempt( *iterator() ); }
		auto getIterator() const { return iterator(); }

		auto& operator*() const { return *iterator(); }
		auto& operator[]( std::ptrdiff_t index ) { return iterator()[index]; }

		bool operator==( InstructionIterator rhs ) const { return iterator() == rhs.iterator(); }
		bool operator!=( InstructionIterator rhs ) const { return iterator() != rhs.iterator(); }

		auto operator++() { instr = make_exempt( *(++iterator() ) ); return *this; }
		auto operator--() { instr = make_exempt( *(--iterator() ) ); return *this; }

		auto operator+( std::ptrdiff_t diff ) const { return InstructionIterator( make_exempt( *( iterator() + diff ) ) ); }
		auto operator-( std::ptrdiff_t diff ) const { return InstructionIterator( make_exempt( *( iterator() - diff ) ) ); }

		InstructionIterator getFixedRegionTop() const;
		bool isTopOfBasicBlock() const;
	};


} // namespace Optimizer2
#endif // LLVMSTRAIGHT_STRAIGHTOPTIMIZER2_OPTIMIZER2_H
