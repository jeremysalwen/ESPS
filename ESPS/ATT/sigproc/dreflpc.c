/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/
static char *sccs_id = "@(#)dreflpc.c	1.2	9/26/95	ATT/ERL";

static double *pa1,*pa2,*pa3,*pa4,*pa5,*pc;
dreflpc(c,a,n) double *c,*a; int *n;{
double ta1;
/*	convert ref to lpc
	c - ref
	a - polyn
	n - no of coef
					*/
*a = 1.;
*(a+1) = *c;
pc = c; pa2 = a+ *n;
for(pa1=a+2;pa1<=pa2;pa1++)
	{
	pc++;
	*pa1 = *pc;
	pa5 = a + (pa1-a)/2;
	pa4 = pa1 - 1;
	for(pa3=a+1;pa3<=pa5;pa3++,pa4--)
		{
		ta1 = *pa3 + *pc * *pa4;
		*pa4 = *pa4 + *pa3 * *pc;
		*pa3 = ta1;
		}
	}
}
