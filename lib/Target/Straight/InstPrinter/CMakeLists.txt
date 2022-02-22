add_llvm_component_library(LLVMStraightAsmPrinter
  StraightInstPrinter.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  Straight
  )
