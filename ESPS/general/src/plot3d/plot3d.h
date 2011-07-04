/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1990 Entropic Speech, Inc.  All rights reserved."   |
|									|
+-----------------------------------------------------------------------+
|									|
|  Program: plot3d							|
|									|
|  This program makes a 3-d plot (perspective drawing with hidden	|
|  lines removed) of data from a FEA file.				|
|									|
|  Module: plot3d.h							|
|									|
|  Include file.							|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
/* @(#)plot3d.h	1.4	8/6/91	ESI */

#include <xview/cms.h>

/* colors */
#define	COL_PANEL_BG	(0 + CMAP_OFFSET)
#define COL_PANEL_FG	(1 + CMAP_OFFSET)
#define COL_CANVAS_BG   (2 + CMAP_OFFSET)
#define COL_TOP_FG	(3 + CMAP_OFFSET)
#define COL_BOT_FG	(4 + CMAP_OFFSET)
#define COL_BOX_FG	(5 + CMAP_OFFSET)
#define COL_AXIS_FG	(6 + CMAP_OFFSET)
#define COL_ERRMSG_FG	(7 + CMAP_OFFSET)
#define COL_ROBAR_FG	(8 + CMAP_OFFSET)
#define COL_ROBAR_BG	(9 + CMAP_OFFSET)
#define COL_BLACK	(10 + CMAP_OFFSET)
#define COL_WHITE	(11 + CMAP_OFFSET)
#define NUM_P3D_COL	12
#define CMAP_OFFSET	CMS_CONTROL_COLORS

/* boolean functions for line drawing */
#define	BFN_SRC 3
#define BFN_XOR 6

/* line-drawing style */
#define LNS_NONE 0
#define LNS_SOLID 1
#define LNS_DOT 2

/* default field in FEA record */
#define DEF_FIELD "re_spec_val"

/* orientation of coordinate system */
#define ORI_LEFT	0
#define ORI_RIGHT	1

/* height of readout bar at top of window */
#define ROBAR_HEIGHT	20

/* codes: can compute time from rec number & record_freq, tag & sampling freq,
   both, or neither? */
/* use NONE (0) for neither */
#define XTIME_FROM_REC 1
#define XTIME_FROM_TAG 2
#define XTIME_FROM_BOTH 3

typedef struct {double	x, z;}	point;

/* These macros provide a convenient (i.e., after the original design!)
   way to use a global to force a monochrome plot; used to implement 
   the -M option.
 */

#define GET_COL_BOX_FG (force_monochrome_plot ? COL_BLACK : COL_BOX_FG)
#define GET_COL_AXIS_FG (force_monochrome_plot ? COL_BLACK : COL_AXIS_FG)
#define GET_COL_PANEL_FG (force_monochrome_plot ? COL_BLACK : COL_PANEL_FG)
#define GET_COL_CANVAS_BG (force_monochrome_plot ? COL_WHITE : COL_CANVAS_BG)
#define GET_COL_TOP_FG (force_monochrome_plot ? COL_BLACK : COL_TOP_FG)
#define GET_COL_BOT_FG (force_monochrome_plot ? COL_BLACK : COL_BOT_FG)

