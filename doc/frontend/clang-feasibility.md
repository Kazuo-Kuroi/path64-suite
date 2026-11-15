# Feasibility: a Clang 3.x/4.0 front end for Path64

Question: can the GCC 4.2.1 front end (cc142/cc1plus42 + gspin + wgen,
described in [gcc-to-whirl.md](gcc-to-whirl.md)) be replaced by one built
on Clang 3.x/4.0, which writes WHIRL directly?

**Short answer: yes. The proven design is the one PathScale and later
Futurewei used: walk Clang's analyzed AST and write Very High WHIRL,
without going through LLVM IR.**
* Clang supplies everything GCC currently supplies: parsing, semantic
  analysis, templates, mangling and class layout.
* The new code takes wgen's place. It also takes over the C++ emission
  that Clang keeps in its LLVM-specific CodeGen library: constructor and
  destructor variants, vtables, RTTI, thunks, cleanups and exception
  regions.
* C alone is a project of a few months. Full C++ is much larger: a
  PathScale engineer put it at "many many man years" in 2012, but a
  WHIRL-emitting design has been done twice since.

## 1. Prior art

**PathScale EKOPath 5 (December 2013).** The release announcement lists a
"new CLANG based parser and semantic analysis" and partial C++11 support:
lambdas, rvalue references, variadic templates, `auto` and `nullptr`
([PRWeb](https://www.prweb.com/releases/pathscale_ekopath_5_compiler_suite_is_now_available/prweb11390430.htm)).
This tree still has the driver side of that work:
* the `P_psclang` and `P_psclang_cpp` phases, which run
  `psclang -x c|c++ … -c -whirl <input> -o <file>.B`
  (`compiler/compiler/src/driver/phases.c`);
* the `-fpsclang`/`-fno-psclang` options;
* `psclang/CMakeLists.txt`, which builds Clang from `PATH64_PSCLANG_SOURCE`.

The Clang-side sources were not released with Path64. `github.com/pathscale/clang`,
`pathscale/psclang` and `path64/clang` don't exist (404), and the
`path64` GitHub organization has only `debugger` and `assembler`.

**cfe-dev, September 2012, "different IR from Clang"**
([thread](https://lists.llvm.org/pipermail/cfe-dev/2012-September/024361.html)).
Someone asked how to make Clang produce Open64 IR. C. Bergström
(pathscale.com) answered:
* "I've looked at this in depth and hooking into clang *before* LLVM IR
  is the way to go."
* "VH WHIRL contains a crapload more source information than the thing
  people primarily call 'LLVM IR'."
* "compiling serious codes or full C++ is many many 'man years' of work."

Sean Silva suggested converting LLVM IR to Mid WHIRL as the easier route,
but agreed that "it makes sense to lower Clang AST's if the target is
'Very High' or 'High' WHIRL". Yulei Sui pointed out that LNO, IPA and
preopt run before Mid WHIRL and need the high-level structure.

**clang2whirl (Futurewei / XC5, 2019–2020)**,
[gitee.com/open64ark/open64](https://gitee.com/open64ark/open64) (`osprey/clang2whirl`):
* From its README: "This front end does Clang AST to WHIRL, NOT LLVM IR to
  WHIRL. The implementation is we keep the interface for Clang CodeGen
  (which converts Clang AST to LLVM IR) unchanged but reimplement it to do
  Clang AST to WHIRL." `whirl_gen.cxx` implements the CodeGen interface,
  and a `WhirlGenConsumer` does the conversion.
* It supports Clang 7.0.1 and 11.0.0, builds into the upstream
  open64-compiler/open64 tree and targets x86-64 only. Every source file
  says "version 2 of the GNU General Public License" (copyright XC5
  Limited, 2019–2020). The Mulan PSL v2 header is only on the
  repository's top-level README.
* It has 61 files and 27,000 lines, including `c2w_cxxabi`, `c2w_expr`, `c2w_stmt`,
  `c2w_type`, `c2w_decl`, `c2w_init`, `c2w_lambda` and `c2w_target`. The
  repository has 21 commits and little activity.
* Its WHIRL is upstream Open64's, which has drifted from Path64's
  (PathScale's fork). Code taken from it would need adapting.

## 2. Why Clang 3.x/4.0

* **Language.** Clang 3.3 and later implement all of C++11, and Clang 3.4
  and later all of C++14 ([C++ status](https://clang.llvm.org/cxx_status.html)).
  Clang 4 has most of C++17 under `-std=c++1z` (structured bindings,
  guaranteed copy elision, `auto` template parameters, …); C++17 was
  complete in Clang 5. For OpenMP:
  * Clang 3.8: "OpenMP 3.1 is fully supported";
  * Clang 3.9: all non-offloading OpenMP 4.0/4.5 features.

  C11 support in 3.x wasn't checked in this pass (Clang's C status page
  covers it).
* **License.** Clang 3.x/4.0 is under the University of Illinois/NCSA
  license: permissive, BSD-like, and compatible with GPL v2. A
  Clang-based front end can link Path64's GPL v2 code and stay GPL v2.
  It avoids GCC 4.2.2+'s GPL v3. LLVM 9 and later moved to Apache 2.0
  with the LLVM exception, which also addresses GPL v2 compatibility, but
  3.x/4.0 avoids the question altogether.
* **Stability.** Clang's AST and CodeGen interfaces change a lot between
  releases. Fixing one version (4.0.1, the last of the 4.0 series, or
  3.9.1) gives a stable target.

## 3. What Clang provides, and what we would write

wgen's job (section 4 of [gcc-to-whirl.md](gcc-to-whirl.md)) mapped onto
Clang:

| Needed for WHIRL | GCC front end today | Clang 3.x/4.0 |
|---|---|---|
| Parsing, name lookup, overloads, templates | GCC | Parse + Sema |
| Explicit implicit conversions | `NOP_EXPR`/`CONVERT_EXPR` | `ImplicitCastExpr` with a `CastKind` |
| Constant folding | `fold` | `Expr::Evaluate` → `APValue` (exact, including floating point via `APFloat`) |
| Class layout | GCC's field offsets | `ASTContext::getASTRecordLayout` (libAST) |
| Name mangling | `DECL_ASSEMBLER_NAME` | `MangleContext` / Itanium mangler (libAST) |
| Vtable layout | GCC | `ItaniumVTableContext` (libAST) |
| Temporaries and cleanups | `TARGET_EXPR`, `CLEANUP_STMT` → wgen | `ExprWithCleanups`, `CXXBindTemporaryExpr`, `MaterializeTemporaryExpr` in the AST; emitting them is **ours** |
| Ctor/dtor variants, thunks, vtables, VTT, RTTI data, guard variables, static init | GCC creates the declarations and data | Clang's CodeGen does this, writing LLVM IR: **ours** |
| Exception regions and tables | `TRY_BLOCK`/`EH_SPEC_BLOCK` → wgen | `CXXTryStmt`, `CXXThrowExpr` in the AST; regions and `misc_info` tables are **ours** |
| Virtual calls, member pointers | `OBJ_TYPE_REF`, `PTRMEM_CST` → wgen | AST nodes; the Itanium lowering is in CodeGen: **ours** |
| Argument passing / calling conventions | not in wgen: `be` (`targ_sim.cxx`) | Clang's `ABIInfo` (CodeGen) is **not needed**: the Path64 back end already does this |
| Debug information | `wgen_dst.cxx` → DST | **ours**, from Clang decls and types |
| OpenMP | `OMP_*` trees → `wgen_omp_*` | `OMP*Directive` AST nodes; lowering to WHIRL pragmas is **ours** (port of wgen's) |

The record layout, mangling and vtable-layout rows are confirmed in
Clang 4.0.1's headers (`include/clang/AST/RecordLayout.h`, `Mangle.h`,
`VTableBuilder.h`): they are part of libAST, not CodeGen. See section 7.

The **ours** cells are the work. Expect something of wgen's size
(33,700 lines, which can serve as a model for the WHIRL side), plus the
C++ emission that GCC does today and Clang leaves to CodeGen.

Clang's own `CGClass.cpp`, `CGVTables.cpp`, `CGRTTI.cpp`,
`CGException.cpp`, `CGCleanup.cpp` and `ItaniumCXXABI.cpp` are the
reference for the C++ parts. clang2whirl's `c2w_cxxabi` is a second
reference.

## 4. Options

**A. Clang AST → Very High WHIRL** (recommended). This is what EKOPath 5
and clang2whirl do.
* An `ASTConsumer` (`HandleTranslationUnit`, or `HandleTopLevelDecl` for C)
  walks declarations and function bodies and builds WHIRL with Path64's
  `common/com` libraries (symtab, `wn`, `ir_bwrite`).
* It keeps structured loops for LNO.
* It links Clang's Basic, Lex, Parse, Sema, AST, Edit and Frontend
  libraries plus LLVM's Support (and Option/MC as the frontend needs
  them). It doesn't need CodeGen or any LLVM target back end.

**B. LLVM IR → WHIRL.** Rejected. It loses loop and scope structure, and
the 2012 discussion and Path64's design both go against it. It could only
reach Mid WHIRL, after the point where LNO and IPA want the program.

**C. Clang AST → gspin → existing wgen.** This would reuse wgen, but gspin
is a copy of GCC's GENERIC trees. Clang's AST would have to be turned into
GCC's tree semantics (`TARGET_EXPR`, `CLEANUP_POINT_EXPR`, `BASELINK`,
GCC's flag bits). Most of the C++ data would have to be created to look
like GCC's, which is the same work as option A plus a translation layer.
It also keeps libspin, which is labeled GPL v3. Not recommended. wgen's
logic for EH regions, cleanups, OpenMP and DST is still worth porting.

**D. Start from clang2whirl.** This means moving it to Path64's WHIRL and
either back to Clang 3.9/4.0 or accepting Clang 7. It is worth reading in
full before writing anything, and reusing if the license is confirmed
GPL-2.0 and the WHIRL differences are small.

## 5. Path64- and IRIX-specific points

* **The driver** already speaks the psclang protocol (`-whirl`, `.B`
  output, `RC_NEED_INLINER`). A new front end can take over the
  `P_psclang` phase instead of adding a new one.
* **IRIX target.** Clang 3.x/4.0 has no IRIX target. We would add a
  `TargetInfo` for `mips-sgi-irix6.5` n32. It covers type sizes, the
  predefined macros the driver passes to cc142 today (`__sgi`,
  `_MIPS_SIM`, `__c99`, …) and the header workarounds (`__generic`,
  `__INTADDR__`). `long double` should use `APFloat`'s
  `PPCDoubleDouble`, the same hi + lo double-double format as SGI's.
* **Exact `long double` constants.** `APFloat` gives exact values, which
  solves the precision limit of the current cross compiler. Today
  constants pass through the x86 host's 64-bit-mantissa `long double`.
* **C++11 needs a C++11 library.** stdcxx is a C++03 library. C++11 code
  needs libc++ from the same LLVM release, which is NCSA/MIT dual-licensed
  and works with libcxxrt, as on FreeBSD. That's a separate port,
  especially for IRIX.
* **`__int128`.** Clang accepts it, but WHIRL still can't compute with it.
  The new front end would reject it the same way wgen does now.
* **Building Clang 4.0 on a 2026 host.** It needs CMake ≥ 3.4.3 and a
  C++11 compiler. Expect a small number of fixes for modern GCC/Clang
  (headers, stricter diagnostics), as this tree needed.

## 6. Plan

Each phase ends with a check against what wgen produces for the same
input. `ir_b2a` dumps of the `.B` files, the language probes, the GCC
ABI cross-tests and the IRIX test bundles make a ready-made differential
test.

0. **Build and target.**
   * Build Clang 4.0.1 (or 3.9.1) libraries alongside the tree.
   * Add the IRIX `TargetInfo`.
   * Check that `-fsyntax-only` accepts the IRIX sysroot headers and glibc's.
   * Confirm the libAST pieces in section 3.
   * Read clang2whirl in full.
1. **C subset → WHIRL.** Functions, scalars, structs, control flow and
   calls, behind `-fpsclang`. Compare WHIRL with wgen's.
2. **Full C** (C99/C11), initializers, bit-fields, varargs, inline `asm`
   and DST. Then switch C to the new front end by default on x86-64 and
   IRIX once the probes and ABI tests match.
3. **C++ core.**
   * Classes and inheritance, constructor/destructor variants, vtables
     and RTTI, thunks.
   * Temporaries and cleanups, exceptions, static initialization.
   * Validate with libcxxrt, the C++ probes and the throw/catch tests on
     IRIX.
4. **C++11/14.** Lambdas, rvalue references, `constexpr`; most of this is
   resolved by Sema. Port libc++ 4.0.
5. **OpenMP, IPA/inline, `-O3`.** Then retire cc142/cc1plus42, libspin
   and wgen, which also removes the GPL v3-labeled libspin from the
   build.

## 7. Phase 0 results (November 2026)

Sources, all kept outside the repository under `~/devel/src`:
* LLVM and Clang 4.0.1 release tarballs, with good signatures from the
  4.0.1 release key (Tom Stellard, `8F0871F202119294`);
* clang2whirl at open64ark commit `1c83f88` (November 2020).

**Clang 4.0.1 builds on this host** (GCC 14.3, CMake 3.31, Ninja) with
targets X86 and Mips. It needs one patch:
`~/devel/src/llvm-4.0.1/patches/0001-CGOpenMPRuntime-gcc14-lambda-capture.patch`.
GCC 14 rejects three lambdas in `CGOpenMPRuntime.cpp` whose parameter
`CGF` has the same name as a capture; the capture was dead, so the patch
drops it.

**Headers.**
* x86-64 glibc: all C11 standard headers plus common POSIX ones parse
  with no errors in C99 and C11 (`_Static_assert`, `_Generic`,
  `<stdatomic.h>`, `<threads.h>`). GCC 14's libstdc++ headers parse in
  C++11 and C++14 mode.
* IRIX 6.5.30 sysroot: Clang 4.0 has no IRIX target, so this used
  `-target mips64-unknown-unknown -mabi=n32 -march=mips4 -funsigned-char`
  plus cc142's IRIX predefined macros. With those, 32 standard and POSIX
  headers, plus `<rld_interface.h>`, parse with no errors in gnu99 and
  C11. From C++98, C++11 and C++14 they parse with no errors too. That
  defines what an IRIX `TargetInfo` must do:
  * `char` unsigned;
  * `long double` as double-double (`__LDBL_MANT_DIG__` 106; the generic
    target gives IEEE quad, 113);
  * `__mips` 4 and `_MIPS_ISA` `_MIPS_ISA_MIPS4`; the generic target
    says MIPS64 even with `-march=mips4`, and IRIX's `<setjmp.h>` then
    defines no `jmp_buf`;
  * `__sgi`, `sgi`, `_SGI_SOURCE`, `_SVR4_SOURCE`, `__SYSTYPE_SVR4`,
    `_LANGUAGE_C` (or `_LANGUAGE_C_PLUS_PLUS` in C++), `_LONGLONG`,
    `_MODERN_C`, `__EXTENSIONS__`, `__DSO__`, `_COMPILER_VERSION`, and no
    Linux macros;
  * the driver's existing `__c99`, `__INTADDR__` and `__generic`
    definitions;
  * in C++: `_WCHAR_T` (IRIX headers otherwise `typedef long wchar_t`,
    which GCC 4.2 tolerated in system headers and Clang doesn't), and
    `restrict` as `__restrict`.

**clang2whirl against Clang 4.0.1 and Path64's WHIRL headers.**
* `-fsyntax-only` on its 23 source files gives 577 errors with 174
  distinct messages; one file (`c2w_target.cxx`) is already clean. This
  is a first pass: more errors will surface as these are fixed and when
  linking.
* About 210 errors are one include problem: Path64 has `WN_INSERT_Block*`
  and `WN_COPY_Tree` in `common/com/wn_util.h`, which wasn't on the
  include path.
* About 120 come from Path64's wider symbol-table indexes. Path64 has
  `ST_IDX` pointer-sized and `TY_IDX`/`STR_IDX` 64-bit; upstream Open64,
  and clang2whirl's `open64decl.h`, use 32 bits. This is a real
  difference between the two WHIRLs, but mechanical.
* Upstream-Open64 additions missing in Path64: `ST_Srcpos`, vtable/RTTI/
  ODR flags on symbols and types, `INITV` label flags, 1-byte `__sync`
  intrinsics, x87 80-bit float constant helpers and
  `X86_64_COMPLEX_X87_CLASS`. These can be added or stubbed.
* 89 errors are Clang 7 → 4.0 API differences:
  * C++17 deduced template specializations, fixed-point types and
    OpenCL atomics are absent in 4.0;
  * `StringRef` can no longer be built from `nullptr`;
  * `driver.cxx`/`cc1_main.cxx` are copies of Clang 7's driver and
    should be replaced with 4.0's.

**Assessment.** Option D (start from clang2whirl) looks cheaper than
writing from scratch. The backport is mostly mechanical at this level.
The work that remains is semantic:
* OpenMP, which clang2whirl lacks entirely;
* fuller debug information (its DST code is a sixth of wgen's);
* the IRIX target and MIPS `va_arg`;
* validation against wgen's output.

## 8. Phase 1 status: C through the Clang front end (November 2026)

clang2whirl is in `compiler/compiler/src/clang2whirl`, ported to Clang
4.0.1 and Path64's WHIRL, and builds as `psclang`. `pathcc -fpsclang`
runs it through the driver's existing psclang phases; the default is
still the GCC front end. x86-64 only so far.

Build it by adding to the cmake command:

    -DPATH64_ENABLE_CLANG_FE=ON
    -DPATH64_CLANG_SOURCE_DIR=<llvm-4.0.1.src, with clang in tools/clang>
    -DPATH64_CLANG_BUILD_DIR=<its build directory>

psclang is Clang's own driver and cc1 with the LLVM code generator
replaced: it links Clang's libraries except clangCodeGen, and
`whirl_gen.cxx` provides `clang::CodeGenAction`. The driver therefore
asks it to "emit LLVM" (`-emit-llvm`) to get a `.B` file, and passes
`-resource-dir` for Clang's builtin headers, staged in
`lib/<version>/clang/include`.

**Results on x86-64** (same tests as the GCC front end, `-O0` and `-O2`):

| Test | GCC 4.2 front end | Clang front end |
|---|---|---|
| C probes (20) | 15 pass; five C11 ones fail | 18 pass |
| compiler-rt unit tests (108, also `-O3`) | 70 pass | 70 pass, the same ones |
| ABI cross-tests with GCC 14 (3 sets, 4 caller/callee pairs) | all match | all match |

* The C probes that now pass and can't with GCC 4.2: `_Generic`,
  `_Static_assert` and `_Thread_local`.
* The two that fail:
  * `_Alignas(32)` on a local variable: the x86-64 back end doesn't
    realign the stack beyond 16 bytes. The GCC front end's
    `__attribute__((aligned(32)))` behaves the same.
  * `<stdatomic.h>`: C11 and `__atomic_*` operations are not mapped to
    Path64's intrinsics yet (compile-time failure).
* The 38 compiler-rt tests that fail under both are the 128-bit integer
  tests.

**What the port needed**, beyond the mechanical fixes of section 7:

* *Missing in upstream, written here:*
  * local variable-length arrays: allocation with `alloca` as wgen does,
    bounds saved at the declaration, `sizeof`, indexing and pointer
    arithmetic on multi-dimensional VLAs;
  * `_Alignas`/`aligned` on variables;
  * `__builtin_bswap*`, `isnormal`, `isinf_sign`, `signbit` and
    `__builtin_object_size` as WHIRL expressions (upstream used
    intrinsics Path64 lacks, or glibc-internal functions).
* *Upstream bugs fixed:*
  * constant conversions between floating types kept the source's bit
    pattern unless the operand was a literal (`double d = INFINITY;`);
  * `double _Complex` constants used the real part twice;
  * a C99 `inline` definition followed by `extern inline` was not emitted;
  * `-pedantic-errors` could not reject anything, because the driver
    passed `-Wno-all`.
* *Path64 back end:*
  * the `ILDA` operator (address of a field through a pointer), which
    upstream Open64 added and clang2whirl emits, is now lowered in VHO
    as upstream does;
  * the symbol-table verifier accepts unnamed array types.
* *Clang 4.0 differences:* `GetGVALinkageForFunction` only accepts
  definitions; the builtin ranges, deduced template specializations and
  fixed-point types of Clang 7 are compiled out.

**Known gaps** (C):

* C11 atomics; `__sync_*` on 1- and 2-byte operands (Path64 has 4- and
  8-byte intrinsics).
* A VLA declared in a loop body is released when the function returns,
  not at the end of each iteration (wgen saves and restores the stack
  pointer per scope).
* Static initializers with label differences (`&&a - &&b`).
* OpenMP; debug information is minimal (section 3).
* Several library builtins map to glibc-internal names (`__isnan`,
  `__finite`, `__fpclassify`): fine on Linux, to be replaced for IRIX.
* `-O` level, `-g` and most `-f` options are not yet passed through to
  psclang's WHIRL generation the way they are to wgen.
* C++ compiles in upstream but hasn't been exercised here. One thing to
  fix first: upstream marks inline/template functions local and relies
  on an "ODR" symbol flag Path64 doesn't have.

**Next:** phase 2 is the rest of C (atomics, VLA scopes, options, debug
info) and a larger C corpus, then the IRIX `TargetInfo`. Phase 3 is C++.

## Sources

* PathScale EKOPath 5 announcement, 5 December 2013:
  <https://www.prweb.com/releases/pathscale_ekopath_5_compiler_suite_is_now_available/prweb11390430.htm>
* cfe-dev, "different IR from Clang", September 2012:
  <https://lists.llvm.org/pipermail/cfe-dev/2012-September/024355.html>,
  <https://lists.llvm.org/pipermail/cfe-dev/2012-September/024356.html>,
  <https://lists.llvm.org/pipermail/cfe-dev/2012-September/024357.html>,
  <https://lists.llvm.org/pipermail/cfe-dev/2012-September/024361.html>
* clang2whirl: <https://gitee.com/open64ark/open64>,
  <https://gitee.com/open64ark/open64/tree/master/osprey/clang2whirl>
* Clang 4.0 documentation:
  <https://releases.llvm.org/4.0.0/tools/clang/docs/InternalsManual.html>,
  <https://releases.llvm.org/4.0.0/tools/clang/docs/IntroductionToTheClangAST.html>,
  <https://releases.llvm.org/4.0.0/tools/clang/docs/RAVFrontendAction.html>,
  <https://releases.llvm.org/4.0.0/LICENSE.TXT>
* Language and OpenMP support: <https://clang.llvm.org/cxx_status.html>,
  <https://releases.llvm.org/3.3/tools/clang/docs/ReleaseNotes.html>,
  <https://releases.llvm.org/3.8.0/tools/clang/docs/ReleaseNotes.html>,
  <https://releases.llvm.org/3.9.0/tools/clang/docs/ReleaseNotes.html>
* PathScale history: <https://en.wikipedia.org/wiki/PathScale>
