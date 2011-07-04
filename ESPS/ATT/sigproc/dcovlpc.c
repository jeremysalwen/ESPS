/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/
static char *sccs_id = "@(#)dcovlpc.c	1.2	9/26/95	ATT/ERL";

#include <stdio.h>
static double *pp,*ppl,*pa;
dcovlpc(p,s,a,n,c)
double *p,*s,*a,*c;
int *n;
/*	solve p*a=s using stabilized covariance method
	p - cov nxn matrix
	s - corrvec
	a lpc coef *a = 1.
	c - ref coefs
				*/
{
  double ee;
  double sqrt(),ps,ps1,thres,d;
  int m,n1;
  m = dchlsky(p,n,c,&d);
  dlwrtrn(p,n,c,s);
  thres = 1.0e-31;
  n1 = *n + 1;
  ps = *(a + *n);
  ps1 = 1.e-8*ps;
  ppl = p + *n * m;
  m = 0;
  for(pp=p;pp<ppl;pp+=n1){
    if(*pp<thres)break;
    m++;
  }
  ee = ps;
  ppl = c + m; pa = a;
  for(pp=c;pp<ppl;pp++){
    ee = ee - *pp * *pp;
    if(ee<thres)break;
    if(ee<ps1)fprintf(stderr,"*w* covlpc is losing accuracy\n");
    *(pa++) = sqrt(ee);
  }
  m = pa - a;
  *c = - *c/sqrt(ps);
  ppl = c + m; pa = a;
  for(pp=c+1;pp<ppl;pp++)
    *pp = - *pp / *(pa++);
  dreflpc(c,a,&m);
  ppl = a + *n;
  for(pp=a+m+1;pp<=ppl;pp++)*pp=0.;
  return(m);
}
