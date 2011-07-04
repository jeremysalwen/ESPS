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
 * Brief description: interface to Xprinter for hardcopy output
 *
 */

static char *sccs_id = "@(#)p3dprint.c	1.1	6/2/93	ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/cms.h>
#include <esps/constants.h>

#define _NO_PROTO


static void  out_place_notify(), out_type_notify();
static Panel_setting outfile_notify(), print_com_notify();
static void resolution_notify(), orient_notify();
static void scale_notify();

static Frame setup_frame;
static Panel setup_panel;
static Panel_item setup_out_place;
static Panel_item setup_resolution, setup_orient, setup_scale;
static Panel_item setup_out_type, setup_outfile;
static Panel_item setup_print_com;

extern Frame panel_frame;
extern int do_color;

/* these are the defaults for PCL or PS output */

int output_resolution = 300;
/*XpOrientation_t output_orientation = XpPORTRAIT; */
double output_scale = 1.0;
int to_printer=1;
char *output_type = "PostScript";
char *output_file = "plot3d.prt", *print_command="cat | lpr";


void
show_print_window()
{
	xv_set(setup_frame, XV_SHOW, TRUE, NULL);
}

static void
show_for_printer()
{
	xv_set(setup_outfile, XV_SHOW, FALSE, NULL);
	xv_set(setup_print_com, XV_SHOW, TRUE, NULL);
}

static void
show_for_file()
{
	xv_set(setup_outfile, XV_SHOW, TRUE, NULL);
	xv_set(setup_print_com, XV_SHOW, FALSE, NULL);
}


void
init_print_window(cms)
 Cms  cms;
{

	
	setup_frame = xv_create(panel_frame, FRAME_CMD,
		XV_WIDTH, 442,
		XV_HEIGHT, 261,
		XV_LABEL, "Printer Setup",
		FRAME_SHOW_FOOTER, FALSE,
		FRAME_SHOW_RESIZE_CORNER, FALSE,
		FRAME_CMD_PUSHPIN_IN, TRUE,
		XV_SHOW, FALSE,
		NULL);

	setup_panel = xv_get(setup_frame, FRAME_CMD_PANEL);
	xv_set(setup_panel,
		XV_X, 0,
		XV_Y, 0,
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		NULL);
    if (do_color)
        xv_set(setup_panel,
                        WIN_CMS,                    cms,
/*
                        WIN_FOREGROUND_COLOR,       COL_PANEL_FG,
                        WIN_BACKGROUND_COLOR,       COL_PANEL_BG,
*/
                        0);


	setup_out_place = xv_create(setup_panel, PANEL_CHOICE,
		XV_X, 160,
		XV_Y, 24,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LAYOUT, PANEL_VERTICAL,
		PANEL_CHOOSE_NONE, FALSE,
		PANEL_NOTIFY_PROC, out_place_notify,
		PANEL_CHOICE_STRINGS,
			"Printer",
			"File",
			NULL,
		NULL);

	setup_out_type = xv_create(setup_panel, PANEL_CHOICE,
		XV_X, 32,
		XV_Y, 72,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_CHOOSE_NONE, FALSE,
		PANEL_NOTIFY_PROC, out_type_notify,
		PANEL_CHOICE_STRINGS,
			"Postscript",
			"PCL",
			NULL,
		NULL);

        setup_resolution = xv_create(setup_panel, PANEL_NUMERIC_TEXT,
                XV_X, 224,
                XV_Y, 80,
                PANEL_VALUE_DISPLAY_LENGTH, 5,
                PANEL_VALUE_STORED_LENGTH, 5,
                PANEL_LABEL_STRING, "Resolution:",
                PANEL_LAYOUT, PANEL_HORIZONTAL,
                PANEL_MAX_VALUE, 10000,
                PANEL_MIN_VALUE, 0,
                PANEL_VALUE, output_resolution,
                PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, resolution_notify,
                NULL);

        setup_orient = xv_create(setup_panel, PANEL_CHOICE,
                XV_X, 24,
                XV_Y, 120,
                PANEL_CHOICE_NROWS, 1,
                PANEL_LAYOUT, PANEL_HORIZONTAL,
                PANEL_CHOOSE_NONE, FALSE,
                PANEL_CHOICE_STRINGS,
                        "Portrait",
                        "Landscape",
                        NULL,
                PANEL_VALUE, 0,	/* Portrait */
		PANEL_NOTIFY_PROC, orient_notify,
                NULL);

        setup_scale = xv_create(setup_panel, PANEL_NUMERIC_TEXT,
                XV_X, 240,
                XV_Y, 128,
                PANEL_VALUE_DISPLAY_LENGTH, 3,
                PANEL_VALUE_STORED_LENGTH, 3,
                PANEL_LABEL_STRING, "Scale:",
                PANEL_LAYOUT, PANEL_HORIZONTAL,
                PANEL_MAX_VALUE, 100,
                PANEL_MIN_VALUE, 0,
                PANEL_VALUE, (int)output_scale,
                PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, scale_notify,
                NULL);


	setup_outfile = xv_create(setup_panel, PANEL_TEXT,
		XV_X, 24,
		XV_Y, 184,
		PANEL_VALUE_DISPLAY_LENGTH, 40,
		PANEL_VALUE_STORED_LENGTH, 256,
		PANEL_LABEL_STRING, "Filename:",
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE, output_file,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, outfile_notify,
		NULL);


	setup_print_com = xv_create(setup_panel, PANEL_TEXT,
		XV_X, 24,
		XV_Y, 200,
		PANEL_VALUE_DISPLAY_LENGTH, 40,
		PANEL_VALUE_STORED_LENGTH, 80,
		PANEL_LABEL_STRING, "Command:",
		PANEL_LAYOUT, PANEL_HORIZONTAL,
		PANEL_VALUE, print_command,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, print_com_notify,
		NULL);

	show_for_printer();
}

static void
out_place_notify(item, value, event)
        Panel_item      item;
        int             value;
        Event           *event;

{
	switch(value) {
	 case 0: show_for_printer();
		 to_printer = YES;
		 break;
	 case 1: show_for_file();
		 to_printer = NO;
		 break;
	}
}

static void
out_type_notify(item, value, event)
        Panel_item      item;
        int             value;
        Event           *event;

{
	switch(value) {
	 case 0: output_type = "PostScript";
		 break;
	 case 1: output_type = "PCL";
	         break;
	}
}

static void
resolution_notify(item, value, event)
	Panel_item	item;
	int		value;
	Event		*event;
{
	output_resolution = (int)xv_get(item,PANEL_VALUE);
}

static void
scale_notify(item, value, event)
	Panel_item	item;
	int		value;
	Event		*event;
{
	output_scale = (int)xv_get(item,PANEL_VALUE);
}

static void
orient_notify(item, value, event)
	Panel_item 	item;
	int		value;
	Event		*event;
{
  /*	switch(value) {
	 case 0: output_orientation = XpPORTRAIT;
		 break;
	 case 1: output_orientation = XpLANDSCAPE;
		 break;
	}
  */
}


static Panel_setting
outfile_notify(item, event)
	Panel_item	item;
	Event		*event;
{
	output_file = (char *) xv_get(item, PANEL_VALUE);
	
}

static Panel_setting
print_com_notify(item, event)
	Panel_item	item;
	Event		*event;
{
	print_command = (char *) xv_get(item, PANEL_VALUE);
}
