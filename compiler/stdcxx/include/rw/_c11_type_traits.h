// -*- C++ -*-
/***************************************************************************
 *
 * _c11_type_traits.h - C++11 and C++14 <type_traits> [meta]
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

#ifndef _RWSTD_RW_C11_TYPE_TRAITS_H_INCLUDED
#define _RWSTD_RW_C11_TYPE_TRAITS_H_INCLUDED

#include <rw/_defs.h>
#include <cstddef>

namespace std {

// [meta.help]
template <class _Tp, _Tp __v>
struct integral_constant
{
    static constexpr const _Tp value = __v;
    typedef _Tp value_type;
    typedef integral_constant type;
    constexpr operator value_type () const noexcept { return value; }
#if __cplusplus >= 201402L
    constexpr value_type operator() () const noexcept { return value; }
#endif
};

template <class _Tp, _Tp __v>
constexpr const _Tp integral_constant<_Tp, __v>::value;

typedef integral_constant<bool, true>  true_type;
typedef integral_constant<bool, false> false_type;

// [meta.trans.other] (the basics others use)
template <bool, class _Tp = void> struct enable_if { };
template <class _Tp> struct enable_if<true, _Tp> { typedef _Tp type; };

template <bool _Bp, class _If, class _Then>
struct conditional { typedef _If type; };
template <class _If, class _Then>
struct conditional<false, _If, _Then> { typedef _Then type; };

// [meta.rel]
template <class _Tp, class _Up> struct is_same : false_type { };
template <class _Tp> struct is_same<_Tp, _Tp> : true_type { };

// [meta.trans.cv]
template <class _Tp> struct remove_const { typedef _Tp type; };
template <class _Tp> struct remove_const<const _Tp> { typedef _Tp type; };
template <class _Tp> struct remove_volatile { typedef _Tp type; };
template <class _Tp> struct remove_volatile<volatile _Tp> { typedef _Tp type; };
template <class _Tp> struct remove_cv
{ typedef typename remove_volatile<typename remove_const<_Tp>::type>::type type; };
template <class _Tp> struct add_const { typedef const _Tp type; };
template <class _Tp> struct add_volatile { typedef volatile _Tp type; };
template <class _Tp> struct add_cv { typedef const volatile _Tp type; };

// [meta.trans.ref]
template <class _Tp> struct remove_reference { typedef _Tp type; };
template <class _Tp> struct remove_reference<_Tp&> { typedef _Tp type; };
template <class _Tp> struct remove_reference<_Tp&&> { typedef _Tp type; };

namespace __rw_tt {

// referenceable types (not void or function types with cv or ref
// qualifiers): the add_* traits leave the others unchanged
struct __two { char __c[2]; };
template <class _Tp> _Tp& __ref_test (int);
template <class _Tp> __two __ref_test (...);
template <class _Tp>
struct __is_referenceable
    : integral_constant<bool, !is_same<decltype (__ref_test<_Tp>(0)), __two>::value> { };

template <class _Tp, bool = __is_referenceable<_Tp>::value>
struct __add_lref { typedef _Tp type; };
template <class _Tp> struct __add_lref<_Tp, true> { typedef _Tp& type; };
template <class _Tp, bool = __is_referenceable<_Tp>::value>
struct __add_rref { typedef _Tp type; };
template <class _Tp> struct __add_rref<_Tp, true> { typedef _Tp&& type; };

}   // namespace __rw_tt

template <class _Tp> struct add_lvalue_reference
{ typedef typename __rw_tt::__add_lref<_Tp>::type type; };
template <class _Tp> struct add_rvalue_reference
{ typedef typename __rw_tt::__add_rref<_Tp>::type type; };

// [declval]
template <class _Tp>
typename add_rvalue_reference<_Tp>::type declval () noexcept;

// [meta.unary.cat]
namespace __rw_tt {
template <class _Tp> struct __is_void : false_type { };
template <> struct __is_void<void> : true_type { };

template <class _Tp> struct __is_nullptr : false_type { };
template <> struct __is_nullptr<decltype (nullptr)> : true_type { };

template <class _Tp> struct __is_integral : false_type { };
template <> struct __is_integral<bool> : true_type { };
template <> struct __is_integral<char> : true_type { };
template <> struct __is_integral<signed char> : true_type { };
template <> struct __is_integral<unsigned char> : true_type { };
template <> struct __is_integral<wchar_t> : true_type { };
template <> struct __is_integral<char16_t> : true_type { };
template <> struct __is_integral<char32_t> : true_type { };
template <> struct __is_integral<short> : true_type { };
template <> struct __is_integral<unsigned short> : true_type { };
template <> struct __is_integral<int> : true_type { };
template <> struct __is_integral<unsigned int> : true_type { };
template <> struct __is_integral<long> : true_type { };
template <> struct __is_integral<unsigned long> : true_type { };
template <> struct __is_integral<long long> : true_type { };
template <> struct __is_integral<unsigned long long> : true_type { };
#ifdef __SIZEOF_INT128__
template <> struct __is_integral<__int128> : true_type { };
template <> struct __is_integral<unsigned __int128> : true_type { };
#endif

template <class _Tp> struct __is_floating_point : false_type { };
template <> struct __is_floating_point<float> : true_type { };
template <> struct __is_floating_point<double> : true_type { };
template <> struct __is_floating_point<long double> : true_type { };

template <class _Tp> struct __is_pointer : false_type { };
template <class _Tp> struct __is_pointer<_Tp*> : true_type { };

template <class _Tp> struct __is_member_pointer : false_type { };
template <class _Tp, class _Cp> struct __is_member_pointer<_Tp _Cp::*> : true_type { };
}   // namespace __rw_tt

template <class _Tp> struct is_void
    : __rw_tt::__is_void<typename remove_cv<_Tp>::type> { };
template <class _Tp> struct is_integral
    : __rw_tt::__is_integral<typename remove_cv<_Tp>::type> { };
template <class _Tp> struct is_floating_point
    : __rw_tt::__is_floating_point<typename remove_cv<_Tp>::type> { };

template <class _Tp> struct is_array : false_type { };
template <class _Tp> struct is_array<_Tp[]> : true_type { };
template <class _Tp, size_t _Np> struct is_array<_Tp[_Np]> : true_type { };

template <class _Tp> struct is_pointer
    : __rw_tt::__is_pointer<typename remove_cv<_Tp>::type> { };

#if __cplusplus >= 201402L
template <class _Tp> struct is_null_pointer
    : is_same<typename remove_cv<_Tp>::type, decltype (nullptr)> { };
#endif

template <class _Tp> struct is_lvalue_reference : false_type { };
template <class _Tp> struct is_lvalue_reference<_Tp&> : true_type { };
template <class _Tp> struct is_rvalue_reference : false_type { };
template <class _Tp> struct is_rvalue_reference<_Tp&&> : true_type { };
template <class _Tp> struct is_reference : false_type { };
template <class _Tp> struct is_reference<_Tp&> : true_type { };
template <class _Tp> struct is_reference<_Tp&&> : true_type { };

template <class _Tp> struct is_union : integral_constant<bool, __is_union (_Tp)> { };
template <class _Tp> struct is_class : integral_constant<bool, __is_class (_Tp)> { };
template <class _Tp> struct is_enum : integral_constant<bool, __is_enum (_Tp)> { };

template <class _Tp> struct is_const : false_type { };
template <class _Tp> struct is_const<const _Tp> : true_type { };
template <class _Tp> struct is_volatile : false_type { };
template <class _Tp> struct is_volatile<volatile _Tp> : true_type { };

// functions: not objects, references, void, or anything else; the only
// types that do not change when made const (and are not references)
template <class _Tp> struct is_function
    : integral_constant<bool, !(is_reference<_Tp>::value
                                || is_const<const _Tp>::value)> { };

namespace __rw_tt {
template <class _Tp> struct __is_member_function_pointer : false_type { };
template <class _Tp, class _Cp>
struct __is_member_function_pointer<_Tp _Cp::*> : is_function<_Tp> { };
}

template <class _Tp> struct is_member_function_pointer
    : __rw_tt::__is_member_function_pointer<typename remove_cv<_Tp>::type> { };
template <class _Tp> struct is_member_pointer
    : __rw_tt::__is_member_pointer<typename remove_cv<_Tp>::type> { };
template <class _Tp> struct is_member_object_pointer
    : integral_constant<bool, is_member_pointer<_Tp>::value
                              && !is_member_function_pointer<_Tp>::value> { };

template <class _Tp> struct is_arithmetic
    : integral_constant<bool, is_integral<_Tp>::value
                              || is_floating_point<_Tp>::value> { };
template <class _Tp> struct is_fundamental
    : integral_constant<bool, is_void<_Tp>::value
                              || __rw_tt::__is_nullptr<typename remove_cv<_Tp>::type>::value
                              || is_arithmetic<_Tp>::value> { };
template <class _Tp> struct is_scalar
    : integral_constant<bool, is_arithmetic<_Tp>::value
                              || is_member_pointer<_Tp>::value
                              || is_pointer<_Tp>::value
                              || __rw_tt::__is_nullptr<typename remove_cv<_Tp>::type>::value
                              || is_enum<_Tp>::value> { };
template <class _Tp> struct is_object
    : integral_constant<bool, is_scalar<_Tp>::value || is_array<_Tp>::value
                              || is_union<_Tp>::value || is_class<_Tp>::value> { };
template <class _Tp> struct is_compound
    : integral_constant<bool, !is_fundamental<_Tp>::value> { };

// [meta.trans.ptr]
template <class _Tp> struct remove_pointer { typedef _Tp type; };
template <class _Tp> struct remove_pointer<_Tp*> { typedef _Tp type; };
template <class _Tp> struct remove_pointer<_Tp* const> { typedef _Tp type; };
template <class _Tp> struct remove_pointer<_Tp* volatile> { typedef _Tp type; };
template <class _Tp> struct remove_pointer<_Tp* const volatile> { typedef _Tp type; };

namespace __rw_tt {
template <class _Tp, bool = __is_referenceable<_Tp>::value || is_void<_Tp>::value>
struct __add_pointer { typedef typename remove_reference<_Tp>::type* type; };
template <class _Tp> struct __add_pointer<_Tp, false> { typedef _Tp type; };
}
template <class _Tp> struct add_pointer
{ typedef typename __rw_tt::__add_pointer<_Tp>::type type; };

// [meta.trans.arr]
template <class _Tp> struct remove_extent { typedef _Tp type; };
template <class _Tp> struct remove_extent<_Tp[]> { typedef _Tp type; };
template <class _Tp, size_t _Np> struct remove_extent<_Tp[_Np]> { typedef _Tp type; };
template <class _Tp> struct remove_all_extents { typedef _Tp type; };
template <class _Tp> struct remove_all_extents<_Tp[]>
{ typedef typename remove_all_extents<_Tp>::type type; };
template <class _Tp, size_t _Np> struct remove_all_extents<_Tp[_Np]>
{ typedef typename remove_all_extents<_Tp>::type type; };

// [meta.unary.prop.query]
template <class _Tp> struct alignment_of
    : integral_constant<size_t, alignof (_Tp)> { };
template <class _Tp> struct rank : integral_constant<size_t, 0> { };
template <class _Tp> struct rank<_Tp[]>
    : integral_constant<size_t, rank<_Tp>::value + 1> { };
template <class _Tp, size_t _Np> struct rank<_Tp[_Np]>
    : integral_constant<size_t, rank<_Tp>::value + 1> { };
template <class _Tp, unsigned _Ip = 0> struct extent : integral_constant<size_t, 0> { };
template <class _Tp> struct extent<_Tp[], 0> : integral_constant<size_t, 0> { };
template <class _Tp, unsigned _Ip> struct extent<_Tp[], _Ip>
    : integral_constant<size_t, extent<_Tp, _Ip - 1>::value> { };
template <class _Tp, size_t _Np> struct extent<_Tp[_Np], 0>
    : integral_constant<size_t, _Np> { };
template <class _Tp, size_t _Np, unsigned _Ip> struct extent<_Tp[_Np], _Ip>
    : integral_constant<size_t, extent<_Tp, _Ip - 1>::value> { };

// [meta.unary.prop]
template <class _Tp> struct is_trivial : integral_constant<bool, __is_trivial (_Tp)> { };
template <class _Tp> struct is_trivially_copyable
    : integral_constant<bool, __is_trivially_copyable (_Tp)> { };
template <class _Tp> struct is_standard_layout
    : integral_constant<bool, __is_standard_layout (_Tp)> { };
template <class _Tp> struct is_pod : integral_constant<bool, __is_pod (_Tp)> { };
template <class _Tp> struct is_literal_type
    : integral_constant<bool, __is_literal_type (_Tp)> { };
template <class _Tp> struct is_empty : integral_constant<bool, __is_empty (_Tp)> { };
template <class _Tp> struct is_polymorphic
    : integral_constant<bool, __is_polymorphic (_Tp)> { };
template <class _Tp> struct is_abstract : integral_constant<bool, __is_abstract (_Tp)> { };
#if __cplusplus >= 201402L
template <class _Tp> struct is_final : integral_constant<bool, __is_final (_Tp)> { };
#endif

namespace __rw_tt {
template <class _Tp, bool = is_arithmetic<_Tp>::value>
struct __is_signed : integral_constant<bool, _Tp (-1) < _Tp (0)> { };
template <class _Tp> struct __is_signed<_Tp, false> : false_type { };
template <class _Tp, bool = is_arithmetic<_Tp>::value>
struct __is_unsigned : integral_constant<bool, _Tp (0) < _Tp (-1)> { };
template <class _Tp> struct __is_unsigned<_Tp, false> : false_type { };
}
template <class _Tp> struct is_signed : __rw_tt::__is_signed<_Tp> { };
template <class _Tp> struct is_unsigned : __rw_tt::__is_unsigned<_Tp> { };

template <class _Tp, class... _Args> struct is_constructible
    : integral_constant<bool, __is_constructible (_Tp, _Args...)> { };
template <class _Tp> struct is_default_constructible
    : is_constructible<_Tp> { };
template <class _Tp> struct is_copy_constructible
    : is_constructible<_Tp, typename add_lvalue_reference<
                                typename add_const<_Tp>::type>::type> { };
template <class _Tp> struct is_move_constructible
    : is_constructible<_Tp, typename add_rvalue_reference<_Tp>::type> { };

template <class _Tp, class _Up> struct is_assignable
    : integral_constant<bool, __is_assignable (_Tp, _Up)> { };
template <class _Tp> struct is_copy_assignable
    : is_assignable<typename add_lvalue_reference<_Tp>::type,
                    typename add_lvalue_reference<
                        typename add_const<_Tp>::type>::type> { };
template <class _Tp> struct is_move_assignable
    : is_assignable<typename add_lvalue_reference<_Tp>::type,
                    typename add_rvalue_reference<_Tp>::type> { };

// is_destructible: references; or complete object types (arrays of known
// bound by their elements) whose destructor can be called
namespace __rw_tt {
template <class _Tp, class = decltype (declval<_Tp&>().~_Tp ())>
true_type __dtor_test (int);
template <class _Tp> false_type __dtor_test (...);

template <class _Tp, bool = is_void<_Tp>::value || is_function<_Tp>::value
                            || (is_array<_Tp>::value && extent<_Tp>::value == 0),
          bool = is_reference<_Tp>::value || is_scalar<_Tp>::value>
struct __is_destructible
    : decltype (__dtor_test<typename remove_all_extents<_Tp>::type>(0)) { };
template <class _Tp, bool _Bp> struct __is_destructible<_Tp, true, _Bp> : false_type { };
template <class _Tp> struct __is_destructible<_Tp, false, true> : true_type { };
}
template <class _Tp> struct is_destructible : __rw_tt::__is_destructible<_Tp> { };

template <class _Tp, class... _Args> struct is_trivially_constructible
    : integral_constant<bool, __is_trivially_constructible (_Tp, _Args...)> { };
template <class _Tp> struct is_trivially_default_constructible
    : is_trivially_constructible<_Tp> { };
template <class _Tp> struct is_trivially_copy_constructible
    : is_trivially_constructible<_Tp, typename add_lvalue_reference<
                                         const _Tp>::type> { };
template <class _Tp> struct is_trivially_move_constructible
    : is_trivially_constructible<_Tp, typename add_rvalue_reference<_Tp>::type> { };
template <class _Tp, class _Up> struct is_trivially_assignable
    : integral_constant<bool, __is_trivially_assignable (_Tp, _Up)> { };
template <class _Tp> struct is_trivially_copy_assignable
    : is_trivially_assignable<typename add_lvalue_reference<_Tp>::type,
                              typename add_lvalue_reference<const _Tp>::type> { };
template <class _Tp> struct is_trivially_move_assignable
    : is_trivially_assignable<typename add_lvalue_reference<_Tp>::type,
                              typename add_rvalue_reference<_Tp>::type> { };
template <class _Tp> struct is_trivially_destructible
    : integral_constant<bool, is_destructible<_Tp>::value
                              && __has_trivial_destructor (_Tp)> { };

template <class _Tp, class... _Args> struct is_nothrow_constructible
    : integral_constant<bool, __is_nothrow_constructible (_Tp, _Args...)> { };
template <class _Tp> struct is_nothrow_default_constructible
    : is_nothrow_constructible<_Tp> { };
template <class _Tp> struct is_nothrow_copy_constructible
    : is_nothrow_constructible<_Tp, typename add_lvalue_reference<
                                        const _Tp>::type> { };
template <class _Tp> struct is_nothrow_move_constructible
    : is_nothrow_constructible<_Tp, typename add_rvalue_reference<_Tp>::type> { };
template <class _Tp, class _Up> struct is_nothrow_assignable
    : integral_constant<bool, __is_nothrow_assignable (_Tp, _Up)> { };
template <class _Tp> struct is_nothrow_copy_assignable
    : is_nothrow_assignable<typename add_lvalue_reference<_Tp>::type,
                            typename add_lvalue_reference<const _Tp>::type> { };
template <class _Tp> struct is_nothrow_move_assignable
    : is_nothrow_assignable<typename add_lvalue_reference<_Tp>::type,
                            typename add_rvalue_reference<_Tp>::type> { };

namespace __rw_tt {
template <class _Tp, bool = is_destructible<_Tp>::value>
struct __nothrow_dtor : false_type { };
template <class _Tp>
struct __nothrow_dtor<_Tp, true>
    : integral_constant<bool, noexcept (declval<_Tp&>().~_Tp ())> { };
template <class _Tp> struct __nothrow_dtor<_Tp&, true> : true_type { };
template <class _Tp> struct __nothrow_dtor<_Tp&&, true> : true_type { };
}
template <class _Tp> struct is_nothrow_destructible
    : integral_constant<bool, is_destructible<_Tp>::value
                              && __rw_tt::__nothrow_dtor<
                                     typename remove_all_extents<_Tp>::type>::value> { };

template <class _Tp> struct has_virtual_destructor
    : integral_constant<bool, __has_virtual_destructor (_Tp)> { };

// [meta.rel]
template <class _Bp, class _Dp> struct is_base_of
    : integral_constant<bool, __is_base_of (_Bp, _Dp)> { };
template <class _From, class _To> struct is_convertible
    : integral_constant<bool, __is_convertible_to (_From, _To)
                              && !is_abstract<_To>::value> { };

// [meta.trans.sign]
namespace __rw_tt {
template <class _Tp, class _Up> struct __copy_cv { typedef _Up type; };
template <class _Tp, class _Up> struct __copy_cv<const _Tp, _Up> { typedef const _Up type; };
template <class _Tp, class _Up> struct __copy_cv<volatile _Tp, _Up> { typedef volatile _Up type; };
template <class _Tp, class _Up> struct __copy_cv<const volatile _Tp, _Up>
{ typedef const volatile _Up type; };

// the integer type of the same size, for enums and character types
template <size_t _Sz, bool _Signed> struct __int_of_size;
template <> struct __int_of_size<1, true> { typedef signed char type; };
template <> struct __int_of_size<1, false> { typedef unsigned char type; };
template <> struct __int_of_size<2, true> { typedef short type; };
template <> struct __int_of_size<2, false> { typedef unsigned short type; };
template <> struct __int_of_size<4, true> { typedef int type; };
template <> struct __int_of_size<4, false> { typedef unsigned int type; };
template <> struct __int_of_size<8, true> { typedef long type; };
template <> struct __int_of_size<8, false> { typedef unsigned long type; };
#ifdef __SIZEOF_INT128__
template <> struct __int_of_size<16, true> { typedef __int128 type; };
template <> struct __int_of_size<16, false> { typedef unsigned __int128 type; };
#endif

template <class _Tp> struct __make_signed
{ typedef typename __int_of_size<sizeof (_Tp), true>::type type; };
template <> struct __make_signed<signed char> { typedef signed char type; };
template <> struct __make_signed<unsigned char> { typedef signed char type; };
template <> struct __make_signed<short> { typedef short type; };
template <> struct __make_signed<unsigned short> { typedef short type; };
template <> struct __make_signed<int> { typedef int type; };
template <> struct __make_signed<unsigned int> { typedef int type; };
template <> struct __make_signed<long> { typedef long type; };
template <> struct __make_signed<unsigned long> { typedef long type; };
template <> struct __make_signed<long long> { typedef long long type; };
template <> struct __make_signed<unsigned long long> { typedef long long type; };

template <class _Tp> struct __make_unsigned
{ typedef typename __int_of_size<sizeof (_Tp), false>::type type; };
template <> struct __make_unsigned<signed char> { typedef unsigned char type; };
template <> struct __make_unsigned<unsigned char> { typedef unsigned char type; };
template <> struct __make_unsigned<short> { typedef unsigned short type; };
template <> struct __make_unsigned<unsigned short> { typedef unsigned short type; };
template <> struct __make_unsigned<int> { typedef unsigned int type; };
template <> struct __make_unsigned<unsigned int> { typedef unsigned int type; };
template <> struct __make_unsigned<long> { typedef unsigned long type; };
template <> struct __make_unsigned<unsigned long> { typedef unsigned long type; };
template <> struct __make_unsigned<long long> { typedef unsigned long long type; };
template <> struct __make_unsigned<unsigned long long> { typedef unsigned long long type; };
}   // namespace __rw_tt

template <class _Tp> struct make_signed
{
    typedef typename __rw_tt::__copy_cv<_Tp,
        typename __rw_tt::__make_signed<typename remove_cv<_Tp>::type>::type>::type type;
};
template <class _Tp> struct make_unsigned
{
    typedef typename __rw_tt::__copy_cv<_Tp,
        typename __rw_tt::__make_unsigned<typename remove_cv<_Tp>::type>::type>::type type;
};

// [meta.trans.other]
namespace __rw_tt {
// the default alignment of aligned_storage<Len>: the strictest one of an
// object no bigger than Len, the largest power of two not over Len
template <size_t _Len, size_t _Al = alignof (max_align_t)>
struct __default_align
    : integral_constant<size_t, (_Al <= _Len || _Al == 1)
                                ? _Al : __default_align<_Len, _Al / 2>::value> { };
template <size_t _Len> struct __default_align<_Len, 1> : integral_constant<size_t, 1> { };
}

template <size_t _Len, size_t _Align = __rw_tt::__default_align<_Len>::value>
struct aligned_storage
{
    struct type { alignas (_Align) unsigned char __data[_Len]; };
};

namespace __rw_tt {
template <size_t... _Ns> struct __static_max;
template <size_t _N0> struct __static_max<_N0>
{ static constexpr const size_t value = _N0; };
template <size_t _N0, size_t _N1, size_t... _Ns> struct __static_max<_N0, _N1, _Ns...>
{
    static constexpr const size_t value =
        _N0 >= _N1 ? __static_max<_N0, _Ns...>::value : __static_max<_N1, _Ns...>::value;
};
}

template <size_t _Len, class _Type0, class... _Types>
struct aligned_union
{
    static constexpr const size_t alignment_value =
        __rw_tt::__static_max<alignof (_Type0), alignof (_Types)...>::value;
    static constexpr const size_t __len =
        __rw_tt::__static_max<_Len, sizeof (_Type0), sizeof (_Types)...>::value;
    typedef typename aligned_storage<__len, alignment_value>::type type;
};
template <size_t _Len, class _Type0, class... _Types>
constexpr const size_t aligned_union<_Len, _Type0, _Types...>::alignment_value;

template <class _Tp> struct decay
{
private:
    typedef typename remove_reference<_Tp>::type _Up;
public:
    typedef typename conditional<
        is_array<_Up>::value,
        typename add_pointer<typename remove_extent<_Up>::type>::type,
        typename conditional<is_function<_Up>::value,
                             typename add_pointer<_Up>::type,
                             typename remove_cv<_Up>::type>::type>::type type;
};

template <class... _Tp> struct common_type;
// common_type<T> is common_type<T, T> (LWG 2141, 2408), so that
// specializations for <T, T> apply
template <> struct common_type<> { };
template <class _Tp> struct common_type<_Tp> : common_type<_Tp, _Tp> { };

namespace __rw_tt {
template <class _Tp> struct __void_t { typedef void type; };

template <class _Tp, class _Up, class = void> struct __common_type2 { };
template <class _Tp, class _Up>
struct __common_type2<_Tp, _Up, typename __void_t<
    decltype (true ? declval<_Tp>() : declval<_Up>())>::type>
{ typedef typename decay<decltype (true ? declval<_Tp>() : declval<_Up>())>::type type; };

// common_type<T, U, V...>: common_type<common_type<T, U>::type, V...>,
// and no type if one of the steps has none
template <class _Void, class _Ct, class... _Rest> struct __common_type_n { };
template <class _Ct, class... _Rest>
struct __common_type_n<typename __void_t<typename _Ct::type>::type, _Ct, _Rest...>
    : common_type<typename _Ct::type, _Rest...> { };
}

// with types that decay to others, common_type of those (where user
// specializations apply); else the type of the conditional expression
template <class _Tp, class _Up> struct common_type<_Tp, _Up>
    : conditional<is_same<_Tp, typename decay<_Tp>::type>::value
                  && is_same<_Up, typename decay<_Up>::type>::value,
                  __rw_tt::__common_type2<_Tp, _Up>,
                  common_type<typename decay<_Tp>::type,
                              typename decay<_Up>::type> >::type { };
template <class _Tp, class _Up, class _Vp, class... _Rest>
struct common_type<_Tp, _Up, _Vp, _Rest...>
    : __rw_tt::__common_type_n<void, common_type<_Tp, _Up>, _Vp, _Rest...> { };

template <class _Tp> struct underlying_type
{ typedef __underlying_type (_Tp) type; };

// INVOKE [func.require], shared by result_of and <functional>.
template <class _Tp> class reference_wrapper;

namespace __rw_tt {

template <class _Tp> struct __member_class;
template <class _Rp, class _Cp> struct __member_class<_Rp _Cp::*> { typedef _Cp type; };

template <class _Tp> struct __is_refwrap : false_type { };
template <class _Tp> struct __is_refwrap<reference_wrapper<_Tp> > : true_type { };

template <class _Cp, class _A0>
struct __invoke_dereference
    : integral_constant<bool,
        !is_base_of<_Cp, typename remove_reference<_A0>::type>::value
        && !__is_refwrap<typename decay<_A0>::type>::value> { };

template <class _Fp, class... _Args>
inline auto __invoke (_Fp&& __f, _Args&&... __args)
    -> decltype (static_cast<_Fp&&>(__f)(static_cast<_Args&&>(__args)...))
{
    return static_cast<_Fp&&>(__f)(static_cast<_Args&&>(__args)...);
}

template <class _Mp, class _A0, class... _Args,
          class _Cp = typename __member_class<typename decay<_Mp>::type>::type,
          class = typename enable_if<is_member_function_pointer<typename decay<_Mp>::type>::value
                                     && is_base_of<_Cp, typename remove_reference<_A0>::type>::value>::type>
inline auto __invoke (_Mp&& __f, _A0&& __a0, _Args&&... __args)
    -> decltype ((static_cast<_A0&&>(__a0).*__f)(static_cast<_Args&&>(__args)...))
{
    return (static_cast<_A0&&>(__a0).*__f)(static_cast<_Args&&>(__args)...);
}

template <class _Mp, class _A0, class... _Args,
          class = typename enable_if<is_member_function_pointer<typename decay<_Mp>::type>::value
                                     && __is_refwrap<typename decay<_A0>::type>::value>::type>
inline auto __invoke (_Mp&& __f, _A0&& __a0, _Args&&... __args)
    -> decltype ((static_cast<_A0&&>(__a0).get().*__f)(static_cast<_Args&&>(__args)...))
{
    return (static_cast<_A0&&>(__a0).get().*__f)(static_cast<_Args&&>(__args)...);
}

template <class _Mp, class _A0, class... _Args,
          class _Cp = typename __member_class<typename decay<_Mp>::type>::type,
          class = typename enable_if<is_member_function_pointer<typename decay<_Mp>::type>::value
                                     && __invoke_dereference<_Cp, _A0>::value>::type>
inline auto __invoke (_Mp&& __f, _A0&& __a0, _Args&&... __args)
    -> decltype (((*static_cast<_A0&&>(__a0)).*__f)(static_cast<_Args&&>(__args)...))
{
    return ((*static_cast<_A0&&>(__a0)).*__f)(static_cast<_Args&&>(__args)...);
}

template <class _Mp, class _A0,
          class _Cp = typename __member_class<typename decay<_Mp>::type>::type,
          class = typename enable_if<is_member_object_pointer<typename decay<_Mp>::type>::value
                                     && is_base_of<_Cp, typename remove_reference<_A0>::type>::value>::type>
inline auto __invoke (_Mp&& __f, _A0&& __a0)
    -> decltype (static_cast<_A0&&>(__a0).*__f)
{
    return static_cast<_A0&&>(__a0).*__f;
}

template <class _Mp, class _A0,
          class = typename enable_if<is_member_object_pointer<typename decay<_Mp>::type>::value
                                     && __is_refwrap<typename decay<_A0>::type>::value>::type>
inline auto __invoke (_Mp&& __f, _A0&& __a0)
    -> decltype (static_cast<_A0&&>(__a0).get().*__f)
{
    return static_cast<_A0&&>(__a0).get().*__f;
}

template <class _Mp, class _A0,
          class _Cp = typename __member_class<typename decay<_Mp>::type>::type,
          class = typename enable_if<is_member_object_pointer<typename decay<_Mp>::type>::value
                                     && __invoke_dereference<_Cp, _A0>::value>::type>
inline auto __invoke (_Mp&& __f, _A0&& __a0)
    -> decltype ((*static_cast<_A0&&>(__a0)).*__f)
{
    return (*static_cast<_A0&&>(__a0)).*__f;
}

template <class _Void, class _Fp, class... _Args> struct __invoke_result { };
template <class _Fp, class... _Args>
struct __invoke_result<decltype ((void)__invoke (declval<_Fp>(), declval<_Args>()...)),
                       _Fp, _Args...>
{ typedef decltype (__invoke (declval<_Fp>(), declval<_Args>()...)) type; };
}   // namespace __rw_tt

template <class _Fn> struct result_of;
template <class _Fp, class... _Args>
struct result_of<_Fp (_Args...)> : __rw_tt::__invoke_result<void, _Fp, _Args...> { };

#if __cplusplus >= 201402L
// [meta.type.synop] aliases (C++14)
template <class _Tp> using remove_const_t = typename remove_const<_Tp>::type;
template <class _Tp> using remove_volatile_t = typename remove_volatile<_Tp>::type;
template <class _Tp> using remove_cv_t = typename remove_cv<_Tp>::type;
template <class _Tp> using add_const_t = typename add_const<_Tp>::type;
template <class _Tp> using add_volatile_t = typename add_volatile<_Tp>::type;
template <class _Tp> using add_cv_t = typename add_cv<_Tp>::type;
template <class _Tp> using remove_reference_t = typename remove_reference<_Tp>::type;
template <class _Tp> using add_lvalue_reference_t = typename add_lvalue_reference<_Tp>::type;
template <class _Tp> using add_rvalue_reference_t = typename add_rvalue_reference<_Tp>::type;
template <class _Tp> using make_signed_t = typename make_signed<_Tp>::type;
template <class _Tp> using make_unsigned_t = typename make_unsigned<_Tp>::type;
template <class _Tp> using remove_extent_t = typename remove_extent<_Tp>::type;
template <class _Tp> using remove_all_extents_t = typename remove_all_extents<_Tp>::type;
template <class _Tp> using remove_pointer_t = typename remove_pointer<_Tp>::type;
template <class _Tp> using add_pointer_t = typename add_pointer<_Tp>::type;
template <size_t _Len, size_t _Align = __rw_tt::__default_align<_Len>::value>
using aligned_storage_t = typename aligned_storage<_Len, _Align>::type;
template <size_t _Len, class... _Types>
using aligned_union_t = typename aligned_union<_Len, _Types...>::type;
template <class _Tp> using decay_t = typename decay<_Tp>::type;
template <bool _Bp, class _Tp = void> using enable_if_t = typename enable_if<_Bp, _Tp>::type;
template <bool _Bp, class _If, class _Then>
using conditional_t = typename conditional<_Bp, _If, _Then>::type;
template <class... _Tp> using common_type_t = typename common_type<_Tp...>::type;
template <class _Tp> using underlying_type_t = typename underlying_type<_Tp>::type;
template <class _Tp> using result_of_t = typename result_of<_Tp>::type;
#endif   // C++14

}   // namespace std

#endif   // _RWSTD_RW_C11_TYPE_TRAITS_H_INCLUDED
