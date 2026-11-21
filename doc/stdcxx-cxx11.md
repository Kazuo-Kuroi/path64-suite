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
| `<iterator>` | Range access, move/reverse adapters and factories, move insertion, SFINAE-safe traits, constexpr input-iterator defaults | iterator reference tests: 156/157 (C++11), 157/158 (C++14); remaining test needs list move insertion |
| `<algorithm>` | Predicate/move algorithms, copy additions, query algorithms, min/max additions, bounded comparisons, move-only compaction, rotation, sort, heaps, partial sort, stable partition, selection and stable sorting | 137/145 in C++14 at O0/O2; constexpr element queries, inplace_merge and C++17 clamp gaps remain |
| `<string>` | Numeric formatting/parsing, UTF-16/32 aliases and traits, string hashes | conversions: 10/10; char.traits: 64/64 applicable |
| `<numeric>` | `iota`, move assignment in adjacent differences, safe inner-product iterator increments | numeric.ops: 9/9 applicable tests |
| `<exception>` | Exception handles, current/make/rethrow operations, nested exceptions, handler getters | propagation/nested: 10/10; original ownership/catch checks: 2/2 |
| `<array>` | `array` | array: 27/27 |
| `<cstddef>`, `<cstdint>` | `nullptr_t`, `max_align_t`; `<cstdint>` in C++11 mode | |
| `<cmath>` | C99 functions and suffixed forms, arithmetic overloads, boolean classification and comparison predicates | c.math: 3/3 |
| `<cinttypes>`, `<cfenv>`, `<cuchar>`, `<ctgmath>`, `<cstdbool>` | Native C interfaces imported into `std`, type-generic math/complex access, boolean compatibility | remaining C-header reference checks: 5/5 (including cstdint) |

New partial headers are `<random>` (engines, adapters, entropy, uniform,
Bernoulli, exponential, normal and lognormal sampling) and `<regex>` (traits,
captures/results and an initial ECMAScript search/match kernel). Remaining
distributions, other regex grammars, lookahead, iterators and replacement are
still open; these headers do not claim whole-header conformance.

The libc++ tuple-extension test `PR31384` differs from stdcxx and the
system C++ library. The remaining C++11 library work includes
container extensions, remaining algorithms, streams and the new headers
listed in the TODO.

**What's left:** `doc/stdcxx-cxx11-TODO.md`, item by item, with the
test tools in `compiler/stdcxx/tests/cxx11`.

**Next phases:**
1. Containers: move, `initializer_list` and `emplace` members for `list`,
   `basic_string`, `map`/`set`.
2. Remaining algorithm dependencies (`<random>`) and list-backed insertion coverage.
3. Streams and bitset additions. Exception handles and nested exceptions are implemented.
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
pending Phase 4. Insertion adapters accept moved values, input iterators
have constexpr default constructors, and stream-buffer iterator traits are
corrected. The remaining insertion test depends on unfinished C++11 `list`
support. `remove`, `remove_if`, `unique` and `rotate` accept move-only values;
`rotate` returns the required iterator. Transform, replacement, swap, lexical
comparison and permutation loops avoid user-defined comma operators.
Sorting, heap operations and partial sorting move values through their helpers;
the sort pivot remains in the range until partitioning completes. Temporary
heap values use iterator value types rather than proxy aliases. `iter_swap`
finds ADL swaps for proxy references and otherwise preserves a value.
Stable partition owns raw temporary storage and its successfully constructed
prefix, reuses partial buffers recursively and falls back to rotation when no
buffer is available. `nth_element` keeps its pivot in the range and partitions
less/equal/greater keys separately, supporting move-only values and packed-bit
proxies without repeatedly scanning equal keys. `stable_sort` shares the owned
raw-buffer helper, moves the left merge run into available storage, and uses
rotation-based merging when the buffer is too small or allocation fails.
Left-run ties preserve stable order. Successfully constructed buffered values
are destroyed even when a move or comparison throws. Its reference suite and
the selection reference suite require the missing `<random>` header.
`stable-sorting.pass.cpp` supplies original coverage for both comparison
directions, partial/failed allocations, packed bits, object identity, stable
ordering, comparison bounds and resource cleanup on exceptions. It passes
with both compilers in C++11/14 at `-O0` and `-O2`.

The original `nth-element.pass.cpp` covers every selection position on small
inputs, descending comparisons, duplicate-heavy inputs, object identity,
packed-bit proxies and cleanup after comparison exceptions. It also exposed
psclang skipping loop-body destructors on `continue`. psclang now emits the
exited scopes' destructors before both `break` and `continue`, preserving
fallthrough paths and enclosing objects. `compiler-loop-cleanup.pass.cpp`
checks reverse destruction order across nested blocks, loops, switches and
range-for. Both new tests pass with psclang and the host compiler in C++11 and
C++14 at `-O0` and `-O2`.

The updated iterator/algorithm/numeric batch plus all original regressions
passes 299/330 selected tests in C++11 at `-O0` and 301/332 in C++14 at `-O2`.
There are no runtime failures: the same 31 compile failures remain (28 missing
`<random>`, two unmarked C++17 clamp tests and one list move-insertion test).
All original regressions pass at both levels: 19 applicable in C++11 and 20
in C++14. The loop cleanup regression and legacy sorting/partition probe also
pass in C++98 at `-O0` and `-O2`.

psclang keeps materialization statements inside each short-circuit operand.
Conditional class temporaries have guards for operand selection and successful
construction; normal and unwind cleanup respect those guards. The original
`compiler-short-circuit.pass.cpp` checks skipped and nested operands, null
pointers, full-expression lifetime and throwing constructors/consumers.
The eight previously failing stream-buffer iterator runtime tests now pass.
`phase3-move-iterators.pass.cpp` checks all insertion adapters with move-only
deque elements and the C++11 rotate return iterator. `<cstdio>` stops importing
`gets` in C++14, where the C library declaration is removed.

Phase 3 validation runs the full iterator, algorithm and numeric-operation
reference directories and eight original library/compiler regressions.
Both compilers pass 288/319 in C++11 and 289/320 in C++14 at `-O0` and
`-O2`, with no runtime failures in this batch. The 31 compile failures are
classified in the TODO. The broader C++14 `-O2` foundations
run, including all fourteen original passing regressions, passes 613/617:
three vector tests assume class iterators and the tuple extension test
`PR31384` differs from stdcxx and the system library. The packed-bool and move
iterator follow-up passes 74/74 at `-O2`. Shared and static C++98 library
objects rebuild, and all 11 legacy C++98/C++03 probes pass at both optimization
levels.


The sorting work also fixes psclang materialization into member addresses:
register-returned trivial class values are evaluated and stored at the member's
address, including nonzero offsets. Scalar pseudo-destructors and trivial
class destructors evaluate their object expression even though no destructor
call is needed. `compiler-member-return.pass.cpp` covers both reductions.
Constructor initializer calls now have unwind regions even before the ordinary
body scope. Each region snapshots the completed record members and bases;
escaping exceptions destroy that prefix in reverse order. Catches within the
constructor retain its members, references do not own their referents, and
initializer argument temporaries survive the consuming call. Construction must
succeed before its temporary is registered for destruction. The formerly
failing `compiler-member-init-unwind.pass.cpp` and the expanded
`compiler-constructor-cleanup.pass.cpp` cover these paths.

`<new>` exposes `get_new_handler` and `bad_array_new_length`. libcxxrt serializes
handler access with boolean CAS and explicit barriers, copies the callback
under the lock, and invokes it after unlocking. This also avoids the existing
MIPS value-CAS result bug and unsupported lock-release intrinsic. The runtime
supplies the new exception's constructors, assignment, destructor, RTTI and
`what()`. psclang now checks C++11 array bounds before allocation, including
payload/cookie limits and initializer-list counts. Invalid bounds select
`bad_array_new_length` for throwing allocation, or null for nonthrowing
allocation. Null allocation results skip cookie writes and initialization.
The new `compiler-array-length.pass.cpp` checks these paths, bound evaluation,
valid allocations and ordinary allocation failure. All eighteen original
regressions pass with psclang in C++14 at `-O0` and `-O2`; seventeen apply in
C++11 and pass at both levels. The runtime helper also cross-builds for IRIX.

The six allocation-error reference tests plus all seventeen original regressions
pass 22/22 in C++11 and 23/23 in C++14 (sized deletion is C++14-only), with both
compilers at `-O0` and `-O2`. The broader C++14 optimized foundations batch stays
599/603 without original tests, with the same four reference incompatibilities.
Shared and static libcxxrt rebuild on Linux and cross-build for big-endian
IRIX n32/MIPS IV. No new IRIX hardware execution is claimed.

## C library header checkpoint

The C++11 `<cmath>` additions live in `rw/_c11_cmath.h`; the C++98 overloads
remain unchanged. The new overloads preserve float and long-double return
types and promote integral/mixed arithmetic arguments as required. Constraints
keep unrelated user-defined types available for ADL. Classification predicates
return `bool`, include integral and long-double arguments, and use the compiler's
classification/sign builtins. Ordered comparisons first exclude NaNs, avoiding
the backend's unfinished C comparison intrinsics and preserving floating-point
flags for quiet NaNs. The reference cmath test includes overload/type checks.

`<cinttypes>`, `<cfenv>` and `<cuchar>` import libc's types and functions, while `<ctgmath>`
includes the math and complex interfaces and `<cstdbool>` provides the C++11
compatibility macro. `<cstdint>` already uses the native C types on the tested
configuration and passes the reference test. `c-library.pass.cpp` additionally
checks integer formatting/parsing and type identity, mixed overloads, ADL,
negative zero, subnormals, long-double classification, rounding modes, exception
flags and stateful UTF-16/32 conversions. Supplementary Unicode conversions
run when the host supplies the `C.UTF-8` locale; ASCII conversion is unconditional.

All eight selected C-header reference tests plus the original stable-sort and
C-header tests pass 10/10 with psclang and host Clang in C++11/14 at `-O0` and
`-O2`. All 200 shared/static C++98 library objects rebuild, and a legacy
stable-sort/math probe passes with GCC, psclang and host Clang at both levels.
The combined iterator/algorithm/numeric/C-header batch and all original
regressions pass 309/340 selected tests in C++11 at `-O0` and 311/342 in C++14
at `-O2`. The 31 compile failures are unchanged (missing random support, two
C++17 clamp tests and list move insertion), with no runtime failures.
These are Linux x86-64 results. IRIX's C++ library port still needs configuration
and execution coverage; its sysroot lacks `<uchar.h>`, and several fenv
functions use older `void` return signatures requiring adapters.

The integration work also exposed a psclang/library ABI problem with
namespaced external numeric-limits constants. It is now fixed by the variable
mangling change described below; `compiler-limits-constants.pass.cpp` is the
promoted reproducer. C math `nan` and `nextafter` work independently of those
constants.

## Numeric string conversion checkpoint

`rw/_c11_string_conversions.h` supplies all nine `to_string`/`to_wstring`
overloads and the narrow/wide `stoi`, `stol`, `stoul`, `stoll`, `stoull`,
`stof`, `stod` and `stold` interfaces. Parsing uses the matching libc function,
checks `ERANGE` and any narrower `int` range, and distinguishes invalid input
from out-of-range values. A scope guard restores the caller's errno on success
or exception; consumed positions change only after successful conversion.
Integer bases, locale rules, floating special values and signed zero follow libc.

Formatting uses the specified printf conversion forms, starts with a small
buffer and grows storage when necessary. The wide path handles libc returning
a negative value rather than the required size for an undersized buffer.
Maximum double and long-double fixed-point output is checked against libc.
The string and C-header additions are selected only in C++11/14 mode.

The ten reference conversion tests and `string-conversions.pass.cpp` pass 11/11
with psclang and host Clang in C++11 and C++14 at `-O0` and `-O2`. The original
checks cover signed/unsigned limits, range errors, unchanged error positions,
errno restoration, bases, embedded NULs, infinities, NaNs, signed zero and
large narrow/wide output. The follow-up `<cstdio>`/`<cstdlib>` exports also
pass both reference header tests across that matrix: bounded and variadic
formatting/scanning, C numeric conversions, long-long absolute value/division,
and `_Exit`. Unicode string aliases/traits are covered by the following checkpoint.

The combined C++14 `-O2` foundations/conversion/original-regression run is
629/633: three raw-pointer-incompatible vector reference tests still fail to
compile, and the unchanged tuple extension `PR31384` differs at runtime. All
23 original regressions pass in C++14 at both levels; 22 apply in C++11 and
pass at both levels. The eleven C++98/C++03 language probes remain green at
`-O0` and `-O2`.

## Unicode string checkpoint

The C++11 layer now supplies `u16string` and `u32string`, corresponding hashes,
dedicated `char_traits<char16_t>`/`char_traits<char32_t>` specializations and
`u16streampos`/`u32streampos`. The traits use `uint_least16_t`/`uint_least32_t`
as their integer types and compare numeric code-unit values rather than raw
byte order. Bounded search returns without inspecting an element after the
range, including null pointers with zero length. Memory moves support overlap.
Narrow, wide and Unicode scalar trait operations support C++11 constant
expressions; the new internal constexpr macro expands to nothing in C++98.

All 64 applicable character-trait reference tests pass; 15 C++20 char8_t tests
are excluded. With the ten numeric-conversion reference tests and
`unicode-strings.pass.cpp`, the focused batch passes 75/75 with psclang and
host Clang in C++11/14 at `-O0` and `-O2`. The original test checks position and
integer types, constant expressions, numeric ordering, overlapping moves,
zero-length operations and searches adjacent to a protected memory page.
String checks cover UTF-16 surrogate pairs, UTF-32 code points, embedded NULs,
noncharacter code units, copies/mutation, modifiers, reserve/resize and hashes.
These aliases store code units; they do not perform Unicode transcoding.

All 200 shared/static C++98 library objects rebuild. These results are from
Linux x86-64; IRIX C++ library configuration and execution remain separate work.
The Phase 3 string feature list is complete; Phase 2 basic_string
move/emplacement modernization is still open.

## Variable linkage checkpoint

psclang previously inferred variable mangling from the immediate declaration
context. An `extern "C++"` block inside a namespace hid that namespace, so
references to the GCC-built library's `__rw` numeric-limits constants used
unmangled names. Two namespaces declaring the same variable name inside these
blocks could also emit duplicate assembler symbols.

Variables with global storage now use Clang's ABI mangling decision. This
preserves C linkage and handles namespaced variables, static data members and
static locals, while ordinary function/template locals retain their local names.
The old failing limits test is now `compiler-limits-constants.pass.cpp`.
`compiler-variable-linkage.pass.cpp` checks distinct namespace variables,
constant data, shared C linkage, static members, guarded local initialization
and template-local values. Its provider/consumer build modes support separate
translation units produced by different compilers.

The linkage/Unicode/limits checks pass 3/3 with psclang and host Clang in
C++11/14 at `-O0` and `-O2`. All original regressions pass at both levels:
25 applicable in C++11 and 26 in C++14. C++98 linkage/limits/sorting checks
pass 3/3 at both levels, and all eleven C++98/C++03 language probes remain
green. Separately compiled psclang/host-Clang and psclang/GCC provider/consumer
objects link and run in both directions at `-O0` and `-O2` (eight combinations).
The combined C++14 `-O2` foundations/strings/traits/original-regression batch
passes 696/700 selected checks, with the same three raw-pointer-incompatible
vector compile failures and tuple-extension `PR31384` runtime difference.
There are 69 reference-marker exclusions and one expected failure.

## Catch-parameter and handler-exit checkpoint

psclang now invokes a nontrivial catch parameter's copy constructor instead
of copying its bytes. The parameter belongs to the handler's compound scope,
including empty handlers, and is destroyed after body locals. Temporary default
arguments to the copy constructor finish before the handler body begins. A
failure during parameter initialization enters a termination cleanup rather
than propagating to another handler.

Return, break and continue now end the catches they leave, interleaved with
lexical object destruction. Return values are evaluated before that cleanup.
This fixes a retained exception when returning an exception handle from a catch.
`compiler-catch-cleanup.pass.cpp` is an original C++98/11/14 regression for
copy/destruction counts, cleanup order, scalar and void returns, loop exits,
empty handlers, throwing parameter destructors, default arguments and the
required termination when copying a catch parameter throws.

## Exception handle and nesting checkpoint

`rw/_c11_exception.h` implements a pointer-sized `exception_ptr` with shared
primary-exception ownership, nullable comparisons, assignment and swap.
`current_exception` adopts the owned reference returned by libcxxrt;
copying retains a reference, and destruction releases it. `rethrow_exception`
uses the runtime's dependent-exception path, preserving object identity while
each throw owns its own unwind state. The runtime helper previously treated
an exception object pointer as an unwind-header pointer; it now locates the
primary header by the actual object layout. `make_exception_ptr` captures
failures thrown while copying the requested exception too.

`nested_exception` captures the current exception, supports shared copies and
terminates when asked to rethrow an empty handle. `throw_with_nested` wraps
eligible class types, preserving final types and existing nested-exception
bases. Its final-class query works in C++11 as well as C++14.
`rethrow_if_nested` excludes inaccessible/ambiguous bases, uses `addressof`
and checks polymorphic objects through RTTI. The header also exposes the
runtime's existing terminate/unexpected-handler getters. Header inclusion and
empty-handle use compile with `-fno-exceptions` in C++11/14.

The ten propagation/nested reference tests and two original regressions pass
12/12 with psclang and host Clang in C++11/14 at `-O0` and `-O2`.
`exception-handles.pass.cpp` checks ownership beyond a handler, cross-thread
copies/rethrows and identity, catch-copy independence, returns and loop exits,
throwing exception copies, nesting a runtime_error and required termination
for empty nesting or failed catch-parameter initialization. The full exception
reference directory passes 18/19 selected tests; the remaining unmarked test
requires the C++17 `uncaught_exceptions` counter API.

All 28 original regressions pass in C++14 and all 27 applicable ones pass in
C++11 at both optimization levels. The C++98 catch/loop/sorting checks pass
3/3 with both compilers at both levels, and all eleven C++98/C++03 language
probes remain green. Linux shared/static libcxxrt and all 200 C++98 stdcxx
objects rebuild; shared/static libcxxrt cross-build for IRIX n32 also succeeds.
These exception tests execute on Linux x86-64; no IRIX C++11 execution is claimed.

The combined C++14 `-O2` foundations/strings/traits/exceptions/original-regression
run passes 708/712 selected checks. The three vector reference compile failures
and tuple-extension PR31384 runtime difference are unchanged, with 69 reference
marker exclusions and one expected failure.


### Bitset and stream checkpoint

Bitset now provides the C++11 constexpr constructors, bounded character-pointer
constructor, `to_ullong`, `all` and hashing. Empty instances and shifts at or
beyond the logical size are handled in C++98 too. All 38 approved reference
checks pass with host Clang and psclang at `-O0`/`-O2`. The optimized mask
regression is fixed by limiting legacy signed-constant normalization to
32-bit operations; canonical constants also represent 64-bit results.

Streams support constrained rvalue insertion/extraction and the six money,
time and floating-format manipulators. Six reference checks and the original
integration test pass in C++11/14 at both optimization levels with both
compilers. Four locale-dependent references are excluded by the runner.
Error categories remain pending; the subsequent move/swap checkpoint is below. Hexadecimal floating output
uses the existing shared formatter: both float flags select `%a`/`%A` and
precision is ignored. This also changes that explicit flag combination for
C++98 callers of the shared library; ordinary formats retain their behavior.

This work also fixes x86-64 return-value lowering: only the low byte or word
is defined for narrow integral returns from native callees. Sixteen mixed
provider/consumer checks pass across GCC, host Clang and both PathScale
frontends at `-O0`/`-O2`. The IRIX backend builds with this guarded change;
these execution results are Linux x86-64 only. The broad C++14 `-O2` checkpoint
has 754/759 passes, 73 unsupported and one expected failure: the known three
vector reference incompatibilities, tuple extension mismatch, and the new
mask-expression compiler issue account for the five failures.

The subsequent wide-mask fix clears the fifth failure above: all 38 bitset
references now pass at psclang `-O2`. `compiler-wide-mask.pass.cpp` covers
the isolated optimizer regression in C++98/11/14 at three optimization levels.


### Stream move and swap checkpoint

Protected moves and swaps now cover `basic_ios`, input/output streams and the
shared virtual base of `basic_iostream`. Formatting, state, exception masks,
fill, ties, callbacks and user storage transfer without invoking callbacks.
Stream buffers expose protected copying and swapping. String/file buffers and
all six owning stream classes provide move construction, move assignment and
member/nonmember swap. Buffer allocations, get/put positions, file positions,
conversion state and caller-owned buffers travel with the resource; each
stream retains its own `rdbuf()` object. File-buffer assignment closes the
previous destination file before transferring ownership.

C++11 callers instantiate the affected classes instead of using the C++98
`extern template` declarations, which otherwise suppress emission of new
members absent from the shared library. Class layouts are unchanged. Both
C++98 shared/static libraries have been rebuilt. The older read/write
string-buffer constructor also incorrectly placed output at the end without
`ate`/`app`; the corrected positioning has C++98/C++14 narrow/wide checks.

The three original move suites and wide-mask regression pass with psclang and
host Clang in C++11/C++14 at `-O0`/`-O2`. The latest complete PathScale C++14
`-O2` original suite passes 35/35; the additional positioning regression passes
separately in both compiler modes. Eleven legacy C++98/03 probes still pass at
both optimization levels, and the IRIX backend builds with the mask fix.
The full host-original suite retains its compiler-array-length incompatibility;
its new stream tests pass.

Expanded base/string references give 49/62 passes with both compilers at
C++14 `-O2`, plus eight locale exclusions. Twelve assertions assume default
`flags()` contains no implementation bits; this library exposes extra legacy
bits. One allocator test needs a non-default-constructible allocator, which
string buffers do not yet preserve. Four ifstream move/swap references use
supplied input fixtures. Four file-buffer move/swap references cannot compile
their support header until `<codecvt>` exists. These limits remain in the TODO;
stream error categories depend on the forthcoming `<system_error>` header.


### Stream compatibility follow-up

The subsequent base/string/formatting reference checkpoint passes 71/71 with
both compilers at C++14 `-O2`, with eight locale exclusions. This clears the
allocator compile failure and twelve flag assertions recorded above.

Public `flags()` and flag-setting return values omit the cached numeric radix
and stdio synchronization marker. Numeric facets and internal manipulators
use an explicit internal accessor. Ordinary streams now start with exactly
`skipws|dec`, with locking enabled; callers can still explicitly request the
existing `nolock`/`nolockbuf` extensions. The stdio synchronization marker stays
attached to the stream when formatting is changed. C++98/C++11/C++14 tests cover
public flag round trips, decimal/octal/hex output, automatic-base input, radix-3
extensions and preservation of stdio synchronization.

String buffers retain the allocator supplied by the initial string and use it
for growth, destruction, external-buffer replacement and returned strings.
Moves and swaps keep allocations paired with a compatible allocator. Moving
between unequal nonpropagating allocators copies into destination-owned storage,
including get/put positions and locale. Failed growth or replacement preserves
the old contents. An empty allocator-storage specialization retains the measured
128-byte default char/wchar string-buffer and 256-byte string-stream layouts on
Linux x86-64. The C++98 shared/static libraries are rebuilt with these changes.
Original tests track allocation owners/counts and require all storage to be
released, including propagation, unequal allocator moves, narrow/wide streams,
caller-owned buffers and allocation failures.

The latest full PathScale original C++14 `-O2` run passes 38/38. New flag and
allocator checks pass with both compilers in C++11/14 at `-O0`/`-O2`; flags and
positioning also pass in C++98. All eleven legacy C++98/03 probes pass at both
optimization levels. A separate numeric-facet review exposed remaining iterator
comma, psclang constructor-expression, string-back and unsigned-overflow-state
issues; its 2/16 result is recorded in the TODO. `<codecvt>` and the
`<system_error>` dependency for stream error categories remain pending.


### Numeric facets and Unicode conversion

All sixteen numeric-facet references now pass with both compilers at C++14
`-O2`. The iterator loop uses builtin comma; psclang normalizes parenthesized
member callees consistently before lowering. Narrow unsigned extraction checks
the magnitude before applying negation at destination width and reports range
failure on saturation. Failed conversions store zero. Numeric bool conversion
sets a value on failure, and named booleans report EOF consistently. Clearing
basefield bits also invalidates the cached radix used for autodetection.

Floating input accepts hexadecimal exponents and infinity/NaN forms. Digit and
grouping buffers grow beyond their old fixed limits; grouping errors consume
the token and retain the converted value while setting failbit. The original
regression covers generic iterators with deleted comma, integer boundaries,
malformed grouping, long tokens, bool errors, float forms and string COW
endpoint mutation. These shared parsing fixes apply to C++98 callers too.
C++11 string `front()`/`back()` use the existing detaching element access.

The new original `<codecvt>` supplies UTF-8/UCS, UTF-16-byte/UCS, and
UTF-8/UTF-16-code-unit facets, with Maxcode limits, BOM modes, byte order,
surrogate validation, partial input/output, length and unshift. Complete scalars
are committed together; truncated or invalid input leaves the scalar's first
byte unconsumed. All 24 approved codecvt references pass with both compilers.
Original checks cover scalar boundaries, malformed/overlong UTF-8, partial
buffers, surrogate units, little-endian BOMs and a wide-file UTF-8 round trip.

The header also clears the file-buffer test-support dependency. File buffers
and owning file streams accept C++11 std::string filenames; all twelve selected
file-buffer/ofstream/fstream move/swap references pass. Together with numeric
and codecvt references and three originals, the checkpoint is 55/55 with both
compilers at C++14 `-O2`. New originals pass in C++11/14 at `-O0`/`-O2`; the
PathScale original suite passes 40 tests in C++11 and 41 in C++14. Parenthesized
calls and stream flags also pass in C++98, and the eleven legacy probes pass
at both levels. Libraries and the frontend have been rebuilt. IRIX backend
builds; Unicode/file execution is checked only on Linux x86-64.

The subsequent system-error/category checkpoint is below.


### System-error and stream-category checkpoint

The original `<system_error>` implements `errc`, error enum traits,
`error_category`, `error_code`, `error_condition`, factories, comparisons,
stream insertion, hashes and `system_error`. Generic/system category singletons
map available POSIX errors and preserve errno while forming messages. On Linux,
FreeBSD and Solaris, message conversion uses the available strerror_r ABI;
other platforms use the C library strerror fallback. Category identity is shared
across translation units. Stream headers add `io_errc`, `iostream_category`
and the corresponding trait/factories.

Concrete standard categories have explicit runtime constructors. This keeps
coalesced singleton objects and guard variables consistent between native Clang
and PathScale while psclang lacks static vptr initialization for some constexpr
polymorphic objects. The broader limitation remains in the TODO. The new header
also exposed virtual lowering bugs: vtable indices now use canonical method
declarations, calls through returned references dispatch virtually, and the
implicit object expression is evaluated once.

All 56 applicable system-error references and four stream-category references
pass with both compilers at C++14 `-O2`; the trait-variable reference is C++17.
Together with the stream/numeric/Unicode batch, the checkpoint is 171/171 with
nine exclusions. Original tests check custom ADL enums, equivalence in both
directions, messages, errno, hashing, threaded singleton use and exceptions.
Four mixed-compiler checks confirm shared category identity, returned codes
and thrown system_error objects at `-O0`/`-O2`. New originals pass in C++11/14
with both compilers at both levels; PathScale's full original suite passes
42 tests in C++11 and 43 in C++14. The virtual-call check also passes in C++98,
and exception-free C++11 header use passes with both compilers at both levels.
Eleven legacy C++98/03 probes remain clear. Execution coverage is Linux x86-64.

The modern `ios_base::failure` interface is still pending. Shared C++98 library
code throws `__rw_failure` and its existing subclasses; those objects currently
have the legacy base hierarchy and layout. Changing the header typedef alone
would make modern catches disagree with shared-library throws. Its system_error
base, stored code and code-aware constructors need a coordinated runtime/ABI
transition. The two modern failure references remain outside the passing
checkpoint. System_error also inherits the legacy runtime_error copy guarantees;
allocation-free/noexcept exception copying belongs in that transition audit.

### Guarded initialization and legacy atomic checkpoint (2026-11-20)

The later stream-failure ABI work supersedes the pending items in the preceding
historical checkpoint; see `stdcxx-cxx11-TODO.md` for the current status.
Throwing local-static initializers now unwind initializer locals/temporaries,
abort the guard, and allow waiting threads or later calls to retry. Successful
full-expression cleanup precedes guard release and object destructor
registration. Nested guards, catches/rethrows, static lambda captures and
mixed host/psclang inline statics have original regression coverage.

Discarded atomic results no longer make code expansion treat the following
EH region as an expression. x86 bitwise/exchange operations retain their
32-/64-bit width when their result is unused. Compiler barriers around legacy
`__sync` operations prevent optimized post-atomic reads from reusing earlier
memory values. The original atomic regression passes C++98/11/14 at `-O0`/`-O2`,
and the threaded initializer reproducer passes 50 consecutive executions.
The system-error/smart-pointer references pass 140/140 applicable tests in
C++14 at both optimization levels (three later-standard exclusions).
The full original regression target passes 54/54 in C++11 and 55/55 in C++14
at both optimization levels. The IRIX backend rebuilds with the shared fix.
This does not implement the remaining Phase 4 `<atomic>` header or complete
native ordered-atomic lowering. Execution coverage is Linux x86-64.

### Random and regex foundation checkpoint (2026-11-20)

The original header-only random implementation adds seed_seq, minstd/MT/ranlux
base engines, stream state, unbiased uniform sampling, canonical real generation,
Bernoulli trials and shuffle. Overflow-safe modular multiplication supports
full-width congruential parameters. Engine/seed/canonical references pass 45/45
with psclang in C++14 at both levels. The selected random and regex foundations
pass 106/110: two missing-adapter distribution compile failures, one missing
legacy ctype_base::blank compile failure, and a regex_error diagnostic-text
difference remain recorded, alongside seven locale exclusions. The validated
subset plus two originals passes 108/108 with both compilers at C++14 O0/O2.
Original tests
check full signed/unsigned ranges, narrow/nonzero-minimum generators, rejected
samples, state round trips, shuffling and custom locale facets.

Adding random unblocks algorithm references: 137/145 now pass at both levels,
up from 115/145. Remaining failures cover C++14 constexpr element queries,
move-only/self-move inplace_merge and C++17 clamp references. The original
suite passes 56/56 in C++11 and 57/57 in C++14 at both levels; both new originals
also pass with host Clang across that matrix. C++98 probes and both runtime
library targets remain clear. Runtime execution is Linux x86-64.

Random adapters, random_device, other distributions and the regex parser,
matching/results/iteration/replacement APIs remain separate TODO work. The
regex traits provide portable named characters and locale facets; full primary
collation and locale-specific multicharacter elements remain open.

### Random adapters and regex capture ranges (2026-11-20)

The header-only random implementation now includes discard-block,
independent-bits and shuffle-order engines, plus ranlux24, ranlux48 and knuth_b.
Shuffle-order indexing computes its scaled integer quotient exactly, including
full-width generators, without overflowing a product or using floating point.
The independent-bits engine implements the standard draw partition/rejection
sequence. Adapter serialization saves/restores decimal stream grammar and
formatting; failed or throwing extraction preserves the prior engine state.
The two previously blocked uniform-distribution evaluation references compile
and pass with the new aliases.

Regex capture ranges now provide the sub_match API and narrow/wide aliases,
including constexpr defaults, length/string conversion, comparisons against
captures/strings/pointers/characters and stream insertion. Unmatched ranges
behave as empty strings. Capture ranges do not implement match_results or
regex parsing/matching; the locale blank-mask gap also remains open.
The adapter/predefined-engine/uniform and capture reference batch passes
100/100 with both compilers in C++14 at O0/O2; its adapter/predefined-engine/
capture subset passes 63/63 with both compilers in C++11 at both levels.
Original checks add full-width arithmetic, narrow-generator rejection, unusual
stream formatting, extraction rollback, embedded NULs and list iterators.

The original matrix passes 58/58 in C++11 and 59/59 in C++14 at both levels.
An initial stream-flags compiler crash in C++14 O2 did not recur in four
isolated runs or the full affected-configuration rerun; neither changed header
is included by that test. This observation remains recorded in the TODO.

### Entropy, stateful sampling and initial regex matching (2026-11-20)

Original header-only additions include POSIX random_device and exponential,
normal and lognormal distributions. The entropy source accepts /dev/urandom,
owns a close-on-exec descriptor, retries interrupted/short reads and throws
system_error on open/read failure. It reports an unknown entropy estimate
(zero) and supplies no deterministic substitute. Cached normal samples survive
copying/serialization; reset clears them. Distribution extraction preserves
prior state on failure and restores formatting/precision across exceptions.

The original regex instruction compiler and ordered VM now support a subset
of ECMAScript: classes/ranges, shorthands, literals/escapes, dot, anchors, word
boundaries, alternatives, captures/noncapturing groups, backreferences and
bounded/unbounded greedy/lazy repeats. Empty iterations terminate; explicit
pattern/code/work/saved-state limits report regex_error. Match/search fill
allocator-aware match_results, preserving iterator-based captures, prefixes,
suffixes and positions; results format default and sed replacement strings.
Failed pattern assignment preserves the prior program, and moved-from regex
objects remain usable. Other grammars, lookahead, iterators and regex_replace
remain open, as do complete syntax/collation conformance. No whole-header
conformance is claimed. Element positions follow the distance-to-first-iterator
rule in the [element-access specification](https://eel.is/c++draft/re.results.acc).

The entropy/distribution references pass 57/57. The validated combined subset
passes 111/111 with both compilers in C++14 at O0/O2, with nine exclusions.
The new originals, including the explicit exception-disabled probe, pass with
both compilers in C++11/14 at both levels. A broad
80-test regex constructor/results/algorithm sample records remaining failures;
some reference files have disabled bodies, so counts do not prove grammar
support. Original checks cover short/interrupted/error reads, fd zero, cached
samples, stream rollback, captures/formatting, byte/wide/list input and resource
exhaustion. Execution coverage remains Linux x86-64.

The work also fixes psclang memory-reading builtin lowering: memcmp/strcmp/
strlen require memory-reference dependencies through intrinsic calls, and
intrinsic calls need conservative effect flags. The reduced optimized memcmp
failure and subsequent string/memory writes have an original regression.
Optimized host regex error cleanup requires frame pointers with the bundled
unwinder; hostcc.sh now retains them, as the PathScale driver already does.
This does not fix the underlying frameless-unwind limitation.

Exception-disabled library use exposed two more mismatches: psclang was lowering
header catch handlers despite absent exception tables, and the driver did not
forward exception/RTTI flags to preprocessing. Disabled-EH try lowering now
keeps only normal body scopes, and macro-changing options reach both phases.
The original exception-disabled source runs automatically using PATH64_FLAGS
metadata; runner self-checks verify that flags are honored.

The final original target passes 62/62 in C++11 and 63/63 in C++14 at both
optimization levels. All four runner self-checks pass. The explicit disabled-EH
regression passes with both compilers across the language/optimization matrix;
C++98 memory-intrinsic probes pass with psclang, host Clang and the GCC frontend.
The IRIX driver rebuilds; IRIX C++11 runtime execution remains unvalidated.
