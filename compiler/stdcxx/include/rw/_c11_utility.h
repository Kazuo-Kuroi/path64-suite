// -*- C++ -*-
/***************************************************************************
 *
 * _c11_utility.h - C++11 and C++14 parts of <utility> [utility]:
 *                  move, forward, swap, exchange, integer_sequence
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

#ifndef _RWSTD_RW_C11_UTILITY_H_INCLUDED
#define _RWSTD_RW_C11_UTILITY_H_INCLUDED

#include <rw/_defs.h>
#include <rw/_c11_type_traits.h>
#include <initializer_list>

namespace std {

// [forward]
template <class _Tp>
inline constexpr _Tp&&
forward (typename remove_reference<_Tp>::type& __t) noexcept
{
    return static_cast<_Tp&&>(__t);
}

template <class _Tp>
inline constexpr _Tp&&
forward (typename remove_reference<_Tp>::type&& __t) noexcept
{
    static_assert (!is_lvalue_reference<_Tp>::value,
                   "can not forward an rvalue as an lvalue");
    return static_cast<_Tp&&>(__t);
}

template <class _Tp>
inline constexpr typename remove_reference<_Tp>::type&&
move (_Tp&& __t) noexcept
{
    return static_cast<typename remove_reference<_Tp>::type&&>(__t);
}

template <class _Tp>
inline constexpr typename conditional<
    !is_nothrow_move_constructible<_Tp>::value && is_copy_constructible<_Tp>::value,
    const _Tp&, _Tp&&>::type
move_if_noexcept (_Tp& __x) noexcept
{
    return std::move (__x);
}

// [utility.swap]
template <class _Tp>
inline typename enable_if<is_move_constructible<_Tp>::value
                          && is_move_assignable<_Tp>::value>::type
swap (_Tp& __a, _Tp& __b)
    noexcept (is_nothrow_move_constructible<_Tp>::value
              && is_nothrow_move_assignable<_Tp>::value)
{
    _Tp __tmp (std::move (__a));
    __a = std::move (__b);
    __b = std::move (__tmp);
}

namespace __rw_c11 {
// is swap(T&, T&) (std::swap or one found by ADL) valid and noexcept
using std::swap;
template <class _Tp, class = decltype (swap (declval<_Tp&>(), declval<_Tp&>()))>
true_type __swappable_test (int);
template <class _Tp> false_type __swappable_test (...);
template <class _Tp> struct __is_swappable : decltype (__swappable_test<_Tp>(0)) { };

template <class _Tp, bool = __is_swappable<_Tp>::value>
struct __is_nothrow_swappable
    : integral_constant<bool, noexcept (swap (declval<_Tp&>(), declval<_Tp&>()))> { };
template <class _Tp> struct __is_nothrow_swappable<_Tp, false> : false_type { };
}   // namespace __rw_c11

template <class _Tp, size_t _Np>
inline typename enable_if<__rw_c11::__is_swappable<_Tp>::value>::type
swap (_Tp (&__a)[_Np], _Tp (&__b)[_Np])
    noexcept (__rw_c11::__is_nothrow_swappable<_Tp>::value)
{
    for (size_t __i = 0; __i != _Np; ++__i) {
        using std::swap;
        swap (__a [__i], __b [__i]);
    }
}

#if __cplusplus >= 201402L
// [utility.exchange]
template <class _Tp, class _Up = _Tp>
inline _Tp exchange (_Tp& __obj, _Up&& __new_val)
{
    _Tp __old = std::move (__obj);
    __obj = std::forward<_Up>(__new_val);
    return __old;
}

// [intseq]
template <class _Tp, _Tp... _Ip>
struct integer_sequence
{
    static_assert (is_integral<_Tp>::value,
                   "integer_sequence needs an integral type");
    typedef _Tp value_type;
    static constexpr size_t size () noexcept { return sizeof... (_Ip); }
};

template <size_t... _Ip>
using index_sequence = integer_sequence<size_t, _Ip...>;

namespace __rw_c11 {
// 0 ... N-1 in log N steps: double the sequence, then add one if odd
template <class _Seq, size_t _Np> struct __double_seq;
template <size_t... _Ip, size_t _Np>
struct __double_seq<index_sequence<_Ip...>, _Np>
{ typedef index_sequence<_Ip..., (_Ip + _Np)...> type; };
template <class _Seq, size_t _Np> struct __add_one;
template <size_t... _Ip, size_t _Np>
struct __add_one<index_sequence<_Ip...>, _Np>
{ typedef index_sequence<_Ip..., _Np> type; };

template <size_t _Np> struct __make_index_seq
{
    typedef typename __double_seq<typename __make_index_seq<_Np / 2>::type,
                                  _Np / 2>::type _Half;
    typedef typename conditional<_Np % 2 == 0, _Half,
        typename __add_one<_Half, _Np - 1>::type>::type type;
};
template <> struct __make_index_seq<0> { typedef index_sequence<> type; };
template <> struct __make_index_seq<1> { typedef index_sequence<0> type; };

template <class _Tp, class _Seq> struct __convert_seq;
template <class _Tp, size_t... _Ip>
struct __convert_seq<_Tp, index_sequence<_Ip...> >
{ typedef integer_sequence<_Tp, _Tp (_Ip)...> type; };
}   // namespace __rw_c11

template <class _Tp, _Tp _Np>
using make_integer_sequence = typename __rw_c11::__convert_seq<
    _Tp, typename __rw_c11::__make_index_seq<size_t (_Np)>::type>::type;
template <size_t _Np>
using make_index_sequence = make_integer_sequence<size_t, _Np>;
template <class... _Tp>
using index_sequence_for = make_index_sequence<sizeof... (_Tp)>;
#endif   // C++14

// [pair.piecewise]
struct piecewise_construct_t { explicit piecewise_construct_t () = default; };
constexpr piecewise_construct_t piecewise_construct = piecewise_construct_t ();

// [tuple.helper], also for pair and array
template <class _Tp> struct tuple_size;
template <class _Tp> struct tuple_size<const _Tp>
    : integral_constant<size_t, tuple_size<_Tp>::value> { };
template <class _Tp> struct tuple_size<volatile _Tp>
    : integral_constant<size_t, tuple_size<_Tp>::value> { };
template <class _Tp> struct tuple_size<const volatile _Tp>
    : integral_constant<size_t, tuple_size<_Tp>::value> { };

template <size_t _Ip, class _Tp> struct tuple_element;
template <size_t _Ip, class _Tp> struct tuple_element<_Ip, const _Tp>
{ typedef typename add_const<typename tuple_element<_Ip, _Tp>::type>::type type; };
template <size_t _Ip, class _Tp> struct tuple_element<_Ip, volatile _Tp>
{ typedef typename add_volatile<typename tuple_element<_Ip, _Tp>::type>::type type; };
template <size_t _Ip, class _Tp> struct tuple_element<_Ip, const volatile _Tp>
{ typedef typename add_cv<typename tuple_element<_Ip, _Tp>::type>::type type; };

#if __cplusplus >= 201402L
template <size_t _Ip, class _Tp>
using tuple_element_t = typename tuple_element<_Ip, _Tp>::type;
#endif

template <class... _Tp> class tuple;
template <class _Tp> struct hash;   // [unord.hash], in <functional>

}   // namespace std

#endif   // _RWSTD_RW_C11_UTILITY_H_INCLUDED
