/*
 * Copyright (C) 2007, 2008. PathScale, LLC. All Rights Reserved.
 */
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
#include "stblock.h"
#include "cgdwarf_targ.h"

static OP *Pending_Save;

// call per-PU
void
Init_Unwind_Info (BOOL trace)
{
  Pending_Save = NULL;
}

void
Emit_Unwind_Directives_For_OP(OP *op, FILE *f)
{
  // Frame-pointer prologues save GRA's callee-saved registers with stores.
  // The push/pop path in calls.cxx handles frameless functions only. Describe
  // these stores too, or an exception can silently corrupt the caller's state.
  if (Pending_Save) {
    OP *save = Pending_Save;
    Pending_Save = NULL;
    INT value = OP_find_opnd_use(save, OU_storeval);
    INT offset = OP_find_opnd_use(save, OU_offset);
    INT base = OP_find_opnd_use(save, OU_base);
    if (value >= 0 && offset >= 0 && base >= 0 &&
        TNs_Are_Equivalent(OP_opnd(save, base), FP_TN)) {
      TN *source = OP_opnd(save, value);
      TN *displacement = OP_opnd(save, offset);
      for (INT i = 0; i < Cgdwarf_Num_Callee_Saved_Regs(); ++i) {
        TN *reg = Cgdwarf_Nth_Callee_Saved_Reg(i);
        ST *slot = Cgdwarf_Nth_Callee_Saved_Reg_Location(i);
        if (!slot || !TNs_Are_Equivalent(source, reg)) continue;
        ST *slot_base;
        INT64 slot_offset;
        Base_Symbol_And_Offset_For_Addressing(slot, 0, &slot_base, &slot_offset);
        if (slot_base != FP_Sym) continue;
        INT64 actual_offset;
        if (TN_has_value(displacement)) actual_offset = TN_value(displacement);
        else if (TN_is_symbol(displacement)) {
          ST *actual_base;
          Base_Symbol_And_Offset_For_Addressing(TN_var(displacement),
              TN_offset(displacement), &actual_base, &actual_offset);
          if (actual_base != FP_Sym) continue;
        } else continue;
        if (actual_offset != slot_offset) continue;
        INT number = REGISTER_machine_id(TN_register_class(reg), TN_register(reg));
        if (number == 1) number = 3; // rbx / ebx
        if (Is_Target_32bit()) {
          if (number == 5) number = 6; // esi
          else if (number == 4) number = 7; // edi
        }
        fprintf(f, "\t.cfi_offset %d, %lld\n", number,
                (long long)(slot_offset - (Is_Target_64bit() ? 16 : 8)));
        break;
      }
    }
  }
  if (Gen_Frame_Pointer && op && OP_bb(op) && BB_entry(OP_bb(op)) && OP_store(op))
    Pending_Save = op;
}

void
Finalize_Unwind_Info(void)
{
  Pending_Save = NULL;
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
	DEBUG_LINE_SECTNAME,
     // gdb does not use the MIPS sections
     // debug_weaknames, etc.
	DEBUG_VARNAMES_SECTNAME,
	DEBUG_TYPENAMES_SECTNAME,
	DEBUG_WEAKNAMES_SECTNAME,
	DEBUG_FUNCNAMES_SECTNAME,
     // we don't use debug_frame in IA-64.
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
        // Bug 1516 - do not emit .debug_* sections if not -g
	if (Debug_Level < 1 &&
	strncmp(name, ".debug_", 7) == 0
	)
	  return FALSE;
        return TRUE;
}

