/* cross.c */

/*	Copyright (c) 1987, 1988, 1989 AT&T	*/
/*      Copyright (c) 1991-1994 Entropic Research Laboratory */
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*           and of Entgropic Research Laboratory               */
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

#ifndef lint
	static char *sccs_id = "@(#)cross.c	1.4 9/21/94		ATT, ESI ";
#endif
#include <stdio.h>
#include <math.h>


cross(data, size, start, nlags, engref, k1, maxloc, maxval, correl)
     short *data, *correl;
     int *maxloc;
     double *engref, *k1, *maxval;
     register int size, start, nlags;
{
  static float *ibuf=NULL, *obuf=NULL;
  static int nin=0, nout=0;
  register float *dp, *dq, *ds, sum;
  register short *p;
  register float engr, t, *dds, amax;
  register double engc;
  register int i, j, iloc;
  int sizei, sizeo;
  
  sizei = size + start + nlags + 1;
  sizeo = nlags + 1;
  
  if(nin < sizei) {
    if(ibuf) ibuf = (float*)realloc(ibuf,sizeof(float)*sizei);
    else  ibuf = (float*)malloc(sizeof(float)*sizei);
    nin = sizei;
    if(!ibuf) {
      printf("Can't allocate buffer memory in cross()\n");
      exit(-1);
    }
  }

  if(nout < sizeo) {
    if(obuf) obuf = (float*)realloc(obuf,sizeof(float)*sizeo);
    else  obuf = (float*)malloc(sizeof(float)*sizeo);
    nout = sizeo;
    if(!obuf) {
      printf("Can't allocate buffer memory in cross()\n");
      exit(-1);
    }
  }

  /* convert input data to floats */
  for(i=0, sum = 0.0, p=data, dp=ibuf; i++ < sizei; ) {
    *dp++ = t = *p++;
    sum += t;
  }
  sum /= sizei;

  for(i=0, dp=ibuf; i++ < sizei; ) {
    *dp++ -= sum;
  }
  
  /* compute energy in reference window */
  for(i=size, dp=ibuf, engr=0.0; i-- >0; ) {
    sum = *dp++;
    engr += sum * sum;
  }

  *engref = engr;

  /* compute 1st lag for k1 computerion */
  for(i=size-1, dq = ibuf, dp=ibuf+1, engc=0.0; i-- >0; )
    engc += *dq++ * *dp++;

  if(engr > 0.0) {
    *k1 = engc/engr;

    /* compute energy at first requested lag */  
    for(i=size, dp=ibuf+start, engc=0.0; i-- >0; ) {
      sum = *dp++;
      engc += sum * sum;
    }

    /* COMPUTE CORRELATIONS AT ALL REQUESTED LAGS */
    for(i=0, dq=obuf, amax=0.0, iloc = -1; i < nlags; i++) {
      for(j=size, sum=0.0, dp=ibuf, dds = ds = ibuf+i+start; j-- > 0; )
	sum += *dp++ * *ds++;

/*      *dq++ = t = (sum * 2.0/(engc + engr));
*/
      *dq++ = t = sum /sqrt(engc * engr);
      engc -= *dds * *dds;
      engc += *ds * *ds;
      if(t > amax) {
	amax = t;
	iloc = i+start;
      }
    }
    *maxloc = iloc;
    *maxval = amax;

    /* output normalized crosscorrelation as integers */
    for(p=correl, i=sizeo, dp=obuf; i-- > 0; )
      *p++ = .5 + (32767.0 * *dp++);
  } else {
    *k1 = 0.0;
    *maxloc = 0;
    *maxval = 0.0;
    for(p=correl,i=sizeo; i-- > 0; )
      *p++ = 0;
  }
}

