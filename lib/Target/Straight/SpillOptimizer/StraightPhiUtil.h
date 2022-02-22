#ifndef LLVMSTRAIGHT_SPILLOPTIMIZER2_STRAIGHTPHIUTIL_H
#define LLVMSTRAIGHT_SPILLOPTIMIZER2_STRAIGHTPHIUTIL_H

#include "llvm/CodeGen/Register.h"

#include <string>

namespace llvm {
class MachineFunction;
class MachineBasicBlock;
class MachineInstr;
class MachineOperand;

namespace StraightSpillOptimizer {

// Strong TypeDefs
// 仮想レジスタ番号 %vreg2や、実レジスタ番号(%ARG0なら30など)にはllvm::Registerを使う
// ZeroReg以外のレジスタ
enum class LLVMVarReg : unsigned int {};
enum class LLVMBasicBlockID : unsigned int {};
enum class FrameIndex : int {};


const llvm::MachineBasicBlock &
getFirstJoinBasicBlock(const llvm::MachineBasicBlock &basic_block);

const llvm::MachineBasicBlock *
getFirstJoinBasicBlockWithoutFunctionCall(const llvm::MachineBasicBlock &basic_block);

bool containsCall(const llvm::MachineBasicBlock &basic_block);

const llvm::MachineInstr *CreatePhi(llvm::MachineFunction &MF,
                                    llvm::MachineBasicBlock &basic_block,
                                    const llvm::MachineInstr &instr);

const llvm::MachineInstr *CreatePhi(llvm::MachineFunction &MF,
                                    llvm::MachineBasicBlock &basic_block,
                                    const unsigned &reg);

std::string regName(llvm::Register lvreg);
std::string regName(const llvm::MachineOperand &operand);
std::string bbName(const llvm::MachineBasicBlock &MB);

LLVMBasicBlockID toID(const llvm::MachineBasicBlock &bb);
LLVMVarReg toID(const llvm::MachineOperand &operand);

bool isZeroReg(const llvm::MachineOperand &operand);
bool isVarReg(const llvm::MachineOperand &operand);
bool isArgReg(const llvm::MachineOperand &operand);
bool isRetReg(const llvm::MachineOperand &operand);
bool isVirtualReg(const llvm::MachineOperand &operand);

bool isArgReg(llvm::Register lvreg);
bool isRetReg(llvm::Register lvreg);
bool isVirtualReg(llvm::Register lvreg);
bool isArgReg(llvm::Register lvreg);
bool isRetReg(llvm::Register lvreg);
bool isVirtualReg(llvm::Register lvreg);

unsigned int srcOperandBegin(const llvm::MachineInstr &instr);

static constexpr FrameIndex not_found_FrameIndex = FrameIndex{-1};

} // namespace StraightSpillOptimizer

} // namespace llvm


#endif // LLVMSTRAIGHT_SPILLOPTIMIZER2_STRAIGHTPHIUTIL_H
