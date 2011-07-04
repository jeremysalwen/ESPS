/*
 * This material contains proprietary software of Entropic Research
 * Laboratory, Inc.  Any reproduction, distribution, or publication
 * without the prior written permission of Entropic Research
 * Laboratory, Inc. is strictly prohibited.  Any public distribution
 * of copies of this work authorized in writing by Entropic Research
 * Laboratory, Inc. must bear the notice
 *
 *    "Copyright (c) 1991 Entropic Research Laboratory, Inc.
 *     All rights reserved."
 *
 * Program: xbbox.c
 *
 * Written by:  John Shore
 * Checked by:
 *
 * functions for popping up a button box
 */

#ifndef lint
static char *sccs_id = "@(#)xbbox.c	1.26	6/5/93     ERL";
#endif

/*
 * system include files
 */
#include <stdio.h>
#include <xview/xview.h>
#include <xview/defaults.h>
#include <xview/frame.h>
#include <xview/panel.h>
#include <xview/cms.h>
#include <xview/notice.h>

/*
 * other include files
 */

#include <esps/esps.h> 

#include <esps/exview.h>

#ifndef NULL
#define NULL 0
#endif

#define EXV_WINDOW_TITLE "ESPS Button Panel"

#define EXV_ICON_TITLE "buttons"

#define LAST_VAL_KEY 13432

extern int debug_level;
extern int do_color; 
extern  char **Envp;

static  int check_par();
static  bbox_par *bbox_defaults();
static  int new_row();

char *savestring();
void exec_command();
void print_bbox_par();

static void exec_data();
static void quit_proc();
static void but_selected();

static void menu_item_sel();
static void choice_item_sel();
static int menu_but_selected();
static Panel_item add_button_menu();
static Panel_item add_panel_list();
static int add_menu_file_buttons();


/* 
 * global variables
 */

static int tot_buttons = 0;	/* cumulative button number */
static int last_row = 0;	/* when this changes, a new row has started*/

static bbox_par *bbox_params = NULL; /* global, ugly. */


bbox_par 
*exv_bbox(params, bbox_frame, bbox_panel)
bbox_par *params;
Frame *bbox_frame;
Panel *bbox_panel;
{
  Panel_item button;
  int n_but = 0;
  menudata *but_menu = NULL;
  int but_menu_ok = 1;

#if defined(DS3100) || defined(SG) || defined(hpux)
/* This is an ugly hack because I can't figure out how to fix this
   correctly on the DS3100 XView port.    Without this hack, the
   aux panels in xvwaves come up black and white.  
*/
Xv_singlecolor ffg, fbg;   /* frame foreground/background colors */
int             dffg = 0x000000,        /* default frame foreground color */
                dfbg = 0xf0ffff;        /*    "      "   background   " */
#endif

  bbox_params = params; /* set global; ugly! */

  if (debug_level > 1)
      Fprintf(stderr, "entered exv_bbox\n");

      /* the following call will return if an ESPS or waves 
	 license exists (but not necessarily checked out or
	 available).
      */

  check_header2(); 
#if defined(DS3100) || defined(SG) || defined(hpux)
  ffg.red = (dffg >> 16) & 0xff; /* unpack some colormap values */
  ffg.green = (dffg >> 8 ) & 0xff;
  ffg.blue = dffg & 0xff;
  fbg.red = (dfbg >> 16) & 0xff;
  fbg.green = (dfbg >> 8 ) & 0xff;
  fbg.blue = dfbg & 0xff;
#endif


  /* make sure panel handle is NULL unless we create one */

  *bbox_panel = NULL;
  
  /* generate default parameters if asked */

  if (bbox_params == NULL) {
      bbox_params = bbox_defaults();
      return(bbox_params);
    }

  /* initialization */

  tot_buttons = last_row = 0;

  if (bbox_params->owner == NULL) 
    *bbox_frame = NULL;

  if (!check_par(bbox_params)) 
      return(NULL);

  if (bbox_params->menu_file != NULL) {

    but_menu = read_olwm_menu(bbox_params->menu_file);

    if (debug_level) {

      if ((but_menu == NULL) || (but_menu->bfirst == NULL))
	Fprintf(stderr, "exv_bbox: no valid menu from olwm menu\n");
      else
	Fprintf(stderr, "exv_bbox: read olwm menu file ok\n");
    }
  }

  if (debug_level > 2) {
    print_bbox_par(bbox_params);
    if (but_menu != NULL)
      print_olwm_menu(but_menu);
  }

  if (bbox_params->owner != NULL) {

      *bbox_panel = xv_create(bbox_params->owner, PANEL,
			      XV_WIDTH, 1100,
			      PANEL_LAYOUT,		PANEL_HORIZONTAL,
			      OPENWIN_SHOW_BORDERS,	TRUE,
			      WIN_BORDER, TRUE,
			      XV_KEY_DATA, 
			       EXVK_BUT_DATA_PROC, bbox_params->but_data_proc,
			      0);

      *bbox_frame = bbox_params->owner;

    }
  else {

      /* no owner, so create frame, complete with icon, and button panel */

      *bbox_frame  = (Frame)xv_create(NULL, FRAME, 
				      XV_SHOW, FALSE,
				      XV_LABEL, bbox_params->title,
#if defined(DS3100) || defined(SG) || defined(hpux)
				      FRAME_FOREGROUND_COLOR, &ffg,
				      FRAME_BACKGROUND_COLOR, &fbg,
				      FRAME_INHERIT_COLORS,   FALSE,
#endif
				      FRAME_INHERIT_COLORS,   TRUE,
				      FRAME_SHOW_FOOTER, FALSE,
				      FRAME_SHOW_RESIZE_CORNER, TRUE,
				      0);

      *bbox_panel = xv_create(*bbox_frame, PANEL,
			      XV_X, 0,
			      XV_WIDTH, 1100,
			      PANEL_LAYOUT,		PANEL_HORIZONTAL,
			      OPENWIN_SHOW_BORDERS,	TRUE,
			      WIN_BORDER, TRUE,
			      XV_KEY_DATA, 
			       EXVK_BUT_DATA_PROC, bbox_params->but_data_proc,
			      0);

      (void) exv_attach_icon(*bbox_frame, ERL_NOBORD_ICON, 
			     bbox_params->icon_title, TRANSPARENT);

    }

  if (bbox_params->quit_button || getenv("BBOX_QUIT_BUTTON")) {

      button = xv_create(*bbox_panel, PANEL_BUTTON,
		       PANEL_LABEL_STRING, bbox_params->quit_label,
		       PANEL_CLIENT_DATA, *bbox_frame,
		       PANEL_NOTIFY_PROC, quit_proc,
		       PANEL_NEXT_ROW, -1,
                       0);
  }
  n_but = 0;

  if ((bbox_params->but_labels != NULL) && (bbox_params->but_data != NULL)) {

      while ((bbox_params->but_labels[n_but] != NULL)
	     && (bbox_params->but_data[n_but] !=NULL))  {

	  button = xv_create(*bbox_panel, PANEL_BUTTON,
			  PANEL_LABEL_STRING, bbox_params->but_labels[n_but],
		          PANEL_CLIENT_DATA,  bbox_params->but_data[n_but],
			  PANEL_NOTIFY_PROC,  but_selected,
			  0);

	  /* start next row if appropriate */

	  if (new_row(bbox_params))
	    xv_set(button, PANEL_NEXT_ROW, -1, 0);

	  n_but++;
	}
    }

  /* now fill in buttons derived from olwm menu file */

  but_menu_ok = 1;

  if (but_menu != NULL) {
      but_menu_ok = add_menu_file_buttons(bbox_panel, bbox_params, but_menu); 
    }

  if ((n_but == 0) && (!but_menu_ok)) {
      Fprintf(stderr, "exv_bbox: could not create any buttons.\n");
      *bbox_panel = NULL;
      return(bbox_params);
    }

  (void)window_fit(*bbox_panel);

  (void)window_fit(*bbox_frame);

/* separating the XV_X, XV_Y sets from the original xv_create seems
necessary to have some window managers pay attention to positioning */


  if (bbox_params->owner == NULL) {
      if ((bbox_params->x_pos >= 0) && (bbox_params->y_pos >= 0)) 
	  xv_set(*bbox_frame, 
		 XV_X,	bbox_params->x_pos,
		 XV_Y,	bbox_params->y_pos,
		 XV_SHOW, bbox_params->show,
		 0);
      else
	  xv_set(*bbox_frame, 
		 XV_SHOW, bbox_params->show,   
		 0);
    }
  else if ((bbox_params->x_pos >= 0) && (bbox_params->y_pos >= 0)) 
    xv_set(*bbox_frame, 
	   XV_X,	bbox_params->x_pos,
	   XV_Y,	bbox_params->y_pos,
	   0);

  return(bbox_params);
 }

static int
add_menu_file_buttons(panel, params, butmenu)
Panel *panel;
bbox_par *params;
menudata *butmenu;
{
/* This function adds to the panel buttons that are specified in 
 * the olwm menuy butmenu.  
 */
  Panel_item button;
  buttondata *butdata;
  int n_buttons = 0;

  butdata = butmenu->bfirst; 
  
  while (butdata != NULL) {
      if (butdata->submenu == NULL) {
	  if ((butdata->name != NULL) && (butdata->exec != NULL)) {
	
	      /* it's a plain button, so we just add it */

	    button = xv_create(*panel, PANEL_BUTTON,
			       PANEL_LABEL_STRING, butdata->name,
			       PANEL_CLIENT_DATA,  butdata->exec,
			       PANEL_NOTIFY_PROC,  but_selected,
			       0);	
	  }
	  else {
	    Fprintf(stderr, 
	      "exv_bbox: skipped button with (nil) name or exec function\n");
	    button = NULL;
	  }
	}
      else {  /* it's a submenu, so we create a menu button or panel choice*/
	if (params->button_choice) 
	  button = add_panel_list(params, panel, (menudata*) butdata->submenu);
	else
	  button = add_button_menu(panel, (menudata*) butdata->submenu);
	}
      if (button != NULL) {
	  n_buttons++;
	  if (new_row(params))
	    xv_set(button, PANEL_NEXT_ROW, -1, 0);
	}
      butdata = butdata->next;
    }
  if (n_buttons > 0) 
    return(1);
  else
    return(0);
}

static Panel_item
add_button_menu(bpanel, smenu)
Panel *bpanel;
menudata *smenu;
{
/* This function adds to the panel (bpanel) a menu button; i.e. 
 * a button which, when pressed, produces a pop-down menu.  The 
 * submenu used derives from an olwm menu file.  This function is 
 * called from add_menu_file_buttons in the case of a submenu. 
 */

  Panel_item menu_button = NULL;
  buttondata *subdata;
  menudata *submenu;
  Menu button_menu;
  Menu_item mi;
  int n_item = 0;

  button_menu = (Menu) xv_create(NULL, MENU, 0);

  subdata = smenu->bfirst;

  while(subdata != NULL) {
      if (subdata->submenu == NULL) {
	  if ((subdata->name != NULL) && (subdata->exec != NULL)) {
	      
	      mi = (Menu_item) xv_create(NULL, MENUITEM, 
					 MENU_STRING, subdata->name, 
					 MENU_NOTIFY_PROC, menu_item_sel,
					 MENU_CLIENT_DATA, subdata->exec,
					 0);
		      
	      xv_set(button_menu, MENU_APPEND_ITEM, mi, 0);
		      
	      n_item++;

	      if (subdata->isDefault) 
		xv_set(button_menu, MENU_DEFAULT, n_item, 0); 
	    }
	  else {
	      Fprintf(stderr, 
	        "exv_bbox: skipped button with (nil) name or exec function\n");
	    }
	}
      else { /* skip submenus beyond level 2 */
	  
	  submenu = (menudata *) subdata->submenu;

	  if (submenu->label) 
	    Fprintf(stderr, 
		    "exv_bbox: submenu \"%s\" beyond level 2, ignored.\n",
		    submenu->label);
	  else
	    Fprintf(stderr, "exv_bbox: submenu beyond level 2 ignored.\n");
	}
      subdata = subdata->next;
    }

  /* menu finished, create title button and attach menu */

  if (n_item) {
    menu_button = xv_create(*bpanel, PANEL_BUTTON, 0);

    xv_set(button_menu, MENU_CLIENT_DATA, menu_button, 0); 

    xv_set(menu_button, PANEL_LABEL_STRING, smenu->label,
	   PANEL_NOTIFY_PROC, menu_but_selected,
	   PANEL_ITEM_MENU, button_menu,
	   0);

  }
  return(menu_button);
}

static Panel_item
add_panel_list(params, bpanel, smenu)
bbox_par *params;
Panel *bpanel;
menudata *smenu;
{
/* This function adds to the panel (bpanel) a panel list; 
 * The set of string items used derives from an olwm menu file.  
 * This function is called from add_menu_file_buttons in the case of a 
 * submenu when panel lists are specified. 
 */

  Panel_item pchoice = NULL;
  char *exec_str[1024];  /* assume no more than 1024 menu items! */
  buttondata *subdata;
  menudata *submenu;
  char *label = (char *) malloc(strlen(smenu->label) + 2);
  int n_item, i;

  sprintf(label, "%s:", smenu->label);

  pchoice = xv_create(*bpanel, PANEL_CHOICE,
		      PANEL_LAYOUT, (params->choice_horizontal ? 
				      PANEL_HORIZONTAL : PANEL_VERTICAL),
		      PANEL_LABEL_STRING, label,
		      PANEL_NOTIFY_PROC, choice_item_sel, 
		      PANEL_CHOOSE_ONE, TRUE,
		      PANEL_CHOOSE_NONE, TRUE,
		      PANEL_VALUE, -1,
		      XV_KEY_DATA, LAST_VAL_KEY, -1, 
		      0);

  subdata = smenu->bfirst;

  n_item = 0;

  while(subdata != NULL) {
      if (subdata->submenu == NULL) {
	  if ((subdata->name != NULL) && (subdata->exec != NULL)) {
	      
	    xv_set(pchoice, PANEL_CHOICE_STRING, n_item, subdata->name, NULL);

	    exec_str[n_item] = subdata->exec;
	    
	    if (subdata->isDefault) 
	    {
	      xv_set(pchoice, PANEL_VALUE, n_item, 0); 	      
	      xv_set(pchoice, XV_KEY_DATA, LAST_VAL_KEY, n_item, 0);
	    }
	    
	    n_item++;
	    }

	  else {
	      Fprintf(stderr, 
	        "exv_bbox: skipped button with (nil) name or exec function\n");
	    }
	}
      else { /* skip submenus beyond level 2 */
	  
	  submenu = (menudata *) subdata->submenu;

	  if (submenu->label) 
	    Fprintf(stderr, 
		    "exv_bbox: submenu \"%s\" beyond level 2, ignored.\n",
		    submenu->label);
	  else
	    Fprintf(stderr, "exv_bbox: submenu beyond level 2 ignored.\n");
	}
      subdata = subdata->next;
    }

  /* Create list of exec commands and attach as client data. */

  if (n_item) {
    char **new_exec_str = (char **) malloc(n_item * sizeof(char *));

    for (i = 0; i < n_item; i++)
      new_exec_str[i] = savestring(exec_str[i]);
	    
    xv_set(pchoice, PANEL_CLIENT_DATA, new_exec_str, NULL);

  }
  return(pchoice);
}

static int
new_row(params)
bbox_par *params;
{
/* Returns 1 if a new row of buttons is being started, else 0;
 * This function makes use of the globals:
 *
 * tot_buttons:  cumulative button number
 * last_row:     when this changes, a new row has started
 */

  int j;			/* actual button number in this row */
  int cols;			/* number of desired buttons per row */

  cols = params->n_per_row;

  /*set button number (don't count optional quit button)*/
  
  j = tot_buttons;

  /* special case for first button (forces new row after optional quit but)*/

  if (j == 0) {
      tot_buttons++;
      return 1;
    }
  /* general case thereafter */

  if ((j/cols) == last_row) {
      tot_buttons++;
      return 0;
    }
  else {
      tot_buttons++;
      last_row++;
      return 1;
    }
}

static bbox_par
*bbox_defaults()
{
/* This function allocates space for exv_bbox a parameter structure
 * and fills it in with default values. 
 */
 
  bbox_par *def = (bbox_par*) malloc(sizeof(bbox_par));

  if (debug_level > 1) 
    Fprintf(stderr, "bbox_defaults: generating defaults for exv_bbox\n");

  def->quit_button = 0;
  def->quit_label = "QUIT";
  def->quit_data = NULL;
  def->quit_data_proc = exec_data;
  def->show = 1;
  def->n_per_row = 10;
  def->title = EXV_WINDOW_TITLE;
  def->icon_title = EXV_ICON_TITLE;
  def->but_data_proc = exec_data; 
  def->owner = (Frame) NULL;
  def->but_labels = def->but_data = (char **) NULL;
  def->menu_file = (char *) NULL;

  def->button_choice = 0;  /* use menu buttons */

  def->choice_horizontal = 0;  /* if choices, do vertical */
  
    /* default positioning leaves it up to window manager */

  def->x_pos = -1;
  def->y_pos = -1;
  return(def);
}

static int
check_par(but_def)
bbox_par *but_def;
{
/*
 * This function performs some rudimentary consistency checks of 
 * exv_bbox parameters.  
 */

  if (but_def->title == NULL) 
    but_def->title = EXV_WINDOW_TITLE;

  if (but_def->icon_title == NULL)
    but_def->icon_title = EXV_ICON_TITLE;

  if ((but_def->menu_file == NULL) &&
    ((but_def->but_labels == NULL) || (but_def->but_data == NULL)
    || (but_def->but_labels[0] == NULL) || (but_def->but_data[0] == NULL)))
    return(0);
  else if (but_def->but_data_proc == NULL) 
    return(0);
  else
    return(1);
}

#define P_PRINT(a,b) (void) fprintf(stderr,"a = b\n", but_def->a)
void
print_bbox_par(but_def)
bbox_par *but_def;
{
/* prints the contents of an exv_bbox parameter structure */

  int i;
  P_PRINT(show, %d);
  P_PRINT(n_per_row, %d);
  P_PRINT(quit_button, %d);
  P_PRINT(x_pos, %d);
  P_PRINT(y_pos, %d);
  P_PRINT(title, %s);

  i = 0;
  if ((but_def->but_labels != NULL) && (but_def->but_data != NULL)) {
      while ((but_def->but_labels[i] != NULL)
	     && (but_def->but_data[i] !=NULL))  {

	  Fprintf(stderr, "but_labels[%d] = %s\n", i, but_def->but_labels[i]);
	  Fprintf(stderr, "but_data[%d] = %s\n", i, but_def->but_data[i]);
	  i++;
	}
    }
  if (but_def->menu_file != NULL) 
    P_PRINT(menu_file, %s);
}

static void
quit_proc(item)
Panel_item item;
{
  Frame frame = (Frame)xv_get(item, PANEL_CLIENT_DATA);

  if (bbox_params->quit_data) 
    bbox_params->quit_data_proc(bbox_params->quit_data, item);

  xv_destroy_safe(frame);
}

static void
but_selected(item)
Panel_item item;
{
/* This function is called when any simple (non-menu) button is pressed.  
 * It retrieves the data string associated with the button and invokes 
 * the button execution function.  
 */
  char *panel_label, *panel_data;
  void (*button_data_proc)();

  panel_label = (char *) xv_get(item, PANEL_LABEL_STRING);
  panel_data = (char *) xv_get(item, PANEL_CLIENT_DATA);

  if (debug_level > 1) {
    Fprintf(stderr, "exv_bbox: panel button %s selected\n", panel_label);
    Fprintf(stderr, "\tdata string is: %s\n", panel_data);
  }
    
  button_data_proc = (void (*)()) xv_get(xv_get(item, XV_OWNER), 
			      XV_KEY_DATA, EXVK_BUT_DATA_PROC);

  button_data_proc(panel_data, item);
}

static int
menu_but_selected(item, event)
Panel_item item;
{
/* This is called when a menu button is pressed.  It doesn't really
 * do anything.
 */
  if (debug_level > 1) 
    Fprintf(stderr, "Menu button %s selected\n", 
	    (char *) xv_get(item, PANEL_LABEL_STRING));
  return XV_OK;
}

static void
menu_item_sel(menu, menu_item)
Menu menu;
Menu_item menu_item;
{
/* This function is called when the button is released over a menu
 * item.  The data string is attached to the menu item via MENU_CLIENT_DATA.
 * A handle to the button that was pushed to get the menu is obtained 
 * via MENU_CLIENT_DATA from the menu.  These two items are then passed
 * to the data procedure (passed by user, or default is exec)
 */

  char *menu_label, *menu_data;
  Panel_item button;
  void (*button_data_proc)();

  menu_label = (char *) xv_get(menu_item, MENU_STRING);
  menu_data = (char *) xv_get(menu_item, MENU_CLIENT_DATA);
  button = (Panel_item) xv_get(menu, MENU_CLIENT_DATA);

  if (debug_level > 1)  {
    Fprintf(stderr, "exv_bbox: Menu item %s selected,\n", menu_label);
    Fprintf(stderr, "\tdata string is: %s\n", menu_data);
  }

  button_data_proc = (void (*)()) xv_get(xv_get(button, XV_OWNER), 
			      XV_KEY_DATA, EXVK_BUT_DATA_PROC);

  button_data_proc(menu_data, button);

}

static void 
choice_item_sel(item, value, event)
Panel_item	item;
int		value;
Event		*event;
{
  void (*button_data_proc)();
  int last_value;
  char **exec_str = (char **) xv_get(item, PANEL_CLIENT_DATA);
  char *name = (char *) xv_get(item, PANEL_LABEL_STRING);

  if (value == -1) 
  {
    if (debug_level > 1)
      Fprintf(stderr, "exv_bbox: same choice selected again.\n");
    
    last_value = xv_get(item, XV_KEY_DATA, LAST_VAL_KEY);

    xv_set(item, PANEL_VALUE, last_value, NULL);

    value = last_value;
  }

  xv_set(item, XV_KEY_DATA, LAST_VAL_KEY, value, NULL); 

  if (debug_level > 1)  {
    Fprintf(stderr, "exv_bbox: Choice item %s selected, value %d,\n", 
	    name, value);

    Fprintf(stderr, "\tdata string is: %s\n", exec_str[value]);
  }

  button_data_proc = (void (*)()) xv_get(xv_get(item, XV_OWNER), 
			      XV_KEY_DATA, EXVK_BUT_DATA_PROC);

  button_data_proc(exec_str[value], item);

}

static void
exec_data(data_string, button)
char *data_string;
Panel_item button;
{
/* This is the default button execution function - it just execs the 
 * data string associated with the button (or menu item).
 */
  if (debug_level)
    Fprintf(stderr, 
    "exv_bbox: using default but_data_proc; will exec this command:\n %s\n", 
	    data_string);

  exec_command(data_string);
}


