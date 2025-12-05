

# Generates target info
function(generate_targ_info config_name output)
    set(TARGET_DEFINITIONS "")
    set(first true)
    foreach(targ ${PATH64_ENABLE_TARGETS})
        set(arch ${_PATH64_TARGET_ARCH_${targ}})
        set(abi_name ${_PATH64_TARGET_BITS_${targ}})
        set(abi ${_PATH64_TARGET_ABI_${targ}})
        set(include_path ${PSC_INCLUDE_PATH_${targ}})
        set(crt_path ${PSC_CRT_PATH_${targ}})
        set(crtbegin_path ${PSC_CRTBEGIN_PATH_${targ}})

        # checking CRT path

        if("${crt_path}" STREQUAL "")
            message(FATAL_ERROR "CRT path for target '${targ}' is not specified. Please set PSC_CRT_PATH_${targ} variable")
        endif()

        if(NOT IS_ABSOLUTE "${crt_path}")
            message(FATAL_ERROR "CRT path for target '${targ}' must be absolute. Specified value: '${crt_path}'")
        endif()

        if("${include_path}" STREQUAL "")
            message(FATAL_ERROR "Include path for target '${targ}' is not specified. Please set PSC_INCLUDE_PATH_${targ} variable")
        endif()

        if(NOT IS_ABSOLUTE "${include_path}")
            message(FATAL_ERROR "Include path for target '${targ}' must be absolute. Specified value: '${include_path}'")
        endif()

        # check files only for native config
        if("${config_name}" STREQUAL "")

            if(NOT EXISTS "${crt_path}/crt1.o")
                message(FATAL_ERROR "file '${crt_path}/crt1.o' does not exist. Please check CRT path for target '${targ}'")
            endif()

            if(NOT EXISTS "${crt_path}/crti.o")
                message(FATAL_ERROR "file '${crt_path}/crti.o' does not exist. Please check CRT path for target '${targ}'")
            endif()

            if(NOT EXISTS "${crt_path}/crtn.o")
                message(FATAL_ERROR "file '${crt_path}/crtn.o' does not exist. Please check CRT path for target '${targ}'")
            endif()

        endif()

    
        # checking dynamic linker

        if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            set(dyn_linker ${PSC_DYNAMIC_LINKER_${targ}})
            if(NOT dyn_linker)
                if("${targ}" STREQUAL "x86_32")
                    set(dyn_linker "/lib/ld-linux.so.2")
                elseif("${targ}" STREQUAL "x86_64")
                    set(dyn_linker "/lib64/ld-linux-x86-64.so.2")
                else()
                    message(FATAL_ERROR "Dynamic linker for ${targ} target is not specified. Please specify PSC_DYNAMIC_LINKER_${targ} variable")
                endif()
            endif()

            message(STATUS "Dynamic linker for ${targ} target: ${dyn_linker}")
        endif()
    
        if(NOT first)
            set(TARGET_DEFINITIONS "${TARGET_DEFINITIONS},\n")
        else()
            set(first false)
        endif()
    
        set(TARGET_DEFINITIONS "${TARGET_DEFINITIONS}\t{\"${targ}\",\t\"${arch}\",\t\"${abi_name}\",\t${abi},\t\"${include_path}\",\t\"${crt_path}\",\t\"${crtbegin_path}\"")
    
        if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            set(TARGET_DEFINITIONS "${TARGET_DEFINITIONS},\t\"${dyn_linker}\"")
        endif()
    
        if(NOT PATH64_ENABLE_PSCRUNTIME)
            set(supcpp_path ${PSC_LIBSUPCPP_PATH_${targ}})
            set(stdcpp_path ${PSC_LIBSTDCPP_PATH_${targ}})
            set(gcc_path ${PSC_LIBGCC_PATH_${targ}})
            set(gcc_eh_path ${PSC_LIBGCC_EH_PATH_${targ}})
            set(gcc_s_path ${PSC_LIBGCC_S_PATH_${targ}})
    
            if(NOT supcpp_path)
                message(FATAL_ERROR "Path to libsupc++ for target ${targ} is not specified. Please specify PSC_LIBSUPCPP_PATH_${targ} variable")
            endif()
    
            if(NOT stdcpp_path)
                message(FATAL_ERROR "Path to libstdc++ for target ${targ} is not specified. Please specify PSC_LIBSTDCPP_PATH_${targ} variable")
            endif()
    
            if(NOT gcc_path)
                message(FATAL_ERROR "Path to libgcc for target ${targ} is not specified. Please specify PSC_LIBGCC_PATH_${targ} variable")
            endif()
    
            if(NOT gcc_eh_path)
                message(FATAL_ERROR "Path to libgcc_eh for target ${targ} is not specified. Please specify PSC_LIBGCC_EH_PATH_${targ} variable")
            endif()
    
            if(NOT gcc_s_path)
                message(FATAL_ERROR "Path to libgcc_s for target ${targ} is not specified. Please specify PSC_LIBGCC_S_PATH_${targ} variable")
            endif()
    
            set(TARGET_DEFINITIONS "${TARGET_DEFINITIONS},\t\"${supcpp_path}\",\t\"${stdcpp_path}\",\t\"${gcc_path}\",\t\"${gcc_eh_path}\",\t\"${gcc_s_path}\"")
        endif()
    
        set(TARGET_DEFINITIONS "${TARGET_DEFINITIONS}}")
    endforeach()
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/targ_info.c.cmake.in"
                   "${output}")
endfunction()


# Configures driver_defs.h header
function(configure_driver_defs output hash_style_supported)
    if(hash_style_supported)
        set(LD_HASH_STYLE_FLAG "--hash-style=gnu")
    else()
        set(LD_HASH_STYLE_FLAG "")
    endif()
    
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    	SET(PATH64_AUXILARY_LD_FLAGS "\"${LD_HASH_STYLE_FLAG} --eh-frame-hdr\"")
    elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
    	SET(PATH64_AUXILARY_LD_FLAGS "\"--eh-frame-hdr\"")
    else()
    	SET(PATH64_AUXILARY_LD_FLAGS "\"\"")
    endif()
    
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/driver_defs.h.cmake.in"
                   "${output}")
endfunction()


# Adds driver with specified properties
function(add_driver config_name
                    hash_style_supported)

    if("${config_name}" STREQUAL "")
        # default configuration
        set(suffix "")
    else()
        set(suffix "_${config_name}")
    endif()

    set(include_dir "${CMAKE_CURRENT_BINARY_DIR}/include${suffix}")

    # Configuring driver_defs.h
    file(MAKE_DIRECTORY "${include_dir}")
    configure_driver_defs("${include_dir}/driver_defs.h"
                          ${hash_style_supported})
    
    # generating target info
    set(targ_info_source "${CMAKE_CURRENT_BINARY_DIR}/targ_info${suffix}.c")
    generate_targ_info("${config_name}" "${targ_info_source}")
    
    add_executable(pathcc${suffix} ${PATHSCALE_DRIVER_SRCS} "${targ_info_source}")
    set_property(TARGET pathcc${suffix} PROPERTY COMPILE_FLAGS "-I${include_dir}")

    if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_link_libraries(pathcc${suffix} iberty)
    endif()

    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
        set_property(TARGET pathcc${suffix} PROPERTY LINK_FLAGS "-static")
    endif()

    install(TARGETS pathcc${suffix}
            RUNTIME DESTINATION bin)
    add_dependencies(pathcc${suffix} driver_gen_sources)
endfunction()


set(PATHSCALE_COMMON_DRIVER_SRCS "errors.c"
                                 "file_utils.c"
                                 "lang_defs.c"
                                 "string_utils.c"
   )

set(PATHSCALE_DRIVER_GENERATED_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/check_combos.c
    ${CMAKE_CURRENT_BINARY_DIR}/implicits.c
    ${CMAKE_CURRENT_BINARY_DIR}/version_hg.c)

set(PATHSCALE_DRIVER_SRCS ${PATHSCALE_COMMON_DRIVER_SRCS}
                          "main.c"
                          "options.c"
                          "option_seen.c"
                          "get_options.c"
                          "opt_actions.c"
                          "file_names.c"
                          "objects.c"
                          "phases.c"
                          "run.c"
                          "special_options.c"
                          "targets.c"
                          "license.c"
                          ${PATHSCALE_DRIVER_GENERATED_SRCS}
   )

set_source_files_properties(${PATHSCALE_DRIVER_GENERATED_SRCS} PROPERTIES GENERATED TRUE)

set(PATHSCALE_TABLE_SRCS  ${PATHSCALE_COMMON_DRIVER_SRCS}
                          "table.c"
   )


# Common definitions

set(driver_defs -DLITTLE_ENDIAN_HOST
                -DHOST_IA32
                -DNOOPT
                -DPSCSUB)

option(PATH64_ENABLE_PATHAS "Enables using pathas assembler" OFF)
if(PATH64_ENABLE_PATHAS)
    list(APPEND driver_defs "-DPATH64_ENABLE_PATHAS")
endif()

option(PATH64_ENABLE_DEFAULT_PATHAS "Enables using pathas as default assembler" OFF)
if(PATH64_ENABLE_DEFAULT_PATHAS)
    list(APPEND driver_defs "-DPATH64_ENABLE_DEFAULT_PATHAS")
endif()


if(PATH64_ENABLE_GNU_FRONTEND)
    list(APPEND driver_defs "-DPATH64_ENABLE_GNU_FRONTEND")
endif()


option(PATH64_ENABLE_DEFAULT_PSCLANG "Enables using psclang as default front-end" OFF)

if(NOT PATH64_ENABLE_GNU_FRONTEND)
    set(PATH64_ENABLE_DEFAULT_PSCLANG ON)
endif()

if(PATH64_ENABLE_DEFAULT_PSCLANG)
    list(APPEND driver_defs "-DPATH64_ENABLE_DEFAULT_PSCLANG")
endif()


option(PATH64_NO_LICENSE_CHECK "Disables license check in driver" ON)
if(PATH64_NO_LICENSE_CHECK)
    list(APPEND driver_defs -DNO_LICENSE_CHECK)
endif()


# Adding definitions for target architectures
set(ARCH_DEFS)
foreach(arch ${PATH64_ENABLE_ARCHES})
    set(ARCH_DEFS ${ARCH_DEFS} ${_PATH64_ARCH_FLAGS_${arch}})
endforeach()
list(APPEND driver_defs ${ARCH_DEFS})


if(PATH64_ENABLE_PSCRUNTIME)
    list(APPEND driver_defs "-DPATH64_ENABLE_PSCRUNTIME")
else()
    list(APPEND driver_defs "-DPATH64_NO_PSCRUNTIME")
endif()


string(REGEX MATCH "^[^-]+" targ ${PSC_TARGET})
path64_canonical_arch(targ ${targ})
path64_get_host_arch(host_arch)
path64_get_target_arch(targ_arch "${targ}")
if(NOT "${host_arch}" STREQUAL "${targ_arch}")
    list(APPEND driver_defs -DPATH64_CROSS)
endif()


# TODO: set proper build date and time 
add_definitions(-std=gnu99 ${driver_defs})


include_directories("."
                    "../common/com"
                    "${CMAKE_CURRENT_BINARY_DIR}")


# table executable

add_executable(table ${PATHSCALE_TABLE_SRCS})
set_property(TARGET table PROPERTY COMPILE_FLAGS "-I${CMAKE_CURRENT_BINARY_DIR}/include")


# Detecting gcc and configuring sort_options script

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

add_executable(sort_options sort_options.cpp)
GET_TARGET_PROPERTY(sort_options_exe sort_options LOCATION)

add_custom_command(
        OUTPUT tmp.options.cpp
        COMMAND cpp -traditional -P ${driver_defs} -DPSC_PRODNAME='"Compiler"' -DKEY -DLITTLE_ENDIAN_HOST -DHOST_IA32 -DNOOPT ${CMAKE_CURRENT_SOURCE_DIR}/OPTIONS > tmp.options.cpp 
        DEPENDS OPTIONS
        )

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/OPTIONS.P
        # COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/sort_options.sh ${CMAKE_CURRENT_SOURCE_DIR} OPTIONS -DX8664 -DPSC_PRODNAME='"Compiler"' -DKEY   -DLITTLE_ENDIAN_HOST -DHOST_IA32 -DNOOPT > ${CMAKE_CURRENT_BINARY_DIR}/OPTIONS.P 
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/sort_options.${SHELL_EXT} ${sort_options_exe} > ${CMAKE_CURRENT_BINARY_DIR}/OPTIONS.P 
        DEPENDS sort_options.${SHELL_EXT} OPTIONS sort_options tmp.options.cpp
        )

else()

# Detecting gawk and gcc and configuring sort_options script
set(SORT_OPTIONS ${CMAKE_CURRENT_BINARY_DIR}/sort_options.sh)

set(PATH64_GCC ${CMAKE_C_COMPILER})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sort_options.sh.cmake.in
               ${SORT_OPTIONS})


# Generating options sources

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/OPTIONS.P
        COMMAND ${SORT_OPTIONS} ${CMAKE_CURRENT_SOURCE_DIR} OPTIONS ${driver_defs} -DPSC_PRODNAME='"Compiler"' -DKEY   -DLITTLE_ENDIAN_HOST -DHOST_IA32 -DNOOPT > ${CMAKE_CURRENT_BINARY_DIR}/OPTIONS.P 
        DEPENDS ${SORT_OPTIONS} OPTIONS
        )

endif()


add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/check_combos.c
        ${CMAKE_CURRENT_BINARY_DIR}/get_option.i
        ${CMAKE_CURRENT_BINARY_DIR}/implicits.c
        ${CMAKE_CURRENT_BINARY_DIR}/init_options.i
        ${CMAKE_CURRENT_BINARY_DIR}/opt_action.i
        ${CMAKE_CURRENT_BINARY_DIR}/option_names.h
        COMMAND table < ${CMAKE_CURRENT_BINARY_DIR}/OPTIONS.P
        DEPENDS table ${CMAKE_CURRENT_BINARY_DIR}/OPTIONS.P
        )

# Generate the version strings	
if(WIN32)
	file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}" BUILD_ROOT)
	string(REPLACE "\\" "\\\\" BUILD_ROOT "${BUILD_ROOT}")
else()
	set(BUILD_ROOT "${CMAKE_CURRENT_BINARY_DIR}")
endif()

site_name(BUILD_HOST)
mark_as_advanced(BUILD_HOST)
set(BUILD_USER $ENV{USER})
set(BUILD_DATE "${PATH64_BUILD_DATE}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version_hg.c.in ${CMAKE_CURRENT_BINARY_DIR}/version_hg.c ESCAPE_QUOTES @ONLY)

add_custom_target(driver_gen_sources
                  DEPENDS
                  ${PATHSCALE_DRIVER_GENERATED_SRCS})



# Driver for host system

# detecting CRT paths for all targets
foreach(targ ${PATH64_ENABLE_TARGETS})
    set(crt_path_name "PSC_CRT_PATH_${targ}")
    set(crt_path "${${crt_path_name}}")

    if("${crt_path}" STREQUAL "")
        execute_process(COMMAND "${CMAKE_C_COMPILER}" "${_PATH64_TARGET_FLAG_${targ}}" "-print-file-name=crt1.o"
                        RESULT_VARIABLE res
                        OUTPUT_VARIABLE crt_path)
        string(REPLACE "\n" "" crt_path "${crt_path}")
        if(res EQUAL 0 AND NOT "${crt_path}" STREQUAL "")
            get_filename_component(crt_path "${crt_path}" ABSOLUTE)
            if(EXISTS "${crt_path}")
                get_filename_component(crt_path "${crt_path}" PATH)
                set(${crt_path_name} "${crt_path}")
            endif()
        endif()
    endif()
endforeach()


# setting default paths to include dirs
foreach(targ ${PATH64_ENABLE_TARGETS})
    set(include_path_name "PSC_INCLUDE_PATH_${targ}")
    set(include_path "${${include_path_name}}")

    if("${include_path}" STREQUAL "")
        if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
            set(${include_path_name} "/usr/include/${targ}-linux-gnu")
        else()
            set(${include_path_name} "/usr/include")
        endif()
    endif()
endforeach()


# checking for --as-needed
message(STATUS "Checking for --as-needed")
set(as_needed_supported FALSE)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(test_dir "${CMAKE_CURRENT_BINARY_DIR}/as_needed")
    make_directory(test_dir)
    write_file("${test_dir}/bar.c" "void bar() {}")
    write_file("${test_dir}/foo.c" "void foo() { bar(); }")
    write_file("${test_dir}/test.c" "int main() { foo(); }")
    execute_process(COMMAND ${CMAKE_C_COMPILER} -shared -o libfoo.so foo.c -fPIC
                    WORKING_DIRECTORY "${test_dir}"
                    RESULT_VARIABLE res)
    if(res EQUAL 0)
        execute_process(COMMAND ${CMAKE_C_COMPILER} -shared -o libbar.so bar.c -fPIC
                        WORKING_DIRECTORY "${test_dir}"
                        RESULT_VARIABLE res)
        if(res EQUAL 0)
            execute_process(COMMAND ${CMAKE_C_COMPILER} -fPIC -o test test.c
                                    -L. -Wl,--as-needed -lfoo -lbar -Wl,--no-as-needed
                            WORKING_DIRECTORY "${test_dir}"
                            RESULT_VARIABLE res)
            if(res EQUAL 0)
                set(as_needed_supported TRUE)
            endif()
        endif()
    endif()
endif()

if(as_needed_supported)
    message(STATUS "--as-needed supported")
else()
    message(STATUS "--as-needed not supported")
    set(NO_AS_NEEDED TRUE)
endif()

add_driver("" ${hash_style_supported})


# Drivers for various linux distros

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")

    set(NO_AS_NEEDED FALSE)

    # deb configuration - for debian multiarch distros with multiarch includes
    set(PSC_INCLUDE_PATH_x86_64 "/usr/include/x86_64-linux-gnu")
    set(PSC_INCLUDE_PATH_x86_32 "/usr/include")
    set(PSC_CRT_PATH_x86_64 "/usr/lib/x86_64-linux-gnu")
    set(PSC_CRT_PATH_x86_32 "/usr/lib32")
    add_driver("deb_multiarch_includes" "TRUE")

    set(PSC_INCLUDE_PATH_x86_64 "/usr/include")
    set(PSC_INCLUDE_PATH_x86_32 "/usr/include")

    # deb configuration - for debian multiarch distros
    set(PSC_CRT_PATH_x86_64 "/usr/lib/x86_64-linux-gnu")
    set(PSC_CRT_PATH_x86_32 "/usr/lib32")
    add_driver("deb_multiarch" "TRUE")

    # deb configuration - for debian based distros
    set(PSC_CRT_PATH_x86_64 "/usr/lib")
    set(PSC_CRT_PATH_x86_32 "/usr/lib32")
    add_driver("deb_hash_style" "TRUE")

    # rpm configuration - for rpm based distros
    set(PSC_CRT_PATH_x86_64 "/usr/lib64")
    set(PSC_CRT_PATH_x86_32 "/usr/lib")
    add_driver("rpm_hash_style" "TRUE")

    # deb configuration without --hash-style ld option support
    set(PSC_CRT_PATH_x86_64 "/usr/lib")
    set(PSC_CRT_PATH_x86_32 "/usr/lib32")
    add_driver("deb_no_hash_style" "FALSE")

    # rpm configuration without --hash-style ld option support
    set(PSC_CRT_PATH_x86_64 "/usr/lib64")
    set(PSC_CRT_PATH_x86_32 "/usr/lib")
    add_driver("rpm_no_hash_style" "FALSE")

    # rpm configuration without --hash-style ld option support
    set(NO_AS_NEEDED TRUE)
    set(PSC_CRT_PATH_x86_64 "/usr/lib64")
    set(PSC_CRT_PATH_x86_32 "/usr/lib")
    add_driver("rpm_no_hash_style_no_as_needed" "FALSE")
endif()



if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

target_link_libraries(table iberty winlib)
target_link_libraries(${exe_refix}pathcc iberty winlib ws2_32)

add_custom_target(
    stageit-driver ALL
    DEPENDS pathcc
)

else()

if(PATH64_ENABLE_CXX)
    add_custom_command(
        OUTPUT ${Path64_BINARY_DIR}/bin/pathCC ${Path64_BINARY_DIR}/lib/${PSC_FULL_VERSION}
        DEPENDS pathcc
        # Paths in symbolic links should be relative. Otherwise bootstrap will not work
		# TODO does this work with LINK_COPY on windows?
        COMMAND ${CMAKE_COMMAND} -E ${LINK_COPY} pathcc ${Path64_BINARY_DIR}/bin/pathCC
    )

    add_custom_target(
        stageit-driver ALL
        DEPENDS ${Path64_BINARY_DIR}/bin/pathCC
    )

    add_dependencies(compiler-stage stageit-driver)


    install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E ${LINK_COPY} \"pathcc${CMAKE_EXECUTABLE_SUFFIX}\" \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/pathCC${CMAKE_EXECUTABLE_SUFFIX}\")")
    install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E ${LINK_COPY} \"pathCC${CMAKE_EXECUTABLE_SUFFIX}\" \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/pathCC-${PSC_FULL_VERSION}${CMAKE_EXECUTABLE_SUFFIX}\")")
endif()

endif()

if(PATH64_ENABLE_FORTRAN)
install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E ${LINK_COPY} \"pathcc${CMAKE_EXECUTABLE_SUFFIX}\" \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/pathf95${CMAKE_EXECUTABLE_SUFFIX}\")")
install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E ${LINK_COPY} \"pathcc${CMAKE_EXECUTABLE_SUFFIX}\" \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/pathf90${CMAKE_EXECUTABLE_SUFFIX}\")")
endif()
install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E ${LINK_COPY} \"pathcc${CMAKE_EXECUTABLE_SUFFIX}\" \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/pathcc-${PSC_FULL_VERSION}${CMAKE_EXECUTABLE_SUFFIX}\")")

if(PATH64_ENABLE_FORTRAN)
install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E ${LINK_COPY} \"pathf95${CMAKE_EXECUTABLE_SUFFIX}\" \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/pathf95-${PSC_FULL_VERSION}${CMAKE_EXECUTABLE_SUFFIX}\")")
install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E ${LINK_COPY} \"pathf90${CMAKE_EXECUTABLE_SUFFIX}\" \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/pathf90-${PSC_FULL_VERSION}${CMAKE_EXECUTABLE_SUFFIX}\")")
endif()
