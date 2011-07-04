/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rodney Johnson, ERL
 * Checked by:
 * Revised by:
 *
 *  xpanels.c
 *  Linked lists of run-time-configurable auxiliary control panels.
 */

static char *sccs_id = "@(#)xpanels.c	1.7	9/28/98	ATT/ERL";


#include <Objects.h>
#include <esps/exview.h>
#include <esps/esps.h>
#include <xaddop.h>

#define DEF_PANEL_LOC_X     1
#define DEF_PANEL_LOC_Y     1

#define DELTA_DEF_PANEL_LOC_X     20
#define DELTA_DEF_PANEL_LOC_Y     20

#define MAX_DEF_PANEL_LOC_X     500
#define MAX_DEF_PANEL_LOC_Y     500

extern char	*savestring();

extern int	debug_level;
extern int	auxpanel_key;

/* Structure for linked list of run-time-configurable auxiliary control panels.
 */
typedef struct auxpanel {
    char	    *name, *input_file, *panel_title, *icon_title;
    int	            quit, cols, panel_choice, choice_horiz, loc_x, loc_y;
    Panel	    panel;
    struct auxpanel *next;
    }   AuxPanel;

static AuxPanel *panels = NULL;

int	def_panel_loc_x = DEF_PANEL_LOC_X,
	def_panel_loc_y = DEF_PANEL_LOC_Y;

static Panel	    define_panel_from_file();
static AuxPanel	    *find_auxpanel();
static AuxPanel	    *link_new_auxpanel();
static int	    unlink_auxpanel();
static int	    destroy_panel();
static Notify_value destroy_func();
static void	    btn_exec_waves();


static Panel define_panel_from_file(input_file, quit, cols,
		       panel_title, icon_title, loc_x, loc_y, panel_choice, choice_horiz)
    char	*input_file;
    int		quit, cols;
    char	*panel_title, *icon_title;
    int		loc_x, loc_y, panel_choice, choice_horiz;
{
    Panel	panel;
    Frame	frame;
    bbox_par	*bpar;
    static int  last_loc_x = 0;
    static int  last_loc_y = 0;
    
    bpar = exv_bbox((bbox_par *) NULL, &frame, &panel);
    bpar->menu_file = input_file;
    bpar->quit_button = quit;
    bpar->but_data_proc = btn_exec_waves;
    bpar->n_per_row = cols;
    bpar->owner = XV_NULL;
    bpar->title = panel_title;
    bpar->icon_title = icon_title;
    bpar->show = 1;
    bpar->button_choice = panel_choice;
    bpar->choice_horizontal = choice_horiz;

    /* if no explicit positioning was set, we put in a 
     *  default position (so twm user doesn't have to position) 
     */
     
    if ((loc_x < 0) || (loc_y < 0)) {

	last_loc_x += DELTA_DEF_PANEL_LOC_X;
	last_loc_y += DELTA_DEF_PANEL_LOC_Y;

	if ((last_loc_x > MAX_DEF_PANEL_LOC_X) 
	    || (last_loc_y > MAX_DEF_PANEL_LOC_Y))

	    last_loc_x = last_loc_y = 0;

	bpar->x_pos = last_loc_x;
	bpar->y_pos = last_loc_y;
      }
    else {
	bpar->x_pos = loc_x;
	bpar->y_pos = loc_y;
      }
    (void) exv_bbox(bpar, &frame, &panel);

    return panel;
}


int make_auxpanel(input_file, panel_name,
	      quit, cols, panel_title, icon_title, loc_x, loc_y, panel_choice, choice_horiz)
    char	*input_file, *panel_name;
    int		quit, cols, panel_choice, choice_horiz;
    char	*panel_title, *icon_title;
    int		loc_x, loc_y;
{
  Panel	panel;
  Frame	frame;
  AuxPanel	*auxp;

  if ((input_file = savestring(input_file)) && *input_file)
    {
      if ((panel_name = savestring(panel_name)) && *panel_name)
	{
	  if (panel_title = savestring(panel_title ? panel_title : ""))
	    {
	      if (icon_title = savestring(icon_title ? icon_title : ""))
		{
		  if ((panel = define_panel_from_file(input_file, quit, cols,
						      panel_title, icon_title, loc_x, loc_y,
						      panel_choice, choice_horiz))
		      != XV_NULL
		      && (frame = xv_get(panel, XV_OWNER)) != XV_NULL)
		    {
		      if (auxp = find_auxpanel(panel_name))
			(void) destroy_panel(auxp->panel);

		      if (auxp = link_new_auxpanel())
			{
			  auxp->name = panel_name;
			  auxp->input_file = input_file;
			  auxp->panel_title = panel_title;
			  auxp->icon_title = icon_title;
			  auxp->quit = quit;
			  auxp->cols = cols;
			  auxp->panel_choice = panel_choice;
			  auxp->choice_horiz = choice_horiz;
			  auxp->loc_x = loc_x;
			  auxp->loc_y = loc_y;
			  auxp->panel = panel;
			  xv_set(frame,
				 XV_KEY_DATA,	auxpanel_key, auxp,
				 0);
			  notify_interpose_destroy_func(frame, destroy_func);
			  return TRUE;
			}
		      destroy_panel(panel);
		    }
		  free(icon_title);
		}
	      free(panel_title);
	    }
	  free(panel_name);
	}
      free(input_file);
    }
  return FALSE;
}


int
close_auxpanel(panel_name, invisible)
    char	*panel_name;
     int invisible;
{
    AuxPanel	*auxp;
    Frame	frame;

    auxp = find_auxpanel(panel_name);
    if (!auxp) return FALSE;
    frame = (Frame) xv_get(auxp->panel, XV_OWNER);
    if (frame == XV_NULL) return FALSE;
    xv_set(frame,	FRAME_CLOSED,	TRUE,
			0);
    if(invisible)
      xv_set(frame, XV_SHOW, FALSE, 0); 
    return TRUE;
}


int
open_auxpanel(panel_name)
    char    *panel_name;
{
    AuxPanel	*auxp;
    Frame	frame;

    auxp = find_auxpanel(panel_name);
    if (!auxp) return FALSE;
    frame = (Frame) xv_get(auxp->panel, XV_OWNER);
    if (frame == XV_NULL) return FALSE;
    xv_set(frame,	FRAME_CLOSED,	    FALSE,
			0);
    xv_set(frame, XV_SHOW, TRUE, 0); 
    return TRUE;
}


int
kill_auxpanel(panel_name)
    char    *panel_name;
{
    AuxPanel	*auxp;
    Frame	frame;

    auxp = find_auxpanel(panel_name);
    if (!auxp) return FALSE;

    return destroy_panel(auxp->panel);
}


static AuxPanel *
link_new_auxpanel()
{
  AuxPanel	*auxp;

  auxp = (AuxPanel *) malloc(sizeof(AuxPanel));
  if (auxp) {
    auxp->name = NULL;
    auxp->input_file = NULL;
    auxp->panel_title = NULL;
    auxp->icon_title = NULL;
    auxp->quit = 0;
    auxp->cols = 1;
    auxp->panel_choice = 0;
    auxp->choice_horiz = 1;
    auxp->loc_x = -1;
    auxp->loc_y = -1;
    auxp->panel = XV_NULL;
    auxp->next = panels;
    panels = auxp;
  }
  return auxp;
}


static AuxPanel *
find_auxpanel(panel_name)
    char    *panel_name;
{
    AuxPanel	*auxp;

    if (!panel_name || !*panel_name) return NULL;

    for (auxp = panels;
	 auxp && strcmp(auxp->name, panel_name);
	 auxp = auxp->next)
    { }

    return auxp;
}


static int
unlink_auxpanel(auxp)
    AuxPanel	*auxp;
{
    AuxPanel	*pp;

    if (auxp == panels)
	panels = auxp->next;
    else
    {
	for (pp = panels; pp && pp->next != auxp; pp = pp->next)
	{ }
	if (!pp) return FALSE;
	pp->next = auxp->next;
    }

    return TRUE;
}


static Notify_value
destroy_func(client, status)
    Notify_client   client;
    Destroy_status  status;
{
    Frame	    frame = (Frame) client;

    if (status == DESTROY_CLEANUP)
    {
	AuxPanel    *auxp;

	if (auxp = (AuxPanel *) xv_get(frame, XV_KEY_DATA, auxpanel_key))
	{
	    (void) unlink_auxpanel(auxp);
	    if (auxp->name) free(auxp->name);
	    if (auxp->icon_title) free(auxp->icon_title);
	    if (auxp->panel_title) free(auxp->panel_title);
	    if (auxp->input_file) free(auxp->input_file);
	    free(auxp);
	    xv_set(frame,
			XV_KEY_DATA,	auxpanel_key, XV_NULL,
			0);
	}
    }

    return(notify_next_destroy_func((Notify_client) frame, status));
}


static int
destroy_panel(panel)
    Panel   panel;
{
    Frame   frame;

    if (panel == XV_NULL) return FALSE;
    frame = (Frame) xv_get(panel, XV_OWNER);
    if (frame == XV_NULL) return FALSE;
    xv_set(frame,	FRAME_NO_CONFIRM,   TRUE,
			0);
    dt_xv_destroy_safe(10,frame);

    return TRUE;
}


static void
btn_exec_waves(data, button)
    char	*data;
    Panel_item	button;
{
    extern char	*dispatch(),
		*receiver_prefixed();
    extern void exec_waves();

    if (debug_level)
	fprintf(stderr, "btn_exec_waves: command \"%s\"\n", (char *) data);

    /* (void) dispatch(receiver_prefixed(data)); */
    exec_waves(data);
}

make_add_op_panel(help, loc_x, loc_y, ops_win, vars_win, proc)
     char *help;
     int loc_x, loc_y, ops_win, vars_win;
     int (*proc)();
{
  addop_par *aop, *xaddop();
  char **w_vars = NULL, **v_vars = NULL, **ops = NULL,
       **object_get_commands(), **waves_get_variable_names(),
       **view_get_variable_names(), **get_vc_attr_list();

  if((aop = xaddop(NULL)) && (ops = object_get_commands()) &&
     (w_vars = waves_get_variable_names()) &&
     (v_vars = view_get_variable_names())) {

    aop->global_vars = w_vars;
    aop->view_vars = v_vars;
    aop->get_attr_list = get_vc_attr_list;
    aop->operations = ops;
    if(loc_x != def_panel_loc_x)
      aop->addop_x_pos = loc_x;
    if(loc_y != def_panel_loc_y)
      aop->addop_y_pos = loc_y;
    aop->show_vars_win = vars_win;
    aop->show_ops_win = ops_win;
    aop->apply_add_op = proc;
    aop->owner = XV_NULL;
    aop->helpfile = help;
    xaddop(aop);
/*    free(aop);
    free(vars);
    free(ops); */
    return(TRUE);
  }
  if(aop)
    free(aop);
  if(ops)
    free(ops);
  return(FALSE);
}

extern char *checking_selectors();

#define CHECK_QUERY(a,b) { char *cret; if((cret = checking_selectors(a,b))) return(cret); }

char *meth_save_panels(ob, str)
     Object *ob;
     char *str;
{
  static char file[NAMELEN];
  static Selector s = {"output", "%s", file, NULL};
  extern char ok[], null[];

  CHECK_QUERY(str,&s)
    *file = 0;
  get_args(str,&s);
  if(file && *file) {
    char scrat[NAMELEN];
    FILE *of;
    
    /* expand any environment variables */
    (void) build_filename(scrat, "", file); 
    if((of = fopen(scrat, "w"))) {
      AuxPanel *ap = panels;
      while(ap) {
	if(ap->panel) {
	  Frame f = (Frame)xv_get(ap->panel, XV_OWNER);
	  ap->loc_x = xv_get(f, XV_X);
	  ap->loc_y = xv_get(f, XV_Y);


/* This sleazy hack is needed since (on Suns) the x,y loc used when 
   creating a window is not the same as you get when asking about 
   the location of window once it is created.  The difference seems 
   to be the window title bar.  */

#ifdef SUN4
	  if(ap->loc_y >= 25)
		ap->loc_y -= 25;
#endif

	  fprintf(of,"make_panel file %s loc_x %d loc_y %d quit_button %d columns %d panel_choice %d choice_horiz %d",ap->input_file,ap->loc_x,ap->loc_y,ap->quit,ap->cols,ap->panel_choice,ap->choice_horiz);
	  if(ap->name && *ap->name)
	    fprintf(of," name %s", ap->name);
	  if(ap->icon_title && *ap->icon_title)
	    fprintf(of," icon_title \"%s\"",ap->icon_title);
	  if(ap->panel_title && *ap->panel_title)
	    fprintf(of," title \"%s\"",ap->panel_title);
	  fprintf(of,"\n");
	}
	ap = ap->next;
      }
      fprintf(of,"return\n");
      fclose(of);
      return(ok);
    } else {
      sprintf(notice_msg,
	      "Problems opening output file %s in meth_save_panels",
	      file);
      show_notice(1,notice_msg);
    }
  } else
    show_notice(1,"No output file was specified to meth_save_panels.");
  return(null);
}
  
