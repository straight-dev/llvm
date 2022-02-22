add_llvm_component_library(LLVMStraightDesc
  StraightMCTargetDesc.cpp
  StraightAsmBackend.cpp
  StraightMCCodeEmitter.cpp
  StraightELFObjectWriter.cpp

  LINK_COMPONENTS
  MC
  StraightAsmPrinter
  StraightInfo
  Support

  ADD_TO_COMPONENT
  Straight
  )
