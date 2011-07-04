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
|  Module: showras.c							|
|									|
|  This program displays data from a Sun raster file in a newly		|
|  created window.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)showras.c	1.1	9/27/89	ESI";
#endif

#include <stdio.h>
#include <suntool/sunview.h>
#include <suntool/canvas.h>

#define Fprintf (void) fprintf
#define SYNTAX \
{ Fprintf(stderr, "Usage: %s\n", "showras [-x debug_level] input.ras"); \
  exit(1); }


struct pixrect	*pr_load();

char		*mktemp();

static void	get_config();

static short    icon_image[] =
    {
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


main(argc, argv)
    int	    argc;
    char    **argv;
{

    extern int	    optind;
    extern char	    *optarg;
    int		    ch;

    int		    debug_level = 0;

    FILE	    *ifile;
    colormap_t	    colormap;
    struct pixrect  *image;
    int		    width, height;
    int		    depth;

    Icon	    icon;

    Frame           frame;
    static char	    *title = "ESPS image plot";
    static char	    *err_msg =
		    "Can't create frame.  Must run under Suntools.";

    Canvas          canvas;
    Pixwin	    *pw;

    int		    avail;

    while ((ch = getopt(argc, argv, "x:")) != EOF)
	switch (ch)
	{
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	default:
	    SYNTAX
	    break;
	}

    if (argc - optind < 1)
    {
	Fprintf(stderr, "%s: no file name.\n", "showras");
	SYNTAX
    }
    if (argc - optind > 1)
    {
	Fprintf(stderr, "%s: too many file names.\n", "showras");
	SYNTAX
    }

    if (strcmp(argv[optind], "-") == 0)
	ifile = stdin;
    else
    {
	ifile = fopen(argv[optind], "r");
	if (!ifile)
	{
	    Fprintf(stderr, "%s: can't open input file \"%s\".\n",
		    "showras", argv[optind]);
	    exit(1);
	}
    }

    image = pr_load(ifile, &colormap);

    icon = icon_create(ICON_IMAGE, &icon_pixrect, 0);

    frame = window_create((Window) NULL, FRAME,
			FRAME_LABEL,    	title,
			FRAME_NO_CONFIRM,	TRUE,
			WIN_ERROR_MSG,		err_msg,
			FRAME_ICON,		icon,
			0);

    width = image->pr_size.x;
    height = image->pr_size.y;
    depth = image->pr_depth;

    canvas = window_create(frame, CANVAS,
			CANVAS_WIDTH,		width,
			CANVAS_HEIGHT,		height,
			WIN_WIDTH,		width,
			WIN_HEIGHT,		height,
			CANVAS_AUTO_SHRINK,	FALSE,
			CANVAS_RETAINED,	TRUE,
			CANVAS_FIXED_IMAGE,	TRUE,
			0);

    window_fit(frame);

    pw = canvas_pixwin(canvas);

    get_config(pw, &avail);

    if (debug_level)
	Fprintf(stderr, "%d bits avail.  image depth %d.\n", avail, depth);

    if (depth != avail && depth != 1)
    {
	Fprintf(stderr,
		"%s: %d-plane image incompatible with %d-plane screen.\n",
		"showras", depth, avail);
	exit(1);
    }

    if (depth != 1 && colormap.type == RMT_EQUAL_RGB)
    {
	pw_setcmsname(pw, mktemp("shrasXXXXXX"));
	pw_putcolormap(pw, 0, colormap.length,
		       colormap.map[0], colormap.map[1], colormap.map[2]);
    }

    pw_rop(pw, 0, 0, width, height, PIX_SRC, image, 0, 0);

    window_main_loop(frame);
}


static void
get_config(pw, avail)
    Pixwin  *pw;
    int	    *avail;
{
    *avail = pw->pw_pixrect->pr_depth;
}
