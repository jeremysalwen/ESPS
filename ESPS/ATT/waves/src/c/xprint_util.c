/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of Entropic
 * Research Laboratory, Inc. Any reproduction, distribution, or publication
 * of this work must be authorized in writing by Entropic Research
 * Laboratory, Inc., and must bear the notice:
 * 
 * "Copyright (c) 1990-1993 Entropic Research Laboratory, Inc. All rights
 * reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written by:  Alan Parker Checked by: Revised by:
 * 
 * Brief description: routines to support xprinter from waves
 * 
 */

static char    *sccs_id = "@(#)xprint_util.c	1.2	8/26/96	ATT/ERL";

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
#include <X11/Xutil.h>
#include <X11/Xatom.h>

extern char	*savestring();

int             doing_print_graphic = 0;

extern int	debug_level;

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

static Display	*xp_printer;

/*********************************************************************/

Display *
get_xp_display()
{
   return xp_printer;
}

/*********************************************************************/

Display *
setup_xp_from_globals(canvas)
    Canvas	    canvas;
{
    char	    command[MAXPATHLEN];
    XpOrientation_t graphic_orientation;
    Display	    *printer;
    char	    notice[50];
    Display	    *theDisp;
    int		    theScreen;
    Colormap	    theCmap;
    int		    cells;
    XColor	    *colorcell_defs;
    int		    i;


    if (!print_graphic_printer)
	sprintf(command, "cat > %s", print_graphic_file);
    else
	strcpy(command, print_graphic_command);

    if (strcasecmp("Landscape", print_graphic_orientation) == 0)
	graphic_orientation = XpLANDSCAPE;
    else if (strcasecmp("Portrait", print_graphic_orientation) == 0)
	graphic_orientation = XpPORTRAIT;
    else
    {
	sprintf(notice_msg, "%s\n%s\n%s\n%s",
		"Unknown value for print_graphic_orientation:",
		print_graphic_orientation,
		"Legal values are Portrait or Landscape.",
		"Resetting it to Portrait now.");
	show_notice(0, notice_msg);

	graphic_orientation = XpPORTRAIT;
	strcpy(print_graphic_orientation, "Portrait");
    }

    printer = XpVaOpenPrinter(NULL,
			      command,
			      print_graphic_type,
			      XpOrientation, graphic_orientation,
			      XpScale, print_graphic_scale,
			      XpResolution, print_graphic_resolution,
			      NULL);

    if (printer == NULL)
    {
	sprintf(notice_msg, "%s\n%s",
		"Cannot configure for graphics print",
		"Be sure $ESPS_BASE/Xp exists and is readable.");
	show_notice(1, notice_msg);

	return NULL;
    }

    if (!strcmp(print_graphic_type, "PostScript"))
    {
	if (print_ps_level != 1 && print_ps_level != 2)
	{
	    sprintf(notice_msg, "%s (%d).\n%s\n%s",
		    "Unsupported print_ps_level",
		    print_ps_level,
		    "Allowed values are 1 and 2.",
		    "Resetting to 2.");
	    show_notice(0, notice_msg);

	    print_ps_level = 2;
	}

	if (XpSetPSLevel(printer, print_ps_level) == 0)
	{
	    show_notice(1, "Internal Error: XpSetPSLevel failed.");

	    return NULL;
	}

	XpSetPSColor(printer, 1);
    }

    /* copy colormap from display to printer */

    if (canvas != XV_NULL)
    {
	extern Cms cms;
	theDisp = (Display *) xv_get(canvas, XV_DISPLAY);

	cells = XpDisplayCells(theDisp, 0);
	colorcell_defs = (XColor *) malloc(sizeof(XColor) * cells);
	for (i = 0; i < cells; i++) 
	    colorcell_defs[i].pixel = i;
	xv_get(cms, CMS_X_COLORS, colorcell_defs);
/*
	for (i = 0; i < cells; i++)  {
	    fprintf(stderr,"pixel %d: ",colorcell_defs[i].pixel);
            fprintf(stderr,"%d %d %d\n",colorcell_defs[i].red,
		colorcell_defs[i].green, colorcell_defs[i].blue);
        }
*/
	XpStoreColors(printer, XpDefaultColormap(printer, 0),
		      colorcell_defs, cells);
	free(colorcell_defs);
    }

    return printer;
}

/*********************************************************************/

void
start_xp(printer, canvas)
    Display	*printer;
    Canvas	canvas;
{
    XpStartDoc(printer, NULL);
    XpStartPage(printer);
    if (canvas != XV_NULL)
	xv_set((Frame) xv_get(canvas, XV_OWNER),
	       FRAME_BUSY, TRUE,
	       NULL);
    doing_print_graphic = 1;
    xp_printer = printer;	/* Why two globals? */
    set_pw_display(printer);
}

/*********************************************************************/

void
finish_xp(printer, canvas)
    Display	*printer;
    Canvas	canvas;
{
    set_pw_display(NULL);
    xp_printer = NULL;
    doing_print_graphic = 0;
    XpEndPage(printer);
    XpEndDoc(printer);
    XpClosePrinter(printer);

    if (canvas != XV_NULL)
	xv_set((Frame) xv_get(canvas, XV_OWNER),
	       FRAME_BUSY, FALSE,
	       NULL);
}

/*********************************************************************/

Display *
setup_xp_EPS_temp(canvas, filename, colorcell_defs, cells)
    Canvas	    canvas;
    char	    *filename;
    XColor	    *colorcell_defs;
    int		    cells;
{
    char	    command[MAXPATHLEN];
    Display	    *printer;
    char	    notice[50];
    Display	    *theDisp;
    int		    theScreen;
    Colormap	    theCmap;
    int		    i;


    if (!filename || !*filename)
	return NULL;

    sprintf(command, "cat > %s", filename);

    printer = XpVaOpenPrinter(NULL,
			      command,
			      "PostScript",
			      XpOrientation, XpPORTRAIT,
			      XpScale, 1.0,
			      XpResolution, print_graphic_resolution,
			      NULL);

    if (printer == NULL)
    {
	sprintf(notice_msg, "%s\n%s",
		"Cannot configure for graphics print",
		"Be sure $ESPS_BASE/Xp exists and is readable.");
	show_notice(1, notice_msg);

	return NULL;
    }

    if (print_ps_level != 1 && print_ps_level != 2)
    {
	sprintf(notice_msg, "%s (%d).\n%s\n%s",
		"Unsupported print_ps_level",
		print_ps_level,
		"Allowed values are 1 and 2.",
		"Resetting to 2.");
	show_notice(0, notice_msg);

	print_ps_level = 2;
    }

    if (XpSetPSLevel(printer, print_ps_level) == 0)
    {
	show_notice(1, "Internal Error: XpSetPSLevel failed.");

	return NULL;
    }

    XpSetPSColor(printer, 1);

    /* copy colormap from display to printer */

    if (colorcell_defs)
	XpStoreColors(printer, XpDefaultColormap(printer, 0),
		      colorcell_defs, cells);
    else if (canvas != XV_NULL)
    {
	extern Cms cms;
	theDisp = (Display *) xv_get(canvas, XV_DISPLAY);

	cells = XpDisplayCells(theDisp, 0);
	colorcell_defs = (XColor *) malloc(sizeof(XColor) * cells);
	for (i = 0; i < cells; i++)
	    colorcell_defs[i].pixel = i;
	xv_get(cms, CMS_X_COLORS, colorcell_defs);
	XpStoreColors(printer, XpDefaultColormap(printer, 0),
		      colorcell_defs, cells);
	free(colorcell_defs);
    }

    return printer;
}
