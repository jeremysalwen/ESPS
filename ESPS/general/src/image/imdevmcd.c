/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1988, 1989 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  Module: imdevmcd.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifdef SP40

#ifndef lint
static char *sccs_id = "@(#)imdevmcd.c	1.3	7/25/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <libgpdefs.h>
#include "image.h"

#define Y_OFF	90

char		*arr_alloc();
void		arr_free();

void		plotimage();

static void	get_config();
static void	mcd_row(), mcd_initbits(), mcd_plotline(), assign();

extern int	debug_level;
extern long	width, height;
extern long	nrows, ncols;
extern long	lmarg, rmarg, tmarg, bmarg;

extern int	oflag;
extern int	dev;
extern int	gray_bits;

extern void	(*dev_initbits)();
extern void	(*dev_row)();
extern void	(*dev_plotline)();

static int	    rownum;

static MGBB_DESC    desc;
static long	    **bitarr;
static long	    bitarrdim[2];
/* static int	    savecolors[16]; */

void
mcd_init()
{
    int	    xl, yb, xr, yt, w, h;
    int	    avail_bits;
    int	    bitoff, mono, nplanes, mask, banks;
    int	    u, v;

    dev_row = mcd_row;
    dev_initbits = mcd_initbits;
    dev_plotline = mcd_plotline;

    assign();

    get_config(&avail_bits);
    if (avail_bits < gray_bits)
    {
	Fprintf(stderr, "%s: %s - exiting\n", "mcd_init",
		"system hasn't enough gray-scale resolution for algorithm");
	exit(1);
    }

    xl = 0;
    yb = Y_OFF;
    xr = width - 1 + lmarg + rmarg;
    yt = Y_OFF + height - 1 + bmarg + tmarg;
    mgibox(xl, yb, xr, yt);

    mgimodfunc(4, 0, 3);
    
    switch (gray_bits)
    {
    case 4:
	{
	    static int  colors[16] =
		{
		    0x000000, 0x111111, 0x222222, 0x333333,
		    0x444444, 0x555555, 0x666666, 0x777777,
		    0x888888, 0x999999, 0xaaaaaa, 0xbbbbbb,
		    0xcccccc, 0xdddddd, 0xeeeeee, 0xffffff
		};

	    mgicms(0, 16, colors);
	}
	break;
    case 1:
	xl = yb = 0;
	w = width;
	h = height;
	bitoff = 0;
	mono = nplanes = mask = banks = 1;
	mgibbdesc1(&desc, xl, yb, w, h, bitoff, mono, nplanes, mask, banks);

	bitarrdim[0] = height;
	bitarrdim[1] = (width + 31)/32;
	bitarr  = (long **) arr_alloc(2, bitarrdim, LONG, 0);
	desc.addr = (char *) &bitarr[0][0];

	for (v = 0; v < bitarrdim[0]; v++)
	    for (u = 0; u < bitarrdim[1]; u++)
	    bitarr[v][u] = 0;
	break;
    }

    rownum = 0;
    plotimage();
}

static void
mcd_initbits()
{
    assign();

    switch (gray_bits)
    {
    case 4:
	mgimodfunc(3, 0, 3);
	break;
    default:
	break;
    }
}

static void
mcd_row(row)
    char    *row;
{
    assign();

    switch (gray_bits)
    {
    case 4:
	{
	    int	v;

	    if (oflag)
		for (v = 0; v < ncols; v++)
		{
		    mgihue(15 - row[v]);
		    mgip(v + lmarg, Y_OFF + bmarg + nrows - 1 - rownum);
		}
	    else
		for (v = 0; v < ncols; v++)
		{
		    mgihue(15 - row[v]);
		    mgip(rownum + lmarg, Y_OFF + bmarg + v);
		}
	}
	break;
    case 1:
	{
	    int		    u, v;
	    int		    sxl, syb, dxl, dyb, w, h;
	    unsigned long   b;

	    if (oflag)
	    {
		for (v = 0; v < ncols; v++)
		    if (row[v])
			bitarr[rownum][v/32]
				|= (unsigned long) 0x80000000 >> (v%32);
		sxl = 0;
		syb = nrows - 1 - rownum;
		dxl = lmarg;
		dyb = Y_OFF + bmarg + syb;
		w = ncols;
		h = 1;
	    }
	    else
	    {
		u = rownum/32;
		b = (unsigned long) 0x80000000 >> (rownum%32);
		for (v = 0; v < ncols; v++)
		    if (row[v])
			bitarr[ncols - 1 - v][u] |= b;
		sxl = rownum;
		syb = 0;
		dxl = rownum + lmarg;
		dyb = Y_OFF + bmarg;
		w = 1;
		h = ncols;
	    }
	    mgibblt1(&desc, sxl, syb, dxl, dyb, w, h);
	}
	break;
    }
    rownum++;
}

void
mcd_fin()
{
    switch (gray_bits)
    {
    case 4:
	break;
    default:
	arr_free((char *) bitarr, 2, LONG, 0);
	mgibbfree(&desc);
	break;
    }

    mgideagp(0, 0);
}

void
mcd_default_size(w, h)
    long    *w, *h;
{
    int	    viewno, xl, yb, xr, yt, placed;

    assign();

    mgigetv(&viewno);
    mgigetvcoor(viewno, &xl, &yb, &xr, &yt, &placed);

    *w = xr - lmarg - rmarg;
    *h = yt - bmarg - tmarg - Y_OFF;
    if (oflag)
    {
	if (*w > *h/4) *w = *h/4;
    }
    else
    {
	if (*h > *w/4) *h = *w/4;
    }
    if (*w < 2) *w = 2;
    if (*h < 2) *h = 2;
}

int
mcd_depth()
{
    int	    avail_bits;

    assign();
    get_config(&avail_bits);
    return avail_bits;
}

static void
assign()
{
    static int	   gpassigned = NO;

    if (!gpassigned)
    {
	mgiasngp(0, 0);
	gpassigned = YES;
    }
}

static void
mcd_plotline(n, h, v)
    long    n;
    long    *h, *v;
{
    int	    x[100], y[100];
    int	    i;
/*!*//* use malloc; keep track of alloc size, etc. */
    for (i = 0; i < n; i++)
    {
	x[i] = h[i] + lmarg;
	y[i] = v[i] + bmarg + Y_OFF;
    }
    mgils((int) n, x, y);
}

#define HC_SIZE 32

static void
get_config(avail)
    int	    *avail;
{
    int	    config[HC_SIZE];

    mgigethc(HC_SIZE, config);

    if (config[6] == 0)
	*avail = 1;
    else
	*avail = config[7];
}

#endif
