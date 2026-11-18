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

## Phase 1 leftovers (library)

Measured with C++11 at `-O0`; "host" means with `--host`.
The affected tuple, functional, smart-pointer and allocator directories pass
285/287 tests in C++11 and 296/298 in C++14 at both `-O0` and `-O2`.
The additional compiler lifetime regression passes at both levels.
The two remaining failures are the stream construction ABI issue and
`PR31384` below. Host Clang passes 286/287 C++11 tests.
The shared and static C++98 library rebuilds pass, as do all 11 C++98/C++03
language probes at both optimization levels.

### `<type_traits>` — 123/123 (+1 XFAIL)
- [x] (Codex) **`result_of` with a `reference_wrapper` object.** For example,
  `result_of<int (S::*(reference_wrapper<S>))()>`: INVOKE on a
  `reference_wrapper` (LWG 2219; libc++ does this in every mode).
  Tests: `meta.trans.other/result_of.pass.cpp`, `result_of11.pass.cpp`.
- [x] (Codex) **`is_swappable_include_order.pass.cpp`.** Swap detection
  and algorithms handle nested arrays when `<algorithm>` comes first.

### `<utility>` — 41/41
- [x] (Codex) **`get` on `const pair&&`.** Test:
  `pairs/pair.astuple/get_const_rv.pass.cpp`.

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
- [ ] **`PR31384.pass.cpp`** exercises a libc++ tuple-like constructor
  extension. Its derived-tuple conversion-operator preference differs here
  with both psclang and host Clang.

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

### `<memory>` — util.smartptr 83/84 (host 84), smartptr 39/39 (host 39)
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
- [x] (Codex) **`shared_ptr(unique_ptr&&)`** fails at run time.
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
- [ ] **`util.smartptr.shared.io/io.pass.cpp`.** `ostringstream` construction
  calls an imported base constructor with virtual bases without its ABI VTT
  parameter. The failure also occurs without `shared_ptr`.
- [x] (Codex) **`unique.ptr.ctor/move.pass.cpp`.** Guarded local statics
  register their object address with `__cxa_atexit`, and register destruction
  before publishing completed initialization.
- [ ] **`<stdatomic.h>` and `std::atomic`:** the `__c11_atomic_*` and
  `__atomic_*` builtins (`c2w_builtin.cxx`). Implementing them makes
  `<atomic>` much simpler.
- [x] (Codex) **Static-storage `initializer_list`.** Namespace-scope lists
  receive runtime initialization for their backing array and extent.
  `compiler-lifetime.pass.cpp` covers this alongside object lifetime fixes.
- [ ] **`vector<bool>` iterator operations.** A probe that inserts two
  bits and exercises const iterators hits the field-access assertion in
  `c2w_expr.cxx`. The same probe fails with the C++98 iterator members
  on a const vector.

## Phase 2: containers

Add the C++11 members to stdcxx's own containers, under `_RWSTD_CXX11`:
- move constructor and move assignment (allocator propagation through
  `allocator_traits`);
- `initializer_list` constructor, assignment and `insert`;
- `emplace`, `emplace_back` and `emplace_front`, `emplace_hint`;
- `insert` and `push_*` from rvalues;
- `cbegin`/`cend`/`crbegin`/`crend` (done for `vector`,
  `vector<bool>`, and `list`);
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

- [ ] **`<iterator>`:**
  * [x] (Codex) `begin`, `end` and C++14 const and reverse range access
    (`iterator.range/begin-end.pass.cpp`);
  * `next`, `prev`, `move_iterator`, `make_move_iterator`;
  * C++14 `make_reverse_iterator`;
  * `iterators` (161 tests).
- [ ] **`<algorithm>`:**
  * `all_of`, `any_of`, `none_of`, `find_if_not`, `copy_if`, `copy_n`,
    `move`, `move_backward`;
  * `is_partitioned`, `partition_copy`, `partition_point`, `is_sorted`,
    `is_sorted_until`, `is_heap`, `is_heap_until`;
  * `minmax`, `minmax_element`, the `initializer_list` forms of
    `min`/`max`, `is_permutation`, `shuffle`;
  * C++14 `equal`/`mismatch` with four iterators;
  * `algorithms` (150 tests).
- [ ] **`<numeric>`:** `iota` (`numerics/numeric.ops`, 25 tests).
- [ ] **`<string>`:**
  * `to_string`, `to_wstring`, `stoi`/`stol`/`stoul`/`stoll`/`stoull`/
    `stof`/`stod`/`stold` (`strings/string.conversions`, 10 tests);
  * `hash<string>` and `hash<wstring>` are implemented.
  * `u16string`/`u32string`, `char_traits<char16_t>`
    and `char_traits<char32_t>` (`strings/char.traits`, 79 tests).
- [ ] **C library headers:**
  * `<cmath>`: the C++11 functions (`isnan`, `isinf`, `fpclassify`,
    `round`, `trunc`, `hypot`...) in `std` (`numerics/c.math`);
  * `<cstdint>`: everything in `std`;
  * `<cinttypes>`, `<cfenv>`, `<cuchar>`, `<ctgmath>`, `<cstdbool>`.
- [ ] **`<exception>`:** `exception_ptr`, `current_exception`,
  `rethrow_exception`, `make_exception_ptr`, `nested_exception`,
  `throw_with_nested`. These need libcxxrt (`language.support`).
- [ ] **`<new>`:** `get_new_handler`, `bad_array_new_length`.
- [ ] **`<ios>`, `<istream>`, `<ostream>`:**
  * rvalue stream insertion and extraction;
  * stream move and swap;
  * `io_errc`, `iostream_category`;
  * manipulators `get_money`, `put_money`, `get_time`, `put_time`,
    `hexfloat`, `defaultfloat`;
  * `input.output` (488 tests: try the ones for the features above).
- [ ] **`<bitset>`:** `to_ullong`, the `unsigned long long` constructor,
  `hash<bitset>` (`utilities/template.bitset`).

## Phase 4: new headers

| Header | libc++ tests | Notes |
|---|---|---|
| `<forward_list>` | `containers/sequences/forwardlist` (67) | |
| `<unordered_map>`, `<unordered_set>` | `containers/unord` (329) | needs `hash<string>` |
| `<ratio>` | `utilities/ratio` (13) | header-only, easy start |
| `<chrono>` | `utilities/time` (269) | clocks via `clock_gettime` (inline) |
| `<system_error>` | `diagnostics/syserr` (57) | categories as inline function-local statics |
| `<atomic>` | `atomics` (52) | `__sync` for now (psclang) |
| `<thread>`, `<mutex>`, `<condition_variable>` | `thread` (219) | pthreads; `call_once` |
| `<future>` | in `thread` | after the three above |
| `<random>` | `numerics/rand` (493) | engines and distributions, header-only |
| `<regex>` | `re` (167) | large; needs locale `regex_traits` |
| `<typeindex>` | `utilities/type.index` (10) | small |
| `<scoped_allocator>` | `utilities/allocator.adaptor` (30) | |
| `<codecvt>` | `localization/locale.stdcvt` (24) | `codecvt_utf8` and friends |
| `<cuchar>` | | |

## Infrastructure

- [ ] **Regular runs.** Run `lcxx.py` from a test target, or a nightly
  job, with a list of the tests expected to pass, so regressions show.
- [ ] **`-std=c++14` and `-O2` across the board.** The numbers above
  are C++11 at `-O0`.
- [ ] **IRIX (n32, big-endian).** The C++11 headers are untested there.
  The IRIX psclang target is still to come.
- [ ] **A C++11 `libstl`?** Whether something must go into a C++11-built
  part of the library (for example `random_device`, `thread` with a
  non-inline `hardware_concurrency`) or can stay inline.
