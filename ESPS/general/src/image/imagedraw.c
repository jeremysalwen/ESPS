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
|  Program: image.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)imagedraw.c	1.3	8/10/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include "image.h"

void		text();
void		pscale();

static void	tick();

extern int	debug_level;
extern long	width, height;
extern long	nrows, ncols;
extern long	lmarg, rmarg, tmarg, bmarg;
extern int	oflag;
extern double	*x, *y;
extern double	xmin, xmax;
extern long	nrecs, nelem;
extern int	scale;
extern void	(*dev_plotline)();
extern char	*h_ttl_text, *v_ttl_text;

static int	charsp, linesp, bl_off, ax_off, tic_len, tic_off;
static long	hor[50], ver[50];


void
axes_and_titles()
{
    int	    i;
    double  start, step, minstep;
    int	    num;
    double  label;
    long    coord;
    double  ymin, ymax;
    int	    trunc;
    int	    maxlen;
    char    savechar;

    ymin = y[0];
    ymax = y[nelem-1];

    charsp = 6*scale;
    linesp = 10*scale;
    bl_off = 3*scale;
    ax_off = 5*scale;
    tic_len = 4*scale;
    tic_off = 5*scale;

    hor[0] = -1;	    ver[0] = -1;
    hor[1] = width;	    ver[1] = -1;
    hor[2] = width;	    ver[2] = height;
    hor[3] = -1;	    ver[3] = height;
    hor[4] = -1;	    ver[4] = -1;
    (*dev_plotline)(5, hor, ver);

    if (oflag)
    {
	tick((long) (height - 1),   xmin,	AX_Y);
	tick((long) 0,		    xmax,	AX_Y);	

	minstep = (xmax - xmin) * (2.0*linesp) / (height - 1);
	pscale(xmin, xmax, minstep, &start, &step, &num);
	for (i = 0; i < num; i++)
	{
	    label = start + i * step;
	    coord = LROUND((height - 1) * (label - xmax) / (xmin - xmax));
	    tick(coord, label, AX_Y);
	}

	tick((long) 0,		    ymin,	AX_X);
	tick((long) (width - 1),    ymax,	AX_X);

	minstep = (ymax - ymin) * (10.0*charsp) / (width - 1);
	pscale(ymin, ymax, minstep, &start, &step, &num);
	for (i = 0; i < num; i++)
	{
	    label = start + i * step;
	    coord = LROUND((width - 1) * (label - ymin) / (ymax - ymin));
	    tick(coord, label, AX_X);
	}
    }
    else
    {
	tick((long) 0,		    xmin,	AX_X);
	tick((long) (width - 1),    xmax,	AX_X);

	minstep = (xmax - xmin) * (10.0*charsp) / (width - 1);
	pscale(xmin, xmax, minstep, &start, &step, &num);
	for (i = 0; i < num; i++)
	{
	    label = start + i * step;
	    coord = LROUND((width - 1) * (label - xmin) / (xmax - xmin));
	    tick(coord, label, AX_X);
	}

	tick((long) 0,		    ymin,	AX_Y);
	tick((long) (height - 1),   ymax,	AX_Y);

	minstep = (ymax - ymin) * (2.0*linesp) / (height - 1);
	pscale(ymin, ymax, minstep, &start, &step, &num);
	for (i = 0; i < num; i++)
	{
	    label = start + i * step;
	    coord = LROUND((height - 1) * (label - ymin) / (ymax - ymin));
	    tick(coord, label, AX_Y);
	}
    }

    maxlen = (width+rmarg)/charsp;
    trunc = strlen(h_ttl_text) > maxlen;
    if (trunc)
    {
	savechar = h_ttl_text[maxlen];
	h_ttl_text[maxlen] = '\0';
    }
    text((long) 0, (long)(- tic_off - 2*linesp + bl_off),
	charsp, 0, h_ttl_text, dev_plotline);
    if (trunc)
	h_ttl_text[maxlen] = savechar;

    maxlen = (height+tmarg)/charsp;
    trunc = strlen(v_ttl_text) > maxlen;
    if (trunc)
    {
	savechar = v_ttl_text[maxlen];
	v_ttl_text[maxlen] = '\0';
    }
    text((long)(- tic_off - 7*charsp - bl_off), (long) 0,
	charsp, 90, v_ttl_text, dev_plotline);
    if (trunc)
	v_ttl_text[maxlen] = savechar;
}

static void
tick(coord, label, axis)
    long    coord;
    double  label;
    int	    axis;
{
    char    labeltxt[50];

    Sprintf(labeltxt, "%g", label);

    switch (axis)
    {
    case AX_X:
	hor[0] = coord;	    ver[0] = -1;
	hor[1] = coord;	    ver[1] = -tic_len;
	text((long)(coord - charsp*strlen(labeltxt)/2),
	    (long)(- tic_off - linesp + bl_off),
	     charsp, 0, labeltxt, dev_plotline);
	break;
    case AX_Y:
	hor[0] = -tic_len;  ver[0] = coord;
	hor[1] = -1;	    ver[1] = coord;
	text((long)(- tic_off - charsp*strlen(labeltxt)),
	    (long)(coord - ax_off + bl_off),
	    charsp, 0, labeltxt, dev_plotline);
	break;
    }

    (*dev_plotline)(2, hor, ver);
}

void
set_margins(flg, scl, l, r, t, b)
    int	    flg, scl;
    long    *l, *r, *t, *b;
{
    if (flg)
    {
	*l = 58*scl;
	*r = 21*scl;
	*t = 7*scl;
	*b = 25*scl;
    }
    else
    {
	*l = *r = *t = *b = 0;
    }
}

