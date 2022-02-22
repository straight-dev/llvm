#include "StraightOptimizer2/Optimizer2.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace Optimizer2 {

	extern const llvm::TargetInstrInfo* TII;

	std::string Function::print() const {
		std::stringstream sstr;
		sstr << "Function_" << to_string() << " :\n";
		for( const auto& basic_block : basicBlocks ) {
			sstr << basic_block->print();
		}
		return sstr.str();
	}

	std::string BasicBlock::print() const {
		std::stringstream sstr;
		sstr << to_string() << " :\n";
		for( const auto& instruction : instructions ) {
			sstr << instruction->print();
		}
		return sstr.str();
	}

	std::string Instruction::print() const {
		std::string mnemonic = (std::string)TII->getName( static_cast<int>( opcode ) );
		if( mnemonic.substr(0,8) != "IMPLICIT" ) {
			for( auto& c : mnemonic ) {
				if( c == '_' ) { c = '.'; }
				if( mnemonic[0] == 'B' && std::islower( c ) ) { c = std::toupper( c ); }
			}
		}

		std::stringstream sstr;
		sstr << to_string_full() << " ";
		sstr << std::left << std::setw( 9 ) << mnemonic << std::right;
		if( opcode == StraightIROpcode::IMPLICIT_FALL_THROUGH ) {
			sstr << ": ";
;		}

		{
			int reg_operand_num = 0;
			for( const auto& reg_operand : regOperands ) {
				if( reg_operand_num != 0 ) {
					sstr << ", ";
				}
				sstr << "<" << std::setw( 3 ) << reg_operand->getDistanceThroughPhi() << ">";
				++reg_operand_num;
			}
			if( bbOperand ) {
				if( reg_operand_num != 0 ) {
					sstr << ", ";
				}
				sstr << (*bbOperand)->to_string();
			}
			if( immidiateOperand ) {
				if( reg_operand_num != 0 ) {
					sstr << ", ";
				}
				sstr << (int64_t)*immidiateOperand;
			}
			if( linkerTargetOperand ) {
				sstr << *linkerTargetOperand;
			}
		}
		{
			int reg_operand_num = 0;
			for( const auto& reg_operand : regOperands ) {
				if( reg_operand_num == 0 ) {
					const std::size_t n = 41 - std::min<std::size_t>( sstr.str().size(), 41 );
					sstr << std::string( n, ' ' );
					sstr << "# ";
				} else {
					if( opcode == StraightIROpcode::PHI ) {
						sstr << "or";
					} else {
						sstr << ", ";
					}
				}
				sstr << reg_operand->to_string();
				++reg_operand_num;
			}
		}
		{
			int user_num = 0;
			const std::size_t n = 64 - std::min<std::size_t>( sstr.str().size(), 64 );
			sstr << std::string( n, ' ' );
			for( const auto& user : users ) {
				if( user_num == 0 ) {
					sstr << "# user: ";
				} else{
					sstr << ", ";
				}
				sstr << user->getConsumer()->to_string();
				++user_num;
			}
		}
		sstr << "\n";
		return sstr.str();
	}

	std::string Function::to_string() const {
		return function_name;
	}

	std::string BasicBlock::to_string() const {
		return "BB_" + std::to_string( bb_id ) + "@" + parent->to_string();
	}

	std::string Instruction::to_string() const {
		std::stringstream sstr;
		sstr << "[" << extra << std::setw( 5 ) << svreg << "]";
		return sstr.str();
	}
	std::string Instruction::to_string_full() const {
		std::stringstream sstr;
		sstr << " [" << extra << std::setw( 5 ) << svreg << "]";
		return sstr.str();
	}

	std::string RegOperand::to_string() const {
		if( isZeroReg() ) {
			return "ZeroReg ";
		} else {
			return producer->to_string();
		}
	}

} // Optimizer2
