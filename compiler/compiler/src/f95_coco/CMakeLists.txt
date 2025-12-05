# this should build coco
SET(PATHSCALE_COCO_SRCS "coco.F90" )

option(PATH64_ENABLE_COCO "Enable f95 coco preprocessor" ON)

set_source_files_properties("coco.F90" PROPERTIES LANGUAGE Fortran)
if(PATH64_ENABLE_FORTRAN)
    if(PATH64_ENABLE_COCO)

        set(flags -D_SGI_SOURCE -D__GNU_BUG_WORKAROUND -DKEY -DFE_GNU_4_2_0 -D_LONGLONG -D_MIPSEL -DTARG_LINUX)

        if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
            set(build_flags "${CMAKE_Fortran_FLAGS_DEBUG}")
        elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
            set(build_flags "${CMAKE_Fortran_FLAGS_RELEASE}")
        endif()

        string(REPLACE " " ";" build_flags_list "${build_flags}")
        set(flags ${flags} ${build_flags_list})

        set(link_flags -lm)

        if(PATH64_ENABLE_PSCRUNTIME AND "X${CMAKE_BUILD_TYPE}" STREQUAL "XRelease")
            set(link_flags -nodefaultlibs ${link_flags} -Wl,-Bstatic -lmv -lmpath -lpathfstart -lpathfortran -leh -lgcc -lpscrt -Wl,-Bdynamic)
        endif()

        set(coco_deps)
        foreach(arch ${PATH64_ENABLE_ARCHES})
            set(coco_exe "${Path64_BINARY_DIR}/lib/${arch}/coco")

            add_custom_command(OUTPUT "${coco_exe}"
                               COMMAND ${Path64_BINARY_DIR}/bin/pathf90 ${flags} -o ${coco_exe}
                                       ${CMAKE_CURRENT_SOURCE_DIR}/coco.F90 ${link_flags}
                               DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/coco.F90)

            list(APPEND coco_deps "${coco_exe}")

            install(PROGRAMS "${coco_exe}"
                    DESTINATION "${PATH64_LIB_PATH}/${arch}")
        endforeach()

        add_custom_target(coco-all ALL DEPENDS ${coco_deps})
        add_dependencies(coco-all compiler-stage-libs-Fortran)
    endif()
endif()
