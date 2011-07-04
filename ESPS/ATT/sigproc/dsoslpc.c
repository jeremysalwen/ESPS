/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/
static char *sccs_id = "@(#)dsoslpc.c	1.2	9/26/95	ATT/ERL";

static struct comp{double c_r,c_i;} *ps1,*ps2;
static double *pa1,*pa2;
dsoslpc(sos,a,nsos) struct comp *sos; double *a; int *nsos;{
/*	subroutine to convert sos to polyn coefs
	sos complex sos
	nsos no of sos
	a polyn coefs
	it returns polyn coef no
					*/
ps2 = sos + *nsos;
pa1 = a + 1; *a = 1.;
for(ps1=sos;ps1<ps2;ps1++)
	{
	*pa1 = *(pa1+1) = 0.;
	for(pa2=pa1-1;pa2>=a;pa2--)
		{
		*(pa2+1) += *pa2 * ps1->c_r;
		*(pa2+2) += *pa2 * ps1->c_i;
		}
	pa1++;
	if(ps1->c_i)pa1++;
	}
return(pa1-a-1);
}
