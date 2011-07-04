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
|   text -- plot a string of Ascii characters using an arbitrary	|
|	    line-drawing function.  Character shapes are encoded in	|
|	    an external string array ascii[] defined in chrs.c		|
|									|
|   Rodney Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)text.c	3.1	10/7/87	ESI";
#endif

#define L_ROUND(n) ((n)>=0 ? (long)((n)+0.5) : -(long)(-(n)+0.5))
#define NEWSTROKE(p) ((p)&0200)
#define LASTPOINT(p) ((p)&010)
#define U_COORD(p) (((p)&0160) >> 4)
#define V_COORD(p) ((p)&07)
#define ST_MAX 50

double  sin();
double  cos();

extern char *ascii[];

text(u0, v0, sp, rot, str, line_plotter)
    long    u0;			/* Coordinate of left end of baseline. */
    long    v0;			/* Coordinate of left end of baseline. */
    long    sp;			/* Character spacing */
    int	    rot;		/* Orientation of baseline (degrees */
				/* counterclockwise from horizontal). */
    char    *str;		/* String to plot. */
    int	    (*line_plotter)();	/* Pointer to line_plotting function. */
{
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

    usp = L_ROUND(sp*cosrot);
    vsp = L_ROUND(sp*sinrot);

    du = L_ROUND(sp*cosrot/6);
    dv = L_ROUND(sp*sinrot/6);

    while ((ch = *str++) != '\0')
    {
	if (ch < ' ' || ch >= '\177')
	    switch(ch)
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

	    do
	    {
		point = *strokes++;
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
	    } while (!LASTPOINT(point));

	    if (i > 1)
		(*line_plotter)((long) i, u, v);

	    u1 += usp;
	    v1 += vsp;
	}
    }

}   /* end text() */
