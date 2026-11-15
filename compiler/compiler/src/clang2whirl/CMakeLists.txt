# psclang: the Clang-based C/C++ front end, Clang AST -> WHIRL. The driver
# runs it for -fpsclang.
#
# Built when PATH64_ENABLE_CLANG_FE is on. It needs a Clang/LLVM 4.0.1
# build tree: set PATH64_CLANG_BUILD_DIR to the build directory and
# PATH64_CLANG_SOURCE_DIR to the llvm source directory (with clang in
# tools/clang).
#
# Only x86-64 for now (phase 1 of doc/frontend/clang-feasibility.md).

set(PATH64_CLANG_BUILD_DIR "" CACHE PATH "Clang/LLVM 4.0.1 build directory")
set(PATH64_CLANG_SOURCE_DIR "" CACHE PATH "LLVM 4.0.1 source directory")

if(NOT EXISTS "${PATH64_CLANG_BUILD_DIR}/bin/llvm-config")
    message(FATAL_ERROR "PATH64_ENABLE_CLANG_FE needs PATH64_CLANG_BUILD_DIR "
                        "pointing at a Clang/LLVM 4.0.1 build directory")
endif()
set(llvm_config "${PATH64_CLANG_BUILD_DIR}/bin/llvm-config")
execute_process(COMMAND ${llvm_config} --version
                OUTPUT_VARIABLE llvm_version OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT llvm_version VERSION_EQUAL "4.0.1")
    message(FATAL_ERROR "clangfe needs LLVM/Clang 4.0.1, found ${llvm_version}")
endif()
# All LLVM archives present in the build; the linker takes only what the
# Clang libraries need (they are linked as one group).
file(GLOB llvm_libfiles "${PATH64_CLANG_BUILD_DIR}/lib/libLLVM*.a")
# (llvm-config --system-libs refuses to answer when the build tree holds
# only the libraries clang needs, so name them here.)
find_package(Threads REQUIRED)
find_package(ZLIB)
set(llvm_system_libs ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
if(ZLIB_FOUND)
    list(APPEND llvm_system_libs ${ZLIB_LIBRARIES})
endif()

set(clang_libs
    clangFrontendTool clangFrontend clangDriver clangSerialization
    clangParse clangSema clangAnalysis clangEdit clangAST clangLex
    clangBasic clangRewriteFrontend clangRewrite
    clangStaticAnalyzerFrontend clangStaticAnalyzerCheckers
    clangStaticAnalyzerCore clangARCMigrate clangASTMatchers)
# No clangCodeGen: clangfe provides clang::CodeGenAction itself
# (whirl_gen.cxx), producing WHIRL instead of LLVM IR.
set(clang_libfiles)
foreach(lib ${clang_libs})
    list(APPEND clang_libfiles "${PATH64_CLANG_BUILD_DIR}/lib/lib${lib}.a")
endforeach()

set(clang_includes
    ${PATH64_CLANG_SOURCE_DIR}/include
    ${PATH64_CLANG_BUILD_DIR}/include
    ${PATH64_CLANG_SOURCE_DIR}/tools/clang/include
    ${PATH64_CLANG_BUILD_DIR}/tools/clang/include)

set(clangfe_common_sources
  ../common/com/config.cxx
  ../common/com/const.cxx
  ../common/com/controls.cxx
  ../common/com/dwarf_DST.cxx
  ../common/com/dwarf_DST_dump.cxx
  ../common/com/dwarf_DST_mem.cxx
  ../common/com/dwarf_DST_producer.cxx
  ../common/com/glob.cxx
  ../common/com/intrn_info.cxx
  ../common/com/ir_bcom.cxx
  ../common/com/ir_bwrite.cxx
  ../common/com/ir_reader.cxx
  ../common/com/irbdata.cxx
  ../common/com/mtypes.cxx
  ../common/com/opcode.cxx
  ../common/com/opcode_core.cxx
  ../common/com/pu_info.cxx
  ../common/com/strtab.cxx
  ../common/com/symtab.cxx
  ../common/com/symtab_verify.cxx
  ../common/com/ttype.cxx
  ../common/com/wn.cxx
  ../common/com/wn_map.cxx
  ../common/com/wn_pragmas.cxx
  ../common/com/wn_simp.cxx
  ../common/com/wn_util.cxx
  ../common/com/wutil.cxx
  ../common/com/xstats.cxx
  ../common/com/@ARCH@/config_targ.cxx
  ../common/com/@ARCH@/config_elf_targ.cxx
  ../common/com/@ARCH@/targ_const.cxx
  ../common/com/@ARCH@/targ_sim.cxx
  ../common/com/@ARCH@/config_host.c
  ../common/com/@ARCH@/config_platform.c
  ../common/com/erglob.cxx
  ../common/com/erlib.cxx
  ../common/com/erlink.cxx
  ../common/com/erirb.cxx
  ../be/com/wn_verifier.cxx
  c_int_model.c
  )

set(clangfe_sources
  c2w_builder.cxx
  c2w_builtin.cxx
  c2w_const.cxx
  c2w_consumer.cxx
  c2w_cxxabi.cxx
  c2w_decl.cxx
  c2w_dst.cxx
  c2w_expr.cxx
  c2w_func.cxx
  c2w_init.cxx
  c2w_lambda.cxx
  c2w_ldst.cxx
  c2w_misc.cxx
  c2w_path64.cxx
  c2w_result.cxx
  c2w_scope.cxx
  c2w_stmt.cxx
  c2w_sym.cxx
  c2w_target.cxx
  c2w_type.cxx
  c2w_utils.cxx
  cc1_main.cxx
  driver.cxx
  whirl_gen.cxx
  )

set(clangfe_defs
  -DFRONT_END -DUSE_DECL_SRCPOS -DNEW_SYMTAB -DFRONT_END_C -DCFE -DCIL
  -DDO_IL_LOWERING=0 -DNO_USR_INCLUDE=TRUE
  -DAUTOMATIC_TEMPLATE_INSTANTIATION=0
  -DINSTANTIATION_BY_IMPLICIT_INCLUSION=0 -DBACK_END_IS_C_GEN_BE=0
  -DMONGOOSE_CIF -DSGI_RAG_BACKEND -DSGI_MONGOOSE -DHANDLE_PRAGMA_WEAK
  -DGPLUSPLUS_FE -DCLANG_2_WHIRL -DNEW_INITIALIZER
  ${PATH64_STDC_FORMAT_MACROS}
  -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS)

path64_is_arch_enabled(x8664_enabled x8664)
if(x8664_enabled)
    set(arch x8664)
    set(tg clangfe-${arch})
    path64_add_executable_for_arch(${tg} ${arch}
                                   ${clangfe_common_sources} ${clangfe_sources})
    set_property(TARGET ${tg} PROPERTY OUTPUT_NAME psclang)
    target_compile_definitions(${tg} PRIVATE ${clangfe_defs})
    target_include_directories(${tg} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/gnu
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/com
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/com/${arch}
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/util
        ${CMAKE_CURRENT_SOURCE_DIR}/../be/com
        ${CMAKE_CURRENT_SOURCE_DIR}/../libelf
        ${CMAKE_CURRENT_SOURCE_DIR}/../libelf/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/targ_info/access
        ${PATHSCALE_BINARY_DIR}/common/targ_info
        ${PATHSCALE_BINARY_DIR}/common/targ_info/${arch})
    # Clang's headers and libraries are built without RTTI; so is
    # clangfe's own C++ code, which subclasses Clang classes.
    foreach(src ${clangfe_sources})
        set_property(SOURCE ${src} APPEND PROPERTY INCLUDE_DIRECTORIES ${clang_includes})
        set_property(SOURCE ${src} APPEND_STRING PROPERTY COMPILE_FLAGS " -fno-rtti")
    endforeach()
    set_property(SOURCE ../common/com/wn_util.cxx PROPERTY COMPILE_DEFINITIONS MONGOOSE_BE)
    target_link_libraries(${tg}
        comutil-${arch} targinfo-${arch}
        -Wl,--start-group ${clang_libfiles} ${llvm_libfiles} -Wl,--end-group
        iberty cmplrs m ${llvm_system_libs})

    # Clang's builtin headers, where the driver's -resource-dir points.
    set(clang_resource_dir ${Path64_BINARY_DIR}/lib/clang)
    add_custom_command(TARGET ${tg} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PATH64_CLANG_BUILD_DIR}/lib/clang/${llvm_version}/include
                ${clang_resource_dir}/include)
    install(TARGETS ${tg} RUNTIME DESTINATION ${PATH64_LIB_PATH}/${arch})
    install(DIRECTORY ${clang_resource_dir}/include
            DESTINATION ${PATH64_LIB_PATH}/clang)
endif()
