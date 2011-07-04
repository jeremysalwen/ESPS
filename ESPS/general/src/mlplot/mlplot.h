/*
 * This material contains proprietary software of Entropic Speech,
 * Inc.  Any reproduction, distribution, or publication without the
 * prior written permission of Entropic Speech, Inc. is strictly
 * prohibited.  Any public distribution of copies of this work
 * authorized in writing by Entropic Speech, Inc. must bear the
 * notice
 *
 * "Copyright (c) 1986, 1987 Entropic Speech, Inc.  All rights reserved."
 *
 *
 * mlplot.h - include file for mlplot(1-ESPS)
 * @(#)mlplot.h	3.3	1/21/88	ESI
 */

#define DRAW(u1,v1,u2,v2, plotline) { \
	long U[2], V[2]; \
	U[0] = (u1); U[1] = (u2); V[0] = (v1); V[1] = (v2); \
	(*plotline)((long) 2, U, V);}
