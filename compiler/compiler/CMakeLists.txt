CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
PROJECT(Path64)

ENABLE_TESTING()

# Disallow in-source build
IF("${Path64_SOURCE_DIR}" STREQUAL "${Path64_BINARY_DIR}")
  MESSAGE(FATAL_ERROR "Project must be built out-of-source.  Remove files from the source tree that were created by this attempt:
  rm -rf CMakeFiles CMakeCache.txt
Then create a separate build directory and run CMake there.
  mkdir ../build
  cd ../build
  cmake ../
This approach keeps the source tree pristine.")
ENDIF()

# Search for various include files and functions
include(${Path64_SOURCE_DIR}/cmake/Modules/ConfigureChecks.cmake)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(LINK_COPY copy_if_different)
else()
    set(LINK_COPY create_symlink)
endif()

# Force the build_type=debug flags to sane values overriding users choices
SET(CMAKE_C_FLAGS_DEBUG  "-O0 -g -D_DEBUG -DIs_True_On=1" CACHE STRING "Flags used by the compiler during debug builds." FORCE)
SET(CMAKE_CXX_FLAGS_DEBUG  "-Wno-deprecated -O0 -g -D_DEBUG -DIs_True_On=1" CACHE STRING "Flags used by the compiler during debug builds." FORCE)
SET(CMAKE_Fortran_FLAGS_DEBUG  "-O0 -g3 -D_DEBUG -DIs_True_On=1" CACHE STRING "Flags used by the compiler during debug builds." FORCE)

# Using -mno-sse3 flag for release build if PATH64_ENABLE_PSCRUNTIME=ON
if(PATH64_ENABLE_PSCRUNTIME)
    set(SSE_FLAGS "-mno-sse3")
endif()

# Force the build_type=release flags to -O2 -DNDEBUG values overriding default cmake -O3
SET(CMAKE_C_FLAGS_RELEASE  "-O2 -DNDEBUG ${SSE_FLAGS}" CACHE STRING "Flags used by the compiler during release builds." FORCE)
SET(CMAKE_CXX_FLAGS_RELEASE  "-Wno-deprecated -O2 -DNDEBUG ${SSE_FLAGS}" CACHE STRING "Flags used by the compiler during release builds." FORCE)
SET(CMAKE_Fortran_FLAGS_RELEASE  "-O2 -DNDEBUG ${SSE_FLAGS}" CACHE STRING "Flags used by the compiler during release builds." FORCE)


OPTION(PATH64_ENABLE_GNU_FRONTEND "Use GNU C/C++ front-end" ON)
OPTION(PATH64_ENABLE_IPA "Enable IPA" ON)


# Define user option whether to build the Fortran compiler.
OPTION(PATH64_ENABLE_FORTRAN "Build Fortran compiler" ON)

if(PATH64_ENABLE_FORTRAN)
    if("${CMAKE_Fortran_COMPILER}" STREQUAL "")
        set(CMAKE_Fortran_COMPILER_FORCED TRUE)
        set(CMAKE_Fortran_COMPILER "true")
    endif()
    enable_language(Fortran)
endif()

# Define user option whether to build the profiling libraries
OPTION(PATH64_ENABLE_PROFILING "Build profiling libraries" ON)
IF(PATH64_ENABLE_PROFILING)
	MESSAGE(STATUS "Profiling build enabled")
ELSE()
	MESSAGE(STATUS "Profiling build disabled")
ENDIF()

# Define user option whether to build OpenMP
OPTION(PATH64_ENABLE_OPENMP "Build OpenMP" ON)
IF(PATH64_ENABLE_OPENMP)
	MESSAGE(STATUS "OpenMP build enabled")
ELSE()
	MESSAGE(STATUS "OpenMP build disabled")
ENDIF()


# GNU-style hashing needs support of the dynamic linker, so white-list
# potential platforms.
IF (NOT DEFINED SUPPORT_HASH_STYLE_GNU)
    SET(SOURCE_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/hash-style.c")

    FILE(WRITE "${SOURCE_FILE}" "#include <stdio.h>
int main(void)
{
	return printf(\"\");
}
")

    MESSAGE(STATUS "Checking for --hash-style=gnu support")
    TRY_RUN(SUPPORT_HASH_STYLE_GNU SUPPORT_HASH_STYLE_GNU_BUILD
	${CMAKE_BINARY_DIR} ${SOURCE_FILE}
        CMAKE_FLAGS -DLINK_LIBRARIES:STRING=-Wl,--hash-style=gnu
	OUTPUT_VARIABLE OUTPUT)
    IF (SUPPORT_HASH_STYLE_GNU_BUILD AND SUPPORT_HASH_STYLE_GNU EQUAL 0)
	MESSAGE(STATUS "Checking for --hash-style=gnu support -- found ")
    ELSE()
	MESSAGE(STATUS "Checking for --hash-style=gnu support -- not found ")
    	FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
    	    "Checking support for --hash-style=gnu support failed with the following output:\n"
    	    "${OUTPUT}\n")
    ENDIF()
ENDIF()


if(SUPPORT_HASH_STYLE_GNU_BUILD AND SUPPORT_HASH_STYLE_GNU EQUAL 0)
    set(hash_style_supported TRUE)
else()
    set(hash_style_supported FALSE)
endif()

#-----------------------------------------------------------------------------
# Target architecture stuff.
include(${Path64_SOURCE_DIR}/src/targets.cmake)


# Use -G0 option if building with pathscale on MIPS
if("${CMAKE_C_COMPILER_ID}" STREQUAL "PathScale")
    path64_get_host_arch(arch)
    if("${arch}" STREQUAL "mips")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -G0")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -G0")
        set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -G0")
    endif()
endif()


if(NOT PATH64_USE_SYSTEM_COMPILER_FOR_TARGET_LIBS OR "${CMAKE_C_COMPILER_ID}" STREQUAL "PathScale")
    set(BOOTSTRAP_HACK_FLAG "-fbootstrap-hack")
else()
    set(BOOTSTRAP_HACK_FLAG "")
endif()


# Define if we are doing a supported build
OPTION(PATH64_ENABLE_SUPPORTED "Enabling supported build" OFF)
IF(PATH64_ENABLE_SUPPORTED)
	MESSAGE(FATAL_ERROR "This source tree is still not officially supported yet")
ELSE()
	MESSAGE(STATUS "Community supported version.  For officially supported version please visit http://www.pathscale.com")
ENDIF()

# Valgrind?
OPTION(PATH64_ENABLE_VALGRIND "Enabling valgrind" OFF)
IF(PATH64_ENABLE_VALGRIND)
	MESSAGE(STATUS "Valgrind enabled")
ELSE()
	MESSAGE(STATUS "Valgrind disabled")
ENDIF()

# Whatever arbitrary number you'd like in 3 string format eg 3.9.99
IF(NOT PSC_FULL_VERSION)
    SET(PSC_FULL_VERSION "4.0.10" CACHE STRING "Defaulting to version 4.0.10")
ENDIF()

MESSAGE(STATUS "Version is set to "${PSC_FULL_VERSION})
string(REGEX REPLACE "^([0-9.]+).*$" "\\1" TMPVERSION ${PSC_FULL_VERSION})
# If it's not in three string format it sucks to be you
string(REGEX REPLACE "^([0-9]+).+$" "\\1" PSC_MAJOR_VERSION ${TMPVERSION})
string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*$" "\\1" PSC_MINOR_VERSION ${TMPVERSION})

IF(NOT PSC_TARGET)
    path64_get_host_arch(arch)
    IF("${arch}" STREQUAL "x8664")
        set(arch "x86_64")
    ELSEIF("${arch}" STREQUAL "x8632")
        set(arch "i386")
    ENDIF()
    string(TOLOWER "${arch}-unknown-${CMAKE_SYSTEM_NAME}" PSC_TARGET)
    IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(PSC_TARGET "${PSC_TARGET}-gnu")
    ENDIF()
ENDIF()

# Checking that CRT paths are specified for all targets
FOREACH(targ ${PATH64_ENABLE_TARGETS})
    if(PSC_CRTBEGIN_PATH_${targ})
        message(STATUS "crtbegin/crtend path for ${targ} target: ${PSC_CRTBEGIN_PATH_${targ}}")
    else()
        set(arch "${_PATH64_TARGET_ARCH_${targ}}")
        if("${arch}" STREQUAL "x8664")
            set(PSC_CRTBEGIN_PATH_${targ} "LOCAL")
        else()
            message(FATAL_ERROR "crtbegin/crtend path for ${targ} target is not specified. Please specify PSC_CRTBEGIN_PATH_${targ} variable")
        endif()
    endif()
ENDFOREACH()

# Do we build with huge pages support
OPTION(PATH64_ENABLE_HUGEPAGES "Enabling supported huge pages" OFF)
IF(PATH64_ENABLE_HUGEPAGES)
	IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		MESSAGE(STATUS "Building with huge pages support.")
	ELSE()
		MESSAGE(FATAL_ERROR "Huge pages not supported on this platform yet.  Please disable this option or contact PathScale for support")
	ENDIF()
ELSE()
	MESSAGE(STATUS "Not building with huge pages support")
ENDIF()

# Use X86 elf file format for whirl objects
OPTION(X86_WHIRL_OBJECTS "Use x86 elf file format for whirl object files" ON)
IF(X86_WHIRL_OBJECTS)
        MESSAGE(STATUS "Using x86 elf file format for whirl object files")
        ADD_DEFINITIONS(-DX86_WHIRL_OBJECTS)
ELSE()
        MESSAGE(STATUS "Using MIPS elf file format for whirl object files")
ENDIF()

# Fuse WHIRL and native code in IPA objects
OPTION(FAT_WHIRL_OBJECTS " Fuse WHIRL and native code in IPA objects" OFF)
IF(FAT_WHIRL_OBJECTS)
        MESSAGE(STATUS "Configuring for fat IPA objects")
        ADD_DEFINITIONS(-DFAT_WHIRL_OBJECTS)
ELSE()
        MESSAGE(STATUS "Configuring for simple WHIRL IPA objects")
ENDIF()


# Are we using the PathScale runtime
OPTION(PATH64_ENABLE_PSCRUNTIME "Using the PathScale c and c++ runtime" OFF)
IF(PATH64_ENABLE_PSCRUNTIME)
	ADD_DEFINITIONS(-DPATH64_ENABLE_PSCRUNTIME)
	MESSAGE(STATUS "PathScale runtime enabled")
ELSE()
	MESSAGE(STATUS "GNU runtime enabled")
ENDIF()


OPTION(PATH64_ENABLE_CXX "Enable C++ support" ON)


# Optional path to libdwarf
OPTION(PATH64_LIBDWARF_PATH "Optional path to libdwarf library")


SET(NAMEPREFIX "path" CACHE STRING "Default name excutable prefix [path]")
IF(NAMEPREFIX)
	MESSAGE(STATUS "Executables will be prefixed with ${NAMEPREFIX}")
ELSE()
	MESSAGE( FATAL_ERROR "The executable name prefix must not be empty." )
ENDIF()


# Set default install prefix (instead of CMake's /usr/local default).
# TODO : How do we fix this so cmake -i shows the correct default prefix?
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX "/opt/path64" CACHE PATH "Installation root prefix" FORCE)
ENDIF()

# On Linux some things needs to link with -ldl
# but on FBSD they should not.
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(libdl "dl")
ELSE()
    set(libdl "")
ENDIF()


# Selecting macro for enabling int types formats for C++
SET(PATH64_STDC_FORMAT_MACROS -D__STDC_FORMAT_MACROS)

OPTION(PATH64_BUILD_32 "Build 32-bit version" OFF)

IF(PATH64_BUILD_32)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    SET(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -m32")
    SET(CMAKE_EXE_LINKER_FLAGS "-m32")
    SET(CMAKE_MODULE_LINKER_FLAGS "-m32")
    SET(CMAKE_SHARED_LINKER_FLAGS "-m32")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
# Forching build for 64-bit on osol
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    SET(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -m64")
    SET(CMAKE_EXE_LINKER_FLAGS "-m64")
    SET(CMAKE_MODULE_LINKER_FLAGS "-m64")
    SET(CMAKE_SHARED_LINKER_FLAGS "-m64")
ENDIF()


# Tell GCC_XML that we are providing gccxml_cc1plus for it.
SET(GCCXML_NATIVE_CC1PLUS 1)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/GCCVersion.cmake)


# Place all targets near the top of the build tree by default.
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Path64_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Path64_BINARY_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Path64_BINARY_DIR}/lib)

FILE(MAKE_DIRECTORY ${Path64_BINARY_DIR}/lib)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink .
                        ${Path64_BINARY_DIR}/lib/${PSC_FULL_VERSION})

SET(PATH64_LIB_PATH lib/${PSC_FULL_VERSION})

# Tell GCC where to install relative to the CMAKE_INSTALL_PREFIX.
SET(PATH64_GCC_INSTALL_PATH ${PATH64_LIB_PATH})

# If readding support for SGI and other platforms, where the
# dynamic linker allows overwritting weak references in modules
# opened via dlopen or similar means:
# -DUSE_WEAK_REFERENCES
ADD_DEFINITIONS(-D_SGI_SOURCE
                -D__GNU_BUG_WORKAROUND
#                -DTARG_X8664
                -DKEY
                -DFE_GNU_4_2_0
                -D_LONGLONG
                -D_MIPSEL
                -DTARG_LINUX
               )

# Define _GNU_SOURCE macro to avoid linking with new versions of glibc on linux
IF("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    ADD_DEFINITIONS(-D_GNU_SOURCE)
ENDIF()

#IF (${BUILD_TARGET_DIR} MATCHES "arm")
#ADD_DEFINITIONS( -DTARG_ST
#                -DTARG_ARM
#                -DARM
#                -DMUMBLE_ARM_BSP 
#                -DSUPPORTS_SELECT
#                -DBE_EXPORTED=
#                -DTARGINFO_EXPORTED=
#               )

IF(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
    # --export-dynamic is not supported on osol
    SET(EXPORT_DYNAMIC_SUPPORTED "no")
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # --export-dynamic is not supported on Windows
    SET(EXPORT_DYNAMIC_SUPPORTED "no")
ELSE()
    SET(EXPORT_DYNAMIC_SUPPORTED "yes")
ENDIF()

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    SET(SHELL_EXT cmd)
    SET(SYMLINK_COMMAND ${CMAKE_COMMAND} -E copy_if_different) 
    SET(PATH64_AUXILARY_LD_FLAGS '"-FIXME"')
    # ADD_DEFINITIONS(-DPSC_FULL_VERSION="${PSC_FULL_VERSION}")
ELSE()
    SET(SHELL_EXT sh)
    SET(SYMLINK_COMMAND ${CMAKE_COMMAND} -E create_symlink)
ENDIF()
# build runtime search paths
include(${Path64_SOURCE_DIR}/cmake/Modules/GCCRuntimeDirs.cmake)

IF (PATH64_DEFAULT_ABI)
    ADD_DEFINITIONS(-DPATH64_DEFAULT_ABI=${PATH64_DEFAULT_ABI})
ENDIF()

INCLUDE(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(HAVE_BIG_ENDIAN)
if (HAVE_BIG_ENDIAN)
    ADD_DEFINITIONS(-DHOST_IS_BIG_ENDIAN=1 -DHOST_IS_LITTLE_ENDIAN=0)
else ()
    ADD_DEFINITIONS(-DHOST_IS_BIG_ENDIAN=0 -DHOST_IS_LITTLE_ENDIAN=1)
endif()

# Fake targets for compiler stage
ADD_CUSTOM_TARGET(compiler-stage)               # compiler core
ADD_CUSTOM_TARGET(compiler-stage-C)             # C compiler
ADD_CUSTOM_TARGET(compiler-stage-CXX)           # CXX compiler
ADD_CUSTOM_TARGET(compiler-stage-Fortran)       # Fortran compiler
ADD_CUSTOM_TARGET(compiler-stage-libs-C)        # C compiler with libs
ADD_CUSTOM_TARGET(compiler-stage-libs-CXX)      # CXX compiler with libs
ADD_CUSTOM_TARGET(compiler-stage-libs-Fortran)  # Fortran compiler with libs
ADD_DEPENDENCIES(compiler-stage-C compiler-stage)
ADD_DEPENDENCIES(compiler-stage-CXX compiler-stage-C)
ADD_DEPENDENCIES(compiler-stage-Fortran compiler-stage)
ADD_DEPENDENCIES(compiler-stage-libs-C compiler-stage-C)
ADD_DEPENDENCIES(compiler-stage-libs-CXX compiler-stage-CXX compiler-stage-libs-C)
ADD_DEPENDENCIES(compiler-stage-libs-Fortran compiler-stage-Fortran)

# Build GCC and pathscale.
IF(PATH64_ENABLE_GNU_FRONTEND)
    ADD_SUBDIRECTORY(GCC)
ENDIF()
ADD_SUBDIRECTORY(src)

ADD_SUBDIRECTORY(bootstrap)
ADD_SUBDIRECTORY(gcc_incl)

