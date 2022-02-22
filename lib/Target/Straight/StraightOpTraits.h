namespace llvm {

class MachineInstr;
class MachineOperand;

namespace Straight {

bool is_load( const llvm::MachineInstr& Ins );
bool is_store( const llvm::MachineInstr& Ins );
bool is_ret( const llvm::MachineInstr& Ins );
bool has_dstReg( const llvm::MachineInstr& Ins );

} // namespace Straight
} // namespace llvm
