/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

static char *sccs_id = "@(#)dchlsky.c	1.2	9/26/95	ATT/ERL";

double *pa1,*pa2,*pa3,*pa4,*pa5,*pal,*pt;
dchlsky(a,n,t,det)
double *a,*t,*det;
int *n;
/*	performs cholesky decomposition
	a - l * l(transpose)
	l - lower triangle
	det det(a)
	a - nxn matrix
	return - no of reduced elements
		results in lower half + diagonal
		upper half undisturbed.
					*/
{
	double sm;
	double sqrt();
	int m;
	*det = 1.;
	m = 0;
	pal = a + *n * *n;
	for(pa1=a;pa1<pal;pa1+= *n){
		pa3=pa1;
		pt = t;
		for(pa2=a;pa2<=pa1;pa2+= *n){
			sm = *pa3;	/*a(i,j)*/
			pa5 = pa2;
			for(pa4=pa1;pa4<pa3;pa4++)
				sm =  sm - *pa4 * *(pa5++);
			if(pa1==pa2){
				if(sm<=0.)return(m);
				*pt = sqrt(sm);
				*det = *det * *pt;
				*(pa3++) = *pt;
				m++;
				*pt = 1. / *pt;
				pt++;
			}
			else
				*(pa3++) = sm * *(pt++);
		}
	}
	return(m);
}
