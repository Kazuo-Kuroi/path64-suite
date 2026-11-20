# PathScale EKOPath / Path64

This repository maintains the Path64 compiler suite, descended from the
PathScale EKOPath C, C++ and Fortran compilers. Current work focuses on
x86-64 Linux, a Clang-based C/C++ front end, C++11/14 library support, and a
Linux-hosted cross compiler for **IRIX 6.5 on MIPS**.

The tree is under active restoration and development. Platform support and
language support vary by front end, ABI and runtime; the status below describes
this checkout rather than the capabilities of historical EKOPath releases.

## History and architecture

PathScale developed EKOPath as an optimizing compiler suite for scientific
and high-performance computing, particularly AMD64 and Intel 64 Linux systems.
Its compiler lineage runs through Open64 and SGI's Pro64/MIPSpro technology.
The MIPS backend also carries work from SiCortex's MIPS64 Linux systems.
Path64 is the open-source compiler tree associated with that history.

The suite's distinguishing architecture is **WHIRL**, a shared intermediate
representation used at several levels of lowering. C, C++ and Fortran front
ends feed a common optimizer and code generator. The backend includes
interprocedural analysis (IPA), loop-nest optimization (LNO), global
optimization (WOPT), register allocation, scheduling and assembly emission.
The two wired architecture backends are `x8664` and `mips`.

The traditional C/C++ route uses a GCC 4.2-derived parser and `wgen` to produce
WHIRL. Historical EKOPath later introduced a Clang-based parser: PathScale's
[December 2013 EKOPath 5 announcement](https://www.prweb.com/releases/pathscale_ekopath_5_compiler_suite_is_now_available/prweb11390430.htm)
describes that transition and partial C++11 support. This fork's `psclang`
uses the separately developed XC5/Futurewei `clang2whirl`, adapted to
LLVM/Clang 4.0.1 and Path64. It translates Clang's analyzed AST directly into
WHIRL; optimization and machine-code generation remain in the Path64 backend.

For more detail, see [GCC to WHIRL](doc/frontend/gcc-to-whirl.md),
[the Clang front-end work](doc/frontend/clang-feasibility.md), and
[the backend inventory](doc/backend-platforms.md).

## Platforms

Linux on x86-64 is the working build host. The maintained target configurations
are x86-64 Linux and the work-in-progress IRIX n32 cross target.

| Target or environment | Current status |
| --- | --- |
| **x86-64 Linux** (`x86_64`) | Main native development and validation target. C and C++ compile, link and run through the GCC and experimental Clang front ends. |
| **IRIX 6.5, MIPS n32** (`mips_32`) | Maintained cross port from Linux. Big-endian R10000/MIPS IV C code compiles and links against an IRIX sysroot. C++ and runtime integration remain incomplete; see below. |
| **32-bit x86** (`x86_32`) | Backend emits i386 ELF objects. A complete multilib runtime/link/run configuration has not been validated in the current audit. |
| **MIPS n64** (`mips_64`) | Backend and ABI code remain; ELF64 MIPS object generation has been checked. Full IRIX n64 runtime integration and execution remain open. |
| **MIPS Linux** | Inherited SiCortex-era paths remain. No current end-to-end build/run validation. |
| **FreeBSD, Solaris/OpenSolaris, other Unix references** | Historical build, driver or runtime branches remain. They are not currently validated suite configurations. |

Windows and Darwin/macOS backend support was removed because it is outside
this fork's scope. ARM and RSK6 CMake entries and associated code were removed
because they were vestigial. IRIX remains in scope. Imported components may
still contain broader platform catalogs; those catalogs do not establish a
working Path64 target.

The [platform audit](doc/backend-platforms.md) records the CMake target matrix,
OS references, backend components and validation evidence in more detail.

### IRIX port

The working configuration targets **IRIX 6.5, n32, big-endian MIPS IV**, with
R10000 as the default processor. It uses GNU binutils for assembly/linking and
IRIX headers, startup files, libc and libm from a sysroot. Executables request
`/lib32/rld` and link the suite's support libraries statically.

C compilation and cross-linking have been checked at `-O0`, `-O2` and `-O3`.
The runtime libraries build for MIPS IV, and the unwinder uses IRIX rld's
`_RLD_DLADDR` service to locate unwind information. The backend emits MIPS
call-frame information. Archived hardware-test results recorded in the
[platform audit](doc/backend-platforms.md#validation-evidence) include startup,
floating-point, constructors, varargs and byte-order probes on an IRIX 6.5.30f
IP35 system at `-O0` and `-O2`. That is limited execution evidence; the port is
still unfinished.

Open work includes:

- C++ standard-library configuration and integration on IRIX. The GCC-based
  `pathCC` and libcxxrt build, and a throw/catch probe can link when supplied
  with a suitable `libstl`; this is not a complete C++ runtime validation.
- Restoring floating-point callee-saved registers when the unwinder resumes
  into a landing pad.
- Correcting MIPS `__sync_val_compare_and_swap`, which currently returns a
  success flag instead of the previous value. The boolean form is correct.
- Further long-double validation; existing hardware results record a
  comparison differing from MIPSpro.
- Fortran, the n64 runtime, native IRIX compiler builds, and use of the vendor
  assembler. Current assembly uses GNU directives such as `.cfi_*`.
- A MIPS/IRIX target for the new Clang front end, which currently targets x86.

## Language and runtime status

| Area | Current status |
| --- | --- |
| GCC-derived C/C++ front end | Default front end; provides the legacy C/C++ path, including C99 with `-std=c99` and C++98/03. It is not the C++11/14 development path. |
| Clang-based C/C++ (`-fpsclang`) | Experimental, built against LLVM/Clang 4.0.1 for `x8664`. C and substantial C++11/14 functionality are exercised on x86-64 Linux. Parser support alone does not imply complete lowering or library support. |
| C++ standard library (`stdcxx`, `libstl`) | C++98 library with C++11/14 additions selected by language mode. Type traits, utility, tuples, functions, smart pointers, array, vector and deque have substantial coverage. Numeric conversions, UTF-16/32 string interfaces, exception handles, nested exceptions and C++11 bitset interfaces are implemented. Rvalue stream operations, stream moves/swaps and money/time/hexadecimal manipulators are available. String buffers preserve allocator ownership, and public stream flags exclude internal radix/stdio state. Unicode conversion facets (`codecvt_utf8`, `codecvt_utf16`, `codecvt_utf8_utf16`) are implemented. System error codes, categories and exceptions, including stream error categories, are implemented. `ios_base::failure` now derives from `system_error` and carries an error code; other new standard headers remain unfinished. |
| C++ ABI runtime and unwinder | libcxxrt and libunwind supply exception handling and ABI support; libcxxrt also supplies C++14 sized deletion, synchronized new-handler access, the C++11 allocation-error class and reference-counted exception propagation. x86-64 has compiler/runtime regression coverage. IRIX has the limitations above. |
| Fortran, OpenMP, profiling, optional math and debugger components | Sources and build switches remain. These components are outside the current minimal C/C++ audit configuration; historical product support is not a current validation claim. |

The exception ABI has changed: rebuild `libstl` and all C++ clients together.
`ios_base::failure` now includes the common system-error base and stored code,
including failures thrown by C++98 stream implementations. Exception messages
use shared ownership so copies do not allocate. Objects built against the
previous exception headers/runtime are incompatible with this transition.

The algorithm reference suite currently passes 115/145 runner-selected tests.
Its remaining compile failures are 28 missing-`<random>` dependencies and two
unmarked C++17 tests. Move-only `sort`, heap operations, `partial_sort` and
`stable_partition`, `nth_element` and `stable_sort` are covered by original
tests, including packed-bit iterators, stable ordering and temporary-buffer
exception cleanup. C++11 math overloads and the C library header wrappers
have Linux x86-64 reference and integration coverage. All 157 applicable iterator
references in C++11 and 158 in C++14 pass. List has move insertion, emplacement
and a node-transferring move constructor; its remaining C++11 APIs and noexcept
move guarantee are unfinished. These are selected suites, not a
whole-library conformance result.

The saved constructor-initializer unwind failure is fixed, with expanded
checks for completed record members/bases, inner catches and temporary
arguments. Invalid array bounds select `bad_array_new_length` or a null result
for nonthrowing allocation; null allocations skip initialization. Loop/switch
body objects are destroyed on `break` and `continue`. The `<new>` allocation-error
APIs are implemented and tested. Namespaced numeric-limits constants now use
the proper ABI names, with host-compiler object interoperability checks.

See [C++11/14 design and status](doc/stdcxx-cxx11.md),
[the implementation TODO](doc/stdcxx-cxx11-TODO.md), and
[the test runner instructions](compiler/stdcxx/tests/cxx11/README).

## Building

All suite components are included in this checkout and built by the top-level
CMake project. The `initial-clone`, `update` and `install` shell scripts are
left over from the separate-repository layout; use CMake for this tree.

### Host requirements

- **CMake 3.13 or newer in the 3.x series.** The commands below use `-S`/`-B`.
  The project and bundled components still declare old CMake
  policy versions; CMake 4 is not supported by the documented configuration.
- A host **Clang or GCC**, a C++11-capable host C++ compiler, and **Ninja**
  (or GNU make).
- **m4**, needed for generated libdwarf sources, and **static glibc development
  libraries**, because the Linux driver links statically. On Fedora/RHEL,
  the relevant packages include `m4` and `glibc-static`.
- For the optional Clang front end, an **LLVM/Clang 4.0.1 source and build
  tree** containing the Clang libraries and generated headers.
- For IRIX, the sysroot and cross-binutils described below.

Use `-DCMAKE_BUILD_TYPE=Debug` with GCC or Clang. The current CMake rules reject
`Release` and `RelWithDebInfo` with a non-PathScale host C compiler. This host
build setting is separate from the `-O0` through `-O3` options accepted by the
compiler being built. Use bounded parallelism; eight jobs have worked in the
local builds, while very large parallel builds have stalled build hosts.

### x86-64 Linux

From the repository root, configure the C/C++ configuration used for current
development, with optional Fortran, OpenMP, profiling and math-library work
disabled:

```sh
cmake -S . -B build-linux -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_INSTALL_PREFIX="$HOME/devel/path64-linux" \
    -DPATH64_ENABLE_TARGETS=x86_64 \
    -DPATH64_ENABLE_FORTRAN=OFF -DPATH64_ENABLE_MATHLIBS=OFF \
    -DPATH64_ENABLE_OPENMP=OFF -DPATH64_ENABLE_PROFILING=OFF \
    -DPATH64_ENABLE_TESTING=OFF

cmake --build build-linux --target compiler-stage-libs-CXX --parallel 8
```

The staged toolchain is usable directly:

```sh
build-linux/Xcompiler/bin/pathcc -std=c99 -O2 hello.c -o hello
build-linux/Xcompiler/bin/pathCC -O2 hello.cc -o hello-cxx
```

The GCC-derived front end is the default. To install the configured suite at
`CMAKE_INSTALL_PREFIX`, build the `install` target:

```sh
cmake --build build-linux --target install --parallel 8
```

`PATH64_ENABLE_TESTING=OFF` disables the suite's legacy testing/bootstrap
integration. With the Clang front end enabled on native Linux x86-64,
`cmake --build build-linux --target stdcxx-check` runs the original C++11/14
regressions at `-O0` and `-O2`, independently of that legacy switch. Set
`PATH64_STDCXX_REFERENCE` to the approved libc++ 8.0.1 source tree to add the
reference checkpoint. Results go in `build-linux/libstdcxx/check-results`;
unexpected failures make the target fail.

### Optional Clang front end and C++11/14

Prepare an LLVM/Clang **4.0.1** build, with Clang's source under
`tools/clang` in the LLVM source tree and the static Clang/LLVM libraries in
the build tree. Add these options to the Linux configuration above (or
reconfigure the same build directory):

```sh
cmake -S . -B build-linux \
    -DPATH64_ENABLE_CLANG_FE=ON \
    -DPATH64_CLANG_SOURCE_DIR=/path/to/llvm-4.0.1.src \
    -DPATH64_CLANG_BUILD_DIR=/path/to/llvm-4.0.1-build

cmake --build build-linux --target compiler-stage-libs-CXX --parallel 8
cmake --build build-linux --target clangfe-x8664 --parallel 8
build-linux/Xcompiler/bin/pathCC -fpsclang -std=c++14 -O2 hello.cc -o hello-cxx14
```

Use `PATH64_ENABLE_CLANG_FE` for this implementation. The older
`PATH64_ENABLE_PSCLANG` packaging hook is a separate historical build path.
The suite uses Clang's parser and semantic libraries with its own WHIRL
emission; it does not use LLVM's machine-code generator.

For library regression testing, see
[the C++ test instructions](compiler/stdcxx/tests/cxx11/README).

### IRIX n32 cross compiler

First supply an IRIX 6.5 sysroot. The port has used the 6.5.30 headers and
libraries, with a layout such as:

```text
irix-sysroot/usr/include
irix-sysroot/usr/lib32/mips4
irix-sysroot/lib32/rld
```

The IRIX system files are supplied separately; they are not part of this
repository. Set paths for the following commands:

```sh
export IRIX_SYSROOT="$HOME/devel/irix-sysroot"
export IRIX_BINUTILS_PREFIX="$HOME/devel/irix-toolchain"
```

Build GNU binutils 2.23.2 with the patches in
[SGUG RSE's binutils package](https://github.com/sgidevnet/sgug-rse/tree/master/packages/binutils).
Apply the following patches in order with `patch -p1` from the binutils source
directory:

1. `binutils-2.22.52.0.4-no-config-h-check.patch`
2. `binutils2_23.sgifixes.patch`
3. `binutils.sgirelinkfix.patch`
4. `binutils.ldirixn32paths.patch`

From a separate binutils build directory, configure and install it. GNU as
uses the `mips-sgi-irix6.5` target name, even though the compiler's GCC target
below is `mips64-sgi-irix6.5`:

```sh
CFLAGS="-O2 -std=gnu11 -fcommon -Wno-error \
        -Wno-implicit-function-declaration -Wno-int-conversion \
        -Wno-incompatible-pointer-types -Wno-implicit-int" \
    /path/to/binutils-2.23.2/configure \
        --prefix="$IRIX_BINUTILS_PREFIX" \
        --target=mips-sgi-irix6.5 \
        --with-sysroot="$IRIX_SYSROOT" \
        --disable-werror --disable-nls --disable-gdb \
        --disable-sim --disable-gold
make -j8 MAKEINFO=true
make MAKEINFO=true install
```

Then configure the suite from the repository root:

```sh
cmake -S . -B build-irix -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_INSTALL_PREFIX="$HOME/devel/path64-irix" \
    -DPSC_TARGET=mips64-sgi-irix6.5 \
    -DPATH64_ENABLE_TARGETS=mips_32 \
    -DPSC_TARGET_OS_mips_32=irix \
    -DPSC_CRT_PATH_mips_32="$IRIX_SYSROOT/usr/lib32/mips4" \
    -DPSC_CRTBEGIN_PATH_mips_32="$IRIX_SYSROOT/usr/lib32/mips4" \
    -DPSC_DYNAMIC_LINKER_mips_32=/lib32/rld \
    -DPSC_INCLUDE_PATH_mips_32="$IRIX_SYSROOT/usr/include" \
    -DPSC_GCC_LIBRARY_DIRS="$IRIX_SYSROOT/usr/lib32/mips4" \
    -DPSC_BINUTILS_TARGET=mips-sgi-irix6.5 \
    -DPSC_BINUTILS_DIR="$IRIX_BINUTILS_PREFIX/bin" \
    -DPATH64_BUILD_STDCXX=OFF \
    -DPATH64_ENABLE_FORTRAN=OFF -DPATH64_ENABLE_MATHLIBS=OFF \
    -DPATH64_ENABLE_OPENMP=OFF -DPATH64_ENABLE_PROFILING=OFF \
    -DPATH64_ENABLE_TESTING=OFF \
    -DPATH64_ENABLE_PATHAS=OFF -DPATH64_ENABLE_DEFAULT_PATHAS=OFF

cmake --build build-irix --target compiler-stage-C --parallel 8
cmake --build build-irix --target compiler-stage-libs-C --parallel 8
build-irix/Xcompiler/bin/pathcc -std=c99 -O2 hello.c -o hello-irix
```

For this target, `-n32 -mips4 -march=r10000` are the defaults. Use `-S` to
inspect assembly without assembling or linking. The resulting executable is
for IRIX; run it on the target system. `PATH64_BUILD_STDCXX=OFF` is intentional
while the IRIX standard-library configuration remains unfinished.

## Source layout

| Directory | Component |
| --- | --- |
| `compiler/compiler` | Drivers, GCC-derived front ends, clang2whirl, Fortran, WHIRL, optimizers, code generators and compiler support libraries |
| `compiler/assembler` | Yasm-derived PathAS assembler, used by the x86 build |
| `compiler/compiler-rt` | Low-level arithmetic and compiler helper routines |
| `compiler/libcxxrt` | C++ ABI runtime |
| `compiler/libunwind` | Exception unwinder, including the IRIX port |
| `compiler/libdwarf-bsd` | DWARF support library |
| `compiler/stdcxx` | Apache stdcxx and the C++11/14 additions |
| `compiler/pathdb` | Historical PathDB debugger |
| Top-level component directories | CMake integration and staging rules for the bundled sources |
| `doc` | Platform audit, front-end notes, C++ library design and TODO |

## Licensing

This is a collection of components with different licenses. There is no
single license covering every file. The component notices and per-file
headers are the primary licensing records; this inventory does not change
them.

| Component | License records in this checkout |
| --- | --- |
| Compiler, optimizers, backend and bundled compiler support code | Many files carry GPL v2-or-later notices. The package-level [compiler COPYING](compiler/compiler/COPYING) states that files marked PathScale, Cray or STMicroelectronics are GPL v3 regardless of contrary headers. Read that notice alongside the individual files. |
| GCC-derived front end | GNU GPL notices, generally v2-or-later in the GCC 4.2-derived sources; PathScale-modified files also require attention to the compiler package notice. See, for example, [c-decl.c](compiler/compiler/GCC/gcc/c-decl.c). |
| XC5/Futurewei clang2whirl | Its implementation files carry GPL v2 notices, for example [c2w_expr.cxx](compiler/compiler/src/clang2whirl/c2w_expr.cxx). Clang-derived files retain their own notices. These differ from the compiler package's GPL v3 statement for the copyright holders it names. |
| LLVM/Clang 4.0.1 build dependency | Historical University of Illinois/NCSA licensing, with separate third-party notices in the supplied LLVM/Clang source distribution. This dependency is supplied outside this checkout. |
| `compiler-rt` | Dual University of Illinois/NCSA and MIT licensing: [LICENSE.TXT](compiler/compiler-rt/LICENSE.TXT). |
| `libcxxrt` | BSD 2-clause: [LICENSE](compiler/libcxxrt/LICENSE) and [COPYRIGHT](compiler/libcxxrt/COPYRIGHT). |
| `libunwind` | MIT-style permission notices in [LICENSE](compiler/libunwind/LICENSE) and [COPYING](compiler/libunwind/COPYING), plus per-file notices. |
| BSD libdwarf | BSD-style per-file notices, such as [libdwarf.c](compiler/libdwarf-bsd/libdwarf.c). Its [COPYING](compiler/libdwarf-bsd/COPYING) separately places PathScale-marked files under GPL v3. |
| PathAS / Yasm | Primarily BSD 2-clause and 3-clause, with Illinois/NCSA and MIT-licensed portions; see [the licensing inventory](compiler/assembler/Copying/COPYING) and the texts beside it. |
| Apache stdcxx | Apache License 2.0: [LICENSE.txt](compiler/stdcxx/LICENSE.txt), with attribution and inherited-source details in [NOTICE.txt](compiler/stdcxx/NOTICE.txt). |
| libc++ 8.0.1 design/test reference | Dual University of Illinois/NCSA and MIT licensing; the reference license is retained in [LICENSE-libcxx.txt](compiler/stdcxx/LICENSE-libcxx.txt). Reference sources/tests are obtained separately as described in the test instructions. |
| PathDB | Source files carry CDDL notices; see [dis.cc](compiler/pathdb/src/dis.cc). Those headers refer to `src/CDDL.LICENSE`, which is missing from this checkout. |

The existing stdcxx component is Apache-2.0-licensed. Current C++ library work
uses the approved libc++ 8.0.1 reference and original implementation/regression
work; LLVM/Clang 4.0.1 is the front-end dependency. New imports of
Apache-2.0-licensed code are outside the current development policy. That
policy does not relicense the stdcxx code already present.
