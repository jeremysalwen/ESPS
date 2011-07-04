/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains proprietary software of Entropic Research
 * Laboratory, Inc.  Any reproduction, distribution, or publication
 * without the prior written permission of Entropic Research
 * Laboratory, Inc. is strictly prohibited.  Any public distribution
 * of copies of this work authorized in writing by Entropic Research
 * Laboratory, Inc. must bear the notice
 *
 *    "Copyright (c) 1993 Entropic Research Laboratory, Inc.
 *     All rights reserved."
 *
 * Program: xaddop.c
 *
 * Written by:  John Shore
 * Checked by:
 *
 * GUI for waves+ add_op command
 */

static char *sccs_id = "@(#)xaddop.c	1.8 9/28/98 ERL/ATT";

/*
 * system include files
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/textsw.h>
#include <xview/xv_xrect.h>
#include <xview/notice.h>
#include <xview/cms.h>

/*
 * other include files
 */

#include <esps/esps.h> 
#include <esps/exview.h>
#if !defined(LINUX_OR_MAC)
#include <malloc/malloc.h>
#endif

#include <xaddop.h>

#ifndef NULL
#define NULL 0
#endif

#define EXV_WINDOW_TITLE "waves+ add_op interface"

#define EXV_ICON_TITLE "add_op"

extern int debug_level;
extern int do_color; 

static  addop_par *addop_defaults();
static void print_addop_par();

static void create_addop_win();
static void create_vars_win();
static void create_ops_win();

static void revise_op_attr();

static void show_ops();
static void show_vars();
static void add_op_help();
static void apply_addop();

static void insert_var();
static void insert_attr();
static void insert_op();
static void insert_gOut();
static void insert_tOut();
static void insert_nOut();

char *savestring();

/*static void quit_proc();*/
static void dismiss_proc();

static void select_object_name();

/* 
 * global variables
 */

static Frame addop_win = XV_NULL;

static Frame vars_win = XV_NULL;
static Frame ops_win = XV_NULL;
static Textsw textwin = XV_NULL;

static Panel_item attrlist; /* scrolling list of attributes */

static Panel_item opName;
static Panel_item windows;
static Panel_item objNameStr;
static Panel_item objNameType;
static Panel_item gOutExt;
static Panel_item tOutExt;
static Panel_item nOutExt;

static char *helpfile;

static addop_par *Gparams;

static int
check_par(params)
addop_par *params;
{
/*
 * This function performs some rudimentary consistency checks of 
 * xaddop parameters.  
 */
  int ok = 1;

  if (params->apply_add_op == NULL) 
  {
    Fprintf(stderr, "xaddop: called with NULL apply_add_op function.\n");
    ok = 0;
  }
  return(ok);
}


addop_par *
xaddop(params)
addop_par *params;
{
  if (debug_level)
      Fprintf(stderr, "entered xaddop\n");

  /* If we're being called again and there's an addop_window already, 
     kill it. */
  
/*
  if (addop_win != NULL && textwin != NULL) 
  {
    textsw_reset(textwin, 0,0);
    dt_xv_destroy_safe(3,addop_win);
    textwin = addop_win = NULL;
  }
*/
  
  /* generate default parameters if asked */

  if (params == NULL) {
      params = addop_defaults();
      return(params);
    }

  /* initialization */

/*  addop_win = vars_win = ops_win = NULL; */

  if (!check_par(params)) 
      return(NULL);

  if (debug_level > 1) {
    print_addop_par(params);
  }

  /* Set some globals */

  helpfile = params->helpfile;

  Gparams = params;

/* To avoid the SGI bug that yields core dumps when we 
   destroy the windows and invoke xaddop again, we only
   create them once.  To do this right, the create_vars
   and create_ops functions should regenerate the 
   scrolling lists, which might change from call to call. */
 
   if (addop_win == XV_NULL) 
   {
     create_addop_win(params);
     create_vars_win(params);
     create_ops_win(params);  
   }

/*
  create_addop_win(params);
  
  create_vars_win(params);

  create_ops_win(params);
*/
  if ((params->addop_x_pos >= 0) && (params->addop_y_pos >= 0)) 
    xv_set(addop_win, 
            XV_X, params->addop_x_pos,
            XV_Y, params->addop_y_pos,
            NULL);

  xv_set(addop_win, XV_SHOW, TRUE, NULL);

  if (params->show_vars_win)
    show_vars(NULL, NULL);
  
  if (params->show_ops_win)
    show_ops(NULL, NULL);

  return(params);
}

static addop_par
*addop_defaults()
{
/* This function allocates space for xaddop's parameter structure
 * and fills it in with default values. 
 */
 
  addop_par *def = (addop_par*) malloc(sizeof(addop_par));

  if (debug_level > 1) 
    Fprintf(stderr, "Generating defaults for xaddop().\n");

  def->view_vars = def->operations = (char **) NULL;
  def->global_vars = def->operations = (char **) NULL;
  def->apply_add_op = NULL;
  def->get_attr_list = NULL;
  def->helpfile = NULL;
  
  /* by default, vars and ops popups are not visible */

  def->show_vars_win = 0;
  def->show_ops_win = 0;
 
  def->addop_x_pos = -1;
  def->addop_y_pos = -1;

  /* default positioning puts them under the addop_win */

  def->vars_x_pos = -1;
  def->vars_y_pos = -1;

  def->ops_x_pos = -1;
  def->ops_y_pos = -1;

  return(def);
}

static void
print_addop_par(params)
addop_par *params;
{
/* prints the contents of an xaddop_par parameter structure */

  int i;

#define P_PRINT(a,b) (void) fprintf(stderr,"a = b\n", params->a)


  P_PRINT(addop_x_pos, %d);
  P_PRINT(addop_y_pos, %d);

  P_PRINT(show_vars_win, %d);

  P_PRINT(vars_x_pos, %d);
  P_PRINT(vars_y_pos, %d);

  P_PRINT(show_ops_win, %d);

  P_PRINT(ops_x_pos, %d);
  P_PRINT(ops_y_pos, %d);


  if (params->global_vars == NULL) 
  {
    Fprintf(stderr, "Warning - NULL list of global variables.\n");
  }    
  else
  {
    i = 0;
    while (params->global_vars[i] != NULL) 
    {
      Fprintf(stderr, "global_vars[%d] = %s\n", i, params->global_vars[i]);
      i++;
    }
  }


  if (params->view_vars == NULL) 
  {
    Fprintf(stderr, "Warning - NULL list of view view variables.\n");
  }    
  else
  {
    i = 0;
    while (params->view_vars[i] != NULL) 
    {
      Fprintf(stderr, "view_vars[%d] = %s\n", i, params->view_vars[i]);
      i++;
    }
  }

  if (params->operations == NULL) 
  {
    Fprintf(stderr, "Warning - NULL list of operations.\n");
  }    
  else 
  {
    i = 0;
    while (params->operations[i] != NULL) 
    {
      Fprintf(stderr, "operations[%d] = %s\n", i, params->operations[i]);
      i++;
    }
  }
  if (params->helpfile == NULL)
    Fprintf(stderr, "NULL helpfile.\n");
  else
    P_PRINT(helpfile, %s);
}

static void
create_addop_win(params)
addop_par *params;
{
  Panel lpanel, rpanel;

  /* Create frame. */

  addop_win =  xv_create(params->owner, FRAME, NULL);

/*  addop_win =  xv_create(params->owner, FRAME_CMD, NULL);*/
  
  xv_set(addop_win, 
         XV_LABEL, savestring("add_op interface"),
         XV_WIDTH, 882,
         XV_HEIGHT, 154,
         XV_SHOW, FALSE,
         FRAME_SHOW_FOOTER, FALSE,
         FRAME_SHOW_RESIZE_CORNER, TRUE,
         NULL);
    
  /* Create left control panel. */


  lpanel = (Panel) xv_create(addop_win, PANEL,
                             XV_X, 0,
                             XV_Y, 0,
                             XV_WIDTH, 730,
                             XV_HEIGHT, 109,
                             WIN_BORDER, FALSE,
                             NULL); 

  /* Add buttons and text to left panel. */


  opName = xv_create(lpanel, PANEL_TEXT,
                     XV_X, 32,
                     XV_Y, 16,
                     PANEL_VALUE_DISPLAY_LENGTH, 40,
                     PANEL_VALUE_STORED_LENGTH, 80,
                     PANEL_LABEL_STRING, savestring("operation name:"),
                     PANEL_LAYOUT, PANEL_HORIZONTAL,
                     PANEL_READ_ONLY, FALSE,
                     NULL);

  windows =  xv_create(lpanel, PANEL_TOGGLE,
                       XV_X, 32,
                       XV_Y, 47,
                       PANEL_CHOICE_NROWS, 1,
                       PANEL_LAYOUT, PANEL_HORIZONTAL,
                       PANEL_LABEL_STRING, savestring("add to windows: "),
                       PANEL_CHOICE_STRINGS,
                         "waveform",
                         "image",
                          NULL,
                       PANEL_VALUE, 1,
                       NULL);

  (void) xv_create(lpanel, PANEL_MESSAGE,

                XV_X, 32,
                XV_Y, 88,
                PANEL_LABEL_STRING, 
                   savestring("Enter the command definition below:"),
                PANEL_LABEL_BOLD, TRUE,
                NULL);

  (void) xv_create(lpanel, PANEL_BUTTON,
                XV_X, 509,
                XV_Y, 24,
                PANEL_LABEL_STRING, 
                   savestring("show variables for insertion..."),
                PANEL_NOTIFY_PROC, show_vars,
                NULL);

  (void) xv_create(lpanel, PANEL_BUTTON,
                XV_X, 504,
                XV_Y, 63,
                PANEL_LABEL_STRING, 
                   savestring("show commands for insertion..."),
                PANEL_NOTIFY_PROC, show_ops,
                NULL);


  /* Create right panel. */

  rpanel = (Panel) xv_create(addop_win, PANEL,
                             XV_X, (int)xv_get(lpanel, XV_X) +
                                   (int)xv_get(lpanel, XV_WIDTH),
                             XV_Y, 0,
                             XV_WIDTH, WIN_EXTEND_TO_EDGE,
                             XV_HEIGHT, 109,
                             WIN_BORDER, TRUE,
                             NULL);

  /* Add buttons to right panel. */

  (void) xv_create(rpanel, PANEL_BUTTON,
                XV_X, 23,
                XV_Y, 47,
                PANEL_LABEL_STRING, savestring("Apply add_op"),
                PANEL_NOTIFY_PROC, apply_addop,
                NULL);

  (void) xv_create(rpanel, PANEL_BUTTON,
                   XV_X, 51,
                   XV_Y, 78,
                   PANEL_LABEL_STRING, savestring("Help"),
                   PANEL_NOTIFY_PROC, add_op_help,
                   NULL);

  (void) xv_create(rpanel, PANEL_BUTTON,
/*              XV_X, 11,  */ /* (for "quit (don't apply) */
                XV_X, 43,
                XV_Y, 16,
/*              PANEL_LABEL_STRING, savestring("Quit (don't apply)"),
                PANEL_NOTIFY_PROC, quit_proc,*/
                PANEL_LABEL_STRING, savestring("Dismiss"),
                PANEL_NOTIFY_PROC, dismiss_proc,
                NULL);

  /* Create text window. */

#ifdef SG
  putenv("EXTRASMENU=/dev/null");    /* Fix SGI textsw bug, ugh. */
#endif

  textwin = xv_create(addop_win, TEXTSW,
                XV_X, 0,
                XV_Y, (int)xv_get(lpanel, XV_Y) +
                      (int)xv_get(lpanel, XV_HEIGHT),
                XV_WIDTH, WIN_EXTEND_TO_EDGE,
                XV_HEIGHT, WIN_EXTEND_TO_EDGE,
                OPENWIN_SHOW_BORDERS, TRUE,
                NULL);

  if ((params->addop_x_pos >= 0) && (params->addop_y_pos >= 0)) 
    xv_set(addop_win, 
           XV_X, params->addop_x_pos,
           XV_Y, params->addop_y_pos,
           NULL);
  else 
    xv_set(addop_win, 
           XV_X, 0,
           XV_Y, 300,  /*place so vars and ops can default above and below */
           NULL);

  exv_attach_icon(addop_win, ERL_NOBORD_ICON, "add_op", TRANSPARENT);

  xv_set(addop_win, XV_SHOW, TRUE, NULL);
}

static void
create_ops_win(params)
addop_par *params;
{
  Panel panel;
  Panel_item oplist;
  int i;

  ops_win = (Frame) xv_create(addop_win, FRAME_CMD,
                              XV_WIDTH, 626,
                              XV_HEIGHT, 194,
                              XV_LABEL, "command inserts for add_op",
                              XV_SHOW, FALSE,
                              FRAME_SHOW_FOOTER, FALSE,
                              FRAME_CMD_PUSHPIN_IN, TRUE,
                              NULL);

/*  xv_set(xv_get(ops_win, FRAME_CMD_PANEL), WIN_SHOW, FALSE, NULL);*/


  panel = (Panel) xv_create(ops_win, PANEL,
                            XV_X, 0,
                            XV_Y, 0,
                            XV_WIDTH, WIN_EXTEND_TO_EDGE,
                            XV_HEIGHT, WIN_EXTEND_TO_EDGE,
                            WIN_BORDER, TRUE,
                            NULL);

  objNameType =  xv_create(panel, PANEL_CHOICE,
                           XV_X, 48,
                           XV_Y, 56,
                           PANEL_CHOICE_NROWS, 3,
                           PANEL_LAYOUT, PANEL_HORIZONTAL,
                           PANEL_CHOOSE_NONE, FALSE,
                           PANEL_LABEL_STRING, savestring("Object:"),
                           PANEL_NOTIFY_PROC, select_object_name,
                           PANEL_CHOICE_STRINGS,
                               savestring("none"),
                               savestring("current object"),
                               savestring("arbitrary"),
                               NULL,
                           NULL);
  
  objNameStr = xv_create(panel, PANEL_TEXT,
                XV_X, 24,
                XV_Y, 136,
                PANEL_VALUE_DISPLAY_LENGTH, 13,
                PANEL_VALUE_STORED_LENGTH, 80,
                PANEL_LABEL_STRING, savestring("Obj. name:"),
                PANEL_LAYOUT, PANEL_HORIZONTAL,
                PANEL_INACTIVE, TRUE,
                PANEL_READ_ONLY, FALSE,
                NULL);

  oplist = xv_create(panel, PANEL_LIST,
                     XV_X, 224,
                     XV_Y, 16,
                     PANEL_LIST_WIDTH, 175,
                     PANEL_LIST_DISPLAY_ROWS, 7,
                     PANEL_LABEL_STRING, savestring("Commands:"),
                     PANEL_LAYOUT, PANEL_VERTICAL,
                     PANEL_READ_ONLY, TRUE,
                     PANEL_CHOOSE_ONE, TRUE,
                     PANEL_CHOOSE_NONE, TRUE,
                     PANEL_NOTIFY_PROC, insert_op,
                     NULL);

  for (i=0; params->operations[i] != NULL; i++) 
  {
    xv_set(oplist, 
           PANEL_LIST_STRING, i, savestring(params->operations[i]),
           NULL);
  }

  attrlist = xv_create(panel, PANEL_LIST,
                      XV_X, 432,
                      XV_Y, 16,
                      PANEL_LIST_WIDTH, 150,
                      PANEL_LIST_DISPLAY_ROWS, 7,
                      PANEL_LABEL_STRING, "Command attributes:",
                      PANEL_LAYOUT, PANEL_VERTICAL,
                      PANEL_READ_ONLY, FALSE,
                      PANEL_CHOOSE_ONE, TRUE,
                      PANEL_CHOOSE_NONE, TRUE,
                      PANEL_NOTIFY_PROC, insert_attr,
                      NULL);

}

static void
create_vars_win(params)
addop_par *params;
{
  Panel lpanel, rpanel;
  Panel_item varlist;
  int i;

  vars_win = (Frame) xv_create(addop_win, FRAME_CMD,
                       XV_WIDTH, 786,
                       XV_HEIGHT, 184,
                       XV_LABEL, "variable inserts for add_op",
                       XV_SHOW, FALSE,
                       FRAME_SHOW_FOOTER, FALSE,
                       FRAME_CMD_PUSHPIN_IN, TRUE, 
                       NULL);

/*  xv_set(xv_get(vars_win, FRAME_CMD_PANEL), WIN_SHOW, FALSE, NULL);*/

  lpanel = (Panel) xv_create(vars_win, PANEL,
                             XV_X, 0,
                             XV_Y, 0,
                             XV_WIDTH, 431,
                             XV_HEIGHT, WIN_EXTEND_TO_EDGE,
                             WIN_BORDER, TRUE,
                             NULL);

  varlist = xv_create(lpanel, PANEL_LIST,
                      XV_X, 16,
                      XV_Y, 8,
                      PANEL_LIST_WIDTH, 175,
                      PANEL_LIST_DISPLAY_ROWS, 7,
                      PANEL_LABEL_STRING, savestring("Global Variables:"),
                      PANEL_LAYOUT, PANEL_VERTICAL,
                      PANEL_READ_ONLY, TRUE,
                      PANEL_CHOOSE_ONE, TRUE,
                      PANEL_CHOOSE_NONE, TRUE,
                      PANEL_NOTIFY_PROC, insert_var,
                      NULL);

  for (i=0; params->global_vars[i] != NULL; i++) 
  {
    xv_set(varlist, 
           PANEL_LIST_STRING, i, savestring(params->global_vars[i]),
           NULL);
  }

  varlist = xv_create(lpanel, PANEL_LIST,
                      XV_X, 224,
                      XV_Y, 8,
                      PANEL_LIST_WIDTH, 175,
                      PANEL_LIST_DISPLAY_ROWS, 7,
                      PANEL_LABEL_STRING, savestring("View-Specific Variables:")
,
                      PANEL_LAYOUT, PANEL_VERTICAL,
                      PANEL_READ_ONLY, TRUE,
                      PANEL_CHOOSE_ONE, TRUE,
                      PANEL_CHOOSE_NONE, TRUE,
                      PANEL_NOTIFY_PROC, insert_var,
                      NULL);

  for (i=0; params->view_vars[i] != NULL; i++) 
  {
    xv_set(varlist, 
           PANEL_LIST_STRING, i, savestring(params->view_vars[i]),
           NULL);
  }

  rpanel = (Panel) xv_create(vars_win, PANEL,
                             XV_X, (int)xv_get(lpanel, XV_X) +
                                   (int)xv_get(lpanel, XV_WIDTH) + 2,
                             XV_Y, 0,
                             XV_WIDTH, WIN_EXTEND_TO_EDGE,
                             XV_HEIGHT, WIN_EXTEND_TO_EDGE,
                             WIN_BORDER, TRUE,
                             NULL);

  (void) xv_create(rpanel, PANEL_MESSAGE,
                   XV_X, 24,
                  XV_Y, 24,
                  PANEL_LABEL_STRING, savestring("Output Files:"),
                  PANEL_LABEL_BOLD, TRUE,
                  NULL);

  (void) xv_create(rpanel, PANEL_BUTTON,
                   XV_X, 24,
                   XV_Y, 54,
                   PANEL_LABEL_STRING, 
                         savestring("Output File (display graphics)"),
                   PANEL_NOTIFY_PROC, insert_gOut,
                   NULL);

  gOutExt = xv_create(rpanel, PANEL_TEXT,
                      XV_X, 237,
                      XV_Y, 58,
                      PANEL_VALUE_DISPLAY_LENGTH, 8,
                      PANEL_VALUE_STORED_LENGTH, 80,
                      PANEL_LABEL_STRING, savestring(".ext:"),
                      PANEL_VALUE, "out",
                      PANEL_LAYOUT, PANEL_HORIZONTAL,
                      PANEL_READ_ONLY, FALSE,
                      NULL);


  (void) xv_create(rpanel, PANEL_BUTTON,
                   XV_X, 24,
                   XV_Y, 93,
                   PANEL_LABEL_STRING, savestring("Output File (display text)"),
                   PANEL_NOTIFY_PROC, insert_tOut, 
                   NULL);


  tOutExt = xv_create(rpanel, PANEL_TEXT,
                      XV_X, 237,
                      XV_Y, 97,
                      PANEL_VALUE_DISPLAY_LENGTH, 8,
                      PANEL_VALUE_STORED_LENGTH, 80,
                      PANEL_LABEL_STRING, savestring(".ext:"),
                      PANEL_VALUE, "out",
                      PANEL_LAYOUT, PANEL_HORIZONTAL,
                      PANEL_READ_ONLY, FALSE,
                      NULL);
  
  (void) xv_create(rpanel, PANEL_BUTTON,
                   XV_X, 24,
                   XV_Y, 132,
                   PANEL_LABEL_STRING, 
                   savestring("Output File (don't display)"),
                   PANEL_NOTIFY_PROC, insert_nOut, 
                   NULL);


  nOutExt = xv_create(rpanel, PANEL_TEXT,
                      XV_X, 237,
                      XV_Y, 136,
                      PANEL_VALUE_DISPLAY_LENGTH, 8,
                      PANEL_VALUE_STORED_LENGTH, 80,
                      PANEL_LABEL_STRING, savestring(".ext:"),
                      PANEL_VALUE, "out",
                      PANEL_LAYOUT, PANEL_HORIZONTAL,
                      PANEL_READ_ONLY, FALSE,
                      NULL);
}

/*quit_proc(item)*/
static void
dismiss_proc(item)
Panel_item item;
{
  /* erase text to avoid popup asking about discarding edits */

  textsw_reset(textwin, 0,0);

  xv_set(addop_win, XV_SHOW, FALSE, NULL);

/*  dt_xv_destroy_safe(4,addop_win);*/

  xv_set(vars_win, FRAME_CMD_PUSHPIN_IN, FALSE, NULL);

  xv_set(vars_win, XV_SHOW, FALSE, NULL);
 
  xv_set(ops_win, FRAME_CMD_PUSHPIN_IN, FALSE, NULL);
 
  xv_set(ops_win, XV_SHOW, FALSE, NULL);

/*  addop_win = NULL;
  vars_win = ops_win = NULL;*/
}

static void 
show_ops(item, event)
Panel_item      item;
Event           *event;
{
  int ax, ay;

  int oh = xv_get(ops_win, XV_HEIGHT);

  if (xv_get(ops_win, XV_SHOW) == TRUE) 
    return;

  if (debug_level)
    Fprintf(stderr, "Pop up operations panel.\n");

  if ((Gparams->ops_x_pos >= 0) && (Gparams->ops_y_pos >= 0)) 
  {    
    xv_set(ops_win, 
           XV_X, Gparams->ops_x_pos,
           XV_Y, Gparams->ops_y_pos,
           NULL);
  }
  else 
  {
    int oy;

    if (Gparams->owner == XV_NULL)  
    {
      ax = xv_get(addop_win, XV_X);
      ay = xv_get(addop_win, XV_Y);
    }
    else 
    {
      ax = xv_get(xv_get(addop_win, XV_OWNER), XV_X)
        + xv_get(addop_win, XV_X);
            
      ay = xv_get(xv_get(addop_win, XV_OWNER), XV_Y)
        + xv_get(addop_win, XV_Y);
    }
    
    oy = ay - oh - 50;  /* subtract two window title bars */

    if (debug_level > 1) 
      Fprintf(stderr, "ax = %d, ay = %d, oh = %d, oy = %d\n", 
              ax, ay, oh, oy);

    if (oy > 0) 
      xv_set(ops_win,
             XV_X, ax,
             XV_Y, oy,
             NULL);
    else
      xv_set(ops_win,
             XV_X, ax,
             NULL);      

  }

  xv_set(ops_win, XV_SHOW, TRUE, NULL);
}


static void 
show_vars(item, event)
Panel_item      item;
Event           *event;
{
  int ax, ay;
  
  int ah = xv_get(addop_win, XV_HEIGHT);

  if (xv_get(vars_win, XV_SHOW) == TRUE) 
    return;

  if (debug_level)
    Fprintf(stderr, "Pop up variables panel.\n");
  
  if ((Gparams->vars_x_pos >= 0) && (Gparams->vars_y_pos >= 0)) 
  {
    xv_set(vars_win, 
           XV_X, Gparams->vars_x_pos,
           XV_Y, Gparams->vars_y_pos,
           NULL);
  }
  else
  {
    if (Gparams->owner == XV_NULL)  
    {
      ax = xv_get(addop_win, XV_X);
      ay = xv_get(addop_win, XV_Y);
    }
    else 
    {
      ax = xv_get(xv_get(addop_win, XV_OWNER), XV_X)
        + xv_get(addop_win, XV_X);
            
      ay = xv_get(xv_get(addop_win, XV_OWNER), XV_Y)
        + xv_get(addop_win, XV_Y);
    }
    
    if (debug_level > 1) 
      Fprintf(stderr, "ax = %d, ay = %d\n", ax, ay);

    xv_set(vars_win,
           XV_X, ax,
           XV_Y, ay + ah,
           NULL);
  }

  xv_set(vars_win, XV_SHOW, TRUE, NULL);
}

static void 
insert_var(item, string, client_data, op, event, row)
Panel_item      item;
char            *string;
Xv_opaque       client_data;
Panel_list_op   op;
Event           *event;
int             row;
{
  char *insert;

  if (op != 1) /* only operate on select */
    return;

  insert = (char *)malloc(strlen(string) + 5);
  
  sprintf(insert, "_%s ", string);

  if (debug_level)
    Fprintf(stderr, "Inserting var string: %s\n", insert);

  textsw_insert(textwin, insert, strlen(string) + 2);
}


static void 
insert_op(item, string, client_data, op, event, row)
Panel_item      item;
char            *string;
Xv_opaque       client_data;
Panel_list_op   op;
Event           *event;
int             row;
{
  char *insert;

  int obtype = xv_get(objNameType, PANEL_VALUE);

  if (op != 1) /* only operate on select */
    return;

  if (obtype == 0) 
  {
    insert = (char *) malloc(strlen(string) + 10);
    
    sprintf(insert, "#%s ", string);
  }
  else if (obtype == 1)
  {
    insert = (char *) malloc(strlen("_name") + strlen(string) + 10);
    
    sprintf(insert, "#_name %s ", string);
  }
  else 
  {
    char *objname = (char *) xv_get(objNameStr, PANEL_VALUE);

    insert = (char *) malloc(strlen(objname) + strlen(string) + 10);
    
    sprintf(insert, "#%s %s ", objname, string);
  }

  if (debug_level)
    Fprintf(stderr, "Inserting %s\n", insert);

  textsw_insert(textwin, insert, strlen(insert));
  free(insert);

  /* rewrite the op attributes */


  revise_op_attr(string);

}

static void 
revise_op_attr(operation) 
char *operation;
{
  static int last_attr = 0; /* row number of last attribute */
  int i;
  char **new_attr = NULL;
  
  xv_set(attrlist, XV_SHOW, FALSE, NULL);

  for (i = last_attr; i > 0; i--) 
  {
    xv_set(attrlist, PANEL_LIST_DELETE, i, NULL);
  }
  
  if (Gparams->get_attr_list != NULL) 
  {
    if((new_attr = (Gparams->get_attr_list)(operation))) {
      
      for (last_attr=0; new_attr[last_attr] != NULL; last_attr++) 
	{
	  xv_set(attrlist, 
		 PANEL_LIST_STRING, last_attr, savestring(new_attr[last_attr]),
		 NULL);
	}
    }
    xv_set(attrlist, XV_SHOW, TRUE, NULL);
    free_string_list(new_attr);
  }
}

static void 
insert_attr(item, string, client_data, op, event, row)
Panel_item      item;
char            *string;
Xv_opaque       client_data;
Panel_list_op   op;
Event           *event;
int             row;
{
  char *insert;

  if (op != 1) /* only operate on select */
    return;

  insert = (char *)malloc(strlen(string) + 5);
  
  sprintf(insert, "%s ", string);

  if (debug_level)
    Fprintf(stderr, "Inserting attribute string: %s\n", insert);

  textsw_insert(textwin, insert, strlen(string) + 1);
}

static void 
insert_gOut(item, event)
Panel_item      item;
Event           *event;
{
  char *base = "out.g.";
  char *ext = (char *) xv_get(gOutExt, PANEL_VALUE);
  char *insert = (char *) malloc(strlen(base) + strlen(ext) + 10);

  sprintf(insert, "_%s%s ", base, ext);

  if (debug_level)
    Fprintf(stderr, "Inserting %s\n", insert);
  
  textsw_insert(textwin, insert, strlen(insert));
}

static void 
insert_tOut(item, event)
Panel_item      item;
Event           *event;
{
  char *base = "out.t.";
  char *ext = (char *) xv_get(tOutExt, PANEL_VALUE);
  char *insert = (char *) malloc(strlen(base) + strlen(ext) + 10);

  sprintf(insert, "_%s%s ", base, ext);

  if (debug_level)
    Fprintf(stderr, "Inserting %s\n", insert);
  
  textsw_insert(textwin, insert, strlen(insert));
}

static void 
insert_nOut(item, event)
Panel_item      item;
Event           *event;
{
  char *base = "out.n.";
  char *ext = (char *) xv_get(gOutExt, PANEL_VALUE);
  char *insert = (char *) malloc(strlen(base) + strlen(ext) + 10);

  sprintf(insert, "_%s%s ", base, ext);

  if (debug_level)
    Fprintf(stderr, "Inserting %s\n", insert);
  
  textsw_insert(textwin, insert, strlen(insert));
}


static void 
apply_addop(item, event)
Panel_item      item;
Event           *event;
{
  char *menu;

  int menu_type = xv_get(windows, PANEL_VALUE);

  int ncom_char = xv_get(textwin, TEXTSW_LENGTH);

  char *command = (char *)malloc(ncom_char + 10);

  char *op = (char *) xv_get(opName, PANEL_VALUE);

  char *full_command;

  if (ncom_char == 0) 
  {
    (void) notice_prompt(addop_win, (Event *) NULL,
			 NOTICE_BLOCK_THREAD, FALSE,
                         NOTICE_MESSAGE_STRINGS, 
                         savestring("The command window has nothing in it!"),
                         NULL,
/*
                         NOTICE_FOCUS_XY, xv_get(addop_win, XV_X),
                                          xv_get(addop_win, XV_Y),
*/
                         NOTICE_BUTTON_YES,      "Continue",
                          NULL);
    return;
  }
  
  if (strlen(op) == 0) 
  {
    (void) notice_prompt(addop_win, (Event *) NULL,
			 NOTICE_BLOCK_THREAD, FALSE,
                         NOTICE_MESSAGE_STRINGS, 
                         savestring("There's no operation name!"),
                         NULL,
/*
                         NOTICE_FOCUS_XY, xv_get(addop_win, XV_X),
                                          xv_get(addop_win, XV_Y),
*/
                         NOTICE_BUTTON_YES,      "Continue",
                         NULL);
    return;
  }

  if(debug_level)
    Fprintf(stderr, "menu_type = %d\n", menu_type);

  switch(menu_type) 
  {
  case 0: /* none (not on a menu) */   
    menu = "none"; 
    break;
  case 1: /* waveform only */
    menu = "wave";
    break;
  case 2: /* image only */
    menu = "spect";
    break;
  case 3:
    menu = "all";
    break;
  }
  
  if ((Textsw_index) xv_get(textwin, TEXTSW_CONTENTS, 0, command, ncom_char) 
      != ncom_char)
  {
    Fprintf(stderr, "Error getting textsw contents.\n");
  }
  else 
    command[ncom_char] = '\0';
  
  full_command = (char *) malloc(ncom_char + strlen(op) + 100);
  
  if (strchr(op, ' ') == NULL) 
    sprintf(full_command, "%add_op name %s menu %s command %s\n", 
            op, menu, command);
  else
    sprintf(full_command, "%add_op name \"%s\" menu %s command %s\n", 
            op, menu, command);

  if (debug_level) 
    Fprintf(stderr, "Applying the command:\n%s\n", full_command);

  if (Gparams->apply_add_op) 
  {
    if (Gparams->apply_add_op(full_command) != 1)

      (void) notice_prompt(addop_win, (Event *) NULL,
			   NOTICE_BLOCK_THREAD, FALSE,
                           NOTICE_MESSAGE_STRINGS, 
                           savestring("add_op failed."),
                           NULL,
/*
                           NOTICE_FOCUS_XY, xv_get(addop_win, XV_X),
                                            xv_get(addop_win, XV_Y),
*/
                           NOTICE_BUTTON_YES,      "Continue",
                           NULL);
  }
  free(full_command);

  free(command);
}

static void 
add_op_help(item, event)
Panel_item      item;
Event           *event;
{
  if (debug_level)
    Fprintf(stderr, "Getting help.\n");

  exv_make_text_window(addop_win, 
                       "Help for add_op op", "Help", helpfile, 1, 0);
}

static void 
select_object_name(item, value, event)
Panel_item      item;
int             value;
Event           *event;
{
  if (value < 2)
  {
    xv_set(objNameStr, PANEL_INACTIVE, TRUE, NULL);
  }
  else
  {
    xv_set(objNameStr, PANEL_INACTIVE, FALSE, NULL);
  }
        
}

