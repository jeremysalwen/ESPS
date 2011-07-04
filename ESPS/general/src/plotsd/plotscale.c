/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  plotscale -- find round numbers for plot boundaries.			|
|									|
|  Joseph T. Buck, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
/*
 * Plotscale works as follows.  First it attempts to find a divisor,
 * as follows.  It computes log10 of the difference vmax - vmin, or
 * mindif, whichever is greater.  int (log10) is the exponent.  Then
 * it divides through by 10^exp to find the mantissa:
 *	(1.0 <= mantissa < 10.0).
 * It finds the first integer larger than the mantissa.  If it is two
 * or three, the divisor is 0.5*10^exp.  Otherwise it is 10^exp.  
 *
 * The returned min and max are evenly divisible by this divisor;
 * *p_vmin is the floor, and *p_vmax is the ceiling.  Let
 *	nticks = (*p_vmax - *p_vmin) / divisor
 * then if nticks < 8, div is the tick spacing.  Otherwise, if nticks
 * is divisible by 3, 3*div is the spacing, otherwise 2*div is the
 * spacing.  *p_ndp is the number of places after the decimal point
 * (0 if the tick spacing is an integer).
 */

#ifndef lint
    static char *sccs_id = "@(#)plotscale.c	3.2 11/4/89 ESI";
#endif

#include <stdio.h>

double pow(), log10();

void
plotscale(vmin, vmax, mindif, p_vmin, p_vmax, p_vtick, p_ndp)
    double  vmin, vmax, mindif, *p_vmin, *p_vmax, *p_vtick;
    int     *p_ndp;
{
    double  diff = vmax - vmin, teno, div, adiff;
    int     o, adj, nticks;

    if (diff <= 0.0) diff = mindif;
    o = log10(diff);
    teno = pow(10.0, (double) o);
    if (teno > diff) teno /= 10.0;
    adj = diff / teno + 0.999;
    adiff = adj * teno;
    div = teno;
    if (adj <= 3)
	div = teno / 2;
/* Now move vmin down so it is divisible by div. */
    if (vmin >= 0.0)
	vmin = ((int) (vmin / div)) * div;
    else
#ifdef MACII
	vmin = 0-((int) (-vmin / div + 0.9999)) * div;
#else
	vmin = -((int) (-vmin / div + 0.9999)) * div;
#endif
/* Now move vmax up so it is divisible by div. */
    if (vmax >= 0.0)
	vmax = ((int) (vmax / div + 0.9999)) * div;
    else
#ifdef MACII
	vmax = 0-((int) (-vmax / div)) * div;
#else
	vmax = -((int) (-vmax / div)) * div;
#endif
    *p_vmin = vmin;
    *p_vmax = vmax;
/* Figure out tick marks. */
    adiff = vmax - vmin;
    nticks = adiff / div;
    if (nticks < 8)
	*p_vtick = div;
    else if (nticks % 3 == 0)
	*p_vtick = div * 3;
    else
	*p_vtick = div * 2;
    *p_ndp = 0;
    if (*p_vtick < 1.0)
	*p_ndp = 0.9999 - log10 (*p_vtick);
    return;
}
