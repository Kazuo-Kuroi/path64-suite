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
  `<utility>`, `<functional>` and `<memory>` add them.
* `_RWSTD_CONSTEXPR14` is `constexpr` from C++14 on (C++11 makes
  `constexpr` member functions `const`).

## Status (phase 1)

| Header | C++11/14 content | libc++ 8 tests (C++11, -O0) |
|---|---|---|
| `<initializer_list>` | new | (used throughout) |
| `<type_traits>` | all of C++11 and the C++14 `_t` aliases, on Clang's builtins | meta: 123/123 (+1 XFAIL for clang-4) |
| `<utility>` | `move`, `forward`, `move_if_noexcept`, `swap` (move, arrays), `declval`, C++11 `pair` (N4387 constructors, piecewise, `get`), `exchange`, `integer_sequence` | utility: 41/41 |
| `<tuple>` | `tuple` (allocator constructors, conversions), `get`, `make_tuple`, `tie`, `forward_as_tuple`, `tuple_cat`, comparisons, `ignore` | tuple: 51/55 |
| `<functional>` | `reference_wrapper`, `ref`/`cref`, INVOKE, `function` (small buffer), `bind` and placeholders, `mem_fn`, `hash` (scalars, pointers, enums) | function.objects: 71/88 |
| `<memory>` | `pointer_traits`, `allocator_traits`, `addressof`, `align`, `unique_ptr` (and `[]`), `make_unique`, `shared_ptr`, `weak_ptr`, `make_shared`, `allocate_shared`, `enable_shared_from_this`, pointer casts, `owner_less`, `get_deleter` | util.smartptr: 58/84; allocator.traits: 20/21; pointer.traits: 9/9 |
| `<array>` | `array` | array: 20/27 |
| `<cstddef>`, `<cstdint>` | `nullptr_t`, `max_align_t`; `<cstdint>` in C++11 mode | |

Most of the remaining failures need the container work of the next phase,
because libc++'s test support headers use C++11 containers.

**What's left:** `doc/stdcxx-cxx11-TODO.md`, item by item, with the
test tools in `compiler/stdcxx/tests/cxx11`.

**Next phases:**
1. Containers: move, `initializer_list` and `emplace` members for
   `vector`, `deque`, `list`, `basic_string`, `map`/`set`.
2. `<iterator>` (`begin`/`end`, `next`/`prev`, `move_iterator`),
   `<algorithm>` and `<numeric>` additions.
3. String conversions (`to_string`, `stoi`, ...) and `hash<string>`.
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

Checking method: the same test is built with psclang and with the host
Clang over the same stdcxx headers and runtime. When the host build is
right and psclang's is wrong, the bug is in psclang.
