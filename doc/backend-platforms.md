# Backend and platform support

Inventory based on the Path64 suite audit of `59ed2d4d8` (2026-11-17),
updated after the platform removals described in the [README](../README).
The maintained platforms in this checkout are x86-64 Linux and the Linux-hosted IRIX 6.5
n32 cross compiler. The [README](../README) describes how to build both.
The tables below also record targets with working code generation but
limited runtime evidence, and platform code inherited from earlier ports.
The validation section records the scope of the checks.

## Target selection

- `PATH64_ENABLE_TARGETS` selects ABI variants. CMake maps these to
  `PATH64_ENABLE_ARCHES` and builds a phase set per architecture. ABI-specific
  libraries live under `lib/<arch>/<bits>`. The first target is the default.
  `PATH64_BUILD_32` instead applies `-m32` to the host build. [Target table and build helpers](../compiler/compiler/src/targets.cmake), [host build options](../compiler/compiler/CMakeLists.txt).
- `PSC_TARGET` configures the embedded GCC front end. `PSC_BINUTILS_TARGET`
  and `PSC_BINUTILS_DIR` select external assembler and linker tools.
  `PSC_TARGET_OS_<target>` sets the OS string in the generated driver table
  and defaults to lowercased `CMAKE_SYSTEM_NAME`. The table also holds
  include paths, CRT paths, and, on a Linux build host, the dynamic
  interpreter. The IRIX recipe explicitly sets each of these. [GCC configure](../compiler/compiler/GCC/config_cmake/CMakeLists.txt), [tool selection](../compiler/compiler/CMakeLists.txt), [driver table generation](../compiler/compiler/src/driver/CMakeLists.txt).
- The driver looks up a table row by ABI and derives its phase and library
  paths from that row. An ABI flag can still reach code generation when the
  build has no matching row. The driver then retains its default row for
  paths. Treat an object produced this way as evidence for code generation
  only. Architecture dispatch in this file recognizes `x8664` and `mips`.
  [Driver target lookup](../compiler/compiler/src/driver/targets.c), [phase arguments](../compiler/compiler/src/driver/phases.c).
- The default detector recognizes x86-64, explicit `x86_32`, and MIPS64
  target strings, with a Solaris x86-64 special case. For a `mips64*`
  target it enables both MIPS ABIs by default, so the IRIX recipe pins
  `mips_32`. The detector's GCC probe for `mips_64` uses `-mabi=32`.
  The older shell wrappers use `-mabi=64` for the same probe. This can
  select the wrong multilib paths in a default CMake configuration.
  [Default detection](../DetectDefaults.cmake), [shell configure](../configure), [release wrapper](../configure-release).

## Enumerated CMake targets

| Target | Backend and driver ABI | Status |
| --- | --- | --- |
| `x86_64` | `x8664`, `-m64`, `ABI_M64` | **Verified on Linux.** Both GCC/wgen and psclang emitted, linked, and ran a small C program against the staged runtime. This is the only ABI enabled in the local native audit build. |
| `x86_32` | `x8664`, `-m32`, `ABI_M32` | **Code generation verified.** Both front ends emitted little-endian i386 ELF objects in the local native build. That x86-64-only stage failed to link a 32-bit executable for lack of compatible system and staged libraries. CMake has i386 startup objects, compiler-rt and unwind sources. A full multilib build and run were outside this check. |
| `mips_32` | `mips`, `-n32`, `ABI_N32` | **Verified IRIX cross-link.** This is the 32-bit-pointer n32 ABI on a 64-bit MIPS ISA. The IRIX driver defaults to big-endian R10000/MIPS IV. A historical MIPS Linux n32 path uses the same backend with different assembler, linker and runtime assumptions. |
| `mips_64` | `mips`, `-64`, `ABI_64` | **Code generation present.** The n64 register/ABI tables and assembler/linker flags exist. The n32 IRIX stage emitted a big-endian ELF64 MIPS IV object with `-64`. IRIX n64 startup/runtime integration and execution remain open. CMake's MIPS unwind source lists cover n32. The SiCortex-era Linux n64 path also lacks current build/run evidence here. |

The CMake table lists the four names above. The CG CMake file wires two
architecture-specific source sets, and target-info generation obtains its ISA
and ABI sources from the corresponding architecture directories.
[Target enum](../compiler/compiler/src/targets.cmake),
[driver CPU and ABI enums](../compiler/compiler/src/driver/opt_actions.h),
[CG sources](../compiler/compiler/src/be/cg/CMakeLists.txt),
[target-info generation](../compiler/compiler/src/common/targ_info/CMakeLists.txt),
[startup objects](../compiler/compiler/src/csu/CMakeLists.txt),
[unwind sources](../libunwind/CMakeLists.txt).

## Backend inventory

| Area | Source and status |
| --- | --- |
| WHIRL, lowering and optimization | Shared WHIRL representation in `common/com`, common lowering in `be/com`, LNO in `be/lno`, WOPT in `be/opt`, and phase coordination in `be/be/driver.cxx`. Both active architectures use these passes and supply target configuration and ABI hooks. The common layers contain architecture conditionals and therefore participate in target behavior. [Backend driver](../compiler/compiler/src/be/be/driver.cxx), [shared config](../compiler/compiler/src/common/com/config.cxx), [lowering](../compiler/compiler/src/be/com/wn_lower.cxx). |
| Instruction selection, registers, scheduling and emission | `be/cg` implements common WHIRL-to-operations lowering, register allocation, scheduling, and emission. `be/cg/{x8664,mips}` supplies instruction expansion, branch and load/store lowering, calling conventions, register properties, scheduling hooks, prologues/epilogues and target assembly/DWARF emission. `common/targ_info/{isa,abi,proc}` generates opcodes, operands, register classes, ABI properties and processor tables. [CG source lists](../compiler/compiler/src/be/cg/CMakeLists.txt), [CG driver](../compiler/compiler/src/be/cg/cg.cxx), [target-info build](../compiler/compiler/src/common/targ_info/CMakeLists.txt). |
| x86 | One backend for both widths. The generated ABI tables call the two variants `n32` and `n64`, as does the internal `-TARG:abi` switch. The driver exposes them as `-m32` and `-m64`. Processor tables include Opteron, Barcelona, Orochi, EM64T, Core, Wolfdale and Sandy. Driver aliases and source cover MMX/x87 and several SSE variants, AVX, XOP and FMA. There is recent codegen work on MMX, SSE/SSSE3/SSE4.1 and AVX paths. The feature tables and intrinsic implementations alone establish no comprehensive ISA conformance claim. [Driver feature map](../compiler/compiler/src/driver/opt_actions.c), [x86 expansion](../compiler/compiler/src/be/cg/x8664/expand.cxx), [x86 processor table](../compiler/compiler/src/common/targ_info/proc/x8664/proc.cxx), [x86 ABI table](../compiler/compiler/src/common/targ_info/abi/x8664/abi_properties.cxx). |
| MIPS | Generated ISA subsets are MIPS IV and MIPS64. The generated processor table contains `r10000`, `mips5kf` (SiCortex Ice9) and `twc9a`. Driver enums also carry MIPS I–VI and older SGI IP/machine IDs. Those names carry no separate validation of each ISA, CPU or board. The IRIX path passes endianness explicitly to GCC and WHIRL, applies SGI double-double `long double` handling, and uses big-endian IRIX linker emulations. The ordinary MIPS link branch uses little-endian Linux-style emulations. [MIPS subsets](../compiler/compiler/src/common/targ_info/isa/mips/isa_subset.cxx), [processor table](../compiler/compiler/src/common/targ_info/proc/mips/proc.cxx), [ABI table](../compiler/compiler/src/common/targ_info/abi/mips/abi_properties.cxx), [driver phases](../compiler/compiler/src/driver/phases.c). |
| Object format, debug and unwind | The working paths are ELF with DWARF. Shared emission lives in `common/com/em_elf.cxx` and `em_dwarf.cxx`, with per-architecture hooks. PathAS is the staged x86 Yasm-based assembler. IRIX uses GNU as/ld and MIPS CFI emission, with target-specific CRT objects and unwind-table lookup through IRIX rld. Library or assembler support for another file format does not establish a compiler port. [ELF emission](../compiler/compiler/src/common/com/em_elf.cxx), [DWARF emission](../compiler/compiler/src/common/com/em_dwarf.cxx), [PathAS build](../assembler/CMakeLists.txt), [IRIX CRT](../compiler/compiler/src/csu/irix-mips/CMakeLists.txt), [IRIX unwinder](../compiler/libunwind/src/os-irix.c). |
| IPA, instrumentation and other optional phases | IPA is enabled in both inspected build caches. The optional `ipa_link` executable carries bundled BFD/ld emulations, and the driver has OS/ABI-specific argument branches. An ordinary compile or link exercises no dedicated IPA test. Fortran, OpenMP and profiling are outside the two audit configurations, although their code and CMake switches remain. Linux huge-page support is optional and the CMake source lists are x86-specific. [IPA build](../compiler/compiler/src/ipa/CMakeLists.txt), [IPA linker](../compiler/compiler/src/ipa_link/ld/CMakeLists.txt), [phase and runtime switches](../compiler/compiler/src/CMakeLists.txt), [huge-page sources](../compiler/compiler/src/libhugetlbfs/CMakeLists.txt). |
| Front ends and inherited targets | GCC 4.2-derived C/C++ via wgen is wired for x86 and MIPS. The experimental `PATH64_ENABLE_CLANG_FE` builds clang2whirl/psclang only for `x8664`. The older top-level `PATH64_ENABLE_PSCLANG` packaging hook is a separate path. Fortran has its own frontend and shares WHIRL. Itanium/IA-64, older IA-32 guards, ST100/ST200/STxP70 and `TARG_SL` branches remain in driver or shared backend code. Only the `x8664` backend supplies current i386 output. The imported GCC target triplet catalog and standalone runtime/assembler architecture lists are broader than Path64's two CG source sets. No ARM, AArch64, RISC-V, PowerPC or SPARC CG source set is wired in this checkout. [GCC frontend design](frontend/gcc-to-whirl.md), [Clang build](../compiler/compiler/src/clang2whirl/CMakeLists.txt), [top-level packaging](../CMakeLists.txt), [driver options](../compiler/compiler/src/driver/OPTIONS), [GCC target catalog](../compiler/compiler/GCC/gcc/config.gcc). |

## OS references

| OS or environment | What remains and its status |
| --- | --- |
| Linux | **Working build host and x86-64 target.** CMake detects GCC CRT/library paths and generates Linux interpreter paths and distro-specific driver variants. Source also contains a SiCortex-era MIPS Linux cross path, including a Gentoo cross directory, little-endian linker emulations and n32 Linux unwind selection. That MIPS Linux path was not rebuilt or run in this audit. [Defaults](../DetectDefaults.cmake), [driver variants](../compiler/compiler/src/driver/CMakeLists.txt), [linker phases](../compiler/compiler/src/driver/phases.c), [unwind build](../libunwind/CMakeLists.txt). |
| IRIX 6.5 | **Maintained cross target.** Use `PSC_TARGET=mips64-sgi-irix6.5`, `PSC_BINUTILS_TARGET=mips-sgi-irix6.5` and `PSC_TARGET_OS_mips_32=irix`. The link uses the IRIX sysroot's startup files and libc/libm, PathScale's n32 CRT and static support libraries, and the `/lib32/rld` interpreter. Native IRIX builds, the vendor assembler, the n64 runtime, `libstl`, and Fortran remain outside the documented working configuration. C++/exception support is partial, including the README's unwind register-restore limitation. The MIPS compare-and-swap return-value issue recorded there also remains outside this check. `IRIX64` in the archived `uname` output denotes the kernel, while the tested binaries are n32. [Port and limits](../README), [GCC target cases](../compiler/compiler/GCC/gcc/config.gcc), [driver phase handling](../compiler/compiler/src/driver/phases.c). |
| FreeBSD | **Historical build/runtime branches.** CMake has driver link flags, avoids `libdl` where appropriate, selects x86-64 unwind sources, and permits OpenMP. The driver has a FreeBSD IPA-link accommodation. The embedded GCC catalog includes FreeBSD x86. No FreeBSD build or executable was verified. [Driver flags](../compiler/compiler/src/driver/CMakeLists.txt), [unwind build](../libunwind/CMakeLists.txt), [driver phase handling](../compiler/compiler/src/driver/phases.c). |
| Solaris / OpenSolaris (`SunOS`) | **Historical x86 build/runtime branches.** CMake forces a 64-bit host build by default and has different driver linkage, export flags and runtime sources. The driver and emitter carry `__sun` branches. These are x86-oriented references and there is no SPARC phase set. No SunOS build or executable was verified. [Compiler build](../compiler/compiler/CMakeLists.txt), [driver build](../compiler/compiler/src/driver/CMakeLists.txt), [unwind build](../libunwind/CMakeLists.txt). |
| NetBSD and other old Unix references | **Isolated branches or inherited target metadata.** NetBSD appears in the driver's dynamic-linker argument handling and the embedded GCC x86/MIPS targets. GCC also lists GNU/kFreeBSD and GNU/kNetBSD triplets. Older `__sgi` and `__hpux` compatibility guards remain in shared code. None provides an additional suite configuration with end-to-end evidence here. [Driver phases](../compiler/compiler/src/driver/phases.c), [GCC target catalog](../compiler/compiler/GCC/gcc/config.gcc), [common host utilities](../compiler/compiler/src/common/util/dso.c). |

One source-audit pitfall is that compiler CMake adds `TARG_LINUX` and
`_MIPSEL` to host-side source builds, including the IRIX cross configuration.
For IRIX, follow the generated target row and the options sent to the
frontend, backend and binutils. Another is that the CMake target OS string
has no general OS support validation. Some runtime recipes still select
sources using `CMAKE_SYSTEM_NAME`, whereas n32 unwind explicitly selects
the IRIX implementation from `PSC_TARGET_OS_mips_32`. [Compiler definitions](../compiler/compiler/CMakeLists.txt),
[driver table generation](../compiler/compiler/src/driver/CMakeLists.txt),
[unwind build](../libunwind/CMakeLists.txt).

## Validation evidence

- Using the Linux `build-x86-audit` stage, compiled and linked a minimal C
  program at `-O2` with both the default GCC frontend and `-fpsclang`.
  Both executables ran and were little-endian x86-64 ELF. Both `-m32 -c`
  invocations produced little-endian i386 ELF objects. The attempted
  `-m32` link in that stage lacked compatible libraries.
- Using the `build-irix` stage, cross-linked the same program at `-O2`.
  `readelf` reported big-endian ELF32, `abi2` (n32), `mips4`,
  `/lib32/rld`, and dependencies on `libc.so.1` and `libm.so`.
  An explicit `-64 -c` produced big-endian ELF64 with MIPS IV flags.
  This audit performed no new IRIX execution.
- The workspace's `../irix-tests-round5-results.tar` contains existing
  execution logs reporting IRIX 6.5.30f on IP35. The diagnostics log
  records successful startup, floating-point, constructor, varargs and
  byte-order runs at `-O0` and `-O2`. The long-double tests ran at both
  levels and recorded a comparison result differing from MIPSpro. These
  transcripts supply narrower, later evidence than the README's earlier
  readelf/objdump-only validation note.
- No other OS or full ABI matrix rebuild was attempted in the original audit.

After the removals, the Linux x86-64 compiler and C/C++ runtime targets
rebuilt and passed C and C++ smoke programs with GCC and psclang. The IRIX
compiler and C runtime rebuilt, and C probes cross-linked as n32 MIPS IV
at `-O0`, `-O2`, and `-O3`. An n32 C++ object and an n64 C object also
built. CMake rejected the deleted ABI slots, an ARM target triple, and both
Windows and Darwin target triples and per-ABI OS settings.

## Removed platforms

Windows and Darwin/macOS were outside the backend's scope. The removal
drops their Path64 driver, backend and host-build branches. Imported
component code may still describe those platforms, including the GCC
target catalog and standalone assembler object formats.
The old ARM and RSK6 entries listed target names without a CG or
target-info source set. The removal drops their CMake entries, ARM
driver scaffolding and ARM-only bundled runtime implementations.
[Target table](../compiler/compiler/src/targets.cmake),
[suite configuration](../CMakeLists.txt).
