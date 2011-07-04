/* draw.c - drawing primitives for genplot(1-ESPS)
 *
 * This material contains proprietary software of Entropic Speech,
 * Inc.  Any reproduction, distribution, or publication without the
 * prior written permission of Entropic Speech, Inc. is strictly
 * prohibited.  Any public distribution of copies of this work
 * authorized in writing by Entropic Speech, Inc. must bear the
 * notice
 *
 *  "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."
 *
 * Author: Rodney Johnson, Entropic Speech, Inc.
 */

#ifndef lint
    static char *sccs_id = "@(#)draw.c	3.2	10/16/87	ESI";
#endif

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include "genplot.h"

#define LBL_MAX 50

drawaxes(ulow, vlow, xlow, ylow, deltax, deltay, xscale, yscale,
	 xticint, yticint, charsp, axflag, line_plotter)
    long    ulow;
    long    vlow;
    long    xlow;
    double  ylow;
    long    deltax;
    double  deltay;
    long    xscale;
    double  yscale;
    long    xticint;
    double  yticint;
    long    charsp;
    int	    (*line_plotter)();	/* pointer to a function, either
				   tek_plotline or gps_plotline */
{
    long    uhigh = ulow + deltax * xscale;
    long    vhigh = vlow + LROUND(deltay * yscale);

    long    vzero = vlow + LROUND(-ylow * yscale);

    long    dx;
    double  dy;

    long    u;
    long    v;

    char    label[LBL_MAX];

    double  fmod();


    /* draw a rectangular box which is the drawing frame */

    DRAW(ulow, vlow, uhigh, vlow, line_plotter)
    DRAW(uhigh, vlow, uhigh, vhigh, line_plotter)
    DRAW(uhigh, vhigh, ulow, vhigh, line_plotter)
    DRAW(ulow, vhigh, ulow, vlow, line_plotter)

    if (axflag && vlow < vzero && vzero < vhigh)
	DRAW(ulow, vzero, uhigh, vzero, line_plotter)

    for (   dx = (xticint - 1) - (xlow + xticint - 1) % xticint;
	    dx <= deltax;
	    dx += xticint
	)
    {
	u = ulow + dx * xscale;

	DRAW(u, vlow, u, vlow - LROUND(charsp/3.0), line_plotter)

	Sprintf(label, "%ld", xlow + dx);
	text(	u - charsp*strlen(label)/2, 
		vlow - 2*charsp,
		charsp, 0, label, line_plotter
	    );
    }

    for (   dy = 0;
	    (v = vlow + LROUND(dy * yscale)) <= vhigh;
	    dy += yticint
	)
    {
	DRAW(ulow, v, ulow - LROUND(charsp/3.0), v, line_plotter)

	Sprintf(label, "%6.2g", ylow + dy);
	text(	ulow - charsp*strlen(label) - charsp/2, 
		v - LROUND(charsp/3.0),
		charsp, 0, label, line_plotter
	    );
    }

}   /* end drawaxes() */


#define HDR_MAX 250

printheader(hdrleft, hdrright, hdrbase, charsp, sdname,
	    tagname, xscale, line_plotter)
    long    hdrleft;
    long    hdrright;
    long    hdrbase;
    long    charsp;
    char    *sdname;
    char    *tagname;
    long    xscale;
    int	    (*line_plotter)();	/* pointer to a function, either
				   tek_plotline or gps_plotline */
{
    char    hdrtxt[HDR_MAX];

/* GLOBAL Variable referenced:  page_num  */

    extern int	page_num;

    Sprintf(hdrtxt, "ESPS File: %s   (%d pixel%s/point)  page %d",
	    sdname, xscale, (xscale==1) ? "" : "s", page_num);

    text(hdrleft, hdrbase, charsp, 0, hdrtxt, line_plotter);

    if (tagname != NULL)
    {
	Sprintf(hdrtxt, "Tagged file: %s", tagname);
	text(hdrleft, hdrbase - LROUND(10*charsp/6.0), 
		charsp, 0, hdrtxt, line_plotter);
    }

    gettime(hdrtxt);

    text(   hdrright - charsp*strlen(hdrtxt),
	    hdrbase, charsp, 0, hdrtxt, line_plotter
	);

}   /* end printheader() */


gettime(str)
    char    *str;
{
    long    time();
    long    tloc;
    char    *ctime();
    char    *tptr;
    int	    i;

    (void) time(&tloc);
    tptr = ctime(&tloc);

    for (i=0; i<4; i++)
	str[i] = tptr[i+20];

    for (i=4; i<17; i++)
	str[i] = tptr[i-1];

    str[17] = '\0';
}


#define NEWSTROKE(p) ((p)&0200)
#define LASTPOINT(p) ((p)&010)
#define U_COORD(p) (((p)&0160) >> 4)
#define V_COORD(p) ((p)&07)
#define ST_MAX 50

extern char *ascii[];

text(u0, v0, sp, rot, str, line_plotter)
    long    u0;
    long    v0;
    long    sp;
    int	    rot;
    char    *str;
    int	    (*line_plotter)();	/* pointer to a function, either
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
