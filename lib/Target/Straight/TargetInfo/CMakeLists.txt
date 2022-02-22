add_llvm_component_library(LLVMStraightInfo
  StraightTargetInfo.cpp

  LINK_COMPONENTS
  Support

  ADD_TO_COMPONENT
  Straight
  )
