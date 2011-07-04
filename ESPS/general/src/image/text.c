/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|   "Copyright (c) 1988 Entropic Speech, Inc.  All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  Module: text.c							|
|									|
|  Text-drawing function for image(1-ESPS).				|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)text.c	1.1	6/2/88	ESI";
#endif

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>

#define NEWSTROKE(p) ((p)&0200)
#define U_COORD(p) (((p)&0160) >> 4)
#define V_COORD(p) ((p)&07)
#define ST_MAX 50

extern char *ascii[];

void
text(u0, v0, sp, rot, str, line_plotter)
    long    u0;
    long    v0;
    int	    sp;
    int	    rot;
    char    *str;
    void    (*line_plotter)();	/* pointer to a function, either
				   tek_plotline or gps_plotline */
{
    double  sin();
    double  cos();

    double  theta,
	    cosrot,
	    sinrot;

    long    u[ST_MAX], v[ST_MAX], 
	    u1 = u0, 
	    v1 = v0, 
	    usp,
	    vsp, 
	    du,
	    dv, 
	    lower,
	    u_coord,
	    v_coord;

    int     ch;

    char    *strokes,
	    point;

    int     i;

    theta = rot * 3.14159265358979323846/180;

    cosrot = cos(theta);
    sinrot = sin(theta);

    usp = LROUND(sp*cosrot);
    vsp = LROUND(sp*sinrot);

    du = LROUND(sp*cosrot/6);
    dv = LROUND(sp*sinrot/6);

    while ((ch = *str++) != '\0')
    {
	if (ch < ' ' || ch >= '\177')
	    switch (ch)
	    {
	    case '\b':
		u1 -= usp;
		v1 -= vsp;
		break;
	    default:
		break;
	    }
	else
	{
	    strokes = ascii[ch - ' '];
	    lower = *strokes++ == 'd' ? 2 : 0;
	    u[0] = u1 + du;
	    v[0] = v1 + dv;
	    i = 1;

	    while (point = *strokes++)
	    {
		if (NEWSTROKE(point))
		{
		    if (i > 1)
			(*line_plotter)((long) i, u, v);
		    i = 0;
		}
		u_coord = U_COORD(point);
		v_coord = V_COORD(point) - lower;
		u[i] = u1 + du*u_coord - dv*v_coord;
		v[i] = v1 + dv*u_coord + du*v_coord;
		i++;
	    }

	    if (i > 1)
		(*line_plotter)((long) i, u, v);

	    u1 += usp;
	    v1 += vsp;
	}
    }

}   /* end text() */
