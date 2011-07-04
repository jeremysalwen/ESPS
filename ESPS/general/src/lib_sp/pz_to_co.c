/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 *
 *  Module Name: pz_to_co.c
 *
 *  Written By:   Brian Sublett   1-30-87
 *  Checked by:   Alan Parker
 *                pz_to_co_d added by Bill Byrne 7-2-91
 *
 *
 *********************************************************/

# ifndef lint
	static char *sccs_id = "@(#)pz_to_co.c	1.6 10/19/93 ESI";
# endif

#include <stdio.h>
#include <esps/esps.h>

#ifndef DEC_ALPHA
    char *calloc();
#endif

int pz_to_co (nroots, real, imag, pnco, co)
int nroots;
double *real, *imag;
short *pnco;
float *co;
    {
    int i, nnew, nco = 1;
    double *co1, *co2, *temp, next[3];

    co1 = (double*) calloc ((unsigned)(nroots*2+1), sizeof (double));
    co2 = (double*) calloc ((unsigned)(nroots*2+1), sizeof (double));

    next [0] = 1.0;
    co1 [0] = 1.0;
    for (i=0; i < nroots; i++)
	{
	if (imag [i] == 0)
	    {
	    nnew = 2;
	    next [1] = -real [i];
	    }
	else
	    {
	    nnew = 3;
	    next[1] = -2*real[i];
	    next[2] = real[i]*real[i] + imag[i]*imag[i];
	    }
	(void)convolv (co1, nco, next, nnew, co2, &nco);
	temp = co1;
	co1 = co2;
	co2 = temp;
	
        }
    *pnco = nco;
    for (i=0; i < *pnco; i++)
	{
	co[i] = co1[i];
	}

    (void) free( co1 );
    (void) free( co2 );
    }


int pz_to_co_d (nroots, real, imag, pnco, co)
int nroots;
double *real, *imag;
short *pnco;
double *co;
{
    int i, nnew, nco = 1;
    double *co1, *co2, *temp, next[3];

    co1 = (double*) calloc ((unsigned)(nroots*2+1), sizeof (double));
    co2 = (double*) calloc ((unsigned)(nroots*2+1), sizeof (double));

    next [0] = 1.0;
    co1 [0] = 1.0;
    for (i=0; i < nroots; i++)	{
	if (imag [i] == 0) {
	    nnew = 2;
	    next [1] = -real [i];
	} else {
	    nnew = 3;
	    next[1] = -2*real[i];
	    next[2] = real[i]*real[i] + imag[i]*imag[i];
	}
	(void)convolv (co1, nco, next, nnew, co2, &nco);
	temp = co1;
	co1 = co2;
	co2 = temp;
	
    }
    *pnco = nco;
    for (i=0; i < *pnco; i++) {
	co[i] = co1[i];
    }

    (void) free( co1 );
    (void) free( co2 );
}

