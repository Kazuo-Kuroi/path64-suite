/*
 *  Copyright (C) 2007. QLogic Corporation. All Rights Reserved.
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


#include <stdio.h>
#include <stdlib.h>
#include "libelf/libelf.h"
#include <list>
#include "elf_defines.h"
#include <elfaccess.h>

#include "defs.h"
#include "erglob.h"
#include "glob.h"
#include "flags.h"
#include "tracing.h"
#include "config.h"
#include "config_asm.h"
#include "be_util.h"
#include "cgir.h"
#include "register.h"
#include "tn_map.h"
#include "em_elf.h"
#include "em_dwarf.h"
#include "cgtarget.h"
#include "calls.h"
#include "cgemit.h"
#include "data_layout.h"
#include "stblock.h"		// Base_Symbol_And_Offset_For_Addressing
#include "cgdwarf_targ.h"
#include "whirl2ops.h"

// ====================================================================
// Call-frame information (.cfi_* directives) for the prologue.
//
// The assembler builds .eh_frame from the .cfi_startproc/.cfi_endproc
// that cgemit brackets each PU with; these directives describe the
// prologue so that unwinders (libeh for C++ exceptions, backtraces) can
// step through PathScale-compiled frames. Only entry blocks are
// described: nothing can throw after an epilogue restores $sp, and
// location-based CFI there would mislabel code laid out after it.
//
// Emit_Unwind_Directives_For_OP runs just before an OP is printed, so the
// directives for a prologue OP are queued and printed at the next call,
// after the OP itself.
// ====================================================================

static OP *Pending_Op;		// prologue OP whose directives are due
static INT64 Cfa_Offset;	// CFA - $sp (or - $fp once the CFA uses $fp)
static BOOL Cfa_Uses_Fp;

// call per-PU
void 
Init_Unwind_Info (BOOL trace)
{
  Pending_Op = NULL;
  Cfa_Offset = 0;
  Cfa_Uses_Fp = FALSE;
}

void 
Finalize_Unwind_Info(void)
{
  Pending_Op = NULL;
}

static INT
Int_Reg_Number (TN *tn)
{
  if (!TN_is_register(tn) || TN_register(tn) == REGISTER_UNDEFINED ||
      TN_register_class(tn) != ISA_REGISTER_CLASS_integer)
    return -1;
  return REGISTER_machine_id(TN_register_class(tn), TN_register(tn));
}

// Registers the MIPS n32/n64 ABIs make callee-saved, plus $ra: their
// prologue saves are what an unwinder must find.
static BOOL
Is_Saved_Int_Reg (INT regno)
{
  return (regno >= 16 && regno <= 23) || regno == 28 || regno == 30 ||
	 regno == 31;
}

// Value of an immediate or stack-offset operand, if known.
static BOOL
Operand_Value (TN *tn, INT64 *value)
{
  if (TN_has_value(tn)) {
    *value = TN_value(tn);
    return TRUE;
  }
  if (TN_is_symbol(tn)) {
    ST *base_st;
    INT64 base_ofst;
    Base_Symbol_And_Offset_For_Addressing(TN_var(tn), TN_offset(tn),
					  &base_st, &base_ofst);
    if (base_st == SP_Sym || base_st == FP_Sym) {
      *value = base_ofst;
      return TRUE;
    }
  }
  return FALSE;
}

static void
Emit_Prologue_Cfi (OP *op, FILE *f)
{
  const TOP top = OP_code(op);

  // $sp -= N
  if ((top == TOP_addiu || top == TOP_daddiu) && OP_results(op) == 1 &&
      Int_Reg_Number(OP_result(op, 0)) == 29 &&
      Int_Reg_Number(OP_opnd(op, 0)) == 29) {
    INT64 delta;
    if (!Cfa_Uses_Fp && Operand_Value(OP_opnd(op, 1), &delta)) {
      Cfa_Offset -= delta;
      fprintf(f, "\t.cfi_def_cfa_offset %lld\n", (long long) Cfa_Offset);
    }
    return;
  }

  // $fp = $sp (+ k): the CFA is then computed from $fp.
  if (OP_results(op) == 1 && OP_opnds(op) >= 2 &&
      Int_Reg_Number(OP_result(op, 0)) == 30 &&
      Int_Reg_Number(OP_opnd(op, 0)) == 29) {
    INT64 k = 0;
    BOOL known = FALSE;
    if ((top == TOP_or || top == TOP_addu || top == TOP_daddu) &&
	Int_Reg_Number(OP_opnd(op, 1)) == 0)
      known = TRUE;
    else if (top == TOP_addiu || top == TOP_daddiu)
      known = Operand_Value(OP_opnd(op, 1), &k);
    if (known && !Cfa_Uses_Fp) {
      Cfa_Offset -= k;
      Cfa_Uses_Fp = TRUE;
      fprintf(f, "\t.cfi_def_cfa 30, %lld\n", (long long) Cfa_Offset);
    }
    return;
  }

  // Save of a callee-saved register or $ra to the frame.
  if (OP_store(op)) {
    const INT base_idx = OP_find_opnd_use(op, OU_base);
    const INT ofst_idx = OP_find_opnd_use(op, OU_offset);
    const INT val_idx = OP_find_opnd_use(op, OU_storeval);
    if (base_idx < 0 || ofst_idx < 0 || val_idx < 0)
      return;
    const INT regno = Int_Reg_Number(OP_opnd(op, val_idx));
    const INT base = Int_Reg_Number(OP_opnd(op, base_idx));
    INT64 ofst;
    if (!Is_Saved_Int_Reg(regno) ||
	!Operand_Value(OP_opnd(op, ofst_idx), &ofst))
      return;
    // The slot's address relative to the CFA.
    if ((base == 29 && !Cfa_Uses_Fp) || (base == 30 && Cfa_Uses_Fp))
      fprintf(f, "\t.cfi_offset %d, %lld\n", regno,
	      (long long) (ofst - Cfa_Offset));
  }
}

void 
Emit_Unwind_Directives_For_OP(OP *op, FILE *f)
{
  if (Pending_Op != NULL) {
    Emit_Prologue_Cfi(Pending_Op, f);
    Pending_Op = NULL;
  }
  if (op != NULL && OP_bb(op) != NULL && BB_entry(OP_bb(op)))
    Pending_Op = op;
}


void
Check_Dwarf_Rel(const Elf32_Rel &current_reloc)
{
  FmtAssert(REL32_type(current_reloc) == R_IA_64_DIR32MSB,
	    ("Unimplemented 32-bit relocation type %d",
	     REL32_type(current_reloc)));
}

void
Check_Dwarf_Rel(const Elf64_AltRel &current_reloc)
{
  FmtAssert(REL64_type(current_reloc) == R_IA_64_DIR64MSB,
	    ("Unimplemented 64-bit relocation type %d",
	     REL64_type(current_reloc)));
}

void
Check_Dwarf_Rela(const Elf64_AltRela &current_reloc)
{
  FmtAssert(FALSE,
	    ("Unimplemented 64-bit relocation type %d",
	     REL64_type(current_reloc)));
}

void
Check_Dwarf_Rela(const Elf32_Rela &current_reloc)
{
  FmtAssert(FALSE,
	    ("Unimplemented 32-bit relocation type %d",
	     REL32_type(current_reloc)));
}
static const char *drop_these[] = {
      // Assembler generates .debug_line from directives itself, so we
      // don't output it.
	".debug_line",
     // gdb does not use the MIPS sections
     // debug_weaknames, etc.
	".debug_weaknames",
	".debug_varnames",
	".debug_typenames",
	".debug_funcnames",
	0
};
// return TRUE if we want to emit the section (IA-64).
// return FALSE if we do not want to for IA-64.
extern BOOL Is_Dwarf_Section_To_Emit(const char *name)
{

	for(int  i = 0; drop_these[i]; ++i) {
	  if(strcmp(name,drop_these[i]) == 0) {
	    return FALSE;
	  }
	}
        if (Debug_Level < 1 && strncmp(name, ".debug_", 7) == 0)
          return FALSE;
        return TRUE;
}

