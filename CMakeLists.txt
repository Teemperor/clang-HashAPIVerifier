cmake_minimum_required(VERSION 3.3)
project(HashApiVerifier)

# LLVM

find_package(LLVM REQUIRED)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    main.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME}
    ${LIBS}
    rt

    ${LLVM_LIBRARY}

    clangTooling
    clangToolingCore
    clangFrontendTool
    clangFrontend
    clangDriver
    clangSerialization
    clangCodeGen
    clangParse
    clangSema
    clangStaticAnalyzerFrontend
    clangStaticAnalyzerCheckers
    clangStaticAnalyzerCore
    clangAnalysis
    clangARCMigrate
    clangRewriteFrontend
    clangRewrite
    clangEdit
    clangAST
    clangLex
    clangBasic

    # TODO use llvm-config instead
    LLVMLTO
    LLVMObjCARCOpts
    LLVMLinker
    LLVMBitWriter
    LLVMIRReader
    LLVMXCoreDisassembler
    LLVMXCoreCodeGen
    LLVMXCoreDesc
    LLVMXCoreInfo
    LLVMXCoreAsmPrinter
    LLVMSystemZDisassembler
    LLVMSystemZCodeGen
    LLVMSystemZAsmParser
    LLVMSystemZDesc
    LLVMSystemZInfo
    LLVMSystemZAsmPrinter
    LLVMSparcDisassembler
    LLVMSparcCodeGen
    LLVMSparcAsmParser
    LLVMSparcDesc
    LLVMSparcInfo
    LLVMSparcAsmPrinter
    LLVMPowerPCDisassembler
    LLVMPowerPCCodeGen
    LLVMPowerPCAsmParser
    LLVMPowerPCDesc
    LLVMPowerPCInfo
    LLVMPowerPCAsmPrinter
    LLVMNVPTXCodeGen
    LLVMNVPTXDesc
    LLVMNVPTXInfo
    LLVMNVPTXAsmPrinter
    LLVMMSP430CodeGen
    LLVMMSP430Desc
    LLVMMSP430Info
    LLVMMSP430AsmPrinter
    LLVMMipsDisassembler
    LLVMMipsCodeGen
    LLVMMipsAsmParser
    LLVMMipsDesc
    LLVMMipsInfo
    LLVMMipsAsmPrinter
    LLVMHexagonDisassembler
    LLVMHexagonCodeGen
    LLVMHexagonDesc
    LLVMHexagonInfo
    LLVMCppBackendCodeGen
    LLVMCppBackendInfo
    LLVMBPFCodeGen
    LLVMBPFDesc
    LLVMBPFInfo
    LLVMBPFAsmPrinter
    LLVMARMDisassembler
    LLVMARMCodeGen
    LLVMARMAsmParser
    LLVMARMDesc
    LLVMARMInfo
    LLVMARMAsmPrinter
    LLVMAMDGPUCodeGen
    LLVMAMDGPUAsmParser
    LLVMAMDGPUUtils
    LLVMAMDGPUDesc
    LLVMAMDGPUInfo
    LLVMAMDGPUAsmPrinter
    LLVMAArch64Disassembler
    LLVMAArch64CodeGen
    LLVMAArch64AsmParser
    LLVMAArch64Desc
    LLVMAArch64Info
    LLVMAArch64AsmPrinter
    LLVMAArch64Utils
    LLVMMIRParser
    LLVMAsmParser
    LLVMLibDriver
    LLVMOption
    LLVMDebugInfoPDB
    LLVMTableGen
    LLVMOrcJIT
    LLVMLineEditor
    LLVMX86Disassembler
    LLVMX86AsmParser
    LLVMX86CodeGen
    LLVMSelectionDAG
    LLVMAsmPrinter
    LLVMX86Desc
    LLVMMCDisassembler
    LLVMX86Info
    LLVMX86AsmPrinter
    LLVMX86Utils
    LLVMMCJIT
    LLVMDebugInfoDWARF
    LLVMPasses
    LLVMipo
    LLVMVectorize
    LLVMInterpreter
    LLVMExecutionEngine
    LLVMRuntimeDyld
    LLVMCodeGen
    LLVMTarget
    LLVMScalarOpts
    LLVMProfileData
    LLVMObject
    LLVMMCParser
    LLVMBitReader
    LLVMInstCombine
    LLVMInstrumentation
    LLVMTransformUtils
    LLVMMC
    LLVMAnalysis
    LLVMCore
    LLVMSupport
)
