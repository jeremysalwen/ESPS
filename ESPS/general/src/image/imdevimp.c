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
|  Module: imdevimp.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)imdevimp.c	1.3	7/25/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include "image.h"

#define IMP_BITMAP_CMD	(unsigned char) 235
#define IMP_OR_OP	(unsigned char) 7
#define IMP_SET_MAG_CMD	(unsigned char) 236
#define IMP_SET_ABS_H_CMD   (unsigned char) 135
#define IMP_SET_ABS_V_CMD   (unsigned char) 137
#define IMP_CRE_PAT_CMD	(unsigned char) 230
#define IMP_DRAW_PAT_CMD    (unsigned char) 234
#define IMP_SET_HV_SY_CMD   (unsigned char) 205
#define IMP_ROT_AXES	(unsigned char) 05
#define IMP_PAGE_W	2550
#define IMP_PAGE_H	3300

extern long	lmarg, rmarg, tmarg, bmarg;

extern int	mag;

char		*arr_alloc();
void		arr_free();

void		plotimage();

static void	imp_row(), imp_initbits(), imp_plotline();

extern int	debug_level;
extern long	width, height;
extern long	nrows, ncols;

extern int	oflag;
extern int	dev;
extern int	gray_bits;

extern void	(*dev_initbits)();
extern void	(*dev_row)();
extern void	(*dev_plotline)();

static void	putshort();

static int	rownum;
static char	**rowbuf;
static long	rowbufdim[] = {32, 0};
static char	*zerorow;
static char	*bits;
static unsigned	hsize, vsize;
static int	h_offset, v_offset;


void
imp_init()
{
    int	    u, v;
    int	    mag_code;

    dev_row = imp_row;
    dev_initbits = imp_initbits;
    dev_plotline = imp_plotline;

    if (gray_bits > 1)
    {
	Fprintf(stderr,
	    "multilevel gray scale not supported on Imagen.\n");
	exit(1);
    }

    hsize = (int) ((ncols + 31)/32);
    vsize = (int) ((nrows + 31)/32);

    rowbufdim[1] = 32*hsize;
    rowbuf = (char **) arr_alloc(2, rowbufdim, CHAR, 0);
    for (u = 0; u < 32; u++)
	for (v = 0; v < rowbufdim[1]; v++)
	rowbuf[u][v] = 0;

    zerorow = (char *) arr_alloc(1, &ncols, CHAR, 0);
    for (v = 0; v < ncols; v++)
	zerorow[v] = 0;

    bits = (char *) malloc(128 * hsize * sizeof(char));
    spsassert(bits, "can't allocate space for bit array");

    switch (mag)
    {
    case 1:
	mag_code = 0;
	break;
    case 2:
	mag_code = 1;
	break;
    case 4:
	mag_code = 2;
	break;
    default:
	Fprintf(stderr, "Magnification must be 1, 2, or 4 on Imagen.\n");
	exit(1);
	break;
    }

    if (oflag)
    {
	h_offset =
	    32*((IMP_PAGE_H + (lmarg - width - rmarg)*mag)/64);
	v_offset =
	    -32*((IMP_PAGE_W +(bmarg + height - tmarg)*mag)/64);
	putchar(IMP_SET_HV_SY_CMD); putchar(IMP_ROT_AXES);
    }
    else
    {
	h_offset =
	    32*((IMP_PAGE_W + (bmarg - height - tmarg)*mag)/64);
	v_offset =
	    32*((IMP_PAGE_H + (lmarg - width - rmarg)*mag)/64);
    }

    putchar(IMP_SET_MAG_CMD); putchar(mag_code);
    rownum = 0;
    plotimage();
}

static void
imp_initbits()
{
    int	    fudge = oflag ? 32*(mag - 1) : 0;
				/* The above should not be necessary */
				/* according to Impress manual. */
				/* Bug in Imagen bitmap magnification */
				/* for some orientations of axes? */

    putchar(IMP_SET_ABS_H_CMD); putshort(h_offset);
    putchar(IMP_SET_ABS_V_CMD); putshort(v_offset + fudge);
    putchar(IMP_BITMAP_CMD);
    putchar(IMP_OR_OP);
    putchar((unsigned char) hsize);
    putchar((unsigned char) vsize);
}

static void
putshort(w)
    int w;
{
    int	    hibyte, lobyte;

    lobyte = w % 256;	if (lobyte < 0) lobyte += 256;
    hibyte = (w - lobyte)/256;
    putchar((char) hibyte); putchar((unsigned char) lobyte);
}


static void
imp_row(row)
    char    *row;
{
    int		u, v;

    for (v = 0; v < ncols; v++)
	rowbuf[rownum%32][v] = row[v];
    rownum++;	    
    if (rownum % 32 == 0)
    {
	int	    i, j;

	for (u = 0; u < 32; u++)
	    for (i = 4*u, v = 0; v < rowbufdim[1]; i += 128)
	    for (j = 0; j < 4; j++)
	    {
		bits[i+j] = 0;
		if (rowbuf[u][v++]) bits[i+j] |= 0x80;
		if (rowbuf[u][v++]) bits[i+j] |= 0x40;
		if (rowbuf[u][v++]) bits[i+j] |= 0x20;
		if (rowbuf[u][v++]) bits[i+j] |= 0x10;
		if (rowbuf[u][v++]) bits[i+j] |= 0x08;
		if (rowbuf[u][v++]) bits[i+j] |= 0x04;
		if (rowbuf[u][v++]) bits[i+j] |= 0x02;
		if (rowbuf[u][v++]) bits[i+j] |= 0x01;
	    }

	for (j = 0; j < 4*rowbufdim[1] /* 128*hdim */; j++)
	    putchar(bits[j]);
    }
}

void
imp_fin()
{
    while (rownum % 32 != 0)
	imp_row(zerorow);
    arr_free((char *) rowbuf, 2, CHAR, 0);
    free((char *) bits);
}

void
imp_default_size(w, h)
    long    *w, *h;
{
    if (oflag)
    {
	*w = 500/mag;
	*h = 2000/mag;
    }
    else
    {
	*w = 2400/mag;
	*h = 600/mag;
    }
}

static void
imp_plotline(n, h, v)
    long    n;
    long    *h, *v;
{
    int	    i;

    putchar(IMP_CRE_PAT_CMD);
    putshort((int) n);
    if (oflag)
	for (i = 0; i < n; i++)
	{
	    putshort((int) (h_offset + h[i]*mag));
	    putshort((int) (v_offset - (v[i] - height)*mag));
	}
    else
	for (i = 0; i < n; i++)
	{
	    putshort((int) (h_offset + v[i]*mag));
	    putshort((int) (v_offset + h[i]*mag));
	}
    putchar(IMP_DRAW_PAT_CMD);
    putchar(IMP_OR_OP);
}
