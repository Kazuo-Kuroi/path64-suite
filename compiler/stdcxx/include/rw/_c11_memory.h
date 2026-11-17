// -*- C++ -*-
/***************************************************************************
 *
 * _c11_memory.h - C++11 and C++14 parts of <memory> [memory]:
 *                 pointer_traits, allocator_traits, addressof, align,
 *                 unique_ptr, shared_ptr, weak_ptr
 *
 * This is an internal header file used to implement the C++ Standard
 * Library. It should never be #included directly by a program.
 *
 ***************************************************************************
 *
 * Licensed to the Apache Software  Foundation (ASF) under one or more
 * contributor  license agreements.  See  the NOTICE  file distributed
 * with  this  work  for  additional information  regarding  copyright
 * ownership.   The ASF  licenses this  file to  you under  the Apache
 * License, Version  2.0 (the  "License"); you may  not use  this file
 * except in  compliance with the License.   You may obtain  a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the  License is distributed on an  "AS IS" BASIS,
 * WITHOUT  WARRANTIES OR CONDITIONS  OF ANY  KIND, either  express or
 * implied.   See  the License  for  the  specific language  governing
 * permissions and limitations under the License.
 *
 * Parts follow the design of libc++ 8 (University of Illinois/NCSA or
 * MIT license; see LICENSE-libcxx.txt).
 *
 **************************************************************************/

#ifndef _RWSTD_RW_C11_MEMORY_H_INCLUDED
#define _RWSTD_RW_C11_MEMORY_H_INCLUDED

#include <rw/_c11_functional.h>   // hash, less, tuple, uses_allocator
#include <rw/_funcbase.h>         // less
#include <exception>
#include <new>
#include <typeinfo>
#include <iosfwd>

namespace std {

template <class _Tp> class allocator;
template <class _Tp> class auto_ptr;

// [specialized.addressof]
template <class _Tp>
inline constexpr _Tp* addressof (_Tp& __x) noexcept
{
    return __builtin_addressof (__x);
}

// [pointer.traits]
namespace __rw_c11 {
template <class _Tp, class = void> struct __has_element_type : false_type { };
template <class _Tp>
struct __has_element_type<_Tp, typename __rw_tt::__void_t<typename _Tp::element_type>::type>
    : true_type { };

template <class _Ptr, bool = __has_element_type<_Ptr>::value> struct __pointer_element;
template <class _Ptr> struct __pointer_element<_Ptr, true>
{ typedef typename _Ptr::element_type type; };
template <template <class, class...> class _Sp, class _Tp, class... _Args>
struct __pointer_element<_Sp<_Tp, _Args...>, false> { typedef _Tp type; };

template <class _Tp, class = void> struct __has_difference_type : false_type { };
template <class _Tp>
struct __has_difference_type<_Tp, typename __rw_tt::__void_t<typename _Tp::difference_type>::type>
    : true_type { };
template <class _Ptr, bool = __has_difference_type<_Ptr>::value>
struct __pointer_difference { typedef ptrdiff_t type; };
template <class _Ptr> struct __pointer_difference<_Ptr, true>
{ typedef typename _Ptr::difference_type type; };

template <class _Tp, class _Up, class = void> struct __has_rebind : false_type { };
template <class _Tp, class _Up>
struct __has_rebind<_Tp, _Up, typename __rw_tt::__void_t<
    typename _Tp::template rebind<_Up> >::type> : true_type { };

template <class _Tp, class _Up, bool = __has_rebind<_Tp, _Up>::value>
struct __pointer_rebind { typedef typename _Tp::template rebind<_Up> type; };
template <template <class, class...> class _Sp, class _Tp, class... _Args, class _Up>
struct __pointer_rebind<_Sp<_Tp, _Args...>, _Up, false> { typedef _Sp<_Up, _Args...> type; };
}   // namespace __rw_c11

template <class _Ptr>
struct pointer_traits
{
    typedef _Ptr pointer;
    typedef typename __rw_c11::__pointer_element<pointer>::type element_type;
    typedef typename __rw_c11::__pointer_difference<pointer>::type difference_type;
    template <class _Up>
    using rebind = typename __rw_c11::__pointer_rebind<pointer, _Up>::type;

private:
    struct __nat { };
public:
    static pointer pointer_to (typename conditional<is_void<element_type>::value,
                                                    __nat, element_type>::type& __r)
    { return pointer::pointer_to (__r); }
};

template <class _Tp>
struct pointer_traits<_Tp*>
{
    typedef _Tp* pointer;
    typedef _Tp element_type;
    typedef ptrdiff_t difference_type;
    template <class _Up> using rebind = _Up*;

private:
    struct __nat { };
public:
    static pointer pointer_to (typename conditional<is_void<element_type>::value,
                                                    __nat, element_type>::type& __r) noexcept
    { return std::addressof (__r); }
};

// [allocator.traits]
namespace __rw_c11 {
#define _RWSTD_C11_HAS_TYPE(name)                                             \
template <class _Tp, class = void> struct __has_##name : false_type { };      \
template <class _Tp>                                                          \
struct __has_##name<_Tp, typename __rw_tt::__void_t<typename _Tp::name>::type> \
    : true_type { };
_RWSTD_C11_HAS_TYPE (pointer)
_RWSTD_C11_HAS_TYPE (const_pointer)
_RWSTD_C11_HAS_TYPE (void_pointer)
_RWSTD_C11_HAS_TYPE (const_void_pointer)
_RWSTD_C11_HAS_TYPE (size_type)
_RWSTD_C11_HAS_TYPE (propagate_on_container_copy_assignment)
_RWSTD_C11_HAS_TYPE (propagate_on_container_move_assignment)
_RWSTD_C11_HAS_TYPE (propagate_on_container_swap)
_RWSTD_C11_HAS_TYPE (is_always_equal)
#undef _RWSTD_C11_HAS_TYPE

template <class _Alloc, bool = __has_pointer<_Alloc>::value>
struct __alloc_pointer { typedef typename _Alloc::value_type* type; };
template <class _Alloc> struct __alloc_pointer<_Alloc, true>
{ typedef typename _Alloc::pointer type; };

template <class _Alloc, class _Ptr, class _Tp, bool = __has_const_pointer<_Alloc>::value>
struct __alloc_rebound_ptr
{ typedef typename pointer_traits<_Ptr>::template rebind<_Tp> type; };
template <class _Alloc, class _Ptr, class _Tp>
struct __alloc_rebound_ptr<_Alloc, _Ptr, _Tp, true>
{ typedef typename _Alloc::const_pointer type; };

template <class _Alloc, class _Ptr, bool = __has_void_pointer<_Alloc>::value>
struct __alloc_void_ptr
{ typedef typename pointer_traits<_Ptr>::template rebind<void> type; };
template <class _Alloc, class _Ptr> struct __alloc_void_ptr<_Alloc, _Ptr, true>
{ typedef typename _Alloc::void_pointer type; };

template <class _Alloc, class _Ptr, bool = __has_const_void_pointer<_Alloc>::value>
struct __alloc_cvoid_ptr
{ typedef typename pointer_traits<_Ptr>::template rebind<const void> type; };
template <class _Alloc, class _Ptr> struct __alloc_cvoid_ptr<_Alloc, _Ptr, true>
{ typedef typename _Alloc::const_void_pointer type; };

template <class _Alloc, class _Ptr, bool = __has_difference_type<_Alloc>::value>
struct __alloc_diff { typedef typename pointer_traits<_Ptr>::difference_type type; };
template <class _Alloc, class _Ptr> struct __alloc_diff<_Alloc, _Ptr, true>
{ typedef typename _Alloc::difference_type type; };

template <class _Alloc, class _Diff, bool = __has_size_type<_Alloc>::value>
struct __alloc_size { typedef typename make_unsigned<_Diff>::type type; };
template <class _Alloc, class _Diff> struct __alloc_size<_Alloc, _Diff, true>
{ typedef typename _Alloc::size_type type; };

#define _RWSTD_C11_ALLOC_PROP(name, dflt)                                     \
template <class _Alloc, bool = __has_##name<_Alloc>::value>                   \
struct __alloc_##name { typedef dflt type; };                                 \
template <class _Alloc> struct __alloc_##name<_Alloc, true>                   \
{ typedef typename _Alloc::name type; };
_RWSTD_C11_ALLOC_PROP (propagate_on_container_copy_assignment, false_type)
_RWSTD_C11_ALLOC_PROP (propagate_on_container_move_assignment, false_type)
_RWSTD_C11_ALLOC_PROP (propagate_on_container_swap, false_type)
#undef _RWSTD_C11_ALLOC_PROP

template <class _Alloc, bool = __has_is_always_equal<_Alloc>::value>
struct __alloc_is_always_equal { typedef typename is_empty<_Alloc>::type type; };
template <class _Alloc> struct __alloc_is_always_equal<_Alloc, true>
{ typedef typename _Alloc::is_always_equal type; };

template <class _Alloc, class _Up, class = void> struct __has_alloc_rebind : false_type { };
template <class _Alloc, class _Up>
struct __has_alloc_rebind<_Alloc, _Up, typename __rw_tt::__void_t<
    typename _Alloc::template rebind<_Up>::other>::type> : true_type { };

template <class _Alloc, class _Up, bool = __has_alloc_rebind<_Alloc, _Up>::value>
struct __alloc_rebind { typedef typename _Alloc::template rebind<_Up>::other type; };
template <template <class, class...> class _Alloc, class _Tp, class... _Args, class _Up>
struct __alloc_rebind<_Alloc<_Tp, _Args...>, _Up, false> { typedef _Alloc<_Up, _Args...> type; };

// the optional allocator members
template <class _Alloc, class _Ptr, class... _Args>
auto __construct_test (int)
    -> decltype (declval<_Alloc&>().construct (declval<_Ptr>(), declval<_Args>()...),
                 true_type ());
template <class _Alloc, class _Ptr, class... _Args> false_type __construct_test (...);

template <class _Alloc, class _Ptr>
auto __destroy_test (int) -> decltype (declval<_Alloc&>().destroy (declval<_Ptr>()),
                                       true_type ());
template <class _Alloc, class _Ptr> false_type __destroy_test (...);

template <class _Alloc>
auto __max_size_test (int) -> decltype (declval<const _Alloc&>().max_size (), true_type ());
template <class _Alloc> false_type __max_size_test (...);

template <class _Alloc>
auto __select_test (int)
    -> decltype (declval<const _Alloc&>().select_on_container_copy_construction (),
                 true_type ());
template <class _Alloc> false_type __select_test (...);

template <class _Alloc, class _Size, class _Hint>
auto __alloc_hint_test (int)
    -> decltype (declval<_Alloc&>().allocate (declval<_Size>(), declval<_Hint>()),
                 true_type ());
template <class _Alloc, class _Size, class _Hint> false_type __alloc_hint_test (...);
}   // namespace __rw_c11

template <class _Alloc>
struct allocator_traits
{
    typedef _Alloc allocator_type;
    typedef typename allocator_type::value_type value_type;

    typedef typename __rw_c11::__alloc_pointer<allocator_type>::type pointer;
    typedef typename __rw_c11::__alloc_rebound_ptr<allocator_type, pointer,
                                                   const value_type>::type const_pointer;
    typedef typename __rw_c11::__alloc_void_ptr<allocator_type, pointer>::type void_pointer;
    typedef typename __rw_c11::__alloc_cvoid_ptr<allocator_type, pointer>::type
        const_void_pointer;
    typedef typename __rw_c11::__alloc_diff<allocator_type, pointer>::type difference_type;
    typedef typename __rw_c11::__alloc_size<allocator_type, difference_type>::type size_type;

    typedef typename __rw_c11::__alloc_propagate_on_container_copy_assignment<
        allocator_type>::type propagate_on_container_copy_assignment;
    typedef typename __rw_c11::__alloc_propagate_on_container_move_assignment<
        allocator_type>::type propagate_on_container_move_assignment;
    typedef typename __rw_c11::__alloc_propagate_on_container_swap<
        allocator_type>::type propagate_on_container_swap;
    typedef typename __rw_c11::__alloc_is_always_equal<allocator_type>::type is_always_equal;

    template <class _Tp>
    using rebind_alloc = typename __rw_c11::__alloc_rebind<allocator_type, _Tp>::type;
    template <class _Tp>
    using rebind_traits = allocator_traits<rebind_alloc<_Tp> >;

    static pointer allocate (allocator_type& __a, size_type __n)
    { return __a.allocate (__n); }

    static pointer allocate (allocator_type& __a, size_type __n, const_void_pointer __hint)
    {
        return __allocate (__a, __n, __hint,
                           decltype (__rw_c11::__alloc_hint_test<allocator_type, size_type,
                                                                 const_void_pointer>(0)) ());
    }

    static void deallocate (allocator_type& __a, pointer __p, size_type __n) noexcept
    { __a.deallocate (__p, __n); }

    template <class _Tp, class... _Args>
    static void construct (allocator_type& __a, _Tp* __p, _Args&&... __args)
    {
        __construct (decltype (__rw_c11::__construct_test<allocator_type, _Tp*, _Args...>(0)) (),
                     __a, __p, std::forward<_Args>(__args)...);
    }

    template <class _Tp>
    static void destroy (allocator_type& __a, _Tp* __p)
    {
        __destroy (decltype (__rw_c11::__destroy_test<allocator_type, _Tp*>(0)) (), __a, __p);
    }

    static size_type max_size (const allocator_type& __a) noexcept
    {
        return __max_size (decltype (__rw_c11::__max_size_test<const allocator_type>(0)) (), __a);
    }

    static allocator_type select_on_container_copy_construction (const allocator_type& __a)
    {
        return __select (decltype (__rw_c11::__select_test<const allocator_type>(0)) (), __a);
    }

private:
    static pointer __allocate (allocator_type& __a, size_type __n,
                               const_void_pointer __hint, true_type)
    { return __a.allocate (__n, __hint); }
    static pointer __allocate (allocator_type& __a, size_type __n,
                               const_void_pointer, false_type)
    { return __a.allocate (__n); }

    template <class _Tp, class... _Args>
    static void __construct (true_type, allocator_type& __a, _Tp* __p, _Args&&... __args)
    { __a.construct (__p, std::forward<_Args>(__args)...); }
    template <class _Tp, class... _Args>
    static void __construct (false_type, allocator_type&, _Tp* __p, _Args&&... __args)
    { ::new ((void*)__p) _Tp (std::forward<_Args>(__args)...); }

    template <class _Tp>
    static void __destroy (true_type, allocator_type& __a, _Tp* __p) { __a.destroy (__p); }
    template <class _Tp>
    static void __destroy (false_type, allocator_type&, _Tp* __p) { __p->~_Tp (); }

    static size_type __max_size (true_type, const allocator_type& __a) noexcept
    { return __a.max_size (); }
    static size_type __max_size (false_type, const allocator_type&) noexcept
    { return size_type (~size_type (0)) / sizeof (value_type); }

    static allocator_type __select (true_type, const allocator_type& __a)
    { return __a.select_on_container_copy_construction (); }
    static allocator_type __select (false_type, const allocator_type& __a)
    { return __a; }
};

// [ptr.align]
inline void* align (size_t __alignment, size_t __size, void*& __ptr, size_t& __space)
{
    size_t __addr = reinterpret_cast<size_t>(__ptr);
    size_t __aligned = (__addr + __alignment - 1) & ~(__alignment - 1);
    size_t __diff = __aligned - __addr;
    if (__size + __diff > __space)
        return 0;
    __space -= __diff;
    return __ptr = reinterpret_cast<void*>(__aligned);
}

// [unique.ptr.dltr]
template <class _Tp>
struct default_delete
{
    static_assert (!is_function<_Tp>::value, "default_delete cannot be instantiated "
                   "for function types");
    constexpr default_delete () noexcept = default;
    template <class _Up, class = typename enable_if<is_convertible<_Up*, _Tp*>::value>::type>
    default_delete (const default_delete<_Up>&) noexcept { }
    void operator() (_Tp* __p) const noexcept
    {
        static_assert (sizeof (_Tp) > 0, "default_delete can not delete incomplete type");
        static_assert (!is_void<_Tp>::value, "default_delete can not delete incomplete type");
        delete __p;
    }
};

template <class _Tp>
struct default_delete<_Tp[]>
{
    constexpr default_delete () noexcept = default;
    template <class _Up, class = typename enable_if<
        is_convertible<_Up (*)[], _Tp (*)[]>::value>::type>
    default_delete (const default_delete<_Up[]>&) noexcept { }
    template <class _Up, class = typename enable_if<
        is_convertible<_Up (*)[], _Tp (*)[]>::value>::type>
    void operator() (_Up* __p) const noexcept
    {
        static_assert (sizeof (_Tp) > 0, "default_delete can not delete incomplete type");
        delete [] __p;
    }
};

namespace __rw_c11 {
// the pointer and the deleter, the deleter (empty, not final) as a base
template <class _Ptr, class _Del, bool = is_empty<_Del>::value && !__is_final (_Del)>
class __ptr_del
{
    _Ptr __p;
    _Del __d;
public:
    __ptr_del () : __p (), __d () { }
    template <class _Pp, class _Dp>
    __ptr_del (_Pp&& __pp, _Dp&& __dp)
        : __p (std::forward<_Pp>(__pp)), __d (std::forward<_Dp>(__dp)) { }
    _Ptr& __ptr () noexcept { return __p; }
    const _Ptr& __ptr () const noexcept { return __p; }
    _Del& __del () noexcept { return __d; }
    const _Del& __del () const noexcept { return __d; }
};

template <class _Ptr, class _Del>
class __ptr_del<_Ptr, _Del, true> : private _Del
{
    _Ptr __p;
public:
    __ptr_del () : _Del (), __p () { }
    template <class _Pp, class _Dp>
    __ptr_del (_Pp&& __pp, _Dp&& __dp)
        : _Del (std::forward<_Dp>(__dp)), __p (std::forward<_Pp>(__pp)) { }
    _Ptr& __ptr () noexcept { return __p; }
    const _Ptr& __ptr () const noexcept { return __p; }
    _Del& __del () noexcept { return *this; }
    const _Del& __del () const noexcept { return *this; }
};

// unique_ptr<T, D>::pointer: D::pointer if there is one, else T*
template <class _Tp, class _Del, class = void>
struct __unique_pointer { typedef _Tp* type; };
template <class _Tp, class _Del>
struct __unique_pointer<_Tp, _Del, typename __rw_tt::__void_t<
    typename remove_reference<_Del>::type::pointer>::type>
{ typedef typename remove_reference<_Del>::type::pointer type; };
}   // namespace __rw_c11

// [unique.ptr.single]
template <class _Tp, class _Dp = default_delete<_Tp> >
class unique_ptr
{
public:
    typedef typename __rw_c11::__unique_pointer<_Tp, _Dp>::type pointer;
    typedef _Tp element_type;
    typedef _Dp deleter_type;

private:
    __rw_c11::__ptr_del<pointer, _Dp> __pd;

    typedef typename conditional<is_reference<_Dp>::value, _Dp,
                                 const _Dp&>::type _LValRefDel;
    typedef typename remove_reference<_Dp>::type&& _RValRefDel;

    template <class _Up, class _Ep>
    struct __convertible_from
        : integral_constant<bool,
              is_convertible<typename unique_ptr<_Up, _Ep>::pointer, pointer>::value
              && !is_array<_Up>::value
              && (is_reference<_Dp>::value ? is_same<_Ep, _Dp>::value
                                           : is_convertible<_Ep, _Dp>::value)> { };

public:
    // [unique.ptr.single.ctor]
    template <bool _Dummy = true, class = typename enable_if<
        _Dummy && !is_pointer<_Dp>::value && is_default_constructible<_Dp>::value>::type>
    constexpr unique_ptr () noexcept : __pd () { }

    template <bool _Dummy = true, class = typename enable_if<
        _Dummy && !is_pointer<_Dp>::value && is_default_constructible<_Dp>::value>::type>
    constexpr unique_ptr (decltype (nullptr)) noexcept : __pd () { }

    template <bool _Dummy = true, class = typename enable_if<
        _Dummy && !is_pointer<_Dp>::value && is_default_constructible<_Dp>::value>::type>
    explicit unique_ptr (pointer __p) noexcept : __pd (__p, _Dp ()) { }

    template <bool _Dummy = true, class = typename enable_if<
        _Dummy && is_constructible<_Dp, _LValRefDel>::value>::type>
    unique_ptr (pointer __p, _LValRefDel __d) noexcept : __pd (__p, __d) { }

    template <bool _Dummy = true, class = typename enable_if<
        _Dummy && !is_reference<_Dp>::value
        && is_constructible<_Dp, _RValRefDel>::value>::type>
    unique_ptr (pointer __p, _RValRefDel __d) noexcept : __pd (__p, std::move (__d)) { }

    unique_ptr (unique_ptr&& __u) noexcept
        : __pd (__u.release (), std::forward<_Dp>(__u.get_deleter ())) { }

    template <class _Up, class _Ep, class = typename enable_if<
        __convertible_from<_Up, _Ep>::value>::type>
    unique_ptr (unique_ptr<_Up, _Ep>&& __u) noexcept
        : __pd (__u.release (), std::forward<_Ep>(__u.get_deleter ())) { }

    template <class _Up, class = typename enable_if<
        is_convertible<_Up*, _Tp*>::value && is_same<_Dp, default_delete<_Tp> >::value>::type>
    unique_ptr (auto_ptr<_Up>&& __p) noexcept : __pd (__p.release (), _Dp ()) { }

    unique_ptr (const unique_ptr&) = delete;
    unique_ptr& operator= (const unique_ptr&) = delete;

    ~unique_ptr () { reset (); }

    // [unique.ptr.single.asgn]
    unique_ptr& operator= (unique_ptr&& __u) noexcept
    {
        reset (__u.release ());
        get_deleter () = std::forward<_Dp>(__u.get_deleter ());
        return *this;
    }

    template <class _Up, class _Ep, class = typename enable_if<
        __convertible_from<_Up, _Ep>::value
        && is_assignable<_Dp&, _Ep&&>::value>::type>
    unique_ptr& operator= (unique_ptr<_Up, _Ep>&& __u) noexcept
    {
        reset (__u.release ());
        get_deleter () = std::forward<_Ep>(__u.get_deleter ());
        return *this;
    }

    unique_ptr& operator= (decltype (nullptr)) noexcept
    {
        reset ();
        return *this;
    }

    // [unique.ptr.single.observers]
    typename add_lvalue_reference<_Tp>::type operator* () const
    { return *__pd.__ptr (); }
    pointer operator-> () const noexcept { return __pd.__ptr (); }
    pointer get () const noexcept { return __pd.__ptr (); }
    deleter_type& get_deleter () noexcept { return __pd.__del (); }
    const deleter_type& get_deleter () const noexcept { return __pd.__del (); }
    explicit operator bool () const noexcept { return __pd.__ptr () != pointer (); }

    // [unique.ptr.single.modifiers]
    pointer release () noexcept
    {
        pointer __t = __pd.__ptr ();
        __pd.__ptr () = pointer ();
        return __t;
    }

    void reset (pointer __p = pointer ()) noexcept
    {
        pointer __tmp = __pd.__ptr ();
        __pd.__ptr () = __p;
        if (__tmp)
            __pd.__del ()(__tmp);
    }

    void swap (unique_ptr& __u) noexcept
    {
        using std::swap;
        swap (__pd.__ptr (), __u.__pd.__ptr ());
        swap (__pd.__del (), __u.__pd.__del ());
    }
};

// [unique.ptr.runtime]
template <class _Tp, class _Dp>
class unique_ptr<_Tp[], _Dp>
{
public:
    typedef typename __rw_c11::__unique_pointer<_Tp, _Dp>::type pointer;
    typedef _Tp element_type;
    typedef _Dp deleter_type;

private:
    __rw_c11::__ptr_del<pointer, _Dp> __pd;

    typedef typename conditional<is_reference<_Dp>::value, _Dp,
                                 const _Dp&>::type _LValRefDel;
    typedef typename remove_reference<_Dp>::type&& _RValRefDel;

    // [unique.ptr.runtime.ctor]: pointer, or U* for U[] convertible to T[]
    template <class _Up>
    struct __ptr_ok
        : integral_constant<bool, is_same<_Up, pointer>::value
                                  || (is_same<pointer, element_type*>::value
                                      && is_pointer<_Up>::value
                                      && is_convertible<typename remove_pointer<_Up>::type (*)[],
                                                        element_type (*)[]>::value)> { };

public:
    template <bool _Dummy = true, class = typename enable_if<
        _Dummy && !is_pointer<_Dp>::value && is_default_constructible<_Dp>::value>::type>
    constexpr unique_ptr () noexcept : __pd () { }

    template <bool _Dummy = true, class = typename enable_if<
        _Dummy && !is_pointer<_Dp>::value && is_default_constructible<_Dp>::value>::type>
    constexpr unique_ptr (decltype (nullptr)) noexcept : __pd () { }

    template <class _Up, class = typename enable_if<
        __ptr_ok<_Up>::value && !is_pointer<_Dp>::value
        && is_default_constructible<_Dp>::value>::type>
    explicit unique_ptr (_Up __p) noexcept : __pd (__p, _Dp ()) { }

    template <class _Up, class = typename enable_if<
        __ptr_ok<_Up>::value && is_constructible<_Dp, _LValRefDel>::value>::type>
    unique_ptr (_Up __p, _LValRefDel __d) noexcept : __pd (__p, __d) { }

    template <class _Up, class = typename enable_if<
        __ptr_ok<_Up>::value && !is_reference<_Dp>::value
        && is_constructible<_Dp, _RValRefDel>::value>::type>
    unique_ptr (_Up __p, _RValRefDel __d) noexcept : __pd (__p, std::move (__d)) { }

    unique_ptr (decltype (nullptr), _LValRefDel __d) noexcept : __pd (pointer (), __d) { }

    unique_ptr (unique_ptr&& __u) noexcept
        : __pd (__u.release (), std::forward<_Dp>(__u.get_deleter ())) { }

    template <class _Up, class _Ep, class = typename enable_if<
        is_array<_Up>::value
        && is_same<pointer, element_type*>::value
        && is_same<typename unique_ptr<_Up, _Ep>::pointer,
                   typename unique_ptr<_Up, _Ep>::element_type*>::value
        && is_convertible<typename unique_ptr<_Up, _Ep>::element_type (*)[],
                          element_type (*)[]>::value
        && (is_reference<_Dp>::value ? is_same<_Ep, _Dp>::value
                                     : is_convertible<_Ep, _Dp>::value)>::type>
    unique_ptr (unique_ptr<_Up, _Ep>&& __u) noexcept
        : __pd (__u.release (), std::forward<_Ep>(__u.get_deleter ())) { }

    unique_ptr (const unique_ptr&) = delete;
    unique_ptr& operator= (const unique_ptr&) = delete;

    ~unique_ptr () { reset (); }

    unique_ptr& operator= (unique_ptr&& __u) noexcept
    {
        reset (__u.release ());
        get_deleter () = std::forward<_Dp>(__u.get_deleter ());
        return *this;
    }

    template <class _Up, class _Ep, class = typename enable_if<
        is_array<_Up>::value
        && is_convertible<typename unique_ptr<_Up, _Ep>::element_type (*)[],
                          element_type (*)[]>::value
        && is_assignable<_Dp&, _Ep&&>::value>::type>
    unique_ptr& operator= (unique_ptr<_Up, _Ep>&& __u) noexcept
    {
        reset (__u.release ());
        get_deleter () = std::forward<_Ep>(__u.get_deleter ());
        return *this;
    }

    unique_ptr& operator= (decltype (nullptr)) noexcept
    {
        reset ();
        return *this;
    }

    typename add_lvalue_reference<_Tp>::type operator[] (size_t __i) const
    { return __pd.__ptr ()[__i]; }
    pointer get () const noexcept { return __pd.__ptr (); }
    deleter_type& get_deleter () noexcept { return __pd.__del (); }
    const deleter_type& get_deleter () const noexcept { return __pd.__del (); }
    explicit operator bool () const noexcept { return __pd.__ptr () != pointer (); }

    pointer release () noexcept
    {
        pointer __t = __pd.__ptr ();
        __pd.__ptr () = pointer ();
        return __t;
    }

    template <class _Up, class = typename enable_if<__ptr_ok<_Up>::value>::type>
    void reset (_Up __p) noexcept
    {
        pointer __tmp = __pd.__ptr ();
        __pd.__ptr () = __p;
        if (__tmp)
            __pd.__del ()(__tmp);
    }

    void reset (decltype (nullptr) = nullptr) noexcept
    {
        pointer __tmp = __pd.__ptr ();
        __pd.__ptr () = pointer ();
        if (__tmp)
            __pd.__del ()(__tmp);
    }

    void swap (unique_ptr& __u) noexcept
    {
        using std::swap;
        swap (__pd.__ptr (), __u.__pd.__ptr ());
        swap (__pd.__del (), __u.__pd.__del ());
    }
};

template <class _Tp, class _Dp>
inline typename enable_if<__rw_c11::__is_swappable<_Dp>::value>::type
swap (unique_ptr<_Tp, _Dp>& __x, unique_ptr<_Tp, _Dp>& __y) noexcept
{ __x.swap (__y); }

template <class _T1, class _D1, class _T2, class _D2>
inline bool operator== (const unique_ptr<_T1, _D1>& __x, const unique_ptr<_T2, _D2>& __y)
{ return __x.get () == __y.get (); }
template <class _T1, class _D1, class _T2, class _D2>
inline bool operator!= (const unique_ptr<_T1, _D1>& __x, const unique_ptr<_T2, _D2>& __y)
{ return !(__x == __y); }
template <class _T1, class _D1, class _T2, class _D2>
inline bool operator< (const unique_ptr<_T1, _D1>& __x, const unique_ptr<_T2, _D2>& __y)
{
    typedef typename common_type<typename unique_ptr<_T1, _D1>::pointer,
                                 typename unique_ptr<_T2, _D2>::pointer>::type _Vp;
    return less<_Vp>()(__x.get (), __y.get ());
}
template <class _T1, class _D1, class _T2, class _D2>
inline bool operator> (const unique_ptr<_T1, _D1>& __x, const unique_ptr<_T2, _D2>& __y)
{ return __y < __x; }
template <class _T1, class _D1, class _T2, class _D2>
inline bool operator<= (const unique_ptr<_T1, _D1>& __x, const unique_ptr<_T2, _D2>& __y)
{ return !(__y < __x); }
template <class _T1, class _D1, class _T2, class _D2>
inline bool operator>= (const unique_ptr<_T1, _D1>& __x, const unique_ptr<_T2, _D2>& __y)
{ return !(__x < __y); }

template <class _T1, class _D1>
inline bool operator== (const unique_ptr<_T1, _D1>& __x, decltype (nullptr)) noexcept
{ return !__x; }
template <class _T1, class _D1>
inline bool operator== (decltype (nullptr), const unique_ptr<_T1, _D1>& __x) noexcept
{ return !__x; }
template <class _T1, class _D1>
inline bool operator!= (const unique_ptr<_T1, _D1>& __x, decltype (nullptr)) noexcept
{ return static_cast<bool>(__x); }
template <class _T1, class _D1>
inline bool operator!= (decltype (nullptr), const unique_ptr<_T1, _D1>& __x) noexcept
{ return static_cast<bool>(__x); }
template <class _T1, class _D1>
inline bool operator< (const unique_ptr<_T1, _D1>& __x, decltype (nullptr))
{ return less<typename unique_ptr<_T1, _D1>::pointer>()(__x.get (), nullptr); }
template <class _T1, class _D1>
inline bool operator< (decltype (nullptr), const unique_ptr<_T1, _D1>& __x)
{ return less<typename unique_ptr<_T1, _D1>::pointer>()(nullptr, __x.get ()); }
template <class _T1, class _D1>
inline bool operator> (const unique_ptr<_T1, _D1>& __x, decltype (nullptr))
{ return nullptr < __x; }
template <class _T1, class _D1>
inline bool operator> (decltype (nullptr), const unique_ptr<_T1, _D1>& __x)
{ return __x < nullptr; }
template <class _T1, class _D1>
inline bool operator<= (const unique_ptr<_T1, _D1>& __x, decltype (nullptr))
{ return !(nullptr < __x); }
template <class _T1, class _D1>
inline bool operator<= (decltype (nullptr), const unique_ptr<_T1, _D1>& __x)
{ return !(__x < nullptr); }
template <class _T1, class _D1>
inline bool operator>= (const unique_ptr<_T1, _D1>& __x, decltype (nullptr))
{ return !(__x < nullptr); }
template <class _T1, class _D1>
inline bool operator>= (decltype (nullptr), const unique_ptr<_T1, _D1>& __x)
{ return !(nullptr < __x); }

#if __cplusplus >= 201402L
// [unique.ptr.create]
namespace __rw_c11 {
template <class _Tp> struct __unique_if { typedef unique_ptr<_Tp> __single; };
template <class _Tp> struct __unique_if<_Tp[]> { typedef unique_ptr<_Tp[]> __array; };
template <class _Tp, size_t _Np> struct __unique_if<_Tp[_Np]> { typedef void __bound; };
}

template <class _Tp, class... _Args>
inline typename __rw_c11::__unique_if<_Tp>::__single make_unique (_Args&&... __args)
{ return unique_ptr<_Tp>(new _Tp (std::forward<_Args>(__args)...)); }

template <class _Tp>
inline typename __rw_c11::__unique_if<_Tp>::__array make_unique (size_t __n)
{
    typedef typename remove_extent<_Tp>::type _Up;
    return unique_ptr<_Tp>(new _Up [__n] ());
}

template <class _Tp, class... _Args>
typename __rw_c11::__unique_if<_Tp>::__bound make_unique (_Args&&...) = delete;
#endif   // C++14

template <class _Tp, class _Dp>
struct hash<unique_ptr<_Tp, _Dp> >
{
    typedef unique_ptr<_Tp, _Dp> argument_type;
    typedef size_t result_type;
    size_t operator() (const argument_type& __p) const
    {
        typedef typename argument_type::pointer _Ptr;
        return hash<_Ptr>()(__p.get ());
    }
};

// [util.smartptr.weakptr] bad_weak_ptr
class bad_weak_ptr : public exception
{
public:
    virtual const char* what () const noexcept { return "bad_weak_ptr"; }
};

namespace __rw_c11 {

[[noreturn]] inline void __throw_bad_weak_ptr ()
{
#ifndef _RWSTD_NO_EXCEPTIONS
    throw bad_weak_ptr ();
#else
    __builtin_abort ();
#endif
}

// the control block: __shared, the shared_ptrs minus one; __weak, the
// weak_ptrs minus one, plus one while there are shared_ptrs
class __shared_count
{
    __shared_count (const __shared_count&);
    __shared_count& operator= (const __shared_count&);

    long __shared;
    long __weak;

protected:
    virtual ~__shared_count () { }

public:
    __shared_count () noexcept : __shared (0), __weak (0) { }

    void __add_shared () noexcept { __sync_add_and_fetch (&__shared, 1L); }
    void __add_weak () noexcept { __sync_add_and_fetch (&__weak, 1L); }

    void __release_shared () noexcept
    {
        if (__sync_sub_and_fetch (&__shared, 1L) == -1) {
            __on_zero_shared ();
            __release_weak ();
        }
    }

    void __release_weak () noexcept
    {
        if (__sync_sub_and_fetch (&__weak, 1L) == -1)
            __on_zero_shared_weak ();
    }

    long use_count () const noexcept
    {
        return __sync_add_and_fetch (const_cast<long*>(&__shared), 0L) + 1;
    }

    // a shared_ptr from a weak_ptr, if there is still an owner
    bool __lock () noexcept
    {
        long __n = __sync_add_and_fetch (&__shared, 0L);
        while (__n != -1) {
            long __old = __sync_val_compare_and_swap (&__shared, __n, __n + 1);
            if (__old == __n)
                return true;
            __n = __old;
        }
        return false;
    }

    virtual const void* __get_deleter (const std::type_info&) const noexcept { return 0; }

private:
    virtual void __on_zero_shared () noexcept = 0;
    virtual void __on_zero_shared_weak () noexcept = 0;
};

// a pointer owned with its deleter (and the allocator for the block)
template <class _Tp, class _Dp, class _Alloc>
class __shared_ptr_pointer : public __shared_count
{
    _Tp __p;
    _Dp __d;
    _Alloc __a;

public:
    __shared_ptr_pointer (_Tp __p, _Dp __d, _Alloc __a)
        : __p (__p), __d (std::move (__d)), __a (std::move (__a)) { }

    virtual const void* __get_deleter (const std::type_info& __t) const noexcept
    {
        return __t == typeid (_Dp) ? std::addressof (__d) : 0;
    }

private:
    virtual void __on_zero_shared () noexcept { __d (__p); __d.~_Dp (); }

    virtual void __on_zero_shared_weak () noexcept
    {
        typedef typename allocator_traits<_Alloc>::template
            rebind_alloc<__shared_ptr_pointer> _Al;
        _Al __al (__a);
        __a.~_Alloc ();
        allocator_traits<_Al>::deallocate (__al, this, 1);
    }
};

// make_shared, allocate_shared: the object in the control block
template <class _Tp, class _Alloc>
class __shared_ptr_emplace : public __shared_count
{
    _Alloc __a;
    typename aligned_storage<sizeof (_Tp), alignof (_Tp)>::type __storage;

public:
    template <class... _Args>
    explicit __shared_ptr_emplace (_Alloc __a, _Args&&... __args) : __a (__a)
    {
        ::new ((void*)&__storage) _Tp (std::forward<_Args>(__args)...);
    }

    _Tp* __get () noexcept { return reinterpret_cast<_Tp*>(&__storage); }

private:
    virtual void __on_zero_shared () noexcept { __get ()->~_Tp (); }

    virtual void __on_zero_shared_weak () noexcept
    {
        typedef typename allocator_traits<_Alloc>::template
            rebind_alloc<__shared_ptr_emplace> _Al;
        _Al __al (__a);
        __a.~_Alloc ();
        allocator_traits<_Al>::deallocate (__al, this, 1);
    }
};

}   // namespace __rw_c11

template <class _Tp> class weak_ptr;
template <class _Tp> class enable_shared_from_this;

// [util.smartptr.shared]
template <class _Tp>
class shared_ptr
{
public:
    typedef _Tp element_type;

private:
    element_type* __ptr;
    __rw_c11::__shared_count* __cntrl;

    template <class _Up> friend class shared_ptr;
    template <class _Up> friend class weak_ptr;

    template <class _Yp, class _OrigPtr>
    void __enable_weak_this (const enable_shared_from_this<_Yp>* __e,
                             _OrigPtr* __ptr) noexcept
    {
        typedef typename remove_cv<_Yp>::type _RawY;
        if (__e && __e->__weak_this.expired ())
            __e->__weak_this = shared_ptr<_RawY>(*this,
                                   const_cast<_RawY*>(static_cast<const _Yp*>(__ptr)));
    }
    void __enable_weak_this (...) noexcept { }

    template <class _Yp, class _Dp, class _Alloc>
    void __own (_Yp* __p, _Dp __d, _Alloc __a)
    {
        typedef __rw_c11::__shared_ptr_pointer<_Yp*, _Dp, _Alloc> _Block;
        typedef typename allocator_traits<_Alloc>::template rebind_alloc<_Block> _Al;
        _Al __al (__a);
#ifndef _RWSTD_NO_EXCEPTIONS
        try {
#endif
            _Block* __b = allocator_traits<_Al>::allocate (__al, 1);
            ::new ((void*)__b) _Block (__p, std::move (__d), __a);
            __cntrl = __b;
#ifndef _RWSTD_NO_EXCEPTIONS
        }
        catch (...) {
            __d (__p);
            throw;
        }
#endif
        __enable_weak_this (__p, __p);
    }

public:
    // [util.smartptr.shared.const]
    constexpr shared_ptr () noexcept : __ptr (0), __cntrl (0) { }
    constexpr shared_ptr (decltype (nullptr)) noexcept : __ptr (0), __cntrl (0) { }

    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, element_type*>::value>::type>
    explicit shared_ptr (_Yp* __p) : __ptr (__p), __cntrl (0)
    {
        __own (__p, default_delete<_Yp>(), allocator<_Yp>());
    }

    template <class _Yp, class _Dp, class = typename enable_if<
        is_convertible<_Yp*, element_type*>::value>::type>
    shared_ptr (_Yp* __p, _Dp __d) : __ptr (__p), __cntrl (0)
    {
        __own (__p, std::move (__d), allocator<_Yp>());
    }

    template <class _Yp, class _Dp, class _Alloc, class = typename enable_if<
        is_convertible<_Yp*, element_type*>::value>::type>
    shared_ptr (_Yp* __p, _Dp __d, _Alloc __a) : __ptr (__p), __cntrl (0)
    {
        __own (__p, std::move (__d), std::move (__a));
    }

    template <class _Dp>
    shared_ptr (decltype (nullptr) __p, _Dp __d) : __ptr (0), __cntrl (0)
    {
        __own<_Tp>(__p, std::move (__d), allocator<_Tp>());
    }

    template <class _Dp, class _Alloc>
    shared_ptr (decltype (nullptr) __p, _Dp __d, _Alloc __a) : __ptr (0), __cntrl (0)
    {
        __own<_Tp>(__p, std::move (__d), std::move (__a));
    }

    // aliasing
    template <class _Yp>
    shared_ptr (const shared_ptr<_Yp>& __r, element_type* __p) noexcept
        : __ptr (__p), __cntrl (__r.__cntrl)
    {
        if (__cntrl)
            __cntrl->__add_shared ();
    }

    shared_ptr (const shared_ptr& __r) noexcept : __ptr (__r.__ptr), __cntrl (__r.__cntrl)
    {
        if (__cntrl)
            __cntrl->__add_shared ();
    }

    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, element_type*>::value>::type>
    shared_ptr (const shared_ptr<_Yp>& __r) noexcept
        : __ptr (__r.__ptr), __cntrl (__r.__cntrl)
    {
        if (__cntrl)
            __cntrl->__add_shared ();
    }

    shared_ptr (shared_ptr&& __r) noexcept : __ptr (__r.__ptr), __cntrl (__r.__cntrl)
    {
        __r.__ptr = 0;
        __r.__cntrl = 0;
    }

    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, element_type*>::value>::type>
    shared_ptr (shared_ptr<_Yp>&& __r) noexcept : __ptr (__r.__ptr), __cntrl (__r.__cntrl)
    {
        __r.__ptr = 0;
        __r.__cntrl = 0;
    }

    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, element_type*>::value>::type>
    explicit shared_ptr (const weak_ptr<_Yp>& __r)
        : __ptr (__r.__ptr), __cntrl (__r.__cntrl)
    {
        if (!__cntrl || !__cntrl->__lock ())
            __rw_c11::__throw_bad_weak_ptr ();
    }

    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, element_type*>::value>::type>
    shared_ptr (auto_ptr<_Yp>&& __r) : __ptr (__r.get ()), __cntrl (0)
    {
        __own (__r.get (), default_delete<_Yp>(), allocator<_Yp>());
        __r.release ();
    }

    template <class _Yp, class _Dp, class = typename enable_if<
        is_convertible<typename unique_ptr<_Yp, _Dp>::pointer, element_type*>::value>::type>
    shared_ptr (unique_ptr<_Yp, _Dp>&& __r) : __ptr (__r.get ()), __cntrl (0)
    {
        if (__ptr) {
            typedef typename conditional<is_reference<_Dp>::value,
                reference_wrapper<typename remove_reference<_Dp>::type>, _Dp>::type _Del;
            __own (__r.get (), _Del (__r.get_deleter ()), allocator<_Yp>());
        }
        __r.release ();
    }

    ~shared_ptr ()
    {
        if (__cntrl)
            __cntrl->__release_shared ();
    }

    // [util.smartptr.shared.assign]
    shared_ptr& operator= (const shared_ptr& __r) noexcept
    {
        shared_ptr (__r).swap (*this);
        return *this;
    }
    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, element_type*>::value>::type>
    shared_ptr& operator= (const shared_ptr<_Yp>& __r) noexcept
    {
        shared_ptr (__r).swap (*this);
        return *this;
    }
    shared_ptr& operator= (shared_ptr&& __r) noexcept
    {
        shared_ptr (std::move (__r)).swap (*this);
        return *this;
    }
    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, element_type*>::value>::type>
    shared_ptr& operator= (shared_ptr<_Yp>&& __r)
    {
        shared_ptr (std::move (__r)).swap (*this);
        return *this;
    }
    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, element_type*>::value>::type>
    shared_ptr& operator= (auto_ptr<_Yp>&& __r)
    {
        shared_ptr (std::move (__r)).swap (*this);
        return *this;
    }
    template <class _Yp, class _Dp, class = typename enable_if<
        is_convertible<typename unique_ptr<_Yp, _Dp>::pointer, element_type*>::value>::type>
    shared_ptr& operator= (unique_ptr<_Yp, _Dp>&& __r)
    {
        shared_ptr (std::move (__r)).swap (*this);
        return *this;
    }

    // [util.smartptr.shared.mod]
    void swap (shared_ptr& __r) noexcept
    {
        std::swap (__ptr, __r.__ptr);
        std::swap (__cntrl, __r.__cntrl);
    }
    void reset () noexcept { shared_ptr ().swap (*this); }
    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, element_type*>::value>::type>
    void reset (_Yp* __p) { shared_ptr (__p).swap (*this); }
    template <class _Yp, class _Dp, class = typename enable_if<
        is_convertible<_Yp*, element_type*>::value>::type>
    void reset (_Yp* __p, _Dp __d) { shared_ptr (__p, __d).swap (*this); }
    template <class _Yp, class _Dp, class _Alloc, class = typename enable_if<
        is_convertible<_Yp*, element_type*>::value>::type>
    void reset (_Yp* __p, _Dp __d, _Alloc __a) { shared_ptr (__p, __d, __a).swap (*this); }

    // [util.smartptr.shared.obs]
    element_type* get () const noexcept { return __ptr; }
    typename add_lvalue_reference<element_type>::type operator* () const noexcept
    { return *__ptr; }
    element_type* operator-> () const noexcept { return __ptr; }
    long use_count () const noexcept { return __cntrl ? __cntrl->use_count () : 0; }
    bool unique () const noexcept { return use_count () == 1; }
    explicit operator bool () const noexcept { return get () != 0; }

    template <class _Up>
    bool owner_before (const shared_ptr<_Up>& __p) const noexcept
    { return __cntrl < __p.__cntrl; }
    template <class _Up>
    bool owner_before (const weak_ptr<_Up>& __p) const noexcept
    { return __cntrl < __p.__cntrl; }

    const void* __get_deleter (const std::type_info& __t) const noexcept
    { return __cntrl ? __cntrl->__get_deleter (__t) : 0; }

    template <class _Yp, class _CntrlBlk>
    static shared_ptr<_Tp> __create_with_control_block (_Yp* __p, _CntrlBlk* __c)
    {
        shared_ptr<_Tp> __r;
        __r.__ptr = __p;
        __r.__cntrl = __c;
        __r.__enable_weak_this (__r.__ptr, __r.__ptr);
        return __r;
    }
};

// [util.smartptr.shared.create]
template <class _Tp, class _Alloc, class... _Args>
inline shared_ptr<_Tp> allocate_shared (const _Alloc& __a, _Args&&... __args)
{
    typedef __rw_c11::__shared_ptr_emplace<_Tp, _Alloc> _Block;
    typedef typename allocator_traits<_Alloc>::template rebind_alloc<_Block> _Al;
    _Al __al (__a);
    _Block* __b = allocator_traits<_Al>::allocate (__al, 1);
#ifndef _RWSTD_NO_EXCEPTIONS
    try {
#endif
        ::new ((void*)__b) _Block (__a, std::forward<_Args>(__args)...);
#ifndef _RWSTD_NO_EXCEPTIONS
    }
    catch (...) {
        allocator_traits<_Al>::deallocate (__al, __b, 1);
        throw;
    }
#endif
    return shared_ptr<_Tp>::__create_with_control_block (__b->__get (), __b);
}

template <class _Tp, class... _Args>
inline shared_ptr<_Tp> make_shared (_Args&&... __args)
{
    return std::allocate_shared<_Tp>(allocator<_Tp>(), std::forward<_Args>(__args)...);
}

// [util.smartptr.shared.cmp]
template <class _Tp, class _Up>
inline bool operator== (const shared_ptr<_Tp>& __x, const shared_ptr<_Up>& __y) noexcept
{ return __x.get () == __y.get (); }
template <class _Tp, class _Up>
inline bool operator!= (const shared_ptr<_Tp>& __x, const shared_ptr<_Up>& __y) noexcept
{ return !(__x == __y); }
template <class _Tp, class _Up>
inline bool operator< (const shared_ptr<_Tp>& __x, const shared_ptr<_Up>& __y) noexcept
{
    typedef typename common_type<_Tp*, _Up*>::type _Vp;
    return less<_Vp>()(__x.get (), __y.get ());
}
template <class _Tp, class _Up>
inline bool operator> (const shared_ptr<_Tp>& __x, const shared_ptr<_Up>& __y) noexcept
{ return __y < __x; }
template <class _Tp, class _Up>
inline bool operator<= (const shared_ptr<_Tp>& __x, const shared_ptr<_Up>& __y) noexcept
{ return !(__y < __x); }
template <class _Tp, class _Up>
inline bool operator>= (const shared_ptr<_Tp>& __x, const shared_ptr<_Up>& __y) noexcept
{ return !(__x < __y); }

template <class _Tp>
inline bool operator== (const shared_ptr<_Tp>& __x, decltype (nullptr)) noexcept
{ return !__x; }
template <class _Tp>
inline bool operator== (decltype (nullptr), const shared_ptr<_Tp>& __x) noexcept
{ return !__x; }
template <class _Tp>
inline bool operator!= (const shared_ptr<_Tp>& __x, decltype (nullptr)) noexcept
{ return static_cast<bool>(__x); }
template <class _Tp>
inline bool operator!= (decltype (nullptr), const shared_ptr<_Tp>& __x) noexcept
{ return static_cast<bool>(__x); }
template <class _Tp>
inline bool operator< (const shared_ptr<_Tp>& __x, decltype (nullptr)) noexcept
{ return less<_Tp*>()(__x.get (), nullptr); }
template <class _Tp>
inline bool operator< (decltype (nullptr), const shared_ptr<_Tp>& __x) noexcept
{ return less<_Tp*>()(nullptr, __x.get ()); }
template <class _Tp>
inline bool operator> (const shared_ptr<_Tp>& __x, decltype (nullptr)) noexcept
{ return nullptr < __x; }
template <class _Tp>
inline bool operator> (decltype (nullptr), const shared_ptr<_Tp>& __x) noexcept
{ return __x < nullptr; }
template <class _Tp>
inline bool operator<= (const shared_ptr<_Tp>& __x, decltype (nullptr)) noexcept
{ return !(nullptr < __x); }
template <class _Tp>
inline bool operator<= (decltype (nullptr), const shared_ptr<_Tp>& __x) noexcept
{ return !(__x < nullptr); }
template <class _Tp>
inline bool operator>= (const shared_ptr<_Tp>& __x, decltype (nullptr)) noexcept
{ return !(__x < nullptr); }
template <class _Tp>
inline bool operator>= (decltype (nullptr), const shared_ptr<_Tp>& __x) noexcept
{ return !(nullptr < __x); }

// [util.smartptr.shared.spec], [util.smartptr.shared.cast], [util.smartptr.getdeleter]
template <class _Tp>
inline void swap (shared_ptr<_Tp>& __x, shared_ptr<_Tp>& __y) noexcept { __x.swap (__y); }

template <class _Tp, class _Up>
inline shared_ptr<_Tp> static_pointer_cast (const shared_ptr<_Up>& __r) noexcept
{ return shared_ptr<_Tp>(__r, static_cast<_Tp*>(__r.get ())); }
template <class _Tp, class _Up>
inline shared_ptr<_Tp> dynamic_pointer_cast (const shared_ptr<_Up>& __r) noexcept
{
    _Tp* __p = dynamic_cast<_Tp*>(__r.get ());
    return __p ? shared_ptr<_Tp>(__r, __p) : shared_ptr<_Tp>();
}
template <class _Tp, class _Up>
inline shared_ptr<_Tp> const_pointer_cast (const shared_ptr<_Up>& __r) noexcept
{ return shared_ptr<_Tp>(__r, const_cast<_Tp*>(__r.get ())); }

template <class _Dp, class _Tp>
inline _Dp* get_deleter (const shared_ptr<_Tp>& __p) noexcept
{
    return const_cast<_Dp*>(static_cast<const _Dp*>(__p.__get_deleter (typeid (_Dp))));
}

template <class _CharT, class _Traits, class _Yp>
inline basic_ostream<_CharT, _Traits>&
operator<< (basic_ostream<_CharT, _Traits>& __os, const shared_ptr<_Yp>& __p)
{
    return __os << __p.get ();
}

template <class _Tp>
struct hash<shared_ptr<_Tp> >
{
    typedef shared_ptr<_Tp> argument_type;
    typedef size_t result_type;
    size_t operator() (const argument_type& __p) const noexcept
    { return hash<_Tp*>()(__p.get ()); }
};

// [util.smartptr.weak]
template <class _Tp>
class weak_ptr
{
public:
    typedef _Tp element_type;

private:
    element_type* __ptr;
    __rw_c11::__shared_count* __cntrl;

    template <class _Up> friend class weak_ptr;
    template <class _Up> friend class shared_ptr;

public:
    constexpr weak_ptr () noexcept : __ptr (0), __cntrl (0) { }

    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, _Tp*>::value>::type>
    weak_ptr (const shared_ptr<_Yp>& __r) noexcept : __ptr (__r.__ptr), __cntrl (__r.__cntrl)
    {
        if (__cntrl)
            __cntrl->__add_weak ();
    }

    weak_ptr (const weak_ptr& __r) noexcept : __ptr (__r.__ptr), __cntrl (__r.__cntrl)
    {
        if (__cntrl)
            __cntrl->__add_weak ();
    }

    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, _Tp*>::value>::type>
    weak_ptr (const weak_ptr<_Yp>& __r) noexcept : __ptr (0), __cntrl (0)
    {
        shared_ptr<_Yp> __s = __r.lock ();   // a pointer that is still valid
        __ptr = __s.get ();
        __cntrl = __r.__cntrl;
        if (__cntrl)
            __cntrl->__add_weak ();
    }

    weak_ptr (weak_ptr&& __r) noexcept : __ptr (__r.__ptr), __cntrl (__r.__cntrl)
    {
        __r.__ptr = 0;
        __r.__cntrl = 0;
    }

    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, _Tp*>::value>::type>
    weak_ptr (weak_ptr<_Yp>&& __r) noexcept : weak_ptr (__r)
    {
        __r.reset ();
    }

    ~weak_ptr ()
    {
        if (__cntrl)
            __cntrl->__release_weak ();
    }

    weak_ptr& operator= (const weak_ptr& __r) noexcept
    {
        weak_ptr (__r).swap (*this);
        return *this;
    }
    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, _Tp*>::value>::type>
    weak_ptr& operator= (const weak_ptr<_Yp>& __r) noexcept
    {
        weak_ptr (__r).swap (*this);
        return *this;
    }
    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, _Tp*>::value>::type>
    weak_ptr& operator= (const shared_ptr<_Yp>& __r) noexcept
    {
        weak_ptr (__r).swap (*this);
        return *this;
    }
    weak_ptr& operator= (weak_ptr&& __r) noexcept
    {
        weak_ptr (std::move (__r)).swap (*this);
        return *this;
    }
    template <class _Yp, class = typename enable_if<is_convertible<_Yp*, _Tp*>::value>::type>
    weak_ptr& operator= (weak_ptr<_Yp>&& __r) noexcept
    {
        weak_ptr (std::move (__r)).swap (*this);
        return *this;
    }

    void swap (weak_ptr& __r) noexcept
    {
        std::swap (__ptr, __r.__ptr);
        std::swap (__cntrl, __r.__cntrl);
    }
    void reset () noexcept { weak_ptr ().swap (*this); }

    long use_count () const noexcept { return __cntrl ? __cntrl->use_count () : 0; }
    bool expired () const noexcept { return use_count () == 0; }

    shared_ptr<_Tp> lock () const noexcept
    {
        shared_ptr<_Tp> __r;
        if (__cntrl && __cntrl->__lock ()) {
            __r.__ptr = __ptr;
            __r.__cntrl = __cntrl;
        }
        return __r;
    }

    template <class _Up>
    bool owner_before (const shared_ptr<_Up>& __r) const noexcept
    { return __cntrl < __r.__cntrl; }
    template <class _Up>
    bool owner_before (const weak_ptr<_Up>& __r) const noexcept
    { return __cntrl < __r.__cntrl; }
};

template <class _Tp>
inline void swap (weak_ptr<_Tp>& __x, weak_ptr<_Tp>& __y) noexcept { __x.swap (__y); }

// [util.smartptr.ownerless]
template <class _Tp> struct owner_less;
template <class _Tp>
struct owner_less<shared_ptr<_Tp> >
{
    typedef bool result_type;
    bool operator() (const shared_ptr<_Tp>& __x, const shared_ptr<_Tp>& __y) const noexcept
    { return __x.owner_before (__y); }
    bool operator() (const shared_ptr<_Tp>& __x, const weak_ptr<_Tp>& __y) const noexcept
    { return __x.owner_before (__y); }
    bool operator() (const weak_ptr<_Tp>& __x, const shared_ptr<_Tp>& __y) const noexcept
    { return __x.owner_before (__y); }
};
template <class _Tp>
struct owner_less<weak_ptr<_Tp> >
{
    typedef bool result_type;
    bool operator() (const weak_ptr<_Tp>& __x, const weak_ptr<_Tp>& __y) const noexcept
    { return __x.owner_before (__y); }
    bool operator() (const shared_ptr<_Tp>& __x, const weak_ptr<_Tp>& __y) const noexcept
    { return __x.owner_before (__y); }
    bool operator() (const weak_ptr<_Tp>& __x, const shared_ptr<_Tp>& __y) const noexcept
    { return __x.owner_before (__y); }
};

// [util.smartptr.enab]
template <class _Tp>
class enable_shared_from_this
{
    mutable weak_ptr<_Tp> __weak_this;

    template <class _Up> friend class shared_ptr;

protected:
    constexpr enable_shared_from_this () noexcept { }
    enable_shared_from_this (const enable_shared_from_this&) noexcept { }
    enable_shared_from_this& operator= (const enable_shared_from_this&) noexcept
    { return *this; }
    ~enable_shared_from_this () { }

public:
    shared_ptr<_Tp> shared_from_this () { return shared_ptr<_Tp>(__weak_this); }
    shared_ptr<const _Tp> shared_from_this () const
    { return shared_ptr<const _Tp>(__weak_this); }
};

// [util.dynamic.safety]: no garbage collection
enum class pointer_safety : unsigned char { relaxed, preferred, strict };
inline void declare_reachable (void*) { }
template <class _Tp> inline _Tp* undeclare_reachable (_Tp* __p) { return __p; }
inline void declare_no_pointers (char*, size_t) { }
inline void undeclare_no_pointers (char*, size_t) { }
inline pointer_safety get_pointer_safety () noexcept { return pointer_safety::relaxed; }

}   // namespace std

#endif   // _RWSTD_RW_C11_MEMORY_H_INCLUDED
