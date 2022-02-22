add_llvm_component_group(Straight)

llvm_replace_compiler_option(CMAKE_CXX_FLAGS "-std=c\\+\\+11" "")
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    if (MSVC_VERSION GREATER_EQUAL "1900")
        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
        if (_cpp_latest_flag_supported)
            add_compile_options("/std:c++latest")
        endif()
    endif()
endif()
set(LLVM_TARGET_DEFINITIONS Straight.td)

tablegen(LLVM StraightGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM StraightGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM StraightGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM StraightGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM StraightGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM StraightGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM StraightGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM StraightGenCallingConv.inc -gen-callingconv)
tablegen(LLVM StraightGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(StraightCommonTableGen)

add_llvm_target(StraightCodeGen
  StraightAsmPrinter.cpp
  StraightCodeGen.cpp
  StraightFrameLowering.cpp
  StraightInstrInfo.cpp
  StraightISelDAGToDAG.cpp
  StraightISelLowering.cpp
  StraightMCInstLower.cpp
  StraightOpTraits.cpp
  StraightPhiPass.cpp
  StraightRegisterInfo.cpp
  StraightSubtarget.cpp
  StraightTargetMachine.cpp
  StraightOptimizer2/EmitAsm.cpp
  StraightOptimizer2/LLVMtoStraightConverter.cpp
  StraightOptimizer2/Optimizer2.cpp
  StraightOptimizer2/StraightAssembler.cpp
  StraightOptimizer2/Transformer.cpp
  StraightOptimizer2/TransformPasses.cpp
  StraightOptimizer2/RouteIterator.cpp
  SpillOptimizer/StraightPhiUtil.cpp
  SpillOptimizer/LivenessAnalysis.cpp
  SpillOptimizer/LiveSpaceAnalysis.cpp
  SpillOptimizer/StraightPhiSpillAnalysis.cpp

  LINK_COMPONENTS
  AsmPrinter
  CodeGen
  Core
  MC
  StraightAsmPrinter
  StraightDesc
  StraightInfo
  SelectionDAG
  Support
  Target

  ADD_TO_COMPONENT
  Straight
  )

add_subdirectory(Disassembler)
add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
