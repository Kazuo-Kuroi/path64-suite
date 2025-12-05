

set(BE_COM_SOURCES
    "be_util.cxx"
    "be_version.cxx"
    "cxx_base.cxx"
    "cxx_graph.cxx"
    "cxx_hash_util.cxx"
    "erbe.cxx"
    "mfmc_misc.cxx"
    "mfmc_query.cxx"
    "mfmc_setup.cxx"
    "mfmc_solve.cxx"
    "opt_addr_flags.cxx"
    "opt_cvtl_rule.cxx"
    "printsrc.cxx"
    "timing.cxx"
    "output_func_start_profiler.cxx"
    "wn_cyg_instrument.cxx"
    "wn_instrument.cxx"
    "ipa_lno_file.cxx"
    "ipa_lno_summary.cxx"
    "ipa_lno_util.cxx"
    "be_symtab.cxx"
    "be_errors.cxx"
    "dvector.cxx"
    "emulate.cxx"
    "fb_cfg.cxx"
    "fb_whirl.cxx"
    "opt_alias_mgr.cxx"
    "opt_alias_rule.cxx"
    "opt_goto.cxx"
    "opt_ipaa_io.cxx"
    "opt_points_to_non_template.cxx"
    "prompf.cxx"
    "standardize.cxx"
    "stblock.cxx"
    "w2op.cxx"
    "whirl2src.cxx"
    "wn_fio.cxx"
    "wn_lower.cxx"
    "u64_lower_wn.cxx"
    "wn_mp.cxx"
    "wn_verifier.cxx"
    "wn_retype_expr.cxx"
    "wn_reassoc.cxx"
    "wb_browser.cxx"
    "wb_buffer.cxx"
    "wb_carray.cxx"
    "wb_ipl.cxx"
    "wb_util.cxx"
    "dep_graph.cxx"
    "com_whirlview.cxx"
    "DaVinci.cxx"
    "privatize_common.cxx"
    "wn_unroll.cxx"
    "wb_ipl_summary.cxx"
    "@ARCH@/betarget.cxx"
    "@ARCH@/sections.cxx"
    "../../common/com/@ARCH@/config_cache_targ.cxx"
    "../../common/com/@ARCH@/config_targ.cxx"
    "../../common/com/@ARCH@/config_elf_targ.cxx"
    "../../common/com/@ARCH@/targ_const.cxx"
    "../../common/com/@ARCH@/targ_sim.cxx"
    "../../common/instrument/instr_reader.cxx"
    "../../common/com/config.cxx"
    "../../common/com/config_cache.cxx"
    "../../common/com/dra_demangle.cxx"
    "../../common/com/dwarf_DST.cxx"
    "../../common/com/dwarf_DST_dump.cxx"
    "../../common/com/fb_info.cxx"
    "../../common/com/intrn_info.cxx"
    "../../common/com/ll.cxx"
    "../../common/com/mtypes.cxx"
    "../../common/com/opcode.cxx"
    "../../common/com/opcode_core.cxx"
    "../../common/com/wutil.cxx"
    "../../common/com/const.cxx"
    "../../common/com/dwarf_DST_mem.cxx"
    "../../common/com/f90_utils.cxx"
    "../../common/com/glob.cxx"
    "../../common/com/ir_bcom.cxx"
    "../../common/com/ir_bread.cxx"
    "../../common/com/ir_bwrite.cxx"
    "../../common/com/ir_reader.cxx"
    "../../common/com/irbdata.cxx"
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
    "../../common/com/xstats.cxx"
    "../../common/targ_info/access/ti_init.c"
    "../../common/targ_info/access/ti_res.c"
    "../../common/targ_info/access/ti_res_count.c"
    "../../common/targ_info/access/ti_res_res.c"
    "../../common/com/@ARCH@/config_host.c"
    "../../common/com/@ARCH@/config_platform.c"

    "../be/dra_clone.cxx"
    "../be/dra_ec.cxx"
    "../be/dra_file_util.cxx"
    "../be/dra_mangle.cxx"

    "${PATHSCALE_BINARY_DIR}/be/preg_list.h"
   )

add_definitions(-D_LEGO_CLONER)

add_custom_command(
    OUTPUT ${PATHSCALE_BINARY_DIR}/be/preg_list.cxx ${PATHSCALE_BINARY_DIR}/be/preg_list.h
    COMMAND ${PATHSCALE_SOURCE_DIR}/be/com/gen_preg_list.${SHELL_EXT} ${PATHSCALE_SOURCE_DIR}/common/util
    DEPENDS ${PATHSCALE_SOURCE_DIR}/be/com/gen_preg_list.${SHELL_EXT}
    WORKING_DIRECTORY ${PATHSCALE_BINARY_DIR}/be
    )
add_custom_target(generate_preg_list DEPENDS ${PATHSCALE_BINARY_DIR}/be/preg_list.h ${PATHSCALE_BINARY_DIR}/be/preg_list.cxx)

set_source_files_properties(${PATHSCALE_BINARY_DIR}/be/preg_list.h PROPERTIES GENERATED 1)
set_source_files_properties(${PATHSCALE_BINARY_DIR}/be/preg_list.cxx PROPERTIES GENERATED 1)

path64_set_multiarch_sources(be_com_sources COMMON ${BE_COM_SOURCES})
path64_add_multiarch_library(be-com STATIC be_com_sources)
path64_set_multiarch_property(be-com COMPILE_FLAGS
                              "-I${CMAKE_CURRENT_SOURCE_DIR}/@ARCH@ -I${CMAKE_CURRENT_SOURCE_DIR}/../be/@ARCH@  -I${CMAKE_CURRENT_SOURCE_DIR}/../cg/@ARCH@  -I${PATHSCALE_BINARY_DIR}/common/targ_info/@ARCH@ -I${CMAKE_CURRENT_SOURCE_DIR}/../../common/com/@ARCH@")
path64_multiarch_link_multiarch_libraries(be-com targinfo comutil)
path64_multiarch_link_libraries(be-com erdesc)
path64_add_multiarch_multiarch_dependencies(be-com generate_topcode)


