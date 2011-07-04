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
|  Module: imdevps.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)imdevps.c	1.1	7/25/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include "image.h"

/*#define PS_PAGE_W	2550*/
/*#define PS_PAGE_H	3300*/

extern long	lmarg, rmarg, tmarg, bmarg;

extern int	mag;

char		*arr_alloc();
void		arr_free();

void		plotimage();

static void	ps_row(), ps_initbits(), ps_plotline();

extern int	debug_level;
extern long	width, height;
extern long	nrows, ncols;

extern int	oflag;
extern int	dev;
extern int	gray_bits;

extern void	(*dev_initbits)();
extern void	(*dev_row)();
extern void	(*dev_plotline)();

static int	rownum;


void
ps_init()
{
    int	    u, v;

    dev_row = ps_row;
    dev_initbits = ps_initbits;
    dev_plotline = ps_plotline;

    if (gray_bits > 1)
    {
	Fprintf(stderr,
	    "multilevel gray scale in PostScript not supported.\n");
	exit(1);
    }

    if (debug_level >= 1)
    {
	Fprintf(stderr,
		"ps_init: %s %ld, %s %ld, %s %ld, %s %ld\n",
		"lmarg", lmarg, "rmarg", rmarg,
		"tmarg", tmarg, "bmarg", bmarg);
	Fprintf(stderr, "\t%s %ld, %s %ld, %s %ld, %s %ld\n",
		"width", width, "height", height,
		"nrows", nrows, "ncols", ncols);
    }

    printf("%%!PS-Adobe-1.0\n");
    printf("%%%%DocumentFonts:\n");
    printf("-90 rotate\n");
    printf("gsave\n");
    printf("    clippath pathbbox\n");
    printf("grestore\n");
    printf("    3 -1 roll add 2 div\n");
    printf("    3 1 roll add 2 div exch\n");
    printf("translate\n");
    printf("matrix currentmatrix\n");
    printf("dup 0 4 getinterval\n");
    printf("dup {\n");
    printf("    dup abs 0.01 lt {\n");
    printf("        pop 0\n");
    printf("    } {\n");
    printf("        %d exch\n", 1);
    printf("        0 lt {neg} if\n");
    printf("    } ifelse\n");
    printf("} forall\n");
    printf("5 -1 roll astore pop\n");
    printf("dup setmatrix\n");
    printf("%d %d translate\n",
	   mag * (lmarg - width - rmarg)/2, mag * (bmarg - height - tmarg)/2);
    printf("currentmatrix\n");
    printf("dup 4 2 getinterval\n");
    printf("dup {round} forall\n");
    printf("3 -1 roll astore pop\n");
    printf("setmatrix\n");
    printf("%%%%EndProlog\n");

    rownum = 0;
    plotimage();
}

static void
ps_initbits()
{
    if (debug_level >= 1)
	Fprintf(stderr, "ps_initbits:\n");

    printf("%d %d scale\n", mag, mag);

    printf("/hexstr %ld string def\n", (long) (ncols + 7)/8);
    printf("/dotimage {\n");
    if (oflag)
	printf("    %ld %ld 1 [1 0 0 -1 %g %g]\n",
					ncols, nrows, 0.5, nrows - 0.5);
    else
	printf("    %ld %ld 1 [0 1 1 0 %g %g]\n", ncols, nrows, 0.5, 0.5);
    printf("    {currentfile hexstr readhexstring pop}\n");
    printf("    image\n");
    printf("} def\n");

    printf("dotimage\n");
}

static char invhex[] = "fedcba9876543210";

static void
ps_row(row)
    char    *row;
{
    int		u, v;

    if (debug_level >= 2)
	Fprintf(stderr, "ps_row: rownum %d\n", rownum);

    if (debug_level >= 3)
    {
	for (v = 0; v < ncols; v++)
	    Fprintf(stderr, "[%d] %d  ", v, row[v]);
	Fprintf(stderr, "\n");
    }

    for (v = 3; v < ncols; v += 4)
    {
	putchar(invhex[(row[v-3]<<3) + (row[v-2]<<2) + (row[v-1]<<1) + row[v]]);
    }
    switch (v - ncols)
    {
    case 3:
	break;
    case 2:
	putchar(invhex[row[v-3]<<3]);
	break;
    case 1:
	putchar(invhex[(row[v-3]<<3) + (row[v-2]<<2)]);
	break;
    case 0:
	putchar(invhex[(row[v-3]<<3) + (row[v-2]<<2) + (row[v-1]<<1)]);
	break;
    }
    if (((ncols+3)/4)%2)	/* Odd number of hex digits written */
	putchar('f');		/* Pad to fill out byte */
    putchar('\n');

    rownum++;	    
}

void
ps_fin()
{
    if (debug_level >= 1)
	Fprintf(stderr, "ps_fin: rownum %d\n", rownum);

    printf("showpage\n");
    printf("%%%%Trailer\n");
}

void
ps_default_size(w, h)
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

    if (debug_level >= 1)
	Fprintf(stderr, "ps_default_size: *w %ld, *h %ld\n", *w, *h);
}

static void
ps_plotline(n, h, v)
    long    n;
    long    *h, *v;
{
    int	    i;

    if (debug_level >= 2)
	Fprintf(stderr, "ps_plotline: n %ld\n", n);

    printf("newpath\n");
    printf("%d %d moveto\n", mag*h[0], mag*v[0]);
    for (i = 1; i < n; i++)
	printf("%d %d lineto\n", mag*h[i], mag*v[i]);
    printf("stroke\n");
}
