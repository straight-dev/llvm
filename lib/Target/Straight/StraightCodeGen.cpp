#include "StraightCodeGen.h"
#include "StraightOpTraits.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace llvm;
#define DEBUG_TYPE "straight-codegen"

#include <iostream>
#include "llvm/IR/Operator.h"

#include "StraightPhiPass.h"
#include "StraightOptimizer2/EmitAsm.h"

char forStraightCodeGen;
char forStraightEmitGlobalObject;

void StraightCodeGenPass::StraightCodeGen_impl( raw_pwrite_stream& Out, MachineFunction& MF ) const {
	LLVM_DEBUG( MF.dump() );
	NoOptPrintAsm(Out, MF);
}

void EmitConstant( llvm::raw_pwrite_stream& Out, const llvm::Constant* constant ) {
	if( const auto*const int_data = dyn_cast<ConstantInt>(constant) ) {
		Out << " i" << int_data->getType()->getBitWidth() << " " << int_data->getZExtValue();
	} else if( const auto*const const_data_array = dyn_cast<ConstantDataArray>(constant) ) {
		for( unsigned i = 0; ; ++i ) {
			if( const auto*const element = constant->getAggregateElement( i ) ) {
				EmitConstant( Out, element );
			} else {
				break;
			}
		}
	} else if( const auto* const const_aggregate_zero = dyn_cast<ConstantAggregateZero>(constant) ) {
		for( unsigned i = 0; ; ++i ) {
			if( const auto*const element = constant->getAggregateElement( i ) ) {
				EmitConstant( Out, element );
			} else {
				break;
			}
		}
	} else if( const auto* const const_nullptr = dyn_cast<ConstantPointerNull>(constant) ) {
		Out << " p32 nullptr";
	} else if( const auto* const float_data = dyn_cast<ConstantFP>(constant) ) {
		if( float_data->getType()->isFloatTy() ) {
			Out << " f32 " << float_data->getValueAPF().bitcastToAPInt().getZExtValue();
		} else if( float_data->getType()->isDoubleTy() ) {
			Out << " f64 " << float_data->getValueAPF().bitcastToAPInt().getZExtValue();
		} else {
			assert( !"floatとdouble以外の浮動小数点数は未実装" );
		}
	} else if( const auto* const const_undef = dyn_cast<UndefValue>(constant) ) {
		if( isa<StructType>(constant->getType()) || isa<ArrayType>(constant->getType()) || isa<VectorType>(constant->getType()) ) {
			// 丸ごとundefである構造体or配列
			for( unsigned i = 0; i < const_undef->getNumElements(); ++i ) {
				EmitConstant( Out, const_undef->getElementValue(i) );
			}
		} else {
			// 単独のundef
			assert( constant->getType()->getPrimitiveSizeInBits() == 8 && "1Byte以外のundefは考慮外" );
			Out << " i" << constant->getType()->getPrimitiveSizeInBits() << " " << 0;
		}
	} else if( dyn_cast<ConstantData>(constant) ) {
		assert( !"未実装の定数データ" );
	} else if( const auto* const const_expr = dyn_cast<ConstantExpr>(constant) ) {
		const auto opcode = const_expr->getOpcode();
		if( opcode == Instruction::BitCast ) {
			const auto* const op = const_expr->op_begin();
			const auto* const gv = dyn_cast<GlobalValue>( op );
			assert( gv && "関数アドレスやstatic constへのポインタ以外の定数式は未実装" );
			Out << " p32 " << gv->getName();
		} else if( opcode == Instruction::GetElementPtr ) {
			const auto* const base = const_expr->getOperand( 0 );
			const Type* type = base->getType()->getPointerElementType();
			if( type->isArrayTy() ) {
				const auto* const n_elem = const_expr->getOperand( 1 );
				assert( n_elem->isZeroValue() && "0番目以外の配列要素へのポインタ取得は未実装" );
				assert( dyn_cast<GlobalValue>( base ) && "名前のない配列へのポインタ取得は未実装" );
				EmitConstant( Out, base );
			} else if( type->isIntegerTy() ) {
				const auto* const n_elem = const_expr->getOperand( 1 );
				const auto* const num = dyn_cast<ConstantInt>( n_elem );
				assert( num && "ポインタの加算は定数以外未実装" );
				Out << " gep32 (";
				EmitConstant( Out, base );
				Out << " + " << num->getZExtValue() * type->getIntegerBitWidth() / 8 << " )";
			} else if( type->isStructTy() ) {
				const auto* const n_elem1 = const_expr->getOperand( 1 );
				const auto* const n_elem2 = const_expr->getOperand( 2 );
				const auto* const n_elem3 = const_expr->getOperand( 3 );
				assert( n_elem1->isZeroValue() && "0番目以外の構造体要素へのポインタ取得は未実装" );
				assert( n_elem2->isZeroValue() && "0番目以外の構造体要素へのポインタ取得は未実装" );
				assert( dyn_cast<GlobalValue>( base ) && "名前のない構造体へのポインタ取得は未実装" );
				const auto* const inner_type = type->getStructElementType( 0 );
				assert( inner_type->isArrayTy() && "構造体内の配列以外へのポインタ取得は未実装" );
				const auto* const elem_type = inner_type->getArrayElementType();
				const auto* const num = dyn_cast<ConstantInt>( n_elem3 );
				assert( num && "ポインタの加算は定数以外未実装" );
				Out << " gep32 (";
				EmitConstant( Out, base );
				Out << " + " << num->getZExtValue() * elem_type->getIntegerBitWidth() / 8 << " )";
			} else {
				assert( !"未実装のGetElementPtr定数式" );
			}
		} else {
			assert( !"未実装の定数式" );
		}
	} else if( const auto* const global_val = dyn_cast<GlobalValue>(constant) ) {
		// 実はここに来るのはグローバル変数だけではなくて関数ポインタの可能性もある、リンカで対応する
		Out << " p32 " << global_val->getName();
	} else if( dyn_cast<ConstantAggregate>(constant) ) {
		for( unsigned i = 0; ; ++i ) {
			if( const auto*const element = constant->getAggregateElement( i ) ) {
				EmitConstant( Out, element );
			} else {
				break;
			}
		}
	} else if( dyn_cast<BlockAddress>(constant) ) {
		assert( !"ブロックアドレスは未実装" );
	}
}

void llvm::StraightEmitGlobalObjectPass::StraightEmitGlobalObject_impl( llvm::raw_pwrite_stream& Out, llvm::Module& M ) const {
	for( const auto& global_object : M.getGlobalList() ) {
		Out << global_object.getName();
		for( const auto& use : global_object.operands() ) {
			if( const auto*const constant = dyn_cast<Constant>( use ) ) {
				EmitConstant( Out, constant );
			} else {
				LLVM_DEBUG(use->dump());
				assert( !"不明なオブジェクトがグローバル領域に存在（リンクし忘れ？）" );
			}
		}
		Out << '\n';
	}
}

void llvm::StraightEmitGlobalObjectPass::StraightEmitAliasList_impl( llvm::raw_pwrite_stream& Out, llvm::Module& M ) const {
	for( const auto& alias : M.getAliasList() ) {
		Out << alias.getName();
		EmitConstant( Out, alias.getAliasee() );
		Out << '\n';
	}
}
