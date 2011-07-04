/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|   "Copyright (c) 1989 Entropic Speech, Inc.  All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  Module: imdevhp.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)imdevhp.c	1.1	8/10/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define ESC		'\033'
#define FF		'\014'
#define HP_POSITION	"%c*p%dx%dY", ESC
#define HP_RESOLUTION	"%c*t%dR", ESC
#define HP_START_GRAPH	"%c*r1A", ESC
#define HP_TRANS_DATA	"%c*b%dW", ESC
#define HP_END_GRAPH	"%c*rB", ESC
#define HP_PAGE_W	2550
#define HP_PAGE_H	3300
#define HP_LMARG	50
#define HP_TMARG	150

int		abs();

char		*arr_alloc();
void		arr_free();

void		plotimage();

static void	hp_row(), hp_initbits(), hp_plotline();

extern int	debug_level;
extern long	width, height;
extern long	nrows, ncols;
extern long	lmarg, rmarg, tmarg, bmarg;

extern int  	mag;

extern int	oflag;
extern int	dev;
extern int	gray_bits;

extern void	(*dev_initbits)();
extern void	(*dev_row)();
extern void	(*dev_plotline)();

static int	rownum;

static unsigned char
		**bitarray;
static long	bitarrdim[2];
static FILE	*outfile = NULL;


void hp_init()
{
    int		    u, v;

    outfile = stdout;
    dev_row = hp_row;
    dev_initbits = hp_initbits;
    dev_plotline = hp_plotline;

    switch (gray_bits)
    {
    case 1:
	break;
    case 4:
    default:
	Fprintf(stderr, "Only 1 bit per pixel supported so far.");
	exit(1);
	break;
    }

    switch (mag)
    {
    case 1:
    case 2:
    case 3:
    case 4:
	break;
    default:
	Fprintf(stderr,
		"Magnification must be 1, 2, 3, or 4 on HP LaserJet.\n");
	exit(1);
	break;
    }

    bitarrdim[0] = width + lmarg + rmarg;
    bitarrdim[1] = (height + bmarg + tmarg + 7)/8;

    bitarray = (unsigned char **) arr_alloc(2, bitarrdim, CHAR, 0);
    for (v = 0; v < bitarrdim[0]; v++)
	for (u = 0; u < bitarrdim[1]; u++)
	bitarray[v][u] = 0;

    rownum = 0;
    plotimage();
}

static void
hp_initbits()
{
}

static void
hp_row(row)
    char    *row;
{
    int		    u, v;
    unsigned char   b;

    if (oflag)
    {
	u = (height + bmarg - 1 - rownum)/8;
	b = 0x80 >> (height + bmarg - 1 - rownum)%8;
	for (v = 0; v < ncols; v++)
	    if (row[v])
		bitarray[v+lmarg][u] |= b;

/*	for (v = 0; v < ncols; v++)
	    if (row[v])
		bitarray[rownum+tmarg][(v+lmarg)/8]
			|= 0x80 >> ((v+lmarg)%8);
*/
    }
    else
    {
	for (v = 0; v < ncols; v++)
	    if (row[v])
		bitarray[rownum+lmarg][(v+bmarg)/8] |=
		    0x80 >> ((v+bmarg)%8);
/*	u = (rownum + lmarg)/8;
	b = 0x80 >> (rownum + lmarg)%8;
	for (v = 0; v < ncols; v++)
	    if (row[v])
		bitarray[ncols - 1 - v + tmarg][u] |= b;
*/
    }
    rownum++;
}

void
hp_fin()
{
    int	    v;
    int	    h_offset, v_offset;

    h_offset = (HP_PAGE_H - (lmarg + width + rmarg)*mag)/2 - HP_TMARG;
    v_offset = (HP_PAGE_W - (bmarg + height + tmarg)*mag)/2 - HP_LMARG;

    printf(HP_POSITION, v_offset, h_offset);
    printf(HP_RESOLUTION, 300/mag);
    printf(HP_START_GRAPH);

    for (v = 0; v < bitarrdim[0]; v++)
    {
	printf(HP_TRANS_DATA, (int) bitarrdim[1]);
	fwrite((char *) bitarray[v],
		sizeof(bitarray[0][0]), (int) bitarrdim[1], outfile);
    }

    printf(HP_END_GRAPH);
    putchar(FF);
    arr_free((char *) bitarray, 2, CHAR, 0);
}

void
hp_default_size(w, h)
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
hp_plotline(m, hor, ver)
    long    m;
    long    *hor, *ver;
{
    int	    i, j, n;
    int	    d_hor, d_ver;
    double  u, v, du, dv;

    for (i = 1; i < m; i++)
    {
	d_hor = hor[i] - hor[i-1];
	d_ver = ver[i] - ver[i-1];
/*Fprintf(stderr, "d_hor %d, d_ver %d\n", d_hor, d_ver);*/

	v = lmarg + hor[i-1] + 0.5;
	u = bmarg + ver[i-1] + 0.5;
/*Fprintf(stderr, "u %g, v %g\n", u, v);*/

	if (d_hor == 0 && d_ver == 0)
	{
	    du = dv = 0.0;
	    n = 1;
	}
	else if (abs(d_hor) > abs(d_ver))
	{
	    dv = (d_hor > 0) ? 1.0 : -1.0;
	    du = (dv * d_ver) / d_hor;
	    n = 1 + abs(d_hor);
	}
	else
	{
	    du = (d_ver > 0) ? 1.0 : -1.0;
	    dv = (du * d_hor) / d_ver;
	    n = 1 + abs(d_ver);
	}
/*Fprintf(stderr, "du %g, dv %g, n %d\n", du, dv, n);*/
	for (j = 0; j < n; j++)
	{
	    unsigned char   b =	0x80 >> ((int) u)%8;

	    bitarray[(int) v][((int) u)/8] |= b;
/*				(unsigned char) (0x80 >> ((int) u)%8);*/
/*Fprintf(stderr, "(int) u %d, (int) v %d\n", (int) u, (int) v);*/
/*Fprintf(stderr, "((int) u)%%8 %d, b %d\n", ((int) u)%8, b);*/
/*Fprintf(stderr, "bitarray[%d][%d] = %d\n", (int) v, ((int) u)/8, */
/*bitarray[(int) v][((int) u)/8]);*/
	    u += du;
	    v += dv;
	}
    }
}
