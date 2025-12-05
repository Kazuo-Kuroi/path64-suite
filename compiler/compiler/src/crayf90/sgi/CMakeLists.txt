# Right now config.cxx and many other files are built twice or even four times because of strange -D defines
# So we must build confix.cxx twice.. Once without -DBACK_END for mfef95 and
# once for be with -DBACK_END

include_directories(
    .
    ../../crayf90/fe90
    ../../arith
    ../../common/util
    ../../common/com
    ../../common/com/@ARCH@
    ../../linux/mfef90_includes
    ../../clibinc/cray
    ../include
    ../../include/cmplrs
    ${PATHSCALE_BINARY_DIR}/common/targ_info/@ARCH@
)


set(PATHSCALE_MFEF95_SRCS       "../../common/com/config.cxx"
				"../../common/com/const.cxx"
				"../../common/com/controls.cxx"
				"../../common/com/dwarf_DST.cxx"
				"../../common/com/dwarf_DST_dump.cxx"
				"../../common/com/dwarf_DST_mem.cxx"
				"../../common/com/dwarf_DST_producer.cxx"
				"../../common/com/f90_utils.cxx"
				"../../common/com/glob.cxx"
				"../../common/com/ir_bcom.cxx"
				"../../common/com/ir_bwrite.cxx"
				"../../common/com/ir_reader.cxx"
				"../../common/com/irbdata.cxx"
				"../../common/com/mtypes.cxx"
				"../../common/com/opcode.cxx"
				"../../common/com/opcode_core.cxx"
				"../../common/com/pu_info.cxx"
				"../../common/com/strtab.cxx"
				"../../common/com/symtab.cxx"
				"../../common/com/symtab_verify.cxx"
				"../../common/com/ttype.cxx"
				"../../common/com/wn.cxx"
				"../../common/com/wn_map.cxx"
				"../../common/com/wn_pragmas.cxx"
				"../../common/com/wn_simp.cxx"
				"../../common/com/wn_util.cxx"
				"../../common/com/wutil.cxx"
				"../../common/com/@ARCH@/config_targ.cxx"
				"../../common/com/@ARCH@/config_elf_targ.cxx"
				"../../common/com/@ARCH@/targ_const.cxx"
				"../../common/com/@ARCH@/targ_sim.cxx"
				"../../crayf90/sgi/cwh_unimp.cxx"
				"../../crayf90/sgi/cwh_addr.cxx"
				"../../crayf90/sgi/cwh_auxst.cxx"
				"../../crayf90/sgi/cwh_block.cxx"
				"../../crayf90/sgi/cwh_data.cxx"
				"../../crayf90/sgi/cwh_directive.cxx"
				"../../crayf90/sgi/cwh_dope.cxx"
				"../../crayf90/sgi/cwh_dst.cxx"
				"../../crayf90/sgi/cwh_expr.cxx"
				"../../crayf90/sgi/cwh_intrin.cxx"
				"../../crayf90/sgi/cwh_io.cxx"
				"../../crayf90/sgi/cwh_mkdepend.cxx"
				"../../crayf90/sgi/cwh_pdgcs.cxx"
				"../../crayf90/sgi/cwh_preg.cxx"
				"../../crayf90/sgi/cwh_types.cxx"
				"../../crayf90/sgi/cwh_stab.cxx"
				"../../crayf90/sgi/cwh_stmt.cxx"
				"../../crayf90/sgi/cwh_stk.cxx"
				"../../crayf90/sgi/decorate_utils.cxx"
				"../../crayf90/sgi/f2c_abi_utils.cxx"
				"../../crayf90/sgi/sgi_cmd_line.cxx"
				"../../crayf90/sgi/path_intrinsic_list.cxx"
				"../../crayf90/sgi/make_depend.cxx"
				"../../common/com/@ARCH@/config_host.c"
				"../../common/com/@ARCH@/config_platform.c")

ADD_DEFINITIONS(-DFRONT_END
                -DFRONT_F90
                -D_TARGET_MONGOOSE
                ${PATH64_STDC_FORMAT_MACROS}
               )

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-D_DEBUG)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

include_directories(
    "../../libcsup/include"
    "../../clibinc"
    "../../libfi/include"
    "../../libf/include"
)


add_custom_command(
    OUTPUT ${Path64_BINARY_DIR}/bin/pathf90
    COMMAND ${CMAKE_COMMAND} -E ${LINK_COPY} pathcc ${Path64_BINARY_DIR}/bin/pathf90
    COMMAND ${CMAKE_COMMAND} -E ${LINK_COPY} pathcc ${Path64_BINARY_DIR}/bin/pathf95
    DEPENDS ${Path64_BINARY_DIR}/bin/pathcc
)

add_custom_target(fortran_driver DEPENDS ${Path64_BINARY_DIR}/bin/pathf90)


path64_set_multiarch_sources(mfef95_sources COMMON ${PATHSCALE_MFEF95_SRCS})
path64_add_multiarch_executable(mfef95 mfef95_sources)
path64_multiarch_link_multiarch_libraries(mfef95 comutil fe90)
path64_multiarch_link_libraries(mfef95 arith cif pathfortran-mfef95 erdesc m)
path64_set_multiarch_property(mfef95 COMPILE_FLAGS
                              "-I${CMAKE_CURRENT_SOURCE_DIR}/../../common/com/@ARCH@")
path64_install_multiarch(mfef95)
path64_add_dependencies_from_multiarch(compiler-stage-Fortran mfef95)
add_dependencies(compiler-stage-Fortran fortran_driver generate_cf95_cat)


