/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/
static char *sccs_id = "@(#)dlpcref.c	1.2	9/26/95	ATT/ERL";

static double *pa1,*pa2,*pa3,*pa4,*pc;
dlpcref(a,s,c,n) double *a,*c,*s; int *n;{
/*	convert lpc to ref
	a - lpc
	c - ref
	s - scratch
	n - no of cofs
					*/
double ta1,rc;
pa2 = s + *n + 1;
pa3 = a;
for(pa1=s;pa1<pa2;pa1++,pa3++) *pa1 = *pa3;
pc = c + *n -1; 
for(pa1=s+*n;pa1>s;pa1--,pc--)
	{
	*pc = *pa1;
	rc = 1. - *pc * *pc;
	pa2 = s + (pa1-s)/2;
	pa4 = pa1-1;
	for(pa3=s+1;pa3<=pa2;pa3++,pa4--)
		{
		ta1 = (*pa3 - *pc * *pa4)/rc;
		*pa4 = (*pa4 - *pc * *pa3)/rc;
		*pa3 = ta1;
		}
	}
}
