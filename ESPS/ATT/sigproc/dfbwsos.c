/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/
static char *sccs_id = "@(#)dfbwsos.c	1.2	9/26/95	ATT/ERL";

static double *pf1,*pf2,*pb;
static struct comp {double c_r,c_i;} *ps;
dfbwsos(f,bw,nform,sos,samprt) double *f,*bw,*samprt; int *nform; struct comp *sos;{
double cos(),exp(),q;
/*	subroutine to convert freqq and bw to sos	*/
ps = sos; pb = bw;
pf2 = f + *nform;
for(pf1=f;pf1<pf2;pf1++,pb++,ps++)
	{
	q = exp(-3.14159265 * *pb / *samprt);
	ps->c_r = -2. * q * cos(6.2831853 * *pf1 / *samprt);
	ps->c_i = q * q;
	}
}
