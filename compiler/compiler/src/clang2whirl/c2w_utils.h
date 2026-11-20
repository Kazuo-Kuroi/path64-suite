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

#ifndef CLANG2WHIRL_UTILS_H
#define CLANG2WHIRL_UTILS_H

#include <assert.h>
#include <vector>

#include "open64decl.h"
#include "open64inc.h"

// forward clang declarations
#include "clangdecl.h"
#include "clanginc.h"
#include "c2w_result.h"

#define raw_assert(x) assert(x)

namespace wgen {

class NonCopyable {
protected:
  NonCopyable() {}

  ~NonCopyable() {}

private:
  NonCopyable(const NonCopyable &obj);

  NonCopyable &operator=(const NonCopyable &obj);
};

class WhirlBlockUtil {
private:
  static std::vector<WN *> blockVector;

public:
  static WN *nwBlock();
  
  static WN *getCurrentBlock();
  
  static void popCurrentBlock();
  
  static void pushBlock(WN *blk);
};

ST *Create_function(const char* name, TY_IDX ret_ty, ...);

ST *Create_function(const char* name, TY_IDX ret_ty, INT parm_cnt, TY_IDX *parm_ty);

ST *Create_tmp_sym(TY_IDX tyi, SRCPOS spos);

ST *Create_tmp_sym(TY_IDX tyi, const char *name, SRCPOS spos);

ST *Create_tmp_sym(TY_IDX tyi, STR_IDX strIdx, SRCPOS spos);

TY_IDX get_field_type(TY_IDX struct_type, UINT32 field_id);

FLD_HANDLE get_fld_and_offset(TY_IDX ty_idx, UINT32 field_id, UINT32& cur_field_id, UINT64& offset);

WN *Emit_dtor_call(ST_IDX st_idx, ST_IDX object_st = ST_IDX_ZERO);

ST_IDX Create_guard_symbol(STR_IDX name, SRCPOS spos, ST_IDX object_st);
WN *Emit_guard_abort(ST_IDX guard, SRCPOS spos);

WN *Emit_guarded_init(STR_IDX guard_str_idx, WN *ctor, ST_IDX dtor_st, SRCPOS spos,
                      ST_IDX object_st = ST_IDX_ZERO, ST_IDX guard_st = ST_IDX_ZERO);

WN *Gen_null_const(TY_IDX ty_idx);

WN *Handle_cond_wn(WN *wn);

WN *Handle_expr_for_copy(WN *blk, WN *expr, TY_IDX ty, SRCPOS spos,
                         const char *name = NULL);

// create a temp (preg or real temp) and store val into it
WN *WGEN_StidTemp(TY_IDX ty, WN *val, const char *name = NULL);

// Vectors element by element, through a memory temporary (a vector in a
// pseudo-register can't be addressed by element). C2W_Vector_Temp makes
// the temporary, storing VAL in it when VAL is not NULL.
ST *C2W_Vector_Temp(WN *blk, TY_IDX vty, WN *val, SRCPOS pos);
WN *C2W_Vector_Elt(ST *st, TY_IDX ety, INT idx);
void C2W_Vector_Set_Elt(WN *blk, ST *st, TY_IDX ety, INT idx, WN *val,
                        SRCPOS pos);

// WGEN wrapper functions for WHIRL
WN *WGEN_CreateComma(TYPE_ID rtype, WN* blk, WN* ldid);
WN *WGEN_CreateCselect(TYPE_ID rtype, WN *cond, WN *twn, WN *fwn);
WN *WGEN_CreateParm(TYPE_ID rtype, WN *parm, TY_IDX ty);
WN *WGEN_CreateReturn(SRCPOS spos);

long double convertToLongDouble(const llvm::APFloat val);
TCON_IDX createTconFromValue(TY_IDX ty_idx, long double value);
TCON_IDX Convert_float_to_tcon(TY_IDX ty_idx, const llvm::APFloat init);
TCON_IDX Convert_complex_float_to_tcon(TY_IDX ty_idx, const llvm::APFloat real, const llvm::APFloat imag);
TCON_IDX Convert_string_to_tcon(const clang::StringLiteral *expr, TY_IDX ty_idx);
TCON_IDX Gen_complex_tcon(TY_IDX ty_idx, UINT64 real_val, long double imagin_val);
TCON_IDX Convert_imaginary_to_tcon(const clang::ImaginaryLiteral *expr, Result elem, TY_IDX ty_idx);
#define CALL_REGION_STACK_MARK ((WN*)1)
bool Check_call_region();
bool Check_wn_from_call_region(WN *wn);
void Mark_call_region(WN *wn);
void Unmark_call_region();
void Clear_call_region();
void GenMstoreForString(WN *addr, WN *value, TY_IDX ty, UINT len, UINT off, SRCPOS spos);
void AddBitfieldInitvForTree(INT64 val, FLD_HANDLE fld, INITV_IDX &last_initv, UINT64 &bytes);
} // namespace wgen


#endif /* CLANG2WHIRL_UTILS_H */
