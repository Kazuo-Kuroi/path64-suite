/* libunwind - a platform-independent unwind library
   Copyright (C) 2008 CodeSourcery

This file is part of libunwind.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#include <stdlib.h>

#include "unwind_i.h"

#ifndef UNW_REMOTE_ONLY

/* Loads $2-$31 from UC's general registers, then $1 ($at) from its slot,
   and jumps to $1 (getcontext.S).  */
extern void _Umips_resume_context (ucontext_t *uc) NORETURN;

HIDDEN int
mips_local_resume (unw_addr_space_t as, unw_cursor_t *cursor, void *arg)
{
  struct cursor *c = (struct cursor *) cursor;
  ucontext_t *uc = arg;		/* local unwinding: as_arg is the context */
  mips_reg_t *gregs = (mips_reg_t *) &uc->uc_mcontext.gregs[0];

  /* $sp is not a DWARF-tracked location: the frame's stack pointer is
     its CFA. The resume address travels in the $at slot, since $at is
     never live across a jump into a landing pad.  */
  gregs[29] = (mips_reg_t) c->dwarf.cfa;
  gregs[1] = (mips_reg_t) c->dwarf.ip;

  Debug (8, "resuming at ip=%llx sp=%llx\n",
	 (unsigned long long) c->dwarf.ip, (unsigned long long) c->dwarf.cfa);
  _Umips_resume_context (uc);
  return -UNW_EINVAL;
}

/* Copy the cursor's view of the general registers into the context that
   mips_local_resume loads. Floating-point registers are not tracked by
   the MIPS port yet.  */
static int
establish_machine_state (struct cursor *c)
{
  int (*access_reg) (unw_addr_space_t, unw_regnum_t, unw_word_t *,
		     int write, void *);
  unw_addr_space_t as = c->dwarf.as;
  void *arg = c->dwarf.as_arg;
  unw_word_t val;
  int reg;

  access_reg = as->acc.access_reg;

  for (reg = UNW_MIPS_R0 + 2; reg <= UNW_MIPS_R31; ++reg)
    {
      if (reg == UNW_MIPS_R29)
	continue;			/* set from the CFA */
      if (tdep_access_reg (c, reg, &val, 0) >= 0)
	(*access_reg) (as, reg, &val, 1, arg);
    }
  return 0;
}

#endif /* !UNW_REMOTE_ONLY */

PROTECTED int
unw_resume (unw_cursor_t *cursor)
{
#ifndef UNW_REMOTE_ONLY
  struct cursor *c = (struct cursor *) cursor;
  int ret;

  Debug (1, "(cursor=%p)\n", c);

  if (c->dwarf.as->acc.resume == NULL)
    return -UNW_EINVAL;
  if ((ret = establish_machine_state (c)) < 0)
    return ret;

  return (*c->dwarf.as->acc.resume) (c->dwarf.as, (unw_cursor_t *) c,
				     c->dwarf.as_arg);
#else
  return -UNW_EINVAL;
#endif
}
