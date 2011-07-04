
/********************************************************
*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
 				

 	
*
*  Module Name: block_filter.c
*
*  Written By:   Brian Sublett
*  Checked By:   Alan Parker
*
*  Purpose:  This subroutine takes a data array, pointed to by x, and
*	     a zfunc pointed to by pzfunc, and filters the array,
*	     returning the result in another array pointed to by y.
*	     If the value of init is zero, the arrays are allocated
*            and filled up with the passed values,
*	     but no filtering takes place.  Any non-zero value of
*	     init causes the filtering to take place.
*
*  Secrets:  The function uses a static array to store its 
*	     state vector, and returns this vector to the calling
*	     function each time.  Since the filter is
*	     implemented in direct form, one state vector
*	     is sufficient for FIR and IIR filters.
*
*********************************************************/

# ifdef SCCS
	static char *sccs_id = "@(#)block_filter.c	1.8 10/19/93 ESI";
# endif

# include <stdio.h>
# include <esps/esps.h>

#ifndef DEC_ALPHA
char *calloc();
char *realloc();
#endif

block_filter (n, x, y, pzfunc, init_state)
int n;
double *x, *y, *init_state;
struct zfunc *pzfunc;
{

    int i, j, m, nnsiz, ddsiz;
    double sum, state_sum;
    static double *state;
    static int alloc_size = 0, order;

#ifdef UE
/*  Check the values passed in */
    spsassert(x != NULL, "block_filter: x is NULL");
    spsassert(y != NULL, "block_filter: y is NULL");
    spsassert(init_state != NULL, "block_filter: init_state is NULL");
    spsassert(pzfunc != NULL, "block_filter: pzfunc is NULL");
#endif

/* Allocate the array space on first call. */

    nnsiz = pzfunc->nsiz;
    ddsiz = pzfunc->dsiz;

    if (nnsiz > ddsiz) order = nnsiz;
    else order = ddsiz;

    if (alloc_size == 0)
        {
	alloc_size = n + order;
        state = (double*) calloc ((unsigned)(n + order), sizeof (double));
	spsassert(state != NULL, "block_filter: calloc failed");
        }
    else if (alloc_size < n + order)
        {
	alloc_size = n + order;
        state = (double*) realloc ((char*)state, alloc_size*sizeof (double));
	spsassert(state != NULL, "block_filter: realloc failed");
        }

/* Copy the previous state to the beginning of the state array. */

    for (i=0; i<order; i++)
 	{
        state [i] = init_state [i];
	}


/* Begin the filtering operation. */

    sum = 0;
    state_sum = 0;
    for (m=0; m<n; m++)
        {
        for (j=1; j<ddsiz; j++)
            {
	    state_sum -= state[order + m-j]*pzfunc->poles[j];
	    }
	state_sum += x[m];
	if (ddsiz >= 1)
	    {
            if (pzfunc->poles[0] != 0) state_sum = state_sum/(pzfunc->poles[0]);
	    else
	        {
	        Fprintf(stderr,"block_filter: poles[0]=0. Exiting.\n");
	        exit (1);
	        }
	    }

	state [order + m] = state_sum;

        for (j=0; j<nnsiz; j++)
    	    {
	    sum += state[order + m-j]*pzfunc->zeros[j];
	    }

        y[m] = sum;
	sum = 0;
        state_sum = 0;
        }

/* Return final state for next time. */

    for (i=0; i<order; i++)
	{
	init_state [i] = state [n + i];
	}
    return (1);
}
