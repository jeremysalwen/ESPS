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
|  Module: imdevtyp.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)imdevtyp.c	1.2	7/25/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include "image.h"

char		*arr_alloc();
void		arr_free();

void		plotimage();

static void	typ_row(), typ_initbits(), typ_plotline();

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
typ_init()
{
    dev_row = typ_row;
    dev_initbits = typ_initbits;
    dev_plotline = typ_plotline;

    if (gray_bits > 1)
    {
	Fprintf(stderr,
	    "multilevel gray scale not supported in typewriter plots.\n");
	exit(1);
    }

    plotimage();
}

static void
typ_initbits()
{
    
}

static void
typ_row(row)
    char    *row;
{
    int	    v;

    for (v = 0; v < ncols; v++)
	putchar(row[v] ? '*' : ' ');
    putchar('\n');
}


void
typ_fin()
{
}


void
typ_default_size(w, h)
    long    *w, *h;
{
    *w = 60;
    *h = 40;
}

static void
typ_plotline(n, h, v)
    long    n;
    long    *h, *v;
{
    
}
