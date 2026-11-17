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


/* =======================================================================
 * =======================================================================
 *
 *  Module: c_q_rel.c
 *  $Revision$
 *  $Date$
 *  $Author$
 *  $Source$
 *
 * =======================================================================
 * =======================================================================
 */

static char *source_file = __FILE__;
static char *rcs_id = "$Source$ $Revision$";


#include "defs.h"
#include "quad.h"

extern INT c_q_lt(QUAD, QUAD, INT *);
#pragma weak c_q_lt = __c_q_lt
#define	c_q_lt __c_q_lt

	/* implements the relational operator < for long doubles */

INT
c_q_lt(QUAD x, QUAD y, INT *p_err )
{
	*p_err = 0;

	if ( x.hi < y.hi )
		return ( 1 );

	if ( (x.hi == y.hi) && (x.lo < y.lo) )
		return ( 1 );

	return ( 0 );
}

extern INT c_q_le(QUAD, QUAD, INT *);
#pragma weak c_q_le = __c_q_le
#define	c_q_le __c_q_le

	/* implements the relational operator <= for long doubles */

INT
c_q_le(QUAD x, QUAD y, INT *p_err )
{
	*p_err = 0;

	if ( x.hi < y.hi )
		return ( 1 );

	if ( (x.hi == y.hi) && (x.lo <= y.lo) )
		return ( 1 );

	return ( 0 );
}

extern INT c_q_eq(QUAD, QUAD, INT *);
#pragma weak c_q_eq = __c_q_eq
#define	c_q_eq __c_q_eq

	/* implements the relational operator == for long doubles */

INT
c_q_eq(QUAD x, QUAD y, INT *p_err )
{
	*p_err = 0;

	if ( (x.hi == y.hi) && (x.lo == y.lo) )
		return ( 1 );

	return ( 0 );
}

extern INT c_q_ne(QUAD, QUAD, INT *);
#pragma weak c_q_ne = __c_q_ne
#define	c_q_ne __c_q_ne

	/* implements the relational operator != for long doubles */

INT
c_q_ne(QUAD x, QUAD y, INT *p_err )
{
	*p_err = 0;

	if ( (x.hi != y.hi) || (x.lo != y.lo) )
		return ( 1 );

	return ( 0 );
}

extern INT c_q_gt(QUAD, QUAD, INT *);
#pragma weak c_q_gt = __c_q_gt
#define	c_q_gt __c_q_gt

	/* implements the relational operator > for long doubles */

INT
c_q_gt(QUAD x, QUAD y, INT *p_err )
{
	*p_err = 0;

	if ( x.hi > y.hi )
		return ( 1 );

	if ( (x.hi == y.hi) && (x.lo > y.lo) )
		return ( 1 );

	return ( 0 );
}

extern INT c_q_ge(QUAD, QUAD, INT *);
#pragma weak c_q_ge = __c_q_ge
#define	c_q_ge __c_q_ge

	/* implements the relational operator >= for long doubles */

INT
c_q_ge(QUAD x, QUAD y, INT *p_err )
{
	*p_err = 0;

	if ( x.hi > y.hi )
		return ( 1 );

	if ( (x.hi == y.hi) && (x.lo >= y.lo) )
		return ( 1 );

	return ( 0 );
}

