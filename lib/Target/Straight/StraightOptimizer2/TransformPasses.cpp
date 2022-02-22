#include "StraightOptimizer2/TransformPasses.h"
#include "StraightOptimizer2/Transformer.hpp"
#include "StraightOptimizer2/RouteRange.h"

namespace Optimizer2 {
	// ====== eliminateIMPLICIT_DEF ======
	void TransformPasses::eliminateIMPLICIT_DEF( const Function& function ) {
		// IMPLICT_DEFのままだとRPが増えないので「命令順を入れ替えたが距離が縮まない」ということが起こって面倒なのでNOPに変換する
		// ただし、引数定義領域内に存在するIMPLICIT_DEFはNOPに変換すると呼び出し規約が壊れるので残す
		for( const auto basic_block : function.getBasicBlocks() ) {
			for( const auto instruction : basic_block->getInstructions() ) {
				if( instruction->isInArgRegion() ) {
					break; // 引数定義領域以降のIMPLICIT_DEFは変換しない（このBasicBlockのみ）
				}
				if( instruction->isIMPLICIT_DEF() ) {
					instruction->changeToNOP();
				}
			}
		}
	}

	// ====== eliminateCOPY ======
	void TransformPasses::eliminateCOPY( const Function& function ) {
		// COPY命令を参照するオペランドを、COPY元に付け替え
		for( const auto basic_block : function.getBasicBlocks() ) {
			for( const auto instruction : basic_block->getInstructions() ) {
				if( instruction->isCOPY() ) {
					Transformer::shortCutRegOperandsAll( instruction );
				}
			}
		}
		// COPY命令をすべて削除
		Transformer::eraseInstructions_if( function, []( const auto& instr ) { return instr->isCOPY(); } );
	}

	// ====== eliminatePHI ======
	std::vector<exempt_ptr<Instruction>> TransformPasses::createPhiList( const BasicBlock& bb ) {
		std::vector<exempt_ptr<Instruction>> phiList;
		for( const auto instr : bb.getInstructions() ) {
			if( instr->isPhi() ) {
				assert( instr->getRegOperands().size() == bb.getPredBlocks().size() );
				phiList.push_back( instr );
			}
		}
		return phiList;
	}

	void TransformPasses::eliminatePHI( const Function& function ) {
		for( const auto bb : function.getBasicBlocks() ) {
			// insertするとイテレータが無効になりうるため、for文で回すのが面倒になる
			// そのため、事前にphi命令だけリストアップしておく
			const auto phiList = createPhiList( *bb );
			if( phiList.empty() ) { continue; }

			// オリジナルの実装通り、「各pred_bbに対してRMOVを追加」を各phi命令について行う
			auto insert_positions = Optimizer2::TransformPasses::createFixedRegionBase( *bb );
			for( const auto phi : phiList ) {
				for( std::size_t i = 0; i < bb->getPredBlocks().size(); ++i ) {
					const auto operand = phi->getRegOperand( i );
					insert_positions[i] = Transformer::insertRelayRMOVBefore( insert_positions[i], operand, 'F' );
					const auto rmov = insert_positions[i].getIns();
					Transformer::changeOperandToRelayRMOV( rmov, operand );
				}
			}
/*
			// この実装は「各phi命令でRMOVを追加」を各pred_bbについて行う、という方式
			for( std::size_t i = 0; i < bb->getPredBlocks().size(); ++i ) {
				const auto incoming_bb = bb->getPredBlock( i );
				InstructionIterator insert_pos = InstructionIterator( incoming_bb->getInstructions().back() );
				for( const auto phi : phiList ) {
					const auto operand = phi->getRegOperand( i );
					insert_pos = Transformer::insertRelayRMOVBefore( insert_pos, operand, 'F' );
					const auto rmov = insert_pos.getIns();
					Transformer::changeOperandToRelayRMOV( rmov, operand );
				}
			}
*/
		}
	}

	// ====== distanceLimit ======
	static constexpr int MaxDistance = 127;

	exempt_ptr<RegOperand> TransformPasses::findLongDistanceOperand( const Function& function ) {
		for( const auto basic_block : function.getBasicBlocks() ) {
			for( const auto instruction : basic_block->getInstructions() ) {
				if( instruction->isPhi() ) { continue; }
				for( const auto operand : instruction->getRegOperands() ) {
					if( operand->getDistanceThroughPhi() > MaxDistance ) {
						return operand;
					}
				}
			}
		}
		return make_exempt<RegOperand>( nullptr );
	}

	exempt_ptr<Instruction> TransformPasses::findNearestRelayRMOV( exempt_ptr<RegOperand> operand ) {
		for( const auto instr : ReverseRouteRange( operand ) ) {
			if( instr == operand->getConsumer() ) { continue; }
			if( instr->isLimitRMOV() && instr->getOnlyOneRegOperand()->getProducer() == operand->getProducer() ) {
				return instr;
			}
		}
		return make_exempt<Instruction>( nullptr );
	}

	exempt_ptr<Instruction> TransformPasses::findLongDistanceLimitRMOV( exempt_ptr<BasicBlock> bb ) {
		return bb->getInstructions().find_if( []( const auto &instr ) {
			return instr->isLimitRMOV() && instr->getOnlyOneRegOperand()->getDistanceThroughPhi() > MaxDistance;
		} );
	}

	void TransformPasses::changeOperandToLimitRMOV( exempt_ptr<RegOperand> operand ) {
		const auto rmov = findNearestRelayRMOV( operand );
		if( rmov && rmov->getOnlyOneRegOperand()->getDistanceThroughPhi() <= MaxDistance ) {
			// 距離が遠いソースオペランドを中継するRMOVが見つかった
			// →ソースオペランドをそのRMOVに変更する
			Transformer::changeOperandToRelayRMOV( rmov, operand );
		} else {
			// 距離が遠いソースオペランドを中継するRMOVは見つからなかった
			// →中継するRMOVを直前に追加する
			const auto insert_pos = InstructionIterator( operand->getConsumer() ).getFixedRegionTop();
			const auto rmov = Transformer::insertRelayRMOVBefore( insert_pos, operand, 'L' ).getIns();
			Transformer::changeOperandToRelayRMOV( rmov, operand );
			if( operand->getDistanceThroughPhi() > MaxDistance ) {
				// 生存変数が多すぎる場合、距離を制限することはできない
				// （どうしようもない）
				assert( !"too large FixedRegion!" );
			}
		}
	}


	void TransformPasses::distanceLimitOnlyLimitRMOV( const Function& function ) {
		bool change;
		do {
			change = false;
			for( const auto basic_block : function.getBasicBlocks() ) {
				while( const auto rmov = findLongDistanceLimitRMOV( basic_block ) ) {
					const auto operand =rmov->getOnlyOneRegOperand();
					while( operand->getDistanceThroughPhi() > MaxDistance ) {
						if( const auto limit_rmov = findNearestRelayRMOV( operand ) ) {
							// 近くに中継するRMOV命令があれば、それに付け替える
							Transformer::changeOperandToRelayRMOV( limit_rmov, operand );
						} else {
							// 距離を制限するために追加したRMOV命令自体の距離が長い場合、前にずらしていく
							Transformer::moveBefore( rmov );
							change = true;
						}
					}
				}
			}
		} while( change );
	}

	void TransformPasses::distanceLimit( const Function& function ) {
		while( const auto longDistanceOperand = findLongDistanceOperand( function ) ) {
			changeOperandToLimitRMOV( longDistanceOperand );
			distanceLimitOnlyLimitRMOV( function );
		}
	}

	std::vector<InstructionIterator> TransformPasses::createFixedRegionBase( const BasicBlock& bb ) {
		std::vector<InstructionIterator> ret;
		for( auto incoming_bb : bb.getPredBlocks() ) {
			const auto last_instr = incoming_bb->getInstructions().back();
			const auto insert_pos = InstructionIterator( last_instr );
			if( last_instr->isImplicitFallthrough() ) {
				ret.push_back( Transformer::insertNOPBefore( insert_pos, -1 ) );
			} else {
				ret.push_back( insert_pos );
			}
		}
		return ret;
	}


} // namespace Optimizer2
