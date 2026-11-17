// -*- C++ -*-
/***************************************************************************
 *
 * _c11_array.h - C++11 and C++14 <array> [array]
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

#ifndef _RWSTD_RW_C11_ARRAY_H_INCLUDED
#define _RWSTD_RW_C11_ARRAY_H_INCLUDED

#include <rw/_c11_utility.h>
#include <rw/_iterator.h>      // reverse_iterator
#include <rw/_algobase.h>      // equal, lexicographical_compare, fill_n, swap_ranges
#include <stdexcept>

namespace std {

namespace __rw_c11 {
[[noreturn]] inline void __throw_array_range ()
{
#ifndef _RWSTD_NO_EXCEPTIONS
    throw out_of_range ("array::at");
#else
    __builtin_abort ();
#endif
}

// the storage: T[N], or for N == 0 something that is not a T
template <class _Tp, size_t _Np> struct __array_traits
{
    typedef _Tp _Type[_Np];
    static constexpr _Tp& __ref (const _Type& __t, size_t __n) noexcept
    { return const_cast<_Tp&>(__t [__n]); }
    static constexpr _Tp* __ptr (const _Type& __t) noexcept
    { return const_cast<_Tp*>(__t); }
};
template <class _Tp> struct __array_traits<_Tp, 0>
{
    struct _Type { };
    static constexpr _Tp& __ref (const _Type&, size_t) noexcept
    { return *static_cast<_Tp*>(0); }
    static constexpr _Tp* __ptr (const _Type&) noexcept { return 0; }
};
}   // namespace __rw_c11

template <class _Tp, size_t _Size>
struct array
{
    typedef _Tp                                   value_type;
    typedef value_type&                           reference;
    typedef const value_type&                     const_reference;
    typedef value_type*                           iterator;
    typedef const value_type*                     const_iterator;
    typedef value_type*                           pointer;
    typedef const value_type*                     const_pointer;
    typedef size_t                                size_type;
    typedef ptrdiff_t                             difference_type;
    typedef std::reverse_iterator<iterator>       reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef __rw_c11::__array_traits<_Tp, _Size> _Traits;
    typename _Traits::_Type __elems;   // public: array is an aggregate

    // [array.fill], [array.swap]
    void fill (const value_type& __u) { std::fill_n (begin (), _Size, __u); }
    void swap (array& __a) noexcept (__rw_c11::__is_nothrow_swappable<_Tp>::value)
    {
        for (size_t __i = 0; __i != _Size; ++__i) {
            using std::swap;
            swap ((*this)[__i], __a [__i]);
        }
    }

    // iterators
    _RWSTD_CONSTEXPR14 iterator begin () noexcept { return data (); }
    constexpr const_iterator begin () const noexcept { return data (); }
    _RWSTD_CONSTEXPR14 iterator end () noexcept { return data () + _Size; }
    constexpr const_iterator end () const noexcept { return data () + _Size; }
    reverse_iterator rbegin () noexcept { return reverse_iterator (end ()); }
    const_reverse_iterator rbegin () const noexcept { return const_reverse_iterator (end ()); }
    reverse_iterator rend () noexcept { return reverse_iterator (begin ()); }
    const_reverse_iterator rend () const noexcept { return const_reverse_iterator (begin ()); }
    constexpr const_iterator cbegin () const noexcept { return begin (); }
    constexpr const_iterator cend () const noexcept { return end (); }
    const_reverse_iterator crbegin () const noexcept { return rbegin (); }
    const_reverse_iterator crend () const noexcept { return rend (); }

    // capacity
    constexpr size_type size () const noexcept { return _Size; }
    constexpr size_type max_size () const noexcept { return _Size; }
    constexpr bool empty () const noexcept { return _Size == 0; }

    // element access
    _RWSTD_CONSTEXPR14 reference operator[] (size_type __n)
    { return _Traits::__ref (__elems, __n); }
    constexpr const_reference operator[] (size_type __n) const
    { return _Traits::__ref (__elems, __n); }
    _RWSTD_CONSTEXPR14 reference at (size_type __n)
    {
        if (__n >= _Size)
            __rw_c11::__throw_array_range ();
        return _Traits::__ref (__elems, __n);
    }
    constexpr const_reference at (size_type __n) const
    {
        return __n >= _Size ? (__rw_c11::__throw_array_range (), _Traits::__ref (__elems, 0))
                            : _Traits::__ref (__elems, __n);
    }
    _RWSTD_CONSTEXPR14 reference front () { return _Traits::__ref (__elems, 0); }
    constexpr const_reference front () const { return _Traits::__ref (__elems, 0); }
    _RWSTD_CONSTEXPR14 reference back () { return _Traits::__ref (__elems, _Size - 1); }
    constexpr const_reference back () const { return _Traits::__ref (__elems, _Size - 1); }

    _RWSTD_CONSTEXPR14 value_type* data () noexcept { return _Traits::__ptr (__elems); }
    constexpr const value_type* data () const noexcept { return _Traits::__ptr (__elems); }
};

template <class _Tp, size_t _Size>
inline bool operator== (const array<_Tp, _Size>& __x, const array<_Tp, _Size>& __y)
{ return std::equal (__x.begin (), __x.end (), __y.begin ()); }
template <class _Tp, size_t _Size>
inline bool operator!= (const array<_Tp, _Size>& __x, const array<_Tp, _Size>& __y)
{ return !(__x == __y); }
template <class _Tp, size_t _Size>
inline bool operator< (const array<_Tp, _Size>& __x, const array<_Tp, _Size>& __y)
{
    return std::lexicographical_compare (__x.begin (), __x.end (),
                                         __y.begin (), __y.end ());
}
template <class _Tp, size_t _Size>
inline bool operator> (const array<_Tp, _Size>& __x, const array<_Tp, _Size>& __y)
{ return __y < __x; }
template <class _Tp, size_t _Size>
inline bool operator<= (const array<_Tp, _Size>& __x, const array<_Tp, _Size>& __y)
{ return !(__y < __x); }
template <class _Tp, size_t _Size>
inline bool operator>= (const array<_Tp, _Size>& __x, const array<_Tp, _Size>& __y)
{ return !(__x < __y); }

template <class _Tp, size_t _Size>
inline typename enable_if<_Size == 0 || __rw_c11::__is_swappable<_Tp>::value>::type
swap (array<_Tp, _Size>& __x, array<_Tp, _Size>& __y) noexcept (noexcept (__x.swap (__y)))
{ __x.swap (__y); }

// [array.tuple]
template <class _Tp, size_t _Size>
struct tuple_size<array<_Tp, _Size> > : integral_constant<size_t, _Size> { };

template <size_t _Ip, class _Tp, size_t _Size>
struct tuple_element<_Ip, array<_Tp, _Size> >
{
    static_assert (_Ip < _Size, "index out of bounds in std::tuple_element<> (std::array)");
    typedef _Tp type;
};

template <size_t _Ip, class _Tp, size_t _Size>
inline constexpr _Tp& get (array<_Tp, _Size>& __a) noexcept
{
    static_assert (_Ip < _Size, "index out of bounds in std::get<> (std::array)");
    return array<_Tp, _Size>::_Traits::__ref (__a.__elems, _Ip);
}
template <size_t _Ip, class _Tp, size_t _Size>
inline constexpr const _Tp& get (const array<_Tp, _Size>& __a) noexcept
{
    static_assert (_Ip < _Size, "index out of bounds in std::get<> (const std::array)");
    return array<_Tp, _Size>::_Traits::__ref (__a.__elems, _Ip);
}
template <size_t _Ip, class _Tp, size_t _Size>
inline constexpr _Tp&& get (array<_Tp, _Size>&& __a) noexcept
{
    static_assert (_Ip < _Size, "index out of bounds in std::get<> (std::array &&)");
    return std::move (array<_Tp, _Size>::_Traits::__ref (__a.__elems, _Ip));
}

}   // namespace std

#endif   // _RWSTD_RW_C11_ARRAY_H_INCLUDED
