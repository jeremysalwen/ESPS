/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1988, 1990 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  Program: plot3d							|
|  Module: p3dtext.c							|
|									|
|  Text-drawing function for plot3d(1-ESPS).				|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)p3dtext.c	1.2	7/27/90	ESI";
#endif

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/constants.h>

#define NEWSTROKE(p) ((p)&0200)
#define U_COORD(p) (((p)&0160) >> 4)
#define V_COORD(p) ((p)&07)
#define ST_MAX 50

extern char *ascii[];

extern void aplotlines();

void
text(u0, v0, sp, ht, str)
    double  u0, v0;
    int	    sp, ht;
    char    *str;
{
    double  u[ST_MAX], v[ST_MAX], 
	    usp,
	    du,
	    dv,
	    lower,
	    u_coord,
	    v_coord;

    int     ch;

    char    *strokes,
	    point;

    int     i;

    if (!str) return;

    usp = sp;
    du = sp/6.0;
    dv = ht/10.0;

    if (sp < 0) u0 -= sp*strlen(str);
    v0 -= 2*dv;

    while ((ch = *str++) != '\0')
    {
	if (ch < ' ' || ch >= '\177')
	    switch (ch)
	    {
	    case '\b':
		u0 -= usp;
		break;
	    default:
		break;
	    }
	else
	{
	    strokes = ascii[ch - ' '];
	    lower = *strokes++ == 'd' ? 2 : 0;
	    u[0] = u0 + du;
	    v[0] = v0;
	    i = 1;

	    while (point = *strokes++)
	    {
		if (NEWSTROKE(point))
		{
		    if (i > 1)
			aplotlines(i, u, v);
		    i = 0;
		}
		u_coord = U_COORD(point);
		v_coord = V_COORD(point) - lower;
		u[i] = u0 + du*u_coord;
		v[i] = v0 + dv*v_coord;
		i++;
	    }

	    if (i > 1)
		aplotlines(i, u, v);

	    u0 += usp;
	}
    }
}   /* end text() */
