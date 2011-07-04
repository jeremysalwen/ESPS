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
|  plotexscale -- find exact numbers for plot boundaries.		|
|									|
|  John Shore, Entropic Speech, Inc.					|
|       A complete hack based on plotscale.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)plotexscal.c	1.1 9/19/97   ERL";
#endif

#include <stdio.h>

void
plotexscale(vmin, vmax, mindif, p_vmin, p_vmax, p_vtick, p_ndp)
    double  vmin, vmax, mindif, *p_vmin, *p_vmax, *p_vtick;
    int     *p_ndp;
{
    double  diff = vmax - vmin;
    int     nticks = 4;

    if (diff < 0.0) diff = mindif;
    *p_vmin = vmin;
    *p_vmax = vmax;
    *p_vtick = diff / nticks;
    *p_ndp = 0;
    return;
}
