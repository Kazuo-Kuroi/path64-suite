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

#include "c2w_expr.h"
#include "c2w_builder.h"
#include "c2w_tracer.h"
#include "c2w_utils.h"
#include <map>
#include <string>
#include <vector>

// clang header files
#include "clanginc.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/TargetBuiltins.h"

// open64 header files
#include "open64inc.h"

using namespace clang;

namespace wgen {

// builtin table
INTRINSIC BuiltinTable[Builtin::FirstTSBuiltin];

// __sync_xxx builtin table
INTRINSIC SyncBuiltinTable[Builtin::BI__sync_swap_16 - Builtin::BI__sync_fetch_and_add + 1];

// x86 builtin table
INTRINSIC TargX86BuiltinTable[X86::LastTSBuiltin - X86::LastTIBuiltin];

void
WhirlExprBuilder::InitializeBuiltinTable() {
  // initialize builtin table, in alphabeta order
  // Path64 has no intrinsics for bswap, isinf_sign, isnormal,
  // object_size or pow; those builtins are not in the table.
  using namespace clang::Builtin;
  BuiltinTable[BI__builtin_clzs]  = INTRN_CLZ32;
  BuiltinTable[BI__builtin_clz]   = INTRN_CLZ32;
  BuiltinTable[BI__builtin_clzl]  = INTRN_CLZ32;
  BuiltinTable[BI__builtin_clzll] = INTRN_CLZ;
  BuiltinTable[BI__builtin_ctzs]  = INTRN_CTZ;
  BuiltinTable[BI__builtin_ctz]   = INTRN_CTZ;
  BuiltinTable[BI__builtin_ctzl]  = INTRN_CTZ;
  BuiltinTable[BI__builtin_ctzll] = INTRN_CTZ64;


  BuiltinTable[BI__builtin_isgreater]      = INTRN_ISGREATER;
  BuiltinTable[BI__builtin_isgreaterequal] = INTRN_ISGREATEREQUAL;
  BuiltinTable[BI__builtin_isless]         = INTRN_ISLESS;
  BuiltinTable[BI__builtin_islessequal]    = INTRN_ISLESSEQUAL;
  BuiltinTable[BI__builtin_islessgreater]  = INTRN_ISLESSGREATER;
  BuiltinTable[BI__builtin_isunordered]    = INTRN_ISUNORDERED;


  BuiltinTable[BI__builtin_popcount]   = INTRN_I4POPCNT;
  BuiltinTable[BI__builtin_popcountl]  = INTRN_I8POPCNT;
  BuiltinTable[BI__builtin_popcountll] = INTRN_I8POPCNT;


  // the back end gives the target's register number, as for wgen
  BuiltinTable[BI__builtin_eh_return_data_regno] = INTRN_BUILTIN_EH_RETURN_DATA_REGNO;
  BuiltinTable[BI__builtin_return_address] = INTRN_RETURN_ADDRESS;
  BuiltinTable[BI__builtin_extract_return_addr] = INTRN_RETURN_ADDRESS;
  BuiltinTable[BI__builtin_frame_address] = INTRN_U8READFRAMEPOINTER;

  BuiltinTable[BImemcmp]  = INTRN_MEMCMP;
  BuiltinTable[BImemcpy]  = INTRN_MEMCPY;
  BuiltinTable[BImemset]  = INTRN_MEMSET;
  BuiltinTable[BIstrcmp]  = INTRN_STRCMP;
  BuiltinTable[BIstrcpy]  = INTRN_STRCPY;
  BuiltinTable[BIstrlen]  = INTRN_STRLEN;
  BuiltinTable[BIstrncpy] = INTRN_STRNCPY;

  // __sync_xxx builtin table
  // always use _I1 and adjust intrinsic op according to the pointee size
  SyncBuiltinTable[BI__sync_fetch_and_add    - BI__sync_fetch_and_add] = INTRN_FETCH_AND_ADD_I4;
  SyncBuiltinTable[BI__sync_fetch_and_add_1  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_ADD_I4;
  SyncBuiltinTable[BI__sync_fetch_and_add_2  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_ADD_I4;
  SyncBuiltinTable[BI__sync_fetch_and_add_4  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_ADD_I4;
  SyncBuiltinTable[BI__sync_fetch_and_add_8  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_ADD_I4;
  SyncBuiltinTable[BI__sync_fetch_and_add_16 - BI__sync_fetch_and_add] = INTRN_FETCH_AND_ADD_I4;

  SyncBuiltinTable[BI__sync_fetch_and_sub    - BI__sync_fetch_and_add] = INTRN_FETCH_AND_SUB_I4;
  SyncBuiltinTable[BI__sync_fetch_and_sub_1  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_SUB_I4;
  SyncBuiltinTable[BI__sync_fetch_and_sub_2  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_SUB_I4;
  SyncBuiltinTable[BI__sync_fetch_and_sub_4  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_SUB_I4;
  SyncBuiltinTable[BI__sync_fetch_and_sub_8  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_SUB_I4;
  SyncBuiltinTable[BI__sync_fetch_and_sub_16 - BI__sync_fetch_and_add] = INTRN_FETCH_AND_SUB_I4;

  SyncBuiltinTable[BI__sync_fetch_and_or    - BI__sync_fetch_and_add] = INTRN_FETCH_AND_OR_I4;
  SyncBuiltinTable[BI__sync_fetch_and_or_1  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_OR_I4;
  SyncBuiltinTable[BI__sync_fetch_and_or_2  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_OR_I4;
  SyncBuiltinTable[BI__sync_fetch_and_or_4  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_OR_I4;
  SyncBuiltinTable[BI__sync_fetch_and_or_8  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_OR_I4;
  SyncBuiltinTable[BI__sync_fetch_and_or_16 - BI__sync_fetch_and_add] = INTRN_FETCH_AND_OR_I4;

  SyncBuiltinTable[BI__sync_fetch_and_and    - BI__sync_fetch_and_add] = INTRN_FETCH_AND_AND_I4;
  SyncBuiltinTable[BI__sync_fetch_and_and_1  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_AND_I4;
  SyncBuiltinTable[BI__sync_fetch_and_and_2  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_AND_I4;
  SyncBuiltinTable[BI__sync_fetch_and_and_4  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_AND_I4;
  SyncBuiltinTable[BI__sync_fetch_and_and_8  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_AND_I4;
  SyncBuiltinTable[BI__sync_fetch_and_and_16 - BI__sync_fetch_and_add] = INTRN_FETCH_AND_AND_I4;

  SyncBuiltinTable[BI__sync_fetch_and_xor    - BI__sync_fetch_and_add] = INTRN_FETCH_AND_XOR_I4;
  SyncBuiltinTable[BI__sync_fetch_and_xor_1  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_XOR_I4;
  SyncBuiltinTable[BI__sync_fetch_and_xor_2  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_XOR_I4;
  SyncBuiltinTable[BI__sync_fetch_and_xor_4  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_XOR_I4;
  SyncBuiltinTable[BI__sync_fetch_and_xor_8  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_XOR_I4;
  SyncBuiltinTable[BI__sync_fetch_and_xor_16 - BI__sync_fetch_and_add] = INTRN_FETCH_AND_XOR_I4;

  SyncBuiltinTable[BI__sync_fetch_and_nand    - BI__sync_fetch_and_add] = INTRN_FETCH_AND_NAND_I4;
  SyncBuiltinTable[BI__sync_fetch_and_nand_1  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_NAND_I4;
  SyncBuiltinTable[BI__sync_fetch_and_nand_2  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_NAND_I4;
  SyncBuiltinTable[BI__sync_fetch_and_nand_4  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_NAND_I4;
  SyncBuiltinTable[BI__sync_fetch_and_nand_8  - BI__sync_fetch_and_add] = INTRN_FETCH_AND_NAND_I4;
  SyncBuiltinTable[BI__sync_fetch_and_nand_16 - BI__sync_fetch_and_add] = INTRN_FETCH_AND_NAND_I4;

  SyncBuiltinTable[BI__sync_add_and_fetch    - BI__sync_fetch_and_add] = INTRN_ADD_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_add_and_fetch_1  - BI__sync_fetch_and_add] = INTRN_ADD_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_add_and_fetch_2  - BI__sync_fetch_and_add] = INTRN_ADD_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_add_and_fetch_4  - BI__sync_fetch_and_add] = INTRN_ADD_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_add_and_fetch_8  - BI__sync_fetch_and_add] = INTRN_ADD_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_add_and_fetch_16 - BI__sync_fetch_and_add] = INTRN_ADD_AND_FETCH_I4;

  SyncBuiltinTable[BI__sync_sub_and_fetch    - BI__sync_fetch_and_add] = INTRN_SUB_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_sub_and_fetch_1  - BI__sync_fetch_and_add] = INTRN_SUB_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_sub_and_fetch_2  - BI__sync_fetch_and_add] = INTRN_SUB_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_sub_and_fetch_4  - BI__sync_fetch_and_add] = INTRN_SUB_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_sub_and_fetch_8  - BI__sync_fetch_and_add] = INTRN_SUB_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_sub_and_fetch_16 - BI__sync_fetch_and_add] = INTRN_SUB_AND_FETCH_I4;

  SyncBuiltinTable[BI__sync_or_and_fetch    - BI__sync_fetch_and_add] = INTRN_OR_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_or_and_fetch_1  - BI__sync_fetch_and_add] = INTRN_OR_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_or_and_fetch_2  - BI__sync_fetch_and_add] = INTRN_OR_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_or_and_fetch_4  - BI__sync_fetch_and_add] = INTRN_OR_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_or_and_fetch_8  - BI__sync_fetch_and_add] = INTRN_OR_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_or_and_fetch_16 - BI__sync_fetch_and_add] = INTRN_OR_AND_FETCH_I4;

  SyncBuiltinTable[BI__sync_and_and_fetch    - BI__sync_fetch_and_add] = INTRN_AND_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_and_and_fetch_1  - BI__sync_fetch_and_add] = INTRN_AND_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_and_and_fetch_2  - BI__sync_fetch_and_add] = INTRN_AND_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_and_and_fetch_4  - BI__sync_fetch_and_add] = INTRN_AND_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_and_and_fetch_8  - BI__sync_fetch_and_add] = INTRN_AND_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_and_and_fetch_16 - BI__sync_fetch_and_add] = INTRN_AND_AND_FETCH_I4;

  SyncBuiltinTable[BI__sync_xor_and_fetch    - BI__sync_fetch_and_add] = INTRN_XOR_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_xor_and_fetch_1  - BI__sync_fetch_and_add] = INTRN_XOR_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_xor_and_fetch_2  - BI__sync_fetch_and_add] = INTRN_XOR_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_xor_and_fetch_4  - BI__sync_fetch_and_add] = INTRN_XOR_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_xor_and_fetch_8  - BI__sync_fetch_and_add] = INTRN_XOR_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_xor_and_fetch_16 - BI__sync_fetch_and_add] = INTRN_XOR_AND_FETCH_I4;

  SyncBuiltinTable[BI__sync_nand_and_fetch    - BI__sync_fetch_and_add] = INTRN_NAND_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_nand_and_fetch_1  - BI__sync_fetch_and_add] = INTRN_NAND_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_nand_and_fetch_2  - BI__sync_fetch_and_add] = INTRN_NAND_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_nand_and_fetch_4  - BI__sync_fetch_and_add] = INTRN_NAND_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_nand_and_fetch_8  - BI__sync_fetch_and_add] = INTRN_NAND_AND_FETCH_I4;
  SyncBuiltinTable[BI__sync_nand_and_fetch_16 - BI__sync_fetch_and_add] = INTRN_NAND_AND_FETCH_I4;

  SyncBuiltinTable[BI__sync_bool_compare_and_swap    - BI__sync_fetch_and_add] = INTRN_BOOL_COMPARE_AND_SWAP_I4;
  SyncBuiltinTable[BI__sync_bool_compare_and_swap_1  - BI__sync_fetch_and_add] = INTRN_BOOL_COMPARE_AND_SWAP_I4;
  SyncBuiltinTable[BI__sync_bool_compare_and_swap_2  - BI__sync_fetch_and_add] = INTRN_BOOL_COMPARE_AND_SWAP_I4;
  SyncBuiltinTable[BI__sync_bool_compare_and_swap_4  - BI__sync_fetch_and_add] = INTRN_BOOL_COMPARE_AND_SWAP_I4;
  SyncBuiltinTable[BI__sync_bool_compare_and_swap_8  - BI__sync_fetch_and_add] = INTRN_BOOL_COMPARE_AND_SWAP_I4;
  SyncBuiltinTable[BI__sync_bool_compare_and_swap_16 - BI__sync_fetch_and_add] = INTRN_BOOL_COMPARE_AND_SWAP_I4;

  SyncBuiltinTable[BI__sync_val_compare_and_swap    - BI__sync_fetch_and_add] = INTRN_VAL_COMPARE_AND_SWAP_I4;
  SyncBuiltinTable[BI__sync_val_compare_and_swap_1  - BI__sync_fetch_and_add] = INTRN_VAL_COMPARE_AND_SWAP_I4;
  SyncBuiltinTable[BI__sync_val_compare_and_swap_2  - BI__sync_fetch_and_add] = INTRN_VAL_COMPARE_AND_SWAP_I4;
  SyncBuiltinTable[BI__sync_val_compare_and_swap_4  - BI__sync_fetch_and_add] = INTRN_VAL_COMPARE_AND_SWAP_I4;
  SyncBuiltinTable[BI__sync_val_compare_and_swap_8  - BI__sync_fetch_and_add] = INTRN_VAL_COMPARE_AND_SWAP_I4;
  SyncBuiltinTable[BI__sync_val_compare_and_swap_16 - BI__sync_fetch_and_add] = INTRN_VAL_COMPARE_AND_SWAP_I4;

  SyncBuiltinTable[BI__sync_lock_test_and_set    - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4;
  SyncBuiltinTable[BI__sync_lock_test_and_set_1  - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4;
  SyncBuiltinTable[BI__sync_lock_test_and_set_2  - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4;
  SyncBuiltinTable[BI__sync_lock_test_and_set_4  - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4;
  SyncBuiltinTable[BI__sync_lock_test_and_set_8  - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4;
  SyncBuiltinTable[BI__sync_lock_test_and_set_16 - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4;

  SyncBuiltinTable[BI__sync_lock_release    - BI__sync_fetch_and_add] = INTRN_LOCK_RELEASE_I4;
  SyncBuiltinTable[BI__sync_lock_release_1  - BI__sync_fetch_and_add] = INTRN_LOCK_RELEASE_I4;
  SyncBuiltinTable[BI__sync_lock_release_2  - BI__sync_fetch_and_add] = INTRN_LOCK_RELEASE_I4;
  SyncBuiltinTable[BI__sync_lock_release_4  - BI__sync_fetch_and_add] = INTRN_LOCK_RELEASE_I4;
  SyncBuiltinTable[BI__sync_lock_release_8  - BI__sync_fetch_and_add] = INTRN_LOCK_RELEASE_I4;
  SyncBuiltinTable[BI__sync_lock_release_16 - BI__sync_fetch_and_add] = INTRN_LOCK_RELEASE_I4;

  SyncBuiltinTable[BI__sync_swap    - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4; // test_and_set?
  SyncBuiltinTable[BI__sync_swap_1  - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4;
  SyncBuiltinTable[BI__sync_swap_2  - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4;
  SyncBuiltinTable[BI__sync_swap_4  - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4;
  SyncBuiltinTable[BI__sync_swap_8  - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4;
  SyncBuiltinTable[BI__sync_swap_16 - BI__sync_fetch_and_add] = INTRN_LOCK_TEST_AND_SET_I4;

  // initialize target specfic builtin table

}

Result
WhirlExprBuilder::GenerateCall(const char* fname, TY_IDX rtype, INT parm_cnt, const Expr* const* args, BOOL retv) {
  Is_True(parm_cnt < MAX_PARM_CNT, ("too much builtin argument"));
  TY_IDX parm_ty[MAX_PARM_CNT];
  WN *ret = WN_Create(OPR_CALL, TY_mtype(rtype), MTYPE_V, parm_cnt);
  WN_Set_Linenum(ret, GetSrcPos());
  for (INT i = 0; i < parm_cnt; ++i) {
    Is_True(args[i] != NULL, ("invald parm %d", i));
    TY_IDX kid_ty = _builder->TB().ConvertType(args[i]->getType());
    WN *kid_wn = ConvertToNode(args[i]);
    Is_True(OPERATOR_is_expression(WN_operator(kid_wn)), ("not expr"));
    WN_kid(ret, i) = WGEN_CreateParm(Mtype_comparison(TY_mtype(kid_ty)),
                                     kid_wn, kid_ty);
    parm_ty[i] = kid_ty;
  }
  parm_ty[parm_cnt] = TY_IDX_ZERO;
  ST *call_st = Create_function(fname, rtype, parm_cnt, parm_ty);
  WN_st_idx(ret) = ST_st_idx(call_st);
  if (TY_mtype(rtype) != MTYPE_V && retv) {
    WN *blk = WN_CreateBlock();
    WN_INSERT_BlockLast(blk, ret);
    WN_Set_Linenum(ret, GetSrcPos());

    WN *ldid = WN_Ldid(TY_mtype(rtype), -1, Return_Val_Preg, rtype);
    ret = WGEN_CreateComma(TY_mtype(rtype), blk, ldid);
  }

  return Result::nwNode(ret, rtype);
}

Result
WhirlExprBuilder::GenerateIntrinsic(const CallExpr *expr, TY_IDX rtype, INTRINSIC iopc, BOOL is_op, BOOL retv) {
  WN* kids[MAX_PARM_CNT];
  Is_True(expr->getNumArgs() <= MAX_PARM_CNT, ("too much builtin argument"));
  INT args_num = expr->getNumArgs();
  for (INT i = 0; i < args_num; ++i) {
    const Expr* kid_expr = expr->getArg(i);
    TY_IDX kid_ty = _builder->TB().ConvertType(kid_expr->getType());
    WN* kid_wn = NULL;
    if (iopc == INTRN_VA_START && i == 1 && kid_expr->isGLValue())
      // get right parm wn for va_start
      kid_wn = ConvertExpr(kid_expr).GetRValue();
    else
      kid_wn = ConvertToNode(kid_expr);

    // make a pointer type for KIND_ARRAY
    if (TY_kind(kid_ty) == KIND_ARRAY)
      kid_ty = Make_Pointer_Type(TY_etype(kid_ty));

    kids[i] = WGEN_CreateParm(Mtype_comparison(TY_mtype(kid_ty)),
                              kid_wn, kid_ty);
  }
  // fix param 1 for BI__atomic_test_and_set
  if (0) {  // Path64: no INTRN_LOCK_TEST_AND_SET_I1
    Is_True(args_num == 2, ("wrong arg num for atomc_test_and_set"));
    kids[1] = WGEN_CreateParm(MTYPE_I4, WN_Intconst(MTYPE_I4, 1),
                              MTYPE_To_TY(MTYPE_I4));
  }
  OPERATOR opr = is_op ? OPR_INTRINSIC_OP : OPR_INTRINSIC_CALL;
  WN* intrn = WN_Create_Intrinsic(opr,
                                  Mtype_comparison(TY_mtype(rtype)), MTYPE_V,
                                  iopc, args_num, kids);
  if (!is_op)
    WN_Set_Linenum(intrn, GetSrcPos());
  if (!is_op && TY_mtype(rtype) != MTYPE_V && retv) {
    WN *blk = WN_CreateBlock();
    WN_INSERT_BlockLast(blk, intrn);
    WN_Set_Linenum(intrn, GetSrcPos());

    WN *ldid = WN_Ldid(TY_mtype(rtype), -1, Return_Val_Preg, rtype);
    WN *stid = WGEN_StidTemp(rtype, ldid, "intrn.ret");
    WN_INSERT_BlockLast(blk, stid);
    WN_Set_Linenum(stid, GetSrcPos());

    ldid = WN_Ldid(TY_mtype(rtype), WN_offset(stid), WN_st(stid), rtype);
    intrn = WGEN_CreateComma(Mtype_comparison(TY_mtype(rtype)), blk, ldid);
  }
  return Result::nwNode(intrn, 0);
}

Result
WhirlExprBuilder::EvaluateBuiltinNaN(QualType type, const Expr *arg0, bool snan) {
  TY_IDX ty = _builder->TB().ConvertType(type);
  const StringLiteral *str = dyn_cast<StringLiteral>(arg0->IgnoreParenCasts());
  if (!str) return Result::nwNone();
  const llvm::fltSemantics &sem =
    _builder->Context()->getFloatTypeSemantics(type);

  llvm::APInt fill;
  if (str->getString().empty())
    fill = llvm::APInt(32, 0);
  else if (str->getString().getAsInteger(0, fill))
    return Result::nwNone();

  llvm::APFloat result(static_cast<float>(0.0));
  if (_builder->Context()->getTargetInfo().isNan2008()) {
    if (snan)
      result = llvm::APFloat::getSNaN(sem, false, &fill);
    else
      result = llvm::APFloat::getQNaN(sem, false, &fill);
  } else {
    if (snan)
      result = llvm::APFloat::getQNaN(sem, false, &fill);
    else
      result = llvm::APFloat::getSNaN(sem, false, &fill);
  }
  TCON_IDX tcon = Convert_float_to_tcon(ty, result);
  ST *st = New_Const_Sym(tcon, ty);
  return Result::nwNode(WN_CreateConst(OPR_CONST, TY_mtype(ty), MTYPE_V, st), ty);
}

Result
WhirlExprBuilder::ConvertBuiltinExpr(const CallExpr *expr, const FunctionDecl *decl,
                                     unsigned builtin_id, BOOL retv) {
  Is_True(decl->getBuiltinID() == builtin_id, ("bad builtin expr"));

  if (builtin_id >= Builtin::FirstTSBuiltin)
    return ConvertTargetBuiltinExpr(expr, decl, builtin_id);

  TY_IDX ty = _builder->TB().ConvertType(expr->getType());
  TYPE_ID ret_mtype = TY_mtype(ty);

  // look up the table at first
  INTRINSIC iopc = BuiltinTable[builtin_id];
  if (iopc != INTRINSIC_NONE) {
    bool is_op = (INTRN_has_no_side_effects(iopc) &&
                  INTRN_return_kind(iopc) != IRETURN_V) ?
                 TRUE : FALSE;
    return GenerateIntrinsic(expr, ty, iopc, is_op, retv);
  }

  BOOL intrinsic_op = FALSE;
  BOOL intrinsic_call = FALSE;
  BOOL whirl_generated = FALSE;

  // Path64: builtins upstream maps to intrinsics that Path64 lacks, or to
  // glibc-internal functions; expanded here as WHIRL expressions.
  switch (builtin_id) {
  case Builtin::BI__builtin_bswap16:
  case Builtin::BI__builtin_bswap32:
  case Builtin::BI__builtin_bswap64:
  case Builtin::BI__builtin_isinf_sign:
  case Builtin::BI__builtin_isnormal:
  case Builtin::BI__builtin_signbit:
  case Builtin::BI__builtin_signbitf:
  case Builtin::BI__builtin_signbitl:
    {
      // evaluate the operand once, into a temporary
      const Expr *arg = expr->getArg(0);
      TY_IDX arg_ty = _builder->TB().ConvertType(arg->getType());
      TYPE_ID amt = TY_mtype(arg_ty);
      WN *blk = WN_CreateBlock();
      WN *stid = WGEN_StidTemp(arg_ty, ConvertToNode(arg), ".builtin.arg");
      WN_Set_Linenum(stid, GetSrcPos());
      WN_INSERT_BlockLast(blk, stid);
#define C2W_ARG() WN_Ldid(amt, WN_offset(stid), WN_st(stid), arg_ty)
      WN *val = NULL;
      TYPE_ID vmt = MTYPE_I4;
      if (builtin_id == Builtin::BI__builtin_bswap16 ||
          builtin_id == Builtin::BI__builtin_bswap32 ||
          builtin_id == Builtin::BI__builtin_bswap64) {
        INT bytes = TY_size(arg_ty);
        vmt = bytes == 8 ? MTYPE_U8 : MTYPE_U4;
        for (INT i = 0; i < bytes; i++) {
          // byte i moves to byte (bytes - 1 - i)
          WN *b = WN_Band(vmt, WN_Lshr(vmt, C2W_ARG(), WN_Intconst(MTYPE_I4, 8 * i)),
                          WN_Intconst(vmt, 0xff));
          b = WN_Shl(vmt, b, WN_Intconst(MTYPE_I4, 8 * (bytes - 1 - i)));
          val = val ? WN_Bior(vmt, val, b) : b;
        }
      }
      else if (builtin_id == Builtin::BI__builtin_signbit ||
               builtin_id == Builtin::BI__builtin_signbitf ||
               builtin_id == Builtin::BI__builtin_signbitl) {
        // The sign bit of the representation (this also holds for -0 and
        // NaN); converting long double to double keeps the sign.
        WN *x = C2W_ARG();
        TYPE_ID fmt = amt;
        if (fmt != MTYPE_F4 && fmt != MTYPE_F8) {
          x = WN_Cvt(fmt, MTYPE_F8, x);
          fmt = MTYPE_F8;
        }
        TYPE_ID umt = fmt == MTYPE_F4 ? MTYPE_U4 : MTYPE_U8;
        val = WN_Lshr(umt, WN_Tas(umt, MTYPE_To_TY(umt), x),
                      WN_Intconst(MTYPE_I4, MTYPE_bit_size(umt) - 1));
        if (umt == MTYPE_U8)
          val = WN_Cvt(MTYPE_U8, MTYPE_I4, val);
      }
      else {
        const llvm::fltSemantics &sem =
          _builder->Context()->getFloatTypeSemantics(arg->getType());
#define C2W_FCONST(v) \
        WN_CreateConst(OPR_CONST, amt, MTYPE_V, \
                       New_Const_Sym(Convert_float_to_tcon(arg_ty, v), arg_ty))
        if (builtin_id == Builtin::BI__builtin_isinf_sign) {
          // (x == +inf) - (x == -inf)
          val = WN_Sub(MTYPE_I4,
                       WN_EQ(amt, C2W_ARG(), C2W_FCONST(llvm::APFloat::getInf(sem, false))),
                       WN_EQ(amt, C2W_ARG(), C2W_FCONST(llvm::APFloat::getInf(sem, true))));
        } else {
          // isnormal: smallest normal <= |x| <= largest finite
          // (false for NaN, infinity, zero and subnormals)
          val = WN_Band(MTYPE_I4,
                        WN_GE(amt, WN_Abs(amt, C2W_ARG()),
                              C2W_FCONST(llvm::APFloat::getSmallestNormalized(sem))),
                        WN_LE(amt, WN_Abs(amt, C2W_ARG()),
                              C2W_FCONST(llvm::APFloat::getLargest(sem))));
        }
#undef C2W_FCONST
      }
#undef C2W_ARG
      return Result::nwNode(WGEN_CreateComma(vmt, blk, val), ty);
    }

  // Math functions as wgen gives them to the back end, as intrinsics it
  // can combine (sin and cos into sincos), simplify under -ffast-math or
  // vectorize. Those that set errno only without it (wgen's bug 14262).
  case Builtin::BIsin:   case Builtin::BI__builtin_sin:
  case Builtin::BIsinf:  case Builtin::BI__builtin_sinf:
  case Builtin::BIcos:   case Builtin::BI__builtin_cos:
  case Builtin::BIcosf:  case Builtin::BI__builtin_cosf:
  case Builtin::BIatan:  case Builtin::BI__builtin_atan:
  case Builtin::BIatanf: case Builtin::BI__builtin_atanf:
  case Builtin::BIexp:   case Builtin::BI__builtin_exp:
  case Builtin::BIexpf:  case Builtin::BI__builtin_expf:
  case Builtin::BIacos:  case Builtin::BI__builtin_acos:
  case Builtin::BIacosf: case Builtin::BI__builtin_acosf:
  case Builtin::BIasin:  case Builtin::BI__builtin_asin:
  case Builtin::BIasinf: case Builtin::BI__builtin_asinf:
  case Builtin::BIlog:   case Builtin::BI__builtin_log:
  case Builtin::BIlogf:  case Builtin::BI__builtin_logf:
  case Builtin::BIlog10: case Builtin::BI__builtin_log10:
  case Builtin::BIlog10f: case Builtin::BI__builtin_log10f:
  case Builtin::BIpow:   case Builtin::BI__builtin_pow:
    {
      bool f4 = ret_mtype == MTYPE_F4;
      if (ret_mtype != MTYPE_F4 && ret_mtype != MTYPE_F8)
        break;
      bool errno_math = _builder->Context()->getLangOpts().MathErrno;
      INTRINSIC iopc = INTRINSIC_NONE;
      switch (builtin_id) {
      case Builtin::BIsin: case Builtin::BI__builtin_sin:
      case Builtin::BIsinf: case Builtin::BI__builtin_sinf:
        iopc = f4 ? INTRN_F4SIN : INTRN_F8SIN; break;
      case Builtin::BIcos: case Builtin::BI__builtin_cos:
      case Builtin::BIcosf: case Builtin::BI__builtin_cosf:
        iopc = f4 ? INTRN_F4COS : INTRN_F8COS; break;
      case Builtin::BIatan: case Builtin::BI__builtin_atan:
      case Builtin::BIatanf: case Builtin::BI__builtin_atanf:
        iopc = f4 ? INTRN_F4ATAN : INTRN_F8ATAN; break;
      case Builtin::BIexp: case Builtin::BI__builtin_exp:
      case Builtin::BIexpf: case Builtin::BI__builtin_expf:
        iopc = f4 ? INTRN_F4EXP : INTRN_F8EXP; break;
      case Builtin::BIacos: case Builtin::BI__builtin_acos:
      case Builtin::BIacosf: case Builtin::BI__builtin_acosf:
        if (!errno_math) iopc = f4 ? INTRN_F4ACOS : INTRN_F8ACOS; break;
      case Builtin::BIasin: case Builtin::BI__builtin_asin:
      case Builtin::BIasinf: case Builtin::BI__builtin_asinf:
        if (!errno_math) iopc = f4 ? INTRN_F4ASIN : INTRN_F8ASIN; break;
      case Builtin::BIlog: case Builtin::BI__builtin_log:
      case Builtin::BIlogf: case Builtin::BI__builtin_logf:
        if (!errno_math) iopc = f4 ? INTRN_F4LOG : INTRN_F8LOG; break;
      case Builtin::BIlog10: case Builtin::BI__builtin_log10:
      case Builtin::BIlog10f: case Builtin::BI__builtin_log10f:
        if (!errno_math) iopc = f4 ? INTRN_F4LOG10 : INTRN_F8LOG10; break;
      default:   // pow (double only)
        if (!errno_math && !f4) iopc = INTRN_F8EXPEXPR; break;
      }
      if (iopc == INTRINSIC_NONE)
        break;   // the library call
      return GenerateIntrinsic(expr, ty, iopc, TRUE, TRUE);
    }

  case Builtin::BI__builtin_powi:
  case Builtin::BI__builtin_powif:
  case Builtin::BI__builtin_powil:
    {
      // x ** n for an integer n, as wgen (its bug 11286: n is 64-bit on
      // 64-bit targets)
      INTRINSIC iopc = builtin_id == Builtin::BI__builtin_powif ? INTRN_F4F4I4EXPEXPR
                     : builtin_id == Builtin::BI__builtin_powil ? INTRN_FQFQI4EXPEXPR
                                                               : INTRN_F8F8I4EXPEXPR;
      TYPE_ID imt = TARGET_64BIT ? MTYPE_I8 : MTYPE_I4;
      WN *kids[2];
      kids[0] = WGEN_CreateParm(ret_mtype, ConvertToNode(expr->getArg(0)), ty);
      kids[1] = WGEN_CreateParm(imt, WN_Type_Conversion(ConvertToNode(expr->getArg(1)), imt),
                                MTYPE_To_TY(imt));
      return Result::nwNode(WN_Create_Intrinsic(OPR_INTRINSIC_OP, ret_mtype, MTYPE_V,
                                                iopc, 2, kids), ty);
    }

  case Builtin::BIfloor: case Builtin::BI__builtin_floor:
  case Builtin::BIfloorf: case Builtin::BI__builtin_floorf:
    {
      bool f4 = builtin_id == Builtin::BIfloorf || builtin_id == Builtin::BI__builtin_floorf;
#ifdef TARG_MIPS
      return GenerateIntrinsic(expr, ty, f4 ? INTRN_FLOORF : INTRN_FLOOR, TRUE, TRUE);
#else
      return Result::nwNode(WN_CreateExp1(OPR_FLOOR, ret_mtype,
                                          f4 ? MTYPE_F4 : MTYPE_F8,
                                          ConvertToNode(expr->getArg(0))), ty);
#endif
    }

  case Builtin::BIsqrt:
  case Builtin::BIsqrtf:
  case Builtin::BI__builtin_sqrt:
  case Builtin::BI__builtin_sqrtf:
#ifndef TARG_MIPS  // MIPS long double (double-double) has no sqrt instruction
  case Builtin::BIsqrtl:
  case Builtin::BI__builtin_sqrtl:
#endif
    {
      // As wgen: the sqrt instruction. When sqrt must set errno, also
      // call the library function if the result is NaN (a negative or
      // NaN argument): r = SQRT(x); if (r != r) r = sqrt(x).
      WN *arg_wn = ConvertToNode(expr->getArg(0));
      if (!_builder->Context()->getLangOpts().MathErrno)
        return Result::nwNode(WN_CreateExp1(OPR_SQRT, ret_mtype, MTYPE_V, arg_wn), ty);

      WN *blk = WN_CreateBlock();
      WN *arg_stid = WGEN_StidTemp(ty, arg_wn, "__sqrt_arg");
      WN_Set_Linenum(arg_stid, GetSrcPos());
      WN_INSERT_BlockLast(blk, arg_stid);
#define C2W_SQRT_ARG() WN_Ldid(ret_mtype, WN_offset(arg_stid), WN_st(arg_stid), ty)
      WN *res_stid = WGEN_StidTemp(ty, WN_CreateExp1(OPR_SQRT, ret_mtype, MTYPE_V,
                                                      C2W_SQRT_ARG()),
                                   "__save_sqrt");
      WN_Set_Linenum(res_stid, GetSrcPos());
      WN_INSERT_BlockLast(blk, res_stid);
#define C2W_SQRT_RES() WN_Ldid(ret_mtype, WN_offset(res_stid), WN_st(res_stid), ty)

      WN *then_blk = WN_CreateBlock();
      llvm::StringRef name = decl->getName();
      if (name.startswith("__builtin_"))
        name = name.drop_front(sizeof("__builtin_") - 1);
      TY_IDX parm_ty[2] = { ty, TY_IDX_ZERO };
      ST *call_st = Create_function(name.str().c_str(), ty, 1, parm_ty);
      WN *call = WN_Create(OPR_CALL, ret_mtype, MTYPE_V, 1);
      WN_st_idx(call) = ST_st_idx(call_st);
      WN_kid(call, 0) = WGEN_CreateParm(ret_mtype, C2W_SQRT_ARG(), ty);
      WN_Set_Call_Default_Flags(call);
      WN_Set_Linenum(call, GetSrcPos());
      WN_INSERT_BlockLast(then_blk, call);
      WN *save = WN_Stid(ret_mtype, WN_offset(res_stid), WN_st(res_stid), ty,
                         WN_Ldid(ret_mtype, -1, Return_Val_Preg, ty));
      WN_Set_Linenum(save, GetSrcPos());
      WN_INSERT_BlockLast(then_blk, save);

      WN *if_wn = WN_CreateIf(WN_NE(ret_mtype, C2W_SQRT_RES(), C2W_SQRT_RES()),
                              then_blk, WN_CreateBlock());
      WN_Set_Linenum(if_wn, GetSrcPos());
      WN_INSERT_BlockLast(blk, if_wn);
      WN *val = C2W_SQRT_RES();
#undef C2W_SQRT_ARG
#undef C2W_SQRT_RES
      return Result::nwNode(WGEN_CreateComma(ret_mtype, blk, val), ty);
    }

  case Builtin::BI__builtin_object_size:
    {
      // Clang folds this when it can; otherwise the documented "don't
      // know" answers: (size_t)-1 for types 0 and 1, 0 for 2 and 3.
      llvm::APSInt v;
      INT64 size;
      if (expr->EvaluateAsInt(v, *_builder->Context()))
        size = v.getSExtValue();
      else {
        llvm::APSInt kind =
          expr->getArg(1)->EvaluateKnownConstInt(*_builder->Context());
        size = (kind.getZExtValue() & 2) ? 0 : -1;
      }
      return Result::nwNode(WN_Intconst(Mtype_comparison(TY_mtype(ty)), size), ty);
    }

  default:
    break;
  }

  // check clang/Basic/Builtins.def for all builtins
  switch (builtin_id) {
  // here we uses GNU extionsion for case range
#if LLVM_VERSION_MAJOR >= 7
  case Builtin::BI__builtin_huge_val ... Builtin::BI__builtin_inff128:
#else
  case Builtin::BI__builtin_huge_val ... Builtin::BI__builtin_infl:
#endif
    {
      const llvm::fltSemantics &sem =
        _builder->Context()->getFloatTypeSemantics(expr->getType());
      llvm::APFloat val = llvm::APFloat::getInf(sem);
      TCON_IDX tcon = Convert_float_to_tcon(ty, val);
      ST *st = New_Const_Sym(tcon, ty);
      return Result::nwNode(WN_CreateConst(OPR_CONST, TY_mtype(ty), MTYPE_V, st), ty);
    }

  case Builtin::BI__builtin_labs ... Builtin::BI__builtin_llabs:
    {
      WN* arg0 = ConvertToNode(expr->getArg(0));
      return Result::nwNode(Handle_cond_wn(arg0), ty);
    }

  case Builtin::BI__builtin_atan2 ... Builtin::BI__builtin_frexpl:  // libc/libm
  case Builtin::BI__builtin_ldexp ... Builtin::BI__builtin_modfl:

#if LLVM_VERSION_MAJOR >= 7
  case Builtin::BI__builtin_nan ... Builtin::BI__builtin_nanf128:
#else
  case Builtin::BI__builtin_nan ... Builtin::BI__builtin_nanl:
#endif
    return EvaluateBuiltinNaN(expr->getType(), expr->getArg(0), false);

#if LLVM_VERSION_MAJOR >= 7
  case Builtin::BI__builtin_nans ... Builtin::BI__builtin_nansf128:
#else
  case Builtin::BI__builtin_nans ... Builtin::BI__builtin_nansl:
#endif
    return EvaluateBuiltinNaN(expr->getType(), expr->getArg(0), true);

  case Builtin::BI__builtin_acos ... Builtin::BI__builtin_truncl: // libc/libm float/double/long variants
  case Builtin::BI__builtin_cabs ... Builtin::BI__builtin_ctanhl: // complex
  case Builtin::BI__builtin_clzs ... Builtin::BI__builtin_popcountll: // arithmetic
    return Result::nwNone();


  // Unary FP classification
  case Builtin::BI__builtin_fpclassify:
    {
      // convert to call to __fpclassify
      Is_True(expr->getNumArgs() == 6, ("invalid arg num"));
      const Expr *arg = expr->getArg(5);
      TY_IDX arg_ty = _builder->TB().ConvertType(arg->getType());
      TY_IDX ret_ty = _builder->TB().ConvertType(expr->getType());
      const char *fname = NULL;
      switch (TY_mtype(arg_ty)) {
      case MTYPE_F4:  fname = "__fpclassifyf";  break;
      case MTYPE_F8:  fname = "__fpclassify";   break;
      case MTYPE_FQ: fname = "__fpclassifyl";  break;
      default:
        Is_True(FALSE, ("unsupported mtype %d", TY_mtype(arg_ty)));
      }
      const Expr *parm[1] = { arg };
      return GenerateCall(fname, ret_ty, 1, parm, retv);
    }

  case Builtin::BI__builtin_isfinite:
    {
      Is_True(expr->getNumArgs() == 1, ("invalid arg num"));
      const Expr *arg = expr->getArg(0);
      TY_IDX arg_ty = _builder->TB().ConvertType(arg->getType());
      TY_IDX ret_ty = _builder->TB().ConvertType(expr->getType());
      const char *fname = NULL;
      switch (TY_mtype(arg_ty)) {
      case MTYPE_F4:  fname = "__finitef";  break;
      case MTYPE_F8:  fname = "__finite";   break;
      case MTYPE_FQ: fname = "__finitel";  break;
      default:
        Is_True(FALSE, ("unsupported mtype %d", TY_mtype(arg_ty)));
      }
      const Expr *parm[1] = { arg };
      return GenerateCall(fname, ret_ty, 1, parm, retv);
    }

  case Builtin::BI__builtin_isinf:
    {
      Is_True(expr->getNumArgs() == 1, ("invalid arg num"));
      const Expr *arg = expr->getArg(0);
      TY_IDX arg_ty = _builder->TB().ConvertType(arg->getType());
      TY_IDX ret_ty = _builder->TB().ConvertType(expr->getType());
      const char *fname = NULL;
      switch (TY_mtype(arg_ty)) {
      case MTYPE_F4:  fname = "__isinff";  break;
      case MTYPE_F8:  fname = "__isinf";   break;
      case MTYPE_FQ: fname = "__isinfl";  break;
      default:
        Is_True(FALSE, ("unsupported mtype %d", TY_mtype(arg_ty)));
      }
      const Expr *parm[1] = { arg };
      return GenerateCall(fname, ret_ty, 1, parm, retv);
    }

  case Builtin::BI__builtin_isnan:
    {
      Is_True(expr->getNumArgs() == 1, ("invalid arg num"));
      const Expr *arg = expr->getArg(0);
      TY_IDX arg_ty = _builder->TB().ConvertType(arg->getType());
      TY_IDX ret_ty = _builder->TB().ConvertType(expr->getType());
      const char *fname = NULL;
      switch (TY_mtype(arg_ty)) {
      case MTYPE_F4:  fname = "__isnanf";  break;
      case MTYPE_F8:  fname = "__isnan";   break;
      case MTYPE_FQ: fname = "__isnanl";  break;
      default:
        Is_True(FALSE, ("unsupported mtype %d", TY_mtype(arg_ty)));
      }
      const Expr *parm[1] = { arg };
      return GenerateCall(fname, ret_ty, 1, parm, retv);
    }

  // FP signbit builtins
  // Special FP builtins
  case Builtin::BI__builtin_canonicalize ... Builtin::BI__builtin_canonicalizel:
    return Result::nwNone();

  case Builtin::BI__builtin_bitreverse8 ... Builtin::BI__builtin_bitreverse64: // bitreverse
    return Result::nwNone();

  case Builtin::BI__builtin_constant_p:
    // TODO: generate INTCONST 1 if arg 0 is really constant
    iopc = INTRN_CONSTANT_P;
    intrinsic_op = TRUE;
    if (ret_mtype == MTYPE_V) ret_mtype = MTYPE_I4;
    break;

  case Builtin::BI__builtin_classify_type:
    {
      Expr::EvalResult Result;
      expr->EvaluateAsRValue(Result, *(_builder->Context()));
      WN *wn = WN_Intconst(Mtype_comparison(TY_mtype(ty)),
                           Result.Val.getInt().getZExtValue());
      whirl_generated = TRUE;
      return Result::nwNode(wn, ty);
    }

  case Builtin::BI__builtin_va_start:
    Is_True(expr->getNumArgs() == 2, ("wrong arg num for va_start"));
    if (TARGET_64BIT) {
      iopc = INTRN_VA_START;
      intrinsic_call = TRUE;
    }
    else {
      WN *arg0 = ConvertToNode(expr->getArg(0));
      WN *arg1 = ConvertToNode(expr->getArg(1));
      TY_IDX arg0_ty = _builder->TB().ConvertType(expr->getArg(0)->getType());
      WN *wn = WN_Istore(Pointer_Mtype, 0, arg0_ty, arg1, arg0, 0);
      return Result::nwNode(wn, ty);
    }
    break;

  case Builtin::BI__builtin_va_end:
    {
      WN* arg0 = ConvertToNode(expr->getArg(0));
      WN* wn = WN_CreateEval(arg0);
      WN_Set_Linenum(wn, GetSrcPos());
      return Result::nwNode(wn, ty);
    }

  case Builtin::BI__builtin_va_copy:
    {
      Is_True(expr->getNumArgs() == 2, ("wrong arg num for va_start"));
      WN *arg0 = ConvertToNode(expr->getArg(0));
      WN *arg1 = ConvertToNode(expr->getArg(1));
      TY_IDX ty_idx = _builder->TB().ConvertType(expr->getArg(0)->getType());
      if (TARGET_64BIT) {
        WN* dst = WGEN_CreateParm(Pointer_Mtype, arg0, ty_idx);
        WN* src = WGEN_CreateParm(Pointer_Mtype, arg1, ty_idx);
        WN* size = WGEN_CreateParm(MTYPE_I4,
                                   WN_Intconst(MTYPE_I4, TY_size(TY_pointed(ty_idx))),
                                   MTYPE_To_TY(MTYPE_I4));
        WN* wn = WN_Create(OPR_INTRINSIC_CALL, ret_mtype, MTYPE_V, 3);
        WN_Set_Linenum(wn, GetSrcPos());
        WN_intrinsic(wn) = INTRN_MEMCPY;
        WN_kid0(wn) = dst;
        WN_kid1(wn) = src;
        WN_kid2(wn) = size;
        return Result::nwNode(wn, ty);
      }
      else {
        WN *wn = WN_Istore(Pointer_Mtype, 0, ty_idx, arg1, arg0, 0);
        return Result::nwNode(wn, ty);
      }
    }

  case Builtin::BI__builtin_stdarg_start:
  case Builtin::BI__builtin_assume_aligned:
    Is_True(false, ("unsupported builtin: %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__builtin_bcmp ... Builtin::BI__builtin_wmemcmp:
    return Result::nwNone();

  case Builtin::BI__builtin_return_address:
  case Builtin::BI__builtin_extract_return_addr:
  case Builtin::BI__builtin_frame_address:
  case Builtin::BI__builtin___clear_cache:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__builtin_flt_rounds:
    return Result::nwNode(WN_Intconst(MTYPE_I4, 1), MTYPE_To_TY(MTYPE_I4));

  case Builtin::BI__builtin_setjmp:
  case Builtin::BI__builtin_longjmp:
    return Result::nwNone();

  case Builtin::BI__builtin_unwind_init:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__builtin_snprintf:
  case Builtin::BI__builtin_vsprintf:
  case Builtin::BI__builtin_vsnprintf:
    return Result::nwNone();

  case Builtin::BI__builtin_thread_pointer:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__builtin_eh_return:
  case Builtin::BI__builtin_frob_return_addr:
  case Builtin::BI__builtin_dwarf_cfa:
  case Builtin::BI__builtin_init_dwarf_reg_size_table:
  case Builtin::BI__builtin_dwarf_sp_column:
  case Builtin::BI__builtin_extend_pointer:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__builtin___memcpy_chk ... Builtin::BI__builtin___vprintf_chk: // object size checking
    return Result::nwNone();

  case Builtin::BI__builtin_unpredictable:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__builtin_expect:
    Is_True(expr->getNumArgs() == 2, ("wrong arg num for expect"));
    iopc = INTRN_EXPECT;
    intrinsic_op = TRUE;
    if (ret_mtype == MTYPE_V) ret_mtype = MTYPE_I4;
    break;

  case Builtin::BIalloca:
  case Builtin::BI__builtin_alloca:
    {
      WN *arg = ConvertToNode(expr->getArg(0));
      Is_True(arg != NULL && OPERATOR_is_expression(WN_operator(arg)),
              ("wrong alloca arg"));
      Is_True(MTYPE_is_integral(WN_rtype(arg)),
              ("alloca arg is not integer"));
      WN *wn = WN_CreateAlloca(arg);
      Set_PU_has_alloca(Get_Current_PU());
      return Result::nwNode(wn, ty);
    }
    break;

  case Builtin::BI__builtin_trap:
  case Builtin::BI__builtin_debugtrap:
  case Builtin::BI__builtin_unreachable:
    iopc = (ret_mtype == MTYPE_V || !retv) ? INTRN_VEXIT : INTRN_I4EXIT;
    intrinsic_call = TRUE;
    break;

  case Builtin::BI__builtin_prefetch:
    {
      WN *addr = ConvertToNode(expr->getArg(0));
      Is_True(addr && OPERATOR_is_expression(WN_operator(addr)),
              ("wrong prefetch address"));
      // TODO: handle prefetch flags
      WN *wn = WN_CreatePrefetch(0, 0, addr);
      WN_Set_Linenum(wn, GetSrcPos());
      return Result::nwNode(wn, 0);
    }
  case Builtin::BI__builtin_readcyclecounter:
  case Builtin::BI__builtin_shufflevector:
  case Builtin::BI__builtin_convertvector:
  case Builtin::BI__builtin_alloca_with_align:
  case Builtin::BI__builtin_call_with_static_chain:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__sync_fetch_and_add ... Builtin::BI__sync_swap_16: // old _sync builtin
    if (builtin_id >= Builtin::BI__sync_lock_release &&
        builtin_id <= Builtin::BI__sync_lock_release_16) {
      Is_True(expr->getNumArgs() == 1, ("wrong arg num for sync_lock_release"));
    }
    else if (builtin_id >= Builtin::BI__sync_bool_compare_and_swap &&
             builtin_id <= Builtin::BI__sync_val_compare_and_swap_16) {
      Is_True(expr->getNumArgs() == 3, ("wrong arg num for sync_compare_and_swap"));
    }
    else {
      Is_True(expr->getNumArgs() == 2, ("wrong arg num for sync function"));
    }
    {
      TY_IDX ptr_ty = _builder->TB().ConvertType(expr->getArg(0)->getType());
      Is_True(TY_kind(ptr_ty) == KIND_POINTER, ("not pointer type"));
      // Path64 has 4- and 8-byte forms; the table holds the 4-byte one.
      iopc = Sync_Intrinsic_For_Size(
               SyncBuiltinTable[builtin_id - Builtin::BI__sync_fetch_and_add],
               TY_size(TY_pointed(ptr_ty)));
      intrinsic_call = TRUE;
    }
    break;

  case Builtin::BI__c11_atomic_init:
  case Builtin::BI__c11_atomic_load:
  case Builtin::BI__c11_atomic_store:
  case Builtin::BI__c11_atomic_exchange:
  case Builtin::BI__c11_atomic_compare_exchange_strong:
  case Builtin::BI__c11_atomic_compare_exchange_weak:
  case Builtin::BI__c11_atomic_fetch_add:
  case Builtin::BI__c11_atomic_fetch_sub:
  case Builtin::BI__c11_atomic_fetch_and:
  case Builtin::BI__c11_atomic_fetch_or:
  case Builtin::BI__c11_atomic_fetch_xor:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__atomic_load ... Builtin::BI__atomic_nand_fetch: // atomic builtin
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__atomic_test_and_set:
    // generate INTRN_LOCK_TEST_AND_SET_I1
    Is_True(expr->getNumArgs() == 2, ("wrong arg num for atomic_clear"));
    {
      TY_IDX ptr_ty = _builder->TB().ConvertType(expr->getArg(0)->getType());
      Is_True(TY_kind(ptr_ty) == KIND_POINTER, ("not pointer type"));
      iopc = Sync_Intrinsic_For_Size(INTRN_LOCK_TEST_AND_SET_I4,
                                     TY_size(TY_pointed(ptr_ty)));
      intrinsic_call = TRUE;
    }
    break;

  case Builtin::BI__atomic_clear:
    // generate INTRN_LOCK_RELEASE_I1
    Is_True(expr->getNumArgs() == 2, ("wrong arg num for atomic_clear"));
    {
      TY_IDX ptr_ty = _builder->TB().ConvertType(expr->getArg(0)->getType());
      Is_True(TY_kind(ptr_ty) == KIND_POINTER, ("not pointer type"));
      iopc = Sync_Intrinsic_For_Size(INTRN_LOCK_RELEASE_I4,
                                     TY_size(TY_pointed(ptr_ty)));
      intrinsic_call = TRUE;
    }
    break;

  case Builtin::BI__c11_atomic_thread_fence:
  case Builtin::BI__atomic_thread_fence:
    // INTRN_THREAD_FENCE
    Is_True(expr->getNumArgs() == 1, ("wrong arg num for thread_fence"));
    // Path64: no ordered fences; use the full barrier.
    iopc = INTRN_SYNCHRONIZE;
    intrinsic_call = TRUE;
    break;

  case Builtin::BI__c11_atomic_signal_fence:
  case Builtin::BI__atomic_signal_fence:
    // INTRN_SIGNAL_FENCE
    Is_True(expr->getNumArgs() == 1, ("wrong arg num for thread_fence"));
    // Path64: a signal fence only orders the compiler; use the full barrier.
    iopc = INTRN_SYNCHRONIZE;
    intrinsic_call = TRUE;
    break;

  case Builtin::BI__atomic_always_lock_free:
    // ALWAYS_LOCK_FREE
    Is_True(expr->getNumArgs() >= 1, ("wrong arg num for always_lock_free"));
    ErrMsg(EC_Unimplemented_Feature, "__atomic_always_lock_free");
    return Result::nwNone();
    break;

  case Builtin::BI__c11_atomic_is_lock_free:  // c11 atomic builtin
  case Builtin::BI__atomic_is_lock_free:
    // ALWAYS_LOCK_FREE
    Is_True(expr->getNumArgs() >= 1, ("wrong arg num for lock_free"));
    ErrMsg(EC_Unimplemented_Feature, "__atomic_is_lock_free");
    return Result::nwNone();
    break;

#if LLVM_VERSION_MAJOR >= 7
  case Builtin::BI__opencl_atomic_init ... Builtin::BI__opencl_atomic_fetch_max: // opencl builtin
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__atomic_fetch_min:
  case Builtin::BI__atomic_fetch_max:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();
#endif

  case Builtin::BI__sync_synchronize:
  case Builtin::BI__sync_fetch_and_min:
  case Builtin::BI__sync_fetch_and_max:
  case Builtin::BI__sync_fetch_and_umin:
  case Builtin::BI__sync_fetch_and_umax:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__builtin_abort:
  case Builtin::BI__builtin_index:
  case Builtin::BI__builtin_rindex:
    // return none to generate call to lib functions
    return Result::nwNone();

  // ignore MS builtins

#if LLVM_VERSION_MAJOR >= 7
  case Builtin::BIva_start:                         // C99 atdarg.h
  case Builtin::BIva_end:
  case Builtin::BIva_copy:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();
#endif

  // generate nornal CALL for below intrinsics
#if LLVM_VERSION_MAJOR >= 7
  case Builtin::BIabort ... Builtin::BIstrtoull:    // C99 stdlib.h
#else
  case Builtin::BIabort ... Builtin::BIrealloc:     // C99 stdlib.h
#endif
  case Builtin::BImemcpy ... Builtin::BIstrlen:     // C99 string.h
#if LLVM_VERSION_MAJOR >= 7
  case Builtin::BIprintf ... Builtin::BIfwrite:     // C99 stdio.h
#else
  case Builtin::BIprintf ... Builtin::BIvsscanf:    // C99 stdio.h
#endif
  case Builtin::BIisalnum ... Builtin::BItoupper:   // C99 ctype.h
  case Builtin::BIwcschr ... Builtin::BIwmemcmp:    // C99 wchar.h
  case Builtin::BIsetjmp ... Builtin::BIlongjmp:    // C99 sjlj
  case Builtin::BIstpcpy ... Builtin::BIstrndup:    // POSIX string.h
  case Builtin::BIindex ... Builtin::BIstrncasecmp: // POSIX strings.h
  case Builtin::BI_exit ...  Builtin::BIvfork:      // POSIX unistd.h
  case Builtin::BI_setjmp ... Builtin::BIsiglongjmp:// POSIX setjmp.h
  case Builtin::BIstrlcpy ... Builtin::BIstrlcat:   // non-std
    return Result::nwNone();

  // ignore ObjC builtins

  case Builtin::BIfabs:
  case Builtin::BIfabsf:
  case Builtin::BIfabsl:
    {
      WN *arg0 =  ConvertToNode(expr->getArg(0));
      return Result::nwNode(WN_Unary(OPR_ABS, ret_mtype, arg0), ty);
    }

  case Builtin::BIatan2 ... Builtin::BIcopysignl:    // builtin math
  case Builtin::BIfinite ... Builtin::BI__exp10f:    // builtin math
    return Result::nwNone();

  case Builtin::BI_Block_object_assign:
  case Builtin::BI_Block_object_dispose:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__builtin_annotation:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  case Builtin::BI__builtin_assume:
    {
      WN *arg0 =  ConvertToNode(expr->getArg(0));
      WN* wn = WN_CreateAffirm(arg0);
      WN_Set_Linenum(wn, GetSrcPos());
      return Result::nwNode(wn, ty);
    }

  case Builtin::BI__builtin_addcb ... Builtin::BI__builtin_subcll: // multiprecision arithmetic
  case Builtin::BI__builtin_add_overflow ... Builtin::BI__builtin_smulll_overflow: // checked arithmetic
    //Is_True(false,
    //        ("unsupported builtin %d\n for checked arithmetic", builtin_id));
    return Result::nwNone();

  // clang builtin (not available in GCC)
  case Builtin::BI__builtin_addressof:
    Is_True(expr->getNumArgs() == 1, ("invalid arg num"));
    {
      Result ret = ConvertExpr(expr->getArg(0));
      Is_True(ret.isSym(), ("not symbol"));
      return Result::nwNode(ret.GetLValue(), ret.Ty());
    }

  case Builtin::BI__builtin_operator_new:
    return EmitCXXNewDeleteCall(expr, false);
  case Builtin::BI__builtin_operator_delete:
    return EmitCXXNewDeleteCall(expr, true);
  case Builtin::BI__builtin_char_memchr:
    // convert to memchr
    Is_True(expr->getNumArgs() == 3, ("invalid arg num"));
    return GenerateCall("memchr", ty, 3, expr->getArgs(), retv);

#if LLVM_VERSION_MAJOR >= 7
  case Builtin::BI__builtin_dump_struct:
#else
  case Builtin::NotBuiltin:
#endif
    Is_True(false,
            ("unsupported builtin %d\n for clang builtin", builtin_id));

   case Builtin::BI__builtin___get_unsafe_stack_start ... Builtin::BI__builtin___get_unsafe_stack_ptr:  // safestack
    Is_True(false,
            ("unsupported builtin %d\n for safestack", builtin_id));

  case Builtin::BI__builtin_nontemporal_store:
    {
      // a plain store; the non-temporal hint is dropped
      TY_IDX vty = _builder->TB().ConvertType(expr->getArg(0)->getType());
      WN *val = ConvertToNode(expr->getArg(0));
      WN *ptr = ConvertToNode(expr->getArg(1));
      WN *st = WN_Istore(TY_mtype(vty), 0, Make_Pointer_Type(vty), ptr, val);
      WN_Set_Linenum(st, GetSrcPos());
      return Result::nwNode(st, 0);   // a statement, as for intrinsic calls
    }
  case Builtin::BI__builtin_nontemporal_load:
    {
      WN *ptr = ConvertToNode(expr->getArg(0));
      return Result::nwNode(WN_Iload(ret_mtype, 0, ty, ptr), ty);
    }

  case Builtin::BI__builtin_coro_resume ... Builtin::BI__builtin_coro_param:  // coroutine
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

#if LLVM_VERSION_MAJOR >= 7
  case Builtin::BIread_pipe ... Builtin::BI__builtin_load_halff:  // opencl
#else
  case Builtin::BIread_pipe ... Builtin::BIget_kernel_preferred_work_group_size_multiple:  // opencl
#endif
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

#if LLVM_VERSION_MAJOR >= 7
  case Builtin::BI__builtin_os_log_format_buffer_size:
  case Builtin::BI__builtin_os_log_format:
#endif
#if LLVM_VERSION_MAJOR >= 7
  case Builtin::BIomp_is_initial_device:
#endif
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();

  // ignore win64 va_start/va_end/va_copy

#if LLVM_VERSION_MAJOR == 11
  case Builtin::BIwmemcpy:
  case Builtin::BIwmemmove:
  case Builtin::BImemccpy:
  case Builtin::BImempcpy:
  case Builtin::BIpthread_create:
    return Result::nwNone();
#endif

  default:
    Is_True(false, ("unsupported builtin %d\n", builtin_id));
    return Result::nwNone();
  }

  if (!retv && intrinsic_call)
    ty = MTYPE_To_TY(MTYPE_V);  // result not used, set return type to void

  if (intrinsic_op || intrinsic_call) {
    return GenerateIntrinsic(expr, ty, iopc, intrinsic_op, retv);
  }

  return Result::nwNone();
}

Result
WhirlExprBuilder::ConvertTargetBuiltinExpr(const CallExpr *expr, const FunctionDecl *decl, unsigned builtin_id) {
  Is_True(decl->getBuiltinID() == builtin_id, ("bad builtin expr"));
#ifdef TARG_X8664
  // x86 builtins by name (__builtin_ia32_*), as wgen's WGEN_target_builtins
  // handles GCC's: most become the same Path64 intrinsic, a few WHIRL.
  static std::map<std::string, INTRINSIC> table;
  if (table.empty()) {
#define C2W_X86_BUILTIN(name, intrn) table[#name] = intrn;
#include "c2w_x86_builtins.def"
#undef C2W_X86_BUILTIN
    // wgen gets these two wrong (a missing break), so they are here
    table["paddusw128"] = INTRN_PADDUSW128;
    table["paddsb128"] = INTRN_PADDSB128;
  }
  StringRef full = _builder->Context()->BuiltinInfo.getName(builtin_id);
  std::string name;
  if (full.startswith("__builtin_ia32_"))
    name = full.drop_front(sizeof("__builtin_ia32_") - 1).str();
  else if (full == "_mm_prefetch") {
    WN *wn = WN_CreatePrefetch(0, 0, ConvertToNode(expr->getArg(0)));
    WN_Set_Linenum(wn, GetSrcPos());
    return Result::nwNode(wn, 0);
  }
  else if (full == "_mm_getcsr")
    name = "stmxcsr";   // GCC's builtin names for these
  else if (full == "_mm_setcsr")
    name = "ldmxcsr";
  else if (full.startswith("_mm_"))
    name = full.drop_front(sizeof("_mm_") - 1).str();   // _mm_sfence, ...
  else {
    ErrMsg(EC_Unimplemented_Feature, full.str().c_str());
    return Result::nwNone();
  }
  TY_IDX ty = _builder->TB().ConvertType(expr->getType());
  TYPE_ID rmt = TY_mtype(ty);
  unsigned nargs = expr->getNumArgs();
#define C2W_ARG(i) ConvertToNode(expr->getArg(i))

  // void intrinsics with side effects (wgen: *intrinsic_op = FALSE)
  static const struct { const char *name; INTRINSIC iopc; } calls[] = {
    { "emms", INTRN_EMMS }, { "storehps", INTRN_STOREHPS },
    { "storelps", INTRN_STORELPS }, { "maskmovdqu", INTRN_MASKMOVDQU },
    { "maskmovq", INTRN_MASKMOVQ }, { "movnti", INTRN_MOVNTI },
    { "movntq", INTRN_MOVNTQ }, { "sfence", INTRN_SFENCE },
    { "lfence", INTRN_LFENCE }, { "mfence", INTRN_MFENCE },
    { "pause", INTRN_PAUSE }, { "clflush", INTRN_CLFLUSH },
    { "stmxcsr", INTRN_STMXCSR }, { "ldmxcsr", INTRN_LDMXCSR },
    { NULL, INTRINSIC_NONE } };
  for (int k = 0; calls[k].name; k++)
    if (name == calls[k].name)
      return GenerateIntrinsic(expr, ty, calls[k].iopc, FALSE, rmt != MTYPE_V);

  // WHIRL operators (wgen does the same)
  if (name == "paddb" || name == "paddw" || name == "paddd" || name == "paddq")
    return Result::nwNode(WN_Add(rmt, C2W_ARG(0), C2W_ARG(1)), ty);
  if (name == "psubb" || name == "psubw" || name == "psubd" || name == "psubq")
    return Result::nwNode(WN_Sub(rmt, C2W_ARG(0), C2W_ARG(1)), ty);
  if (name == "sqrtpd" || name == "sqrtps")
    return Result::nwNode(WN_Sqrt(rmt, C2W_ARG(0)), ty);
  if (name == "minpd")
    return Result::nwNode(WN_Binary(OPR_MIN, rmt, C2W_ARG(0), C2W_ARG(1)), ty);
  if (name == "maxpd")
    return Result::nwNode(WN_Binary(OPR_MAX, rmt, C2W_ARG(0), C2W_ARG(1)), ty);
  if (name == "vec_init_v2si" || name == "vec_init_v4hi" ||
      name == "vec_init_v8qi") {
    // the elements packed into 64 bits, first one lowest
    unsigned bits = 64 / nargs;
    WN *wn = NULL;
    for (int k = nargs - 1; k >= 0; k--) {
      WN *e = WN_CreateCvtl(OPC_U8CVTL, bits,
                            WN_Type_Conversion(C2W_ARG(k), MTYPE_U8));
      wn = wn ? WN_Bior(MTYPE_U8, WN_Shl(MTYPE_U8, wn, WN_Intconst(MTYPE_U8, bits)), e)
              : e;
    }
    return Result::nwNode(WN_Tas(rmt, ty, wn), ty);
  }
  if (name == "undef128") {
    // any value will do: an uninitialized temporary
    ST *st = Gen_Temp_Symbol(ty, ".undef");
    return Result::nwNode(WN_Ldid(rmt, 0, st, ty), ty);
  }
  if (name == "movnti64") {
    // a non-temporal store; the hint is dropped
    WN *val = C2W_ARG(1);
    WN *ptr = C2W_ARG(0);
    TY_IDX vty = MTYPE_To_TY(MTYPE_I8);
    WN *st = WN_Istore(MTYPE_I8, 0, Make_Pointer_Type(vty), ptr, val);
    WN_Set_Linenum(st, GetSrcPos());
    return Result::nwNode(st, 0);   // a statement, as for intrinsic calls
  }
  // MMX shifts by an immediate count (Clang's names; GCC 4.2 had only the
  // forms taking the count in a 64-bit operand, the same intrinsics)
  static const struct { const char *name; INTRINSIC iopc; } mmx_shifts[] = {
    { "psllwi", INTRN_PSLLW_MMX }, { "pslldi", INTRN_PSLLD_MMX },
    { "psrlwi", INTRN_PSRLW_MMX }, { "psrldi", INTRN_PSRLD_MMX },
    { "psrawi", INTRN_PSRAW_MMX }, { "psradi", INTRN_PSRAD_MMX },
    { NULL, INTRINSIC_NONE } };
  for (int k = 0; mmx_shifts[k].name; k++)
    if (name == mmx_shifts[k].name) {
      WN *kids[2];
      TY_IDX aty = _builder->TB().ConvertType(expr->getArg(0)->getType());
      kids[0] = WGEN_CreateParm(TY_mtype(aty), C2W_ARG(0), aty);
      kids[1] = WGEN_CreateParm(MTYPE_I8,
                                WN_Type_Conversion(C2W_ARG(1), MTYPE_I8),
                                MTYPE_To_TY(MTYPE_I8));
      return Result::nwNode(WN_Create_Intrinsic(OPR_INTRINSIC_OP, rmt, MTYPE_V,
                                                mmx_shifts[k].iopc, 2, kids), ty);
    }
  if (name == "psllqi" || name == "psrlqi") {
    // 64-bit MMX shift: the value is one 64-bit lane
    WN *v = WN_Tas(MTYPE_U8, MTYPE_To_TY(MTYPE_U8), C2W_ARG(0));
    WN *cnt = WN_Type_Conversion(C2W_ARG(1), MTYPE_U8);
    WN *r = name == "psllqi" ? WN_Shl(MTYPE_U8, v, cnt) : WN_Lshr(MTYPE_U8, v, cnt);
    return Result::nwNode(WN_Tas(rmt, ty, r), ty);
  }
  if (name == "movntdqa") {
    // a non-temporal load: an ordinary load (the back end can't expand
    // INTRN_MOVNTDQA)
    WN *ptr = ConvertToNode(expr->getArg(0));
    return Result::nwNode(WN_Iload(rmt, 0, ty, ptr), ty);
  }
  if (name == "pmaddwd128") {
    // r[i] = a[2i]*b[2i] + a[2i+1]*b[2i+1], 16-bit products summed in 32
    // bits. The back end has only the MMX pmaddwd (wgen uses it here too,
    // which leaves the upper half wrong), so element by element.
    TY_IDX vty = _builder->TB().ConvertType(expr->getArg(0)->getType());
    TY_IDX hty = MTYPE_To_TY(MTYPE_I2);
    TY_IDX wty = MTYPE_To_TY(MTYPE_I4);
    WN *blk = WN_CreateBlock();
    SRCPOS pos = GetSrcPos();
    ST *sa = C2W_Vector_Temp(blk, vty, ConvertToNode(expr->getArg(0)), pos);
    ST *sb = C2W_Vector_Temp(blk, vty, ConvertToNode(expr->getArg(1)), pos);
    ST *res = C2W_Vector_Temp(blk, ty, NULL, pos);
    for (int k = 0; k < 4; k++) {
      WN *lo = WN_Mpy(MTYPE_I4, C2W_Vector_Elt(sa, hty, 2 * k),
                      C2W_Vector_Elt(sb, hty, 2 * k));
      WN *hi = WN_Mpy(MTYPE_I4, C2W_Vector_Elt(sa, hty, 2 * k + 1),
                      C2W_Vector_Elt(sb, hty, 2 * k + 1));
      C2W_Vector_Set_Elt(blk, res, wty, k, WN_Add(MTYPE_I4, lo, hi), pos);
    }
    return Result::nwNode(WGEN_CreateComma(rmt, blk, WN_Ldid(rmt, 0, res, ty)), ty);
  }
  if (name.compare(0, 5, "psign") == 0) {
    // SSSE3 psign: each a[i] negated where b[i] < 0, kept where > 0,
    // zero where 0 (Path64 has no intrinsic for it)
    QualType aqt = expr->getArg(0)->getType();
    const VectorType *vt = aqt->getAs<VectorType>();
    TY_IDX vty = _builder->TB().ConvertType(aqt);
    TY_IDX ety = _builder->TB().ConvertType(vt->getElementType());
    TYPE_ID emt = TY_mtype(ety), cmt = Mtype_comparison(emt);
    WN *blk = WN_CreateBlock();
    SRCPOS pos = GetSrcPos();
    ST *sa = C2W_Vector_Temp(blk, vty, C2W_ARG(0), pos);
    ST *sb = C2W_Vector_Temp(blk, vty, C2W_ARG(1), pos);
    ST *res = C2W_Vector_Temp(blk, vty, NULL, pos);
    for (unsigned k = 0; k < vt->getNumElements(); k++) {
      WN *bk = C2W_Vector_Elt(sb, ety, k);
      WN *ak = C2W_Vector_Elt(sa, ety, k);
      WN *neg = WN_Neg(cmt, C2W_Vector_Elt(sa, ety, k));
      WN *v = WN_Select(cmt, WN_LT(cmt, bk, WN_Intconst(cmt, 0)), neg,
                        WN_Select(cmt, WN_EQ(cmt, C2W_Vector_Elt(sb, ety, k),
                                             WN_Intconst(cmt, 0)),
                                  WN_Intconst(cmt, 0), ak));
      C2W_Vector_Set_Elt(blk, res, ety, k, v, pos);
    }
    WN *val = WN_Ldid(TY_mtype(vty), 0, res, vty);
    return Result::nwNode(WGEN_CreateComma(TY_mtype(vty), blk, val), vty);
  }
#undef C2W_ARG

  std::map<std::string, INTRINSIC>::iterator it = table.find(name);
  if (it == table.end()) {
    ErrMsg(EC_Unimplemented_Feature, full.str().c_str());
    return Result::nwNone();
  }
  INTRINSIC iopc = it->second;
  if (name == "psllwi128" || name == "pslldi128" || name == "psllqi128" ||
      name == "psrlwi128" || name == "psrldi128" || name == "psrlqi128" ||
      name == "psrawi128" || name == "psradi128") {
    // As wgen: these instructions take the count in an xmm register, so
    // the integer count is converted to a vector.
    WN *kids[2];
    TY_IDX aty = _builder->TB().ConvertType(expr->getArg(0)->getType());
    kids[0] = WGEN_CreateParm(Mtype_comparison(TY_mtype(aty)),
                              ConvertToNode(expr->getArg(0)), aty);
    WN *cnt = ConvertToNode(expr->getArg(1));
    cnt = WN_Cvt(WN_rtype(cnt), MTYPE_V16I8, cnt);
    kids[1] = WGEN_CreateParm(MTYPE_V16I8, cnt, MTYPE_To_TY(MTYPE_V16I8));
    return Result::nwNode(WN_Create_Intrinsic(OPR_INTRINSIC_OP, rmt, MTYPE_V,
                                              iopc, 2, kids), ty);
  }
  if (INTRN_return_kind(iopc) == IRETURN_V)
    return GenerateIntrinsic(expr, ty, iopc, FALSE, FALSE);
  // As wgen (its bug 11876): MMX results take the intrinsic's own type.
  // (Only for a 64-bit result: some intrinsics serve the 128-bit form too,
  // pmaddwd for one, and keep the M8 return kind of the MMX one.)
  if (TY_size(ty) == 8)
  switch (INTRN_return_kind(iopc)) {
  case IRETURN_M8I1: ty = MTYPE_To_TY(MTYPE_M8I1); break;
  case IRETURN_M8I2: ty = MTYPE_To_TY(MTYPE_M8I2); break;
  case IRETURN_M8I4: ty = MTYPE_To_TY(MTYPE_M8I4); break;
  default: break;
  }
  Result r = GenerateIntrinsic(expr, ty, iopc, TRUE, TRUE);
  if (TY_mtype(ty) != rmt && r.isNode())
    return Result::nwNode(WN_Tas(rmt, _builder->TB().ConvertType(expr->getType()),
                                 r.Node()), _builder->TB().ConvertType(expr->getType()));
  return r;
#else
  ErrMsg(EC_Unimplemented_Feature,
         _builder->Context()->BuiltinInfo.getName(builtin_id));
  return Result::nwNone();
#endif
}

Result
WhirlExprBuilder::ConvertAtomicExpr(const AtomicExpr *expr, BOOL retv) {
  // Path64: C11/GNU __atomic builtins need intrinsics Path64 does not
  // have yet (INTRN_ATOM_*, 1/2-byte and float forms). Upstream's
  // lowering is kept below, disabled, as the starting point.
  ErrMsg(EC_Unimplemented_Feature, "C11 / __atomic builtins");
  return Result::nwNone();
#if 0 // upstream
  TRACE_FUNC();
  QualType AtomicTy = expr->getPtr()->getType()->getPointeeType();
  QualType MemTy = AtomicTy;
  if (const AtomicType *AT = AtomicTy->getAs<AtomicType>())
     MemTy = AT->getValueType();
  QualType atomic_ty = expr->getPtr()->getType()->getPointeeType();
  QualType mem_ty = atomic_ty;
  if (const AtomicType *type = atomic_ty->getAs<AtomicType>())
    mem_ty = type->getValueType();

  //WN *val1, *val2, *dest, *order_fail, *is_weak;
  //WN *ptr = ConvertExpr(expr->getPtr()).node();
  TY_IDX ptr_ty = _builder->TB().ConvertType(expr->getPtr()->getType());
  Is_True(TY_kind(ptr_ty) == KIND_POINTER, ("not ptr type"));
  TY_IDX elm_ty = TY_pointed(ptr_ty);

  if (expr->getOp() == AtomicExpr::AO__c11_atomic_init ||
      expr->getOp() == AtomicExpr::AO__opencl_atomic_init) {
    Result lhs = ConvertExpr(expr->getPtr());
    Result rhs = ConvertExpr(expr->getVal1());
    RV rv = GetRV(expr, retv);
    return EmitAssignNode(lhs, rhs, elm_ty, rv);
  }

  WN *order = ConvertToNode(expr->getOrder());
  TY_IDX order_ty = _builder->TB().ConvertType(expr->getOrder()->getType());
  WN *parm[8] = { NULL, NULL, NULL, NULL,
                  NULL, NULL, NULL, NULL };
  TY_IDX parm_ty[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  parm[0] = ConvertToNode(expr->getPtr());
  parm_ty[0] = ptr_ty;

  std::string lib_call_name;
  INTRINSIC   iopc = INTRINSIC_NONE;
  BOOL        extra_iload = FALSE;   // if an extra iload is needed
  BOOL        intrinsic_op = FALSE;
  WN         *order_wn = NULL;
  UINT        parm_cnt = 0;
  switch (expr->getOp()) {
  case AtomicExpr::AO__c11_atomic_init:
  case AtomicExpr::AO__opencl_atomic_init:
    Is_True(false, ("Already handled above with EmitAtomicInit"));

  case AtomicExpr::AO__atomic_exchange:
  case AtomicExpr::AO__opencl_atomic_exchange:
    // atomic_exchange(*ptr, *val, *ret): *ret = xchg(ptr, *val)
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = ConvertToNode(expr->getVal2());
    parm_ty[2] =  _builder->TB().ConvertType(expr->getVal2()->getType());
    parm[3] = order;
    parm_ty[3] = order_ty;
    parm_cnt = 4;
    switch (TY_size(elm_ty)) {
    case 1:  iopc = INTRN_ATOM_XCHG_1;  break;
    case 2:  iopc = INTRN_ATOM_XCHG_2;  break;
    case 4:  iopc = INTRN_ATOM_XCHG_4;  break;
    case 8:  iopc = INTRN_ATOM_XCHG_8;  break;
    default:
      Is_True(FALSE, ("unsupported type %s", TY_name(elm_ty)));
    }
    break;

  case AtomicExpr::AO__c11_atomic_exchange:
  case AtomicExpr::AO__atomic_exchange_n:
    // ret = atomic_exchange_n(*ptr, val): ret = xchg(ptr, val)
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    // map to legacy LOCK_TEST_AND_SET_In
    switch (TY_size(elm_ty)) {
    case 1:  iopc = INTRN_LOCK_TEST_AND_SET_I1;  break;
    case 2:  iopc = INTRN_LOCK_TEST_AND_SET_I2;  break;
    case 4:  iopc = INTRN_LOCK_TEST_AND_SET_I4;  break;
    case 8:  iopc = INTRN_LOCK_TEST_AND_SET_I8;  break;
    default:
      Is_True(FALSE, ("unsupported mtype %d", TY_mtype(elm_ty)));
    }
    break;

  case AtomicExpr::AO__atomic_store:
  case AtomicExpr::AO__c11_atomic_store:
  case AtomicExpr::AO__opencl_atomic_store:
    // atomic_store(*ptr, *val): *ptr = *val
    // fall through
  case AtomicExpr::AO__atomic_store_n:
    // atomic_store_n(*ptr, val): *ptr = val;
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    switch (TY_size(elm_ty)) {
    case 1:
      iopc = (expr->getOp() == AtomicExpr::AO__atomic_store_n)
               ? INTRN_ATOM_STORE_N_1 : INTRN_ATOM_STORE_1;
      break;
    case 2:
      iopc = (expr->getOp() == AtomicExpr::AO__atomic_store_n)
               ? INTRN_ATOM_STORE_N_2 : INTRN_ATOM_STORE_2;
      break;
    case 4:
      iopc = (expr->getOp() == AtomicExpr::AO__atomic_store_n)
               ? INTRN_ATOM_STORE_N_4 : INTRN_ATOM_STORE_4;
      break;
    case 8:
      iopc = (expr->getOp() == AtomicExpr::AO__atomic_store_n)
               ? INTRN_ATOM_STORE_N_8 : INTRN_ATOM_STORE_8;
      break;
    default:
      Is_True(FALSE, ("unsupported type %s", TY_name(elm_ty)));
    }
    break;

  case AtomicExpr::AO__atomic_load:
  case AtomicExpr::AO__opencl_atomic_load:
    // atomic_load(*ptr, *ret): *ret = *ptr
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    iopc = INTRN_ATOM_LOAD;
    break;

  case AtomicExpr::AO__c11_atomic_load:
  case AtomicExpr::AO__atomic_load_n:
    // ret = atomic_load(*ptr): ret = *ptr
    parm[1] = order;
    parm_ty[1] = order_ty;
    parm_cnt = 2;
    intrinsic_op = TRUE;
    iopc = INTRN_ATOM_LOAD_N;
    break;

  case AtomicExpr::AO__c11_atomic_fetch_add:
  case AtomicExpr::AO__opencl_atomic_fetch_add:
  case AtomicExpr::AO__atomic_fetch_add:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_FETCH_AND_ADD_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_FETCH_AND_ADD_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_FETCH_AND_ADD_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_FETCH_AND_ADD_I8;  break;
    case MTYPE_F4:  iopc = INTRN_FETCH_AND_ADD_F4;  break;
    case MTYPE_F8:  iopc = INTRN_FETCH_AND_ADD_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__atomic_add_fetch:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_ADD_AND_FETCH_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_ADD_AND_FETCH_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_ADD_AND_FETCH_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_ADD_AND_FETCH_I8;  break;
    case MTYPE_F4:  iopc = INTRN_ADD_AND_FETCH_F4;  break;
    case MTYPE_F8:  iopc = INTRN_ADD_AND_FETCH_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__c11_atomic_fetch_and:
  case AtomicExpr::AO__opencl_atomic_fetch_and:
  case AtomicExpr::AO__atomic_fetch_and:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_FETCH_AND_AND_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_FETCH_AND_AND_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_FETCH_AND_AND_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_FETCH_AND_AND_I8;  break;
    case MTYPE_F4:  iopc = INTRN_FETCH_AND_AND_F4;  break;
    case MTYPE_F8:  iopc = INTRN_FETCH_AND_AND_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__atomic_and_fetch:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_AND_AND_FETCH_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_AND_AND_FETCH_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_AND_AND_FETCH_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_AND_AND_FETCH_I8;  break;
    case MTYPE_F4:  iopc = INTRN_AND_AND_FETCH_F4;  break;
    case MTYPE_F8:  iopc = INTRN_AND_AND_FETCH_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__c11_atomic_fetch_or:
  case AtomicExpr::AO__opencl_atomic_fetch_or:
  case AtomicExpr::AO__atomic_fetch_or:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_FETCH_AND_OR_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_FETCH_AND_OR_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_FETCH_AND_OR_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_FETCH_AND_OR_I8;  break;
    case MTYPE_F4:  iopc = INTRN_FETCH_AND_OR_F4;  break;
    case MTYPE_F8:  iopc = INTRN_FETCH_AND_OR_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__atomic_or_fetch:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_OR_AND_FETCH_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_OR_AND_FETCH_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_OR_AND_FETCH_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_OR_AND_FETCH_I8;  break;
    case MTYPE_F4:  iopc = INTRN_OR_AND_FETCH_F4;  break;
    case MTYPE_F8:  iopc = INTRN_OR_AND_FETCH_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__c11_atomic_fetch_sub:
  case AtomicExpr::AO__opencl_atomic_fetch_sub:
  case AtomicExpr::AO__atomic_fetch_sub:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_FETCH_AND_SUB_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_FETCH_AND_SUB_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_FETCH_AND_SUB_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_FETCH_AND_SUB_I8;  break;
    case MTYPE_F4:  iopc = INTRN_FETCH_AND_SUB_F4;  break;
    case MTYPE_F8:  iopc = INTRN_FETCH_AND_SUB_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__atomic_sub_fetch:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_SUB_AND_FETCH_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_SUB_AND_FETCH_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_SUB_AND_FETCH_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_SUB_AND_FETCH_I8;  break;
    case MTYPE_F4:  iopc = INTRN_SUB_AND_FETCH_F4;  break;
    case MTYPE_F8:  iopc = INTRN_SUB_AND_FETCH_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__c11_atomic_fetch_xor:
  case AtomicExpr::AO__opencl_atomic_fetch_xor:
  case AtomicExpr::AO__atomic_fetch_xor:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_FETCH_AND_XOR_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_FETCH_AND_XOR_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_FETCH_AND_XOR_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_FETCH_AND_XOR_I8;  break;
    case MTYPE_F4:  iopc = INTRN_FETCH_AND_XOR_F4;  break;
    case MTYPE_F8:  iopc = INTRN_FETCH_AND_XOR_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__atomic_xor_fetch:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_XOR_AND_FETCH_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_XOR_AND_FETCH_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_XOR_AND_FETCH_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_XOR_AND_FETCH_I8;  break;
    case MTYPE_F4:  iopc = INTRN_XOR_AND_FETCH_F4;  break;
    case MTYPE_F8:  iopc = INTRN_XOR_AND_FETCH_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] = order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__c11_atomic_compare_exchange_strong:
  case AtomicExpr::AO__c11_atomic_compare_exchange_weak:
    // order is specified
    // TODO: check the constant value. 5 is _SEQ_CST
    order_wn = WN_Intconst(MTYPE_I4, 5);
    // fall throw
  case AtomicExpr::AO__opencl_atomic_compare_exchange_strong:
  case AtomicExpr::AO__opencl_atomic_compare_exchange_weak:
  case AtomicExpr::AO__atomic_compare_exchange:
    // bool atomic_compare_exchange(*ptr, *expected, *desired)
    // need an extra iload parm2
    extra_iload = TRUE;
    // fall through
  case AtomicExpr::AO__atomic_compare_exchange_n:
    // bool atomic_compare_exchange_n(*ptr, *expected, desired)
    // map to legacy INTRN_BOOL_COMPARE_AND_SWAP_In
    switch (TY_size(elm_ty)) {
    case 1:  iopc = INTRN_BOOL_COMPARE_AND_SWAP_I1;  break;
    case 2:  iopc = INTRN_BOOL_COMPARE_AND_SWAP_I2;  break;
    case 4:  iopc = INTRN_BOOL_COMPARE_AND_SWAP_I4;  break;
    case 8:  iopc = INTRN_BOOL_COMPARE_AND_SWAP_I8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    // need an iload on parm1
    parm[1] = WN_Iload(Mtype_comparison(TY_mtype(elm_ty)), 0,
                       elm_ty, ConvertToNode(expr->getVal1()));
    parm_ty[1] = elm_ty;
    if (extra_iload)
      parm[2] = WN_Iload(Mtype_comparison(TY_mtype(elm_ty)), 0,
                         elm_ty, ConvertToNode(expr->getVal2()));
    else
      parm[2] = ConvertToNode(expr->getVal2());
    parm_ty[2] = elm_ty;
    parm[3] = order_wn ? order_wn : ConvertToNode(expr->getWeak());
    parm_ty[3] = order_wn ? order_ty : _builder->TB().ConvertType(expr->getWeak()->getType());
    parm[4] = order;
    parm_ty[4] = order_ty;
    parm[5] = ConvertToNode(expr->getOrderFail());
    parm_ty[5] = _builder->TB().ConvertType(expr->getOrderFail()->getType());
    parm_cnt = 6;
    break;

  case AtomicExpr::AO__opencl_atomic_fetch_min:
  case AtomicExpr::AO__atomic_fetch_min:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_FETCH_AND_MIN_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_FETCH_AND_MIN_I8;  break;
    case MTYPE_F4:  iopc = INTRN_FETCH_AND_MIN_F4;  break;
    case MTYPE_F8:  iopc = INTRN_FETCH_AND_MIN_F8;  break;
    case MTYPE_I1:
    case MTYPE_U1:
    case MTYPE_I2:
    case MTYPE_U2:
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] =  order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__opencl_atomic_fetch_max:
  case AtomicExpr::AO__atomic_fetch_max:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_FETCH_AND_MAX_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_FETCH_AND_MAX_I8;  break;
    case MTYPE_F4:  iopc = INTRN_FETCH_AND_MAX_F4;  break;
    case MTYPE_F8:  iopc = INTRN_FETCH_AND_MAX_F8;  break;
    case MTYPE_I1:
    case MTYPE_U1:
    case MTYPE_I2:
    case MTYPE_U2:
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] =  order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__atomic_fetch_nand:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_FETCH_AND_NAND_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_FETCH_AND_NAND_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_FETCH_AND_NAND_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_FETCH_AND_NAND_I8;  break;
    case MTYPE_F4:  iopc = INTRN_FETCH_AND_NAND_F4;  break;
    case MTYPE_F8:  iopc = INTRN_FETCH_AND_NAND_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] =  order_ty;
    parm_cnt = 3;
    break;

  case AtomicExpr::AO__atomic_nand_fetch:
    switch (TY_mtype(elm_ty)) {
    case MTYPE_I1:
    case MTYPE_U1:  iopc = INTRN_NAND_AND_FETCH_I1;  break;
    case MTYPE_I2:
    case MTYPE_U2:  iopc = INTRN_NAND_AND_FETCH_I2;  break;
    case MTYPE_I4:
    case MTYPE_U4:  iopc = INTRN_NAND_AND_FETCH_I4;  break;
    case MTYPE_I8:
    case MTYPE_U8:  iopc = INTRN_NAND_AND_FETCH_I8;  break;
    case MTYPE_F4:  iopc = INTRN_NAND_AND_FETCH_F4;  break;
    case MTYPE_F8:  iopc = INTRN_NAND_AND_FETCH_F8;  break;
    default:
      Is_True(FALSE, ("TODO: not supported mtype"));
    }
    parm[1] = ConvertToNode(expr->getVal1());
    parm_ty[1] =  _builder->TB().ConvertType(expr->getVal1()->getType());
    parm[2] = order;
    parm_ty[2] =  order_ty;
    parm_cnt = 3;
    break;
  }

  QualType val_ty = expr->getType().getUnqualifiedType();
  TY_IDX rtype_idx = _builder->TB().ConvertType(val_ty);

  // wrap parm with OPR_PARM
  for (UINT i = 0; i < parm_cnt; ++i) {
    Is_True(parm[i] != NULL, ("invalid parm"));
    parm[i] = WGEN_CreateParm(Mtype_comparison(WN_rtype(parm[i])),
                              parm[i], parm_ty[i]);
  }

  WN *call_wn = NULL;
  if (iopc != INTRINSIC_NONE) {
    if (intrinsic_op) {
      // generate op and return the OP directly
      call_wn = WN_Create_Intrinsic(OPR_INTRINSIC_OP,
                                    Mtype_comparison(TY_mtype(rtype_idx)),
                                    MTYPE_V,
                                    iopc, parm_cnt, parm);
      return Result::nwNode(call_wn, rtype_idx);
    }
    call_wn = WN_Create_Intrinsic(OPR_INTRINSIC_CALL,
                                  Mtype_comparison(TY_mtype(rtype_idx)),
                                  MTYPE_V,
                                  iopc, parm_cnt, parm);
  }
  else {
    Is_True(!lib_call_name.empty(), ("lib_call_name not set"));
    if (expr->isOpenCL()) {
      lib_call_name = "__opencl" +
                      StringRef(lib_call_name).drop_front(1).str();
    }

    // create pu type
    TY_IDX ty_idx;
    TY &func = New_TY(ty_idx);
    TY_Init(func, 0, KIND_FUNCTION, MTYPE_UNKNOWN, 0);
    Set_TY_align(ty_idx, 1);

    // set return type
    TYLIST_IDX tylist_idx;
    Set_TYLIST_type(New_TYLIST(tylist_idx), rtype_idx);
    Set_TY_tylist(ty_idx, tylist_idx);
    Set_TYLIST_type(New_TYLIST(tylist_idx), 0);

    // create pu info
    PU_IDX pu_idx;
    PU &pu = New_PU(pu_idx);
    TY_IDX pu_ty_idx = ty_idx;
    PU_Init(pu, pu_ty_idx, GLOBAL_SYMTAB + 1);
    ty_idx = (TY_IDX) pu_idx;
    ST *st = New_ST(GLOBAL_SYMTAB);
    ST_Init (st, Save_Str(lib_call_name.c_str()), CLASS_FUNC,
             SCLASS_TEXT, EXPORT_LOCAL, ty_idx);
    Set_ST_Srcpos(*st, GetSrcPos());

    call_wn = WN_Create(OPR_CALL, TY_mtype(rtype_idx), MTYPE_V, parm_cnt);
    WN_st_idx(call_wn) = ST_st_idx(st);

    for (UINT i = 0; i < parm_cnt; ++i) {
      Is_True(parm[i] != NULL && WN_operator(parm[i]) == OPR_PARM,
              ("invalid parm"));
      WN_kid(call_wn, i) = parm[i];
    }
  }
  WN_Set_Linenum(call_wn, GetSrcPos());
  Is_True(call_wn != NULL, ("call_wn not set"));

  if (!retv || val_ty->isVoidType())
    return Result::nwNode(call_wn, rtype_idx);

  WN *blk = WN_CreateBlock();
  WN_INSERT_BlockLast(blk, call_wn);

  WN *ret = WN_Ldid(TY_mtype(rtype_idx), -1, Return_Val_Preg, rtype_idx);
  if (iopc != INTRINSIC_NONE) {
    WN *stid = WGEN_StidTemp(rtype_idx, ret, "intrn.ret");
    WN_INSERT_BlockLast(blk, stid);
    WN_Set_Linenum(stid, GetSrcPos());
    ret = WN_Ldid(TY_mtype(rtype_idx), WN_offset(stid), WN_st(stid), rtype_idx);
  }
  WN *comma = WGEN_CreateComma(WN_rtype(ret), blk, ret);

  return Result::nwNode(comma, rtype_idx);
#endif
}

} // namespace wgen
