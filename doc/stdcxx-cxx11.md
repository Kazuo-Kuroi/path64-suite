# C++11 in stdcxx

The suite's C++ library is Apache stdcxx 5.0.0 (trunk). It is a C++98
library with a 2008 C++0x draft layer that is switched off and does not
build with a real C++11 compiler. This work adds the C++11 (and C++14)
library for the Clang front end. libc++ 8.0.1 (University of
Illinois/NCSA or MIT license; `compiler/stdcxx/LICENSE-libcxx.txt`) is
the reference for the design. Its test suite is the conformance test.

## Design

* **Gate.** `_RWSTD_CXX11` is defined when `__cplusplus >= 201103L` (psclang
  with `-std=c++11` and up; see `rw/_defs.h`). `_RWSTD_NO_CXX11` turns it
  off. In C++11 mode the 2008 draft layer is off (`_RWSTD_NO_EXT_CXX_0X`).
* **C++98 is unchanged.** The GCC front end, and psclang without
  `-std=c++11`, see the same headers as before. The library binary
  (`libstl`) is still built in C++98 and does not change.
* **Header-only.** The C++11 parts are templates and inline functions in
  `rw/_c11_*.h`, included by the standard headers in C++11 mode.
  `<type_traits>`, `<tuple>` and `<array>` switch to them;
  `<utility>`, `<functional>` and `<memory>` add them. libcxxrt also supplies
  the C++14 sized-deallocation entry points declared by `<new>`.
* `_RWSTD_CONSTEXPR14` is `constexpr` from C++14 on (C++11 makes
  `constexpr` member functions `const`).

## Status

| Header | C++11/14 content | libc++ 8 tests (C++11, -O0) |
|---|---|---|
| `<initializer_list>` | new | (used throughout) |
| `<type_traits>` | all of C++11, C++14 `is_null_pointer` and `_t` aliases, on Clang's builtins | meta: 123/123 (+1 XFAIL for clang-4) |
| `<utility>` | `move`, `forward`, `move_if_noexcept`, `swap` (move, arrays), `declval`, C++11 `pair` (N4387 constructors, piecewise, `get`), `exchange`, `integer_sequence` | utility: 41/41 |
| `<tuple>` | `tuple` (allocator constructors, conversions), `get`, `make_tuple`, `tie`, `forward_as_tuple`, `tuple_cat`, comparisons, `ignore` | tuple: 54/55 |
| `<functional>` | `reference_wrapper`, `ref`/`cref`, INVOKE, `function` (small buffer), `bind` and placeholders, `mem_fn`, `hash` (scalars, pointers, enums), typed bitwise and C++14 transparent functors | function.objects: 88/88 |
| `<memory>` | `pointer_traits`, `allocator_traits`, `addressof`, `align`, `unique_ptr` (and `[]`), `make_unique`, `shared_ptr`, `weak_ptr`, `make_shared`, `allocate_shared`, `enable_shared_from_this`, pointer casts, `owner_less`, `get_deleter`, shared-pointer atomics | util.smartptr: 84/84; allocator.traits: 21/21; pointer.traits: 9/9 |
| `<vector>` | Normal and packed-bool C++11 constructors, allocator propagation, moves, initializer lists, emplacement and modifiers; bool proxy swaps and hashing | vector: 51/54 (three raw-pointer-incompatible reference tests; see TODO); vector.bool: 42/42 (C++11), 44/44 (C++14) |
| `<deque>` | C++11 constructors, allocator propagation, moves, initializer lists, emplacement, modern modifiers and map compaction | deque: 61/61 |
| `<iterator>` | Range access, `next`, `prev`, move and reverse adapters/factories, SFINAE-safe traits | next/prev: 2/2; move.iterators: 29/29; reverse.iterators: 28/28 (C++11), 29/29 (C++14); iterator.primitives: 17/17 |
| `<algorithm>` | Predicate/move algorithms, counted/conditional copy, partition/sorted/heap queries, min/max additions, permutation queries, C++14 bounded equal/mismatch | Phase 3 partial coverage; see TODO and validation below |
| `<numeric>` | `iota`, move assignment in adjacent differences, safe inner-product iterator increments | numeric.ops: 9/9 applicable tests |
| `<array>` | `array` | array: 27/27 |
| `<cstddef>`, `<cstdint>` | `nullptr_t`, `max_align_t`; `<cstdint>` in C++11 mode | |

The libc++ tuple-extension test `PR31384` differs from stdcxx and the
system C++ library. The remaining C++11 library work includes
container extensions, remaining algorithms, streams and the new headers
listed in the TODO.

**What's left:** `doc/stdcxx-cxx11-TODO.md`, item by item, with the
test tools in `compiler/stdcxx/tests/cxx11`.

**Next phases:**
1. Containers: move, `initializer_list` and `emplace` members for `list`,
   `basic_string`, `map`/`set`.
2. Remaining `<iterator>` additions (including `make_reverse_iterator`),
   `<algorithm>` and `<numeric>` additions.
3. String conversions (`to_string`, `stoi`, ...) and Unicode string types.
4. New headers: `<forward_list>`, `<unordered_map>`, `<unordered_set>`,
   `<ratio>`, `<chrono>`, `<system_error>`, `<atomic>`, `<thread>`,
   `<mutex>`, `<condition_variable>`, `<random>`; later `<future>`,
   `<regex>`, `<codecvt>`, `<scoped_allocator>`, `<typeindex>`.

## Compiler bugs found on the way (psclang, fixed)

The library work turned up these clang2whirl bugs, most of them in
C++98 code as well:

* **`main` without `return`** returned garbage instead of 0.
* **Trivial copy and move constructors** were skipped:
  * in member initializers (`R(S& x) : s(x) {}` left `s` unset);
  * in `new T(t)`, including placement new.
* **Trivial copy inside an object.** A trivial copy into a member used
  the enclosing object's type.
* **Temporaries stored with the wrong type.** A temporary bound to a
  reference parameter was stored with its pointer type.
* **`&` of a reference** loaded one more time. This affected reference
  members, calls returning references and lambda captures.
* **Member operator calls on temporaries.** `C()(x)` passed the object's
  value as `this`.
* **Member pointers:**
  * a member function pointer could not initialize an aggregate member;
  * a member function pointer bound to a forwarding reference crashed.
* **`__builtin_addressof`** worked only on variables.
* **Constructor initialization state** leaked between empty bases and members.
* **Return expressions** read scalar values after local destructors ran.
* **Nontrivial value parameters** use invisible references through function
  signatures, ordinary calls and inherited constructor forwarding.
* **Member destructors** handle typedefs and specialized template types.
* **Conditional lifetimes** keep statement temporaries within their branch
  and construct class-valued alternatives into one destination.
* **Narrow enum casts** retain WHIRL's widened register types.
* **Nontrivial class results** use the caller's return area through direct,
  indirect and virtual calls, member pointers and adjusting thunks. This
  preserves self-referential objects and interoperation with host-compiled
  functions; a completed result must not be copied as raw aggregate bytes.
* **Cleanup sharing** compares destructor sequences without modifying them
  on a failed match, preserving reverse construction order on exceptional exits.
* **Optimized x86 exception unwinding** retains frame pointers in C++ functions
  with exceptions enabled, including helpers without local handlers. The
  existing frameless path lacks stack-adjustment CFI and could skip cleanups.
* **Returned class objects** are materialized before ordinary member calls.
  Inherited member operators on prvalues copy their base subobject's value
  into the object temporary.
* **Unbraced loop bodies** destroy expression temporaries on each iteration.
* **Empty-object pointer arithmetic** uses a nonzero stride while retaining
  empty-base layout optimization.
* **EH branch traversal** finds throwing calls after nonthrowing constructors,
  including exception objects thrown from an unbraced `if` body.
* **Class static data members** retain external linkage and ODR coalescing.
  This also fixes optimized string assignment across the runtime boundary.
* **Member calls through pointer prvalues** pass the allocated object address.
* **Namespace-scope initializer lists** receive runtime initialization.
* **Constructor exception cleanup** preserves argument temporary ownership.
* **Static-local destruction** registers the object address with `__cxa_atexit`.
* **Virtual-base construction** passes VTT arguments and initializes vptrs
  from construction vtables. Base casts use the active virtual-base offsets.
* **Nontrivial return values** use the hidden return pointer convention.
* **Out-of-line methods, vtables and thunks** export their ABI entry points.
* **Template static members** instantiate definitions through Sema.
* **Inline-function local statics and guards** coalesce across object files,
  so shared-pointer atomic operations share one lock pool. Inline function
  definitions also use weak external linkage.

`compiler/stdcxx/tests/cxx11/compiler-lifetime.pass.cpp` covers the reduced
lifetime, conditional construction, pointer call and enum regressions.
`compiler-virtual-base.pass.cpp` covers diamond inheritance, construction
dispatch, null and reference base casts, bit iterators and exact stream output.


Expanded x86-64 validation passes 488/489 tests in C++11 and 503/504 in
C++14 at both `-O0` and `-O2`. The remaining run failure is the libc++
tuple extension described above. Mixed GCC/psclang object files verify
nontrivial return values, value parameters, virtual-base construction and
thunks. Cross-file static initialization and threaded shared-pointer atomic
operations also pass.

Checking method: the same test is built with psclang and with the host
Clang over the same stdcxx headers and runtime. When the host build is
right and psclang's is wrong, the bug is in psclang.


Phase 3 adds the existing-header algorithm and numeric APIs listed above.
The original `phase3-algorithms.pass.cpp` checks min/max tie selection and
comparison complexity exhaustively on small sequences through forward
iterators, as well as counted copy, bounded comparisons and C++14 constant
evaluation. `shuffle` and reference tests that include `<random>` remain
pending Phase 4. Iterator coverage still lacks move insertion support and
has stream-iterator failures; legacy algorithms still need move-only value
support and protection against overloaded comma operators. These are not
marked complete in the TODO.

Phase 3 validation runs the full iterator, algorithm and numeric-operation
reference directories plus the original regression at both `-O0` and `-O2`.
C++11 passes 249/312
with psclang and 257/312 with host Clang; C++14 passes 250/313 and 258/313,
respectively. The eight psclang-only runtime failures are stream-buffer
iterator tests. Both compilers have 55 compile failures in each standard,
including missing `<random>` dependencies and unmarked C++17 tests;
25 C++11 and 24 C++14 tests are excluded by reference markers. The C++14
optimized foundations run remains 601/605 with its four previously recorded
reference incompatibilities. Shared and static C++98 library objects rebuild,
and all 11 legacy C++98/C++03 compiler probes pass at both optimization levels.
