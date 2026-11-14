# crtbegin.o and crtend.o for IRIX n32, assembled with the staged
# compiler (pathcc hands .s files to GNU as).

set(crt_out ${Path64_BINARY_DIR}/lib/mips/32)
set(crt_cc ${Path64_BINARY_DIR}/bin/pathcc)

foreach(crt crtbegin crtend)
    add_custom_command(
        OUTPUT ${crt_out}/${crt}.o
        COMMAND ${CMAKE_COMMAND} -E make_directory ${crt_out}
        COMMAND ${crt_cc} -n32 -c ${CMAKE_CURRENT_SOURCE_DIR}/${crt}.s
                -o ${crt_out}/${crt}.o
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${crt}.s
    )
    add_custom_target(${crt}-mips_32 ALL DEPENDS ${crt_out}/${crt}.o)
    # Needs the staged driver; the C runtime is linked with it.
    add_dependencies(${crt}-mips_32 compiler-stage-C)
    add_dependencies(compiler-stage-libs-C ${crt}-mips_32)
endforeach()

install(FILES ${crt_out}/crtbegin.o ${crt_out}/crtend.o
        DESTINATION ${PATH64_LIB_PATH}/mips/32)
