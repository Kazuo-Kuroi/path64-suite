// -*- C++ -*-
/***************************************************************************
 *
 * _c11_uses_alloc.h - C++11 allocator_arg_t, uses_allocator
 *                     [allocator.tag], [allocator.uses]
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

#ifndef _RWSTD_RW_C11_USES_ALLOC_H_INCLUDED
#define _RWSTD_RW_C11_USES_ALLOC_H_INCLUDED

#include <rw/_c11_utility.h>

namespace std {

// [allocator.tag]
struct allocator_arg_t { explicit allocator_arg_t () = default; };
constexpr allocator_arg_t allocator_arg = allocator_arg_t ();

// [allocator.uses.trait]
namespace __rw_c11 {
template <class _Tp, class = void> struct __has_allocator_type : false_type { };
template <class _Tp>
struct __has_allocator_type<_Tp, typename __rw_tt::__void_t<
    typename _Tp::allocator_type>::type> : true_type { };

template <class _Tp, class _Alloc, bool = __has_allocator_type<_Tp>::value>
struct __uses_allocator
    : integral_constant<bool, is_convertible<_Alloc,
                                             typename _Tp::allocator_type>::value> { };
template <class _Tp, class _Alloc>
struct __uses_allocator<_Tp, _Alloc, false> : false_type { };
}   // namespace __rw_c11

template <class _Tp, class _Alloc>
struct uses_allocator : __rw_c11::__uses_allocator<_Tp, _Alloc> { };

namespace __rw_c11 {
// [allocator.uses.construction]: 0, no allocator; 1, (allocator_arg_t,
// alloc, args...); 2, (args..., alloc)
template <class _Tp, class _Alloc, class... _Args>
struct __uses_alloc_ctor_imp
{
    typedef typename remove_cv<typename remove_reference<_Alloc>::type>::type _RawAlloc;
    static const bool __ua = uses_allocator<_Tp, _RawAlloc>::value;
    static const bool __ic =
        is_constructible<_Tp, allocator_arg_t, _Alloc, _Args...>::value;
    static const int value = __ua ? 2 - __ic : 0;
};

template <class _Tp, class _Alloc, class... _Args>
struct __uses_alloc_ctor
    : integral_constant<int, __uses_alloc_ctor_imp<_Tp, _Alloc, _Args...>::value> { };
}   // namespace __rw_c11

}   // namespace std

#endif   // _RWSTD_RW_C11_USES_ALLOC_H_INCLUDED
