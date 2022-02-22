#include "StraightOptimizer2/Optimizer2.h"

#include "StraightOpTraits.h"
#include "StraightRegisterInfo.h"
#include "MCTargetDesc/StraightMCTargetDesc.h"

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/IR/GlobalValue.h"


#include "StraightPhiPass.h"

#include <map>


namespace Optimizer2 {

	// --- utility ---

	static int getRetRegNum( const llvm::MachineOperand& operand ) {
		if( !operand.isReg() ) { return -1; }
		switch( operand.getReg() ) {
		case llvm::Straight::RET0: return 0;
		case llvm::Straight::RET1: return 1;
		case llvm::Straight::RET2: return 2;
		case llvm::Straight::RET3: return 3;
		case llvm::Straight::RET4: return 4;
		case llvm::Straight::RET5: return 5;
		case llvm::Straight::RET6: return 6;
		case llvm::Straight::RET7: return 7;
		case llvm::Straight::RET8: return 8;
		case llvm::Straight::RET9: return 9;
		default:
			return -1;
		}
	}

	static bool isRetReg( const llvm::MachineOperand& operand ) {
		return getRetRegNum( operand ) >= 0;
	}

	[[maybe_unused]]
	static int getRegOperandStartIndex( const llvm::MachineInstr& instr ) {
		if( instr.getOpcode() == llvm::Straight::JALR ) {
			return 0;
		} else if( instr.getOpcode() == llvm::Straight::JR ) {
			for( int i = 0; ; ++i ) {
				assert( instr.getOperand(i).isReg() );
				if( !isRetReg( instr.getOperand(i) ) ) {
					return i;
				}
			}
		} else if( llvm::Straight::is_store( instr ) ) {
			return 0;
		} else {
			return 1;
		}
	}

	static StraightIROpcode toSPIns( StraightIROpcode opcode ) {
		switch( opcode ) {
		case StraightIROpcode::LD_8  : return StraightIROpcode::SPLD_8  ;
		case StraightIROpcode::LD_8u : return StraightIROpcode::SPLD_8u ;
		case StraightIROpcode::LD_16 : return StraightIROpcode::SPLD_16 ;
		case StraightIROpcode::LD_16u: return StraightIROpcode::SPLD_16u;
		case StraightIROpcode::LD_32 : return StraightIROpcode::SPLD_32 ;
		case StraightIROpcode::LD_32u: return StraightIROpcode::SPLD_32u;
		case StraightIROpcode::LD_f32: return StraightIROpcode::SPLD_f32;
		case StraightIROpcode::LD_64 : return StraightIROpcode::SPLD_64 ;
		case StraightIROpcode::ST_8  : return StraightIROpcode::SPST_8  ;
		case StraightIROpcode::ST_16 : return StraightIROpcode::SPST_16 ;
		case StraightIROpcode::ST_32 : return StraightIROpcode::SPST_32 ;
		case StraightIROpcode::ST_64 : return StraightIROpcode::SPST_64 ;
		default: assert( !"SP in not memroy access instruction!" );
		}
	}

	static StraightIROpcode toStraightIROpcode( unsigned opcode ) {
		// pseudo instructions
		if( opcode == llvm::Straight::ST_f32 ) { return StraightIROpcode::ST_32; }
		if( opcode == llvm::Straight::ST_f64 ) { return StraightIROpcode::ST_64; }
		if( opcode == llvm::Straight::LD_f64 ) { return StraightIROpcode::LD_64; }
		return static_cast<StraightIROpcode>( opcode );
	}

	// ======

	class LLVMIRtoStraightAsmConverter {
		using LLVM_physical_register = unsigned int;
		using LLVM_machine_basic_block = unsigned int;

		std::map<llvm::Register, exempt_ptr<Instruction>> vreg2instr;
		std::map<std::pair<LLVM_machine_basic_block, LLVM_physical_register>, exempt_ptr<Instruction>> arg2instr;
		std::map<std::pair<LLVM_machine_basic_block, LLVM_physical_register>, exempt_ptr<Instruction>> ret2instr;
		std::map<LLVM_machine_basic_block, exempt_ptr<BasicBlock>> mbb2strbb;

		const llvm::MachineFunction& MF;
		std::unique_ptr<Function> func;


		static std::pair<const llvm::MachineOperand&, const llvm::MachineOperand&> getPhiTarget( const llvm::MachineBasicBlock* pred_bb, const llvm::MachineInstr& instruction );


		void reserveSpace();

		void connectBasicBlocks() const;

		void connectInstructions() const;
			void connectConditionalBranchInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const;
			void connectUnconditionalBranchInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr, bool is_fall_through ) const;
			void connectCallInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr, LLVM_machine_basic_block bb_id ) const;
			void connectReturnInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const;
			void correctReturnValueInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const;
			void connectPhiInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr, const llvm::MachineBasicBlock& ) const;
			void connectStoreInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const;
			void correctGlobalAddressInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const;
			void connectSPADDiInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const;
			void connectNormalInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const;
			void correctCallArgument( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr, LLVM_machine_basic_block bb_id ) const;
				void setBBOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const;
				void setLinkerTargetOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const;
				void connectOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const;
					void setReturnedValueOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const;
					void setRegisterOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const;
					void setImmidiateOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const;


	public:
		explicit LLVMIRtoStraightAsmConverter( llvm::MachineFunction& MF );
		std::unique_ptr<Function> getConvertedFunction();
	};

	LLVMIRtoStraightAsmConverter::LLVMIRtoStraightAsmConverter( llvm::MachineFunction& MF ) : MF( MF ), func( std::make_unique<Function>( MF.getName() ) ) {
		// 情報収集とアドレスの確保(相互参照があるので先に確保しないとダメ、一括でできない)
		reserveSpace();
		// まずBasicBlockのつながり(相互参照)を構築
		connectBasicBlocks();
		// つぎにInstructionのつながり(相互参照、BasicBlockのつながりも使う)を構築
		connectInstructions();
	}

	std::unique_ptr<Function> LLVMIRtoStraightAsmConverter::getConvertedFunction() {
		return std::move( func );
	}

	// ======

	void LLVMIRtoStraightAsmConverter::reserveSpace() {
		int straight_virtual_register_serial_number = 0;
		for( const auto& basic_block : MF ) {
			const LLVM_machine_basic_block bb_id = basic_block.getNumber();
			func->basicBlocks.push_back( std::make_unique<BasicBlock>( bb_id, make_exempt( func ) ) );
			const exempt_ptr<BasicBlock> bb = make_exempt( func->basicBlocks.back() );
			mbb2strbb.emplace( bb_id, bb );

			if( func->basicBlocks.size() == 1 ) {
				// 先頭のBasicBlockの前に引数の暗黙定義を追加
				//　後の方の引数ほど先に定義されるので逆順に走査
				const auto& MRI = MF.getRegInfo();
				for( auto iter = std::make_reverse_iterator(MRI.livein_end()); iter != std::make_reverse_iterator(MRI.livein_begin()); ++iter ) {
					bb->instructions.push_back( std::make_unique<Instruction>( straight_virtual_register_serial_number++, StraightIROpcode::IMPLICIT_ARG_VALUE, bb ) );
					const exempt_ptr<Instruction> instr = make_exempt( bb->instructions.back() );
					vreg2instr.emplace( (*iter).first, instr );
					// 関数の引数が未使用だとllvmの最適化で消えてしまっていることがあるが、呼び出し規約のために復活させる
					// 末尾の引数が未使用の場合は検出できないが、呼び出し規約が壊れるわけではないので問題なし
					unsigned next = iter + 1 != std::make_reverse_iterator(MRI.livein_begin()) ? static_cast<unsigned>((*(iter + 1)).first) : llvm::Straight::ARG0 - 1;
					int diff = (*iter).first - next;
					assert( diff > 0 );
					// diffが2以上の時、間が飛んでいるので復活させる
					for( int i = 1; i < diff; ++i ) {
						bb->instructions.push_back( std::make_unique<Instruction>( straight_virtual_register_serial_number++, StraightIROpcode::IMPLICIT_ARG_VALUE, bb ) );
						const exempt_ptr<Instruction> instr = make_exempt( bb->instructions.back() );
						vreg2instr.emplace( (*iter).first - i, instr );
					}
				}
				// 積まれたリターンアドレス
				bb->instructions.push_back( std::make_unique<Instruction>( straight_virtual_register_serial_number++, StraightIROpcode::IMPLICIT_RET_ADDR, bb ) );
				const exempt_ptr<Instruction> retaddr = make_exempt( bb->instructions.back() );
				vreg2instr.emplace( llvm::Straight::RETADDR, retaddr );
			} else if( func->basicBlocks.end()[-2]->containsCall() ) {
				// 関数呼び出しから帰ってきたBasicBlockに返り値を定義する暗黙定義を追加
				for( const auto& instruction : basic_block ) {
					if( instruction.getOpcode() == llvm::Straight::COPY && isRetReg( instruction.getOperand( 1 ) ) ) {
						bb->instructions.push_back( std::make_unique<Instruction>( straight_virtual_register_serial_number++, StraightIROpcode::IMPLICIT_RET_VALUE, bb ) );
						auto instr = make_exempt(bb->instructions.back());
						ret2instr.emplace( std::make_pair( bb_id, instruction.getOperand( 1 ).getReg() ), instr );
					}
				}
				// リターン命令のスペースを確保
				bb->instructions.push_back( std::make_unique<Instruction>( straight_virtual_register_serial_number++, StraightIROpcode::IMPLICIT_RET_SPACE, bb ) );
			}

			for( const auto& instruction : basic_block ) {
				// デバッグ情報付きで作ったllvmIRだと出てくるがとりあえずつぶす
				if( static_cast<StraightIROpcode>( instruction.getOpcode() ) == StraightIROpcode::DBG_VALUE ) { continue; }
				bb->instructions.push_back( std::make_unique<Instruction>( straight_virtual_register_serial_number++, toStraightIROpcode( instruction.getOpcode() ), bb ) );
				const exempt_ptr<Instruction> instr = make_exempt( bb->instructions.back() );
				if( llvm::Straight::has_dstReg( instruction ) ) {
					auto& dest = instruction.getOperand( 0 );
					if( dest.getReg().isVirtual() ) {
						const llvm::Register instr_id = dest.getReg();
						vreg2instr.emplace( instr_id, instr );
					} else {
						// デスティネーションが実レジスタ（返り値の設定）
						const llvm::Register instr_id = dest.getReg();
						arg2instr.emplace( std::make_pair( bb_id, instr_id ), instr );
					}
				}
			}
		}
	}

	// ======

	void LLVMIRtoStraightAsmConverter::connectBasicBlocks() const {
		auto iter_this_bb = func->basicBlocks.begin();
		for( const auto& basic_block : MF ) {
			for( const auto*const pred_block : basic_block.predecessors() ) {
				const LLVM_machine_basic_block pred_bb_id = pred_block->getNumber();
				(*iter_this_bb)->predBlocks.push_back( mbb2strbb.at( pred_bb_id ) );
			}
			// succ_block を参照することはせず、分岐命令のオペランドを確認することで構成する
			// これは、（あれば）succBlocks[0]が無条件分岐先、（あれば）succBlocks[1]が条件分岐先、としたいため
			/*
			for( const auto*const succ_block : basic_block.successors() ) {
				const LLVM_machine_basic_block succ_bb_id = succ_block->getNumber();
				(*iter_this_bb)->succBlocks.push_back( mbb2strbb.at( succ_bb_id ) );
			}
			*/
			++iter_this_bb;
		}
	}

	// ======

	std::pair<const llvm::MachineOperand&, const llvm::MachineOperand&> LLVMIRtoStraightAsmConverter::getPhiTarget( const llvm::MachineBasicBlock* pred_bb, const llvm::MachineInstr& instruction ) {
		// dest, src1, src1BB, src2, src2BB, ...

		for( unsigned int i = 1; ; i += 2 ) {
			const auto& operand = instruction.getOperand( i );
			const auto& bbOperand = instruction.getOperand( i + 1 );
			assert( operand.isReg() );
			assert( bbOperand.isMBB() );

			if( bbOperand.getMBB() == pred_bb ) {
				return { operand, bbOperand };
			}
		}
	}

	// ======

	void LLVMIRtoStraightAsmConverter::correctCallArgument( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr, LLVM_machine_basic_block bb_id ) const {
		const exempt_ptr<Instruction> producer = arg2instr.at( std::make_pair( bb_id, operand.getReg() ) );
		assert( producer->opcode == StraightIROpcode::COPY );
		assert( producer->regOperands.size() == 1 );

		// 暗黙利用の場合、usersには追加しない
		producer->opcode = StraightIROpcode::RMOV;
		producer->extra = 'A';
	}

	void LLVMIRtoStraightAsmConverter::setReturnedValueOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const {
		for( exempt_ptr<BasicBlock> transit_bb = instr->parent; ; ) {
			if( transit_bb->instructions[0]->opcode == StraightIROpcode::IMPLICIT_RET_VALUE ) {
				const exempt_ptr <Instruction> producer = ret2instr.at( std::make_pair(transit_bb->bb_id, operand.getReg()) );
				assert( producer->opcode == StraightIROpcode::IMPLICIT_RET_VALUE );

				instr->regOperands.push_back( std::make_unique<RegOperand>( producer, instr ) );
				return;
			}

			// 一本道で持って来ることを要求
			// この制約は 生存解析を行うAddPhiPassを通していれば満たされるはず
			assert( transit_bb->predBlocks.size() == 1 );
			transit_bb = transit_bb->predBlocks[0];
		}
	}

	void LLVMIRtoStraightAsmConverter::setRegisterOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const {
		if( operand.getReg() == llvm::Straight::ZeroReg ) {
			instr->regOperands.push_back( std::make_unique<RegOperand>( RegOperand::ZeroReg, instr ) );
		} else if( operand.getReg() == llvm::Straight::SP ) {
			instr->opcode = toSPIns( instr->opcode );
		} else {
			const exempt_ptr<Instruction> producer = vreg2instr.at( operand.getReg() );
			instr->regOperands.push_back( std::make_unique<RegOperand>( producer, instr ) );
			producer->users.insert( make_exempt( instr->regOperands.back() ) );
		}
	}

	void LLVMIRtoStraightAsmConverter::setImmidiateOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const {
		instr->immidiateOperand = operand.getImm();
	}

	void LLVMIRtoStraightAsmConverter::connectOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const {
		if( operand.isReg() ) {
			if( isRetReg( operand ) ) {
				setReturnedValueOperand( operand, instr );
			} else {
				setRegisterOperand( operand, instr );
			}
		}
		if( operand.isImm() ) {
			setImmidiateOperand( operand, instr );
		}
	}

	void LLVMIRtoStraightAsmConverter::setBBOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const {
		assert( operand.isMBB() );
		instr->bbOperand = mbb2strbb.at( operand.getMBB()->getNumber() );
	}

	void LLVMIRtoStraightAsmConverter::setLinkerTargetOperand( const llvm::MachineOperand& operand, exempt_ptr<Instruction> instr ) const {
		if( operand.isGlobal() ) {
			instr->linkerTargetOperand = "Function_" + std::string( operand.getGlobal()->getName() );
		} else if( operand.isSymbol() ) {
			instr->linkerTargetOperand = "Function_" + std::string( operand.getSymbolName() );
		} else {
			assert( !"No function name!" );
		}
	}

	// ======

	void LLVMIRtoStraightAsmConverter::connectConditionalBranchInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const {
		assert( instruction.getNumOperands() == 3 );
		connectOperand( instruction.getOperand( 0 ), instr );
		connectOperand( instruction.getOperand( 1 ), instr );
		setBBOperand( instruction.getOperand( 2 ), instr );

		// succBlocks[1] は 条件分岐先
		const auto*const succ_block = instruction.getOperand( 2 ).getMBB();
		const LLVM_machine_basic_block succ_bb_id = succ_block->getNumber();
		instr->parent->succBlocks.push_back( mbb2strbb.at( succ_bb_id ) );
	}

	void LLVMIRtoStraightAsmConverter::connectUnconditionalBranchInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr, bool is_fall_through ) const {
		assert( instruction.getNumOperands() == 1 );
		setBBOperand( instruction.getOperand( 0 ), instr );
		if( is_fall_through ) {
			instr->opcode = StraightIROpcode::IMPLICIT_FALL_THROUGH;
		}

		// succBlocks[0] は 無条件分岐先
		const auto*const succ_block = instruction.getOperand( 0 ).getMBB();
		const LLVM_machine_basic_block succ_bb_id = succ_block->getNumber();
		instr->parent->succBlocks.insert( instr->parent->succBlocks.begin(), mbb2strbb.at( succ_bb_id ) );
	}

	void LLVMIRtoStraightAsmConverter::connectCallInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr, LLVM_machine_basic_block bb_id ) const {
		if( instr->opcode == StraightIROpcode::JAL ) {
			// 普通の関数コール、関数名の情報がオペランドになっている
			setLinkerTargetOperand( instruction.getOperand( 1 ), instr );
			// Call命令のレジスタオペランド扱いの引数をSTRAIGHT流の定義に修正
			for( unsigned int i = 2; i < instruction.getNumOperands(); ++i ) {
				correctCallArgument( instruction.getOperand( i ), instr, bb_id );
			}
		} else {
			assert( instr->opcode == StraightIROpcode::JALR );
			// レジスタ間接関数コール
			connectOperand( instruction.getOperand( 1 ), instr );
			setImmidiateOperand( instruction.getOperand( 2 ), instr );
			// Call命令のレジスタオペランド扱いの引数をSTRAIGHT流の定義に修正
			for( unsigned int i = 3; i < instruction.getNumOperands(); ++i ) {
				correctCallArgument( instruction.getOperand( i ), instr, bb_id );
			}
		}
	}

	void LLVMIRtoStraightAsmConverter::connectReturnInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const {
		const unsigned int n = instruction.getNumOperands();
		for( unsigned int i = 2; i < n - 1; ++i ) {
			assert( isRetReg( instruction.getOperand( i ) ) );
		}
		assert( instruction.getOperand( 0 ).isReg() );
		assert( instruction.getOperand( 1 ).isImm() );
		setRegisterOperand( instruction.getOperand( 0 ), instr );
		setImmidiateOperand( instruction.getOperand( 1 ), instr );
	}

	void LLVMIRtoStraightAsmConverter::correctReturnValueInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const {
		assert( instr->opcode == StraightIROpcode::COPY );
		const exempt_ptr<Instruction> producer = vreg2instr.at( instruction.getOperand( 1 ).getReg() );

		instr->opcode = StraightIROpcode::RMOV;
		instr->extra = 'Z';
		instr->regOperands.push_back( std::make_unique<RegOperand>( producer, instr ) );
		producer->users.insert( make_exempt( instr->regOperands[0] ) );
	}

	void LLVMIRtoStraightAsmConverter::connectStoreInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const {
		assert( instruction.getNumOperands() == 3 );
		assert( instruction.getOperand( 0 ).isReg() ); // ストア値
		assert( instruction.getOperand( 1 ).isReg() ); // アドレス
		assert( instruction.getOperand( 2 ).isImm() ); // オフセット

		setRegisterOperand( instruction.getOperand( 0 ), instr );
		setRegisterOperand( instruction.getOperand( 1 ), instr );
		setImmidiateOperand( instruction.getOperand( 2 ), instr );
	}

	void LLVMIRtoStraightAsmConverter::correctGlobalAddressInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const {
		assert( instruction.getNumOperands() == 2 );
		assert( instruction.getOperand( 1 ).isGlobal() );
		instr->linkerTargetOperand = std::string( instruction.getOperand( 1 ).getGlobal()->getName() );
	}

	void LLVMIRtoStraightAsmConverter::connectPhiInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr, const llvm::MachineBasicBlock& basic_block ) const {
		for( const auto*const pred_bb : basic_block.predecessors() ) {
			// PHI命令のオペランドの順番とpred_bbの順番は異なる可能性があるので、StraightAsm形式ではそろえる
			const auto[operand, bbOperand] = getPhiTarget( pred_bb, instruction );
			const exempt_ptr<Instruction> producer = vreg2instr.at( operand.getReg() );
			const exempt_ptr<BasicBlock> phi_target_bb = mbb2strbb.at( bbOperand.getMBB()->getNumber() );

			instr->regOperands.push_back( std::make_unique<RegOperand>( producer, instr, phi_target_bb ) );
			producer->users.insert( make_exempt( instr->regOperands.back() ) );
		}
	}

	void LLVMIRtoStraightAsmConverter::connectSPADDiInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const {
		assert( instruction.getNumOperands() == 1 );
		assert( instruction.getOperand( 0 ).isImm() ); // 加算値
		setImmidiateOperand( instruction.getOperand( 0 ), instr );
	}

	void LLVMIRtoStraightAsmConverter::connectNormalInstruction( const llvm::MachineInstr& instruction, exempt_ptr<Instruction> instr ) const {
		for( unsigned int i = 1; i < instruction.getNumOperands(); ++i ) {
			connectOperand( instruction.getOperand( i ), instr );
		}
	}

	// ======

	void LLVMIRtoStraightAsmConverter::connectInstructions() const {
		auto iter_this_bb = func->basicBlocks.begin();
		for( const auto& basic_block : MF ) {
			if( (*iter_this_bb)->instructions.empty() ) {
				++iter_this_bb;
				continue;
			}

			const LLVM_machine_basic_block bb_id = basic_block.getNumber();
			auto iter_this_instruction = (*iter_this_bb)->instructions.begin();
			while( iter_this_instruction != (*iter_this_bb)->instructions.end() && (*iter_this_instruction)->isImplicitDef() ) { ++iter_this_instruction; }

			for( const auto& instruction : basic_block ) {
				if( static_cast<StraightIROpcode>( instruction.getOpcode() ) == StraightIROpcode::DBG_VALUE ) {
					continue;
				}

				const exempt_ptr<Instruction> instr = make_exempt( *iter_this_instruction );

				if( instruction.isConditionalBranch() ) {
					// 条件分岐命令
					connectConditionalBranchInstruction( instruction, instr );
				} else if( instruction.isUnconditionalBranch() ) {
					// 無条件分岐命令
					const bool is_fall_through = (iter_this_bb+1) != func->basicBlocks.end() && mbb2strbb.at( instruction.getOperand( 0 ).getMBB()->getNumber() ) == iter_this_bb[1];
					connectUnconditionalBranchInstruction( instruction, instr, is_fall_through );
				} else if( instruction.isCall() ) {
					// 関数コール・レジスタ間接関数コール
					connectCallInstruction( instruction, instr, bb_id );
				} else if( instruction.isReturn() ) {
					// リターン命令
					connectReturnInstruction( instruction, instr );
				} else if( isRetReg( instruction.getOperand( 0 ) ) ) {
					// リターン値生成命令
					correctReturnValueInstruction( instruction, instr );
				} else if( llvm::Straight::is_store( instruction ) ) {
					// ストア命令
					connectStoreInstruction( instruction, instr );
				} else if( instruction.getOpcode() == llvm::Straight::Global ) {
					// グローバル変数のアドレス取得命令
					correctGlobalAddressInstruction( instruction, instr );
				} else if( instr->isPhi() ) {
					// phi命令
					connectPhiInstruction( instruction, instr, basic_block );
				} else if( instr->opcode == StraightIROpcode::SPADDi ) {
					// SPADDi命令
					connectSPADDiInstruction( instruction, instr );
				} else {
					// 普通の命令
					connectNormalInstruction( instruction, instr );
				}

				++iter_this_instruction;
			}
			assert( iter_this_instruction == (*iter_this_bb)->instructions.end() );
			++iter_this_bb;
		}
		assert( iter_this_bb == func->basicBlocks.end() );
	}

	std::unique_ptr<Function> convertLLVMIRtoStraightAsm( llvm::MachineFunction& MF ) {
		LLVMIRtoStraightAsmConverter x( MF );
		return x.getConvertedFunction();
	}


} // Optimizer2
