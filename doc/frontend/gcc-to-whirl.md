# The C/C++ front end: GCC 4.2 → gspin → wgen → WHIRL

This document describes how C and C++ source becomes WHIRL in Path64 today.
It is meant for anyone changing the front end, and as the baseline for
replacing it (see [clang-feasibility.md](clang-feasibility.md)).

## Overview

C and C++ compile in two processes connected by a file:

```
source.c ──cc142──▶ source.spin ──wgen42──▶ source.B ──(inline)──▶ be
            GCC 4.2.1 parser,       WHIRL generator      WHIRL optimizer and
            Sema, folding;          (Path64 code)        code generator
            writes GCC trees
            as a gspin file
```

| Piece | Where | Size | License (file headers) |
|---|---|---|---|
| GCC 4.2.1 C front end (`cc142`) and C++ front end (`cc1plus42`) | `compiler/compiler/GCC/gcc`, `GCC/gcc/cp` | GCC 4.2.1 plus 113 `#ifdef KEY` changes in 35 files | GPL v2 or later |
| gspin translation, inside GCC | `GCC/gcc/tree.c` (`gs_x_1`, about 2,000 lines from line 9,350) | | GPL v2 or later |
| libspin: the gspin node format and file I/O | `GCC/libspin` (7,000 lines) | 239 tree codes, 322 accessors | **GPL v3** |
| wgen: gspin → WHIRL | `compiler/compiler/src/wgen` (33,700 lines) | | mostly GPL v2 or later; some files v2 only, some v3 |

The driver runs the phases (`src/driver/phases.c`):

* `cc142` / `cc1plus42` get `-spinfile <file>.spin` and write the spin file
  instead of assembly.
* `wgen42 -fS,<file>.spin -fB,<file>.B` reads it and writes WHIRL.
* If wgen exits with `RC_NEED_INLINER` (C++ functions marked inline), the
  driver runs the `inline` phase on the `.B` file (`run.c`).
* `be` then optimizes and generates code.

GCC 4.2.1 is the last GCC release under GPL v2 or later. GCC 4.2.2 and
everything after it are GPL v3.

## 1. GCC side: what gets translated, and when

The front end keeps GCC's lexer, preprocessor, parser and semantic analysis.
It stops GCC before its middle end: gspin translates **GENERIC** (GCC's
language-level trees, before gimplification), so loops, scopes, C++
cleanups and exception constructs are still structured.

* **C** (`c-decl.c`, `finish_function`): each function is translated by
  `gspin (fndecl)` *before* `c_genericize`. The comment there gives the
  reason: genericizing "lowers the tree and destroys high-level program
  info useful for program optimization". At the end of the file,
  `c_write_global_declarations` writes the spin file and **exits** with
  `EXIT_SUCCESS` (or 2 on errors). GCC's own optimizers and RTL back end
  never run.
* **C++** (`cp/decl2.c`, `cp_finish_file`): after the whole translation
  unit is parsed, templates instantiated and implicit members generated,
  `gspin (global_namespace)` translates everything reachable from the
  global namespace. GCC decides which declarations it would emit; those
  calls (`passes.c`, `varasm.c`, `decl2.c`) are recorded with
  `gspin_gxx_emits_decl`, `gspin_gxx_emits_thunk_decl` and
  `gspin_gxx_emits_asm`, so wgen emits the same set.
* GCC was changed in several places to keep information wgen needs:
  * constructor and destructor clones are made before gimplification
    (`cp-gimplify.c`);
  * `__builtin_va_start` is not lowered (`gimplify.c`);
  * the visibility of declarations is recorded;
  * and more, in 113 `KEY` blocks in 35 files.

What GCC has already resolved by the time wgen sees the trees:

* Overload resolution, implicit conversions (as `NOP_EXPR` /
  `CONVERT_EXPR`), template instantiation, name lookup.
* Name mangling: `DECL_ASSEMBLER_NAME` is computed in `gs_x_1` and passed
  along.
* Class layout: field offsets and sizes (`DECL_FIELD_OFFSET`, `TYPE_SIZE`),
  base-class subobjects, vtable layout. Vtables, VTTs and RTTI objects
  (`typeinfo`) are ordinary `VAR_DECL`s with `CONSTRUCTOR` initializers.
* Constructor/destructor variants (complete, base, deleting) as separate
  `FUNCTION_DECL`s, and thunks as `THUNK_DECL`s with their adjustments.
* Constant folding (`fold`).

What GCC leaves for wgen:

* All control flow and expressions, as GENERIC trees.
* Scope cleanups and temporaries: `CLEANUP_STMT`, `CLEANUP_POINT_EXPR`,
  `TARGET_EXPR`, `AGGR_INIT_EXPR`, `INIT_EXPR`.
* Exception handling structure: `TRY_BLOCK`, `HANDLER`, `EH_SPEC_BLOCK`,
  `MUST_NOT_THROW_EXPR`, `THROW_EXPR`, `TRY_CATCH_EXPR`,
  `TRY_FINALLY_EXPR`.
* OpenMP directives (`OMP_PARALLEL`, `OMP_FOR`, … and their clauses).
* Virtual calls (`OBJ_TYPE_REF`) and pointers to members (`PTRMEM_CST`).

## 2. The gspin format (libspin)

gspin is a serialized copy of GCC's trees, independent of GCC's own data
structures, so that wgen (C++, Path64 headers) never links GCC.

* **Nodes** (`gspin-tree.h`, `gspin-tree.c`). A node is a code
  (`gs_code_t`) plus a fixed number of operand pointers. The arity comes
  from `tree_code_table` in `gspin-tree.c`; for example `FUNCTION_DECL`
  and `VAR_DECL` have 43 operands. Leaf node types carry values: `IB_INT`,
  `IB_STRING`, `IB_BIT_VECTOR` and others. Lists are `CONS`/`EMPTY` chains.
* **Fields** (`gspin-tel.h`, "tree element layout"). Each GCC tree class
  has fixed operand slots, such as `GS_TREE_TYPE`, `GS_DECL_NAME` and
  `GS_DECL_ASSEMBLER_NAME`. It also has one 64-bit flag vector,
  `GS_FLAGS`, whose bit numbers are defined per class (`GS_TREE_PUBLIC`,
  `GS_DECL_WEAK`, …).
  * Accessors are generated by the `GS_LOOKUP`, `GS_LOOKUP_FLAG` and
    `GS_UPDATE_FLAG` macros: 322 of them, such as `gs_decl_weak (t)` and
    `gs_type_precision (t)`.
  * All 64 flag bits of the declaration class are in use, so new
    information needs a new operand. That means raising the arity for
    every code that has it, as was done for `GS_DECL_VISIBILITY`.
* **Program root** (`GS_PROGRAM_*` in `gspin-tel.h`): cc1 command-line
  arguments, global trees, integer types, the declaration list, the
  declarations GCC emitted, top-level `asm`s, program flags and weak
  declarations.
* **File I/O** (`gspin-io.c`). The writer converts node pointers to
  indices and writes the node pool. The reader `mmap`s the file and
  converts indices back to pointers. The format is private to one
  compilation: `cc142` and `wgen42` must be built from the same libspin.

**Translation** (`GCC/gcc/tree.c`, `gs_x_1`):
* It walks a GCC tree recursively and creates or updates the gspin node
  for each tree, memoized through `GS_NODE (t)` and a sequence number.
* A `FUNCTION_DECL` body is only translated when that function is being
  "spun"; when it is only referenced, just the declaration is translated.
* Declarations seen again later are revisited, so flags set after their
  first translation (for example `TREE_PUBLIC`, `DECL_WEAK`, visibility)
  are updated.

## 3. wgen: gspin → WHIRL

`src/wgen/main.c` reads the spin file and then:

1. calls `Process_Cc1_Command_Line` (cc1's options become wgen settings),
   `WGEN_Init` and `WGEN_File_Init`;
2. runs `WGEN_Expand_Top_Level_Decl` and `WGEN_Expand_Defers` for each
   entry in `GS_PROGRAM_DECLARATIONS`;
3. finishes with `WGEN_Alias_Finish` (C), `WGEN_Weak_Finish`,
   `WGEN_File_Finish` and `WGEN_Finish`, which write the `.B` file through
   `ir_bwrite`.

| File | Lines | Role |
|---|---|---|
| `wgen_expr.cxx` | 8,900 | Expressions (126 tree codes), builtins and intrinsics, calls, virtual calls, `TARGET_EXPR`/`AGGR_INIT_EXPR` temporaries, lvalue and address computation, bit-fields, 128-bit-integer rejection |
| `wgen_decl.cxx` | 5,000 | Functions (`WGEN_Start_Function`, `WGEN_Finish_Function`), global and local variables, aggregate initializers (vtables, RTTI, arrays and structs → `INITO`/`INITV`), thunks (`WGEN_Generate_Thunk`), namespaces and templates, deferred functions, nested functions, aliases, weak symbols |
| `wgen_stmt.cxx` | 4,600 | Statements: `if`, loops (`WHILE_DO`, `DO_WHILE`, labels and gotos), `switch` (`SWITCH`/`CASEGOTO`), `asm`, C++ cleanups (a cleanup stack run on scope exit, break, continue, return and goto), exception handling (`WGEN_Expand_Try`, `WGEN_Expand_EH_Spec`, handlers → WHIRL EH `REGION`s), OpenMP (`WGEN_Expand_Omp`) |
| `wgen_spin_symbol.cxx` | 2,600 | Types (`Get_TY`: GCC types → `TY`, including record layout from GCC's field offsets) and symbols (`Get_ST`: storage class, export class and visibility, sections, TLS) |
| `wgen_dst.cxx` | 3,100 | Debug information (DST, which the back end writes as DWARF) |
| `wgen_omp_*.cxx`, `omp_directive.cxx` | 4,300 | OpenMP directive and clause checking and lowering into WHIRL pragmas and regions |
| `wgen_misc.cxx`, `main.c`, `c_int_model.c`, `varasm.c` | 1,300 | Options, file setup, error reporting, register names |

What wgen produces:

* **Very High WHIRL**: structured `IF`, `WHILE_DO`, `DO_WHILE`, `SWITCH`,
  `REGION`s for exception handling and OpenMP, and `COMMA`/`CSELECT`
  expressions. Lowering to High, Mid and Low WHIRL happens in the back
  end (`be/com/wn_lower.cxx`), and LNO sees the structured loops.
* **Symbol table**: `ST`s, `TY`s, `PU`s, and `INITO`s for initialized
  data. The source's types and layout come straight from GCC's.
* **Exception handling**: WHIRL EH regions, plus per-function tables in
  the PU's `misc_info` INITO (exception pointer, filter, type-info table,
  exception specifications). The back end turns these into
  `.gcc_except_table` and `.eh_frame` for the Itanium C++ ABI (personality
  `__gxx_personality_v0`). In C, the same `misc_info` slot holds the list
  of nested functions (see `PU::Get_eh_spec` in
  `common/com/symtab.cxx`).
* **Inlining requests**: C++ `inline` functions are marked
  `PU_is_inline_function`, and wgen asks for the inliner phase.

wgen does **not** do target ABI lowering: argument passing, return
values and struct classification come later from the back end's
`targ_sim.cxx` for each target. Whatever produces WHIRL only has to give
the right types and layout.

## 4. Interfaces a replacement front end must match

Anything that replaces cc1 + gspin + wgen has to produce the same kind of
`.B` file:

* **Very High WHIRL** with structured control flow, so LNO and IPA work
  as they do now. The 2012 cfe-dev discussion
  (see [clang-feasibility.md](clang-feasibility.md)) makes the same point.
* **Types and layout** as `TY`s that match the C/C++ ABI the back end
  implements: Itanium C++ ABI record layout, vtables and RTTI objects as
  initialized data, mangled names.
* **C++ constructs** that GCC currently resolves, so a new front end must:
  * emit the constructor/destructor variants (C1/C2/D0/D1/D2);
  * emit thunks, vtables, VTTs and `typeinfo` objects;
  * emit guard variables for local statics;
  * emit static initialization functions (`_GLOBAL__I_*`, placed in
    `.ctors`);
  * emit temporaries with their destructors, and cleanup regions;
  * emit EH regions and the `misc_info` tables described above.
* **Debug information** as DST.
* **OpenMP** as the same WHIRL pragmas and regions that
  `wgen_omp_directives.cxx` produces.
* **Driver protocol**: write the `.B` file and exit with `RC_OKAY`, or
  with `RC_NEED_INLINER` when the inliner should run. The driver already
  has a path for such a front end: `P_psclang` runs
  `psclang -x c|c++ … -c -whirl <input> -o <file>.B`
  (`phases.c`, `add_psclang_language_args`). `psclang/CMakeLists.txt`
  builds it from `PATH64_PSCLANG_SOURCE`, which is not in this tree.

## 5. Licensing notes (from the file headers in this tree)

These notes record what the file headers say. They are not legal advice.

* Most of the compiler (SGI's Open64 code) is "GPL version 2, or (at your
  option) any later version".
* About 30 files from the Path64 period say "version 3", **including all
  of libspin**, which `cc142`, `cc1plus42` and `wgen42` all link.
  `wgen/main.c` and several wgen headers also say version 3.
* About 27 files say "version 2" with no "or later". In wgen these are:
  * `wgen_expr.cxx`, `wgen_stmt.cxx`, `omp_directive.cxx`,
    `wgen_omp_directives.cxx` and `wgen_omp_check_stack.cxx`: copyright
    PathScale, and AMD for `wgen_expr.cxx`;
  * elsewhere: a handful of back-end and `common` files, copyright
    PathScale, ST Microelectronics and SGI.

So the headers already disagree: a binary that combines GPL-v3-only
libspin with v2-only wgen files can't satisfy both. If the project moves
to GPL v3 (for a newer GCC front end, for example), the v2-only files are
what needs checking with their copyright holders. Most of them are
PathScale's, and PathScale itself relabeled libspin as v3.
