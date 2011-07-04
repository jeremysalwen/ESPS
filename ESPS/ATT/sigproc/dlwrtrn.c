/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/
static char *sccs_id = "@(#)dlwrtrn.c	1.2	9/26/95	ATT/ERL";

static double *pxl,*pa,*py,*pyl,*ps,*pa1,*px;
dlwrtrn(a,n,x,y)
double *a,*x,*y; int *n;
/*	routine to solve ax=y with cholesky 
	a - nxn matrix
	x,y -vectors
					*/
{
	double sm;
	*x = *y / *a;
	pxl = x + 1;
	pyl = y + *n;
	pa = a + *n;
	for(py=y+1;py<pyl;py++,pxl++){
		sm = *py;
		pa1 = pa;
		for(px=x;px<pxl;px++)
			sm = sm - *(pa1++) * *px;
		pa += *n;
		*px = sm / *pa1;
	}
}
