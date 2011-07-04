/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc.
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  Rodney W. Johnson, Entropic Speech, Inc.
 * Checked by:
 * Revised by:
 *
 * Display interactive X window and handle interactions.
 *
 */

static char *sccs_id = "@(#)p3dinter.c	1.28	11/11/94	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/epaths.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/canvas.h>
#include <xview/svrimage.h>
#include <xview/xv_xrect.h>
#include <xview/cms.h>
#include <xview/scrollbar.h>
#include <xview/font.h>
#include <xview/notice.h>
#include <xview/cursor.h>
#include <xview/svrimage.h>
#include <esps/constants.h>
#include <esps/exview.h>
#include "plot3d.h"

#define _NO_PROTO
#include <X11/Intrinsic.h>
#include <X11/Shell.h>


extern int              data_loaded;
extern char		*ProgName;
extern int  		debug_level;
extern int              force_monochrome_plot;
extern char		*iname;		/* input file name */

static Xv_Window	paintwin;
static Display		*display;
static Window		xwin;
static GC		gc;
static Xv_Font		font;
int			do_color;
int			want_box;
static int		want_axes, want_cpanel, want_plot;
static unsigned long	*xcolors;

static int 	color_status();

static void	sca_notify();
static void	ori_notify();
static void	len_notify(),	dep_notify(),	rot_notify();
static void	wid_notify(),	hsk_notify(),	bea_notify();
static void	hgt_notify(),	vsk_notify(),	ele_notify();

static int	plot_notify(),	box_notify(),	axis_notify(),	clea_notify();
static int	hcop_notify(),	help_notify(),	quit_notify();
static int	file_notify(),  screen_plot_notify(), hardcp_plot_notify();
static int	hardcp_setup_notify();
static Xv_opaque
		plot_item_notify(),
		box_item_notify(),
		axis_item_notify(),
		clea_item_notify(),
		x_rnum_notify(), x_rtime_notify(), x_tag_notify(),
			x_ttime_notify(), x_other_notify(),
		y_inum_notify(), y_freq_notify(), y_other_notify();
void            no_data_notice(),
		xprinter_error_notice();
extern Xv_opaque
		load_data_notify(),
		load_params_notify(),
		enable_waves_cursors(),
		disable_waves_cursors(),
		save_params_notify();
extern void	show_print_window();
		
extern void	init_file_windows();
extern void     init_print_window();
extern void	init_readout_bar();

static void	init_graphics();
static void	repaint_proc();
static void	resize();
static void	canv_event_proc();
void		repaint();
void		clear_window();
void		do_box(), do_axes();
static void	do_plot();
void		string_extents();
extern void	clear_readouts(), post_readouts();

extern void	set_canv_dimens();
extern void	set_box_len(),	set_box_wid(),	set_box_hgt();
extern int	get_box_len(),	get_box_wid(),	get_box_hgt();
extern void	set_hskew(),	set_vskew(),	set_finv();
extern double	get_hskew(),	get_vskew(),	get_finv();
extern void	set_ori();
extern int	get_ori();
extern void	set_rot(),	set_bear(),	set_elev();
extern double	get_rot(),	get_bear(),	get_elev();
extern void	draw_axes();
extern void	draw_box();
extern void	make_rmat(),	update_rmat();
extern void	draw_plot();
extern void	draw_testplot();
extern void	draw_pointplot();

extern void	xyz_vals();

Frame		    canvas_frame;
static Canvas	    canvas;
static Menu	    canv_menu;
static Scrollbar    h_scrollbar, v_scrollbar;

Frame		    panel_frame=XV_NULL;
Panel	            panel;
Panel		    print_setup_panel;
static Panel_item   sca_slider;
static Panel_item   file_button;
static Menu	    file_menu;
Panel_item	    ori_setting;
Panel_item	    len_slider, dep_slider, rot_slider;
Panel_item	    wid_slider, hsk_slider, bea_slider;
Panel_item	    hgt_slider, vsk_slider, ele_slider;
static Panel_item   plot_button, box_button;
static Panel_item   axis_button;
static Menu	    axis_menu, rec_ax_men_gen(), itm_ax_men_gen();
static Menu	    plot_menu;
static Xv_opaque    axes_toggle_notify();
static Panel_item   clea_button;
static Panel_item   hcop_button, help_button, quit_button;
Menu_item    	    waves_on, waves_off;
int		    xv_not_running=1;
extern int	    send_to_waves;

static Xv_Cursor    default_cursor, blank_cursor;
XFontStruct	    *Xpxfs;	

#define HELPFILE_LEN 100
static char 	    man_page[HELPFILE_LEN];
#define HELPFILE "/plot3d.man"

static short cursor_bits[] = {
	/* Format_version=1, Width=16, Height=16, Depth=1, Valid_bits_per_item=16
	0x0000, 0x03E0, 0x0C18, 0x1004, 0x2006, 0x201A, 0x4021, 0x40C1,
	0x4101, 0x4201, 0x4C01, 0x3002, 0x2002, 0x1004, 0x0C18, 0x03E0 */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
	};

void
gr_init(ac, av)
    int	    *ac;
    char    **av;
{
    xv_init(XV_INIT_ARGC_PTR_ARGV, ac, av, 0);

}


void
set_frame_title(name)
    char    *name;
{
    xv_set(canvas_frame,
			XV_LABEL,		    name,
			0);
}

void
interact(i_axes, i_box, i_cpanel, i_plot)
    int		    i_axes, i_box, i_cpanel, i_plot;
{
    Cms		    cms;
    static Xv_singlecolor  ffg = {0x00, 0x00, 0x00};
    static Xv_singlecolor  fbg = {0xf0, 0xff, 0xff};

#define SPSLIB (char*)build_esps_path("lib")

    if ( (strlen(SPSLIB)+strlen(HELPFILE)+1) > HELPFILE_LEN)
	(void)strcpy(man_page,(char *)FIND_ESPS_LIB(NULL,"plot3d.man"));
    else {
        (void)strcpy(man_page,SPSLIB);
        (void)strcat(man_page,HELPFILE);
    }


    want_axes = i_axes;
    want_box = i_box;
    want_cpanel = i_cpanel;
    want_plot = i_plot;

    /* This call seems to make the system observe the -Wp xview option
       correctly.  Without it, some systems (e.g. Indigo) just ignore -Wp and
       -WG! */
    (void)xv_find(XV_NULL, FONT,
		  FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
		  0);

    /* Create control panel */

    if (i_cpanel) {	/* Create panel frame */
	
	panel_frame = xv_create(XV_NULL, FRAME,
				XV_LABEL,    "ESPS 3-D Plot Conrol Panel",
				XV_X,		    0,
				XV_Y,		    0,
				XV_WIDTH,		    929,
				XV_HEIGHT,		    212,
				FRAME_CLOSED,	 	    FALSE,
				FRAME_SHOW_FOOTER,	    FALSE,
				0);

	(void) exv_attach_icon(panel_frame, P3D_ICON, "plot3d", TRANSPARENT);

	/* Create panel. */
	
	panel = xv_create(panel_frame, PANEL,
			  XV_X,			    0,
			  XV_Y,			    0,
			  XV_WIDTH,		    WIN_EXTEND_TO_EDGE,
			  XV_HEIGHT,		    WIN_EXTEND_TO_EDGE,
			  OPENWIN_SHOW_BORDERS,   FALSE,
			  0);
	
	/* Was setting background on control panel.  Violates Open
	   Look GUI standard and led to smeared tracks of a different
	   color when sliders were moved. */
	/*
	  if (do_color)
	  xv_set(panel,
	  WIN_CMS,		    cms,
	  WIN_FOREGROUND_COLOR,	    COL_PANEL_FG,
	  WIN_BACKGROUND_COLOR,	    COL_PANEL_BG,
	  0);
	  */
	
	/* Create choice item. */

	ori_setting = xv_create(panel, PANEL_CHOICE,
				XV_X,			    649,
				XV_Y,			    15,
				XV_WIDTH,		    170,
				XV_HEIGHT,		    21,
				PANEL_VALUE_X,		    733,
				PANEL_VALUE_Y,		    15,
				PANEL_LAYOUT,		    PANEL_HORIZONTAL,
				PANEL_CHOICE_STRINGS,
				"Left",
				"Right",
				0,
				PANEL_VALUE,		    (get_ori() == ORI_LEFT)
				? 0 : 1,
				PANEL_LABEL_STRING,	    "orientation",
				PANEL_NOTIFY_PROC,	    ori_notify,
				0);
	
	/* Create sliders. */
	
	sca_slider = xv_create(panel, PANEL_SLIDER,
			       XV_X,			    18,
			       XV_Y,			    19,
			       XV_WIDTH,		    252,
			       XV_HEIGHT,		    15,
			       PANEL_VALUE_X,		    62,
			       PANEL_VALUE_Y,		    19,
			       PANEL_SLIDER_WIDTH,	    100,
			       PANEL_LABEL_STRING,	    "scale",
			       PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			       PANEL_SHOW_RANGE,	    TRUE,
			       PANEL_SHOW_VALUE,	    TRUE,
			       PANEL_MIN_VALUE,	    0,
			       PANEL_MAX_VALUE,	    500,
			       PANEL_VALUE,		    100,
			       PANEL_NOTIFY_LEVEL,	    PANEL_ALL,
			       PANEL_NOTIFY_PROC,	    sca_notify,
			       0);
	
	len_slider = xv_create(panel, PANEL_SLIDER,
			       XV_X,			    10,
			       XV_Y,			    57,
			       XV_WIDTH,		    260,
			       XV_HEIGHT,		    15,
			       PANEL_VALUE_X,		    62,
			       PANEL_VALUE_Y,		    57,
			       PANEL_SLIDER_WIDTH,	    100,
			       PANEL_LABEL_STRING,	    "length",
			       PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			       PANEL_SHOW_RANGE,	    TRUE,
			       PANEL_SHOW_VALUE,	    TRUE,
			       PANEL_MIN_VALUE,	    0,
			       PANEL_MAX_VALUE,	    500,
			       PANEL_VALUE,		    get_box_len(),
			       PANEL_NOTIFY_LEVEL,	    PANEL_ALL,
			       PANEL_NOTIFY_PROC,	    len_notify,
			       0);
	
	dep_slider = xv_create(panel, PANEL_SLIDER,
			       XV_X,			    328,
			       XV_Y,			    57,
			       XV_WIDTH,		    255,
			       XV_HEIGHT,		    15,
			       PANEL_VALUE_X,		    375,
			       PANEL_VALUE_Y,		    57,
			       PANEL_SLIDER_WIDTH,	    100,
			       PANEL_LABEL_STRING,	    "depth",
			       PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			       PANEL_SHOW_RANGE,	    TRUE,
			       PANEL_SHOW_VALUE,	    TRUE,
			       PANEL_MIN_VALUE,	    0,
			       PANEL_MAX_VALUE,	    500,
			       PANEL_VALUE,		    ROUND(get_finv()),
			       PANEL_NOTIFY_LEVEL,	    PANEL_ALL,
			       PANEL_NOTIFY_PROC,	    dep_notify,
			       0);
	
	rot_slider = xv_create(panel, PANEL_SLIDER,
			       XV_X,			    631,
			       XV_Y,			    57,
			       XV_WIDTH,		    282,
			       XV_HEIGHT,		    15,
			       PANEL_VALUE_X,		    693,
			       PANEL_VALUE_Y,		    57,
			       PANEL_SLIDER_WIDTH,	    96,
			       PANEL_LABEL_STRING,	    "rotation",
			       PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			       PANEL_SHOW_RANGE,	    TRUE,
			       PANEL_SHOW_VALUE,	    TRUE,
			       PANEL_MIN_VALUE,	    -180,
			       PANEL_MAX_VALUE,	    180,
			       PANEL_VALUE,		    ROUND(180.0/PI*get_rot()),
			       PANEL_NOTIFY_LEVEL,	    PANEL_ALL,
			       PANEL_NOTIFY_PROC,	    rot_notify,
			       0);
	
	wid_slider = xv_create(panel, PANEL_SLIDER,
			       XV_X,			    16,
			       XV_Y,			    95,
			       XV_WIDTH,		    254,
			       XV_HEIGHT,		    15,
			       PANEL_VALUE_X,		    62,
			       PANEL_VALUE_Y,		    95,
			       PANEL_SLIDER_WIDTH,	    100,
			       PANEL_LABEL_STRING,	    "width",
			       PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			       PANEL_SHOW_RANGE,	    TRUE,
			       PANEL_SHOW_VALUE,	    TRUE,
			       PANEL_MIN_VALUE,	    0,
			       PANEL_MAX_VALUE,	    500,
			       PANEL_VALUE,		    get_box_wid(),
			       PANEL_NOTIFY_LEVEL,	    PANEL_ALL,
			       PANEL_NOTIFY_PROC,	    wid_notify,
			       0);
	
	hsk_slider = xv_create(panel, PANEL_SLIDER,
			       XV_X,			    307,
			       XV_Y,			    95,
			       XV_WIDTH,		    284,
			       XV_HEIGHT,		    15,
			       PANEL_VALUE_X,		    367,
			       PANEL_VALUE_Y,		    95,
			       PANEL_SLIDER_WIDTH,	    100,
			       PANEL_LABEL_STRING,	    "H. skew",
			       PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			       PANEL_SHOW_RANGE,	    TRUE,
			       PANEL_SHOW_VALUE,	    TRUE,
			       PANEL_MIN_VALUE,	    -200,
			       PANEL_MAX_VALUE,	    200,
			       PANEL_VALUE,		    ROUND(100.0*get_hskew()),
			       PANEL_NOTIFY_LEVEL,	    PANEL_ALL,
			       PANEL_NOTIFY_PROC,	    hsk_notify,
			       0);
	
	bea_slider = xv_create(panel, PANEL_SLIDER,
			       XV_X,			    632,
			       XV_Y,			    95,
			       XV_WIDTH,		    281,
			       XV_HEIGHT,		    15,
			       PANEL_VALUE_X,		    693,
			       PANEL_VALUE_Y,		    95,
			       PANEL_SLIDER_WIDTH,	    96,
			       PANEL_LABEL_STRING,	    "bearing",
			       PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			       PANEL_SHOW_RANGE,	    TRUE,
			       PANEL_SHOW_VALUE,	    TRUE,
			       PANEL_MIN_VALUE,	    -180,
			       PANEL_MAX_VALUE,	    180,
			       PANEL_VALUE,		    ROUND(180.0/PI*get_bear()),
			       PANEL_NOTIFY_LEVEL,	    PANEL_ALL,
			       PANEL_NOTIFY_PROC,	    bea_notify,
			       0);
	
	hgt_slider = xv_create(panel, PANEL_SLIDER,
			       XV_X,			    10,
			       XV_Y,			    133,
			       XV_WIDTH,		    260,
			       XV_HEIGHT,		    15,
			       PANEL_VALUE_X,		    62,
			       PANEL_VALUE_Y,		    133,
			       PANEL_SLIDER_WIDTH,	    100,
			       PANEL_LABEL_STRING,	    "height",
			       PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			       PANEL_SHOW_RANGE,	    TRUE,
			       PANEL_SHOW_VALUE,	    TRUE,
			       PANEL_MIN_VALUE,	    0,
			       PANEL_MAX_VALUE,	    500,
			       PANEL_VALUE,		    get_box_hgt(),
			       PANEL_NOTIFY_LEVEL,	    PANEL_ALL,
			       PANEL_NOTIFY_PROC,	    hgt_notify,
			       0);
	
	vsk_slider = xv_create(panel, PANEL_SLIDER,
			       XV_X,			    308,
			       XV_Y,			    133,
			       XV_WIDTH,		    283,
			       XV_HEIGHT,		    15,
			       PANEL_VALUE_X,		    367,
			       PANEL_VALUE_Y,		    133,
			       PANEL_SLIDER_WIDTH,	    100,
			       PANEL_LABEL_STRING,	    "V. skew",
			       PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			       PANEL_SHOW_RANGE,	    TRUE,
			       PANEL_SHOW_VALUE,	    TRUE,
			       PANEL_MIN_VALUE,	    -200,
			       PANEL_MAX_VALUE,	    200,
			       PANEL_VALUE,		    ROUND(100.0*get_vskew()),
			       PANEL_NOTIFY_LEVEL,	    PANEL_ALL,
			       PANEL_NOTIFY_PROC,	    vsk_notify,
			       0);
	
	ele_slider = xv_create(panel, PANEL_SLIDER,
			       XV_X,			    629,
			       XV_Y,			    133,
			       XV_WIDTH,		    276,
			       XV_HEIGHT,		    15,
			       PANEL_VALUE_X,		    701,
			       PANEL_VALUE_Y,		    133,
			       PANEL_SLIDER_WIDTH,	    96,
			       PANEL_LABEL_STRING,	    "elevation",
			       PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			       PANEL_SHOW_RANGE,	    TRUE,
			       PANEL_SHOW_VALUE,	    TRUE,
			       PANEL_MIN_VALUE,	    -90,
			       PANEL_MAX_VALUE,	    90,
			       PANEL_VALUE,		    ROUND(180.0/PI*get_elev()),
			       PANEL_NOTIFY_LEVEL,	    PANEL_ALL,
			       PANEL_NOTIFY_PROC,	    ele_notify,
			       0);
	
	/* Create button menus and associated windows. */
	
	file_menu = xv_create(XV_NULL, MENU_COMMAND_MENU,
			      MENU_ITEM,
			      MENU_STRING,		"load data",
			      MENU_NOTIFY_PROC,		load_data_notify,
			      0,
			      MENU_ITEM,
			      MENU_STRING,		"load params",
			      MENU_NOTIFY_PROC,		load_params_notify,
			      0,
			      MENU_ITEM,
			      MENU_STRING,		"save params",
			      MENU_NOTIFY_PROC,		save_params_notify,
			      0,
			      MENU_DEFAULT,		    1,
			      MENU_GEN_PIN_WINDOW,	    panel_frame,
			      "",
			      0);
	
	/*!*//* Postpone creating windows until first needed? */
	init_file_windows(cms);

	plot_menu = xv_create(XV_NULL, MENU_COMMAND_MENU,
			      MENU_ITEM,
			      MENU_STRING,		"screen",
			      MENU_NOTIFY_PROC,		screen_plot_notify,
			      0,
			      MENU_ITEM,
			      MENU_STRING,		"print setup",
			      MENU_NOTIFY_PROC,	        hardcp_setup_notify,
			      0,
			      MENU_ITEM,
			      MENU_STRING,		"print graphic",
			      MENU_NOTIFY_PROC,	        hardcp_plot_notify,
			      0,
			      0);

	init_print_window(cms);
	
	axis_menu = xv_create(XV_NULL, MENU_COMMAND_MENU,
			      MENU_ITEM,
			      MENU_STRING,		"on/off",
			      MENU_NOTIFY_PROC,		axes_toggle_notify,
			      0,
			      MENU_ITEM,
			      MENU_STRING,		"record labeling",
			      MENU_GEN_PULLRIGHT,		rec_ax_men_gen,
			      0,
			      MENU_ITEM,
			      MENU_STRING,		"item labeling",
			      MENU_GEN_PULLRIGHT,		itm_ax_men_gen,
			      0,
			      0);
	
	waves_on = (Menu_item) xv_create(XV_NULL, MENUITEM,
					 MENU_STRING,		    "enable xwaves cursors",
					 MENU_NOTIFY_PROC,	    enable_waves_cursors,
					 MENU_RELEASE,
					 0);
	
	waves_off = (Menu_item) xv_create(XV_NULL, MENUITEM,
					  MENU_STRING,		    "disable xwaves cursors",
					  MENU_NOTIFY_PROC,	    disable_waves_cursors,
					  MENU_RELEASE,
					  0);
	
	xv_set(axis_menu,
	       MENU_APPEND_ITEM,	    waves_on,
	       MENU_APPEND_ITEM,	    waves_off,
	       0);
	
	xv_set(waves_off,
	       MENU_INACTIVE,		    !send_to_waves,
	       0);
	
	xv_set(waves_on,
	       MENU_INACTIVE,    send_to_waves ||
	       !(time_ok_for_x() || freq_ok_for_y()),
	       0);
	
	/* Create buttons */
	
	file_button = xv_create(panel, PANEL_BUTTON,
				XV_X,			    432,
				XV_Y,			    16,
				XV_WIDTH,		    61,
				XV_HEIGHT,		    25,
				PANEL_LABEL_STRING,	    " file ",
				PANEL_ITEM_MENU,	    file_menu,
				PANEL_NOTIFY_PROC,	    file_notify,
				0);
	
	
	plot_button = xv_create(panel, PANEL_BUTTON,
				XV_X,			    44,
				XV_Y,			    167,
				XV_WIDTH,		    53,
				XV_HEIGHT,		    25,
				PANEL_LABEL_STRING,	    " plot  ",
				PANEL_ITEM_MENU,	    plot_menu,
				PANEL_NOTIFY_PROC,	    plot_notify,
				0);
	
	axis_button = xv_create(panel, PANEL_BUTTON,
				XV_X,			    172,
				XV_Y,			    167,
				XV_WIDTH,		    55,
				XV_HEIGHT,		    25,
				PANEL_LABEL_STRING,	    " axes ",
				PANEL_ITEM_MENU,	    axis_menu,
				PANEL_NOTIFY_PROC,	    axis_notify,
				0);
	
	box_button = xv_create(panel, PANEL_BUTTON,
			       XV_X,			    300,
			       XV_Y,			    167,
			       XV_WIDTH,		    55,
			       XV_HEIGHT,		    25,
			       PANEL_LABEL_STRING,	    "  box  ",
			       PANEL_NOTIFY_PROC,	    box_notify,
			       0);
	
	clea_button = xv_create(panel, PANEL_BUTTON,
				XV_X,			    428,
				XV_Y,			    167,
				XV_WIDTH,		    54,
				XV_HEIGHT,		    25,
				PANEL_LABEL_STRING,	    " clear ",
				PANEL_NOTIFY_PROC,	    clea_notify,
				0);
	
	hcop_button = xv_create(panel, PANEL_BUTTON,
				XV_X,			    556,
				XV_Y,			    167,
				XV_WIDTH,		    55,
				XV_HEIGHT,		    25,
				PANEL_LABEL_STRING,	    "points",
				PANEL_NOTIFY_PROC,	    hcop_notify,
				0);
	
	help_button = xv_create(panel, PANEL_BUTTON,
				XV_X,			    684,
				XV_Y,			    167,
				XV_WIDTH,		    56,
				XV_HEIGHT,		    25,
				PANEL_LABEL_STRING,	    " help  ",
				XV_KEY_DATA,
				EXVK_HELP_NAME,	    man_page,
				XV_KEY_DATA,
				EXVK_HELP_TITLE,    "ESPS plot3d manual page",
				PANEL_NOTIFY_PROC,	    help_notify,
				0);
	
	quit_button = xv_create(panel, PANEL_BUTTON,
				XV_X,			    812,
				XV_Y,			    167,
				XV_WIDTH,		    53,
				XV_HEIGHT,		    25,
				PANEL_LABEL_STRING,	    " quit  ",
				PANEL_NOTIFY_PROC,	    quit_notify,
				0);
	/* Display */
	
	xv_set(panel_frame,
	       XV_SHOW,		    TRUE,
	       0);
    } /* end create control panel */



    /* Create canvas frame and canvases */

    canvas_frame = xv_create(XV_NULL,  FRAME,
			/* Next 3 lines fix colormap problem on Sony--
			   don't know why. */
/*
			FRAME_INHERIT_COLORS,	    FALSE,
			FRAME_FOREGROUND_COLOR,	    &ffg,
			FRAME_BACKGROUND_COLOR,	    &fbg,
*/
			     XV_SHOW,		    FALSE,
			XV_LABEL,		    iname,
			FRAME_CLOSED,		    FALSE,
			FRAME_SHOW_FOOTER,	    FALSE,
			0);

    xv_set(canvas_frame,
	   XV_X,		    1,
	   XV_Y,		    i_cpanel ? 264 : 1,
	   XV_WIDTH,		    929,
	   XV_HEIGHT,		    500,
	   0);
    (void) exv_attach_icon(canvas_frame, P3D_ICON, "plot3d", TRANSPARENT);

    do_color = color_status(canvas_frame)  && init_cms(&cms);

    init_readout_bar(cms);

    canvas = xv_create(canvas_frame, CANVAS,
			XV_X,			    0,
			XV_Y,			    ROBAR_HEIGHT,
			XV_WIDTH,		    WIN_EXTEND_TO_EDGE,
			XV_HEIGHT,		    WIN_EXTEND_TO_EDGE,
			CANVAS_X_PAINT_WINDOW,	    TRUE,
			0);

    h_scrollbar = xv_create(canvas, SCROLLBAR,
			SCROLLBAR_DIRECTION,	    SCROLLBAR_HORIZONTAL,
			0);
    v_scrollbar = xv_create(canvas, SCROLLBAR,
			SCROLLBAR_DIRECTION,	    SCROLLBAR_VERTICAL,
			0);

    xv_set(canvas,
			CANVAS_AUTO_EXPAND,	    TRUE,
			CANVAS_AUTO_SHRINK,	    FALSE,
			0);

    if (do_color)
    {
      /* It's setting the WIN_BACKGROUND color that yields the
	 poor lettering in the pop-up menu */

	xv_set(canvas_paint_window(canvas),
			WIN_CMS,		    cms,
			WIN_FOREGROUND_COLOR,	    GET_COL_PANEL_FG,
			WIN_BACKGROUND_COLOR,	    GET_COL_CANVAS_BG,
			0);

	xcolors = (unsigned long *) xv_get(canvas_paint_window(canvas),
					   WIN_X_COLOR_INDICES);
    }

    xv_set(canvas_paint_window(canvas),
			WIN_CONSUME_EVENTS,
			    WIN_MOUSE_BUTTONS,
			    LOC_DRAG,
			    LOC_MOVE,
			    0,
			WIN_EVENT_PROC,		    canv_event_proc,
			0);

    /* Create canvas menu */

    canv_menu = (Menu) xv_create(XV_NULL, MENU_COMMAND_MENU,
			MENU_ITEM,
			    MENU_STRING,		"plot",
			    MENU_NOTIFY_PROC,		plot_item_notify,
			    0,
			MENU_ITEM,
			    MENU_STRING,		"box on/off",
			    MENU_NOTIFY_PROC,		box_item_notify,
			    0,
			MENU_ITEM,
			    MENU_STRING,		"axes on/off",
			    MENU_NOTIFY_PROC,		axis_item_notify,
			    0,
			MENU_ITEM,
			    MENU_STRING,		"clear",
			    MENU_NOTIFY_PROC,		clea_item_notify,
			    0,
			MENU_ITEM,
			    MENU_STRING,		"print setup",
			    MENU_NOTIFY_PROC,	        hardcp_setup_notify,
			    0,
			MENU_ITEM,
			    MENU_STRING,		"print graphic",
			    MENU_NOTIFY_PROC,	        hardcp_plot_notify,
			    0,
			0);


    set_frame_title(iname);
    xv_set(canvas,
			CANVAS_REPAINT_PROC,	    init_graphics,
			CANVAS_RESIZE_PROC,	    resize,
			0);
    xv_set(canvas_frame,
	   XV_SHOW,		    TRUE,
	   0);


    /* Begin */
    xv_not_running=0;
    xv_main_loop(panel_frame);
}


/* NOTIFY FUNCTIONS FOR CANVAS MENU ITEMS */

static Xv_opaque
plot_item_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    (void) screen_plot_notify(XV_NULL, (Event *) NULL);
    return XV_NULL;
}


static Xv_opaque
box_item_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    (void) box_notify(XV_NULL, (Event *) NULL);
    return XV_NULL;
}

static Xv_opaque
axis_item_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    return axes_toggle_notify(menu, item);
}

static Xv_opaque
clea_item_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    (void) clea_notify(XV_NULL, (Event *) NULL);
    return XV_NULL;
}


/* NOTIFY FUNCTIONS FOR CHOICE ITEMS */


static void
ori_notify(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    if (debug_level)
        fprintf(stderr, "ori_notify: value: %u\n", value);

    do_axes();
    do_box();

    switch (value)
    {
    case 0:
	set_ori(ORI_LEFT);
	break;
    case 1:
	set_ori(ORI_RIGHT);
	break;
    }

    do_axes();
    want_box = YES;
    do_box();
}


/* NOTIFY FUNCTIONS FOR SLIDERS */


static void
sca_notify(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    static int	init = YES;
    static int	old_len, old_wid, old_hgt;

    if (debug_level)
        Fprintf(stderr, "sca_notify: value: %d\n", value);

    do_box();

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_down(event)
	|| init)
    {
	do_axes();
	old_len = get_box_len();
	old_wid = get_box_wid();
	old_hgt = get_box_hgt();
	init = NO;
    }
    if (event_id(event) == LOC_DRAG
	|| event_is_ascii(event)
	|| event_is_button(event))
    {
	set_box_len((old_len*value + 50)/100);
	set_box_wid((old_wid*value + 50)/100);
	set_box_hgt((old_hgt*value + 50)/100);
    }
    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_up(event))
    {
	xv_set(sca_slider,
			PANEL_VALUE,		    100,
			0);
	xv_set(len_slider,
			PANEL_VALUE,		    get_box_len(),
			0);
	xv_set(wid_slider,
			PANEL_VALUE,		    get_box_wid(),
			0);
	xv_set(hgt_slider,
			PANEL_VALUE,		    get_box_hgt(),
			0);
	init = YES;
	do_axes();
    }

    want_box = YES;
    do_box();
}


static void
len_notify(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "len_notify: value: %d\n", value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_down(event))
	do_axes();
    do_box();

    set_box_len(value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_up(event))
	do_axes();
    want_box = YES;
    do_box();
}


static void
dep_notify(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "dep_notify: value: %d\n", value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_down(event))
	do_axes();
    do_box();

    set_finv((double)value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_up(event))
	do_axes();
    want_box = YES;
    do_box();
}


static void
rot_notify(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "rot_notify: value: %d\n", value);
    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_down(event))
	do_axes();
    do_box();

    set_rot(PI/180.0*value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_up(event))
	do_axes();
    want_box = YES;
    do_box();
}


static void
wid_notify(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "wid_notify: value: %d\n", value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_down(event))
	do_axes();
    do_box();

    set_box_wid(value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_up(event))
	do_axes();
    want_box = YES;
    do_box();
}


static void
hsk_notify(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "hsk_notify: value: %d\n", value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_down(event))
	do_axes();
    do_box();

    set_hskew((double) value / 100.0);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_up(event))
	do_axes();
    want_box = YES;
    do_box();
}


static void
bea_notify(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "bea_notify: value: %d\n", value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_down(event))
	do_axes();
    do_box();

    set_bear(PI/180.0*value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_up(event))
	do_axes();
    want_box = YES;
    do_box();
}


static void
hgt_notify(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "hgt_notify: value: %d\n", value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_down(event))
	do_axes();
    do_box();

    set_box_hgt(value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_up(event))
	do_axes();
    want_box = YES;
    do_box();
}


static void
vsk_notify(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "vsk_notify: value: %d\n", value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_down(event))
	do_axes();
    do_box();

    set_vskew((double) value / 100.0);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_up(event))
	do_axes();
    want_box = YES;
    do_box();
}


static void
ele_notify(item, value, event)
    Panel_item	item;
    int		value;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "ele_notify: value: %d\n", value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_down(event))
	do_axes();
    do_box();

    set_elev(PI/180.0*value);

    if (event_is_ascii(event)
	|| event_is_button(event) && event_is_up(event))
	do_axes();
    want_box = YES;
    do_box();
}


/* NOTIFY FUNCTIONS FOR PANEL BUTTONS */


static int
file_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "file_notify\n");

    return XV_OK;
}


static int
screen_plot_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "plot_notify\n");

    if ( data_loaded ) {
	clear_window();
	want_box = NO;
	want_plot = YES;
	do_plot();
	do_axes();
    } else
	(void) no_data_notice();

    return XV_OK;
}

static int
hardcp_setup_notify(item, event)
    Panel_item  item;
    Event       *event;
{

	show_print_window();
        return XV_OK;

}

static int
hardcp_plot_notify(item, event)
    Panel_item  item;
    Event       *event;
{
#ifdef HELL_FREEZES
    Display *printer, *save_display;
    GC save_gc;
    XGCValues gcv;
    int old_len, old_wid, old_hgt;
    extern char *print_command;
    extern char *output_type, *output_file;
    extern double output_scale;
    extern int output_resolution;
    extern XpOrientation_t output_orientation;
    extern int to_printer;
    char command[1024];
    static char     dot_list[2] = {3, 9};

    if (debug_level)
        Fprintf(stderr, "do_hardcopy\n");

    if ( data_loaded ) {
	if(!to_printer)
		sprintf(command,"cat > %s",output_file);
	else
		strcpy(command,print_command);

	printer = XpVaOpenPrinter(NULL,
		command,
		output_type,
		XpScale, output_scale,
		XpResolution, output_resolution,
		XpOrientation, output_orientation,
		NULL);

	if(printer == NULL) {
		xprinter_error_notice("Cannot configure for graphics save.");
		return XV_OK;
	}

        save_display = display;
        save_gc = gc;
        display = printer;

	gc = XpCreateGC(display, 0, 0, &gcv);


	Xpxfs = XpLoadQueryFont(display,
	   "-adobe-times-medium-r-normal--12-120-300-300-p-150-iso8859-1");
	if(Xpxfs)
		XpSetFont(display,gc,Xpxfs->fid);
	else
		fprintf(stderr,"cannot get font for EPS\n");


        XpStartDoc(printer);
        XpStartPage(printer);
	want_box = NO;
	want_plot = YES;
	do_plot();
	do_axes();

        XpEndPage(printer);
        XpEndDoc(printer);

        XpClosePrinter(printer);
	XpFreeGC(printer, gc);
        display = save_display;
        gc = save_gc;
    } else
       (void) no_data_notice();
#endif
    return XV_OK;
}




static int
box_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "box_notify\n");

    if (data_loaded) {
	want_box = !want_box;
	draw_box();
    } else
	(void) no_data_notice();

    return XV_OK;
}


static int
axis_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "axis_notify\n");

    return XV_OK;
}

static int
plot_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "plot_notify\n");

    if ( !data_loaded )
       (void) no_data_notice();

    return XV_OK;
}


/*!*//* Move this menu stuff out of the middle of the button functions. */

static Xv_opaque
axes_toggle_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    if (debug_level)
	fprintf(stderr, "axes_toggle_notify\n");

    if ( data_loaded ) {
	want_axes = !want_axes;
	draw_axes();
    }

    return XV_NULL;
}



static Menu
rec_ax_men_gen(item, op)
    Menu_item	    item;
    Menu_generate   op;
{
    extern int		time_ok_for_x();
    static Menu		menu = MENU_NULL;
    static Menu_item	rnumber_item, rtime_item, tag_item, ttime_item, other_item;
    int			time_ok, rtime_ok, ttime_ok;

    switch (op) {
	case MENU_DISPLAY: {
	    if (menu == MENU_NULL) {
		menu = (Menu) xv_create(XV_NULL, MENU_COMMAND_MENU, 0);
		rnumber_item = (Menu_item) xv_create(XV_NULL, MENUITEM,
						     MENU_STRING,		"record number",
						     MENU_NOTIFY_PROC,		x_rnum_notify,
						     MENU_RELEASE,
						     0);
		rtime_item = (Menu_item) xv_create(XV_NULL, MENUITEM,
						   MENU_STRING,		    "time (from rec #)",
						   MENU_NOTIFY_PROC,	    x_rtime_notify,
						   MENU_RELEASE,
						   0);
		tag_item = (Menu_item) xv_create(XV_NULL, MENUITEM,
						 MENU_STRING,		    "tag",
						 MENU_NOTIFY_PROC,	    x_tag_notify,
						 MENU_RELEASE,
						 0);
		ttime_item = (Menu_item) xv_create(XV_NULL, MENUITEM,
						   MENU_STRING,		    "time (from tag)",
						   MENU_NOTIFY_PROC,	    x_ttime_notify,
						   MENU_RELEASE,
						   0);
		/*
		  other_item = (Menu_item) xv_create(XV_NULL, MENUITEM,
		  MENU_STRING,		    "other",
		  MENU_NOTIFY_PROC,	    x_other_notify,
		  MENU_RELEASE,
		  */
		/*!*//* TEST */
		/*
		  MENU_INACTIVE,		    YES,
		  0);
		  */
		xv_set(menu,
		       MENU_APPEND_ITEM,	    rnumber_item,
		       MENU_APPEND_ITEM,	    rtime_item,
		       MENU_APPEND_ITEM,	    tag_item,
		       MENU_APPEND_ITEM,	    ttime_item,
		       /* MENU_APPEND_ITEM,	    other_item, */
		       0);
	    }
	    if ( data_loaded ) {
		time_ok = time_ok_for_x();
		rtime_ok = time_ok == XTIME_FROM_REC || time_ok == XTIME_FROM_BOTH;
		ttime_ok = time_ok == XTIME_FROM_TAG || time_ok == XTIME_FROM_BOTH;
		xv_set(rtime_item, MENU_INACTIVE, !rtime_ok, 0);
		xv_set(tag_item, MENU_INACTIVE, !tag_ok_for_x(), 0);
		xv_set(ttime_item, 	MENU_INACTIVE, !ttime_ok, 0);
	    } else {
		xv_set(rnumber_item, MENU_INACTIVE, TRUE, NULL);
		xv_set(rtime_item, MENU_INACTIVE, TRUE, NULL);
		xv_set(tag_item, MENU_INACTIVE, TRUE, NULL);
		xv_set(ttime_item, MENU_INACTIVE, TRUE, NULL);
	    }
	    break;
	}
	default: {
	    break;
	}
    }

    return menu;
}


static Xv_opaque
x_rnum_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    extern void	make_x_rnum();

    if(send_to_waves)
	disable_waves_cursors();
    make_x_rnum();
    return XV_NULL;
}


static Xv_opaque
x_rtime_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    extern void	make_x_rtime();

    make_x_rtime();
    return XV_NULL;
}


static Xv_opaque
x_tag_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    extern void	make_x_tag();

    if(send_to_waves)
	disable_waves_cursors();
    make_x_tag();
    return XV_NULL;
}


static Xv_opaque
x_ttime_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    extern void	make_x_ttime();

    make_x_ttime();
    return XV_NULL;
}


static Xv_opaque
x_other_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    extern void	make_x_other();

    if(send_to_waves)
	disable_waves_cursors();
    make_x_other();
    return XV_NULL;
}


static Menu
itm_ax_men_gen(item, op)
    Menu_item	    item;
    Menu_generate   op;
{
    extern int		freq_ok_for_y();
    static Menu		menu = MENU_NULL;
    static Menu_item	freq_item, other_item, itemno_item;

    switch (op)
    {
    case MENU_DISPLAY:
	if (menu == MENU_NULL)
	{
	    menu = (Menu) xv_create(XV_NULL, MENU_COMMAND_MENU, 0);
	    itemno_item = (Menu_item) xv_create(XV_NULL, MENUITEM,
			    MENU_STRING,		"item number",
			    MENU_NOTIFY_PROC,		y_inum_notify,
			    MENU_RELEASE,
			0);
	    freq_item = (Menu_item) xv_create(XV_NULL, MENUITEM,
			MENU_STRING,		    "frequency",
			MENU_NOTIFY_PROC,	    y_freq_notify,
			MENU_RELEASE,
			0);
/*
	    other_item = (Menu_item) xv_create(XV_NULL, MENUITEM,
			MENU_STRING,		    "other",
			MENU_NOTIFY_PROC,	    y_other_notify,
			MENU_RELEASE,
*/
/*!*//* TEST */
/*
			MENU_INACTIVE,		    YES,
			0);
*/
	    xv_set(menu,
			MENU_APPEND_ITEM,	    itemno_item,
			MENU_APPEND_ITEM,	    freq_item,
			/* MENU_APPEND_ITEM,	    other_item, */
			0);
	}
	if (data_loaded)
	    xv_set(freq_item, MENU_INACTIVE, !freq_ok_for_y(),0);
	else {
	    xv_set(itemno_item, MENU_INACTIVE, TRUE, 0);
	    xv_set(freq_item, MENU_INACTIVE, TRUE, 0);
	}
	break;
    default:
	break;
    }

    return menu;
}


static Xv_opaque
y_inum_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    extern void	make_y_inum();

    if(send_to_waves)
	disable_waves_cursors();
    make_y_inum();
    return XV_NULL;
}


static Xv_opaque
y_freq_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    extern void	make_y_freq();

    make_y_freq();
    return XV_NULL;
}


static Xv_opaque
y_other_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    extern void	make_y_other();

    if(send_to_waves)
	disable_waves_cursors();
    make_y_other();
    return XV_NULL;
}






static int
clea_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "clea_notify\n");

    want_axes = NO;
    want_box = NO;
    want_plot = NO;

    clear_window();

    return XV_OK;
}


static int
hcop_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "hcop_notify\n");

    draw_pointplot();

    return XV_OK;
}


static int
help_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    extern int	exv_get_help();

    if (debug_level)
	Fprintf(stderr, "help_notify\n");

    (void) exv_get_help(item, event);

    return XV_OK;
}


static int
quit_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    if (debug_level)
	Fprintf(stderr, "quit_notify\n");

    xv_destroy_safe(canvas_frame);
    if ( want_cpanel )
	xv_destroy_safe(panel_frame);

    return XV_OK;
}


static void
init_graphics(canv, pw, disp, xw, xrects)
    Canvas	    canv;
    Xv_Window	    pw;
    Display	    *disp;
    Window	    xw;
    Xv_xrectlist    *xrects;
{
    XGCValues	    gc_val;
    int		    w, h;
    static char     dot_list[2] = {3, 9};
    Server_image    svri;

    if (debug_level)
	Fprintf(stderr, "init_graphics\n");

    paintwin = pw;
    display = disp;
    xwin = xw;
    gc_val.function = GXxor;
    font = (Xv_Font) xv_find(XV_NULL, FONT,
			FONT_FAMILY,		    FONT_FAMILY_COUR,
			0);
    gc_val.font = (Font) xv_get(font, XV_XID);
    gc = XCreateGC(display, xwin, GCFunction | GCFont, &gc_val);
    XCopyGC(display, DefaultGC(display, DefaultScreen(display)),
	    GCForeground|GCBackground, gc);

    XSetDashes(display, gc, 0, dot_list, 2);

    default_cursor = (Xv_Cursor) xv_get( pw, WIN_CURSOR);
    svri = (Server_image) xv_create( XV_NULL, SERVER_IMAGE,
				    XV_WIDTH, 16, XV_HEIGHT, 16,
				    SERVER_IMAGE_BITS, cursor_bits, NULL);
    blank_cursor = (Xv_Cursor) xv_create( XV_NULL, CURSOR, CURSOR_IMAGE, svri, NULL);

    w = (int) xv_get(pw, XV_WIDTH);
    h = (int) xv_get(pw, XV_HEIGHT);
    set_canv_dimens(w, h);

    if (debug_level)
	Fprintf(stderr, "canvas width: %d.  canvas height: %d\n", w, h);

    xv_set(canvas,
			CANVAS_REPAINT_PROC,	    repaint_proc,
			0);

/*  On some systems  XView bug causes extra repaints on startup. */
#if defined(SONY) || defined(DS3100) || defined(hpux)
    repaint_proc(canv, pw, disp, xw, xrects);
#endif

    repaint_proc(canv, pw, disp, xw, xrects);
}


void
string_extents(str, asc, desc, wid, def_wid)
    char	*str;
    int		*asc, *desc, *wid, *def_wid;
{
    XCharStruct	overall;
    int		dir;

    /*    if(!XpIsPrinter(display)) {
    	(void) XTextExtents((XFontStruct *) xv_get(font, FONT_INFO),
			str, strlen(str),
			&dir, asc, desc, &overall);
    	*wid = overall.width;
    	*def_wid = (int) xv_get(font, FONT_DEFAULT_CHAR_WIDTH);
	} else*/ {
    	(void) XTextExtents(Xpxfs,
			str, strlen(str),
			&dir, asc, desc, &overall);
    	*wid = overall.width;
	*def_wid = *wid; /*XpTextWidth(Xpxfs, "m", 1);*/
    }
	
}


void
drawing_style(fn, col, lns)
    int		    fn, col, lns;
{
    int		    xfn;
    unsigned long   xcol;
    int		    xlns;

    switch (fn)
    {
    case BFN_SRC:
	xfn = GXcopy;
	break;
    case BFN_XOR:
	xfn = GXxor;
	break;
    default:
	return;
    }

    XSetFunction(display, gc, xfn);

    if (do_color)
    {
	switch (fn)
	{
	case BFN_SRC:
	    xcol = xcolors[col];
	    break;
	case BFN_XOR:
	    xcol = xcolors[col]^xcolors[GET_COL_CANVAS_BG];
	    break;
	default:
	    return;
	}

	XSetForeground(display, gc, xcol);
    }

    switch (lns)
    {
    case LNS_SOLID:
	xlns = LineSolid;
	break;
    case LNS_DOT:
	xlns = LineOnOffDash;
	break;
    }
    XSetLineAttributes(display, gc, (unsigned) 0, xlns, CapButt, JoinMiter);
}


static void
repaint_proc(canv, pw, disp, xw, xrects)
    Canvas	    canv;
    Xv_Window	    pw;
    Display	    *disp;
    Window	    xw;
    Xv_xrectlist    *xrects;
{
    if (debug_level)
	Fprintf(stderr, "repaint_proc\n");

    repaint();
}


void
repaint()
{
    static first_time = 1;

    static struct timeval ptp;
    struct timeval tp;
    struct timezone tz;
    int dif,difs;

    gettimeofday(&tp,&tz);

    if ( ( ((tp.tv_sec - ptp.tv_sec)*1000000) + (tp.tv_usec - ptp.tv_usec)) < 2000000 && !first_time )
	return;

    clear_window();

    if ( data_loaded ) {

	if ( first_time )
	    first_time = 0;
	
	do_plot();
	do_axes();
	do_box();
    }

    gettimeofday(&ptp,&tz);
}


static void
resize(canvas, width, height)
    Canvas	canvas;
    int		width, height;
{
    int		w, h;

    w = (int) xv_get(canvas_paint_window(canvas), XV_WIDTH);
    h = (int) xv_get(canvas_paint_window(canvas), XV_HEIGHT);

    if (debug_level)
    {
	Fprintf(stderr, "resize\n");
	Fprintf(stderr, "width %d.  height %d.\n", width, height);
	Fprintf(stderr, "canvas XV_WIDTH %d.  XV_HEIGHT %d.\n",
		(int) xv_get(canvas, XV_WIDTH),
		(int) xv_get(canvas, XV_HEIGHT));
	Fprintf(stderr, "paint window XV_WIDTH %d.  XV_HEIGHT %d.\n",
		w, h);
    }

    set_canv_dimens(w, h);
}


static void
canv_event_proc(pw, event, arg)
    Xv_Window	pw;
    Event	*event;
    caddr_t	arg;
{
    static int	old_x, old_y;
    static use_blank_cursor=0;
    int		x, y;
    double	xx, yy;

    switch(event_id(event))
    {
    case MS_LEFT:
	if (event_is_down(event))
	{
	    old_x = event_x(event);
	    old_y = event_y(event);

	    clear_readouts();
	    do_axes();
	}
	else	/* button release */
	{
	    xv_set(hsk_slider,
			PANEL_VALUE,		    ROUND(100.0*get_hskew()),
			0);
	    xv_set(vsk_slider,
			PANEL_VALUE,		    ROUND(100.0*get_vskew()),
			0);
	    do_axes();
	}
	break;
    case MS_MIDDLE:
	if (event_is_down(event))
	{
	    old_x = event_x(event);
	    old_y = event_y(event);

	    clear_readouts();
	    do_axes();
	}
	else	/* button release */
	{
	    if ( want_cpanel ) {
		xv_set(bea_slider,
			PANEL_VALUE,		    ROUND(180.0/PI*get_bear()),
			0);
		xv_set(ele_slider,
			PANEL_VALUE,		    ROUND(180.0/PI*get_elev()),
			0);
		xv_set(rot_slider,
			PANEL_VALUE,		    ROUND(180.0/PI*get_rot()),
			0);
	    }
	    do_axes();
	}
	break;
    case MS_RIGHT:
	if (event_is_down(event))
	{
	    clear_readouts();
	    menu_show(canv_menu, pw, event, 0);
	}
	else	/* button release */
	{
	}
	break;
    case LOC_DRAG:
	x = event_x(event);
	y = event_y(event);
	if (event_left_is_down(event))
	{
	    do_box();

	    set_hskew(get_hskew() - (x - old_x)/100.0);
	    set_vskew(get_vskew() + (y - old_y)/100.0);

	    want_box = YES;
	    do_box();
	}
	if (event_middle_is_down(event))
	{
	    double  theta, phi, psi;

	    do_box();

/*!*//* Code for controlling all 3 rotational degrees of freedom with
	mouse movements in the plane.  Needs work to make the interface
	intuitive. */
/*
	    update_rmat(-(x - old_x)/100.0, (y - old_y)/100.0,
			&theta, &phi, &psi);
	    set_bear(theta);
	    set_elev(phi);
	    set_rot(psi);
*/

/*!*//* Code for controlling just bearing and elevation with the mouse. */

	    theta = get_bear() - (x - old_x)/100.0;
	    if (theta > PI) theta -= 2.0*PI;
	    if (theta < -PI) theta += 2.0*PI;
	    phi = get_elev() + (y - old_y)/100.0;
	    if (phi > 0.5*PI) phi = 0.5*PI;
	    if (phi < -0.5*PI) phi = -0.5*PI;
	    set_bear(theta);
	    set_elev(phi);

/*!*//* End of 2-deg-of-freedom code */

	    want_box = YES;
	    do_box();
	}
	old_x = x;
	old_y = y;
	break;
    case LOC_MOVE:
	x = event_x(event);
	y = event_y(event);

	if (data_loaded && base_trans((double) x, (double) y, &xx, &yy))
	{
	    double	xval, yval, zval;

	    if ( !use_blank_cursor ) {
		xv_set( pw, WIN_CURSOR, blank_cursor, NULL);
		use_blank_cursor = 1;
	    }
	    xyz_vals(xx, yy, &xval, &yval, &zval);
	    post_readouts(xval, yval, zval);
	}
	else {
	    if ( use_blank_cursor ) {
		xv_set( pw, WIN_CURSOR, default_cursor, NULL);
		use_blank_cursor = 0;
	    }
	    clear_readouts();
	}
	break;
    }
}




void
clear_window()
{
    XClearWindow(display, xwin);
}


void
do_axes()
{
    if (want_axes) draw_axes();
}


void
do_box()
{
    if (want_box)
	draw_box();

}


static void
do_plot()
{

    if (want_plot) {
	xv_set( canvas_frame, FRAME_BUSY, TRUE, NULL);
	if ( want_cpanel )
	    xv_set( panel_frame, FRAME_BUSY, TRUE, NULL);
	draw_plot();
	xv_set( canvas_frame, FRAME_BUSY, FALSE, NULL);
	if ( want_cpanel )
	    xv_set( panel_frame, FRAME_BUSY, FALSE, NULL);
    }
}


void
drawpoint(u, v)
    double  u, v;
{
    XDrawPoint(display, xwin, gc, ROUND(u), ROUND(v));
}


void
drawline(u0, v0, u1, v1)
    double  u0, u1, v0, v1;
{
    XDrawLine(display, xwin, gc, ROUND(u0), ROUND(v0), ROUND(u1), ROUND(v1));
}

void
drawblob(u0, v0, diag)
    double  u0, v0;
    int diag;
{
/*
    XDrawArc(display, xwin, gc, ROUND(u0)-diag, ROUND(v0)-diag, ROUND(u0)+diag, ROUND(v0)+diag);
*/
}



void
drawlines(n, u, v)
    int		    n;
    double	    *u, *v;
{
    static int	    alloc_size = -1;
    static XPoint   *p;
    int		    i;

    if (alloc_size < n)
    {
	if (alloc_size == -1)
	    p = (XPoint *) malloc((unsigned) n * sizeof(XPoint));
	else
	    p = (XPoint *) realloc((char *) p, (unsigned) n * sizeof(XPoint));

	alloc_size = n;
    }

    for (i = 0; i < n; i++)
    {
	p[i].x = ROUND(u[i]);
	p[i].y = ROUND(v[i]);
    }

    XDrawLines(display, xwin, gc, p, n, CoordModeOrigin);
}


void
draw_string(u, v, text)
    double  u, v;
    char    *text;
{
    XDrawString(display, xwin, gc, ROUND(u), ROUND(v), text, strlen(text));
}

static int
color_status(frame)
    Frame frame;
{
    Display *display = (Display *) xv_get(frame, XV_DISPLAY);

    return DisplayPlanes(display, DefaultScreen(display)) > 1;
}


int
init_cms(cms)
    Cms	    *cms;
{
    Xv_singlecolor  colors[NUM_P3D_COL];

#define SET_COLOR(i, r, g, b) \
	colors[(i)-CMAP_OFFSET].red = (r); \
	colors[(i)-CMAP_OFFSET].green = (g); \
	colors[(i)-CMAP_OFFSET].blue = (b);

    SET_COLOR(COL_PANEL_BG,  255, 238, 204)
    SET_COLOR(COL_PANEL_FG,    0,   0,   0)
    SET_COLOR(COL_CANVAS_BG, 255, 255, 204)
    SET_COLOR(COL_TOP_FG,      0,   0, 204)
    SET_COLOR(COL_BOT_FG,    136, 136,   0)
    SET_COLOR(COL_BOX_FG,    255,   0,   0)
    SET_COLOR(COL_AXIS_FG,     0, 102,   0)
    SET_COLOR(COL_ERRMSG_FG, 255,   0,   0)
    SET_COLOR(COL_ROBAR_FG,    0,   0,   0)
    SET_COLOR(COL_ROBAR_BG,  255, 255, 255)
    SET_COLOR(COL_BLACK,       0,   0,   0)
    SET_COLOR(COL_WHITE,     255, 255, 255)

#undef SET_COLOR

    *cms = xv_create(XV_NULL, CMS,
			CMS_CONTROL_CMS,    TRUE,
			CMS_SIZE,	    NUM_P3D_COL + CMS_CONTROL_COLORS,
			CMS_COLORS,	    colors,
			0);

    return cms != XV_NULL;
}

void
no_data_notice()
{
    notice_prompt( canvas_frame, NULL,
		  NOTICE_MESSAGE_STRINGS, "No data loaded.", "Use file button to load data.", NULL,
		  NOTICE_BUTTON_YES, "Continue...",
		  NULL);
}

void
xprinter_error_notice(s)
char *s;
{
	notice_prompt(canvas_frame, NULL,
		NOTICE_MESSAGE_STRINGS, "Error in Print Setup", s, NULL,
		NOTICE_BUTTON_YES, "Continue...",
		NULL);
}
