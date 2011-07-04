/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1989-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John shore
 * Checked by:
 * Revised by:
 *
 * Brief description: functions for creating help and text windows
 *
 */

static char *sccs_id = "@(#)xhelp.c	1.24 4/6/93 ERL/ESI";

#ifdef SG
#define NO_TEXTSW
#define USE_XTEXT
#endif

/*
 * system include files
 */
#include <stdio.h>
#include <xview/xview.h>
#include <xview/defaults.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/svrimage.h>
#include <xview/tty.h>
#include <xview/text.h>
#include <xview/cms.h>
#include <xview/notice.h>
#include <xview/scrollbar.h>

/*
 * other include files
 */

#include <esps/esps.h> 
#include <esps/exview.h>

#ifndef NULL
#define NULL 0
#endif

extern int debug_level;
extern int do_color; 

static void search_forward_proc();
static void search_backward_proc();
static void free_data();
char *savestring();
static void quit_proc();
char *e_temp_name();

int ran_xterm;

int
exv_get_help(item, event)
    Panel_item	item;
    Event       *event;  /* arg is unused */
{
  /* This function will pop up a help window containing a 
    help file or a cleaned man page -- we assume the name is passed 
    via the key EXVK_HELP_NAME; if full path, assume file; otherwise man page;
    we also assume that a title for the window is passed via EXVK_HELP_TITLE
   */
  
  Frame owner;
  Frame help_win;
  char *prog = (char *) xv_get(item, XV_KEY_DATA, EXVK_HELP_NAME);
  char *title = (char *) xv_get(item, XV_KEY_DATA, EXVK_HELP_TITLE);
  char *helpfile = NULL; 
  int x,y;			/* coordinates of help item */

  if (debug_level > 1) 
    Fprintf(stderr, "exv_get_help: calling on help for %s\n", prog);

  owner = (Frame) xv_get((Panel) xv_get(item, XV_OWNER), XV_OWNER);

  if ((helpfile = exv_get_helpfile(prog)) == NULL)  {

  if (ran_xterm) return XV_OK;

  if (debug_level)   
      Fprintf(stderr, "exv_get_help: Can't get help for %s\n", prog);

    x = (int) xv_get(item, XV_X);
    y = (int) xv_get(item, XV_Y);

    (void)notice_prompt(owner, (Event *) NULL,
			NOTICE_MESSAGE_STRINGS, 
			"Couldn't get helpfile or couldn't process",
			"man page.  For man pages, try running eman",
			"in a tty window.", 
			NULL,
			NOTICE_FOCUS_XY,        x, y,
			NOTICE_BUTTON_YES,      "Continue",
			0);

    xv_set(owner,	FRAME_BUSY,	FALSE,
			0); 
  }
  else
  {
    if ((title == NULL) || (strlen(title) == 0)) 
      (void) sprintf(title, "ESPS Help Window");


    help_win = exv_make_text_window(owner, title, "help", helpfile, 
				WITH_FIND_BUTTON, USE_FRAME_INDEP);

    /* Be nice, and remove the temp file containing cleaned ESPS man entry.
     * The file will actually disappear when the calling program exits.  
     */

  if (strchr(prog, '/') == NULL) 
      unlink(helpfile);
  }
  if (help_win == NULL) 
    XV_ERROR;
  else
    return XV_OK;
}

  /*This function will pop up a help window, given an owner frame, 
    a frame title, and a help file; an optional FIND button is provided,
    and another parameter determines if command frame (pushpin) or 
    indpendent frame with icon.  For independent frames, an icon title 
    can be provided.
   */

Frame
exv_make_text_window(owner, window_title, icon_title, textfile, find, indep)
     Frame owner;		/* owning frame (may be NULL)*/
     char *window_title;	/* text window title */
     char *icon_title;		/* icon title (for independent windows)*/
     char *textfile;		/* text file */
     int  find;			/* whether or not to have find button */
     int indep;			/* if true, FRAME, else FRAME_CMD */
{
  Frame help_frame;
  Panel button_panel;
  Panel_item find_panel_item1;
  Panel_item find_panel_item2;
  Panel_item quit_item;
  Textsw help_win;
  Menu find_panel_menu;
  int x,y;			/* coordinates of help item */
  char *notice_message;
  FILE *tfd;
  Textsw_index *first_ptr;
  Panel_item find_string;

  if (debug_level > 1) 
    Fprintf(stderr, "entered exv_make_text_window\n");

  if (textfile == NULL) {
    Fprintf(stderr, 
	    "exv_make_text_window called with NULL textfile - exiting.\n");
    return((Frame) NULL);
  }
  
  if (debug_level > 1) 
    Fprintf(stderr, "exv_make_text_window: textfile is %s\n", textfile);

  /* make sure the file is readable; if not pop a notice and exit*/

  if ((tfd = fopen(textfile, "r")) == NULL) {

    notice_message = (char *) malloc(100);
    sprintf(notice_message, "Couldn't open text file %s", textfile);

    if (owner != NULL) {

      x = (int) xv_get(owner, XV_X);
      y = (int) xv_get(owner, XV_Y);      
      (void)notice_prompt(owner, (Event *) NULL,
			  NOTICE_MESSAGE_STRINGS, 
			  notice_message,
			  NULL,
			  NOTICE_FOCUS_XY,        x, y,
			  NOTICE_BUTTON_YES,      "Continue",
			  0);
    }
    else {
      Fprintf(stderr, "exv_make_text_window: couldn't open %s\n", textfile);
    }
    return((Frame) NULL);
  }
  else {
    fclose(tfd);
  }

  /* busy out the main frame (if there is one)*/

  if (owner != NULL) 
    xv_set(owner, FRAME_BUSY, TRUE, 0);

  /* create text window */ 

  if (indep != USE_FRAME_CMD) 

    help_frame = (Frame)xv_create(owner, FRAME, 
				  0);
  else

    help_frame = (Frame)xv_create(owner, FRAME_CMD, 
				  FRAME_CMD_PUSHPIN_IN,     TRUE,
				  0);
  
  xv_set(help_frame, 
	 XV_LABEL, window_title,
	 XV_SHOW,           FALSE,
	 FRAME_SHOW_FOOTER, FALSE,
	 FRAME_SHOW_RESIZE_CORNER, TRUE,
	 WIN_ROWS,     33,
	 WIN_COLUMNS,  75, /*really weird - if you say 80, on Sun you get 
			     something like 85; might need hacking for
			     other systems - js*/
	 0);

  first_ptr = (Textsw_index *)calloc(1,sizeof(Textsw_index));

  if (first_ptr == NULL) {
    Fprintf(stderr, "exv_make_text_window: alloc failed\n");
    return((Frame) NULL);
    }

  *first_ptr = 1;

  help_win = (Textsw) xv_create(help_frame, TEXTSW, 
				XV_X,                   0,
				XV_Y,                   0,
				OPENWIN_SHOW_BORDERS,   TRUE,
				TEXTSW_FILE,		textfile,
				TEXTSW_BROWSING,	TRUE,
				TEXTSW_DISABLE_LOAD,	TRUE,
				TEXTSW_DISABLE_CD,	TRUE,
				XV_KEY_DATA, EXVK_FIRST, first_ptr,
/*
				XV_KEY_DATA_REMOVE_PROC, free_data,
*/
				0);

  if (find == WITH_FIND_BUTTON) {

    button_panel = xv_create(help_frame, PANEL,
			     PANEL_LAYOUT,		PANEL_HORIZONTAL,
			     OPENWIN_SHOW_BORDERS,	TRUE,
			     XV_X,                      0,
			     XV_Y,                      0,
			     0);

    quit_item = xv_create(button_panel, PANEL_BUTTON,
                        XV_Y, 10,
                        PANEL_LABEL_STRING, "Quit",
			PANEL_CLIENT_DATA, help_frame,
                        PANEL_NOTIFY_PROC, quit_proc,
                        0);


    find_panel_item1 = xv_create(button_panel, PANEL_BUTTON,
				XV_Y,                   10,
				PANEL_LABEL_STRING,    "Search Forward",
				PANEL_NOTIFY_PROC,      search_forward_proc,
				PANEL_CLIENT_DATA,      help_win,
				0);	

    find_panel_item2 = xv_create(button_panel, PANEL_BUTTON,
				XV_Y,                   10,
				PANEL_LABEL_STRING,    "Search Backward",
				PANEL_NOTIFY_PROC,      search_backward_proc,
				PANEL_CLIENT_DATA,      help_win,
				0);	

    find_string = xv_create(button_panel, PANEL_TEXT,
				XV_Y,                   10,
				PANEL_LABEL_STRING,	"Search Text:",
				PANEL_VALUE_STORED_LENGTH,	50,
				PANEL_VALUE_DISPLAY_LENGTH,	22,
				PANEL_NOTIFY_LEVEL,	PANEL_SPECIFIED,
				PANEL_NOTIFY_STRING,	"\n\r",
				PANEL_NOTIFY_PROC, 	search_forward_proc,
				PANEL_CLIENT_DATA,      help_win,
				0);

    xv_set(find_panel_item1, XV_KEY_DATA, EXVK_FIND_STRING, find_string, NULL);
    xv_set(find_panel_item2, XV_KEY_DATA, EXVK_FIND_STRING, find_string, NULL);
    xv_set(find_string, XV_KEY_DATA, EXVK_FIND_STRING, find_string, NULL);

  
    (void)window_fit_height(button_panel);

    xv_set(help_win,	WIN_BELOW,	button_panel,
	   0);

    xv_set(button_panel,	XV_WIDTH,	xv_get(help_win, XV_WIDTH),
	   0);

  }

  xv_set(help_frame,	XV_WIDTH,	xv_get(help_win, XV_WIDTH),
	   0);

  if (indep != USE_FRAME_CMD) 
    (void) exv_attach_icon(help_frame, 
			   ERL_NOBORD_ICON, 
			   (icon_title != NULL ? icon_title : "help"),
			   TRANSPARENT);

  xv_set(help_frame, XV_SHOW, TRUE, 0); 

  if (owner != NULL) 
    xv_set(owner,	FRAME_BUSY, FALSE, 0); 

  return(help_frame);
}

static
void
search_forward_proc(item, event)
	Panel_item item;
	Event *event;
{

  Textsw	textsw = (Textsw) xv_get(item, PANEL_CLIENT_DATA);
  char *s;
  Textsw_index *first_time, first, last_plus_one;
  int top, bottom;
  Panel_item find_string;

  find_string = (Panel_item)xv_get(item,XV_KEY_DATA, EXVK_FIND_STRING);
  if((s = (char *)xv_get(find_string, PANEL_VALUE)) == NULL)
  {
	fprintf(stderr,"");
	return;
  }
  first_time = (Textsw_index *)xv_get(textsw, XV_KEY_DATA, EXVK_FIRST);
  

  if (*first_time) {
	*first_time = 0;
  	first = (Textsw_index)xv_get(textsw,TEXTSW_FIRST);
  } else {
	textsw_file_lines_visible(textsw,&top,&bottom);
	first = textsw_index_for_file_line(textsw, top+3);
  }

  if (textsw_find_bytes(textsw, &first, &last_plus_one,
             s, (unsigned)strlen(s), 0) == -1)
  {
	fprintf(stderr,"");
	return;
  }
  textsw_normalize_view(textsw, first);
  textsw_set_selection(textsw,first,last_plus_one, 1);

}



static
void
search_backward_proc(item, event)
	Panel_item item;
	Event *event;
{
  char *s;
  Textsw_index first, last_plus_one;
  Panel_item find_string;

  Textsw	textsw = (Textsw) xv_get(item, PANEL_CLIENT_DATA);

  find_string = (Panel_item)xv_get(item,XV_KEY_DATA, EXVK_FIND_STRING);
  first = (Textsw_index)xv_get(textsw,TEXTSW_FIRST);

  if((s = (char *)xv_get(find_string, PANEL_VALUE)) == NULL)
  {
	fprintf(stderr,"");
	return;
  }

  if (textsw_find_bytes(textsw, &first, &last_plus_one,
             s, (unsigned)strlen(s), 1) == -1)
  {
	fprintf(stderr,"");
	return;
  }
  textsw_normalize_view(textsw, first);
  textsw_set_selection(textsw,first,last_plus_one, 1);
}

char *
exv_get_helpfile(progfile)
    char *progfile;
{
  /*This function decodes the progfile; if it's a bare file, all we 
    do is check that we can read it. If it represents an ESPS program,
    we put cleaned eman output in a temp file and return that name
    */

  char	*filename;		/* filename for help */

  /* command  to clean an ESPS man page; this just removes underlining
     and the copyright line (which has lots of control characters embedded 
     into it; it would be better to leave in a cleaned up copyright and 
     last-changed string, as well as to delete large groups of empty lines;
     We use an sed script; probably should change to awk.
   */

/*  char *clean_com = "sed -e \"s/_//g\" -e \"/Copyright c/d\" -e \"/^$/d\" ";*/

  char	*clean_com = "sed -e \"s/_//g\" -e \"/Copyright c/d\" ";

  /* In the `eman | sed' pipeline further down, the sed returns exit
	status zero for the pipeline even if the eman fails.
	The `test' gives nonzero exit status on an empty file.
	This depends on eman's standard output being empty when it fails. */

  char	*check_com = "test -s";

  FILE	*hfd; 
  int	status = 0;
  char	*template = "ehelpXXXXXX";
  char	command[500];
  char *pager=NULL;	/* name of pager (more,less) to use for machines
			   where xtext doesn't work right */
  char *getenv();

  ran_xterm = 0;

  if (progfile == NULL) 
    return((char *) NULL);

  /* filename assumed to have '/' somewhere */

  if (strchr(progfile, '/') != NULL)
      filename = progfile;

  else { /*assume it's the name of an ESPS program*/

    filename = e_temp_name(template); 

    if (filename == (char *)NULL) 
      return((char *)NULL); 

#if  !defined(SUN3) && !defined(SUN4)
    sprintf(command, "xterm -e  eman %s &", progfile);
    ran_xterm = 1;
    status = system(command);
    return ((char *)NULL);
#else
    sprintf(command, "eman %s | %s > %s ; %s %s",
	    progfile, clean_com, filename, check_com, filename);
    if (debug_level)
      Fprintf(stderr, "exv_get_helpfile: running shell command: %s\n", command);
    status = system(command);

#endif
  }

  if (((hfd = fopen(filename, "r")) == NULL) || (status != 0)) {
    return((char *) NULL);
  }
  else {
    fclose(hfd);
#ifdef NO_TEXTSW
#ifdef USE_XTEXT
    sprintf(command, "xtext -t %s -i %s -F %s &", filename, 
	filename,  filename);
    ran_xterm = 1;
    status = exec_command(command);
    return ((char *)NULL);
#else
    if(!(pager=getenv("PAGER")))
	pager="more";
    sprintf(command, "xterm -T %s -n %s -e  %s %s &", filename, 
	filename, pager, filename);
    ran_xterm = 1;
    status = exec_command(command);
    return ((char *)NULL);
#endif
#else
    return(savestring(filename));
#endif
  }
}


static void
quit_proc(item)
Panel_item item;
{
  Frame frame = (Frame)xv_get(item, PANEL_CLIENT_DATA);
  xv_destroy_safe(frame);
}

static void
free_data(object, key, data)
Xv_object object;
int key;
caddr_t data;
{
	free(data);
}
