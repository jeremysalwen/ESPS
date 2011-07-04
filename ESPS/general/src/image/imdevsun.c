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
|  Module: imdevsun.c							|
|									|
|  This program displays data from an ESPS file as a half-tone		|
|  gray-scale image.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifdef SUNVUE

#ifndef lint
static char *sccs_id = "@(#)imdevsun.c	1.5	4/4/90	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>
#include "image.h"

char		*arr_alloc();
void		arr_free();

void		plotimage();
void		set_margins();

static void	get_config(), setcmap();
static void	mcd_row(), mcd_initbits(), mcd_plotline();

static int	repaint(), resize();

extern int	debug_level;
extern long	width, height;
extern long	nrows, ncols;
extern long	lmarg, rmarg, tmarg, bmarg;
extern int	scale;

extern int	oflag;
extern int	dev;
extern int	gray_bits;
extern int	Bflag;

extern void	(*dev_initbits)();
extern void	(*dev_row)();
extern void	(*dev_plotline)();

static int	rownum;

static char	*title = "ESPS image plot";

static Pixwin	*pw;
static short	icon_image[] = {
/* Format_version=1, Width=64, Height=64, Depth=1, Valid_bits_per_item=16
 */
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x7E3C,0x7C3C,0x0001,0x8000,0x4042,0x4242,0x0001,
	0x8000,0x4040,0x4240,0x0001,0x8000,0x7C3C,0x423C,0x0001,
	0x8000,0x4002,0x7C02,0x0001,0x8000,0x4002,0x4002,0x0001,
	0x8000,0x4042,0x4042,0x0001,0x8000,0x7E3C,0x403C,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x81FF,0xFFFF,0xFFFF,0xFF81,
	0x8100,0x88AA,0xAB77,0x7F81,0x8102,0x2215,0x55DD,0xFF81,
	0x8100,0x446A,0xABBB,0xBF81,0x8101,0x1115,0x56EE,0xFF81,
	0x8100,0x88AA,0xAB77,0x7F81,0x8102,0x2215,0x55DD,0xFF81,
	0x8100,0x446A,0xABBB,0xBF81,0x8101,0x1115,0x56EE,0xFF81,
	0x8100,0x88AA,0xAB77,0x7F81,0x8102,0x2215,0x55DD,0xFF81,
	0x8100,0x446A,0xABBB,0xBF81,0x8101,0x1115,0x56EE,0xFF81,
	0x8100,0x88AA,0xAB77,0x7F81,0x8102,0x2215,0x55DD,0xFF81,
	0x8100,0x446A,0xABBB,0xBF81,0x8101,0x1115,0x56EE,0xFF81,
	0x8100,0x88AA,0xAB77,0x7F81,0x8102,0x2215,0x55DD,0xFF81,
	0x8100,0x446A,0xABBB,0xBF81,0x8101,0x1115,0x56EE,0xFF81,
	0x8100,0x88AA,0xAB77,0x7F81,0x8102,0x2215,0x55DD,0xFF81,
	0x8100,0x446A,0xABBB,0xBF81,0x8101,0x1115,0x56EE,0xFF81,
	0x81FF,0xFFFF,0xFFFF,0xFF81,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0x8000,0x0000,0x0000,0x0001,
	0x8000,0x0000,0x0000,0x0001,0xFFFF,0xFFFF,0xFFFF,0xFFFF
};
mpr_static(icon_pixrect, 64, 64, 1, icon_image);


int
mcd_init()
{
    int		    avail_bits;
    int	    	    u, v;
    int		    frame_width_adj, frame_height_adj;
    Frame           frame;
    Canvas          canvas;
    Icon	    icon;
    static char	    *err_msg =
		    "Can't create frame.  Must run under Suntools.";

    dev_row = mcd_row;
    dev_initbits = mcd_initbits;
    dev_plotline = mcd_plotline;

    icon = icon_create(ICON_IMAGE, &icon_pixrect, 0);

    frame = window_create((Window) NULL, FRAME,
			FRAME_LABEL,    	title,
			FRAME_NO_CONFIRM,	TRUE,
			WIN_ERROR_MSG,		err_msg,
			FRAME_ICON,		icon,
			0);

    if(frame == NULL) return 0;

    canvas = window_create(frame, CANVAS,
			CANVAS_RETAINED,	FALSE,
			CANVAS_FIXED_IMAGE,	FALSE,
			CANVAS_REPAINT_PROC,	repaint,
			CANVAS_RESIZE_PROC,	resize,
			0);

    frame_width_adj = (int) window_get(frame, WIN_WIDTH)
				- (int) window_get(canvas, WIN_WIDTH);
    frame_height_adj = (int) window_get(frame, WIN_HEIGHT)
				- (int) window_get(canvas, WIN_HEIGHT);

    window_set(frame, 
	WIN_WIDTH,	(int) (width + lmarg + rmarg + frame_width_adj),
	WIN_HEIGHT,	(int) (height + tmarg + bmarg + frame_height_adj),
	0);

    if (debug_level)
	Fprintf(stderr, "Frame dimensions %d x %d\n",
			width + lmarg + rmarg + frame_width_adj,
			height + tmarg + bmarg + frame_height_adj);

    pw = canvas_pixwin(canvas);

    get_config(&avail_bits);

    if (avail_bits < gray_bits)
    {
	Fprintf(stderr, "%s: %s - exiting\n", "mcd_init",
		"system hasn't enough gray-scale resolution for algorithm");
	exit(1);
    }

    if (debug_level)
	Fprintf(stderr, "System has %d bit gray scale.\n", avail_bits);

    setcmap(gray_bits);
    window_set(canvas,
		CANVAS_RETAINED,	TRUE,
		0);
    window_main_loop(frame);
    return 1;
}

static void
mcd_initbits()
{
    
}

static
resize(canvas, width, height)
    Canvas  canvas;
    int	    width, height;
{
    window_set(canvas,	CANVAS_REPAINT_PROC,	repaint,
			0);
}

static
repaint(canvas, pixwin, repaint_area)
    Canvas	canvas;
    Pixwin	*pixwin;
    Rectlist	*repaint_area;
{
    long    can_width, can_height;
    int	    window_too_small = NO;

    can_width = (int) window_get(canvas, CANVAS_WIDTH);
    can_height = (int) window_get(canvas, CANVAS_HEIGHT);
    width = can_width - lmarg - rmarg;
    height = can_height - tmarg - bmarg;
    if (Bflag)
    {
	window_too_small = width < 2 || height < 2;
	if (window_too_small)
	{
	    width = can_width;	height = can_height;
	    Bflag = NO;
	    lmarg = rmarg = tmarg = bmarg = 0;
	}
    }
    nrows = oflag ? height : width;
    ncols = oflag ? width : height;

    if (debug_level >= 2)
	Fprintf(stderr,
	    "Repainting.  %ld \"rows\".  %ld \"cols\".\n", nrows, ncols);

    pw_writebackground(pw, 0, 0, can_width, can_height, PIX_SRC);

    rownum = 0;
    plotimage();
    window_set(canvas,	CANVAS_REPAINT_PROC,	(int (*)()) NULL,
		    0);

    if (window_too_small)
    {
	window_too_small = NO;
	Bflag = YES;
	set_margins(Bflag, scale, &lmarg, &rmarg, &tmarg, &bmarg);
    }
}

static void
mcd_row(row)
    char    *row;
{
    int		u, v;

    Rect    rect;

    rect.r_left = 0;
    rect.r_top = 0;
    rect.r_width = width + lmarg + rmarg;
    rect.r_height = height + tmarg + bmarg;

    pw_batch_on(pw);
/*
    pw_lock(pw, &rect);
*/
    if (oflag)
    {
	for (v = 0; v < ncols; v++)
	    pw_put(pw, (int) (v + lmarg), (int) (rownum + tmarg), row[v]);
    }
    else
    {
	for (v = 0; v < ncols; v++)
	    pw_put(pw, (int) (rownum + lmarg),
				    (int) (tmarg + ncols - v - 1), row[v]);
    }
/*
    pw_unlock(pw);
*/
    pw_batch_off(pw);
    rownum++;
}

void
mcd_fin()
{
}

void
mcd_default_size(w, h)
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

int
mcd_depth()
{
    Frame   frame;
    Canvas  canvas;
    int	    avail_bits;

    frame = window_create((Window) NULL, FRAME, 
			FRAME_NO_CONFIRM,	TRUE,
			0);
    if (frame == NULL) return 0;
    canvas = window_create(frame, CANVAS, 0);
    avail_bits = (canvas_pixwin(canvas))->pw_pixrect->pr_depth;
    window_destroy(canvas);
    window_destroy(frame);
    return avail_bits;
}
    
static void
mcd_plotline(n, h, v)
    long    n;
    long    *h, *v;
{
    int	    i;
    int	    h_off, v_off;

    h_off = lmarg;
    v_off = height - 1 + tmarg;

    for (i = 1; i < n; i++)
    {
	pw_vector(pw,
		h_off + (int) h[i-1],	v_off - (int) v[i-1],
		h_off + (int) h[i],	v_off - (int) v[i],
		PIX_SRC, 15);
    }
}

static void
setcmap(n)
    int	    n;
{
    extern int	(*cmap)[3];	/* array of RGB triples */
    extern int	cmap_len;	/* number of colormap entries */
    char	*mktemp(), *savestring();
    int		i;

    switch (n)
    {
    case 1:
	break;
    case 4:
	{
	    unsigned char	    r[16], g[16], b[16];
	    static unsigned char    rgb[16] =
		{ 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
		  0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00 };

	    pw_setcmsname(pw, mktemp(savestring("imageXXXXXX")));

	    if (cmap && cmap_len == 16)
	    {
		for (i = 0; i < 16; i++)
		{
		    r[i] = cmap[i][0];
		    g[i] = cmap[i][1];
		    b[i] = cmap[i][2];
		}
		pw_putcolormap(pw, 0, 16, r, g, b);
	    }
	    else
	    {
		pw_putcolormap(pw, 0, 16, rgb, rgb, rgb);
	    }
	}
	break;
    default:    
	break;
    }
}

static void
get_config(avail)
    int	    *avail;
{
    *avail = pw->pw_pixrect->pr_depth;
}

#endif
