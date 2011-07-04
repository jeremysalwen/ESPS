/* --------------------------------------------------------------------------
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				


				EMPHASIS

performs pre-emphasis ASSUMING a 2nd order pole-zero filter

The output data overlays the input data in the array 'data'

ASSUMES that 'state' has previously been properly initialized

Goes to a special entry point (FAST) in the case where:
	den[0] = num[0] = 1.0

Implementation note:
  X(Z) input time series	variable name: data
  Y(Z) output time series	variable name: data
  N(Z) numerator filter		variable name: num
  D(Z) denominator filter	variable name: den
  S(Z) state			variable name: state

  Y(Z) / X(Z) = N(Z) / D(Z)
Let S(Z) = X(Z) / D(Z)
then	Y(Z) = S(Z) * N(Z)

Written by: Bernard G. Fraenkel
--------------------------------------------------------------------------- */


#ifndef lint
	static char *sccsid = "@(#)emphasis.c	1.3	7/31/87 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>

extern int debug_level;

emphasis (data, length, num, den, state)
double *data, *num, *den, *state;
int     length;
{
    register int    i;
    double *dptr, ooden, *pnum1, *pnum2, *pden1, *pden2, *pstate1, *pstate2;

    pnum1 = num + 1;
    pnum2 = num + 2;
    pden1 = den + 1;
    pden2 = den + 2;
    pstate2 = state + 2;
    pstate1 = state + 1;

#ifdef UE
    spsassert(data != NULL, "emphasis: data is NULL");
    spsassert(num != NULL, "emphasis: num is NULL");
    spsassert(den != NULL, "emphasis: den is NULL");
    spsassert(state != NULL, "emphasis: state is NULL");
#endif


    if (*num == 1.0 && *den == 1.0)
	goto FAST;
    else if (*den == 0.0)
	faterr ("emphasis", "den[0] == 0.0", -2);
    else
	ooden = 1.0 / *den;

    if (debug_level > 2)
	Fprintf (stderr, "emphasis: regular entry point\n");

    for (i = 0, dptr = data; i < length; i++, dptr++) {
	*state = *dptr - *pden2 * *pstate2;
	*dptr = *pnum2 * *pstate2;
	*pstate2 = *pstate1;	/* state[2] = state[1] */

	*state -= *pden1 * *pstate1;
	*dptr += *pnum1 * *pstate1;

	*state *= ooden;
	*pstate1 = *state;	/* state[1] = state[0] */
	*dptr += *num * *state;
    }
    return (0);

 /* ---------- */

FAST: 				/* Fast version when den[0] = num[0] = 1.0
				   */

    if (debug_level > 2)
	Fprintf (stderr, "emphasis: entry point: FAST\n");

    for (i = 0, dptr = data; i < length; i++, dptr++) {
	*state = *dptr - *pden2 * *pstate2;
	*dptr = *pnum2 * *pstate2;
	*pstate2 = *pstate1;	/* state[2] = state[1] */

	*state -= *pden1 * *pstate1;
	*dptr += *pnum1 * *pstate1;
	*pstate1 = *state;	/* state[1] = state[0] */

	*dptr += *state;
    }
    return (0);

}

/* ----------------------------------------------------------------------- */

