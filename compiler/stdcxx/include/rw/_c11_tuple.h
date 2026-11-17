// -*- C++ -*-
/***************************************************************************
 *
 * _c11_tuple.h - C++11 and C++14 <tuple> [tuple]
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

#ifndef _RWSTD_RW_C11_TUPLE_H_INCLUDED
#define _RWSTD_RW_C11_TUPLE_H_INCLUDED

#include <rw/_c11_pair.h>
#include <rw/_c11_uses_alloc.h>
#include <rw/_allocator.h>

namespace std {

namespace __rw_c11 {

// indices 0 ... N-1 (C++11 has no index_sequence)
template <size_t _Np, size_t... _Ip>
struct __make_tuple_indices : __make_tuple_indices<_Np - 1, _Np - 1, _Ip...> { };
template <size_t... _Ip>
struct __make_tuple_indices<0, _Ip...> { typedef __tuple_indices<_Ip...> type; };

// one element of a tuple: empty classes (not final) as bases, so that they
// take no space
template <size_t _Ip, class _Hp,
          bool = is_empty<_Hp>::value && !__is_final (_Hp)>
class __tuple_leaf
{
    _Hp __value;

public:
    constexpr __tuple_leaf () noexcept (is_nothrow_default_constructible<_Hp>::value)
        : __value () { }

    template <class _Alloc>
    __tuple_leaf (integral_constant<int, 0>, const _Alloc&) : __value () { }
    template <class _Alloc>
    __tuple_leaf (integral_constant<int, 1>, const _Alloc& __a)
        : __value (allocator_arg_t (), __a) { }
    template <class _Alloc>
    __tuple_leaf (integral_constant<int, 2>, const _Alloc& __a) : __value (__a) { }

    template <class _Tp, class = typename enable_if<
        conditional<is_same<typename decay<_Tp>::type, __tuple_leaf>::value,
                    false_type, is_constructible<_Hp, _Tp> >::type::value>::type>
    explicit constexpr __tuple_leaf (_Tp&& __t)
        noexcept (is_nothrow_constructible<_Hp, _Tp>::value)
        : __value (std::forward<_Tp>(__t)) { }

    template <class _Tp, class _Alloc>
    __tuple_leaf (integral_constant<int, 0>, const _Alloc&, _Tp&& __t)
        : __value (std::forward<_Tp>(__t)) { }
    template <class _Tp, class _Alloc>
    __tuple_leaf (integral_constant<int, 1>, const _Alloc& __a, _Tp&& __t)
        : __value (allocator_arg_t (), __a, std::forward<_Tp>(__t)) { }
    template <class _Tp, class _Alloc>
    __tuple_leaf (integral_constant<int, 2>, const _Alloc& __a, _Tp&& __t)
        : __value (std::forward<_Tp>(__t), __a) { }

    __tuple_leaf (const __tuple_leaf&) = default;
    __tuple_leaf (__tuple_leaf&&) = default;


    int __swap (__tuple_leaf& __t)
        noexcept (__is_nothrow_swappable<__tuple_leaf>::value)
    {
        using std::swap;
        swap (__value, __t.__value);
        return 0;
    }

    _RWSTD_CONSTEXPR14 _Hp& __get () noexcept { return __value; }
    constexpr const _Hp& __get () const noexcept { return __value; }
};

template <size_t _Ip, class _Hp>
class __tuple_leaf<_Ip, _Hp, true> : private _Hp
{
public:
    constexpr __tuple_leaf () noexcept (is_nothrow_default_constructible<_Hp>::value) { }

    template <class _Alloc>
    __tuple_leaf (integral_constant<int, 0>, const _Alloc&) { }
    template <class _Alloc>
    __tuple_leaf (integral_constant<int, 1>, const _Alloc& __a)
        : _Hp (allocator_arg_t (), __a) { }
    template <class _Alloc>
    __tuple_leaf (integral_constant<int, 2>, const _Alloc& __a) : _Hp (__a) { }

    template <class _Tp, class = typename enable_if<
        conditional<is_same<typename decay<_Tp>::type, __tuple_leaf>::value,
                    false_type, is_constructible<_Hp, _Tp> >::type::value>::type>
    explicit constexpr __tuple_leaf (_Tp&& __t)
        noexcept (is_nothrow_constructible<_Hp, _Tp>::value)
        : _Hp (std::forward<_Tp>(__t)) { }

    template <class _Tp, class _Alloc>
    __tuple_leaf (integral_constant<int, 0>, const _Alloc&, _Tp&& __t)
        : _Hp (std::forward<_Tp>(__t)) { }
    template <class _Tp, class _Alloc>
    __tuple_leaf (integral_constant<int, 1>, const _Alloc& __a, _Tp&& __t)
        : _Hp (allocator_arg_t (), __a, std::forward<_Tp>(__t)) { }
    template <class _Tp, class _Alloc>
    __tuple_leaf (integral_constant<int, 2>, const _Alloc& __a, _Tp&& __t)
        : _Hp (std::forward<_Tp>(__t), __a) { }

    __tuple_leaf (const __tuple_leaf&) = default;
    __tuple_leaf (__tuple_leaf&&) = default;


    int __swap (__tuple_leaf& __t)
        noexcept (__is_nothrow_swappable<__tuple_leaf>::value)
    {
        using std::swap;
        swap (__get (), __t.__get ());
        return 0;
    }

    _RWSTD_CONSTEXPR14 _Hp& __get () noexcept { return static_cast<_Hp&>(*this); }
    constexpr const _Hp& __get () const noexcept
    { return static_cast<const _Hp&>(*this); }
};

template <class... _Tp> inline void __swallow (_Tp&&...) noexcept { }

// a condition made dependent on a template parameter, for enable_if
template <bool _Bp, class> struct __dep_bool : integral_constant<bool, _Bp> { };

template <class _Indices, class... _Tp> struct __tuple_impl;

template <size_t... _Ip, class... _Tp>
struct __tuple_impl<__tuple_indices<_Ip...>, _Tp...>
    : public __tuple_leaf<_Ip, _Tp>...
{
    constexpr __tuple_impl () { }

    // from values (_Up...: as many as _Tp, or the first ones of them,
    // the others value-initialized)
    struct __from_values { };
    template <class... _Up>
    explicit constexpr __tuple_impl (__from_values, _Up&&... __u)
        : __tuple_leaf<_Ip, _Tp>(std::forward<_Up>(__u))... { }

    struct __alloc_default { };
    template <class _Alloc>
    __tuple_impl (__alloc_default, const _Alloc& __a)
        : __tuple_leaf<_Ip, _Tp>(__uses_alloc_ctor<_Tp, _Alloc>(), __a)... { }

    template <class _Alloc, class... _Up>
    __tuple_impl (allocator_arg_t, const _Alloc& __a, _Up&&... __u)
        : __tuple_leaf<_Ip, _Tp>(__uses_alloc_ctor<_Tp, _Alloc, _Up>(), __a,
                                 std::forward<_Up>(__u))... { }

    __tuple_impl (const __tuple_impl&) = default;
    __tuple_impl (__tuple_impl&&) = default;

    void __swap (__tuple_impl& __t)
    {
        __swallow (__tuple_leaf<_Ip, _Tp>::__swap (
                       static_cast<__tuple_leaf<_Ip, _Tp>&>(__t))...);
    }
};

template <class _Tp> struct __is_tuple_like : false_type { };
template <class... _Tp> struct __is_tuple_like<tuple<_Tp...> > : true_type { };
template <class _T1, class _T2> struct __is_tuple_like<pair<_T1, _T2> > : true_type { };

}   // namespace __rw_c11

// [tuple.helper]
template <class... _Tp>
struct tuple_size<tuple<_Tp...> > : integral_constant<size_t, sizeof... (_Tp)> { };

namespace __rw_c11 {
template <size_t _Ip, class... _Tp> struct __type_at { };   // out of range
template <class _Hp, class... _Tp> struct __type_at<0, _Hp, _Tp...> { typedef _Hp type; };
template <size_t _Ip, class _Hp, class... _Tp>
struct __type_at<_Ip, _Hp, _Tp...> : __type_at<_Ip - 1, _Tp...> { };
}

template <size_t _Ip, class... _Tp>
struct tuple_element<_Ip, tuple<_Tp...> >
{
    static_assert (_Ip < sizeof... (_Tp), "tuple_element index out of range");
    typedef typename __rw_c11::__type_at<_Ip, _Tp...>::type type;
};

template <size_t _Ip, class... _Tp>
constexpr typename tuple_element<_Ip, tuple<_Tp...> >::type&
get (tuple<_Tp...>&) noexcept;
template <size_t _Ip, class... _Tp>
constexpr const typename tuple_element<_Ip, tuple<_Tp...> >::type&
get (const tuple<_Tp...>&) noexcept;
template <size_t _Ip, class... _Tp>
constexpr typename tuple_element<_Ip, tuple<_Tp...> >::type&&
get (tuple<_Tp...>&&) noexcept;

namespace __rw_c11 {
// element-wise checks, false (and not expanded) for packs of other lengths
template <bool, class _TT, class _UT> struct __all_constructible : false_type { };
template <class... _Tp, class... _Up>
struct __all_constructible<true, __tuple_types<_Tp...>, __tuple_types<_Up...> >
    : __all<is_constructible<_Tp, _Up>::value...> { };
template <bool, class _TT, class _UT> struct __all_convertible : false_type { };
template <class... _Tp, class... _Up>
struct __all_convertible<true, __tuple_types<_Tp...>, __tuple_types<_Up...> >
    : __all<is_convertible<_Up, _Tp>::value...> { };

template <bool, class _TT, class _UT> struct __all_assignable : false_type { };
template <class... _Tp, class... _Up>
struct __all_assignable<true, __tuple_types<_Tp...>, __tuple_types<_Up...> >
    : __all<is_assignable<_Tp, _Up>::value...> { };

template <class... _Tp> struct __tuple_ctor_checks
{
    template <class... _Up> struct __assignable
        : __all_assignable<sizeof... (_Up) == sizeof... (_Tp),
                           __tuple_types<_Tp&...>, __tuple_types<_Up...> > { };
    template <class... _Up> struct __constructible
        : __all_constructible<sizeof... (_Up) == sizeof... (_Tp),
                              __tuple_types<_Tp...>, __tuple_types<_Up...> > { };
    template <class... _Up> struct __convertible
        : __all_convertible<sizeof... (_Up) == sizeof... (_Tp),
                            __tuple_types<_Tp...>, __tuple_types<_Up...> > { };
};
}   // namespace __rw_c11

// [tuple.tuple]
template <class... _Tp>
class tuple
{
    typedef __rw_c11::__tuple_impl<
        typename __rw_c11::__make_tuple_indices<sizeof... (_Tp)>::type,
        _Tp...> _Impl;
    typedef typename _Impl::__from_values __from_values;
    _Impl __impl;

    template <size_t _Jp, class... _Up> friend constexpr
    typename tuple_element<_Jp, tuple<_Up...> >::type& get (tuple<_Up...>&) noexcept;
    template <size_t _Jp, class... _Up> friend constexpr
    const typename tuple_element<_Jp, tuple<_Up...> >::type&
    get (const tuple<_Up...>&) noexcept;
    template <size_t _Jp, class... _Up> friend constexpr
    typename tuple_element<_Jp, tuple<_Up...> >::type&& get (tuple<_Up...>&&) noexcept;

    template <class... _Up> friend class tuple;

    typedef __rw_c11::__tuple_ctor_checks<_Tp...> _Checks;

    // the conversion from tuple<_Up...> (or tuple<_Up...>&&) is not used
    // when it would be the copy/move constructor, or for tuple<T> made from
    // a tuple<U> T converts from (LWG 2549)
    template <class _Tuple, class... _Up>
    struct __convert_ok
        : integral_constant<bool, sizeof... (_Up) == sizeof... (_Tp)
                                  && !is_same<tuple<_Up...>, tuple>::value> { };

    template <class _Tuple, class _U0>
    struct __single_ok
        : integral_constant<bool,
              !(is_convertible<_Tuple, typename __rw_c11::__type_at<0, _Tp...>::type>::value
                || is_constructible<typename __rw_c11::__type_at<0, _Tp...>::type,
                                    _Tuple>::value
                || is_same<typename __rw_c11::__type_at<0, _Tp...>::type, _U0>::value)> { };

    template <class _Tuple, class... _Up>
    struct __tuple_conv_ok : true_type { };

    template <class _Tuple, class _U0>
    struct __tuple_conv_ok<_Tuple, _U0>
        : integral_constant<bool, sizeof... (_Tp) != 1 || __single_ok<_Tuple, _U0>::value> { };

public:
    // [tuple.cnstr] 4-5
    template <bool _Dummy = true, class = typename enable_if<
        _Dummy && __rw_c11::__all<is_default_constructible<_Tp>::value...>::value>::type>
    constexpr tuple () noexcept (__rw_c11::__all<
        is_nothrow_default_constructible<_Tp>::value...>::value) { }

    tuple (const tuple&) = default;
    tuple (tuple&&) = default;

    // [tuple.cnstr] 6-8: implicit when all are convertible
    template <bool _Dummy = true, typename enable_if<
        _Dummy && (sizeof... (_Tp) > 0)
        && _Checks::template __constructible<const _Tp&...>::value
        && _Checks::template __convertible<const _Tp&...>::value, int>::type = 0>
    constexpr tuple (const _Tp&... __t)
        noexcept (__rw_c11::__all<is_nothrow_copy_constructible<_Tp>::value...>::value)
        : __impl (__from_values (), __t...) { }

    template <bool _Dummy = true, typename enable_if<
        _Dummy && (sizeof... (_Tp) > 0)
        && _Checks::template __constructible<const _Tp&...>::value
        && !_Checks::template __convertible<const _Tp&...>::value, int>::type = 0>
    explicit constexpr tuple (const _Tp&... __t)
        noexcept (__rw_c11::__all<is_nothrow_copy_constructible<_Tp>::value...>::value)
        : __impl (__from_values (), __t...) { }

    // [tuple.cnstr] 9-11
    template <class... _Up, typename enable_if<
        sizeof... (_Up) == sizeof... (_Tp) && (sizeof... (_Up) > 0)
        && !(sizeof... (_Up) == 1
             && is_same<typename decay<typename __rw_c11::__type_at<0, _Up...>::type>::type,
                        tuple>::value)
        && _Checks::template __constructible<_Up&&...>::value
        && _Checks::template __convertible<_Up&&...>::value, int>::type = 0>
    constexpr tuple (_Up&&... __u)
        noexcept (__rw_c11::__all<is_nothrow_constructible<_Tp, _Up>::value...>::value)
        : __impl (__from_values (), std::forward<_Up>(__u)...) { }

    template <class... _Up, typename enable_if<
        sizeof... (_Up) == sizeof... (_Tp) && (sizeof... (_Up) > 0)
        && !(sizeof... (_Up) == 1
             && is_same<typename decay<typename __rw_c11::__type_at<0, _Up...>::type>::type,
                        tuple>::value)
        && _Checks::template __constructible<_Up&&...>::value
        && !_Checks::template __convertible<_Up&&...>::value, int>::type = 0>
    explicit constexpr tuple (_Up&&... __u)
        noexcept (__rw_c11::__all<is_nothrow_constructible<_Tp, _Up>::value...>::value)
        : __impl (__from_values (), std::forward<_Up>(__u)...) { }

    // [tuple.cnstr] 15-17: from const tuple<U...>&
    template <class... _Up, typename enable_if<
        __convert_ok<const tuple<_Up...>&, _Up...>::value
        && __tuple_conv_ok<const tuple<_Up...>&, _Up...>::value
        && _Checks::template __constructible<const _Up&...>::value
        && _Checks::template __convertible<const _Up&...>::value, int>::type = 0>
    constexpr tuple (const tuple<_Up...>& __t)
        : tuple (__t, typename __rw_c11::__make_tuple_indices<sizeof... (_Up)>::type ()) { }

    template <class... _Up, typename enable_if<
        __convert_ok<const tuple<_Up...>&, _Up...>::value
        && __tuple_conv_ok<const tuple<_Up...>&, _Up...>::value
        && _Checks::template __constructible<const _Up&...>::value
        && !_Checks::template __convertible<const _Up&...>::value, int>::type = 0>
    explicit constexpr tuple (const tuple<_Up...>& __t)
        : tuple (__t, typename __rw_c11::__make_tuple_indices<sizeof... (_Up)>::type ()) { }

    // [tuple.cnstr] 18-20: from tuple<U...>&&
    template <class... _Up, typename enable_if<
        __convert_ok<tuple<_Up...>&&, _Up...>::value
        && __tuple_conv_ok<tuple<_Up...>&&, _Up...>::value
        && _Checks::template __constructible<_Up&&...>::value
        && _Checks::template __convertible<_Up&&...>::value, int>::type = 0>
    constexpr tuple (tuple<_Up...>&& __t)
        : tuple (std::move (__t),
                 typename __rw_c11::__make_tuple_indices<sizeof... (_Up)>::type ()) { }

    template <class... _Up, typename enable_if<
        __convert_ok<tuple<_Up...>&&, _Up...>::value
        && __tuple_conv_ok<tuple<_Up...>&&, _Up...>::value
        && _Checks::template __constructible<_Up&&...>::value
        && !_Checks::template __convertible<_Up&&...>::value, int>::type = 0>
    explicit constexpr tuple (tuple<_Up...>&& __t)
        : tuple (std::move (__t),
                 typename __rw_c11::__make_tuple_indices<sizeof... (_Up)>::type ()) { }

    // [tuple.cnstr] 21-26: from pairs
    template <class _U1, class _U2, typename enable_if<
        sizeof... (_Tp) == 2
        && _Checks::template __constructible<const _U1&, const _U2&>::value
        && _Checks::template __convertible<const _U1&, const _U2&>::value, int>::type = 0>
    constexpr tuple (const pair<_U1, _U2>& __p)
        : __impl (__from_values (), __p.first, __p.second) { }

    template <class _U1, class _U2, typename enable_if<
        sizeof... (_Tp) == 2
        && _Checks::template __constructible<const _U1&, const _U2&>::value
        && !_Checks::template __convertible<const _U1&, const _U2&>::value, int>::type = 0>
    explicit constexpr tuple (const pair<_U1, _U2>& __p)
        : __impl (__from_values (), __p.first, __p.second) { }

    template <class _U1, class _U2, typename enable_if<
        sizeof... (_Tp) == 2
        && _Checks::template __constructible<_U1&&, _U2&&>::value
        && _Checks::template __convertible<_U1&&, _U2&&>::value, int>::type = 0>
    constexpr tuple (pair<_U1, _U2>&& __p)
        : __impl (__from_values (), std::forward<_U1>(__p.first),
                  std::forward<_U2>(__p.second)) { }

    template <class _U1, class _U2, typename enable_if<
        sizeof... (_Tp) == 2
        && _Checks::template __constructible<_U1&&, _U2&&>::value
        && !_Checks::template __convertible<_U1&&, _U2&&>::value, int>::type = 0>
    explicit constexpr tuple (pair<_U1, _U2>&& __p)
        : __impl (__from_values (), std::forward<_U1>(__p.first),
                  std::forward<_U2>(__p.second)) { }

    // [tuple.cnstr] 25-26: allocator-extended
    template <class _Alloc, bool _Dummy = true, class = typename enable_if<
        _Dummy && __rw_c11::__all<is_default_constructible<_Tp>::value...>::value>::type>
    tuple (allocator_arg_t, const _Alloc& __a)
        : __impl (typename _Impl::__alloc_default (), __a) { }

    template <class _Alloc, bool _Dummy = true, class = typename enable_if<
        _Dummy && (sizeof... (_Tp) > 0)
        && _Checks::template __constructible<const _Tp&...>::value>::type>
    tuple (allocator_arg_t, const _Alloc& __a, const _Tp&... __t)
        : __impl (allocator_arg_t (), __a, __t...) { }

    template <class _Alloc, class... _Up, class = typename enable_if<
        sizeof... (_Up) == sizeof... (_Tp) && (sizeof... (_Up) > 0)
        && _Checks::template __constructible<_Up&&...>::value>::type>
    tuple (allocator_arg_t, const _Alloc& __a, _Up&&... __u)
        : __impl (allocator_arg_t (), __a, std::forward<_Up>(__u)...) { }

    template <class _Alloc>
    tuple (allocator_arg_t, const _Alloc& __a, const tuple& __t)
        : tuple (allocator_arg_t (), __a, __t,
                 typename __rw_c11::__make_tuple_indices<sizeof... (_Tp)>::type ()) { }

    template <class _Alloc>
    tuple (allocator_arg_t, const _Alloc& __a, tuple&& __t)
        : tuple (allocator_arg_t (), __a, std::move (__t),
                 typename __rw_c11::__make_tuple_indices<sizeof... (_Tp)>::type ()) { }

    template <class _Alloc, class... _Up, class = typename enable_if<
        __convert_ok<const tuple<_Up...>&, _Up...>::value
        && _Checks::template __constructible<const _Up&...>::value>::type>
    tuple (allocator_arg_t, const _Alloc& __a, const tuple<_Up...>& __t)
        : tuple (allocator_arg_t (), __a, __t,
                 typename __rw_c11::__make_tuple_indices<sizeof... (_Up)>::type ()) { }

    template <class _Alloc, class... _Up, class = typename enable_if<
        __convert_ok<tuple<_Up...>&&, _Up...>::value
        && _Checks::template __constructible<_Up&&...>::value>::type>
    tuple (allocator_arg_t, const _Alloc& __a, tuple<_Up...>&& __t)
        : tuple (allocator_arg_t (), __a, std::move (__t),
                 typename __rw_c11::__make_tuple_indices<sizeof... (_Up)>::type ()) { }

    template <class _Alloc, class _U1, class _U2, class = typename enable_if<
        sizeof... (_Tp) == 2
        && _Checks::template __constructible<const _U1&, const _U2&>::value>::type>
    tuple (allocator_arg_t, const _Alloc& __a, const pair<_U1, _U2>& __p)
        : __impl (allocator_arg_t (), __a, __p.first, __p.second) { }

    template <class _Alloc, class _U1, class _U2, class = typename enable_if<
        sizeof... (_Tp) == 2
        && _Checks::template __constructible<_U1&&, _U2&&>::value>::type>
    tuple (allocator_arg_t, const _Alloc& __a, pair<_U1, _U2>&& __p)
        : __impl (allocator_arg_t (), __a, std::forward<_U1>(__p.first),
                  std::forward<_U2>(__p.second)) { }

    // [tuple.assign]: element by element, also for references
    tuple& operator= (typename conditional<
            __rw_c11::__all<is_copy_assignable<_Tp>::value...>::value,
            const tuple&, const __rw_c11::__nat_pair&>::type __t)
    {
        __assign (__t, typename __rw_c11::__make_tuple_indices<sizeof... (_Tp)>::type ());
        return *this;
    }

    tuple& operator= (typename conditional<
            __rw_c11::__all<is_move_assignable<_Tp>::value...>::value,
            tuple&&, __rw_c11::__nat_pair&&>::type __t)
        noexcept (__rw_c11::__all<is_nothrow_move_assignable<_Tp>::value...>::value)
    {
        __move_assign (__t, typename __rw_c11::__make_tuple_indices<sizeof... (_Tp)>::type ());
        return *this;
    }

    template <class... _Up, class = typename enable_if<
        _Checks::template __assignable<const _Up&...>::value>::type>
    tuple& operator= (const tuple<_Up...>& __t)
    {
        __assign (__t, typename __rw_c11::__make_tuple_indices<sizeof... (_Tp)>::type ());
        return *this;
    }

    template <class... _Up, class = typename enable_if<
        _Checks::template __assignable<_Up&&...>::value>::type>
    tuple& operator= (tuple<_Up...>&& __t)
    {
        __move_assign (__t, typename __rw_c11::__make_tuple_indices<sizeof... (_Tp)>::type ());
        return *this;
    }

    template <class _U1, class _U2, class = typename enable_if<
        __rw_c11::__dep_bool<sizeof... (_Tp) == 2, _U1>::value>::type>
    tuple& operator= (const pair<_U1, _U2>& __p)
    {
        std::get<0>(*this) = __p.first;
        std::get<1>(*this) = __p.second;
        return *this;
    }

    template <class _U1, class _U2, class = typename enable_if<
        __rw_c11::__dep_bool<sizeof... (_Tp) == 2, _U1>::value>::type>
    tuple& operator= (pair<_U1, _U2>&& __p)
    {
        std::get<0>(*this) = std::forward<_U1>(__p.first);
        std::get<1>(*this) = std::forward<_U2>(__p.second);
        return *this;
    }

    // [tuple.swap]
    void swap (tuple& __t)
        noexcept (__rw_c11::__all<__rw_c11::__is_nothrow_swappable<_Tp>::value...>::value)
    {
        __impl.__swap (__t.__impl);
    }

private:
    template <class _Tuple, size_t... _Ip>
    constexpr tuple (_Tuple&& __t, __rw_c11::__tuple_indices<_Ip...>)
        : __impl (__from_values (),
                  std::get<_Ip>(std::forward<_Tuple>(__t))...) { }

    template <class _Alloc, class _Tuple, size_t... _Ip>
    tuple (allocator_arg_t, const _Alloc& __a, _Tuple&& __t,
           __rw_c11::__tuple_indices<_Ip...>)
        : __impl (allocator_arg_t (), __a,
                  std::get<_Ip>(std::forward<_Tuple>(__t))...) { }

    template <class _Tuple, size_t... _Ip>
    void __assign (const _Tuple& __t, __rw_c11::__tuple_indices<_Ip...>)
    {
        __rw_c11::__swallow ((std::get<_Ip>(*this) = std::get<_Ip>(__t), 0)...);
    }

    template <class... _Up, size_t... _Ip>
    void __move_assign (tuple<_Up...>& __t, __rw_c11::__tuple_indices<_Ip...>)
    {
        __rw_c11::__swallow ((std::get<_Ip>(*this) =
            std::forward<typename __rw_c11::__type_at<_Ip, _Up...>::type>(
                std::get<_Ip>(__t)), 0)...);
    }
};

template <>
class tuple<>
{
public:
    constexpr tuple () noexcept { }
    template <class _Alloc>
    tuple (allocator_arg_t, const _Alloc&) noexcept { }
    template <class _Alloc>
    tuple (allocator_arg_t, const _Alloc&, const tuple&) noexcept { }
    void swap (tuple&) noexcept { }
};

// [tuple.elem]
template <size_t _Ip, class... _Tp>
inline constexpr typename tuple_element<_Ip, tuple<_Tp...> >::type&
get (tuple<_Tp...>& __t) noexcept
{
    typedef typename tuple_element<_Ip, tuple<_Tp...> >::type _Type;
    return static_cast<__rw_c11::__tuple_leaf<_Ip, _Type>&>(__t.__impl).__get ();
}

template <size_t _Ip, class... _Tp>
inline constexpr const typename tuple_element<_Ip, tuple<_Tp...> >::type&
get (const tuple<_Tp...>& __t) noexcept
{
    typedef typename tuple_element<_Ip, tuple<_Tp...> >::type _Type;
    return static_cast<const __rw_c11::__tuple_leaf<_Ip, _Type>&>(__t.__impl).__get ();
}

template <size_t _Ip, class... _Tp>
inline constexpr typename tuple_element<_Ip, tuple<_Tp...> >::type&&
get (tuple<_Tp...>&& __t) noexcept
{
    typedef typename tuple_element<_Ip, tuple<_Tp...> >::type _Type;
    return static_cast<_Type&&>(
        static_cast<__rw_c11::__tuple_leaf<_Ip, _Type>&>(__t.__impl).__get ());
}

template <size_t _Ip, class... _Tp>
inline constexpr const typename tuple_element<_Ip, tuple<_Tp...> >::type&&
get (const tuple<_Tp...>&& __t) noexcept
{
    return static_cast<const typename tuple_element<_Ip, tuple<_Tp...> >::type&&>(
        std::get<_Ip>(__t));
}

#if __cplusplus >= 201402L
namespace __rw_c11 {
template <class _Tp, class... _Types> struct __find_exactly_one;
template <class _Tp> struct __find_exactly_one<_Tp>
{
    static constexpr size_t value = 0;
    static constexpr size_t __count = 0;
};
template <class _Tp, class _Hp, class... _Types>
struct __find_exactly_one<_Tp, _Hp, _Types...>
{
    typedef __find_exactly_one<_Tp, _Types...> _Rest;
    static constexpr size_t __count = (is_same<_Tp, _Hp>::value ? 1 : 0) + _Rest::__count;
    static constexpr size_t value = is_same<_Tp, _Hp>::value ? 0 : 1 + _Rest::value;
};
template <class _Tp, class... _Types> struct __index_of
{
    typedef __find_exactly_one<_Tp, _Types...> _Find;
    static_assert (_Find::__count == 1, "type must occur exactly once in the tuple");
    static constexpr size_t value = _Find::value;
};
}

template <class _T1, class... _Types>
inline constexpr _T1& get (tuple<_Types...>& __t) noexcept
{ return std::get<__rw_c11::__index_of<_T1, _Types...>::value>(__t); }
template <class _T1, class... _Types>
inline constexpr const _T1& get (const tuple<_Types...>& __t) noexcept
{ return std::get<__rw_c11::__index_of<_T1, _Types...>::value>(__t); }
template <class _T1, class... _Types>
inline constexpr _T1&& get (tuple<_Types...>&& __t) noexcept
{ return std::get<__rw_c11::__index_of<_T1, _Types...>::value>(std::move (__t)); }
template <class _T1, class... _Types>
inline constexpr const _T1&& get (const tuple<_Types...>&& __t) noexcept
{ return std::get<__rw_c11::__index_of<_T1, _Types...>::value>(std::move (__t)); }
#endif   // C++14

// [tuple.creation]
namespace __rw_c11 {
struct __ignore_t
{
    template <class _Tp>
    const __ignore_t& operator= (_Tp&&) const { return *this; }
};
}
constexpr const __rw_c11::__ignore_t ignore = __rw_c11::__ignore_t ();

template <class... _Tp>
inline constexpr tuple<typename __rw_c11::__unwrap_ref_decay<_Tp>::type...>
make_tuple (_Tp&&... __t)
{
    return tuple<typename __rw_c11::__unwrap_ref_decay<_Tp>::type...>(
        std::forward<_Tp>(__t)...);
}

template <class... _Tp>
inline constexpr tuple<_Tp&&...> forward_as_tuple (_Tp&&... __t) noexcept
{
    return tuple<_Tp&&...>(std::forward<_Tp>(__t)...);
}

template <class... _Tp>
inline constexpr tuple<_Tp&...> tie (_Tp&... __t) noexcept
{
    return tuple<_Tp&...>(__t...);
}

// tuple_cat: the elements of tuple-like arguments (tuple, pair, array)
namespace __rw_c11 {
template <class... _Tp> struct __tuple_types;
template <class _Tp> struct __as_types;
template <class... _Tp> struct __as_types<tuple<_Tp...> > { typedef __tuple_types<_Tp...> type; };
template <class _T1, class _T2> struct __as_types<pair<_T1, _T2> >
{ typedef __tuple_types<_T1, _T2> type; };

template <class _Tp, class _Seq> struct __array_types;
template <class _Tp, size_t... _Ip> struct __array_types<_Tp, __tuple_indices<_Ip...> >
{
    template <size_t> struct __same { typedef _Tp type; };
    typedef __tuple_types<typename __same<_Ip>::type...> type;
};

}   // namespace __rw_c11
template <class _Tp, size_t _Np> struct array;

// Make these overloads visible to the qualified get calls in tuple_cat,
// even when <array> is included after <tuple>.
template <size_t _Ip, class _Tp, size_t _Np>
constexpr _Tp& get (array<_Tp, _Np>&) noexcept;
template <size_t _Ip, class _Tp, size_t _Np>
constexpr const _Tp& get (const array<_Tp, _Np>&) noexcept;
template <size_t _Ip, class _Tp, size_t _Np>
constexpr _Tp&& get (array<_Tp, _Np>&&) noexcept;
template <size_t _Ip, class _Tp, size_t _Np>
constexpr const _Tp&& get (const array<_Tp, _Np>&&) noexcept;

namespace __rw_c11 {
template <class _Tp, size_t _Np> struct __as_types<array<_Tp, _Np> >
    : __array_types<_Tp, typename __make_tuple_indices<_Np>::type> { };

template <class _A, class _B> struct __cat_types;
template <class... _Ap, class... _Bp>
struct __cat_types<__tuple_types<_Ap...>, __tuple_types<_Bp...> >
{ typedef __tuple_types<_Ap..., _Bp...> type; };

template <class _Types> struct __to_tuple;
template <class... _Tp> struct __to_tuple<__tuple_types<_Tp...> > { typedef tuple<_Tp...> type; };

template <class... _Tuples> struct __tuple_cat_types;
template <> struct __tuple_cat_types<> { typedef __tuple_types<> type; };
template <class _T0, class... _Tuples> struct __tuple_cat_types<_T0, _Tuples...>
{
    typedef typename __cat_types<
        typename __as_types<typename remove_cv<typename remove_reference<_T0>::type>::type>::type,
        typename __tuple_cat_types<_Tuples...>::type>::type type;
};

template <class _Acc, class... _Tuples> struct __tuple_cat_impl;
template <class _Acc> struct __tuple_cat_impl<_Acc>
{
    static _RWSTD_CONSTEXPR14 _Acc __cat (_Acc&& __acc) { return std::move (__acc); }
};

template <class... _Ap, class _T0, class... _Tuples>
struct __tuple_cat_impl<tuple<_Ap...>, _T0, _Tuples...>
{
    typedef typename remove_cv<typename remove_reference<_T0>::type>::type _Raw0;
    typedef typename __to_tuple<typename __cat_types<__tuple_types<_Ap...>,
        typename __as_types<_Raw0>::type>::type>::type _Next;
    typedef typename __to_tuple<typename __cat_types<__tuple_types<_Ap...>,
        typename __tuple_cat_types<_T0, _Tuples...>::type>::type>::type _Result;

    template <size_t... _Ip, size_t... _Jp>
    static _RWSTD_CONSTEXPR14 _Next __step (tuple<_Ap...>&& __acc, _T0&& __t0,
                         __tuple_indices<_Ip...>, __tuple_indices<_Jp...>)
    {
        return _Next (std::get<_Ip>(std::move (__acc))...,
                      std::get<_Jp>(std::forward<_T0>(__t0))...);
    }

    static _RWSTD_CONSTEXPR14 _Result __cat (tuple<_Ap...>&& __acc, _T0&& __t0, _Tuples&&... __rest)
    {
        return __tuple_cat_impl<_Next, _Tuples...>::__cat (
            __step (std::move (__acc), std::forward<_T0>(__t0),
                    typename __make_tuple_indices<sizeof... (_Ap)>::type (),
                    typename __make_tuple_indices<tuple_size<_Raw0>::value>::type ()),
            std::forward<_Tuples>(__rest)...);
    }
};
}   // namespace __rw_c11

template <class... _Tuples>
inline _RWSTD_CONSTEXPR14 typename __rw_c11::__to_tuple<
    typename __rw_c11::__tuple_cat_types<_Tuples...>::type>::type
tuple_cat (_Tuples&&... __tpls)
{
    return __rw_c11::__tuple_cat_impl<tuple<>, _Tuples...>::__cat (
        tuple<> (), std::forward<_Tuples>(__tpls)...);
}

// [tuple.rel]
namespace __rw_c11 {
template <size_t _Ip>
struct __tuple_compare
{
    template <class _Tp, class _Up>
    static constexpr bool __eq (const _Tp& __x, const _Up& __y)
    {
        return __tuple_compare<_Ip - 1>::__eq (__x, __y)
            && std::get<_Ip - 1>(__x) == std::get<_Ip - 1>(__y);
    }
};
template <> struct __tuple_compare<0>
{
    template <class _Tp, class _Up>
    static constexpr bool __eq (const _Tp&, const _Up&) { return true; }
};

template <size_t _Ip, size_t _Np>
struct __tuple_less
{
    template <class _Tp, class _Up>
    static constexpr bool __lt (const _Tp& __x, const _Up& __y)
    {
        return std::get<_Ip>(__x) < std::get<_Ip>(__y) ? true
             : std::get<_Ip>(__y) < std::get<_Ip>(__x) ? false
             : __tuple_less<_Ip + 1, _Np>::__lt (__x, __y);
    }
};
template <size_t _Np> struct __tuple_less<_Np, _Np>
{
    template <class _Tp, class _Up>
    static constexpr bool __lt (const _Tp&, const _Up&) { return false; }
};
}   // namespace __rw_c11

template <class... _Tp, class... _Up>
inline constexpr bool operator== (const tuple<_Tp...>& __x, const tuple<_Up...>& __y)
{
    static_assert (sizeof... (_Tp) == sizeof... (_Up),
                   "can't compare tuples of different sizes");
    return __rw_c11::__tuple_compare<sizeof... (_Tp)>::__eq (__x, __y);
}

template <class... _Tp, class... _Up>
inline constexpr bool operator!= (const tuple<_Tp...>& __x, const tuple<_Up...>& __y)
{
    return !(__x == __y);
}

template <class... _Tp, class... _Up>
inline constexpr bool operator< (const tuple<_Tp...>& __x, const tuple<_Up...>& __y)
{
    static_assert (sizeof... (_Tp) == sizeof... (_Up),
                   "can't compare tuples of different sizes");
    return __rw_c11::__tuple_less<0, sizeof... (_Tp)>::__lt (__x, __y);
}

template <class... _Tp, class... _Up>
inline constexpr bool operator> (const tuple<_Tp...>& __x, const tuple<_Up...>& __y)
{
    return __y < __x;
}

template <class... _Tp, class... _Up>
inline constexpr bool operator>= (const tuple<_Tp...>& __x, const tuple<_Up...>& __y)
{
    return !(__x < __y);
}

template <class... _Tp, class... _Up>
inline constexpr bool operator<= (const tuple<_Tp...>& __x, const tuple<_Up...>& __y)
{
    return !(__y < __x);
}

// [tuple.special]
template <class... _Tp>
inline typename enable_if<__rw_c11::__all<__rw_c11::__is_swappable<_Tp>::value...>::value>::type
swap (tuple<_Tp...>& __x, tuple<_Tp...>& __y) noexcept (noexcept (__x.swap (__y)))
{
    __x.swap (__y);
}

// [tuple.traits]
template <class... _Tp, class _Alloc>
struct uses_allocator<tuple<_Tp...>, _Alloc> : true_type { };

// [pairs.pair] 18: the piecewise constructor
template <class _T1, class _T2>
template <class... _Args1, class... _Args2>
inline pair<_T1, _T2>::pair (piecewise_construct_t __pc,
                             tuple<_Args1...> __first_args,
                             tuple<_Args2...> __second_args)
    : pair (__pc, __first_args, __second_args,
            typename __rw_c11::__make_tuple_indices<sizeof... (_Args1)>::type (),
            typename __rw_c11::__make_tuple_indices<sizeof... (_Args2)>::type ())
{ }

template <class _T1, class _T2>
template <class... _Args1, class... _Args2, size_t... _I1, size_t... _I2>
inline pair<_T1, _T2>::pair (piecewise_construct_t,
                             tuple<_Args1...>& __first_args,
                             tuple<_Args2...>& __second_args,
                             __rw_c11::__tuple_indices<_I1...>,
                             __rw_c11::__tuple_indices<_I2...>)
    : first (std::forward<_Args1>(std::get<_I1>(__first_args))...),
      second (std::forward<_Args2>(std::get<_I2>(__second_args))...)
{ }

}   // namespace std

#endif   // _RWSTD_RW_C11_TUPLE_H_INCLUDED
