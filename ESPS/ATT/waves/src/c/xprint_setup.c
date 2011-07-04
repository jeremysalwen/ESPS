/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1995 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)xprint_setup.c	1.8	08 Sep 1997	ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/notice.h>
#include <esps/constants.h>

#define _NO_PROTO
#include <Xp.h>


static void  out_place_notify(), out_type_notify();
static Panel_setting outfile_notify(), print_com_notify();
static void resolution_notify(), orient_notify();
static void scale_notify(), ps_level_notify();

static Frame setup_frame;
static Panel setup_panel;
static Panel_item setup_out_place;
static Panel_item setup_resolution, setup_orient, setup_scale;
static Panel_item setup_out_type, setup_outfile;
static Panel_item setup_print_com, setup_ps_level;

/* globals for Xprinter control */
extern int print_graphic_printer; 	/* 1 for printer, 0 for file */
extern int print_graphic_resolution;
extern char print_graphic_orientation[]; /* Portrait or Landscape */
extern char print_graphic_type[];    	/* PostScript or PCL */
extern char print_graphic_file[];
extern char print_graphic_command[];
extern double print_graphic_scale;
extern int print_only_plot;
extern int print_ps_level;  	/* level 1 PS results in BW spectrograms */

extern Frame daddy;
extern int do_color;
extern int debug_level;

static void send_attach_all_vals();

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

/* these are the defaults for PCL or PS output */

char *
meth_print_setup(o, str)
	char *o, *str;
{
	if(debug_level)
	  fprintf(stderr,"meth_print_setup\n");
	xv_set(setup_frame, XV_SHOW, TRUE, NULL);
	xv_set(setup_out_place,PANEL_VALUE,print_graphic_printer,NULL);
	if(strcasecmp(print_graphic_type,"PCL")==0) 
		xv_set(setup_out_type,PANEL_VALUE,1,NULL);
	else {
		strcpy(print_graphic_type,"PostScript");
		xv_set(setup_out_type,PANEL_VALUE,0,NULL);
	}
	xv_set(setup_resolution,PANEL_VALUE,print_graphic_resolution,NULL);
	xv_set(setup_scale,PANEL_VALUE,(int)print_graphic_scale,NULL);
	if(strcasecmp(print_graphic_orientation,"portrait")==0)
		xv_set(setup_orient,PANEL_VALUE,0,NULL);
	else {
		strcpy(print_graphic_orientation,"Landscape");
		xv_set(setup_orient,PANEL_VALUE,1,NULL);
	}
	xv_set(setup_outfile,PANEL_VALUE,print_graphic_file,NULL);
	xv_set(setup_print_com,PANEL_VALUE,print_graphic_command,NULL);
	xv_set(setup_ps_level,PANEL_VALUE,print_ps_level-1,NULL);
	if(print_graphic_printer) 
		show_for_printer();
	else
		show_for_file();
	send_attach_all_vals();
	return("ok");
}

void
init_print_setup_window()
{

	
	setup_frame = xv_create(daddy, FRAME,
		XV_WIDTH, 442,
		XV_HEIGHT, 261,
		XV_LABEL, "Printer Setup",
		FRAME_SHOW_FOOTER, FALSE,
/*		FRAME_SHOW_RESIZE_CORNER, FALSE, */
/*		FRAME_CMD_PUSHPIN_IN, TRUE, */
		XV_SHOW, FALSE,
		NULL);

	setup_panel = xv_create(setup_frame, PANEL,
		XV_X, 0,
		XV_Y, 0,
		NULL);


	setup_out_place = xv_create(setup_panel, PANEL_CHOICE,
		XV_X, /* 160, */ 46,
		XV_Y, 24,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LAYOUT, PANEL_VERTICAL,
		PANEL_CHOOSE_NONE, FALSE,
		PANEL_NOTIFY_PROC, out_place_notify,
		PANEL_CHOICE_STRINGS,
			"File",
			"Printer",
			NULL,
		NULL);

	setup_ps_level = xv_create(setup_panel, PANEL_CHOICE,
		XV_X, /* 160, */ 228,
		XV_Y, 24,
		PANEL_CHOICE_NROWS, 1,
		PANEL_LAYOUT, PANEL_VERTICAL,
		PANEL_CHOOSE_NONE, FALSE,
		PANEL_NOTIFY_PROC, ps_level_notify,
		PANEL_CHOICE_STRINGS,
			"PS Level 1",
			"PS Level 2",
			NULL,
		PANEL_VALUE, 1,
		NULL);

	setup_out_type = xv_create(setup_panel, PANEL_CHOICE,
		XV_X, 36,
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
                PANEL_VALUE, print_graphic_resolution,
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
                PANEL_VALUE, (int)print_graphic_scale,
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
		PANEL_VALUE, print_graphic_file,
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
		PANEL_VALUE, print_graphic_command,
		PANEL_READ_ONLY, FALSE,
		PANEL_NOTIFY_PROC, print_com_notify,
		NULL);


	show_for_printer();
}

static
char str[100];

static void
ps_level_notify(item, value, event)
        Panel_item      item;
        int             value;
        Event           *event;

{
	switch(value) {
	 case 1: print_ps_level = 2;
		 break;
	 case 0: print_ps_level = 1;
	         notice_prompt(daddy,NULL,
	 	     NOTICE_MESSAGE_STRINGS,
			"PS Level 1 will result in monochrome spectrograms",
			"Use grayscale colormap for best results",
		     NULL,
		     NOTICE_BUTTON_YES, "Continue",
		     NULL);
		 break;
	}
	sprintf(str,"set print_ps_level %d",print_ps_level);
	xwaves_ipc_send_attachment("all",str);
}


static void
out_place_notify(item, value, event)
        Panel_item      item;
        int             value;
        Event           *event;

{
	switch(value) {
	 case 1: show_for_printer();
		 print_graphic_printer = 1;
		 break;
	 case 0: show_for_file();
		 print_graphic_printer = 0;
		 break;
	}
	sprintf(str,"set print_graphic_printer %d",print_graphic_printer);
	xwaves_ipc_send_attachment("all",str);
}

static void
out_type_notify(item, value, event)
        Panel_item      item;
        int             value;
        Event           *event;

{
	switch(value) {
	 case 0: strcpy(print_graphic_type, "PostScript");
		 break;
	 case 1: strcpy(print_graphic_type, "PCL");
	         notice_prompt(daddy,NULL,
	 	     NOTICE_MESSAGE_STRINGS,
			"PCL support is limited.   PostScript is the",
			"best choice for output.",
		     NULL,
		     NOTICE_BUTTON_YES, "Continue",
		     NULL);
		 break;
        }
	sprintf(str,"set print_graphic_type %s",print_graphic_type);
	xwaves_ipc_send_attachment("all",str);
}

static void
resolution_notify(item, value, event)
	Panel_item	item;
	int		value;
	Event		*event;
{
	print_graphic_resolution = (int)xv_get(item,PANEL_VALUE);
	sprintf(str,"set print_graphic_resolution %d",
		print_graphic_resolution);
	xwaves_ipc_send_attachment("all",str);
}

static void
scale_notify(item, value, event)
	Panel_item	item;
	int		value;
	Event		*event;
{
	print_graphic_scale = (int)xv_get(item,PANEL_VALUE);
	sprintf(str,"set print_graphic_scale %f",print_graphic_scale);
	xwaves_ipc_send_attachment("all",str);
}

static void
orient_notify(item, value, event)
	Panel_item 	item;
	int		value;
	Event		*event;
{
	switch(value) {
	 case 0: strcpy(print_graphic_orientation, "Portrait");
		 break;
	 case 1: strcpy(print_graphic_orientation, "Landscape");
		 break;
	}
	sprintf(str,"set print_graphic_orientation %s",
			print_graphic_orientation);
	xwaves_ipc_send_attachment("all",str);
}


static Panel_setting
outfile_notify(item, event)
	Panel_item	item;
	Event		*event;
{
	strcpy(print_graphic_file, (char *) xv_get(item, PANEL_VALUE));
	sprintf(str,"set print_graphic_file %s", print_graphic_file);
	xwaves_ipc_send_attachment("all",str);
	
}

static Panel_setting
print_com_notify(item, event)
	Panel_item	item;
	Event		*event;
{
	strcpy(print_graphic_command, (char *) xv_get(item, PANEL_VALUE));
	sprintf(str,"set print_graphic_command %s", print_graphic_command);
	xwaves_ipc_send_attachment("all",str);
}

static void
send_attach_all_vals()
{
	sprintf(str,"set print_graphic_printer %d",print_graphic_printer);
	xwaves_ipc_send_attachment("all",str);
	sprintf(str,"set print_graphic_type %s",print_graphic_type);
	xwaves_ipc_send_attachment("all",str);
	sprintf(str,"set print_graphic_resolution %d",
		print_graphic_resolution);
	xwaves_ipc_send_attachment("all",str);
	sprintf(str,"set print_graphic_scale %f",print_graphic_scale);
	xwaves_ipc_send_attachment("all",str);
	sprintf(str,"set print_graphic_orientation %s",
			print_graphic_orientation);
	xwaves_ipc_send_attachment("all",str);
	sprintf(str,"set print_graphic_file %s", print_graphic_file);
	xwaves_ipc_send_attachment("all",str);
	sprintf(str,"set print_graphic_command %s", print_graphic_command);
	xwaves_ipc_send_attachment("all",str);
	sprintf(str,"set print_ps_level %d", print_ps_level);
	xwaves_ipc_send_attachment("all",str);
}
