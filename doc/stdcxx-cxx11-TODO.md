# C++11 stdcxx: TODO

The C++11 (and C++14) library for the Clang front end (psclang) is being
added to stdcxx. `doc/stdcxx-cxx11.md` has the design and what is done.
This file lists what is left, for anyone who wants to take a piece. Put
your name on an item before you start, and tick it off (or update the
numbers) in the same change that does it.

## Getting set up

1. **Build the suite** as usual. The build's stage directory,
   `BUILD/Xcompiler`, has `bin/pathCC` and the staged headers in
   `include/1.0.0/stl`.
2. **Fetch libc++ 8.0.1.** Its tests are the conformance check:
   ```sh
   compiler/stdcxx/tests/cxx11/fetch-libcxx.sh ~/libcxx-8.0.1
   export LIBCXX_SRC=~/libcxx-8.0.1 PATH64_STAGE=BUILD/Xcompiler
   ```
3. **Run tests.** Paths are relative to libc++'s `test/std`:
   ```sh
   compiler/stdcxx/tests/cxx11/lcxx.py -v utilities/tuple
   compiler/stdcxx/tests/cxx11/lcxx.py -std c++14 -O O2 containers/sequences/vector
   ```
   The results are PASS, COMPILE, RUN, TIMEOUT, UNSUPPORTED (excluded by
   libc++'s markers for that `-std`) and XFAIL (libc++ expects a failure
   with clang-3 or clang-4; psclang is Clang 4.0.1).
4. **After editing a header** under `compiler/stdcxx/include`, stage it:
   ```sh
   ninja -C BUILD libstdcxx/stdcxx-headers-stage
   ```
   For a *new* header file, re-run `cmake .` in BUILD first; the header
   list is a glob, evaluated when CMake runs.

## Library bug or compiler bug?

psclang is still young, and a failing test can be either. Run the test
again with `--host`, or compile your own test with
`compiler/stdcxx/tests/cxx11/hostcc.sh`. Both use the host's `clang++`
with the same stdcxx headers and runtime libraries.
* **Right with `--host`, wrong with psclang:** a psclang bug (clang2whirl
  or the back end). Reduce it to a small test without the library, then
  fix it or file it under "psclang bugs" below.
* **Wrong with both:** a library bug.

## Rules

* **The gate.** C++11 code is only under `_RWSTD_CXX11`. It is defined for
  `__cplusplus >= 201103L` and turned off by `_RWSTD_NO_CXX11`.
* **C++98 must not change.** The GCC front end and C++98 builds see
  exactly the headers they did before. Check with the GCC front end's
  C++ probes and a library build (below).
* **Header-only.** The library binary (`libstl`) is built in C++98; don't
  add symbols to it. New code goes in templates and inline functions, in
  `rw/_c11_<name>.h`, included from the standard header.
* **Names.**
  * Internal names go in `std::__rw_c11` (helpers) or `std::__rw_tt`
    (traits).
  * Identifiers are reserved names: `_Tp`, `__x`.
  * Use `_RWSTD_CONSTEXPR14` for `constexpr` that C++11 does not allow
    (C++11 makes `constexpr` member functions `const`).
* **psclang builtins.**
  * Use `__sync_*` for atomics, not `__atomic_*` or `__c11_atomic_*`:
    psclang does not implement those yet.
  * `__builtin_add_overflow` and the other checked-arithmetic builtins
    are not implemented either.
* **libc++ 8 is the reference.** It is NCSA/MIT licensed; see
  `compiler/stdcxx/LICENSE-libcxx.txt`. Following its design is fine.
  Files that do say so in their header comment. Don't take code from
  libc++ 9 or later (relicensed to Apache-2.0 with the LLVM exception).
* **Before committing,** run the affected libc++ directories at `-O0` and
  `-O2`. If you touched a header that C++98 code uses (`rw/_defs.h`,
  `_pair.h`, `_algobase.h`, the containers), also run the GCC front
  end's C++ probes and rebuild `libstl`. Update the numbers here and in
  `doc/stdcxx-cxx11.md`.

## Phase 1 status (library)

Measured with C++11 at `-O0`; "host" means with `--host`.
The affected tuple, functional, smart-pointer and allocator directories pass
286/287 tests in C++11 and 297/298 in C++14 at both `-O0` and `-O2`.
The compiler lifetime and virtual-inheritance regressions pass at both levels.
The remaining failure is the libc++ tuple extension `PR31384` below.
Host Clang passes 286/287 C++11 tests.
The shared and static C++98 library rebuilds pass, as do all 11 C++98/C++03
language probes at both optimization levels.

Expanded validation includes traits, utility, pointer traits, array and both
compiler regressions. On x86-64 it passes 488/489 tests in C++11 and 503/504
in C++14 at each optimization level, with one expected Clang-4 failure
excluded from each denominator. `PR31384` is the sole remaining run failure.

### `<type_traits>` — 123/123 (+1 XFAIL)
- [x] (Codex) **`result_of` with a `reference_wrapper` object.** For example,
  `result_of<int (S::*(reference_wrapper<S>))()>`: INVOKE on a
  `reference_wrapper` (LWG 2219; libc++ does this in every mode).
  Tests: `meta.trans.other/result_of.pass.cpp`, `result_of11.pass.cpp`.
- [x] (Codex) **`is_swappable_include_order.pass.cpp`.** Swap detection
  and algorithms handle nested arrays when `<algorithm>` comes first.

- [x] (Codex) **C++14 category traits and qualified function decay.**
  `is_null_pointer` handles cv qualification, and `decay` avoids forming
  pointers to cv-qualified function types. Traits pass 125/125 in C++14,
  alongside the expected Clang-4 failure.

### `<utility>` — 41/41
- [x] (Codex) **`get` on `const pair&&`.** Test:
  `pairs/pair.astuple/get_const_rv.pass.cpp`.
- [x] (Codex) **C++14 `exchange` with an empty braced value.** String default
  construction accepts copy-list initialization through a header definition
  available alongside the C++98 extern instantiation. Utility passes 43/43
  in C++14.

### `<tuple>` — 54/55
- [x] (Codex) **`get` on `const tuple&&`.** Test: `tuple.elem/get_const_rv`.
- [x] (Codex) **SFINAE-friendly `tuple_size` for cv-qualified types.**
  Tests:
  `tuple_size_incomplete`, `tuple_size_value_sfinae`.
- [x] (Codex) **`tuple_cat` of `std::array` arguments.**
  Test: `tuple.creation/tuple_cat`.
- [x] (Codex) **`std::allocator<void>` from `<tuple>`.** The existing
  specialization is visible to allocator-extended tuple constructors.
  Test: `tuple.cnstr/alloc_UTypes`.
- [x] (Codex) **Constructor constraints.** Copy, move, forwarding and
  allocator-extended constructors reject incompatible arguments through SFINAE.
  `move`, `PR23256_constrain_UTypes_ctor` and
  `PR22806_constrain_tuple_like_ctor` pass.
- [x] (Codex) **Constant-expression assignment to `ignore`.** The C++14
  `tuple.general/ignore` test passes.
- [x] (Codex) **`PR31384.pass.cpp` reviewed.** This test exercises a libc++
  tuple-like constructor extension. Its derived-tuple conversion-operator
  preference differs here with both psclang and host Clang. The system C++
  library also fails at the same assertion. Its result remains visible in
  the test counts.

### `<functional>` — 88/88
- [x] (Codex) **Bitwise functors.** Added typed `bit_and`, `bit_or`, and
  `bit_xor` in all modes, plus C++14 `bit_not` and the transparent forms.
- [x] (Codex) **Legacy callable typedefs.** `function` exposes its
  argument typedefs for unary and binary signatures. `reference_wrapper`
  exposes the weak result and argument types of the wrapped callable. Tests:
  `func.wrap.func/types`, `refwrap/weak_result`.
- [x] (Codex) **Hash support.** `<string>` supplies `hash<string>` and
  `hash<wstring>`. Signed and unsigned `__int128` hashing completes
  `unord.hash/enabled_hashes`, including its existing `hash<bool>` check.
- [x] (Codex) **`<functional>` includes `<memory>` and `<cstdint>`.**
  The inherited-constructor parameter forwarding fix also makes
  `derive_from.pass.cpp` pass.
- [x] (Codex) **`F_incomplete.pass.cpp`.** Copy and move operations avoid
  evaluating the callable constraint against the wrapper itself.
- [x] (Codex) **Reentrant target destruction.** Move and swap clear the
  wrapper's state before destroying the inline target. The host test
  `move_reentrant.pass.cpp` passes with this change. See the psclang
  failure below.
- [x] (Codex) **Math and integer dependencies.** Classification declarations
  precede their template uses. Mixed arithmetic `pow` supports nested binds.
  `bind/copy`, `bind/nested`, `unord.hash/floating` and `unord.hash/integral` pass.
- [x] (Codex) **C++14 transparent functors.** Arithmetic, comparison,
  logical and bitwise functors support `<void>` and the default `<>`
  forms. The typed operators also support constant evaluation in C++14.

### `<memory>` — util.smartptr 84/84 (host 84), smartptr 39/39 (host 39)
- [x] (Codex) **Fancy pointers.** The control blocks have to work with
  allocators whose `pointer` is a class (`min_pointer`): convert with
  `pointer_traits`, keep raw pointers. Tests: `shared_ptr` constructors
  with an allocator, `allocate_shared*`.
- [x] (Codex) **`enable_shared_from_this` as a private base** must be ignored, not
  rejected. Test: `util.smartptr.enab`.
- [x] (Codex) **`owner_less` typedefs:** `first_argument_type` and the rest.
- [x] (Codex) **`unique_ptr` deleters must not be copied.**
  * `__ptr_del` should forward its deleter.
  * Non-copyable deleters must work.
  * The constructor constraints need fixing (`pointer_deleter`).
  * `noexcept` and the converting assignment in `move_convert*`.
- [x] (Codex) **`shared_ptr(unique_ptr&&)`.** Failed control-block allocation
  preserves the unique owner and its deleter.
- [x] (Codex) **`enabled_hash` checks.** The missing specializations were
  signed and unsigned `__int128`. `hash<bool>` already existed.
- [x] (Codex) **`shared_ptr` atomics** (`atomic_load`, `atomic_store`,
  `atomic_exchange`, `atomic_compare_exchange_*`): 12 tests. Use a
  spin-lock pool with `__sync`.
- [x] (Codex) **`allocator_traits::allocate` with a hint** on an incomplete
  `value_type` (`allocate_hint.pass.cpp`).

### `<array>` — 27/27
- [x] (Codex) **`array<T, 0>`.** Its storage matches the size and alignment of
  `T[1]` without constructing an element. Const elements disable assignment
  and zero-length swap is `noexcept`. Tests: `implicit_copy`,
  `size_and_alignment`, `array.swap/swap`.
- [x] (Codex) **`std::addressof` and `std::uintptr_t` from `<array>`.**
  Tests: `contiguous.pass.cpp`, `array.data/data*.pass.cpp`.
- [x] (Codex) **`get` on `const array&&`.**

## psclang bugs found by the tests (compiler)

These fail with psclang and pass with `--host`. Reduce each one to a small
test without the library before fixing it in clang2whirl.
- [x] (Codex) **`std::function` from lambdas and functors.** Placement-new
  destinations preserve the allocation pointer type, including member pointers.
- [x] (Codex) **`std::function` small-buffer assignment.** Conditional
  statement temporaries are destroyed inside the branch that constructs them.
  `move_reentrant.pass.cpp` passes with psclang at both optimization levels.
- [x] (Codex) **Throwing in a `shared_ptr` constructor.** Each constructor
  captures its live argument temporaries in an unwind region before adding
  its destination to the cleanup stack. The four `*_throw.pass.cpp` cases pass.
- [x] (Codex) **`dynamic_pointer_cast.pass.cpp`.** Class-valued conditional
  expressions construct into one shared destination. Inactive alternatives
  no longer leave uninitialized objects on the destructor stack.
- [x] (Codex) **`util.smartptr.shared.io/io.pass.cpp`.** Base constructors and
  destructors receive ABI VTT arguments. Construction vtables, virtual-base
  casts and hidden nontrivial return values interoperate with the runtime.
  Exact stream output and mixed GCC/psclang object files are also checked.
- [x] (Codex) **`unique.ptr.ctor/move.pass.cpp`.** Guarded local statics
  register their object address with `__cxa_atexit`, and register destruction
  before publishing completed initialization.
- [x] (Codex) **Static-storage `initializer_list`.** Namespace-scope lists
  receive runtime initialization for their backing array and extent.
  `compiler-lifetime.pass.cpp` covers this alongside object lifetime fixes.
- [x] (Codex) **`vector<bool>` iterator operations.** Base casts now return
  an address with the target type. Const forward and reverse iterator
  operations pass in C++11, and the C++98 iterator probe also passes.
- [x] (Codex) **Template statics across translation units.** Static member
  definitions instantiate through Sema. Inline-function local statics and
  their guards coalesce across object files, including the shared-pointer
  atomic lock pool. Inline function definitions use weak external linkage.
  A four-thread probe also covers cross-file stores and reentrant deleters.


## Phase 2: containers

- [x] (Codex) **Normal `vector<T>` C++11 members.** Allocator-aware copy/move
  construction and assignment, initializer lists, emplacement, rvalue insertion,
  `const_iterator` modifiers, default insertion, `data`, `shrink_to_fit`, and
  required `noexcept` declarations use the existing storage representation.
  Element construction uses `allocator_traits`, including fancy pointers and
  move-only elements. C++98 retains its original implementation.
  The unmodified reference suite passes 51/54 runnable tests; three tests assume
  class iterators (unqualified `distance`/`next`, and `--v.end()`) and fail to
  compile with stdcxx's permitted raw-pointer iterators. Temporary copies using
  `std::distance`, `std::next` and `std::prev(v.end())` check these cases.
  The adapted cases and original regressions pass in C++11/C++14 at `-O0`
  and `-O2`. Three later-standard tests are unsupported in these modes.
  Original `vector-exceptions.pass.cpp` checks rollback, object counts and
  aliased emplacement. The vector work also fixes psclang's unbraced-loop
  temporary destruction, empty-object pointer strides and EH traversal of
  branches with a nonthrowing constructor before a throwing call.

- [x] (Codex) **`vector<bool>` C++11 members.** Allocator-traits rebinding,
  fancy-pointer allocation, allocator propagation, moves, initializer lists,
  emplacement, `const_iterator` modifiers, `shrink_to_fit`, proxy swaps and
  `hash<vector<bool, A>>` retain packed word storage. Required nonthrowing
  operations have `noexcept` declarations. The reference suite passes 42/42
  tests in C++11 and 44/44 in C++14; two C++14 tests are unsupported in C++11.
  Mutable/const iterators also support subtraction in either order.
  Original `vector-bool.pass.cpp` checks single-pass input ranges, allocation
  rollback, proxy/bool swaps and hash equality after storage padding changes.
  Compiler fixes materialize returned records for member calls and copy base
  subobject values when a prvalue supplies an inherited member operator.

- [x] (Codex) **`deque` C++11 members.** Allocator-aware construction and
  propagation, moves, initializer lists, emplacement at both ends and in the
  middle, `const_iterator` modifiers, default insertion, range access and
  `shrink_to_fit` retain the existing segmented representation. Internal raw
  block/map pointers use an allocator-traits bridge for fancy allocators.
  Growth guards release both allocations on failure; shrinking compacts the
  pointer map without relocating elements. All 61 runnable reference tests
  pass in C++11/C++14; three later-standard tests are unsupported.
  `deque-exceptions.pass.cpp` checks allocation and construction rollback,
  aliased emplacement, reference stability and unequal-allocator moves.

The deque checks also fix nontrivial class returns: psclang constructs results
in the caller's ABI return area rather than copying a completed object.
Direct, indirect, virtual and member-pointer calls and adjusting thunks are
covered by `compiler-return-object.pass.cpp` and mixed host/psclang probes.
On x86, optimized C++ with exceptions retains frame pointers by default:
frameless helpers lack stack-adjustment CFI and could otherwise skip cleanup
frames. Cleanup sharing also preserves destruction order when landing-pad
sequences differ. `compiler-unwind.pass.cpp` and deque rollback checks cover
these paths.
C++14 sized deletion is declared in `<new>` and implemented in libcxxrt, with
`sized-delete.pass.cpp` checking replacement of unsized deallocators.

The other containers below still need their C++11 extensions; `list` is next.

Add the C++11 members to stdcxx's own containers, under `_RWSTD_CXX11`:
- move constructor and move assignment (allocator propagation through
  `allocator_traits`);
- `initializer_list` constructor, assignment and `insert`;
- `emplace`, `emplace_back` and `emplace_front`, `emplace_hint`;
- `insert` and `push_*` from rvalues;
- `cbegin`/`cend`/`crbegin`/`crend` (done for `vector`,
  `vector<bool>`, `deque`, and `list`);
- `shrink_to_fit`, `data()` for `vector`;
- `erase` and `insert` taking `const_iterator`;
- `noexcept` where the standard says;
- allocator-extended copy and move constructors.

Construct elements with `allocator_traits<A>::construct`, so that
`emplace` works with any allocator.

| Container | Header | libc++ tests |
|---|---|---|
| vector | `vector`, `vector.ipp` | `containers/sequences/vector` (57) |
| vector\<bool\> | `vector` | `containers/sequences/vector.bool` (44) |
| deque | `deque`, `deque.ipp` | `containers/sequences/deque` (64) |
| list | `list`, `list.ipp` | `containers/sequences/list` (63) |
| map, multimap, set, multiset | `map`, `set`, `rw/_tree.h` | `containers/associative` (246) |
| stack, queue, priority_queue | `stack`, `queue` | `containers/container.adaptors` (99) |
| basic_string | `string`, `string.ipp` | `strings/basic.string` (217) |

`basic_string` is reference-counted (copy-on-write) in stdcxx. C++11
does not allow that (`data()` and `operator[]` must not reallocate,
iterators stay valid). Decide whether to keep copy-on-write for C++98 and
use a non-shared representation in C++11. A separate representation
changes the layout, so C++98 and C++11 code can't share string objects
across a library boundary. Write the decision here before starting.

## Phase 3: the rest of the existing headers

The iterator/algorithm/numeric checkpoint includes eight original library
and compiler regressions: 288/319 pass in C++11 and 289/320 in C++14 with both
psclang and host Clang at `-O0` and `-O2`. This batch has no runtime failures;
the 31 compile failures are classified below. Reference markers exclude 25
C++11 and 24 C++14 tests.

- [ ] **`<iterator>`:**
  * [x] (Codex) `begin`, `end` and C++14 const and reverse range access
    (`iterator.range/begin-end.pass.cpp`);
  * [x] (Codex) `next`, `prev`;
  * [x] (Codex) `move_iterator`, `make_move_iterator` (29 reference tests);
  * [x] (Codex) C++14 `make_reverse_iterator`, mixed reverse-iterator
    comparisons, value initialization, SFINAE-safe iterator traits and
    cv-correct pointer traits;
  * [x] (Codex) Move insertion through all three output adapters, constexpr
    input-stream iterator default constructors, stream-buffer iterator
    reference typedefs. Front insertion is checked independently with deque;
    its reference test still requires C++11 move insertion in `list`.
  * [x] (Codex) psclang short-circuit operand materialization and conditional
    temporary cleanup: stream-buffer iterator runtime tests now pass.
    `compiler-short-circuit.pass.cpp` covers skipped calls, nested operands,
    full-expression lifetime, constructor failures and unwind cleanup.
  * `iterators` (161 tests): 156/157 applicable in C++11 and 157/158 in
    C++14; the list-dependent front-insertion test remains.
- [ ] **`<algorithm>`:**
  * [x] (Codex) `all_of`, `any_of`, `none_of`, `find_if_not`;
  * [x] (Codex) `move`, `move_backward` (including the deque-specific cases);
  * [x] (Codex) `copy_if`, `copy_n`, C++11 `generate_n` return value;
  * [x] (Codex) `is_partitioned`, `partition_copy`, `partition_point`,
    `is_sorted`, `is_sorted_until`, `is_heap`, `is_heap_until`;
  * [x] (Codex) `minmax`, `minmax_element`, the `initializer_list` forms of
    `min`/`max`, `is_permutation` (including the C++14 bounded overloads);
  * [x] (Codex) Move-only `remove`, `remove_if`, `unique`, and `rotate`;
    C++11 `rotate` return value; overloaded-comma protection in transforms,
    replacement copies, range swaps, lexical comparisons and permutations.
  * `shuffle` (pending `<random>`);
  * [x] (Codex) C++14 `equal`/`mismatch` with four iterators;
  * [x] (Codex) Move-only `sort`, heap operations and `partial_sort`;
    pivot partitioning avoids copying or comparing moved-from values.
    Heap temporaries hold iterator values, not packed-bit proxy aliases;
    proxy `iter_swap` finds ADL swaps or saves a value for its fallback.
  * [x] (Codex) Move-only `stable_partition`, with owned temporary storage,
    partial-buffer recursion, allocation-failure fallback and exception cleanup.
    Original regressions exercise throwing moves/predicates, bounded buffers,
    stable order, identity preservation, comparison bounds and packed bits.
  * [x] (Codex) Move-only `nth_element`, with an in-range pivot and three-way
    partitioning to discard equal keys together. Original tests cover every
    selection position, duplicate-heavy inputs, descending comparisons,
    packed-bit proxies, identity preservation and comparison exceptions.
  * [x] (Codex) Move-only `stable_sort`, with owned, partially constructed
    temporary storage shared with `stable_partition`. Buffered merges preserve
    left-run ties; undersized/failed allocations use rotation-based merging.
    Original tests cover stable identity ordering, both comparator directions,
    packed-bit proxies, comparison bounds and throwing moves/comparisons.
  * `algorithms` (150 tests): 115/145 runner-selected tests pass. The remaining
    30 compile failures are 28 missing `<random>` dependencies and two
    unmarked C++17 `clamp` tests. Five tests are excluded by reference markers.
    The `stable_sort` and `nth_element` reference tests require `<random>`;
    their move-only implementations have original regression coverage. The
    overall item remains open.
- [x] (Codex) **`<numeric>`:** `iota`; move assignment in
  `adjacent_difference` and safe iterator increments in `inner_product`.
  `numerics/numeric.ops`: 9/9 applicable tests; 16 later-standard tests
  excluded in C++11/14.
- [x] (Codex) **`<string>` Phase 3 additions:**
  * [x] (Codex) `to_string`, `to_wstring`, `stoi`/`stol`/`stoul`/`stoll`/
    `stoull`/`stof`/`stod`/`stold`, for narrow and wide strings.
    Parsing preserves errno, checks both libc and narrower `int` ranges,
    and updates the consumed position only on success. Formatting grows
    storage for large fixed-point results. Ten reference tests plus the
    original boundary/formatting regression pass 11/11 with both compilers
    in C++11/14 at `-O0` and `-O2`.
    The same work adds the C++11 stdio formatting/scanning names and stdlib
    numeric-conversion, long-long division and `_Exit` names in `std`.
    Both reference header checks pass with both compilers across that matrix;
  * `hash<string>` and `hash<wstring>` are implemented.
  * [x] (Codex) `u16string`/`u32string`, their hashes, dedicated
    `char_traits<char16_t>`/`char_traits<char32_t>` and stream-position aliases.
    Code-unit integer types match `<cstdint>`; bounded searches never inspect
    the next element. Narrow/wide scalar trait operations are constexpr in
    C++11/14. `strings/char.traits`: 64/64 applicable reference tests pass;
    15 C++20 char8_t tests are excluded. With conversion tests and the original
    Unicode regression, 75/75 pass with both compilers in C++11/14 at `-O0`/`-O2`.
    Original checks include guard-page search bounds, overlapping moves,
    surrogate/code-point storage, mutation/copies, modifiers, hashes and constants.
    C++11 `front()`/`back()` are also implemented, including COW detachment.
    Phase 2 basic_string move/emplacement modernization remains separate.
- [x] (Codex) **C library headers (Linux x86-64):**
  * `<cmath>`: C++11 C99 functions, float/long-double and integral/mixed
    overloads, suffixed functions, boolean classification/comparison predicates
    and ADL-safe arithmetic constraints. Classification handles signed zero,
    subnormals and long double; unordered comparisons avoid ordered NaN tests.
  * `<cstdint>`: existing native-libc imports verified, including type identity,
    integer limits and constant macros; no replacement implementation needed.
  * `<cinttypes>`, `<cfenv>`, `<cuchar>`, `<ctgmath>`, `<cstdbool>` wrappers.
    Unicode conversion delegates to libc and its locale/state handling.
  * Eight libc++ reference tests and two original regressions pass 10/10
    with psclang and host Clang in C++11/14 at `-O0`/`-O2`. The originals
    exercise stable sorting and C-header integration, floating-point flags,
    rounding modes, signed zero, NaNs, integer formatting/parsing, and UTF-16/32.
    All 200 shared/static C++98 library objects rebuild; a legacy stable-sort/
    math probe passes with GCC, psclang and host Clang at both levels.
    IRIX C++ library integration remains open: its sysroot lacks `<uchar.h>`
    and several native fenv functions return `void` rather than C99's `int`.
- [x] (Codex) **`<exception>`:** `exception_ptr`, `current_exception`,
  `rethrow_exception`, `make_exception_ptr`, `nested_exception`,
  `throw_with_nested`, `rethrow_if_nested`; exposes the runtime's existing
  `get_terminate`/`get_unexpected` APIs. Exception handles own libcxxrt primary
  references and rethrow through dependent exceptions; the runtime's rethrow
  helper now derives the primary header from the object pointer correctly.
  Ten propagation/nested reference tests plus two original regressions pass
  12/12 with psclang and host Clang in C++11/14 at `-O0` and `-O2`.
  Original checks cover lifetime/catch cleanup, cross-thread rethrows, nested
  runtime_error propagation, throwing copies and required termination cases.
  The full exception reference tree has 18/19 selected passes; its unmarked
  `uncaught_exceptions` test requests C++17 and remains outside this item.
  All original regressions pass: 27 applicable in C++11 and 28 in C++14 at
  both optimization levels. C++98 cleanup probes pass with both compilers;
  all eleven C++98/03 language probes remain green. Linux shared/static
  runtimes and all 200 C++98 library objects rebuild; libcxxrt also cross-builds
  shared/static for IRIX n32. No IRIX C++11 execution is claimed.
- [x] (Codex) **`<new>`:** `get_new_handler` and `bad_array_new_length`.
  The six allocation-error reference tests and original handler concurrency/
  callback checks pass with psclang and host Clang in C++11/C++14 at `-O0`/`-O2`.
  Shared/static libcxxrt builds on Linux x86-64 and cross-builds for IRIX n32.
  Invalid-bound `new[]` exception selection is also implemented; see below.
- [ ] **`<ios>`, `<istream>`, `<ostream>`:**
  * [x] (Codex) constrained rvalue insertion/extraction, preserving derived types;
  * [x] (Codex) protected stream-base moves/swaps, stream-buffer copy/swap,
    and string/file buffer and stream moves, assignments and nonmember swaps.
    Stream state moves once across the shared virtual base; buffer pointers
    stay attached to each owning stream. Buffered positions, callback storage,
    user data, external buffers, old-file closure and moved-from reuse have
    original narrow/wide coverage with both compilers in C++11/14 at `-O0`/`-O2`.
    The C++98 shared/static libraries are rebuilt; C++11 callers instantiate the
    affected templates so new members do not require C++11 library binaries.
    String-buffer output now starts at the beginning unless `ate`/`app` is set;
    a separate original regression passes in C++98 and C++14 with both compilers.
  * [x] (Codex) Stream compatibility follow-ups: public flags omit cached
    radix/stdio bits; internal numeric facets use the complete flags. Ordinary
    streams start with exactly `skipws|dec`; locking is enabled unless the
    caller explicitly selects the existing `nolock`/`nolockbuf` extensions.
    Numeric bases, automatic detection, nonstandard radix 3 and stdio
    synchronization remain covered. String buffers retain the input string's
    allocator, use it for every allocation and returned string, and preserve
    allocator ownership across moves/swaps. Unequal nonpropagating allocators
    copy into destination storage. Failed growth retains the previous contents.
    Empty storage for std::allocator preserves the measured default char/wchar
    buffer/stream sizes (128/256 bytes on Linux x86-64).
    The expanded base/string/formatting checkpoint passes 71/71 with both
    compilers at C++14 `-O2` (eight locale exclusions). Original allocator/flag
    tests pass in C++11/14 at `-O0`/`-O2`; flag tests also run in C++98.
    The file-buffer reference support dependency is now supplied by `<codecvt>`.
  * [x] (Codex) Numeric-facet follow-ups: avoid overloaded iterator comma,
    normalize parenthesized member callees in psclang, set range failure for
    unsigned int, apply narrow unsigned negation once, and store zero on failed
    numeric conversion. Clearing the base field also clears its cached radix.
    Bool values/failure/EOF state, hexadecimal floats and infinity/NaN tokens,
    grouping errors with retained values, and dynamically grown digit/group
    buffers now have coverage. All 16 numeric-facet references pass with both
    compilers at C++14 `-O2`. String `front()`/`back()` preserve COW detachment;
    modern callers instantiate the new members from headers.
  * [x] (Codex) `io_errc`, `iostream_category`, traits and error-code/condition
    factories. Four stream-category references pass, alongside 56 system-error
    references (one C++17 variable-trait test excluded), with both compilers.
  * [x] (Codex) Modern `ios_base::failure`: common C++98/C++11 runtime
    hierarchy derives from `system_error`, with error-code constructors and
    `code()`. Runtime stream throws carry `iostream_category`/`io_errc::stream`.
    Both modern failure references pass. Exception messages have shared ownership;
    copying/assigning runtime_error, system_error and failure does not allocate,
    and captured stream exceptions remain valid after the throwing thread exits.
    **ABI transition: rebuild libstl and all clients together.** The failure
    layout, system-error RTTI and private exception-message representation changed.
  * [x] (Codex) `get_money`, `put_money`, `get_time`, `put_time`,
    `hexfloat`, `defaultfloat`, including facet exceptions and output failures.
    Six reference tests and an original integration test pass with psclang and
    host Clang in C++11/14 at `-O0`/`-O2`. Four locale-dependent references are
    unsupported by the runner. The shared formatter now interprets both float
    flags as hexadecimal and ignores precision for that format.
- [x] (Codex) **`<bitset>`:** constexpr default/unsigned-long-long constructors,
  bounded character-pointer constructor, `to_ullong`, `all`, `hash<bitset>`.
  Oversized shifts and empty-bitset operations are corrected in C++98 too.
  All 38 reference tests pass with host Clang and with psclang at `-O0`/`-O2`.
  Original boundary, overflow, hash and protected-page checks pass in both modes.
- [x] (Codex) **x86-64 narrow scalar return ABI:** explicitly narrow returned
  register values before use. Native callees may leave upper bits dirty when
  returning bool, char or short. All sixteen mixed-object checks pass across
  both PathScale frontends, host Clang and GCC at `-O0`/`-O2`; the IRIX backend
  also builds (the change is x86-64 only).
- [x] (Codex) **Optimized 64-bit mask expression:** restrict the legacy
  signed-constant normalization to 32-bit operations. A canonical U4 constant
  may represent the correct result of a 64-bit operation; sign-extending it
  changed `0xffffffff` into `-1`. The original regression covers mask widths
  31/32/33/63/64 and unsigned arithmetic in C++98/11/14 at `-O0`/`-O2`/`-O3`.

Allocation-error/constructor follow-up: all six reference allocation-error tests
plus all seventeen original passing regressions give 22/22 in C++11 (one C++14
sized-deletion test excluded) and 23/23 in C++14, with both compilers at `-O0`
and `-O2`. The broader C++14 `-O2` foundations batch remains 599/603 without
original tests: three raw-pointer-incompatible vector references and the tuple
extension `PR31384` are unchanged.

The numeric/Unicode/file-stream checkpoint passes 55/55 with both compilers
at C++14 `-O2`: 16 numeric references, 24 codecvt references, 12 file-stream
move/swap references and three original regressions. All original regressions
pass with PathScale in C++11/14 at `-O0`/`-O2` (40/41, sized-delete exclusion in
C++11). New regressions pass with both compilers across that matrix. C++98
parenthesized-member and stream-flag checks pass at both levels; eleven legacy
C++98/03 probes remain clear. Unicode/file execution is Linux x86-64; IRIX
remains a WIP port.

## Phase 4: new headers

| Header | libc++ tests | Notes |
|---|---|---|
| `<forward_list>` | `containers/sequences/forwardlist` (67) | |
| `<unordered_map>`, `<unordered_set>` | `containers/unord` (329) | needs `hash<string>` |
| `<ratio>` | `utilities/ratio` (13) | header-only, easy start |
| `<chrono>` | `utilities/time` (269) | clocks via `clock_gettime` (inline) |
| `<system_error>` | `diagnostics/syserr` (57) | Implemented: errc, categories, codes/conditions, comparisons, hashes and system_error; 56 applicable references pass, one C++17 trait-variable test excluded. Allocation-free exception copies and the modern stream-failure hierarchy are implemented; rebuild clients for the new exception ABI. |
| `<atomic>` | `atomics` (52) | `__sync` is available. Native `__c11_atomic_*` and `__atomic_*` lowering remains to be implemented in `c2w_builtin.cxx`. |
| `<thread>`, `<mutex>`, `<condition_variable>` | `thread` (219) | pthreads; `call_once` |
| `<future>` | in `thread` | after the three above |
| `<random>` | `numerics/rand` (493) | engines and distributions, header-only |
| `<regex>` | `re` (167) | large; needs locale `regex_traits` |
| `<typeindex>` | `utilities/type.index` (10) | small |
| `<scoped_allocator>` | `utilities/allocator.adaptor` (30) | |
| `<codecvt>` | `localization/locale.stdcvt` (24) | Implemented: original `codecvt_utf8`, `codecvt_utf16`, `codecvt_utf8_utf16`; 24/24 references pass with both compilers. |
| `<cuchar>` | | |

The system-error/category and existing stream/numeric/Unicode checkpoint passes
171/171 with both compilers at C++14 `-O2` (nine exclusions). Original tests
pass with PathScale: 42 in C++11, 43 in C++14, at `-O0`/`-O2`. New checks pass
with both compilers across that matrix; the virtual-call regression also runs
in C++98. Four mixed Clang/PathScale provider/consumer checks cover category
identity, returned codes and thrown system_error objects at both levels.
C++11 header use also compiles/runs with `-fno-exceptions` in both compiler modes.

### Stream-failure ABI and constant-vptr checkpoint (2026-11-19)

The system-error, stream-category, modern failure and exception-propagation
batch plus three original regressions passes **75/75** with psclang and host
Clang in C++14 at `-O2` (one C++17 exclusion). All original tests pass with
psclang at `-O0`/`-O2`: **45/45** in C++11 (one C++14-only exclusion) and
**46/46** in C++14. Twelve C++98 stream-provider/modern-consumer links cover
legacy PathScale, psclang and host providers at both optimization levels.
Eight C++98 register-unwind links and four mixed links each for constant vptrs
and system errors pass. Both compilers also pass the no-exceptions header probe.
These results require the coordinated exception ABI rebuild described above.

## Infrastructure

- [x] (Codex) **Virtual-call redeclarations and object expressions:** canonical
  declarations identify vtable slots, including member-pointer constants and
  destructor entries. Unqualified calls dispatch virtually through references
  returned by functions, casts and direct objects; explicit qualification keeps
  direct calls. Evaluate the object once for both this and vtable lookup.
  `compiler-virtual-redeclaration.pass.cpp` covers these paths and deletion.
- [x] (Codex) **Constant-initialized polymorphic statics (psclang):** emit
  complete-object vtable relocations, including secondary bases, nested members
  and arrays, at their AST layout offsets. Preserve constant data and register
  destructors without rerunning constructors. Static-local arrays use destructor
  wrappers and reverse destruction order. Standard error categories no longer
  need explicit runtime constructors to avoid missing vptrs. Original tests cover
  early startup dispatch, mutation, bitfields and mixed Clang/PathScale objects.
- [x] (Codex) **Static integral members:** in-class initializers do not define
  external storage. Fold value reads while retaining external references for
  actual storage uses, avoiding duplicate definitions in C++98 stream clients.
- [x] (Codex) **x86 frame-pointer unwind saves:** emit CFI for callee-saved
  registers spilled by GRA. Optimized stream cleanup previously lost caller
  registers across exceptions, breaking later catch dispatch in mixed objects.
  An original register-preservation test and C++98-provider/modern-consumer
  stream tests exercise this path.

- [x] (Codex) **Constructor initializer exception cleanup (psclang).**
  Completed record members and bases are recorded in initialization order,
  with EH snapshots that destroy only the completed prefix in reverse order.
  References do not own their referents. Initializer argument temporaries stay
  alive through the call and are registered only after successful construction.
  The saved reproducer is now `compiler-member-init-unwind.pass.cpp`.
  `compiler-constructor-cleanup.pass.cpp` checks later initializer/body failures,
  failed subobject construction, references, base cleanup, delegating constructors,
  handled/nonmatching inner catches and argument-temporary lifetimes.
- [x] (Codex) **Invalid array-bound exceptions (psclang).** C++11 `new[]`
  checks negative bounds, payload/cookie size limits and initializer-list counts
  before allocation, using a saved bound. Throwing allocation selects
  `bad_array_new_length` through libcxxrt's ABI helper; nonthrowing allocation
  returns null. Null results skip cookie stores and initialization.
  `compiler-array-length.pass.cpp` covers these paths and ordinary `bad_alloc`.

- [x] (Codex) **Loop/switch body jump cleanup (psclang).** `break` and
  `continue` destroy live automatic objects in the scopes they leave, in
  reverse construction order, preserving objects belonging to enclosing
  scopes. Cleanup snapshots leave fallthrough paths intact. The original
  `compiler-loop-cleanup.pass.cpp` checks nested blocks, switches, for/while/do
  loops and C++11 range-for; `nth-element.pass.cpp` also exercises a vector
  destroyed by `continue`.

- [x] (Codex) **Namespaced numeric-limits constants (psclang).** Variables
  with global storage now use Clang's ABI mangling decision, including
  namespace variables inside transparent `extern "C++"` blocks. C linkage
  stays unmangled, and ordinary function locals retain their local names.
  `compiler-limits-constants.pass.cpp` is the promoted reproducer.
  `compiler-variable-linkage.pass.cpp` checks namespace collisions, constants,
  shared C linkage, static members, guarded locals and template-local values.
  Provider/consumer modes allow host-Clang/GCC object interoperability checks.

- [x] (Codex) **Catch parameters and handler-exit cleanup (psclang).**
  Nontrivial catch-by-value parameters invoke their copy constructors and own
  their handler-scope lifetimes, including empty handlers. Copy-initialization
  failures terminate; default-argument temporaries finish before the body.
  Return, break and continue interleave object destruction with end-catch
  calls for the scopes they leave. `compiler-catch-cleanup.pass.cpp` checks
  C++98/11/14 copy counts, destruction order, value/void returns, loop exits,
  empty handlers, throwing parameter destructors and termination on copy failure.

- [ ] **Regular runs.** Run `lcxx.py` from a test target, or a nightly
  job, with a list of the tests expected to pass, so regressions show.
- [ ] **`-std=c++14` and `-O2` across the board.** Completed batches record
  their C++11/C++14 optimization matrix above; extend it to the remaining work.
- [ ] **IRIX (n32, big-endian).** The C++11 headers are untested there.
  The IRIX psclang target is still to come.
- [ ] **A C++11 `libstl`?** Whether something must go into a C++11-built
  part of the library (for example `random_device`, `thread` with a
  non-inline `hardware_concurrency`) or can stay inline.
