/*
  Path64 port of clang2whirl.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.
*/

// Interfaces clang2whirl uses from upstream Open64 that Path64's WHIRL
// lacks or spells differently.

#ifndef C2W_PATH64_H
#define C2W_PATH64_H

#include <string.h>
#include "defs.h"
#include "srcpos.h"
#include "symtab.h"
#include "irbdata.h"
#include "wn.h"
#include "wn_util.h"
#include "wintrinsic.h"
#include "errors.h"
#include "erglob.h"

// Symbol source positions: Path64's ST has no source position, so these
// lose the line number of a few compiler-generated statements.
inline SRCPOS ST_Srcpos(const ST *) { return 0; }
inline SRCPOS ST_Srcpos(const ST &) { return 0; }
inline SRCPOS ST_Srcpos(ST_IDX) { return 0; }
inline void Set_ST_Srcpos(ST &, SRCPOS) {}

// C++ information upstream records for devirtualization and ODR checks.
// Path64's back end does not use it.
inline void Set_ST_is_odr(ST *) {}
inline void Set_ST_is_vtable(ST *) {}
inline void Set_PU_is_constructor(PU &) {}
inline void Set_FLD_is_virtual(FLD_HANDLE) {}

// Class type <-> vtable symbol, kept in side tables (upstream stores them
// in TY and ST).
extern ST_IDX TY_vtable(TY_IDX ty);
extern void Set_TY_vtable(TY_IDX ty, ST_IDX vtable);
extern TY_IDX ST_vtable_ty_idx(const ST *st);
extern void Set_ST_vtable_ty_idx(ST *st, TY_IDX ty);

// Label-difference initializers (GNU &&a - &&b in static data) need
// back-end support Path64 does not have; using them is an error.
enum INITVLABELFLAGS {
  INITVLABELFLAGS_FIRST = 0,
  INITVLABELFLAGS_UNUSED = 0,
  INITVLABELFLAGS_VALUES_FIRST,
  INITVLABELFLAGS_VALUES_PLUS,
  INITVLABELFLAGS_VALUES_MINUS,
  INITVLABELFLAGS_VALUES_LAST,
  INITVLABELFLAGS_LAST = INITVLABELFLAGS_VALUES_LAST
};
extern void Set_INITV_lab_flags(INITV_IDX inv, INT16 flags);
inline void Set_INITV_lab_mtype(INITV_IDX, TYPE_ID) {}

extern UINT Get_INITV_Size(INITV_IDX inv);   // irbdata.cxx
extern UINT FLD_get_count(TY_IDX struct_ty);
extern WN *WN_COPY_Tree_Without_Commas(WN *wn);

// __sync builtins: Path64 has 4- and 8-byte forms only. Given the 4-byte
// intrinsic, return the one for an operand of SIZE bytes; 1- and 2-byte
// operands are reported as unsupported.
extern INTRINSIC Sync_Intrinsic_For_Size(INTRINSIC i4, UINT size);

#define BZERO(p, n) memset((p), 0, (n))

#endif /* C2W_PATH64_H */
