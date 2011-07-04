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
|  Module: p3dfiles.c							|
|									|
|  Handle command windows for load and saving params and loading data.	|
|									|
|  Rodney W. Johnson, Entropic Speech, Inc.				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)p3dfiles.c	1.7	7/24/91	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/cms.h>
#include <esps/constants.h>
#include "plot3d.h"


short		get_fea_type();
long		get_fea_siz();

extern int  	debug_level;
extern int	do_color;

extern int      data_loaded;
extern char     *iname;
extern char     *start_rec_string;
extern char     *end_rec_string;
extern char     *start_item_string;
extern char     *end_item_string;
extern char     *farg_string;
extern char     *dir_string;

Xv_opaque	load_data_notify(),
		load_params_notify(),
		save_params_notify();

static int	ld_dat_notify();
static char	*ld_dat();
static int	ld_par_notify();
static char	*ld_par();
static int	sav_par_notify();
static char	*sav_par();

static char	*make_path();

extern void	do_box(), do_axes();
extern void	getsym();

extern char	*read_data();

extern void	set_frame_title();

extern void	set_box_len(),	set_box_wid(),	set_box_hgt();
extern int	get_box_len(),	get_box_wid(),	get_box_hgt();
extern void	set_hskew(),	set_vskew(),	set_finv();
extern double	get_hskew(),	get_vskew(),	get_finv();
extern void	set_ori();
extern int	get_ori();
extern void	set_rot(),	set_bear(),	set_elev();
extern double	get_rot(),	get_bear(),	get_elev();

static Frame	    ld_dat_frame;
static Panel	    ld_dat_panel;
static Panel_item   ld_dat_dir_text;
static Panel_item   ld_dat_file_text;
static Panel_item   ld_dat_button;
static Panel_item   ld_dat_startrec_text, ld_dat_endrec_text;
static Panel_item   ld_dat_fld_text;
static Panel_item   ld_dat_startitem_text, ld_dat_enditem_text;
static Panel_item   ld_dat_rb_msg;
static Panel_item   ld_dat_msg;

static Frame	    ld_par_frame;
static Panel	    ld_par_panel;
static Panel_item   ld_par_dir_text;
static Panel_item   ld_par_file_text;
static Panel_item   ld_par_msg;
static Panel_item   ld_par_button;

static Frame	    sav_par_frame;
static Panel	    sav_par_panel;
static Panel_item   sav_par_dir_text;
static Panel_item   sav_par_file_text;
static Panel_item   sav_par_msg;
static Panel_item   sav_par_button;


void
init_file_windows(cms)
    Cms			cms;
{
    extern Frame	panel_frame;


    /* Create load data panel */

    ld_dat_frame = xv_create(panel_frame, FRAME_CMD,
			XV_WIDTH,		    452,
			XV_HEIGHT,		    164,
			XV_LABEL,		    "3-D Plot:  Load Data",
			XV_SHOW,		    FALSE,
			FRAME_SHOW_FOOTER,	    FALSE,
			FRAME_CMD_PUSHPIN_IN,	    FALSE,
			0);

    ld_dat_panel = xv_get(ld_dat_frame, FRAME_CMD_PANEL);
    xv_set(ld_dat_panel,
			XV_X,			    0,
			XV_Y,			    0,
			XV_WIDTH,		    WIN_EXTEND_TO_EDGE,
			XV_HEIGHT,		    WIN_EXTEND_TO_EDGE,
			0);
    if (do_color)
	xv_set(ld_dat_panel,
			WIN_CMS,		    cms,
			WIN_FOREGROUND_COLOR,	    COL_PANEL_FG,
			WIN_BACKGROUND_COLOR,	    COL_PANEL_BG,
			0);

    ld_dat_dir_text = xv_create(ld_dat_panel, PANEL_TEXT,
			XV_X,			    12,
			XV_Y,			    21,
			XV_WIDTH,		    341,
			XV_HEIGHT,		    15,
			PANEL_LABEL_STRING,	    "directory",
			PANEL_VALUE_X,		    81,
			PANEL_VALUE_Y,		    21,
			PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			PANEL_VALUE_DISPLAY_LENGTH, 34,
			PANEL_VALUE_STORED_LENGTH,  120,
			PANEL_READ_ONLY,	    FALSE,
			0);
    if ( data_loaded )
	xv_set( ld_dat_dir_text, PANEL_VALUE, dir_string, NULL);

    ld_dat_file_text = xv_create(ld_dat_panel, PANEL_TEXT,
			XV_X,			    49,
			XV_Y,			    53,
			XV_WIDTH,		    304,
			XV_HEIGHT,		    15,
			PANEL_LABEL_STRING,	    "file",
			PANEL_VALUE_X,		    81,
			PANEL_VALUE_Y,		    53,
			PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			PANEL_VALUE_DISPLAY_LENGTH, 34,
			PANEL_VALUE_STORED_LENGTH,  120,
			PANEL_READ_ONLY,	    FALSE,
			0);
    if ( data_loaded )
	xv_set( ld_dat_file_text, PANEL_VALUE, iname, NULL);

    ld_dat_button = xv_create(ld_dat_panel, PANEL_BUTTON,
			XV_X,			    372,
			XV_Y,			    67,
			XV_WIDTH,		    61,
			XV_HEIGHT,		    25,
			PANEL_LABEL_STRING,	    "  load  ",
			PANEL_NOTIFY_PROC,	    ld_dat_notify,
			0);

    ld_dat_startrec_text = xv_create(ld_dat_panel, PANEL_TEXT,
			XV_X,			    60,
			XV_Y,			    85,
			XV_WIDTH,		    62,
			XV_HEIGHT,		    15,
			PANEL_LABEL_STRING,	    "-r",
			PANEL_VALUE_X,		    82,
			PANEL_VALUE_Y,		    85,
			PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			PANEL_VALUE_DISPLAY_LENGTH, 5,
			PANEL_VALUE_STORED_LENGTH,  12,
			PANEL_READ_ONLY,	    FALSE,
			0);
    if ( data_loaded )
	xv_set( ld_dat_startrec_text, PANEL_VALUE, start_rec_string, NULL);

    ld_dat_endrec_text = xv_create(ld_dat_panel, PANEL_TEXT,
			XV_X,			    129,
			XV_Y,			    85,
			XV_WIDTH,		    51,
			XV_HEIGHT,		    15,
			PANEL_LABEL_STRING,	    ":",
			PANEL_VALUE_X,		    140,
			PANEL_VALUE_Y,		    85,
			PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			PANEL_VALUE_DISPLAY_LENGTH, 5,
			PANEL_VALUE_STORED_LENGTH,  12,
			PANEL_READ_ONLY,	    FALSE,
			0);
    if ( data_loaded )
	xv_set( ld_dat_endrec_text, PANEL_VALUE, end_rec_string, NULL);

    ld_dat_fld_text = xv_create(ld_dat_panel, PANEL_TEXT,
			XV_X,			    41,
			XV_Y,			    117,
			XV_WIDTH,		    152,
			XV_HEIGHT,		    15,
			PANEL_LABEL_STRING,	    "field",
			PANEL_VALUE_X,		    81,
			PANEL_VALUE_Y,		    117,
			PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			PANEL_VALUE_DISPLAY_LENGTH, 14,
			PANEL_VALUE_STORED_LENGTH,  80,
			PANEL_VALUE,		    DEF_FIELD,
			PANEL_READ_ONLY,	    FALSE,
			0);
    if ( data_loaded )
	xv_set( ld_dat_fld_text, PANEL_VALUE, farg_string, NULL);

    ld_dat_startitem_text = xv_create(ld_dat_panel, PANEL_TEXT,
			XV_X,			    203,
			XV_Y,			    117,
			XV_WIDTH,		    53,
			XV_HEIGHT,		    15,
			PANEL_LABEL_STRING,	    "[",
			PANEL_VALUE_X,		    216,
			PANEL_VALUE_Y,		    117,
			PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			PANEL_VALUE_DISPLAY_LENGTH, 5,
			PANEL_VALUE_STORED_LENGTH,  12,
			PANEL_READ_ONLY,	    FALSE,
			0);
    if ( data_loaded )
	xv_set( ld_dat_startitem_text, PANEL_VALUE, start_item_string, NULL);

    ld_dat_enditem_text = xv_create(ld_dat_panel, PANEL_TEXT,
			XV_X,			    265,
			XV_Y,			    117,
			XV_WIDTH,		    51,
			XV_HEIGHT,		    15,
			PANEL_LABEL_STRING,	    ":",
			PANEL_VALUE_X,		    276,
			PANEL_VALUE_Y,		    117,
			PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			PANEL_VALUE_DISPLAY_LENGTH, 5,
			PANEL_VALUE_STORED_LENGTH,  12,
			PANEL_READ_ONLY,	    FALSE,
			0);
    if ( data_loaded )
	xv_set( ld_dat_enditem_text, PANEL_VALUE, end_item_string, NULL);

    ld_dat_rb_msg = xv_create(ld_dat_panel, PANEL_MESSAGE,
			XV_X,			    324,
			XV_Y,			    117,
			XV_WIDTH,		    5,
			XV_HEIGHT,		    13,
			PANEL_LABEL_STRING,	    "]",
			PANEL_LABEL_BOLD,	    TRUE,
			0);

    ld_dat_msg = xv_create(ld_dat_panel, PANEL_MESSAGE,
			XV_X,			    12,
			XV_Y,			    141,
			/* XV_HEIGHT,		    13, */
			PANEL_LABEL_STRING,	    "",
			PANEL_LABEL_BOLD,	    TRUE,
			0);

    if (do_color)
	xv_set(ld_dat_msg,
			PANEL_ITEM_COLOR,	    COL_ERRMSG_FG,
			0);

    /* Create load params panel */

    ld_par_frame = xv_create(panel_frame, FRAME_CMD,
			XV_WIDTH,		    452,
			XV_HEIGHT,		    120,
			XV_LABEL,		    "3-D Plot:  Load Params",
			XV_SHOW,		    FALSE,
			FRAME_SHOW_FOOTER,	    FALSE,
			FRAME_CMD_PUSHPIN_IN,	    FALSE,
			0);

    ld_par_panel = xv_get(ld_par_frame, FRAME_CMD_PANEL);
    xv_set(ld_par_panel,
			XV_X,			    0,
			XV_Y,			    0,
			XV_WIDTH,		    WIN_EXTEND_TO_EDGE,
			XV_HEIGHT,		    WIN_EXTEND_TO_EDGE,
			0);
    if (do_color)
	xv_set(ld_par_panel,
			WIN_CMS,		    cms,
			WIN_FOREGROUND_COLOR,	    COL_PANEL_FG,
			WIN_BACKGROUND_COLOR,	    COL_PANEL_BG,
			0);

    ld_par_dir_text = xv_create(ld_par_panel, PANEL_TEXT,
			XV_X,			    12,
			XV_Y,			    26,
			XV_WIDTH,		    341,
			XV_HEIGHT,		    15,
			PANEL_LABEL_STRING,	    "directory",
			PANEL_VALUE_X,		    81,
			PANEL_VALUE_Y,		    26,
			PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			PANEL_VALUE_DISPLAY_LENGTH, 34,
			PANEL_VALUE_STORED_LENGTH,  120,
			PANEL_READ_ONLY,	    FALSE,
			0);

    ld_par_file_text = xv_create(ld_par_panel, PANEL_TEXT,
			XV_X,			    49,
			XV_Y,			    68,
			XV_WIDTH,		    304,
			XV_HEIGHT,		    15,
			PANEL_LABEL_STRING,	    "file",
			PANEL_VALUE_X,		    81,
			PANEL_VALUE_Y,		    68,
			PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			PANEL_VALUE_DISPLAY_LENGTH, 34,
			PANEL_VALUE_STORED_LENGTH,  120,
			PANEL_READ_ONLY,	    FALSE,
			0);

    ld_par_msg = xv_create(ld_par_panel, PANEL_MESSAGE,
			XV_X,			    12,
			XV_Y,			    95,
			/* XV_HEIGHT,		    13, */
			PANEL_LABEL_STRING,	    "",
			PANEL_LABEL_BOLD,	    TRUE,
			0);

    if (do_color)
	xv_set(ld_par_msg,
			PANEL_ITEM_COLOR,	    COL_ERRMSG_FG,
			0);

    ld_par_button = xv_create(ld_par_panel, PANEL_BUTTON,
			XV_X,			    372,
			XV_Y,			    47,
			XV_WIDTH,		    61,
			XV_HEIGHT,		    25,
			PANEL_LABEL_STRING,	    "  load  ",
			PANEL_NOTIFY_PROC,	    ld_par_notify,
			0);

    /* Create save params panel */

    sav_par_frame = xv_create(panel_frame, FRAME_CMD,
			XV_WIDTH,		    452,
			XV_HEIGHT,		    120,
			XV_LABEL,		    "3-D Plot:  Save Params",
			XV_SHOW,		    FALSE,
			FRAME_SHOW_FOOTER,	    FALSE,
			FRAME_CMD_PUSHPIN_IN,	    FALSE,
			0);

    sav_par_panel = xv_get(sav_par_frame, FRAME_CMD_PANEL);
    xv_set(sav_par_panel,
			XV_X,			    0,
			XV_Y,			    0,
			XV_WIDTH,		    WIN_EXTEND_TO_EDGE,
			XV_HEIGHT,		    WIN_EXTEND_TO_EDGE,
			0);
    if (do_color)
	xv_set(sav_par_panel,
			WIN_CMS,		    cms,
			WIN_FOREGROUND_COLOR,	    COL_PANEL_FG,
			WIN_BACKGROUND_COLOR,	    COL_PANEL_BG,
			0);

    sav_par_dir_text = xv_create(sav_par_panel, PANEL_TEXT,
			XV_X,			    12,
			XV_Y,			    26,
			XV_WIDTH,		    341,
			XV_HEIGHT,		    15,
			PANEL_LABEL_STRING,	    "directory",
			PANEL_VALUE_X,		    81,
			PANEL_VALUE_Y,		    26,
			PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			PANEL_VALUE_DISPLAY_LENGTH, 34,
			PANEL_VALUE_STORED_LENGTH,  120,
			PANEL_READ_ONLY,	    FALSE,
			0);

    sav_par_file_text = xv_create(sav_par_panel, PANEL_TEXT,
			XV_X,			    49,
			XV_Y,			    68,
			XV_WIDTH,		    304,
			XV_HEIGHT,		    15,
			PANEL_LABEL_STRING,	    "file",
			PANEL_VALUE_X,		    81,
			PANEL_VALUE_Y,		    68,
			PANEL_LAYOUT,		    PANEL_HORIZONTAL,
			PANEL_VALUE_DISPLAY_LENGTH, 34,
			PANEL_VALUE_STORED_LENGTH,  120,
			PANEL_READ_ONLY,	    FALSE,
			0);

    sav_par_msg = xv_create(sav_par_panel, PANEL_MESSAGE,
			XV_X,			    12,
			XV_Y,			    95,
			/* XV_HEIGHT,		    13, */
			PANEL_LABEL_STRING,	    "",
			PANEL_LABEL_BOLD,	    TRUE,
			0);

    if (do_color)
	xv_set(sav_par_msg,
			PANEL_ITEM_COLOR,	    COL_ERRMSG_FG,
			0);

    sav_par_button = xv_create(sav_par_panel, PANEL_BUTTON,
			XV_X,			    372,
			XV_Y,			    47,
			XV_WIDTH,		    61,
			XV_HEIGHT,		    25,
			PANEL_LABEL_STRING,	    "  save  ",
			PANEL_NOTIFY_PROC,	    sav_par_notify,
			0);
}


/* NOTIFY FUNCTIONS FOR MENU ITEMS */


Xv_opaque
load_data_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    if (debug_level)
        fprintf(stderr, "load_data_notify\n");

    xv_set(ld_dat_frame,
			XV_SHOW,		    TRUE,
			0);
    return XV_NULL;
}


Xv_opaque
load_params_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    if (debug_level)
        fprintf(stderr, "load_params_notify\n");

    xv_set(ld_par_frame,
			XV_SHOW,		    TRUE,
			0);
    return XV_NULL;
}


Xv_opaque
save_params_notify(menu, item)
    Menu	menu;
    Menu_item	item;
{
    if (debug_level)
        fprintf(stderr, "save_params_notify\n");

    xv_set(sav_par_frame,
			XV_SHOW,		    TRUE,
			0);
    return XV_NULL;
}


static int
num_token(txt, buf, val)
    char    *txt, *buf;
    long    *val;
{
    switch (sscanf(txt, "%s%s", buf, buf))
    {
    case EOF:
	buf[0] = '\0';
	break;
    case 1:
	if (sscanf(buf, "%ld", val) != 1)
	    return YES;
	break;
    default:
	return YES;
    }

    return NO;	/* No error */
}


static char *
lpanel_range(stx, etx, sval, eval, inc)
    char    *stx, *etx;
    long    *sval, *eval;
    int	    *inc;	/* does end value start with '+' for "incremental"? */
			   
{
	/* 13 is 1 plus PANEL_VALUE_STORED_LENGTH of various text items. */
    char    sbuf[13], ebuf[13];
    long    val;

    if (num_token(stx, sbuf, sval))
	return "Badly formed starting value.";
    if (num_token(etx, ebuf, &val))
	return "Badly formed end value.";
    switch (ebuf[0])
    {
    case '\0':
	*inc = NO;
	break;
    case '+':
	*inc = YES;
	if (sbuf[0] == '\0')
	    return "<empty>:+<incr> not allowed.";
	*eval = *sval + val;
	break;
    default:
	*inc = NO;
	*eval = val;
	break;
    }

    return NULL;
}


static char *
fea_panel_range(ftx, stx, etx, fbuf, sval, eval, inc, hd)
    char	    *ftx, *stx, *etx;
    char	    *fbuf;
    long	    *sval, *eval;
    int		    *inc;
    struct header   *hd;
{
    char	    *errmsg;
    static char	    errbuf[150];
    long	    fld_size;
/*!*//* Much in common with plot3d.c.  Consolidate. */

    switch (sscanf(ftx, "%s%s", fbuf, fbuf))
    {
    case EOF:
	return "Empty field name.";
/*!*//* or allow for default */
	break;
    case 1:
	break;
    default:
	return "Badly formed field name.";
	break;
    }

    switch (get_fea_type(fbuf, hd))
    {
    case UNDEF:
	return "Field not defined in file.";
	break;
    case DOUBLE:
    case FLOAT:
    case LONG:
    case SHORT:
    case BYTE:
	break;
    case DOUBLE_CPLX:
    case FLOAT_CPLX:
    case LONG_CPLX:
    case SHORT_CPLX:
    case BYTE_CPLX:
	return "Field is complex.";
	break;
    case CHAR:
    case CODED:
    case EFILE:
    case AFILE:
    default:
	return "Field is non-numeric.";
    }

    fld_size = get_fea_siz(fbuf, hd, (short *) NULL, (long **) NULL);

    *sval = 0;
    *eval = fld_size - 1;
    errmsg = lpanel_range(stx, etx, sval, eval, inc);
    if (errmsg)
    {
	sprintf(errbuf, "Item range: %s", errmsg);
	return errbuf;
    }

    if (*eval > fld_size - 1)
	*eval = fld_size - 1;
    if (*sval < 0)
	return "Item range: starting value negative.";
    if (*eval < *sval)
	return "Item range: empty.";

    return NULL;
}


char *
iopen(name, hd, strm)
    char	    *name;
    struct header   **hd;
    FILE	    **strm;
{
    char    *msg;

    *strm = fopen(name, "r");
    if (!*strm)
	return "Can't open file.";
    *hd = read_header(*strm);

    if (!*hd)
	msg = "Not an ESPS file.";
    else
    if ((*hd)->common.type != FT_FEA)
    {
	*hd = NULL;	/* Memory leak.  Need a free_header function. */
	msg = "Not an ESPS FEA file.";
    }
    else
	return NULL;

    fclose(*strm);
    *strm = NULL;
    return msg;
}


static char *
ld_dat(dirtx, filetx, srectx, erectx, fldtx, sitemtx, eitemtx)
    char	    *dirtx, *filetx;
    char	    *srectx, *erectx;
    char	    *fldtx, *sitemtx, *eitemtx;
{
    extern void	    repaint();
    char    path[241];
/*!*//* 241 is 1 plus sum of PANEL_VALUE_STORED_LENGTH of ld_dat_dir_text
	and ...file_text.  Use defined constants. */
    char	    *errmsg;
    static char	    errbuf[150];
    long	    srec, erec;
    /* 81 is 1 plus PANEL_VALUE_STORED_LENGTH of ld_dat_fld_text */
    char	    fld[81];
    long	    sitem, eitem;
/*!*/
    int		    iteminc, recinc;	/* Do end item and end record start
					    with '+' for "incremental"? */
    struct header   *hd;
    FILE	    *file;

    data_loaded = 1;

    errmsg = make_path(dirtx, filetx, path, &errmsg);
    if (errmsg)
	return errmsg;

    errmsg = iopen(path, &hd, &file);
    if (errmsg)
	return errmsg;

    errmsg = fea_panel_range(fldtx, sitemtx, eitemtx,
			     fld, &sitem, &eitem, &iteminc, hd);
    if (errmsg)
	return errmsg;

/*!*//* Much in common with plot3d.c.  Consolidate. */
    srec = 1;
    erec = LONG_MAX;
    errmsg = lpanel_range(srectx, erectx, &srec, &erec, &recinc);
    if (errmsg)
    {
	sprintf(errbuf, "Record range: %s", errmsg);
	return errbuf;
    }
    if (srec < 1)
	return "Record range: starting value not positive.";
    if (erec < srec)
	return "Record range: empty.";

    errmsg = read_data(file, hd, fld, sitem, eitem, &srec, &erec);

    if (debug_level)
	fprintf(stderr,
	    "path \"%s\"\trecords %ld: %ld\nfield \"%s\"\titems %ld: %ld\n",
	    path, srec, erec, fld, sitem, eitem);

    if (errmsg) {
	data_loaded = 0;
	return errmsg;
    }
    

    repaint(); 
    set_frame_title(path);
    
    return NULL;
}

static int
ld_dat_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    char	*errmsg = NULL;

    if (debug_level)
	Fprintf(stderr, "ld_dat_notify\n");

    errmsg = ld_dat((char *) xv_get(ld_dat_dir_text, PANEL_VALUE),
		    (char *) xv_get(ld_dat_file_text, PANEL_VALUE),
		    (char *) xv_get(ld_dat_startrec_text, PANEL_VALUE),
		    (char *) xv_get(ld_dat_endrec_text,	PANEL_VALUE),
		    (char *) xv_get(ld_dat_fld_text, PANEL_VALUE),
		    (char *) xv_get(ld_dat_startitem_text, PANEL_VALUE),
		    (char *) xv_get(ld_dat_enditem_text, PANEL_VALUE));

    xv_set(ld_dat_msg,
			PANEL_LABEL_STRING,	    errmsg ? errmsg : "",
			0);
    xv_set(item,
			PANEL_NOTIFY_STATUS,	    errmsg ? XV_ERROR : XV_OK,
			0);

    return XV_OK;
}


static char *
ld_par(dirtx, filetx)
    char    *dirtx, *filetx;
{
    extern int		want_box;
    extern Panel_item   ori_setting;
    extern Panel_item   len_slider, dep_slider, rot_slider;
    extern Panel_item   wid_slider, hsk_slider, bea_slider;
    extern Panel_item   hgt_slider, vsk_slider, ele_slider;
    extern char		*onames[];

    char	path[241];
    char	*errmsg;

    int		length, width, height;
    double	depth;
    double	alpha, beta;
    int		orientation;
    char	*ostr = NULL;
    double	theta, phi, psi;

    errmsg = make_path(dirtx, filetx, path, &errmsg);
    if (errmsg)
	return errmsg;

    if (read_params(path, SC_NOCOMMON, (char *) NULL))
	return "Can't read file.";

/*!*//* Code in common with plot3d.c & slider creation.  Consolidate. */
    do_axes();
    do_box();

    getsym("box_length", ST_INT, (char *) &length);
    getsym("box_width", ST_INT, (char *) &width);
    getsym("box_height", ST_INT, (char *) &height);
    set_box_len(length);
    set_box_wid(width);
    set_box_hgt(height);
    xv_set(len_slider,
			PANEL_VALUE,		    length,
			0);
	    xv_set(wid_slider,
			PANEL_VALUE,		    width,
			0);
	    xv_set(hgt_slider,
			PANEL_VALUE,		    height,
			0);
    getsym("depth", ST_FLOAT, (char *) &depth);
    set_finv(depth);
    xv_set(dep_slider,
			PANEL_VALUE,		    ROUND(depth),
			0);
    getsym("horizontal_skew", ST_FLOAT, (char *) &alpha);
    getsym("vertical_skew", ST_FLOAT, (char *) &beta);
    set_hskew(alpha);
    set_vskew(beta);
    xv_set(hsk_slider,
			PANEL_VALUE,		    ROUND(100.0*alpha),
			0);
    xv_set(vsk_slider,
			PANEL_VALUE,		    ROUND(100.0*beta),
			0);
    getsym("orientation", ST_STRING, (char *) &ostr);
    if (ostr)
    {
	orientation = lin_search(onames, ostr);
	if (orientation == -1)
	{
	    Fprintf(stderr, "%s: orientation \"%s\" not recognized.\n",
		    "ld_par", ostr);
	}
	else
	    set_ori(orientation);
    }
    xv_set(ori_setting,
			PANEL_VALUE,		    (orientation == ORI_LEFT)
							? 0 : 1,
			0);
    getsym("bearing", ST_FLOAT, (char *) &theta);
    getsym("elevation", ST_FLOAT, (char *) &phi);
    getsym("rotation", ST_FLOAT, (char *) &psi);
    set_bear(theta*PI/180.0);
    set_elev(phi*PI/180.0);
    set_rot(psi*PI/180.0);
    xv_set(bea_slider,
			PANEL_VALUE,		    ROUND(theta),
			0);
    xv_set(ele_slider,
			PANEL_VALUE,		    ROUND(phi),
			0);
    xv_set(rot_slider,
			PANEL_VALUE,		    ROUND(psi),
			0);

    do_axes();
    want_box = YES;
    do_box();

    return NULL;
}

static int
ld_par_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    char	*errmsg;

    if (debug_level)
	Fprintf(stderr, "ld_par_notify\n");

    errmsg = ld_par((char *) xv_get(ld_par_dir_text, PANEL_VALUE),
		    (char *) xv_get(ld_par_file_text, PANEL_VALUE));

    xv_set(ld_par_msg,
			PANEL_LABEL_STRING,	    errmsg ? errmsg : "",
			0);
    xv_set(item,
			PANEL_NOTIFY_STATUS,	    errmsg ? XV_ERROR : XV_OK,
			0);

    return XV_OK;
}


static char *
sav_par(dirtx, filetx)
    char    *dirtx, *filetx;
{
    char	path[241];
    extern char	*onames[];
    char	*errmsg;

    errmsg = make_path(dirtx, filetx, path, &errmsg);
    if (errmsg)
	return errmsg;

    if (fputsym_i("box_length", get_box_len(), path)
	|| fputsym_i("box_width", get_box_wid(), path)
	|| fputsym_i("box_height", get_box_hgt(), path)
	|| fputsym_f("depth", get_finv(), path)
	|| fputsym_f("horizontal_skew", get_hskew(), path)
	|| fputsym_f("vertical_skew", get_vskew(), path)
	|| fputsym_s("orientation", onames[get_ori()], path)
	|| fputsym_f("bearing", get_bear()*180.0/PI, path)
	|| fputsym_f("elevation", get_elev()*180.0/PI, path)
	|| fputsym_f("rotation", get_rot()*180.0/PI, path)
       )
	return "Can't write file.";

    return NULL;
}

static int
sav_par_notify(item, event)
    Panel_item	item;
    Event	*event;
{
    char	*errmsg;

    if (debug_level)
	Fprintf(stderr, "sav_par_notify\n");

    errmsg = sav_par((char *) xv_get(sav_par_dir_text, PANEL_VALUE),
		     (char *) xv_get(sav_par_file_text, PANEL_VALUE));

    xv_set(sav_par_msg,
			PANEL_LABEL_STRING,	    errmsg ? errmsg : "",
			0);
    xv_set(item,
			PANEL_NOTIFY_STATUS,	    errmsg ? XV_ERROR : XV_OK,
			0);

    return XV_OK;
}


static char *
make_path(dir, file, path, msg)
    char    *dir, *file, *path, **msg;
{
    switch (sscanf(dir, "%s%s", path, path))
    {
    case EOF:
	path[0] = '\0';
	break;
    case 1:
	strcat(path, "/");
	path += strlen(path);
	break;
    default:
	return "Badly formed directory name.";
    }

    if (sscanf(file, "%s%s", path, path) != 1)
	return "Badly formed file name.";

    return NULL;
}


