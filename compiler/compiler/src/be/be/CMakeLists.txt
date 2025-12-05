PROJECT(BE)

SET(PATHSCALE_BE_BE_COMMON_SRCS
                         "../com/data_layout.cxx"
                         "../com/wb.cxx"
                         "cleanup.cxx"
                         "fill_align.cxx"
                         "goto_conv.cxx"
                         "mem_ctr.cxx"
                         "omp_lower.cxx"
                         "options_stack.cxx"
                         "rewrite_pragmas.cxx"
                         "wb_omp.cxx"
                         "wb_lwr.cxx"
                         "wb_anl.cxx"
                         "@ARCH@/fill_align_targ.cxx"
                         "@ARCH@/driver_targ.cxx"
                         "${PATHSCALE_BINARY_DIR}/be/preg_list.cxx"
                         "../../ipa/main/optimize/ipo_tlog_utils.cxx"
                         "../region/ori.cxx"
                         "../region/rbi.cxx"
                         "../region/region_init.cxx"
                         "../region/region_bounds.cxx"
                         "../region/region_util.cxx"
                         "../lno/soe.cxx"
                         "../lno/mat.cxx"
                         "../vho/f90_lower.cxx"
                         "../vho/f90_lower_dep.cxx"
                         "../vho/vho_lower.cxx"
                         "../vho/wb_f90_lower.cxx"
                         )

SET(PATHSCALE_BE_BE_X8664_SRCS)

SET(PATHSCALE_BE_BE_MIPS_SRCS
                         "../com/mips/quad_sim.cxx"
                           )

SET(PATHSCALE_BE_BE_EXEC_COMMON_SRCS 
			 "driver.cxx"
			 "driver_util.cxx"
			 "iter.cxx"
			 "../com/phase.cxx"
# The following files must not be in be.so since they are shared between
# inline/be/ipa but with if-defs that make them different for each binary
			 "../com/clone_DST_utils.cxx"
                         "../com/clone.cxx"
                         "../../common/com/dwarf_DST_producer.cxx"
			 )

ADD_DEFINITIONS(
# Apparently someone likes legos
	-D_LEGO_CLONER)

add_custom_command(
    OUTPUT   extension_include.h
    COMMAND  ${PATHSCALE_SOURCE_DIR}/scripts/common/preprocess.pl -I${PATHSCALE_SOURCE_DIR}/common/com -I${PATHSCALE_SOURCE_DIR}/common/com/${BUILD_TARGET_DIR} -I${PATHSCALE_BINARY_DIR}/../GCC/gcc -I${PATHSCALE_SOURCE_DIR}/../GCC/gcc -MD  ${PATHSCALE_SOURCE_DIR}/include/extension_include.hin -o ${PATHSCALE_BINARY_DIR}/include/extension_include.h
)
SET_SOURCE_FILES_PROPERTIES(extension_include.h PROPERTIES GENERATED 1)
ADD_CUSTOM_TARGET(generate_extension DEPENDS extension_include.h)

set_source_files_properties(${PATHSCALE_BINARY_DIR}/be/preg_list.h PROPERTIES GENERATED 1)
set_source_files_properties(${PATHSCALE_BINARY_DIR}/be/preg_list.cxx PROPERTIES GENERATED 1)

#add_library(be-static STATIC ${PATHSCALE_BE_BE_SRCS})
path64_set_multiarch_sources(be_sources COMMON ${PATHSCALE_BE_BE_COMMON_SRCS})
path64_set_multiarch_sources(be_sources x8664 ${PATHSCALE_BE_BE_COMMON_SRCS} ${PATHSCALE_BE_BE_X8664_SRCS})
path64_set_multiarch_sources(be_sources mips ${PATHSCALE_BE_BE_COMMON_SRCS} ${PATHSCALE_BE_BE_MIPS_SRCS})
path64_add_multiarch_library(be-static STATIC be_sources)
path64_set_multiarch_property(be-static OUTPUT_NAME be)
path64_multiarch_link_multiarch_libraries(be-static cg wopt prompf_anl whirl2c whirl2f lno ipl targinfo be-com comutil)
path64_multiarch_link_libraries(be-static m ${libdl})
path64_set_multiarch_property(be-static COMPILE_FLAGS
                              "-I${CMAKE_CURRENT_SOURCE_DIR}/@ARCH@ -I${CMAKE_CURRENT_SOURCE_DIR}/../com/@ARCH@  -I${CMAKE_CURRENT_SOURCE_DIR}/../cg/@ARCH@  -I${PATHSCALE_BINARY_DIR}/common/targ_info/@ARCH@ -I${CMAKE_CURRENT_SOURCE_DIR}/../../common/com/@ARCH@")
path64_add_multiarch_multiarch_dependencies(be-static generate_topcode)


path64_set_multiarch_sources(be_exec_sources COMMON ${PATHSCALE_BE_BE_EXEC_COMMON_SRCS})
path64_add_multiarch_executable(be-exec be_exec_sources)
# TODO : Must be the elf, elfutil and dwarf built from the source and not a system installed one
# TODO : Make sure link targets are static
path64_multiarch_link_libraries(be-exec -Wl,--whole-archive)
path64_multiarch_link_multiarch_libraries(be-exec be-static cg wopt)
path64_multiarch_link_libraries(be-exec  rtkutils -Wl,--no-whole-archive)
path64_multiarch_link_libraries(be-exec elfutil elf)
path64_multiarch_link_multiarch_libraries(be-exec cg wopt prompf_anl targinfo comutil ipl lno whirl2c whirl2f be-com)
path64_set_multiarch_property(be-exec OUTPUT_NAME "be")
path64_set_multiarch_property(be-exec COMPILE_FLAGS
                              "-DSHARED_BUILD  -I${CMAKE_CURRENT_SOURCE_DIR}/@ARCH@  -I${CMAKE_CURRENT_SOURCE_DIR}/../com/@ARCH@ -I${CMAKE_CURRENT_SOURCE_DIR}/../cg/@ARCH@  -I${PATHSCALE_BINARY_DIR}/common/targ_info/@ARCH@ -I${CMAKE_CURRENT_SOURCE_DIR}/../../common/com/@ARCH@")
path64_add_multiarch_multiarch_dependencies(be-exec generate_topcode)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    path64_multiarch_link_libraries(be-exec winlib iberty)
endif()

set(link_flags "")
if(PATH64_LIBDWARF_PATH)
    set(link_flags "${link_flags} -L ${PATH64_LIBDWARF_PATH} -Wl,-rpath,${PATH64_LIBDWARF_PATH}")
endif()
path64_multiarch_link_libraries(be-exec dwarf)

if(NOT "${CMAKE_SYSTEM_NAME}" MATCHES "SunOS" AND "${EXPORT_DYNAMIC_SUPPORTED}" MATCHES "yes")
    # --export-dynamic is not needed on osol
    set(link_flags "${link_flags} -Wl,--export-dynamic")
endif()

path64_set_multiarch_property(be-exec LINK_FLAGS "${link_flags}")

path64_install_multiarch(be-exec)


# ipl -> be symlinks

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(LINK_COPY copy)
else()
    set(LINK_COPY create_symlink)
endif()

set(ipl_symlink_deps)

foreach(arch ${PATH64_ENABLE_ARCHES})
    set(stage_link ${Path64_BINARY_DIR}/lib/${PSC_FULL_VERSION}/${arch}/ipl${CMAKE_EXECUTABLE_SUFFIX})
    add_custom_command(
        OUTPUT ${stage_link}
	COMMAND ${CMAKE_COMMAND} -E ${LINK_COPY} be${CMAKE_EXECUTABLE_SUFFIX} ${stage_link}
        WORKING_DIRECTORY ${Path64_BINARY_DIR}/lib/${arch}
    )
    list(APPEND ipl_symlink_deps ${stage_link})

    install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E ${LINK_COPY} \"be${CMAKE_EXECUTABLE_SUFFIX}\" \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/${PSC_FULL_VERSION}/${arch}/ipl${CMAKE_EXECUTABLE_SUFFIX}\")")

endforeach()

add_custom_target(ipl-symlinks DEPENDS ${ipl_symlink_deps})
path64_add_dependencies_from_multiarch(ipl-symlinks be-exec)

path64_add_dependencies_from_multiarch(be-all be-exec be)
add_dependencies(be-all ipl-symlinks)

