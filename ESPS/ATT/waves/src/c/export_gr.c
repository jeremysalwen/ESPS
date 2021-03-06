/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 * 
 *    "Copyright (c) 1990-1993 Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 * 
 * Brief description: routines to support xprinter from waves
 * 
 */

static char    *sccs_id = "@(#)export_gr.c	1.18	10/1/97	ATT/ERL";

#include <stdio.h>
#include <xview/font.h>
#if !defined(APOLLO_68K) && !defined(DS3100)
#include <malloc/malloc.h>
#endif
#include <xview/notice.h>
#include <esps/unix.h>
#include <Objects.h>
#include <esps/exview.h>
#define _NO_PROTO
#include <Xp.h>
#include <sys/param.h>
#include <limits.h>
#include <xprint_util.h>

extern char	*savestring();
extern char	*get_next_item();
extern char	*basename();

int             debug_level = 0;

/* from globals.c */
extern int      print_graphic_printer;
extern int      print_graphic_resolution;
extern char     print_graphic_orientation[];
extern char     print_graphic_type[];
extern char     print_graphic_file[];
extern char     print_graphic_command[];
extern double   print_graphic_scale;
extern int      print_ps_level;
extern char     temp_path[];

static void	cont_print_ensemble();
extern int cmap_depth;

/*********************************************************************/

void
e_print_graphic(canvas, event, arg)
    Canvas          canvas;
    Event          *event;
    caddr_t         arg;
{
    Display        *printer;
    View           *v;
    Signal         *s;


    if (cmap_depth > 8) {
  	sprintf(notice_msg, 
	        "Sorry: print_graphic isn't implemented for colormap depth %d",
	        cmap_depth);
        show_notice(0,notice_msg);
        return;
    }
    printer = setup_xp_from_globals(canvas);
    if (!printer)
	return;

    start_xp(printer, canvas);

    /* re-render image here */
    v = (View *) xv_get(canvas, WIN_CLIENT_DATA);
    s = v->sig;

    for (v = s->views; v; v = v->next)
	plot_view(v);
    /**/

    finish_xp(printer, canvas);
}


/*********************************************************************/

extern Frame	daddy;
extern char	ens_print_atts[];

typedef struct file_info file_info;
typedef struct cont_info cont_info;

struct file_info {
    int		x, y;
    char	*filename;
    file_info	*next;
};

struct cont_info {
    Object	*obj;
    Display	*printer;
    char	*functions;
    char	*next_func;
    file_info	*att_files;
    char    	*next_file;
    int		minx, miny;
};


void
do_print_ensemble(ob)
    Object	    *ob;
{
    Display	    *printer;
    cont_info	    *env;
    char	    *func;

    if (!ob || !ob->name)
	return;

    if (debug_level)
	fprintf(stderr, "do_print_ensemble().\n");

    if (cmap_depth > 8) {
  	sprintf(notice_msg, 
	        "Sorry: ensemble printing isn't implemented for colormap depth %d",
	        cmap_depth);
        show_notice(0,notice_msg);
        return;
    }

    if (strcmp(print_graphic_type, "PostScript"))
    {
	sprintf(notice_msg, "%s\n%s\n%s",
		"Sorry: ensemble printing isn't implemented for",
		print_graphic_type,
		"only PostScript.");
	show_notice(0, notice_msg);
	return;
    }


    /* 
     * Set up output printer.
     */

    printer = setup_xp_from_globals(XV_NULL);
    if (!printer)
	return;

    XpStartDoc(printer, NULL);
    XpStartPage(printer);

    /*
     * Set up continuation for getting attachment graphics.
     */

    env = (cont_info *) malloc(sizeof(cont_info));
    if (!env)
    {
	show_notice(1, "Allocation failure in do_print_ensemble.");
	return;
    }

    env->obj = ob;
    env->printer = printer;
    env->functions =
	func = savestring(ens_print_atts);
    env->next_func = func + strspn(func, " \t\n");
    env->att_files = NULL;
    env->next_file = NULL;
    env->minx = INT_MAX;
    env->miny = INT_MAX;

    /* Hand off the task to cont_print_ensemble. */

    cont_print_ensemble((void *) env, "");
}


/*
 * For each attachment listed in the global "ens_print_atts",
 * "cont_print_ensemble" sends a command of the form
 * 	<object> print_EPS_temp output <temp_file> return_id <id>
 * having first added itself to a list of pending callbacks to be
 * resumed when a reply to the messages is received.  (Here <object>
 * is the object name, <temp_file> is a file name obtained from
 * "mktemp", and <id> is a number obtained from
 * "set_return_value_callback" to identify the callback).
 * If the attachment has a view of the object to be included in the
 * "print ensemble" output, it writes a temporary EPS file with
 * the indicated temp file name and replies with a message of the
 * form
 *	<host> completion <id> loc_x <x> loc_y <y>
 * where <host> is normally "waves", and <x> and <y> are the screen
 * coordinates of the upper left corner of the view.  (The "loc_x <x>
 * loc_y <y>" portion is made available to the callback via the argument
 * "str".)  Upon receiving such a reply, "cont_print_ensemble" adds the
 * temp file name and the coordinates to a list of files with screen
 * coordinates.  If the attachment has no visible view for the object,
 * it replies with
 *	<host> completion <id>
 * (which results in an empty string for "str").  In that case,
 * "cont_print_ensemble" adds nothing to the list of temp files, but
 * simply proceeds with the next attachment.  (The initial call from
 * "do_print_ensemble" has an empty string for "str", so the function
 * proceeds to the first attachment without adding anything to the
 * list.)  The function also proceeds to the next attachment is
 * the attemp to send the "print_EPS_temp" command fails.
 * After the last attachment has been processed, the function
 * determines the screen coordinates of the bounding box of all
 * attachment views and xwaves views for the object, starts the output
 * file, and incorporates each attachment temp file by using the
 * Xprinter function "XpEPS_Put".  It then loops through all views
 * of the object displayed by xwaves itself, writing a temporary
 * EPS file for each and incorporating the temp file in the output
 * by using "XpEPS_Put".
 * Between invocations as a callback, "cont_print_ensemble" keeps
 * necessary state information in a "cont_info" structure, not static
 * variables; more than one "print ensemble" may be in progress at the
 * same time.
 */

static void
cont_print_ensemble(ptr, str)
    void    *ptr;
    char    *str;
{
    static int	x, y;
    static Selector
	s1 = {"loc_x", "%d", (char *) &x, NULL},
	s0 = {"loc_y", "%d", (char *) &y, &s1};
    cont_info	*env;
    file_info	*att_file;
    char	*func;
    char	temp_graphic_file[MAXPATHLEN];
    int		id;
    char	msg[MES_BUF_SIZE];
    Object	*ob;
    Display	*printer;
    int		minx, miny;
    View	*v;
    Signal	*s;
    Frame	frame;
    Canvas	canvas = XV_NULL;
    Display	*theDisp;
    int		theScreen;
    Colormap	theCmap;
    int		cells;
    XColor	*colorcell_defs;
    int		i;
    FILE	*file;
    XpEPS_trf	transf;
    file_info	*next_att;
    double	xtrans, ytrans;
    Display	*t_printer;
    Rect 	rect;


    if (debug_level)
	fprintf(stderr, "cont_print_ensemble: str \"%s\"\n",
		(str) ? str : "<NULL>");

    if (!ptr || !str)
	return;

    env = (cont_info *) ptr;
    ob = env->obj;

    if (get_args(str, &s0) == 2)
    {
	if (x < env->minx)
	    env->minx = x;
	if (y < env->miny)
	    env->miny = y;

	if (debug_level)
	    fprintf(stderr,
		    "%s: attachment window at %d %d; origin now %d %d.\n",
		    "cont_print_ensemble", x, y, env->minx, env->miny);

	att_file = (file_info *) malloc(sizeof(file_info));
	if (!att_file)
	{
	    show_notice(1, "allocation failure in cont_print_ensemble.");
	    return;
	}

	att_file->x = x;
	att_file->y = y;
	att_file->filename = env->next_file;
	att_file->next = env->att_files;
	env->att_files = att_file;
    }
    else if (env->next_file)
	free(env->next_file);

    func = env->next_func;
    if (*func)
    {
	env->next_func = get_next_item(func);
	func = strtok(func, " \t\n");
	func = basename(func);
	sprintf(temp_graphic_file, "%s/%s%s",
		temp_path, func, "PSXXXXXX");
#if !defined(LINUX)
	(void) mktemp(temp_graphic_file);
#else
	(void) mkstemp(temp_graphic_file);
#endif
	env->next_file = savestring(temp_graphic_file);

	id = set_return_value_callback(cont_print_ensemble, (void *) env);

	sprintf(msg, "%s print_EPS_temp output %s return_id %d",
		ob->name, temp_graphic_file, id);

	if (debug_level)
	    fprintf(stderr, "%s: xwaves_ipc_send(%s, \"%s\").\n",
		    "cont_print_ensemble",
		    (func) ? func : "<NULL>", (msg) ? msg : "<NULL>");

	if (!xwaves_ipc_send(func, msg))
	{
	    if (debug_level)
		fprintf(stderr, "xwaves_ipc_send failed.\n");

	    cont_print_ensemble((void *) env, "");
	}

	return;
    }

    /*
     * Find a visible canvas, if any, among the views of the signals
     * of the ensemble.  Find the upper left corner of the bounding
     * box of all such canvases and all areas to be included for
     * attachment views.
     */

    printer = env->printer;
    minx = env->minx;
    miny = env->miny;

    for (s = ob->signals; s; s = s->others)
	for (v = s->views; v; v = v->next)
	    if (v->canvas != XV_NULL)
	    {
		frame = (Frame) xv_get(v->canvas, XV_OWNER);

		if (!xv_get(frame, FRAME_CLOSED))
		{
		    if (canvas == XV_NULL)
			canvas = v->canvas;

		    frame_get_rect(frame, &rect);
		    x = rect.r_left;
		    if (x < minx)
			minx = x;

		    y = rect.r_top;
		    if (y < miny)
			miny = y;

		    if (debug_level)
			fprintf(stderr,
				"%s: canvas at %d %d; origin now %d %d.\n",
				"cont_print_ensemble", x, y, minx, miny);
		}
	    }

    if (canvas == XV_NULL && !env->att_files)
	return;

    if (canvas != XV_NULL)
    {
	/* Copy colormap from display. */

	extern Cms cms;
        theDisp = (Display *) xv_get(canvas, XV_DISPLAY);
        cells = XpDisplayCells(theDisp, 0);
        colorcell_defs = (XColor *) malloc(sizeof(XColor) * cells);
        for (i = 0; i < cells; i++)
	    colorcell_defs[i].pixel = i;
        xv_get(cms, CMS_X_COLORS, colorcell_defs);
        XpStoreColors(printer, XpDefaultColormap(printer, 0),
		      colorcell_defs, cells);
    }

    att_file = env->att_files;
    while (att_file)
    {
	if (debug_level)
	    fprintf(stderr, "cont_print_ensemble: att_file %s.\n",
		    att_file->filename);

	file = fopen(att_file->filename, "r");

	if (file)
	{
	    transf.origin_x = att_file->x - minx;
	    transf.origin_y = att_file->y - miny;
	    transf.scale_x = 1.0;
	    transf.scale_y = 1.0;
	    transf.rotate = 0.0;

	    XpEPS_Put(printer, file, &transf);

	    unlink(att_file->filename);
	}

	next_att = att_file->next;
	free(att_file->filename);
	free(att_file);
	att_file = next_att;
    }

    free(env->functions);
    free(env);

    sprintf(temp_graphic_file, "%s/%s", temp_path, "wavesPSXXXXXX");
#if !defined(LINUX)
    (void) mktemp(temp_graphic_file);
#else
    (void) mkstemp(temp_graphic_file);
#endif

    /* re-render image here */

    if (debug_level)
	fprintf(stderr, "print_ensemble: origin %d %d\n", minx, miny);

    for (s = ob->signals; s; s = s->others)
	for (v = s->views; v; v = v->next)
	    if ((canvas = v->canvas) != XV_NULL)
	    {
		frame = (Frame) xv_get(v->canvas, XV_OWNER);

		if (!xv_get(frame, FRAME_CLOSED))
		{
                    frame_get_rect(frame, &rect);
		    x = rect.r_left;
	            y = rect.r_top;
		    xtrans = x - minx;
		    ytrans = y - miny;

		    if (debug_level)
			fprintf(stderr,
				"print_ensemble: offset %g %g\n",
				xtrans, ytrans);

		    /* Set up printer for EPS temp file. */

		    t_printer =
			setup_xp_EPS_temp(canvas, temp_graphic_file,
					  colorcell_defs, cells);
		    if (!t_printer)
			return;

		    start_xp(t_printer, canvas);

		    plot_view(v);

		    finish_xp(t_printer, canvas);

		    file = fopen(temp_graphic_file, "r");
		    if (!file)
		    {
			XpAbortDoc(t_printer);
			break;
		    }

		    transf.origin_x = xtrans;
		    transf.origin_y = ytrans;
		    transf.scale_x = 1.0;
		    transf.scale_y = 1.0;
		    transf.rotate = 0.0;

		    XpEPS_Put(printer, file, &transf);

		    unlink(temp_graphic_file);
		}
	    }
    /**/

    free(colorcell_defs);

    XpEndPage(printer);
    XpEndDoc(printer);

    XpClosePrinter(printer);
}
