/*
 *  This material contains proprietary software of Entropic Speech, Inc.
 *  Any reproduction, distribution, or publication without the the prior
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing
 *  by Entropic Speech, Inc. must bear the notice:
 *
 *     "Copyright (c) 1987 Entropic Speech, Inc.; all rights reserved"
 *
 *  Program: getdct
 *
 *  Written by: Jim Elliott
 *
 *  Computes Nx1 discrete cosine transform (forward or inverse).
 */

#ifndef lint
static char *sccs_id = "@(#)getdct.c	1.3	10/11/89 ESI";
#endif

/*
 *  System include files.
 */

#include <math.h>
#include <stdio.h>

/*
 *  SPS include files.
 */

#include <esps/esps.h>
#include <esps/constants.h>

/*
 *  Defines.
 */

#define MAX_SIZE 256

/*
 ********************************************************************************
 *  Routine computes DCT using matrix multiplication for N = 1, 2, ..., MAX_SIZE.
 *  Forward and inverse operations are indicated by positive and negative values
 *  of N, respectively. If |N| = 1, the first element of the input vector is copied
 *  to the output vector; if N = 0, or |N| > MAX_SIZE, the routine is aborted.
 ********************************************************************************
 */

get_dct( in, out, N )

float in[], out[];
int N;
{
    int n;

    n = N;
    if ( n < 0 )
        n = -n;
	
    spsassert ( n > 0 && n <= MAX_SIZE, "invalid transform size" );

    if ( N > 0 )
        (void)dct( in, out, N );
    else
        (void)idct( in, out, -N );
}

/*
 ********************************************************************************
 *  Routine computes Nx1 forward DCT.
 ********************************************************************************
 */

dct( x, X, N )

int N;
float x[], X[];
{
    float arg;
    int k, m;

    if ( N == 1 )
    {
	X[0] = x[0];
	return;
    }

    for ( k = 0; k < N; k++ )
    {
	X[k] = 0;

	for ( m = 0; m < N; m++ )
	{
	    arg = 2.0 * (float) m;
	    arg += 1.0;
	    arg *= (float) k * PI;
	    arg /= 2.0 * (float) N;

	    X[k] += x[m] * cos( arg );
	}

	if ( k == 0 )
	    X[k] /= sqrt( 2.0 );

	X[k] *= 2.0 / N;
    }
}

/*
 ********************************************************************************
 *  Routine computes Nx1 inverse DCT.
 ********************************************************************************
 */

idct( X, x, N )

short N;
float x[], X[];
{
    float arg, c;
    int k, m;

    if ( N == 1 )
    {
	x[0] = X[0];
	return;
    }

    for ( m = 0; m < N; m++ )
    {
	x[m] = 0;

	for ( k = 0; k < N; k++ )
	{
	    arg = 2.0 * (float) m;
	    arg += 1.0;
	    arg *= (float) k * PI;
	    arg /= 2.0 * (float) N;

	    c = 1.0;

	    if ( k == 0 )
	        c /= sqrt( 2.0 );
	        
	    x[m] += c * X[k] * cos( arg );
	}
    }
}
