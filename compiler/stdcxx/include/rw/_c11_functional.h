// -*- C++ -*-
/***************************************************************************
 *
 * _c11_functional.h - C++11 and C++14 parts of <functional>
 *                     [function.objects]: reference_wrapper, invoke,
 *                     function, bind, mem_fn, hash
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

#ifndef _RWSTD_RW_C11_FUNCTIONAL_H_INCLUDED
#define _RWSTD_RW_C11_FUNCTIONAL_H_INCLUDED

#include <rw/_c11_tuple.h>
#include <exception>
#include <new>
#include <typeinfo>

namespace std {

// INVOKE [func.require]
namespace __rw_c11 {

template <class _Fp, class... _Args>
inline auto __invoke (_Fp&& __f, _Args&&... __args)
    -> decltype (std::forward<_Fp>(__f)(std::forward<_Args>(__args)...))
{
    return std::forward<_Fp>(__f)(std::forward<_Args>(__args)...);
}

template <class _Mp, class _A0, class... _Args,
          class _Cp = typename __rw_tt::__member_class<typename decay<_Mp>::type>::type,
          class = typename enable_if<is_member_function_pointer<typename decay<_Mp>::type>::value
                                     && is_base_of<_Cp, typename remove_reference<_A0>::type>::value>::type>
inline auto __invoke (_Mp&& __f, _A0&& __a0, _Args&&... __args)
    -> decltype ((std::forward<_A0>(__a0).*__f)(std::forward<_Args>(__args)...))
{
    return (std::forward<_A0>(__a0).*__f)(std::forward<_Args>(__args)...);
}

template <class _Mp, class _A0, class... _Args,
          class _Cp = typename __rw_tt::__member_class<typename decay<_Mp>::type>::type,
          class = typename enable_if<is_member_function_pointer<typename decay<_Mp>::type>::value
                                     && !is_base_of<_Cp, typename remove_reference<_A0>::type>::value>::type>
inline auto __invoke (_Mp&& __f, _A0&& __a0, _Args&&... __args)
    -> decltype (((*std::forward<_A0>(__a0)).*__f)(std::forward<_Args>(__args)...))
{
    return ((*std::forward<_A0>(__a0)).*__f)(std::forward<_Args>(__args)...);
}

template <class _Mp, class _A0,
          class _Cp = typename __rw_tt::__member_class<typename decay<_Mp>::type>::type,
          class = typename enable_if<is_member_object_pointer<typename decay<_Mp>::type>::value
                                     && is_base_of<_Cp, typename remove_reference<_A0>::type>::value>::type>
inline auto __invoke (_Mp&& __f, _A0&& __a0) -> decltype (std::forward<_A0>(__a0).*__f)
{
    return std::forward<_A0>(__a0).*__f;
}

template <class _Mp, class _A0,
          class _Cp = typename __rw_tt::__member_class<typename decay<_Mp>::type>::type,
          class = typename enable_if<is_member_object_pointer<typename decay<_Mp>::type>::value
                                     && !is_base_of<_Cp, typename remove_reference<_A0>::type>::value>::type>
inline auto __invoke (_Mp&& __f, _A0&& __a0) -> decltype ((*std::forward<_A0>(__a0)).*__f)
{
    return (*std::forward<_A0>(__a0)).*__f;
}

// INVOKE(f, args..., R): the result converted to R (or discarded, void)
template <class _Rp, bool = is_void<_Rp>::value>
struct __invoke_r
{
    template <class... _Args>
    static _Rp __call (_Args&&... __args)
    { return __invoke (std::forward<_Args>(__args)...); }
};
template <class _Rp> struct __invoke_r<_Rp, true>
{
    template <class... _Args>
    static void __call (_Args&&... __args)
    { __invoke (std::forward<_Args>(__args)...); }
};

// is INVOKE(F, Args...) valid and convertible to R
template <class _Rp, class _Fp, class... _Args>
struct __invokable_r
{
    template <class _Gp, class = decltype (__invoke (declval<_Gp>(), declval<_Args>()...))>
    static true_type __test (int);
    template <class _Gp> static false_type __test (...);
    typedef decltype (__test<_Fp>(0)) _Valid;

    template <class _Gp, bool = _Valid::value> struct __conv : false_type { };
    template <class _Gp> struct __conv<_Gp, true>
        : integral_constant<bool, is_void<_Rp>::value
                                  || is_convertible<decltype (__invoke (
                                         declval<_Gp>(), declval<_Args>()...)), _Rp>::value> { };
    static const bool value = __conv<_Fp>::value;
};

}   // namespace __rw_c11

// [refwrap]
template <class _Tp>
class reference_wrapper
{
    _Tp* __ptr;

    static void __bind_only (_Tp&) noexcept;
    static void __bind_only (_Tp&&) = delete;

public:
    typedef _Tp type;

    template <class _Up, class = decltype (__bind_only (declval<_Up>())),
              class = typename enable_if<!is_same<typename decay<_Up>::type,
                                                  reference_wrapper>::value>::type>
    reference_wrapper (_Up&& __u) noexcept
        : __ptr (__builtin_addressof (static_cast<_Tp&>(std::forward<_Up>(__u)))) { }

    reference_wrapper (const reference_wrapper&) noexcept = default;
    reference_wrapper& operator= (const reference_wrapper&) noexcept = default;

    operator _Tp& () const noexcept { return *__ptr; }
    _Tp& get () const noexcept { return *__ptr; }

    template <class... _Args>
    auto operator() (_Args&&... __args) const
        -> decltype (__rw_c11::__invoke (declval<_Tp&>(), std::forward<_Args>(__args)...))
    {
        return __rw_c11::__invoke (get (), std::forward<_Args>(__args)...);
    }
};

template <class _Tp>
inline reference_wrapper<_Tp> ref (_Tp& __t) noexcept
{ return reference_wrapper<_Tp>(__t); }
template <class _Tp>
inline reference_wrapper<_Tp> ref (reference_wrapper<_Tp> __t) noexcept
{ return std::ref (__t.get ()); }
template <class _Tp>
inline reference_wrapper<const _Tp> cref (const _Tp& __t) noexcept
{ return reference_wrapper<const _Tp>(__t); }
template <class _Tp>
inline reference_wrapper<const _Tp> cref (reference_wrapper<_Tp> __t) noexcept
{ return std::cref (__t.get ()); }
template <class _Tp> void ref (const _Tp&&) = delete;
template <class _Tp> void cref (const _Tp&&) = delete;

// [func.memfn]
namespace __rw_c11 {
template <class _Tp>
class __mem_fn
{
    _Tp __f;

public:
    typedef _Tp type;
    explicit __mem_fn (_Tp __f) noexcept : __f (__f) { }

    template <class... _Args>
    auto operator() (_Args&&... __args) const
        -> decltype (__invoke (declval<const _Tp&>(), std::forward<_Args>(__args)...))
    {
        return __invoke (__f, std::forward<_Args>(__args)...);
    }
};
}

template <class _Rp, class _Tp>
inline __rw_c11::__mem_fn<_Rp _Tp::*> mem_fn (_Rp _Tp::* __pm) noexcept
{
    return __rw_c11::__mem_fn<_Rp _Tp::*>(__pm);
}

// [func.wrap.badcall]
class bad_function_call : public exception
{
public:
    virtual const char* what () const noexcept { return "std::bad_function_call"; }
};

namespace __rw_c11 {
[[noreturn]] inline void __throw_bad_function_call ()
{
#ifndef _RWSTD_NO_EXCEPTIONS
    throw bad_function_call ();
#else
    __builtin_abort ();
#endif
}
}

// [func.wrap.func]
template <class _Fp> class function;

namespace __rw_c11 {

template <class _Fp> struct __is_function_object : false_type { };
template <class _Fp> struct __is_function_object<function<_Fp> > : true_type { };

// is a target "empty" (a null function or member pointer, an empty function)
template <class _Fp>
inline bool __not_null (const _Fp&) { return true; }
template <class _Rp, class... _Args>
inline bool __not_null (_Rp (*__p)(_Args...)) { return __p != 0; }
template <class _Rp, class _Cp>
inline bool __not_null (_Rp _Cp::* __p) { return __p != 0; }
template <class _Rp, class... _Args>
inline bool __not_null (const function<_Rp (_Args...)>& __f) { return !!__f; }

// the type-erased callable: one vtable of functions per target type
template <class _Rp, class... _Args>
struct __func_vtable
{
    _Rp (*__call) (void*, _Args&&...);
    void (*__copy) (const void*, void*);     // copy-construct into storage
    void (*__move) (void*, void*) noexcept;  // move into storage, destroy source
    void (*__destroy) (void*) noexcept;
    const std::type_info& (*__type) () noexcept;
    bool __local;                            // stored in the small buffer
};

// the small buffer: callables up to three pointers big that move without
// throwing are stored in the function object itself
union __func_storage
{
    void* __heap;
    alignas (alignof (max_align_t) < 16 ? alignof (max_align_t) : 16)
        unsigned char __buf [3 * sizeof (void*)];
};

template <class _Fn>
struct __func_is_local
    : integral_constant<bool, sizeof (_Fn) <= sizeof (__func_storage)
                              && alignof (__func_storage) % alignof (_Fn) == 0
                              && is_nothrow_move_constructible<_Fn>::value> { };

template <class _Fn, class _Rp, class... _Args>
struct __func_impl
{
    static const bool __local = __func_is_local<_Fn>::value;

    static _Fn* __target (void* __s)
    {
        return __local ? static_cast<_Fn*>(__s)
                       : static_cast<_Fn*>(static_cast<__func_storage*>(__s)->__heap);
    }

    static _Rp __call (void* __s, _Args&&... __args)
    {
        return __invoke_r<_Rp>::__call (*__target (__s), std::forward<_Args>(__args)...);
    }

    static void __copy (const void* __from, void* __to)
    {
        const _Fn& __f = *__target (const_cast<void*>(__from));
        if (__local)
            ::new (__to) _Fn (__f);
        else
            static_cast<__func_storage*>(__to)->__heap = new _Fn (__f);
    }

    static void __move (void* __from, void* __to) noexcept
    {
        if (__local) {
            _Fn* __f = static_cast<_Fn*>(__from);
            ::new (__to) _Fn (std::move (*__f));
            __f->~_Fn ();
        }
        else
            static_cast<__func_storage*>(__to)->__heap =
                static_cast<__func_storage*>(__from)->__heap;
    }

    static void __destroy (void* __s) noexcept
    {
        if (__local)
            static_cast<_Fn*>(__s)->~_Fn ();
        else
            delete static_cast<_Fn*>(static_cast<__func_storage*>(__s)->__heap);
    }

    static const std::type_info& __type () noexcept { return typeid (_Fn); }

    static const __func_vtable<_Rp, _Args...> __vtable;
};

template <class _Fn, class _Rp, class... _Args>
const __func_vtable<_Rp, _Args...> __func_impl<_Fn, _Rp, _Args...>::__vtable = {
    &__func_impl::__call, &__func_impl::__copy, &__func_impl::__move,
    &__func_impl::__destroy, &__func_impl::__type, __func_impl::__local
};

}   // namespace __rw_c11

template <class _Rp, class... _Args>
class function<_Rp (_Args...)>
{
    typedef __rw_c11::__func_vtable<_Rp, _Args...> _VTable;

    __rw_c11::__func_storage __s;
    const _VTable* __vt;

    template <class _Fp>
    struct __callable
        : integral_constant<bool, !is_same<typename decay<_Fp>::type, function>::value
                                  && __rw_c11::__invokable_r<_Rp, typename decay<_Fp>::type&,
                                                             _Args...>::value> { };

    void* __storage () noexcept { return &__s; }
    const void* __storage () const noexcept { return &__s; }

public:
    typedef _Rp result_type;

    // [func.wrap.func.con]
    function () noexcept : __vt (0) { }
    function (decltype (nullptr)) noexcept : __vt (0) { }

    function (const function& __f) : __vt (0)
    {
        if (__f.__vt) {
            __f.__vt->__copy (__f.__storage (), __storage ());
            __vt = __f.__vt;
        }
    }

    function (function&& __f) noexcept : __vt (0)
    {
        if (__f.__vt) {
            __f.__vt->__move (__f.__storage (), __storage ());
            __vt = __f.__vt;
            __f.__vt = 0;
        }
    }

    template <class _Fp, class = typename enable_if<__callable<_Fp>::value>::type>
    function (_Fp __f) : __vt (0)
    {
        typedef __rw_c11::__func_impl<_Fp, _Rp, _Args...> _Impl;
        if (__rw_c11::__not_null (__f)) {
            if (_Impl::__local)
                ::new (__storage ()) _Fp (std::move (__f));
            else
                __s.__heap = new _Fp (std::move (__f));
            __vt = &_Impl::__vtable;
        }
    }

    template <class _Alloc>
    function (allocator_arg_t, const _Alloc&) noexcept : __vt (0) { }
    template <class _Alloc>
    function (allocator_arg_t, const _Alloc&, decltype (nullptr)) noexcept : __vt (0) { }
    template <class _Alloc>
    function (allocator_arg_t, const _Alloc&, const function& __f) : function (__f) { }
    template <class _Alloc>
    function (allocator_arg_t, const _Alloc&, function&& __f)
        : function (std::move (__f)) { }
    template <class _Fp, class _Alloc,
              class = typename enable_if<__callable<_Fp>::value>::type>
    function (allocator_arg_t, const _Alloc&, _Fp __f) : function (std::move (__f)) { }

    function& operator= (const function& __f)
    {
        function (__f).swap (*this);
        return *this;
    }

    function& operator= (function&& __f) noexcept
    {
        function (std::move (__f)).swap (*this);
        return *this;
    }

    function& operator= (decltype (nullptr)) noexcept
    {
        __reset ();
        return *this;
    }

    template <class _Fp, class = typename enable_if<__callable<_Fp>::value>::type>
    function& operator= (_Fp&& __f)
    {
        function (std::forward<_Fp>(__f)).swap (*this);
        return *this;
    }

    template <class _Fp>
    function& operator= (reference_wrapper<_Fp> __f) noexcept
    {
        function (__f).swap (*this);
        return *this;
    }

    ~function () { __reset (); }

    // [func.wrap.func.mod]
    void swap (function& __f) noexcept
    {
        if (&__f == this)
            return;
        __rw_c11::__func_storage __tmp;
        const _VTable* __tvt = __vt;
        if (__vt)
            __vt->__move (__storage (), &__tmp);
        if (__f.__vt)
            __f.__vt->__move (__f.__storage (), __storage ());
        if (__tvt)
            __tvt->__move (&__tmp, __f.__storage ());
        __vt = __f.__vt;
        __f.__vt = __tvt;
    }

    template <class _Fp, class _Alloc>
    void assign (_Fp&& __f, const _Alloc& __a)
    {
        function (allocator_arg, __a, std::forward<_Fp>(__f)).swap (*this);
    }

    // [func.wrap.func.cap]
    explicit operator bool () const noexcept { return __vt != 0; }

    // [func.wrap.func.inv]
    _Rp operator() (_Args... __args) const
    {
        if (!__vt)
            __rw_c11::__throw_bad_function_call ();
        return __vt->__call (const_cast<void*>(__storage ()),
                             std::forward<_Args>(__args)...);
    }

    // [func.wrap.func.targ]
    const std::type_info& target_type () const noexcept
    {
        return __vt ? __vt->__type () : typeid (void);
    }

    template <class _Tp>
    _Tp* target () noexcept
    {
        if (!__vt || __vt->__type () != typeid (_Tp))
            return 0;
        return __vt->__local ? static_cast<_Tp*>(__storage ())
                             : static_cast<_Tp*>(__s.__heap);
    }

    template <class _Tp>
    const _Tp* target () const noexcept
    {
        return const_cast<function*>(this)->template target<_Tp>();
    }

private:
    void __reset () noexcept
    {
        if (__vt) {
            const _VTable* __v = __vt;
            __vt = 0;
            __v->__destroy (__storage ());
        }
    }
};

template <class _Rp, class... _Args>
inline bool operator== (const function<_Rp (_Args...)>& __f, decltype (nullptr)) noexcept
{ return !__f; }
template <class _Rp, class... _Args>
inline bool operator== (decltype (nullptr), const function<_Rp (_Args...)>& __f) noexcept
{ return !__f; }
template <class _Rp, class... _Args>
inline bool operator!= (const function<_Rp (_Args...)>& __f, decltype (nullptr)) noexcept
{ return (bool)__f; }
template <class _Rp, class... _Args>
inline bool operator!= (decltype (nullptr), const function<_Rp (_Args...)>& __f) noexcept
{ return (bool)__f; }

template <class _Rp, class... _Args>
inline void swap (function<_Rp (_Args...)>& __x, function<_Rp (_Args...)>& __y) noexcept
{ __x.swap (__y); }

template <class _Rp, class... _Args, class _Alloc>
struct uses_allocator<function<_Rp (_Args...)>, _Alloc> : true_type { };

// [func.bind]
template <class _Tp> struct is_bind_expression : false_type { };
template <class _Tp> struct is_bind_expression<const _Tp> : is_bind_expression<_Tp> { };
template <class _Tp> struct is_placeholder : integral_constant<int, 0> { };
template <class _Tp> struct is_placeholder<const _Tp> : is_placeholder<_Tp> { };

namespace placeholders {
template <int _Np> struct __ph { };
constexpr __ph<1> _1 = __ph<1>();
constexpr __ph<2> _2 = __ph<2>();
constexpr __ph<3> _3 = __ph<3>();
constexpr __ph<4> _4 = __ph<4>();
constexpr __ph<5> _5 = __ph<5>();
constexpr __ph<6> _6 = __ph<6>();
constexpr __ph<7> _7 = __ph<7>();
constexpr __ph<8> _8 = __ph<8>();
constexpr __ph<9> _9 = __ph<9>();
constexpr __ph<10> _10 = __ph<10>();
}
template <int _Np> struct is_placeholder<placeholders::__ph<_Np> >
    : integral_constant<int, _Np> { };

template <class _Fp, class... _BoundArgs> class __rw_bind;
template <class _Rp, class _Fp, class... _BoundArgs> class __rw_bind_r;

template <class _Fp, class... _BoundArgs>
struct is_bind_expression<__rw_bind<_Fp, _BoundArgs...> > : true_type { };
template <class _Rp, class _Fp, class... _BoundArgs>
struct is_bind_expression<__rw_bind_r<_Rp, _Fp, _BoundArgs...> > : true_type { };

namespace __rw_c11 {

template <class _Tp> struct __is_refwrap : false_type { };
template <class _Tp> struct __is_refwrap<reference_wrapper<_Tp> > : true_type { };

// [func.bind.bind] 10: the value given for a bound argument _Ti when the
// call's arguments are the tuple _Uj (of rvalue or lvalue references)
template <class _Tp, class _Uj>
inline _Tp& __mu (reference_wrapper<_Tp> __t, _Uj&) { return __t.get (); }

template <class _Ti, class... _Uj, size_t... _Indx>
inline auto __mu_expand (_Ti& __ti, tuple<_Uj...>& __uj, __tuple_indices<_Indx...>)
    -> decltype (__ti (std::forward<_Uj>(std::get<_Indx>(__uj))...))
{
    return __ti (std::forward<_Uj>(std::get<_Indx>(__uj))...);
}

template <class _Ti, class... _Uj, class = typename enable_if<
    is_bind_expression<_Ti>::value>::type>
inline auto __mu (_Ti& __ti, tuple<_Uj...>& __uj)
    -> decltype (__mu_expand (__ti, __uj,
                              typename __make_tuple_indices<sizeof... (_Uj)>::type ()))
{
    return __mu_expand (__ti, __uj,
                        typename __make_tuple_indices<sizeof... (_Uj)>::type ());
}

template <class _Ti, class _Uj, int _Np = is_placeholder<_Ti>::value,
          class = typename enable_if<(_Np > 0)>::type>
inline auto __mu (_Ti&, _Uj& __uj)
    -> decltype (std::forward<typename tuple_element<_Np - 1, _Uj>::type>(
                     std::get<_Np - 1>(__uj)))
{
    return std::forward<typename tuple_element<_Np - 1, _Uj>::type>(
        std::get<_Np - 1>(__uj));
}

template <class _Ti, class _Uj, class = typename enable_if<
    !is_bind_expression<_Ti>::value && is_placeholder<_Ti>::value == 0
    && !__is_refwrap<typename remove_cv<_Ti>::type>::value>::type>
inline _Ti& __mu (_Ti& __ti, _Uj&) { return __ti; }

template <class _Fp, class _BoundArgs, size_t... _Indx, class _Args>
inline auto __apply_bind (_Fp& __f, _BoundArgs& __bound, __tuple_indices<_Indx...>,
                          _Args&& __args)
    -> decltype (__invoke (__f, __mu (std::get<_Indx>(__bound), __args)...))
{
    return __invoke (__f, __mu (std::get<_Indx>(__bound), __args)...);
}

template <class _Rp, class _Fp, class _BoundArgs, size_t... _Indx, class _Args>
inline _Rp __apply_bind_r (_Fp& __f, _BoundArgs& __bound, __tuple_indices<_Indx...>,
                           _Args&& __args)
{
    return __invoke_r<_Rp>::__call (__f, __mu (std::get<_Indx>(__bound), __args)...);
}

}   // namespace __rw_c11

template <class _Fp, class... _BoundArgs>
class __rw_bind
{
protected:
    typedef typename decay<_Fp>::type _Fd;
    typedef tuple<typename decay<_BoundArgs>::type...> _Td;
    typedef typename __rw_c11::__make_tuple_indices<sizeof... (_BoundArgs)>::type _Indices;

    _Fd __f;
    _Td __bound_args;

public:
    template <class _Gp, class... _BA, class = typename enable_if<
        is_constructible<_Fd, _Gp>::value
        && !is_same<typename remove_reference<_Gp>::type, __rw_bind>::value>::type>
    explicit __rw_bind (_Gp&& __g, _BA&&... __bound)
        : __f (std::forward<_Gp>(__g)), __bound_args (std::forward<_BA>(__bound)...) { }

    template <class... _Args>
    auto operator() (_Args&&... __args)
        -> decltype (__rw_c11::__apply_bind (declval<_Fd&>(), declval<_Td&>(), _Indices (),
                                             declval<tuple<_Args&&...> >()))
    {
        return __rw_c11::__apply_bind (__f, __bound_args, _Indices (),
                                       tuple<_Args&&...>(std::forward<_Args>(__args)...));
    }

    template <class... _Args>
    auto operator() (_Args&&... __args) const
        -> decltype (__rw_c11::__apply_bind (declval<const _Fd&>(), declval<const _Td&>(),
                                             _Indices (), declval<tuple<_Args&&...> >()))
    {
        return __rw_c11::__apply_bind (__f, __bound_args, _Indices (),
                                       tuple<_Args&&...>(std::forward<_Args>(__args)...));
    }
};

template <class _Rp, class _Fp, class... _BoundArgs>
class __rw_bind_r : public __rw_bind<_Fp, _BoundArgs...>
{
    typedef __rw_bind<_Fp, _BoundArgs...> _Base;

public:
    typedef _Rp result_type;

    template <class _Gp, class... _BA, class = typename enable_if<
        is_constructible<typename _Base::_Fd, _Gp>::value
        && !is_same<typename remove_reference<_Gp>::type, __rw_bind_r>::value>::type>
    explicit __rw_bind_r (_Gp&& __g, _BA&&... __bound)
        : _Base (std::forward<_Gp>(__g), std::forward<_BA>(__bound)...) { }

    template <class... _Args>
    _Rp operator() (_Args&&... __args)
    {
        return __rw_c11::__apply_bind_r<_Rp>(
            this->__f, this->__bound_args, typename _Base::_Indices (),
            tuple<_Args&&...>(std::forward<_Args>(__args)...));
    }

    template <class... _Args>
    _Rp operator() (_Args&&... __args) const
    {
        return __rw_c11::__apply_bind_r<_Rp>(
            this->__f, this->__bound_args, typename _Base::_Indices (),
            tuple<_Args&&...>(std::forward<_Args>(__args)...));
    }
};

template <class _Fp, class... _BoundArgs>
inline __rw_bind<_Fp, _BoundArgs...> bind (_Fp&& __f, _BoundArgs&&... __bound)
{
    return __rw_bind<_Fp, _BoundArgs...>(std::forward<_Fp>(__f),
                                         std::forward<_BoundArgs>(__bound)...);
}

template <class _Rp, class _Fp, class... _BoundArgs>
inline __rw_bind_r<_Rp, _Fp, _BoundArgs...> bind (_Fp&& __f, _BoundArgs&&... __bound)
{
    return __rw_bind_r<_Rp, _Fp, _BoundArgs...>(std::forward<_Fp>(__f),
                                                 std::forward<_BoundArgs>(__bound)...);
}

// [unord.hash]
namespace __rw_c11 {
// FNV-1a over the bytes
inline size_t __hash_bytes (const void* __p, size_t __n) noexcept
{
    const unsigned char* __c = static_cast<const unsigned char*>(__p);
    size_t __h = sizeof (size_t) == 8 ? size_t (14695981039346656037ULL) : size_t (2166136261U);
    const size_t __prime = sizeof (size_t) == 8 ? size_t (1099511628211ULL) : size_t (16777619U);
    for (size_t __i = 0; __i != __n; ++__i)
        __h = (__h ^ __c [__i]) * __prime;
    return __h;
}

// the primary template: enums hash as their underlying type (LWG 2148);
// other types have no hash (it can't be called)
template <class _Tp, bool = is_enum<_Tp>::value>
struct __enum_hash
{
    typedef _Tp argument_type;
    typedef size_t result_type;
    size_t operator() (_Tp __v) const noexcept
    {
        typedef typename underlying_type<_Tp>::type _Up;
        return static_cast<size_t>(static_cast<_Up>(__v));
    }
};
template <class _Tp>
struct __enum_hash<_Tp, false>
{
    __enum_hash () = delete;
    __enum_hash (const __enum_hash&) = delete;
    __enum_hash& operator= (const __enum_hash&) = delete;
};

template <class _Tp>
struct __scalar_hash
{
    typedef _Tp argument_type;
    typedef size_t result_type;
    size_t operator() (_Tp __v) const noexcept { return static_cast<size_t>(__v); }
};

template <class _Tp>
struct __float_hash
{
    typedef _Tp argument_type;
    typedef size_t result_type;
    size_t operator() (_Tp __v) const noexcept
    {
        if (__v == 0)
            return 0;   // +0.0 and -0.0
        return __hash_bytes (&__v, sizeof (__v) == 16 ? 10 : sizeof (__v));
    }
};
}   // namespace __rw_c11

template <class _Tp> struct hash : __rw_c11::__enum_hash<_Tp> { };

template <class _Tp> struct hash<_Tp*>
{
    typedef _Tp* argument_type;
    typedef size_t result_type;
    size_t operator() (_Tp* __p) const noexcept
    { return reinterpret_cast<size_t>(__p); }
};

#define _RWSTD_C11_SCALAR_HASH(T) \
    template <> struct hash<T> : __rw_c11::__scalar_hash<T> { };
_RWSTD_C11_SCALAR_HASH (bool)
_RWSTD_C11_SCALAR_HASH (char)
_RWSTD_C11_SCALAR_HASH (signed char)
_RWSTD_C11_SCALAR_HASH (unsigned char)
_RWSTD_C11_SCALAR_HASH (char16_t)
_RWSTD_C11_SCALAR_HASH (char32_t)
_RWSTD_C11_SCALAR_HASH (wchar_t)
_RWSTD_C11_SCALAR_HASH (short)
_RWSTD_C11_SCALAR_HASH (unsigned short)
_RWSTD_C11_SCALAR_HASH (int)
_RWSTD_C11_SCALAR_HASH (unsigned int)
_RWSTD_C11_SCALAR_HASH (long)
_RWSTD_C11_SCALAR_HASH (unsigned long)
_RWSTD_C11_SCALAR_HASH (long long)
_RWSTD_C11_SCALAR_HASH (unsigned long long)
#undef _RWSTD_C11_SCALAR_HASH

template <> struct hash<float> : __rw_c11::__float_hash<float> { };
template <> struct hash<double> : __rw_c11::__float_hash<double> { };
template <> struct hash<long double> : __rw_c11::__float_hash<long double> { };

template <> struct hash<decltype (nullptr)>
{
    typedef decltype (nullptr) argument_type;
    typedef size_t result_type;
    size_t operator() (decltype (nullptr)) const noexcept { return 0; }
};

}   // namespace std

#endif   // _RWSTD_RW_C11_FUNCTIONAL_H_INCLUDED
