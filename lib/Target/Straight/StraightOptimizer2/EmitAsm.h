#pragma once

namespace llvm {
	class raw_pwrite_stream;
	class MachineFunction;
}

void NoOptPrintAsm( llvm::raw_pwrite_stream& Out, llvm::MachineFunction& MF );
