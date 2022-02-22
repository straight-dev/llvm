add_llvm_component_library(LLVMStraightDisassembler
  StraightDisassembler.cpp

  LINK_COMPONENTS
  MCDisassembler
  StraightInfo
  Support

  ADD_TO_COMPONENT
  Straight
)

