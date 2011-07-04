/*

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
   "Copyright (c) 1987, 1988 Entropic Speech, Inc.; All rights reserved"
 				

 	
*/

/***************************************************************************

Cepstrum consists of two modules that deal with cepstums and equivalent
linear predictive coeffcients. The two functions are

    afc2cep(afc, cep, order) - convert autoregressive filter coefficients 
			    to cepstrum coefficients

    cep2afc(cep, afc, order)- convert cepstrum coefficient to autoregressive 
			    filter coefficients

    afc - "order" autoregressive filter coefficients not including
	    the first coefficient, which is = -1.

    cep - "order" cepstrum coefficients not including the first 
	    one, which is = ln[residual power].

    order - the number of coefficients to transform. Also equal to
	    the analysis filter order.

 **************************************************************************/
#include <stdio.h>
#include <math.h>
#include <esps/esps.h>

#ifndef lint
static char *sccs_id = "@(#)cepstrum.c	1.7 1/26/88 ESI";
#endif


int
afc2cep(afc, cep, order)
int order;
float *afc, *cep;

{
    int k, n;
    float tmp;

/* do some error checking on the input
*/
#ifdef UE
    spsassert(cep != NULL, "afc2cep: cep is NULL");
    spsassert(afc != NULL, "afc2cep: afc is NULL");
    spsassert(order >= 0, "afc2cep: order is less than zero");
#endif

    /*compute cepstrum coefficients*/
    for(n=0; n<order; n++){
	tmp = 0.;
	for(k=0; k<=n-1; k++)
	    tmp += (n-k)*cep[n-k-1]*afc[k];

        cep[n] = afc[n] + tmp/(n+1);
    }
    return(0);
}

int
cep2afc(cep, afc, order)
int order;
float *cep, *afc;
{
    int j, k, l;
    float sum;

/* do some error checking on the input
*/
#ifdef UE
    spsassert(cep != NULL, "afc2cep: cep is NULL");
    spsassert(afc != NULL, "afc2cep: afc is NULL");
    spsassert(order >= 0, "afc2cep: order is less than zero");
#endif

    for(k=0; k<order; k++){
	sum = 0;
	for(j=1; j<=k; j++){
	    l = k-j;
	    sum += afc[j-1]*cep[l]*(l+1);
	}
	afc[k] = cep[k] - sum/(k+1);
    }
    
    return(0);
}

