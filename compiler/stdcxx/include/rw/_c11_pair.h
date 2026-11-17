// -*- C++ -*-
/***************************************************************************
 *
 * _c11_pair.h - C++11 and C++14 std::pair [pairs]
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

#ifndef _RWSTD_RW_C11_PAIR_H_INCLUDED
#define _RWSTD_RW_C11_PAIR_H_INCLUDED

#include <rw/_c11_utility.h>

namespace std {

template <class _Tp> class reference_wrapper;

namespace __rw_c11 {
template <bool... _Bp> struct __all;
template <> struct __all<> : true_type { };
template <bool... _Bp> struct __all<true, _Bp...> : __all<_Bp...> { };
template <bool... _Bp> struct __all<false, _Bp...> : false_type { };

// make_pair and make_tuple: reference_wrapper<T> gives T&
template <class _Tp> struct __unwrap_ref { typedef _Tp type; };
template <class _Tp> struct __unwrap_ref<reference_wrapper<_Tp> > { typedef _Tp& type; };
template <class _Tp> struct __unwrap_ref_decay
    : __unwrap_ref<typename decay<_Tp>::type> { };

template <class _Tp> struct __is_pair;

// a "not a type" for disabled overloads that must still be declarable
struct __nat_pair { };

template <size_t...> struct __tuple_indices { };
template <class... _Tp> struct __tuple_types { };
}   // namespace __rw_c11

// [pairs.pair]
template <class _T1, class _T2>
struct pair
{
    typedef _T1 first_type;
    typedef _T2 second_type;

    _T1 first;
    _T2 second;

    pair (const pair&) = default;
    pair (pair&&) = default;

    // [pairs.pair] 2-3
    template <class _U1 = _T1, class _U2 = _T2, class = typename enable_if<
        is_default_constructible<_U1>::value
        && is_default_constructible<_U2>::value>::type>
    constexpr pair () : first (), second () { }

    // [pairs.pair] 6-8: implicit when both are convertible
    template <class _U1 = _T1, class _U2 = _T2, typename enable_if<
        is_copy_constructible<_U1>::value && is_copy_constructible<_U2>::value
        && is_convertible<const _U1&, _U1>::value
        && is_convertible<const _U2&, _U2>::value, int>::type = 0>
    constexpr pair (const _T1& __x, const _T2& __y)
        : first (__x), second (__y) { }

    template <class _U1 = _T1, class _U2 = _T2, typename enable_if<
        is_copy_constructible<_U1>::value && is_copy_constructible<_U2>::value
        && !(is_convertible<const _U1&, _U1>::value
             && is_convertible<const _U2&, _U2>::value), int>::type = 0>
    explicit constexpr pair (const _T1& __x, const _T2& __y)
        : first (__x), second (__y) { }

    // [pairs.pair] 9-11
    template <class _U1, class _U2, typename enable_if<
        is_constructible<_T1, _U1&&>::value && is_constructible<_T2, _U2&&>::value
        && is_convertible<_U1&&, _T1>::value
        && is_convertible<_U2&&, _T2>::value, int>::type = 0>
    constexpr pair (_U1&& __x, _U2&& __y)
        : first (std::forward<_U1>(__x)), second (std::forward<_U2>(__y)) { }

    template <class _U1, class _U2, typename enable_if<
        is_constructible<_T1, _U1&&>::value && is_constructible<_T2, _U2&&>::value
        && !(is_convertible<_U1&&, _T1>::value
             && is_convertible<_U2&&, _T2>::value), int>::type = 0>
    explicit constexpr pair (_U1&& __x, _U2&& __y)
        : first (std::forward<_U1>(__x)), second (std::forward<_U2>(__y)) { }

    // [pairs.pair] 12-14
    template <class _U1, class _U2, typename enable_if<
        is_constructible<_T1, const _U1&>::value
        && is_constructible<_T2, const _U2&>::value
        && is_convertible<const _U1&, _T1>::value
        && is_convertible<const _U2&, _T2>::value, int>::type = 0>
    constexpr pair (const pair<_U1, _U2>& __p)
        : first (__p.first), second (__p.second) { }

    template <class _U1, class _U2, typename enable_if<
        is_constructible<_T1, const _U1&>::value
        && is_constructible<_T2, const _U2&>::value
        && !(is_convertible<const _U1&, _T1>::value
             && is_convertible<const _U2&, _T2>::value), int>::type = 0>
    explicit constexpr pair (const pair<_U1, _U2>& __p)
        : first (__p.first), second (__p.second) { }

    // [pairs.pair] 15-17
    template <class _U1, class _U2, typename enable_if<
        is_constructible<_T1, _U1&&>::value && is_constructible<_T2, _U2&&>::value
        && is_convertible<_U1&&, _T1>::value
        && is_convertible<_U2&&, _T2>::value, int>::type = 0>
    constexpr pair (pair<_U1, _U2>&& __p)
        : first (std::forward<_U1>(__p.first)),
          second (std::forward<_U2>(__p.second)) { }

    template <class _U1, class _U2, typename enable_if<
        is_constructible<_T1, _U1&&>::value && is_constructible<_T2, _U2&&>::value
        && !(is_convertible<_U1&&, _T1>::value
             && is_convertible<_U2&&, _T2>::value), int>::type = 0>
    explicit constexpr pair (pair<_U1, _U2>&& __p)
        : first (std::forward<_U1>(__p.first)),
          second (std::forward<_U2>(__p.second)) { }

    // [pairs.pair] 18 (defined in <tuple>)
    template <class... _Args1, class... _Args2>
    pair (piecewise_construct_t, tuple<_Args1...> __first_args,
          tuple<_Args2...> __second_args);

    // [pairs.pair] 19-28: assignment, also of pairs of references; a
    // pair that can't be assigned gets an operator= taking __nat_pair
    pair& operator= (typename conditional<
            is_copy_assignable<_T1>::value && is_copy_assignable<_T2>::value,
            const pair&, const __rw_c11::__nat_pair&>::type __p)
    {
        first = __p.first;
        second = __p.second;
        return *this;
    }

    pair& operator= (typename conditional<
            is_move_assignable<_T1>::value && is_move_assignable<_T2>::value,
            pair&&, __rw_c11::__nat_pair&&>::type __p)
        noexcept (is_nothrow_move_assignable<_T1>::value
                  && is_nothrow_move_assignable<_T2>::value)
    {
        first = std::forward<_T1>(__p.first);
        second = std::forward<_T2>(__p.second);
        return *this;
    }

    template <class _U1, class _U2, class = typename enable_if<
        is_assignable<_T1&, const _U1&>::value
        && is_assignable<_T2&, const _U2&>::value>::type>
    pair& operator= (const pair<_U1, _U2>& __p)
    {
        first = __p.first;
        second = __p.second;
        return *this;
    }

    template <class _U1, class _U2, class = typename enable_if<
        is_assignable<_T1&, _U1&&>::value
        && is_assignable<_T2&, _U2&&>::value>::type>
    pair& operator= (pair<_U1, _U2>&& __p)
    {
        first = std::forward<_U1>(__p.first);
        second = std::forward<_U2>(__p.second);
        return *this;
    }

    void swap (pair& __p)
        noexcept (__rw_c11::__is_nothrow_swappable<_T1>::value
                  && __rw_c11::__is_nothrow_swappable<_T2>::value)
    {
        using std::swap;
        swap (first, __p.first);
        swap (second, __p.second);
    }

private:
    template <class... _Args1, class... _Args2, size_t... _I1, size_t... _I2>
    pair (piecewise_construct_t, tuple<_Args1...>& __first_args,
          tuple<_Args2...>& __second_args,
          __rw_c11::__tuple_indices<_I1...>, __rw_c11::__tuple_indices<_I2...>);
};

// [pairs.spec]
template <class _T1, class _T2>
inline constexpr bool
operator== (const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return __x.first == __y.first && __x.second == __y.second;
}

template <class _T1, class _T2>
inline constexpr bool
operator!= (const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return !(__x == __y);
}

template <class _T1, class _T2>
inline constexpr bool
operator< (const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return __x.first < __y.first
        || (!(__y.first < __x.first) && __x.second < __y.second);
}

template <class _T1, class _T2>
inline constexpr bool
operator> (const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return __y < __x;
}

template <class _T1, class _T2>
inline constexpr bool
operator>= (const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return !(__x < __y);
}

template <class _T1, class _T2>
inline constexpr bool
operator<= (const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
{
    return !(__y < __x);
}

template <class _T1, class _T2>
inline typename enable_if<__rw_c11::__is_swappable<_T1>::value
                          && __rw_c11::__is_swappable<_T2>::value>::type
swap (pair<_T1, _T2>& __x, pair<_T1, _T2>& __y) noexcept (noexcept (__x.swap (__y)))
{
    __x.swap (__y);
}

template <class _T1, class _T2>
inline constexpr pair<typename __rw_c11::__unwrap_ref_decay<_T1>::type,
                      typename __rw_c11::__unwrap_ref_decay<_T2>::type>
make_pair (_T1&& __x, _T2&& __y)
{
    return pair<typename __rw_c11::__unwrap_ref_decay<_T1>::type,
                typename __rw_c11::__unwrap_ref_decay<_T2>::type>
        (std::forward<_T1>(__x), std::forward<_T2>(__y));
}

// [pair.astuple]
template <class _T1, class _T2>
struct tuple_size<pair<_T1, _T2> > : integral_constant<size_t, 2> { };

template <class _T1, class _T2>
struct tuple_element<0, pair<_T1, _T2> > { typedef _T1 type; };
template <class _T1, class _T2>
struct tuple_element<1, pair<_T1, _T2> > { typedef _T2 type; };

namespace __rw_c11 {
template <size_t _Ip> struct __get_pair;
template <> struct __get_pair<0>
{
    template <class _T1, class _T2>
    static constexpr _T1& __get (pair<_T1, _T2>& __p) noexcept { return __p.first; }
    template <class _T1, class _T2>
    static constexpr const _T1& __get (const pair<_T1, _T2>& __p) noexcept
    { return __p.first; }
    template <class _T1, class _T2>
    static constexpr _T1&& __move (pair<_T1, _T2>&& __p) noexcept
    { return std::forward<_T1>(__p.first); }
};
template <> struct __get_pair<1>
{
    template <class _T1, class _T2>
    static constexpr _T2& __get (pair<_T1, _T2>& __p) noexcept { return __p.second; }
    template <class _T1, class _T2>
    static constexpr const _T2& __get (const pair<_T1, _T2>& __p) noexcept
    { return __p.second; }
    template <class _T1, class _T2>
    static constexpr _T2&& __move (pair<_T1, _T2>&& __p) noexcept
    { return std::forward<_T2>(__p.second); }
};
}   // namespace __rw_c11

template <size_t _Ip, class _T1, class _T2>
inline constexpr typename tuple_element<_Ip, pair<_T1, _T2> >::type&
get (pair<_T1, _T2>& __p) noexcept
{
    return __rw_c11::__get_pair<_Ip>::__get (__p);
}

template <size_t _Ip, class _T1, class _T2>
inline constexpr const typename tuple_element<_Ip, pair<_T1, _T2> >::type&
get (const pair<_T1, _T2>& __p) noexcept
{
    return __rw_c11::__get_pair<_Ip>::__get (__p);
}

template <size_t _Ip, class _T1, class _T2>
inline constexpr typename tuple_element<_Ip, pair<_T1, _T2> >::type&&
get (pair<_T1, _T2>&& __p) noexcept
{
    return __rw_c11::__get_pair<_Ip>::__move (std::move (__p));
}

template <size_t _Ip, class _T1, class _T2>
inline constexpr const typename tuple_element<_Ip, pair<_T1, _T2> >::type&&
get (const pair<_T1, _T2>&& __p) noexcept
{
    return static_cast<const typename tuple_element<_Ip, pair<_T1, _T2> >::type&&>(
        std::get<_Ip>(__p));
}

#if __cplusplus >= 201402L
template <class _T1, class _T2>
inline constexpr _T1& get (pair<_T1, _T2>& __p) noexcept { return __p.first; }
template <class _T1, class _T2>
inline constexpr const _T1& get (const pair<_T1, _T2>& __p) noexcept
{ return __p.first; }
template <class _T1, class _T2>
inline constexpr _T1&& get (pair<_T1, _T2>&& __p) noexcept
{ return std::forward<_T1>(__p.first); }
template <class _T1, class _T2>
inline constexpr const _T1&& get (const pair<_T1, _T2>&& __p) noexcept
{ return std::get<0>(std::move (__p)); }
template <class _T1, class _T2>
inline constexpr _T1& get (pair<_T2, _T1>& __p) noexcept { return __p.second; }
template <class _T1, class _T2>
inline constexpr const _T1& get (const pair<_T2, _T1>& __p) noexcept
{ return __p.second; }
template <class _T1, class _T2>
inline constexpr _T1&& get (pair<_T2, _T1>&& __p) noexcept
{ return std::forward<_T1>(__p.second); }
template <class _T1, class _T2>
inline constexpr const _T1&& get (const pair<_T2, _T1>&& __p) noexcept
{ return std::get<1>(std::move (__p)); }
#endif   // C++14

}   // namespace std

#endif   // _RWSTD_RW_C11_PAIR_H_INCLUDED
