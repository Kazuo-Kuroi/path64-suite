/*
  Path64 port of clang2whirl.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.
*/

#include <map>
#include "c2w_path64.h"
#include "errors.h"
#include "erglob.h"

static std::map<TY_IDX, ST_IDX> ty_vtable;
static std::map<ST_IDX, TY_IDX> vtable_ty;

ST_IDX TY_vtable(TY_IDX ty)
{
  std::map<TY_IDX, ST_IDX>::const_iterator i = ty_vtable.find(TY_IDX_index(ty));
  return i == ty_vtable.end() ? (ST_IDX) 0 : i->second;
}

void Set_TY_vtable(TY_IDX ty, ST_IDX vtable)
{
  ty_vtable[TY_IDX_index(ty)] = vtable;
}

TY_IDX ST_vtable_ty_idx(const ST *st)
{
  std::map<ST_IDX, TY_IDX>::const_iterator i = vtable_ty.find(ST_st_idx(st));
  return i == vtable_ty.end() ? (TY_IDX) 0 : i->second;
}

void Set_ST_vtable_ty_idx(ST *st, TY_IDX ty)
{
  vtable_ty[ST_st_idx(st)] = ty;
}

void Set_INITV_lab_flags(INITV_IDX, INT16 flags)
{
  if (flags != INITVLABELFLAGS_UNUSED)
    ErrMsg(EC_Unimplemented_Feature,
           "label differences (&&a - &&b) in static initializers");
}

// Upstream Open64's symtab.cxx.
UINT FLD_get_count(TY_IDX struct_ty_idx)
{
  UINT count = 1;
  FLD_ITER fld_iter = Make_fld_iter(TY_fld(struct_ty_idx));
  do {
    FLD_HANDLE fld(fld_iter);
    if (fld.Is_Null())
      break;
    if (TY_kind(FLD_type(fld)) == KIND_STRUCT &&
        TY_fld(FLD_type(fld)) != FLD_HANDLE())
      count += FLD_get_count(FLD_type(fld));
    else
      count++;
  } while (!FLD_last_field(fld_iter++));
  return count;
}

// Upstream Open64's wn_util.cxx: copy a tree, dropping comma operands.
WN *WN_COPY_Tree_Without_Commas(WN *wn)
{
  if (wn == NULL)
    return NULL;
  if (WN_operator(wn) == OPR_COMMA)
    return WN_COPY_Tree_Without_Commas(WN_kid1(wn));
  if (WN_operator(wn) == OPR_RCOMMA)
    return WN_COPY_Tree_Without_Commas(WN_kid0(wn));

  WN *new_wn = WN_CopyNode(wn);
  if (WN_opcode(wn) == OPC_BLOCK) {
    WN *prev = NULL;
    for (WN *kid = WN_first(wn); kid; kid = WN_next(kid)) {
      WN *copy = WN_COPY_Tree_Without_Commas(kid);
      if (prev == NULL)
        WN_first(new_wn) = copy;
      else
        WN_next(prev) = copy;
      WN_prev(copy) = prev;
      prev = copy;
    }
    WN_last(new_wn) = prev;
  } else {
    for (INT i = 0; i < WN_kid_count(wn); i++)
      WN_kid(new_wn, i) = WN_COPY_Tree_Without_Commas(WN_kid(wn, i));
  }
  return new_wn;
}

INTRINSIC Sync_Intrinsic_For_Size(INTRINSIC i4, UINT size)
{
  if (size == 4)
    return i4;
  if (size == 8) {
    switch (i4) {
#define I8(op) case INTRN_##op##_I4: return INTRN_##op##_I8;
    I8(ADD_AND_FETCH) I8(SUB_AND_FETCH) I8(OR_AND_FETCH)
    I8(XOR_AND_FETCH) I8(AND_AND_FETCH) I8(NAND_AND_FETCH)
    I8(FETCH_AND_ADD) I8(FETCH_AND_SUB) I8(FETCH_AND_OR)
    I8(FETCH_AND_XOR) I8(FETCH_AND_AND) I8(FETCH_AND_NAND)
    I8(LOCK_TEST_AND_SET) I8(LOCK_RELEASE)
    I8(BOOL_COMPARE_AND_SWAP) I8(VAL_COMPARE_AND_SWAP)
#undef I8
    default:
      break;
    }
  }
  ErrMsg(EC_Unimplemented_Feature, "__sync and __atomic builtins on "
         "operands other than 4 or 8 bytes");
  return i4;
}
