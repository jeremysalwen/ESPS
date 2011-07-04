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
|  Module: imdevras.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)imdevras.c	1.2	7/25/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define	RAS_MAGIC   0x59a66a95
#define	RT_STANDARD 1
#define	RMT_NONE    0


int		abs();

char		*arr_alloc();
void		arr_free();

void		plotimage();

static void	ras_row(), ras_initbits(), ras_plotline();
static void	put_long();

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

static int	rownum;

static unsigned char
		**bitarray;
static long	bitarrdim[2];
static FILE	*outfile = NULL;


void ras_init()
{
    unsigned long   ras_magic, ras_width, ras_height, ras_depth;
    unsigned long   ras_length, ras_type, ras_maptype, ras_maplength;
    int		    u, v;

    outfile = stdout;
    dev_row = ras_row;
    dev_initbits = ras_initbits;
    dev_plotline = ras_plotline;

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

    ras_magic = RAS_MAGIC;
    ras_width = width + lmarg + rmarg;
    ras_height = height + bmarg + tmarg;
    ras_depth = 1;

    bitarrdim[0] = ras_height;
    bitarrdim[1] = 2 * ((ras_width*ras_depth + 15)/16);

    ras_length = bitarrdim[0] * bitarrdim[1];
    ras_type = RT_STANDARD;
    ras_maptype = RMT_NONE;
    ras_maplength = 0;

    bitarray = (unsigned char **) arr_alloc(2, bitarrdim, CHAR, 0);
    for (v = 0; v < bitarrdim[0]; v++)
	for (u = 0; u < bitarrdim[1]; u++)
	bitarray[v][u] = 0;

    put_long(ras_magic);
    put_long(ras_width);
    put_long(ras_height);
    put_long(ras_depth);
    put_long(ras_length);
    put_long(ras_type);
    put_long(ras_maptype);
    put_long(ras_maplength);

    rownum = 0;
    plotimage();
}

static void
ras_initbits()
{
}

static void
ras_row(row)
    char    *row;
{
    int		    u, v;
    unsigned char   b;

    if (oflag)
    {
	for (v = 0; v < ncols; v++)
	    if (row[v])
		bitarray[rownum+tmarg][(v+lmarg)/8]
			|= 0x80 >> ((v+lmarg)%8);
    }
    else
    {
	u = (rownum + lmarg)/8;
	b = 0x80 >> (rownum + lmarg)%8;
	for (v = 0; v < ncols; v++)
	    if (row[v])
		bitarray[ncols - 1 - v + tmarg][u] |= b;
    }
    rownum++;
}

void
ras_fin()
{
    int	    v;

    for (v = 0; v < bitarrdim[0]; v++)
	fwrite((char *) bitarray[v],
		sizeof(bitarray[0][0]), (int) bitarrdim[1], outfile);

    arr_free((char *) bitarray, 2, CHAR, 0);
}

void
ras_default_size(w, h)
    long    *w, *h;
{
    if (oflag)
    {
	*w = 150;
	*h = 600;
    }
    else
    {
	*w = 600;
	*h = 150;
    }
}

static void
ras_plotline(m, hor, ver)
    long    m;
    long    *hor, *ver;
{
    int	    i, j, n;
    int	    h_off, v_off;
    int	    d_hor, d_ver;
    double  u, v, du, dv;

    h_off = lmarg;
    v_off = height - 1 + tmarg;

    for (i = 1; i < m; i++)
    {
	d_hor = hor[i] - hor[i-1];
	d_ver = ver[i] - ver[i-1];
/*Fprintf(stderr, "d_hor %d, d_ver %d\n", d_hor, d_ver);*/

	u = h_off + hor[i-1] + 0.5;
	v = v_off - ver[i-1] + 0.5;
/*Fprintf(stderr, "u %g, v %g\n", u, v);*/

	if (d_hor == 0 && d_ver == 0)
	{
	    du = dv = 0.0;
	    n = 1;
	}
	else if (abs(d_hor) > abs(d_ver))
	{
	    du = (d_hor > 0) ? 1.0 : -1.0;
	    dv = - (du * d_ver) / d_hor;
	    n = 1 + abs(d_hor);
	}
	else
	{
	    dv = (d_ver > 0) ? -1.0 : 1.0;
	    du = - (dv * d_hor) / d_ver;
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

static void
put_long(x)
    unsigned long   x;
{
    unsigned char   u[4];

    u[0] = x >> 24;
    u[1] = 0xff & (x >> 16);
    u[2] = 0xff & (x >> 8);
    u[3] = 0xff & x;
    fwrite((char *) u, sizeof(u[0]), 4, outfile);
}
