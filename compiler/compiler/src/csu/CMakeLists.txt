

path64_is_target_enabled(res x86_64)
if(res)
    ADD_SUBDIRECTORY(elf-x86_64)
endif()

path64_is_target_enabled(res x86_32)
if(res)
    ADD_SUBDIRECTORY(elf-i386)
endif()


path64_is_target_enabled(res mips_32)
if(res AND "${PSC_TARGET_OS_mips_32}" STREQUAL "irix")
    ADD_SUBDIRECTORY(irix-mips)
endif()
