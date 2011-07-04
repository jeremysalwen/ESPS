/*	
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987, 1991 Entropic Speech, Inc.; All rights reserved"
 	
			
  blt: finds the bilinear transform of an rray of floats. 
  Author: Bill Byrne
  Date:  2/4/1991
*/
#ifndef lint
static      char *sccs_id = "@(#)blt.c	1.5         1/22/93 ESI";
#endif

#include <math.h>
#include <stdio.h>
#include <esps/esps.h>

void stage1_d();
void stage2_d();
void stageN_d();

void blt( seq, dim, param)
int dim;
float *seq, param;
{
    int i, endptr;
    static double *inptr=NULL, *outptr=NULL;
    double *holdptr;
    static int ldim=0;

    spsassert( seq != NULL, "blt called with null pointer to sequence.");
    spsassert( dim > 0, "blt: called with sequence length 0");
    spsassert( param < 1.0 && param > -1.0 , 
	      "blt: called with parameter outside (-1,1).");

    if ( param == 0.0 ) 
	return;

    if (ldim != dim) {
	if (inptr != NULL) 
	    free(inptr);
	if (outptr != NULL) 
	    free(outptr);
	ldim = dim;
	inptr = (double *) calloc((unsigned)dim, sizeof(double));
	outptr = (double *) calloc((unsigned)dim, sizeof(double));
	spsassert( inptr != NULL && outptr != NULL, 
		  "blt: can't allocate memory.\n");
    }
    endptr = dim-1;
    
    /* reverse */
    for (i=0; i<=endptr; i++)
	inptr[endptr-i] = seq[i];

    stage1_d(inptr, outptr, dim, param);
    seq[0] = outptr[endptr];

    holdptr = inptr;
    inptr = outptr;
    outptr = holdptr;

    stage2_d(inptr, outptr, dim, param);
    seq[1] = outptr[endptr];

    for (i=2; i<dim; i++) {
	holdptr = inptr;
	inptr = outptr;
	outptr = holdptr;
	stageN_d(inptr, outptr, dim, param);
	seq[i] = outptr[endptr];
    }
    
    return;
}


void stage1_d(in, out, dim, param)
int dim;
double *in, *out, param;
{
    int i;

    out[0] = in[0];
    for (i=1;i<dim; i++) 
	out[i] = in[i] + param * out[i-1];

    return;
}

void stage2_d(in, out, dim, param)
int dim;
double *in, *out, param;
{
    int i;
    float s;

    s = 1.0 - param*param;

    out[0] = 0;
    for (i=1; i<dim; i++)
	out[i] = param * out[i-1] + s * in[i-1];

    return;
}

void stageN_d(in, out, dim, param)
int dim;
double *in, *out, param;
{
    int i;

    out[0] = -1.0 * param * in[0];
    for (i=1; i<dim; i++)
	out[i] = param * (out[i-1] - in[i]) + in[i-1];

    return;
}


