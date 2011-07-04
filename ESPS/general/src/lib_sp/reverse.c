/********************************************************
*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
*
*  Module Name: reverse.c
*
*  Written By:   Brian Sublett   7/18/86
*
*  Checked by:   Alan Parker
*
*  Purpose:  These two functions reverse the byte order
*	     of long and short integers.
*
*  Entry points:  long long_reverse();
*		  short short_reverse ();
*
*********************************************************/

# ifndef lint
	static char *sccs_id = "@(#)reverse.c	1.3 11/19/87 ESI";
# endif


/* Reverse the bytes for longs. */

long long_reverse (in)
long in;
    {
    union l_union {
	long lval;
	char lcval[sizeof(long)]; };
    union l_union tmp1, tmp2;
    register int i, sizel;
    sizel = sizeof (long);
    tmp1.lval = in;
    for (i=0; i < sizel; i++)
	tmp2.lcval[i] = tmp1.lcval[sizel - i - 1];
    return (tmp2.lval);
    }

/* Reverse the bytes for shorts. */

short short_reverse (in)
short in;
    {
    union s_union {
	short sval;
	char scval[sizeof(short)]; };
    union s_union tmp1, tmp2;
    tmp1.sval = in;
    tmp2.scval[0] = tmp1.scval[1];
    tmp2.scval[1] = tmp1.scval[0];
    return (tmp2.sval);
    }
