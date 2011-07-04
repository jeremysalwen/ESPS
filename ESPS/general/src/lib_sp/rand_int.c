/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *       "Copyright 1986 Entropic Speech, Inc."; All Rights Reserved
 *
 * Program:	rand_int
 *
 * Written by:  John Shore
 * Checked by:  Alan Parker
 */

#ifndef lint
static char *sccs_id = "@(#)rand_int.c	1.3	11/19/87 ESI";
#endif
/*
 * defines
 */
#define LARGERAND 2147483646.0	/*-1 plus maximum value returned by random()*/
/*
 * system functions and variables
 */
long random();
/*
 * main program
 */
long
rand_int(max_int)
/*
 * rand_int() returns a random integer uniformly distributed between 
 * zero and max_int.  That is, max_int is the number of different
 * values (starting with zero) that rand_int may return.  
 */
long max_int;
{
    float fval;
    fval = (max_int + 1) * (random() / LARGERAND);
    return (long) fval;
}
