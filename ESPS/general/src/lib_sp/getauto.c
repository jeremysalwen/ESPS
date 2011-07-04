/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	
*/
#ifndef lint
static char *sccs_id = "@(#)getauto.c	1.6 2/20/96 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>

void
get_auto(x, lnt, r, order)
float x[];
double r[];
int lnt,order;
/*
Compute autocorrelation coefficients 
Input:
	x --- input data vector of length "lnt",
	order --- No. of lags for which auto-correlation is to be computed.
Output:
	r[0] --- Signal Energy
	r[1], ..., r[order] --- Normalized auto-correlation values
*/
 {
	int j,i;
	double temp;
	
#ifdef UE
	spsassert(x != NULL, "get_auto: x is NULL");
	spsassert(r != NULL, "get_auto: r is NULL");
#endif
        temp = 0.0;
	for (i=0; i < lnt; i++) temp += x[i]*x[i];
	r[0] = temp;

	if( r[0] == 0.){    /* No energy: fake autocorrelation of white noise*/
	  for(j=1; j<=order; j++) r[j] = 0.;
	  return;
	}
	for (j = 1; j <= order; j++)
	{
		temp = 0.0;
		for (i = 0; i < lnt-j; i++) temp += x[i]*x[i + j];
		r[j] = temp/r[0];
	}

 }
