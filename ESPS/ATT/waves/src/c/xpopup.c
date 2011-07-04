/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Mark Beutnagel, AT&T Bell Laboratories
 * Checked by:
 * Revised by:  Rodney Johnson, Entropic Research Laboratory
 *              (convert to XView)
 *
 *
 * popup.c - popup window for string entry
 *
 * popup_get_string() makes a window at specified screen coordinates;
 * creates text input field using specified prompt and length;
 * initializes text input field with original string value;
 * quits when user types [RETURN] or clicks on CANCEL button;
 * returns string pointer or null, respectively.
 */

static char *sccs_id = "@(#)xpopup.c	1.2	9/26/95	ATT/ERL";


#include <stdio.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/canvas.h>
#include <xview/text.h>
#ifndef APOLLO_68K
#include <xview/seln.h>
#endif

extern Frame	    base_frame;

static Frame	    popup = XV_NULL;
static Panel	    panel;
static Panel_item   str_item;
static void	    (*proc)();
static char	    *data;
static char	    *sptr;
static int	    slen;


static Panel_setting
accept_proc(item, event)
    Panel_item	item;
    Event	*event;
{
    char	*str_val;

    str_val = (char *) xv_get(str_item, PANEL_VALUE);
    if (*str_val)
    {
	strncpy(sptr, str_val, slen);
	sptr[slen] = '\0';

	(*proc)(data);

	xv_set(popup,
			XV_SHOW,		FALSE,
			0);
    }

    return PANEL_NONE;
}


static int
cancel_proc(item, event)
    Panel_item	item;
    Event	*event;
{
    xv_set(item,
			PANEL_NOTIFY_STATUS,	XV_OK,
			0);

    return XV_OK;

}


void
popup_get_string(prompt, str, strn, x, y, p, continuation)
    char	*prompt;	/* prompt msg */
    char	*str;		/* holds string (old & new) */
    int		strn;		/* max length of string */
    int		x, y;		/* position of calling panel item */
    char	*p;
    void	(*continuation)();
{
    Rect	rect;

    if(!prompt || !*prompt || !str || strn<=0) return;

    if (popup == XV_NULL)
    {
	popup = xv_create(base_frame, FRAME_CMD,
			XV_SHOW,		FALSE,
			FRAME_SHOW_HEADER,	FALSE,
			FRAME_SHOW_FOOTER,	FALSE,
                        FRAME_CMD_PUSHPIN_IN,	FALSE,
			0);
	panel = xv_get(popup, FRAME_CMD_PANEL);
	str_item = xv_create(panel, PANEL_TEXT,
			PANEL_NOTIFY_PROC,	accept_proc,
			0);
	(void) xv_create(panel, PANEL_BUTTON,
			PANEL_LABEL_STRING,	"Cancel",
			PANEL_NOTIFY_PROC,	cancel_proc,
			0);

    }

    xv_set(panel,
			PANEL_CARET_ITEM,	str_item,
			0);

    xv_set(str_item,
			PANEL_VALUE_DISPLAY_LENGTH,
						strn,
			PANEL_LABEL_STRING,	prompt,
			PANEL_VALUE,		str,
			0);
    proc = continuation;
    data = p;
    sptr = str;
    slen = strn;

    window_fit(panel);
    window_fit(popup);

    frame_get_rect(popup, &rect);
    rect.r_left = x;
    rect.r_top = y;
    frame_set_rect(popup, &rect);

    xv_set(popup,
			XV_SHOW,		TRUE,
			0);
}

