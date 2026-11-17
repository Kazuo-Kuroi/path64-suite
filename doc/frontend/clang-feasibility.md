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
  psclang's WHIRL generation the way they are to wgen (done in phase 2,
  section 9).
* C++ compiles in upstream but hasn't been exercised here. One thing to
  fix first: upstream marks inline/template functions local and relies
  on an "ODR" symbol flag Path64 doesn't have.

**Next:** phase 2 is the rest of C (atomics, VLA scopes, options, debug
info) and a larger C corpus, then the IRIX `TargetInfo`. Phase 3 is C++.

## 9. Phase 2 status: options and symbols (November 2026)

The first part of phase 2 compared psclang with the GCC front end across
51 option sets (`-O0` to `-O3` and `-Os`, `-g` levels, PIC/PIE,
visibility, common, fast-math and errno, inline semantics, `-std`,
warnings, sections and others). For each set it compared the objects'
symbols (binding, type, visibility, section), section lists and debug
information. Then it recompiled the runtime libraries' 236 C files
(libmpath, libmv, libpscrt, libunwind, compiler-rt) with `-fpsclang` and
compared each object with the one the GCC front end built.

**Options:**

* *Path64 option groups.* The driver gives every front end `-OPT:`
  groups (`-ffast-math`, `-fstrict-aliasing`, `-Os` and others
  expand to them), and psclang failed on them. psclang's driver now
  passes them to its cc1 with `-Xclang`. cc1 removes them before Clang
  parses its arguments and gives them to the WHIRL generator, as wgen
  does.
* *Levels.* `Opt_Level` and `Debug_Level` come from Clang's own parse of
  `-O` and `-g`, so `Configure()` sees the real level. They used to be
  fixed at the defaults.
* *Options only cc1 used to get.* The `OPTIONS` table sends some flags
  only to cc1/cc1plus ("cfe"). The ones Clang reads the same way now go
  to psclang as well, in command-line order: `-w`, `-pedantic`,
  `-fno-common`, `-fno-ident`, exceptions, RTTI,
  elide-constructors, cxa-atexit, `-Wabi`. `-fgnu89-inline`, which the
  table passed to no front end, is added for psclang.
* *Inliner.* psclang now always gets the inliner phase, unless the
  command line chooses. wgen asks for it with exit status
  `RC_NEED_INLINER` whenever a file has an inline function. psclang
  can't do that: Clang's driver runs cc1 as a child and treats any other
  status as a failure. Without the inliner, inline functions were never
  inlined at `-O2`, and their `static` bodies were left in the object.

**Symbols:**

* tentative definitions are common symbols, following Clang's
  `isVarDeclStrongDefinition` and `-fno-common`;
* `weak` and visibility on variables, `-fvisibility` on definitions
  only (declarations take only explicit visibility, as in Clang's
  CodeGen);
* `const` variables with a static initializer go in `.rodata`, and
  all-zero initializers go in `.bss`;
* an `inline` function with external linkage is no longer INTERNAL in
  C (that rule is for C++).
* *Inline-only bodies.* The C99 `inline` and gnu89 `extern inline`
  bodies are emitted above `-O0` and marked must-inline, as wgen does.
  The exception is a body that calls the builtin of its own name
  (glibc's string and fortify wrappers), the same check as Clang's
  `isTriviallyRecursive`.
* *Aliases.* They were emitted only when used, and always weak. They
  are now emitted always: a weak alias names its strong symbol, and any
  other alias shares the target's storage (wgen's `WGEN_Assemble_Alias`).
  A target with several aliases now gets all of them.

**Builtins:** `sqrt`, `sqrtf` and `sqrtl` (not `sqrtl` on MIPS) use the
sqrt instruction, as wgen does. With `-fno-math-errno` that is all;
otherwise the library function is called only when the result is NaN.
`__builtin_eh_return_data_regno` uses the back end's intrinsic.

**Results on x86-64:**

* Option sets: with `-std=gnu89` or `-fgnu89-inline`, the symbols match
  the GCC front end's except for the expected differences below.
* Runtime libraries: 234 of 236 C files compile. 145 objects have the
  same symbols as the GCC front end's, and the other 89 differ only in
  local symbols: Clang drops unused `static` functions and variables
  from headers, and names function-local statics differently. 40 libm
  functions from the psclang-built libmpath and libmv give bit-identical
  results to the GCC-built ones over 18,000 to 40,000 inputs each.
* No regressions: C probes 18/20 (the same two failures); the ABI
  cross-tests with GCC 14 all match (3 sets, both levels, all four
  pairings); compiler-rt unit tests 98/139 at `-O0`, `-O2` and `-O3`,
  with the same failures as the GCC front end.

**Expected differences from the GCC front end:**

* *Inline semantics.* Clang defaults to C99 (gnu11), GCC 4.2 to gnu89:
  a plain `inline` definition is not emitted, and `extern inline` is.
  `-std=gnu89` or `-fgnu89-inline` give GCC 4.2's behaviour. Modern GCC
  defaults to the C99 rules too.
* *Unused statics.* Unused `static` functions and variables are not
  emitted.
* *Fast-math NaN tests.* GCC's `-ffast-math` also folds `x != x` to
  false before WHIRL.

**Remaining gaps** (in addition to section 8's; section 10 closes the
first, second and fourth):

* *x86 target builtins.* `__builtin_ia32_*` (Clang's SSE intrinsic
  headers) and `__builtin_nontemporal_*` are not converted. That
  includes libpscrt's `memcpy_em64t.c` and `memcpy_opteron.c`, whose
  `em64tintrin.h` uses GCC 4.2's builtin names.
* *Debug information.* Only subprogram entries: no variables,
  parameters or types.
* *`-mregparm`.* wgen turns it into a per-function register parameter
  count; psclang ignores it (`-m32` only).
* *Other math builtins.* `acos`, `asin`, `log`, `log10`, `sin`, `cos`
  and the like stay library calls; wgen maps some to intrinsics.

## 10. Phase 2 status: debug information, intrinsics, VLA scopes (November 2026)

**Debug information** (`-g2` and up, as wgen):
* *Variables and parameters.* Globals, block-scope externs, locals and
  static locals (under their function), and parameters, with `...` for
  variadic functions.
* *Types.* Every C type form: qualifiers, pointers, arrays with bounds,
  function types, structs and unions with bit-fields, enums, typedefs.
  Only types the program uses are described.

On a test of C's type forms, each variable, parameter and function has
the same described type as with the GCC front end, except where psclang
is more exact:
* typedefs are kept;
* bit-fields carry their declared type;
* `signed char` has its own encoding;
* `int m[3][4]` is one array with two dimensions.

Line tables match. readelf finds no problems in the DWARF of the 235
library objects built with `-g`.

**x86 intrinsics.** psclang now uses Clang's own intrinsic headers
(PathScale's are written for GCC 4.2's builtins) and gets the SSE flags
in Clang's spelling.
* *Builtins.* `__builtin_ia32_*` map by name to the intrinsic wgen gives
  each: 379 builtins in `clang2whirl/c2w_x86_builtins.def`, generated
  from GCC 4.2's names. wgen's special cases are ported, and the
  builtins only Clang has are added.
* *Vector operations* in Clang's headers:
  * `__builtin_shufflevector` (an instruction where one fits);
  * `__builtin_convertvector`;
  * `v[i]`;
  * vector comparisons;
  * integer vector multiply and divide.

A test of 174 SSE through SSE4.1 intrinsics gives results identical to
GCC 14 at `-O0` and `-O2`. The GCC front end gets 16 of the 114 SSE/SSE2
results wrong, and its SSSE3 header does not compile.

The test also found x86 back-end bugs, which affected both front ends:
* `packssdw128` and `packuswb128` emitted `packsswb128`;
* wrong load-folding entries for `pabs*`, `pmaddubsw128`, `phaddd`,
  `phsubd` and `blendvp*`;
* folding a load into an instruction with an immediate operand dropped
  the immediate;
* `-msse4_1` reached the back end as `sse4_1=off`.

**Stack space of VLAs.** A VLA's space is returned when its scope ends,
and also on `break`, `continue` and `goto` out of it. On the VLA test the
stack use matches GCC 14's. The GCC front end gives the space back only
at the end of a scope, so a `continue` or `goto` in a loop keeps all of
it.

**Math functions.** `sin`, `cos`, `atan` and `exp` become intrinsics, as
with wgen, and so do `acos`, `asin`, `log`, `log10` and `pow` without
errno. `__builtin_powi` and `floor` are handled too. The back end then
makes the same calls for both front ends (`sin` and `cos` become one
`sincos`).

**Also:** clang2whirl now sets up the error tables, so an unimplemented
feature is reported instead of crashing psclang. All 236 runtime-library
C files compile with `-fpsclang`.

**Results on x86-64.** Unchanged:
* C probes 18/20;
* ABI cross-tests 18/18;
* compiler-rt 98/139;
* the libm bit-identity test, 40 functions.

The option comparison is unchanged as well.

**Remaining gaps:**
* *Label differences* (`&&a - &&b`) in static initializers are reported
  as not implemented. WHIRL has no initializer for a label minus a label.
  The GCC front end accepts them but the program crashes; label address
  tables work in both.
* *`-mregparm`* (`-m32` only) is ignored.
* *C11 atomics* and 1- and 2-byte `__sync_*` (section 8). GCC 4.2 has
  neither.
* *The two failing C probes:* `_Alignas(32)` on a local (the back end,
  both front ends) and `<stdatomic.h>`.
* *Math intrinsics* map only for `float` and `double`, not `long
  double`.

**Bugs found in the GCC front end**, since fixed (both front ends stay
in use):
* *Vector casts* converted element values, so SSE compare masks became
  -1.0.
* *Scalar `ss`/`sd` intrinsics, `movehl`/`movelh`, `pmaddwd`* were wrong;
  the cause was mostly the shared x86 back end.
* *Shift units:* `_mm_slli_si128`/`_mm_srli_si128` and `palignr` passed
  bits where the instructions take bytes.
* *SSSE3 header:* `<tmmintrin.h>` didn't compile, and `psign` and
  `stream_load` were missing.
* *VLA space* was kept on `continue` and `goto`.
* *Label differences* in static initializers were miscompiled; they are
  now reported as not implemented.

Both front ends now give GCC 14's results on 243 intrinsic results:
SSE/SSE2 (114), scalar `ss`/`sd` (32), MMX (37) and SSE3 through SSE4.1
(60), at `-O0` and `-O2`.

## 11. Phase 2 status: C++, C11 and C++11/14 (November 2026)

psclang already used the suite's own C++ library headers (Apache
stdcxx). Standard-library C++ failed for other reasons, now fixed.

**C++ fixes in clang2whirl:**
* *Exception tables.* `catch` types and the per-function type table were
  integers, not symbols; the WHIRL verifier rejected every `try` with a
  typed `catch`.
* *References.*
  * Binding a reference to `*p`, `s.f`, `p->f` or `a[i]` stored the
    wrong address.
  * Storing through a reference to a member of a member was mistyped.
  * `return *p;` from a function returning a reference dropped a load,
    so `std::vector` and `std::string` iterators read garbage.
  * `const T&` bound to a temporary did not get its own temporary.
  * A namespace-scope reference to a temporary was not set, and the
    temporary's destructor did not run at exit.
* *`delete`.*
  * Through a base pointer, `delete` called the static type's
    destructor instead of the virtual one.
  * The deleting destructor (D0) neither destroyed virtual bases nor
    called `operator delete`.
  * `delete` of a null pointer to a class with a destructor crashed.
* *Arrays.*
  * `new T[n]` built element 0 n times.
  * The array cookie followed the constructor instead of the Itanium
    rule (destructor, or a sized `operator delete[]`), so `delete[]`
    freed the wrong address.
  * Sized `operator delete` and `delete[]` got no size.
* *`dynamic_cast<T&>`* did not throw `std::bad_cast`.
* *Range-based `for`* lost its declarations (loops ran forever or
  crashed).
* *Global initializers.* `__cxx_global_var_init` had no `RETURN`. With
  no exit that uses memory, WOPT's dead-store elimination dropped the
  last dynamic initialization that nothing after it in the function
  reads; such a global stayed 0 at `-O1` and above. Initializations are
  also grouped in one IF per `init_priority`, as wgen does.
* *Constructor/destructor attributes* (C and C++) are placed in
  `.ctors`/`.dtors`, as wgen does.
* *`int` to `unsigned long`.* This is now an `I8I4CVT`, as wgen gives
  the back end. WOPT turned the `U8I4CVT` of an expression into a zero
  extension, which broke stdcxx's string reference count at `-O2`.

**C11 and C++11/14.** The driver accepts:
* `-std=c11`, `gnu11`, `c1x`, `gnu1x`, `iso9899:2011`;
* `-std=c++11`, `gnu++11`, `c++0x`, `gnu++0x`, `c++14`, `gnu++14`,
  `c++1y`, `gnu++1y`, `c++1z`, `gnu++1z`.

They are passed to psclang. With the GCC front end the driver stops with
"needs the Clang front end (-fpsclang)".

psclang also gained:
* `_Atomic` objects: loads and stores, and `+=`, `-=`, `&=`, `|=`,
  `^=`, `++`, `--` on 4- and 8-byte integers as one locked instruction;
* `__builtin_trap` (a call to `abort`, as wgen makes it) and
  `__builtin_unreachable`.

**Programs only Clang 4 builds.** Each gives host GCC 14's output at
`-O0` and `-O2`:
* *C11:* `_Generic`, `_Static_assert`, anonymous structs and unions,
  `_Alignas`, `_Thread_local`, `_Noreturn`, `u8` strings, and `_Atomic`
  counters under four threads.
* *C++11:*
  * move semantics with rvalue references;
  * lambdas with captures, `auto`/`decltype`, `constexpr`;
  * variadic templates and perfect forwarding;
  * `enum class`, `final`/`override`, delegating constructors,
    `= default`/`= delete`;
  * alias templates, `nullptr`, `thread_local`, `char16_t`/`char32_t`;
  * range-based `for` over arrays, `std::vector`, `std::string` and user
    iterators.
* *C++14:* generic lambdas, init-captures, return type deduction,
  `decltype(auto)`, relaxed `constexpr`, variable templates, binary
  literals and digit separators.

**Results on x86-64:**
* C probes: psclang 18/20, GCC front end 15/20 (unchanged; one psclang
  run also passed `c11_align`, which depends on the stack's alignment).
* compiler-rt 98/139 and ABI cross-tests 18/18 with both front ends
  (unchanged).
* C++ probes: psclang 20/20, GCC front end 11/20 (the GCC front end
  rejects the C++11 probes).

**Remaining gaps:**
* *C++11 library.* stdcxx 4.x has no C++11 parts, not even
  `<initializer_list>`, so `std::move`, `std::unique_ptr`, initializer
  lists for containers and the like are missing. A C++11 library (for
  example LLVM's libc++ 4.0, which matches Clang 4) would be needed.
* *`<stdatomic.h>`.* The `__c11_atomic_*` and `__atomic_*` builtins are
  not implemented; only `_Atomic` objects and their operators are.
* *Back end.* WOPT's handling of `U8I4CVT` (above) is worked around in
  psclang, not fixed.

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
