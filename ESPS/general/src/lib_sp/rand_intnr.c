/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *       "Copyright 1986 Entropic Speech, Inc."; All Rights Reserved
 *
 * Program:	rand_int_nrep.c
 *
 * Written by:  John Shore
 * Checked by:  Alan Parker
 *
 * This routine provides random sampling without replacement for
 * integers uniformly distributed between zero and max_int.  
 */

#ifndef lint
static char *sccs_id = "@(#)rand_intnr.c	1.5	1/4/96 ESI";
#endif
/*
 * defines
 */
#define BIGRAND 2147483647.0	/*maximum value returned by random()*/

#include <stdio.h>
#include <esps/esps.h>
/*
 * system functions and variables
 */
#ifndef DEC_ALPHA
    char *calloc();
long random();
void free();
#endif

/*
 * SPS functions
 */
long rand_int();
/*
 * main program
 */
long
rand_intnr(max_int, reset)
/*
 * This routine provides random sampling without replacement for
 * integers uniformly distributed between zero and max_int.  
 */
long max_int;
int reset;
{
    static int veryfirst = 1;	/*flag for initial call to routine*/
    static long *seen;	    /*array for values already seen*/
    static long callcount = 0;	/*number of previous calls during no replace*/
    register long i, newval, needanother = 1;
    /*first check for reset -- "replace" all the integers*/
    if (reset || veryfirst || (callcount > max_int)) {
	if (!veryfirst) free((char *)seen); else veryfirst = 0;
	seen = (long *)calloc((unsigned)(max_int + 1), (unsigned) sizeof(long));
	spsassert(seen != NULL, "rand_intnr: calloc failed!");
	callcount = 0;
    }
    /*now get a sample; first see if there's only one possible answer left 
      and if so find it*/
    if (callcount == max_int)
	for (i = 0; i < max_int; i++) 
	    if (!seen[i])  {callcount++; return i;}
    /*more than one possible answer, so generate another one*/
    while (needanother) {
	newval = rand_int(max_int);
	if (!seen[newval]) needanother = 0;
    }
    seen[newval] = 1;
    callcount++;
    return newval;
}
