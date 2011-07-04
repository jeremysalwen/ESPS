/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/
static char *sccs_id = "@(#)dcwmtrx.c	1.2	9/26/95	ATT/ERL";

/* cov mat for wtd lpc	*/
static double *pdl1,*pdl2,*pdl3,*pdl4,*pdl5,*pdl6,*pdll;
dcwmtrx(s,ni,nl,np,phi,shi,ps,w)
double *s,*phi,*shi,*ps,*w; int *ni,*nl,*np;
{
	double sm;
	int i,j;
	*ps = 0;
	for(pdl1=s+*ni,pdl2=w,pdll=s+*nl;pdl1<pdll;pdl1++,pdl2++)
		*ps += *pdl1 * *pdl1 * *pdl2;

	for(pdl3=shi,pdl4=shi+*np,pdl5=s+*ni;pdl3<pdl4;pdl3++,pdl5--){
		*pdl3 = 0.;
		for(pdl1=s+*ni,pdl2=w,pdll=s+*nl,pdl6=pdl5-1;
			pdl1<pdll;pdl1++,pdl2++,pdl6++)
			*pdl3 += *pdl1 * *pdl6 * *pdl2;

	}

	for(i=0;i<*np;i++)
		for(j=0;j<=i;j++){
			sm = 0.;
			for(pdl1=s+*ni-i-1,pdl2=s+*ni-j-1,pdl3=w,pdll=s+*nl-i-1;
				pdl1<pdll;)
				sm += *pdl1++ * *pdl2++ * *pdl3++;

			*(phi + *np * i + j) = sm;
			*(phi + *np * j + i) = sm;
		}
}
