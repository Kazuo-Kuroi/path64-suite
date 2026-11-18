/***************************************************************************
 *
 * _iterator.h - Iterator declarations for the Standard Library
 *
 * This is an internal header file used to implement the C++ Standard
 * Library. It should never be #included directly by a program.
 *
 * $Id$
 *
 ***************************************************************************
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
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
 * Copyright 1994-2006 Rogue Wave Software.
 * 
 **************************************************************************/

#ifndef _RWSTD_RW_ITERATOR_H_INCLUDED
#define _RWSTD_RW_ITERATOR_H_INCLUDED

#ifndef _RWSTD_RW_ITERBASE_H_INCLUDED
#  include <rw/_iterbase.h>
#endif   // _RWSTD_RW_ITERBASE_H_INCLUDED


#ifdef _RWSTD_CXX11
#  include <rw/_c11_utility.h>
#endif

#if defined (_RWSTD_CXX11) && __cplusplus >= 201402L
#  include <initializer_list>
#endif


_RWSTD_NAMESPACE (std) { 


// Reverse iterator.     

//
//  Macros for reverse iterators to accomodate non-standard compilers
//
#ifndef _RWSTD_NO_CLASS_PARTIAL_SPEC 
#  define _RWSTD_ITER_TEMPLATE template <class _Iterator>
#  define _RWSTD_ITER_ID(i)    i <_Iterator>
#  define _RWSTD_ITER_DIFF_TYPE(i, ignore)                             \
          typename iterator_traits<_Iterator>::difference_type
#else
#  define _RWSTD_ITER_TEMPLATE                                         \
          template <class _Iterator, class _Category, class _TypeT,    \
                    class _Reference, class _Pointer, class _Distance>
#  define _RWSTD_ITER_ID(i)                                            \
          i <_Iterator, _Category, _TypeT, _Reference, _Pointer, _Distance>
#  define _RWSTD_ITER_DIFF_TYPE(ignore, distance) distance          
#endif   // _RWSTD_NO_CLASS_PARTIAL_SPEC 


#ifndef _RWSTD_NO_CLASS_PARTIAL_SPEC 

template <class _Iterator>
class reverse_iterator
    : public iterator<typename iterator_traits<_Iterator>::iterator_category, 
                      typename iterator_traits<_Iterator>::value_type, 
                      typename iterator_traits<_Iterator>::difference_type,
                      typename iterator_traits<_Iterator>::pointer, 
                      typename iterator_traits<_Iterator>::reference>
{
    typedef iterator_traits<_Iterator>             traits_type;
public:
    typedef typename traits_type::difference_type difference_type;
    typedef typename traits_type::value_type      value_type;
    typedef typename traits_type::pointer         pointer;
    typedef typename traits_type::reference       reference;

#else   // if defined (_RWSTD_NO_CLASS_PARTIAL_SPEC)

template <class _Iterator, class _Category, class _TypeT,
          class _Reference = _TypeT&, 
          class _Pointer = _TypeT*,
          class _Distance = _RWSTD_PTRDIFF_T>
class reverse_iterator
    : public iterator<_Category, _TypeT, _Distance, _Pointer, _Reference>

{
public:
    typedef _Distance  difference_type;
    typedef _TypeT     value_type;
    typedef _Reference reference;
    typedef _Pointer   pointer;

#endif   // _RWSTD_NO_CLASS_PARTIAL_SPEC

    typedef _Iterator iterator_type;

    reverse_iterator () { }

    // 24.4.1.3.1, p1
    explicit reverse_iterator (iterator_type __rhs) : current (__rhs) { }
 

#ifndef _RWSTD_NO_CLASS_PARTIAL_SPEC 

    // 24.4.1.3.1, p2
    template <class _TypeU>
    reverse_iterator (const reverse_iterator<_TypeU>& __rhs)
        : current (__rhs.base ()) { }

#else   // if defined (_RWSTD_NO_CLASS_PARTIAL_SPEC)

    // 24.4.1.3.1, p2
    template <class _Iterator2, class _Category2, class _TypeU,
              class _Reference2, class _Pointer2, class _Distance2>
    reverse_iterator (const reverse_iterator<_Iterator2, _Category2, _TypeU,
                      _Reference2, _Pointer2, _Distance2>& __rhs)
        : current (__rhs.base ()) { }

#endif   // _RWSTD_NO_CLASS_PARTIAL_SPEC 


    // 24.4.1.3.2, p1
    iterator_type base () const {
        return current;
    }

    // 24.4.1.3.3, p1
    reference operator* () const {
        iterator_type __tmp (base ());
        return *--__tmp;
    }

    // 24.4.1.3.4, p1
    _RWSTD_OPERATOR_ARROW (pointer operator->() const);

    // 24.4.1.3.5, p1
    reverse_iterator& operator++ () {
        return --current, *this;
    }

    // 24.4.1.3.5, p3
    reverse_iterator operator++ (int) {
        reverse_iterator __tmp (*this);
        return ++*this, __tmp;
    }

    // 24.4.1.3.6, p1
    reverse_iterator& operator-- () {
        return ++current, *this;
    }

    // 24.4.1.3.6, p3
    reverse_iterator operator-- (int) {
        reverse_iterator __tmp (*this);
        return --*this, __tmp;
    }

    // 24.4.1.3.8, p1
    reverse_iterator& operator+= (difference_type __n) {
        return current -= __n, *this;
    }

    // 24.4.1.3.10, p1
    reverse_iterator& operator-= (difference_type __n) {
        return *this += -__n;
    }

    // 24.4.1.3.7, p1
    reverse_iterator operator+ (difference_type __n) const {
        return reverse_iterator (*this) += __n;
    }

    // 24.4.1.3.9, p1
    reverse_iterator operator- (difference_type __n) const {
        return reverse_iterator (*this) -= __n;
    }

    // 24.4.1.3.11, p1
    reference operator[] (difference_type __n) const {
        return *(*this + __n);
    }

protected:

    iterator_type current;
};


// 24.4.1.3.12
_RWSTD_ITER_TEMPLATE
inline bool operator== (const _RWSTD_ITER_ID (reverse_iterator)& __x,
                        const _RWSTD_ITER_ID (reverse_iterator)& __y)
{
    return __x.base () == __y.base ();
}


// 24.4.1.3.13
_RWSTD_ITER_TEMPLATE
inline bool operator< (const _RWSTD_ITER_ID (reverse_iterator)& __x,
                       const _RWSTD_ITER_ID (reverse_iterator)& __y)
{
    return __y.base() < __x.base();
}


// 24.4.1.3.14
_RWSTD_ITER_TEMPLATE
inline bool operator!= (const _RWSTD_ITER_ID (reverse_iterator)& __x, 
                        const _RWSTD_ITER_ID (reverse_iterator)& __y)
{
    return !(__x == __y);
}


// 24.4.1.3.15
_RWSTD_ITER_TEMPLATE
inline bool operator> (const _RWSTD_ITER_ID (reverse_iterator)& __x, 
                       const _RWSTD_ITER_ID (reverse_iterator)& __y)
{
    return __y < __x;
}


// 24.4.1.3.16
_RWSTD_ITER_TEMPLATE
inline bool operator>= (const _RWSTD_ITER_ID (reverse_iterator)& __x, 
                        const _RWSTD_ITER_ID (reverse_iterator)& __y)
{
    return !(__x < __y);
}


// 24.4.1.3.17
_RWSTD_ITER_TEMPLATE
inline bool operator<= (const _RWSTD_ITER_ID (reverse_iterator)& __x,
                        const _RWSTD_ITER_ID (reverse_iterator)& __y)
{
    return !(__y < __x);
}


// 24.4.1.3.18
_RWSTD_ITER_TEMPLATE
inline _RWSTD_ITER_DIFF_TYPE (_Iterator, _Distance)
operator- (const _RWSTD_ITER_ID (reverse_iterator)& __x, 
           const _RWSTD_ITER_ID (reverse_iterator)& __y)
{
    return __y.base () - __x.base ();
}


// 24.4.1.3.19
_RWSTD_ITER_TEMPLATE
inline _RWSTD_ITER_ID (reverse_iterator)
operator+ (_RWSTD_ITER_DIFF_TYPE (_Iterator, _Distance) __n, 
           const _RWSTD_ITER_ID (reverse_iterator)&     __x)
{
    return __x + __n;
}

#undef _RWSTD_ITER_DIFF_TYPE

#ifdef _RWSTD_NO_CLASS_PARTIAL_SPEC

}   // namespace std

_RWSTD_NAMESPACE (__rw) { 

// Reverse bidirectional iterator.       
// This is needed to get around non-standard compilers that insist
// on instantiating all members of a class whether they're used 
// or not.

template <class _Iterator, class _Category, class _TypeT, 
          class _Reference = _TypeT&, 
          class _Pointer = _TypeT*, 
          class _Distance = _RWSTD_PTRDIFF_T>
class __reverse_bi_iterator
    : public _STD::iterator<_Category, _TypeT, _Distance, _Pointer, _Reference>
{
public:
    typedef _Distance  difference_type;
    typedef _TypeT     value_type;
    typedef _Reference reference;
    typedef _Pointer   pointer;
    typedef _Iterator  iterator_type;

    __reverse_bi_iterator () { }

    explicit __reverse_bi_iterator (const iterator_type &__rhs) 
        : current (__rhs) { }


#ifndef _RWSTD_NO_CLASS_PARTIAL_SPEC 

      template <class _TypeU>
      __reverse_bi_iterator (const __reverse_bi_iterator<_TypeU>& __rhs)
          : current (__rhs.base ()) { }

#else   // if defined (_RWSTD_NO_CLASS_PARTIAL_SPEC )

      template <class _Iterator2, class _Category2, class _TypeU,
                class _Reference2, class _Pointer2, class _Distance2>
      __reverse_bi_iterator (const __reverse_bi_iterator<_Iterator2,
                                                         _Category2,
                                                         _TypeU,
                                                         _Reference2,
                                                         _Pointer2,
                                                         _Distance2>& __rhs)
          : current (__rhs.base ()) { }
#endif


    iterator_type base () const {
        return current;
    }

    reference operator* () const {
        iterator_type __tmp = base ();
        return *--__tmp;
    }

    _RWSTD_OPERATOR_ARROW (pointer operator->() const);

    __reverse_bi_iterator& operator++ () {
        return --current, *this;
    }

    __reverse_bi_iterator  operator++ (int) {
        __reverse_bi_iterator __tmp (*this);
        ++*this;
        return __tmp;
    }

    __reverse_bi_iterator& operator-- () {
        return ++current, *this;
    }

    __reverse_bi_iterator operator-- (int) {
        __reverse_bi_iterator __tmp (*this);
        --*this;
        return __tmp;
    }

protected:

    iterator_type current;
};


_RWSTD_ITER_TEMPLATE
inline bool operator== (const _RWSTD_ITER_ID (__reverse_bi_iterator)& __x, 
                        const _RWSTD_ITER_ID (__reverse_bi_iterator)& __y)
{
    return __x.base () == __y.base ();
}


_RWSTD_ITER_TEMPLATE
inline bool operator!= (const _RWSTD_ITER_ID (__reverse_bi_iterator)& __x,
                        const _RWSTD_ITER_ID (__reverse_bi_iterator)& __y)
{
    return !(__x == __y);
}

#undef _RWSTD_ITER_TEMPLATE
#undef _RWSTD_ITER_ID

}   // namespace __rw

_RWSTD_NAMESPACE (std) { 

#endif   // _RWSTD_NO_CLASS_PARTIAL_SPEC


#ifndef _RWSTD_NO_CLASS_PARTIAL_SPEC
#  define _RWSTD_INSERT_ITERATOR_BASE(ignore)       \
          iterator<output_iterator_tag, void, void, void, void>
#else
   // necessary to allow __iterator_category, __value_type, etc. to work
#  define _RWSTD_INSERT_ITERATOR_BASE(cont)         \
          iterator<output_iterator_tag,             \
                   typename cont::value_type,      \
                   typename cont::difference_type, \
                   typename cont::pointer,         \
                   typename cont::reference>
#endif   // _RWSTD_NO_CLASS_PARTIAL_SPEC


template <class _Container>
class back_insert_iterator: public _RWSTD_INSERT_ITERATOR_BASE (_Container)
{
public:
    typedef _Container container_type;

    explicit back_insert_iterator (container_type& __rhs)
        : container (&__rhs) { }

    back_insert_iterator&
    operator= (typename container_type::const_reference __x) {
        return container->push_back (__x), *this;
    }

    back_insert_iterator& operator*  () {
        return *this;
    }

    back_insert_iterator& operator++ () {
        return *this;
    }

    back_insert_iterator operator++ (int) {
        return *this;
    }

protected:

    container_type* container;
};


template <class _Container>
inline back_insert_iterator<_Container> back_inserter (_Container& __x)
{
    return back_insert_iterator<_Container>(__x);
}


template <class _Container>
class front_insert_iterator: public _RWSTD_INSERT_ITERATOR_BASE (_Container)
{
public:
    typedef _Container container_type;

    explicit front_insert_iterator (container_type& __rhs)
        : container (&__rhs) { }

    front_insert_iterator&
    operator= (typename container_type::const_reference __x) { 
        return container->push_front (__x), *this;
    }

    front_insert_iterator& operator* () {
        return *this;
    }

    front_insert_iterator& operator++ () {
        return *this;
    }

    front_insert_iterator operator++ (int) {
        return *this;
    }

protected:

    container_type* container;
};


template <class _Container>
inline front_insert_iterator<_Container> front_inserter (_Container& __x)
{
    return front_insert_iterator<_Container>(__x);
}


template <class _Container>
class insert_iterator: public _RWSTD_INSERT_ITERATOR_BASE (_Container)
{
public:
    typedef _Container container_type;

    insert_iterator (container_type                    &__x,
                     typename container_type::iterator __it)
      : iter (__it), container (&__x) { }

    insert_iterator&
    operator= (typename container_type::const_reference __x) { 
        iter = container->insert (iter, __x);
        return ++iter, *this;
    }

    insert_iterator& operator* () {
        return *this;
    }

    insert_iterator& operator++ () {
        return *this;
    }

    insert_iterator& operator++ (int) {
        return *this;
    }

protected:

    typename container_type::iterator iter;
    container_type*                    container;
};


template <class _Container, class _Iterator>
inline insert_iterator<_Container> inserter (_Container& __x, _Iterator __it)
{
    typedef typename _Container::iterator _Iter;

    return insert_iterator<_Container> (__x, _Iter (__it));
}


#ifdef _RWSTD_CXX11

// [move.iterators]
template <class _Iterator>
class move_iterator {
    _Iterator _C_current;
public:
    typedef _Iterator iterator_type;
    typedef typename iterator_traits<_Iterator>::iterator_category iterator_category;
    typedef typename iterator_traits<_Iterator>::value_type value_type;
    typedef typename iterator_traits<_Iterator>::difference_type difference_type;
    typedef _Iterator pointer;
    typedef typename iterator_traits<_Iterator>::reference _OriginalReference;
    typedef typename conditional<is_reference<_OriginalReference>::value,
        typename remove_reference<_OriginalReference>::type&&, _OriginalReference>::type reference;

    constexpr move_iterator () : _C_current () { }
    explicit constexpr move_iterator (_Iterator __it) : _C_current (std::move (__it)) { }
    template <class _Other, class = typename enable_if<is_convertible<_Other, _Iterator>::value>::type>
    constexpr move_iterator (const move_iterator<_Other>& __other) : _C_current (__other.base ()) { }
    template <class _Other>
    typename enable_if<is_assignable<_Iterator&, const _Other&>::value, move_iterator&>::type
    operator= (const move_iterator<_Other>& __other) { _C_current = __other.base (); return *this; }

    constexpr _Iterator base () const { return _C_current; }
    constexpr reference operator* () const { return static_cast<reference>(*_C_current); }
    constexpr pointer operator-> () const { return _C_current; }
    reference operator[] (difference_type __n) const { return static_cast<reference>(_C_current[__n]); }
    move_iterator& operator++ () { ++_C_current; return *this; }
    move_iterator operator++ (int) { move_iterator __old (*this); ++*this; return __old; }
    move_iterator& operator-- () { --_C_current; return *this; }
    move_iterator operator-- (int) { move_iterator __old (*this); --*this; return __old; }
    move_iterator& operator+= (difference_type __n) { _C_current += __n; return *this; }
    move_iterator& operator-= (difference_type __n) { _C_current -= __n; return *this; }
    move_iterator operator+ (difference_type __n) const { return move_iterator (_C_current + __n); }
    move_iterator operator- (difference_type __n) const { return move_iterator (_C_current - __n); }
};

template <class _Left, class _Right>
inline constexpr bool operator== (const move_iterator<_Left>& __left, const move_iterator<_Right>& __right)
{ return __left.base () == __right.base (); }
template <class _Left, class _Right>
inline constexpr bool operator!= (const move_iterator<_Left>& __left, const move_iterator<_Right>& __right)
{ return !(__left == __right); }
template <class _Left, class _Right>
inline constexpr bool operator< (const move_iterator<_Left>& __left, const move_iterator<_Right>& __right)
{ return __left.base () < __right.base (); }
template <class _Left, class _Right>
inline constexpr bool operator> (const move_iterator<_Left>& __left, const move_iterator<_Right>& __right)
{ return __right < __left; }
template <class _Left, class _Right>
inline constexpr bool operator<= (const move_iterator<_Left>& __left, const move_iterator<_Right>& __right)
{ return !(__right < __left); }
template <class _Left, class _Right>
inline constexpr bool operator>= (const move_iterator<_Left>& __left, const move_iterator<_Right>& __right)
{ return !(__left < __right); }
template <class _Left, class _Right>
inline auto operator- (const move_iterator<_Left>& __left, const move_iterator<_Right>& __right)
    -> decltype (__left.base () - __right.base ())
{ return __left.base () - __right.base (); }
template <class _Iterator>
inline move_iterator<_Iterator> operator+ (typename move_iterator<_Iterator>::difference_type __n,
                                          const move_iterator<_Iterator>& __it)
{ return __it + __n; }
template <class _Iterator>
inline constexpr move_iterator<_Iterator> make_move_iterator (_Iterator __it)
{ return move_iterator<_Iterator> (std::move (__it)); }

// [iterator.operations]
template <class _InputIter>
inline _InputIter next (_InputIter __it,
    typename iterator_traits<_InputIter>::difference_type __n = 1)
{
    std::advance (__it, __n);
    return __it;
}

template <class _BidirectionalIter>
inline _BidirectionalIter prev (_BidirectionalIter __it,
    typename iterator_traits<_BidirectionalIter>::difference_type __n = 1)
{
    std::advance (__it, -__n);
    return __it;
}

// [iterator.range]
template <class _Container>
inline auto begin (_Container& __c) -> decltype (__c.begin ())
{ return __c.begin (); }

template <class _Container>
inline auto begin (const _Container& __c) -> decltype (__c.begin ())
{ return __c.begin (); }

template <class _Container>
inline auto end (_Container& __c) -> decltype (__c.end ())
{ return __c.end (); }

template <class _Container>
inline auto end (const _Container& __c) -> decltype (__c.end ())
{ return __c.end (); }

template <class _Tp, _RWSTD_SIZE_T _Size>
inline _RWSTD_CONSTEXPR14 _Tp* begin (_Tp (&__a)[_Size]) noexcept
{ return __a; }

template <class _Tp, _RWSTD_SIZE_T _Size>
inline _RWSTD_CONSTEXPR14 _Tp* end (_Tp (&__a)[_Size]) noexcept
{ return __a + _Size; }

#if __cplusplus >= 201402L

template <class _Container>
inline constexpr auto cbegin (const _Container& __c)
    noexcept (noexcept (std::begin (__c))) -> decltype (std::begin (__c))
{ return std::begin (__c); }

template <class _Container>
inline constexpr auto cend (const _Container& __c)
    noexcept (noexcept (std::end (__c))) -> decltype (std::end (__c))
{ return std::end (__c); }

template <class _Container>
inline auto rbegin (_Container& __c) -> decltype (__c.rbegin ())
{ return __c.rbegin (); }

template <class _Container>
inline auto rbegin (const _Container& __c) -> decltype (__c.rbegin ())
{ return __c.rbegin (); }

template <class _Container>
inline auto rend (_Container& __c) -> decltype (__c.rend ())
{ return __c.rend (); }

template <class _Container>
inline auto rend (const _Container& __c) -> decltype (__c.rend ())
{ return __c.rend (); }

template <class _Tp, _RWSTD_SIZE_T _Size>
inline reverse_iterator<_Tp*> rbegin (_Tp (&__a)[_Size])
{ return reverse_iterator<_Tp*>(__a + _Size); }

template <class _Tp, _RWSTD_SIZE_T _Size>
inline reverse_iterator<_Tp*> rend (_Tp (&__a)[_Size])
{ return reverse_iterator<_Tp*>(__a); }

template <class _Tp>
inline reverse_iterator<const _Tp*> rbegin (initializer_list<_Tp> __list)
{ return reverse_iterator<const _Tp*>(__list.end ()); }

template <class _Tp>
inline reverse_iterator<const _Tp*> rend (initializer_list<_Tp> __list)
{ return reverse_iterator<const _Tp*>(__list.begin ()); }

template <class _Container>
inline auto crbegin (const _Container& __c) -> decltype (std::rbegin (__c))
{ return std::rbegin (__c); }

template <class _Container>
inline auto crend (const _Container& __c) -> decltype (std::rend (__c))
{ return std::rend (__c); }

#endif   // C++14
#endif   // _RWSTD_CXX11


}   // namespace std

#endif   // _RWSTD_RW_ITERATOR_H_INCLUDED
