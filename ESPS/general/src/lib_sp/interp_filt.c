/********************************************************
*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
 				

 	
*
*  Module Name: interp_filt.c
*
*  Written By:   Brian Sublett   
*
*  Purpose:  This subroutine takes a data array, pointed to by x, and
*	     a zfunc pointed to by pzfunc, and filters the array,
*	     returning the result in another array pointed to by y.
*	     The output sampling rate is equal to the input sampling
*	     rate times (up/down).
*
*********************************************************/

# ifndef lint
	static char *sccs_id = "@(#)interp_filt.c	1.7 10/19/93 ESI";
# endif

# include <stdio.h>
# include <esps/esps.h>

#ifndef DEC_ALPHA
char *calloc();
#endif

interp_filt (n, x, y, pzfunc, init_state, up, down, poutflag, preturn)
int n, up, down, *poutflag, *preturn;
double *x, *y, *init_state;
struct zfunc *pzfunc;
{

    int i, j, m, nnsiz, ddsiz, eup, edown, uporder, outcount;
    double sum, state_sum, *px, *py;
    static double  *state;
    static int firstime = 1, order;

/* Allocate the array space on first call. */

    nnsiz = pzfunc->nsiz;
    ddsiz = pzfunc->dsiz;

    if (nnsiz > ddsiz) order = nnsiz;
    else order = ddsiz;

    uporder = up*order;

    if (firstime)
        {
        state = (double*) calloc ((unsigned)(up*n + uporder), sizeof (double));
	spsassert(state != NULL, "interp_filt: calloc failed!");

        firstime = 0;
        }

    if (pzfunc->dsiz > 0) eup = 1;
    else eup = up;

    edown = down;

/* Copy the previous state to the beginning of the state array. */

    for (i=0; i<uporder; i++)
 	{
        state [i] = init_state [i];
	}

/* Begin the filtering operation. */

    sum = 0;
    state_sum = 0;
    outcount = *poutflag;
    for (m=0,px=x,py=y,*preturn=0; m<up*n; m++)
        {
	if (!(m % eup))  /* Add a point to the state array. */
	    {
            for (j=1; j<ddsiz; j++)
                {
	        state_sum -= state[uporder + m-j]*pzfunc->poles[j];
	        }
	    if (!(m % up)) state_sum += *(px++);
	    if (ddsiz >= 1)
	        {
                if (pzfunc->poles[0] != 0) state_sum /= pzfunc->poles[0];
	        else
	            {
	            (void)fprintf(stderr,"interp_filt: poles[0]=0. Exiting.\n");
	            exit (1);
	            }
	        }
	    }
	state [uporder + m] = state_sum;
	if (!((m - *poutflag) % edown))    /* Compute an output point. */
	    {
            for (j=0; j<nnsiz; j++)
    	        {
	        sum += state[uporder + m-j]*pzfunc->zeros[j];
	        }
            *(py++) = sum;
	    (*preturn) ++;
	    outcount += edown;
	    }
	sum = 0;
        state_sum = 0;
        }

/* Return final state for next time. */

    *poutflag = outcount - up*n;

    for (i=0; i<uporder; i++)
	{
	init_state [i] = state [up*n + i];
	}

    return (1);
}

