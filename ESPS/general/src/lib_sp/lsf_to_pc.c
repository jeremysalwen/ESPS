/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

   This module knows how to compute prediction filter coefficients from 
line spectral frequencies. The parameters have the following meanings:

   "lsf" is the floating array that contains the line spectral frequencies
   "pc" is a real array that returns the linear prediction coefficients
   "order" is an integer that is equal to the analysis filter order
   "bandwidth" is equal to 1/2 the sampling frequency

Advantage of integer truncation during division is taken when the "order"
is odd.

Written by David Burton
Checked by Alan Parker
*/


#ifndef lint
	static char *sccsid = "@(#)lsf_to_pc.c	1.6  2/25/88 ESI";
#endif

/*system include files*/

#include <stdio.h>
#include <math.h>
#include <esps/unix.h>
#include <esps/esps.h>

/* local defines*/

#define ERROR_EXIT(text) {(void) fprintf(stderr, "lsf_to_pc: %s - exiting\n",text); return(-1);}


int
lsf_to_pc(lsf, pc, order, bandwidth)
float pc[], bandwidth, lsf[];
int  order;
{
	float *diff_freq, *sum_freq;
	float *diff_a; /*temporary storage for computing 
				pc polynomial */
	float *sum_b; /*temporary storage for computing
				pc polynomial */
	float quadratic[3], pi, ts;
	int i, current_order = 2, odd = 0;
	float *diff_gain, *sum_gain;
	float *tmp;	/*temporary storage for computing pc
				   polynomial*/	
	double acos(), cos();

	int debug_level = 0; /* for initial debug purposes only*/

/*
 * Check order;  is it even or odd
*/
	if((int)(order/2)*2 != order)
	    odd = 1;
	if(debug_level > 0)
	    (void)fprintf(stderr, "lsf_to_pc: order is %d,  odd is %d\n", 
				order,  odd);
/*
 * Allocate memory for arrays
*/
	diff_freq = (float *)calloc((unsigned)(order/2 + 1), sizeof(float));
spsassert(diff_freq != NULL,"Can't calloc space for diff_freq");
	sum_freq = (float *)calloc((unsigned)(order/2 + 1), sizeof(float));
spsassert(sum_freq != NULL,"Can't calloc space for sum_freq");
	diff_a = (float *)calloc((unsigned)(order+3), sizeof(float));
spsassert(diff_a != NULL,"Can't calloc space for diff_a");
	sum_b = (float *)calloc((unsigned)(order+3), sizeof(float));
spsassert(sum_b != NULL,"Can't calloc space for sum_b");
	tmp = (float *)calloc((unsigned)(order+3), sizeof(float));
spsassert(tmp != NULL,"Can't calloc space for tmp");
	diff_gain = (float *)calloc((unsigned)(order/2 + 1), sizeof(float));
spsassert(diff_gain != NULL,"Can't calloc space for diff_gain");
	sum_gain = (float *)calloc((unsigned)(order/2 + 1), sizeof(float));
spsassert(sum_gain != NULL,"Can't calloc space for sum_gain");

/* separate  difference frequencies from sum frequencies */

	for(i=0; i< order/2; i++) {
	    sum_freq[i] = lsf[2*i];
	    diff_freq[i] = lsf[2*i + 1];
	}
	
	if(odd)
	    sum_freq[order/2] = lsf[order-1];

	if(debug_level>0){
          for(i=0; i<order/2;i++){
	    (void)fprintf(stderr, "sum_freq[%d] = %f\n", i, sum_freq[i]);
	    (void)fprintf(stderr, "diff_freq[%d] = %f\n", i, diff_freq[i]);
	  }
	  if(odd)
	    (void)fprintf(stderr, "sum_freq[%d]= %f\n", order/2,
	    sum_freq[order/2]);
	}
/* compute difference and sum filter stage gains */
/* compute constants needed */
	ts = (float)(1. / (2. * bandwidth));
	pi = (float)(2. * acos((double)0.));



 /* first difference gains */

	for(i=0; i<order/2; i++) {
	    diff_gain[i] = (float)(-2. * 
		        cos ((double)(2. * pi * diff_freq[i] * ts)));
	}

 /* now sum gains */

	for(i=0; i<order/2; i++) {
	    sum_gain[i] = (float) (-2. * 
				cos ((double)(2. * pi * sum_freq[i] * ts)));
	}
	
	if(odd)
    	    sum_gain[order/2] = (float) (-2. * 
			cos ((double)(2. * pi * sum_freq[order/2] * ts)));

	if(debug_level>0){
	    for(i=0;i<order/2;i++){
		(void)fprintf(stderr, "sum_gain[%d] = %f\n", i,
		sum_gain[i]);
		(void)fprintf(stderr, "diff_gain[%d] = %f\n", i,
		diff_gain[i]);
	    }
	    if(odd)
		(void)fprintf(stderr, "sum_gain[%d] = %f\n", order/2,
		sum_gain[order/2]);
	}

/* Do polynomial multiplication */
/* first do difference frequency terms*/
    /* initialize first and last term of quadratic*/
	quadratic[0] = 1.;
	quadratic[2] = 1.;
   /* fill in first three terms of product polynomial */
	diff_a[0] = 1.;
	diff_a[1] = diff_gain[0];
	diff_a[2] = 1.;

	for (i=1; i<order/2;i++) {
	   quadratic[1] = diff_gain[i]; /* fill in middle quadratic term*/
	(void)mult_poly(quadratic, diff_a, current_order);
	current_order += 2;
	}

/* if order is even multiply by 1 - 1/z;
   if order is odd multiply by 1 - (1/z)(1/z)*/

    if(odd){
	quadratic[2] = -1.;
	quadratic[1] = 0.;
	(void)mult_poly(quadratic, diff_a, current_order);
    }
    else{
	quadratic[1] = -1.;
	quadratic[2] = 0.;
	(void)mult_poly(quadratic, diff_a, current_order);
    }

    if(debug_level>0){
	for(i=0;i<order+2;i++)
	    (void)fprintf(stderr, "diff_filter[%d] = %f\n", i, diff_a[i]);
    }

/* do sum_freq terms*/

    /* initialize first and last term of quadratic*/
	current_order = 2;
	quadratic[0] = 1.;
	quadratic[2] = 1.;
   /* fill in first three terms of product polynomial */
	sum_b[0] = 1.;
	sum_b[1] = sum_gain[0];
	sum_b[2] = 1.;
	for (i=1; i<order/2;i++) {
	   quadratic[1] = sum_gain[i];
	(void)mult_poly(quadratic, sum_b, current_order);
	current_order += 2;
	}

/* if order is even now multiply by 1  + 1/z;
   if order is odd multiply by the last LSF quadratic */

    if(odd){
	quadratic[0] = 1.;
	quadratic[2] = 1.;
	quadratic[1] = sum_gain[(order-1)/2];
	(void)mult_poly(quadratic, sum_b, current_order);
    }
    else{
	quadratic[1] = 1.;
	quadratic[2] = 0.;
	(void)mult_poly(quadratic, sum_b, current_order);
    }


    if(debug_level>0){
	for(i=0;i<order+2;i++)
	    (void)fprintf(stderr, "sum_filter[%d] = %f\n", i, sum_b[i]);
    }

/* now add the two parts and divide by 2 */

	for(i=0;i<order+2;i++)
	   	tmp[i] = .5*(diff_a[i] + sum_b[i]);
/* change sign of coefficients 1 through order and assign to pc*/
	for(i=1;i<order+1;i++){
	     pc[i-1] = -tmp[i];
}

/*
 * Clean up
*/
    free((char *)diff_freq);
    free((char *)sum_freq);
    free((char *)diff_a);
    free((char *)sum_b);
    free((char *)tmp);
    free((char *)diff_gain);
    free((char *)sum_gain); 
/* all done */
	return(0);
}

int
mult_poly(quadratic, full, order)
	int order;
	float quadratic[], full[];

{
	static int cnt = 0;
	int i, j;
	float *multiplier, *res_tmp, *tmp;

/*
 * Allocate memory
*/
	multiplier = (float *)calloc((unsigned)(order+6), sizeof(float));
spsassert(multiplier != NULL,"Couldn't allocate space for multiplier");
	res_tmp = (float *)calloc((unsigned)(order+3), sizeof(float));
spsassert(res_tmp != NULL,"Couldn't allocate space for res_tmp");
	tmp = (float *)calloc((unsigned)(order+5), sizeof(float));
spsassert(tmp != NULL,"Couldn't allocate space for tmp");
/* zero arrays */

	for(i=0;i<order+6;i++)
	    multiplier[i] = 0;

	for(i=0;i<order+5;i++)
	     tmp[i] = 0.;

	for(i=0;i<order+3;i++)
	     res_tmp[i] = 0.;

/* put input data into temporary array */

	for (i=0;i<=order;i++){
	    tmp[i+2] = full[i];
	}


/* reverse quadratic term and store at the end of a temporary array*/
	for(i=0; i<3; i++){
	     multiplier[order+5-i] = quadratic[i];
	}


/* now multiply tmp[] times multiplier[]*/

	for(i=0;i < order+3;i++) {
	/*shift the contents of multiplier down one*/
	    multiplier[order+2-i] = multiplier[order+3-i];
	    multiplier[order+3-i] = multiplier[order+4-i];
	    multiplier[order+4-i] = multiplier[order+5-i];
	    multiplier[order+5-i] = 0.;
	    for(j=0;j < order+5;j++) {
		res_tmp[order+2-i] += tmp[j]*multiplier[j];
	    }
	}


/* copy res_tmp[] into the return array full[] */

	for(i=0;i < order+3;i++){
	   full[i] = res_tmp[i];
	}

/*
 * clean up
*/
      free((char *)multiplier);
      free((char *)tmp);
      free((char *)res_tmp); 
    return(0);
}
