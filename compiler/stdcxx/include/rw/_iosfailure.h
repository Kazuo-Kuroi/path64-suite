/***************************************************************************
 *
 * _iosfailure.h - definition of the ios_base::failure class
 *
 * This is an internal header file used to implement the C++ Standard
 * Library. It should never be #included directly by a program.
 *
 * $Id$
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
 * Copyright 1994-2008 Rogue Wave Software, Inc.
 * 
 **************************************************************************/

#ifndef _RWSTD_RW_IOSFAILURE_H_INCLUDED
#define _RWSTD_RW_IOSFAILURE_H_INCLUDED

#ifndef _RWSTD_RW_EXCEPTION_H_INCLUDED
#  include <rw/_exception.h>
#endif   // _RWSTD_RW_EXCEPTION_H_INCLUDED

#ifndef _RWSTD_RW_IOSFWD_H_INCLUDED
#  include <rw/_iosfwd.h>
#endif   // _RWSTD_RW_IOSFWD_H_INCLUDED


#include <rw/_system_error_base.h>
#ifdef _RWSTD_CXX11
#include <system_error>
#endif

_RWSTD_NAMESPACE (__rw) {

struct _RWSTD_EXPORT __rw_failure: __rw_system_error
{
#ifdef _RWSTD_CXX11
    explicit __rw_failure(const _STD::string& text,
                         const _STD::error_code& code = _STD::error_code::_C_stream())
        : __rw_system_error(code,text) {}
    explicit __rw_failure(const char* text = 0,
                         const _STD::error_code& code = _STD::error_code::_C_stream())
        : __rw_system_error(code,text ? text : "") {}
#else
    explicit __rw_failure(const _STD::string& text)
        : __rw_system_error(text,_STD::error_code::_C_stream()) {}
    explicit __rw_failure(const char* text = 0)
        : __rw_system_error(text,_STD::error_code::_C_stream()) {}
#endif
    virtual ~__rw_failure() _THROWS(());
};


// (optionally) thrown when stream sets badbit
struct _RWSTD_EXPORT __rw_badbit_set: __rw_failure
{
    virtual ~__rw_badbit_set () _THROWS (());
};


// (optionally) thrown when stream sets eofbit
struct _RWSTD_EXPORT __rw_eofbit_set: __rw_failure
{
    virtual ~__rw_eofbit_set () _THROWS (());
};


// (optionally) thrown when stream sets failbit
struct _RWSTD_EXPORT __rw_failbit_set: __rw_failure
{
    virtual ~__rw_failbit_set () _THROWS (());
};

}   // namespace __rw


#endif   // _RWSTD_RW_IOSFAILURE_H_INCLUDED
