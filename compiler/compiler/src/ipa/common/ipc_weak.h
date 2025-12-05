/*
 * Copyright 2003, 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


/* ====================================================================
 * ====================================================================
 *
 * Module: ipc_weak.h
 * $Source: /home/bos/bk/kpro64-pending/ipa/common/SCCS/s.ipc_weak.h $
 *
 * Revision history:
 *  14-Jun-95 - Original Version
 *  12-Apr-96 - Suppress C++ warnings
 *
 * Description:
 *
 *  Declare symbols defined in ld and referenced in ipa.so weak so that
 *  ipa.so will link cleanly.
 *
 * TODO:  C++ doesn't yet recognize #pragma weak, so we suppress this
 * all for C++ compilations.  Reinstate when C++ supports them.
 *
 * ====================================================================
 * ====================================================================
 */

#ifndef ipc_weak_INCLUDED
#define ipc_weak_INCLUDED


#ifdef __PROCESS_H__
extern int create_tmpdir (int);
extern string create_unique_file (const string, char);
extern string create_tmp_file (const string);
extern void add_to_tmp_file_list (string);
extern string tmpdir;
extern string *get_command_line (an_object_file_ptr, string, string, int*);
extern int make_link (const string, const string);
#endif /* __PROCESS_H__ */

#ifdef __LD_MAIN_H__
extern int arg_count;                  // TK debug
#endif /* __LD_MAIN_H__ */

#endif /* ipc_weak_INCLUDED */
