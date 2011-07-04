/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/
static char *sccs_id = "@(#)dlpcwtd.c	1.2	9/26/95	ATT/ERL";

#include <stdio.h>

double *ps,*psl,*pp,*ppl,*pc,*pcl,*pph1,*pph2,*pph3,*pphl;

dlpcwtd(s,ls,p,np,c,phi,shi,xl,w)
double *s,*p,*c,*phi,*shi,*xl,*w;
int *ls,*np;
/*	pred anal subroutine with ridge reg
	s - speech
	ls - length of s
	p - pred coefs
	np - polyn order
	c - ref coers
	phi - cov matrix
	shi - cov vect
					*/
{
  int m,np1,mm;
  double d,pre,pre3,pre2,pre0,pss,pss7,thres;
  double ee;
  np1  =  *np  +  1;
  dcwmtrx(s,np,ls,np,phi,shi,&pss,w);
  if(*xl>=1.0e-4)
    {
      pph1 = phi; ppl = p + *np;
      for(pp=p;pp<ppl;pp++){
	*pp = *pph1;
	pph1 += np1;
      }
      *ppl = pss;
      pss7 = .0000001 * pss;
      mm = dchlsky(phi,np,c,&d);
      if(mm< *np)fprintf(stderr,"LPCHFA error covariance matric rank %d \n",mm);
      dlwrtrn(phi,np,c,shi);
      ee = pss;
      thres = 0.;
      pph1 = phi; pcl = c + mm;
      for(pc=c;pc<pcl;pc++)
	{
	  if(*pph1<thres)break;
	  ee = ee - *pc * *pc;
	  if(ee<thres)break;
	  if(ee<pss7)
	    fprintf(stderr,"LPCHFA is losing accuracy\n");
	}
      m = pc - c;
      if(m != mm)
	fprintf(stderr,"*W* LPCHFA error - inconsistent value of m %d \n",m);
      pre = ee * *xl;
      pphl = phi + *np * *np;
      for(pph1=phi+1;pph1<pphl;pph1+=np1)
	{
	  pph2 = pph1;
	  for(pph3=pph1+ *np-1;pph3<pphl;pph3+= *np)
	    {
	      *pph3 = *(pph2++);
	    }
	}
      pp = p; pre3 = .375 * pre; pre2 = .25 * pre; pre0 = .0625 * pre;
      for(pph1=phi;pph1<pphl;pph1+=np1)
	{
	  *pph1 = *(pp++) + pre3;
	  if((pph2=pph1- *np)>phi)
	    *pph2 = *(pph1-1) = *pph2 - pre2;
	  if((pph3=pph2- *np)>phi)
	    *pph3 = *(pph1-2) = *pph3 + pre0;
	}
      *shi -= pre2;
      *(shi+1) += pre0;
      *(p+ *np) = pss + pre3;
    }
  m = dcovlpc(phi,shi,p,np,c);
  return(m);
}
