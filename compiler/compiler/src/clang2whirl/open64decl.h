/*
  Copyright (C) 2019-2020 XC5 Limited, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement 
  or the like.  Any license provided herein, whether implied or 
  otherwise, applies only to this software file.  Patent licenses, if 
  any, provided herein do not apply to combinations of this program with 
  other software, or any other product whatsoever.  

  You should have received a copy of the GNU General Public License along
  with this program; if not, write the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston MA 02111-1307, USA.

  http://www.xcalibyte.com

  For more information, see:
  http://github.com/open64-compiler/open64
  http://gitee.com/open64-compiler/open64

*/

#ifndef OPEN64DECL_H
#define OPEN64DECL_H

// forward declaration for open64 classes
typedef struct mem_pool MEM_POOL;
typedef struct pu_info PU_Info;
typedef struct DST_idx DST_INFO_IDX;

class WN;

class ST;

class TY;

class FLD_HANDLE;

// open64 type definitions: use Path64's own, whose widths differ from
// upstream Open64's (ST_IDX is pointer-sized, TY_IDX and STR_IDX 64-bit).
#include "defs.h"
#include "symtab_idx.h"
#include "mtypes.h"

#endif /* OPEN64DECL_H */
