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

#ifndef prp_driver_INCLUDED
#define prp_driver_INCLUDED
/* Linux assumes weak declaration followed by definition and reference will
 * create a weak symbol which won't cause an error at link time; Mach-O
 * creates an undefined symbol which causes link to fail. */
#define Prp_Process_Command_Line(a, b, c, d) \
  assert(!"Prp_Process_Command_Line")
#define Prp_Needs_Whirl2c() 0
#define Prp_Needs_Whirl2f() 0
#define Prp_Init() assert(!"Prp_Init")
#define Prp_Instrument_And_EmitSrc(a) assert(!"Prp_Instrument_And_EmitSrc")
#define Prp_Fini() assert(!"Prp_Fini")

#endif /* prp_driver_INCLUDED */
