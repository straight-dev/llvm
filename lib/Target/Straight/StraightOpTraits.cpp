#include "StraightOpTraits.h"
#include "MCTargetDesc/StraightMCTargetDesc.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"

namespace llvm {
namespace Straight {

bool is_load( const llvm::MachineInstr& Ins ) {
	return Ins.getOpcode() == Straight::LD_8
		|| Ins.getOpcode() == Straight::LD_8u
		|| Ins.getOpcode() == Straight::LD_16
		|| Ins.getOpcode() == Straight::LD_16u
		|| Ins.getOpcode() == Straight::LD_32
		|| Ins.getOpcode() == Straight::LD_32u
		|| Ins.getOpcode() == Straight::LD_64
		|| Ins.getOpcode() == Straight::LD_f32
		|| Ins.getOpcode() == Straight::LD_f64
		|| Ins.getOpcode() == Straight::SPLD_8
		|| Ins.getOpcode() == Straight::SPLD_8u
		|| Ins.getOpcode() == Straight::SPLD_16
		|| Ins.getOpcode() == Straight::SPLD_16u
		|| Ins.getOpcode() == Straight::SPLD_32
		|| Ins.getOpcode() == Straight::SPLD_32u
		|| Ins.getOpcode() == Straight::SPLD_64
		|| Ins.getOpcode() == Straight::SPLD_f32;
}

bool is_store( const llvm::MachineInstr& Ins ) {
	return Ins.getOpcode() == llvm::Straight::ST_8
		|| Ins.getOpcode() == llvm::Straight::ST_16
		|| Ins.getOpcode() == llvm::Straight::ST_32
		|| Ins.getOpcode() == llvm::Straight::ST_64
		|| Ins.getOpcode() == llvm::Straight::ST_f32
		|| Ins.getOpcode() == llvm::Straight::ST_f64
		|| Ins.getOpcode() == llvm::Straight::SPST_8
		|| Ins.getOpcode() == llvm::Straight::SPST_16
		|| Ins.getOpcode() == llvm::Straight::SPST_32
		|| Ins.getOpcode() == llvm::Straight::SPST_64
		|| Ins.getOpcode() == llvm::Straight::SPST_f32
		|| Ins.getOpcode() == llvm::Straight::SPST_f64;
}

bool is_ret( const llvm::MachineInstr& Ins ) {
	return Ins.getOpcode() == llvm::Straight::JR;
}

bool has_dstReg( const llvm::MachineInstr& Ins ) {
  return !Ins.isBranch() && !is_ret(Ins) && !is_store(Ins) && !Ins.isCall() &&
         Ins.getOpcode() != llvm::Straight::SPADDi &&
         Ins.getOpcode() != llvm::Straight::LIFETIME_START &&
         Ins.getOpcode() != llvm::Straight::LIFETIME_END;
}

} // namespace Straight
} // namespace llvm
