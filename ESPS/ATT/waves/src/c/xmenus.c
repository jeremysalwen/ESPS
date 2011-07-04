/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987 through 1993  AT&T,	*/
/*	  Entropic Speech, Inc., and			*/
/*	  Entropic Research Laboratory, Inc.		*/
/*	  All Rights Reserved.				*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC RESEARCH LABORATORY, INC.			*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* xmenus.c */
/*
  Use walking menus to change button operation modes.
  Implement menu choice vectoring in a general way.
  A linked list of menu text items and associated operations
  is set up (and possibly dynamically modified).  When the
  "mode switch" is pulled right, a menu is generated based on
  this list and vectoring to the function appropriate for
  the mode is implemented by setting the menu item value to the
  function pointer.  Each function has the form of a window
  event proc.
  */

#ifndef lint
static char *sccs_id = "@(#)xmenus.c	1.14	11/13/95	ATT/ERL";
#endif

#include <Objects.h>
#include <xview/font.h>
#include <esps/unix.h>
#include <esps/limits.h>

#define SINGLE_HIT if(((event_id(event) == LOC_MOVE)||event_is_down(event)) && (event_id(event) != LOC_DRAG))

char	*savestring();

extern char *checking_selectors();

#define CHECK_QUERY(a,b) { char *cret; if((cret = checking_selectors(a,b))) return(cret); }

extern Menuop *spect_get_ops();

Event *last_event;

int	run_esps_prog();
void	markers_to_common(), menu_operate();

extern double ref_size;
extern Signal *get_playable_signal();

extern int	write_common;	/* in globals.c */

extern int	menu_item_key;	/* key for storing last selected menu item
				   as XV_KEY_DATA for canvas. */

extern Menu	wave_menu, spect_menu;
Menu		w_button_submenu, s_button_submenu;

static struct named_menu {char *name; Menu *menu; struct named_menu *submenus;}

		w_menl[] = {{NULL,	&w_button_submenu,  NULL},
			    {NULL,	NULL,		    NULL}},

		s_menl[] = {{NULL,	&s_button_submenu,  NULL},
			    {NULL,	NULL,		    NULL}},

		menl[] =   {{"wave",	&wave_menu,	    w_menl},
			    {"spect",	&spect_menu,	    s_menl},
			    {NULL,	NULL,		    NULL}};

extern int	w_verbose, debug_level;
 
void	e_play_between_marks(), e_play_window(), e_play_file(),
	e_play_from_cursor(), e_page_ahead(), e_page_back(),
	e_align_views(), e_position_view(), e_zoom_in(), e_zoom_out(),
	e_wb_spectrogram(), e_nb_spectrogram(), e_output_bitmap(),
	e_save_segment(), e_delete_segment(), e_insert_file(),
	e_kill(), e_forward_window(), e_backward_window(),
	e_zoom_full_out(), e_repeat_previous(),
	e_blow_up(), e_blow_up_call_function(), e_up_down_move_marks(),
	e_move_closest_mark(), e_modify_signal(), c_print_graphic(),
	e_print_ensemble();

extern void e_print_graphic(), do_print_ensemble();

static Menu wave_submenu = (Menu)NULL;

#ifdef TESTING_HANDLERS
void bus_err()
{
  int it[4], *pit = it;
  int nut;

  fprintf(stderr,"bus_err:\n");
  nut = (int)pit;
  nut++;
  pit = (int*)nut;
  *pit = 5;
  fprintf(stderr,"bus_err:%x %d\n",pit,*pit);
}
void seg_viol()
{
  int it[4], *pit = it;
  int nut;

  fprintf(stderr,"seg_viol:\n");
  pit = (int*)3;
  *pit = 5;
  fprintf(stderr,"seg_viol:%x %d\n",pit,*pit);
}
void zero_divide()
{
  double t2, t=1.23456, t3=0.0;
  fprintf(stderr,"zero_divide\n");
  
  t2 = t/t3;
  fprintf(stderr,"zero_divide: t:%f t2:%f t3:%f\n",t,t2,t3);
}
void underflow()
{
  double big=1.12345678, bigger = 111.11111, sum;
  int trouble = 0;
  
  fprintf(stderr,"underflow\n");
  while (trouble++ < 100000) {
    sum = big + bigger;
    bigger = bigger * bigger;
  }
  fprintf(stderr,"underflow: i:%d sum:%f\n",trouble,sum);
}
#endif  

/* This list comprises the "built-in" xwaves waveform view menu commands. */
/*   The arguments for the called routines are (canvas, event, arg). */
Menuop
#ifdef TESTING_HANDLERS
testh5 = {"violate2",seg_viol, NULL, NULL},
testh4 = {"underflow",underflow, NULL, &testh5},
testh3 = {"div. by 0",zero_divide, NULL, &testh4},
testh2 = {"buss error",bus_err, NULL, &testh3},
testh1 = {"seg violate",seg_viol, NULL, &testh2},
  rbo14 =   {"kill window", e_kill, NULL, &testh1},
#else
  rbo14 =   {"kill window", e_kill, NULL, NULL},
#endif
  rbo13c =   {"print ensemble", e_print_ensemble, NULL, &rbo14},
  rbo13b =   {"print graphic", c_print_graphic, NULL, &rbo13c},
  rbo13 =   {"insert file", e_insert_file, NULL, &rbo13b},
  rbo12 =   {"delete segment", e_delete_segment, NULL, &rbo13},
  rbo11 =   {"save segment in file", e_save_segment, NULL, &rbo12},
  rbo10 =   {"output bitmap",NULL /* e_output_bitmap*/, NULL, &rbo11},
  rbo9 =    {"spectrogram (N.B.)", e_nb_spectrogram, NULL, &rbo10},
  rbo8 =    {"spectrogram (W.B.)", e_wb_spectrogram, NULL, &rbo9},
  rbo7a =   {"zoom full out", e_zoom_full_out, NULL, &rbo8},
  rbo7 =    {"zoom out", e_zoom_out, NULL, &rbo7a},
  rbo6b =   {"zoom in",	e_zoom_in, NULL, &rbo7},
  rbo6 =    {"bracket markers", e_position_view, NULL, &rbo6b},
  rbo5c =   {"align & rescale", e_align_views, NULL, &rbo6},
  rbo5b =   {"window back", e_backward_window, NULL, &rbo5c},
  rbo5a =   {"window ahead", e_forward_window, NULL, &rbo5b},
  rbo5 =    {"page back", e_page_back, NULL, &rbo5a},
  rbo4 =    {"page ahead", e_page_ahead, NULL, &rbo5},
  rbo3 =    {"play to end of file", e_play_from_cursor, NULL, &rbo4},
  rbo2 =    {"play entire file", e_play_file, NULL, &rbo3},
  rbo1 =    {"play window contents", e_play_window, NULL, &rbo2},
  right_but_menu =  {"play between marks", e_play_between_marks, NULL, &rbo1},
  wave_operators = {"repeat previous", e_repeat_previous, NULL, &right_but_menu};

/* The following menu ops. are specific to left and middle buttons. */
Menuop 
  mbo3 = {"blow up time", e_blow_up, NULL, &wave_operators},
  mbo4 = {"up/down move", e_up_down_move_marks, NULL, &mbo3},
  mbo5 = {"blow up; function", e_blow_up_call_function, NULL, &mbo4},
  lbo3 = {"modify signal", e_modify_signal, NULL, &mbo5},
  def_aux_but_ops = {"move closest", e_move_closest_mark, NULL, &lbo3},
  *aux_but_ops = &def_aux_but_ops;

void (*right_button_proc)() = menu_operate;

Menuop *blowup_op = NULL;

/*********************************************************************/
Menuop *wave_get_ops()
{
  return(&wave_operators);
}

/*********************************************************************/
Canvas menu_get_canvas(event)
     Event *event;
{
  Xv_object   win;
  Canvas      canvas;

  win = event_window(event);

  canvas = xv_get(win, CANVAS_PAINT_CANVAS_WINDOW);

  /* Sometimes (e.g. if you fiddle with a pullright item before making a
     final selection) the xv_get(menu, MENU_FIRST_EVENT) returns
     a LOC_DRAG event for which event_window returns the actual canvas,
     rather than an MS_RIGHT event for which event_window returns the
     canvas paint window.  (XView BUG???) */
  if (!canvas) canvas = win;
  return(canvas);
}

/*********************************************************************/
View *menu_get_view(menu)
     Menu menu;
{
  Event *event = (Event *) xv_get(menu, MENU_FIRST_EVENT);
  Canvas canvas = menu_get_canvas(event);
  View *v = (View *)xv_get(canvas, WIN_CLIENT_DATA);

  return(v);
}

/*********************************************************************/
Menuop *last_menuop(mo)
     Menuop *mo;
{
  while(mo) {
    if( ! mo->next)
      return(mo);
    mo = mo->next;
  }
  return(NULL);
}

/*********************************************************************/
Menuop *find_operator(ml, opname)
     Menuop *ml;
     char *opname;
{
  if(opname && *opname) {
    while(ml) {
      if(!strcmp(ml->name, opname))
	return(ml);
      ml = ml->next;
    }
  }
  return(NULL);
}

void donothing()
{
  return;
}

Menuop null_menu_operator = {"NO OP", donothing, NULL, NULL};

/*********************************************************************/
Menuop *get_null_op()
{
  return(&null_menu_operator);
}

/*********************************************************************/
void menu_set_blowup_op(val)
     char *val;
{
  if(val && *val) {
    Moplist  *menu_get_op_lists(), *mol = menu_get_op_lists();
    
    while(mol) {
      if((blowup_op = find_operator(mol->first_op, val)) &&
	 blowup_op->proc)
	return;
      mol = mol->next;
    }

    if(!(blowup_op && blowup_op->proc))
      blowup_op = &null_menu_operator;
    
    if(w_verbose > 1)
      fprintf(stderr, "Can't find an operator called \"%s\" in menu_set_blowup_op(); ignoring\n", val);
  }
}
  

/*********************************************************************/
Menuop *find_some_operator(ml, opname)
     Menuop *ml;
     char *opname;
{

  if((ml = find_operator(ml, opname)))
    return(ml);
  else
    return(&null_menu_operator);
}

extern Menuop spect_operators, def_aux_sbut_ops;

Moplist
  mol2 = {"wave", &wave_operators, &def_aux_but_ops, NULL},
  mol1 = {"spect", &spect_operators, &def_aux_sbut_ops, &mol2};

/*********************************************************************/
Moplist *menu_get_op_lists()
{
  return(&mol1);
}

/*********************************************************************/
Menuop *new_menuop(menu, name, proc, data)
     char *name, *menu;
     void (*proc)();
     void *data;
{
  if(name && *name) {
    Menuop *mo, *moe;
    Moplist *mol = &mol1;
    char *mname;

    if(!(menu && *menu))
      mname = "all";
    else
      mname = menu;

    while(mol) {
      if(!strcmp(mname, mol->name))
	break;
      mol = mol->next;
    }
    if(!mol) {
      Moplist *m = &mol1;
      while(m->next)
	m = m->next;
      m->next = mol = (Moplist*)malloc(sizeof(Moplist));
      mol->name = savestring(mname);
      mol->next = NULL;
      mol->reserve_ops = mol->first_op = mo = (Menuop*)calloc(1, sizeof(Menuop));
      mo->name = savestring(name);
    } else
      if(!(mo = find_operator(mol->first_op, name))) {
	moe = last_menuop(mol->first_op);
	mo = moe->next = (Menuop*)calloc(1, sizeof(Menuop));
	mo->name = savestring(name);
      }
    mo->proc = proc;
    mo->data = data;
    return(mo);
  }
  return(NULL);
}
	
/*********************************************************************/
char *
get_menu_label(mi,ml)
     register void (*mi)();
     register Menuop *ml;
{
  while(ml) {
    if(mi == ml->proc) return(ml->name);
    ml = ml->next;
  }
  return(NULL);
}

/*********************************************************************/
static caddr_t 
menu_null_proc(m, mi)
     Menu m;
     Menu_item mi;
{
  if(debug_level)
    fprintf(stderr,"Menu null proc\n");
  return(NULL);
}

/*********************************************************************/
/* Notify_proc called when an item in the waveform or spectrogram window
   menu is selected.  Heller shows the return type as Xv_opaque in version 1,
   changed to void in version 2.  Nevertheless, the code for version 2
   (in .../lib/libxvol/menu) still shows the type as Xv_opaque.  The value
   apparently is used as an argument for the menu_done_proc, if any, and
   is irrelevant if no menu_done_proc is defined. */
Xv_opaque
do_menu(menu, item)
    Menu        menu;
    Menu_item   item;
{
    Event       *event;
    Canvas      canvas;
    Menuop *mo;
    int		i;
    View *v;
    Signal *s;
    Object *o;

    if (debug_level) 
      (void) fprintf(stderr, "do_menu: function entered\n"); 

    event = (Event *) xv_get(menu, MENU_FIRST_EVENT);

    canvas = menu_get_canvas(event);

    if(canvas && (v = (View *)xv_get(canvas, WIN_CLIENT_DATA)) &&
       (s = v->sig) && (o = (Object*)(s->obj)))
      set_current_obj_name(o->name);

    if((mo = (Menuop*)xv_get(item, MENU_VALUE)) && mo->proc) {

      xv_set(canvas,
	     XV_KEY_DATA, menu_item_key, mo,
	     0);
      
      xv_set(event_window(event),
	     WIN_MOUSE_XY, event_x(event), event_y(event),
	     0);

      mo->proc(canvas, event, mo->data);
    }

    return XV_NULL;		/* Value irrelevant. */
}

/***********************************************************************/
/* Find top-level menu item, if any, named name.  xv_find won't work
   because, among other reasons, it looks in submenus. */

Menu_item
find_menu_item(menu, name)
    Menu    menu;
    char    *name;
{
    int		i;
    Menu_item	item;
    char	*str;

    for (i = (int) xv_get(menu, MENU_NITEMS);
	 i >= 1
	   && !((item = (Menu_item) xv_get(menu, MENU_NTH_ITEM, i))
		!= MENU_ITEM_NULL
	      && (str = (char *) xv_get(item, MENU_STRING))
	      && !strcmp(str, name));
	 i--
	) { }

    return (i >= 1) ? item : MENU_ITEM_NULL;
}
  
/*********************************************************************/
static caddr_t op_select_proc(m, mi)
     Menu m;
     Menu_item mi;
{
  Menuop *selected_op = NULL;
  Menu mp = m, mpp;

  if(debug_level)
    fprintf(stderr,"op_select_proc %s\n",xv_get(mi, MENU_STRING));
  if((selected_op = (Menuop*)xv_get(mi, MENU_VALUE)) &&
     (m = (Menu)xv_get(m, MENU_PARENT))) {
    View *v;
    extern char def_left_op[], def_middle_op[];
      
    while((mpp = (Menu)xv_get(mp, MENU_PARENT)))
      mp = mpp;
    v = menu_get_view(mp);
    if(debug_level)
      fprintf(stderr,"Item: %s\n",xv_get(m, MENU_STRING));
    if(v)
      if( !strcmp("left", (char*)xv_get(m, MENU_STRING)))
	v->left_but_proc = selected_op;
    else
	v->mid_but_proc = selected_op;
    if(v->sig)
      update_window_titles(v->sig);
  }
  return(NULL);
}

/*********************************************************************/
Menu 
make_generic_menu(mo, callback)
     void (*callback)();
    Menuop	*mo;
{
  Menu	menu;

  menu = xv_create(XV_NULL, MENU, 0);
  if (menu == MENU_NULL) {
    printf("make_generic_menu:  couldn't create menu.\n");
    exit_lm();
  }

  for ( ; mo; mo = mo->next)
    if(mo->proc)
      xv_set(menu,
	     MENU_ITEM,
	     MENU_STRING,	mo->name,
	     MENU_NOTIFY_PROC,	callback,
	     MENU_VALUE,	mo,
	     0,
	     0);

  return(menu);  
}

/*********************************************************************/
typedef struct but_it {
  Menu ops;
  char *name;
} Button_items;

/*********************************************************************/
Menu make_op_list(mi, op)
     Menu_item mi;
     Menu_generate op;
{
  Button_items *bi = (Button_items*)xv_get(mi, MENU_CLIENT_DATA);
  if(bi) {
    Menu allops = bi->ops;
    switch(op) {
    case MENU_DISPLAY:
      {
	Menuop *mo;
	Menu_item item;

	if( !allops ) {
	  allops = xv_create(XV_NULL, MENU,
			     MENU_NOTIFY_PROC,	op_select_proc,
			     0);
	  bi->ops = allops;
	}
	if(allops) {
	  Moplist *menu_get_op_lists(), *mol = menu_get_op_lists();
	  while(mol) {
	    if(!strcmp(mol->name,"all") || !strcmp(mol->name,bi->name)) {
	      for (mo = mol->reserve_ops; mo; mo = mo->next) {
		if(mo->proc && mo->name && *mo->name) {

		  item = find_menu_item(allops, mo->name);
	  
		  if (item != MENU_ITEM_NULL) 
		    xv_set(item,
			   MENU_VALUE,	    mo,
			   0);
		  else
		    xv_set(allops,
			   MENU_STRING_ITEM, mo->name, mo,
			   0);
		}
	      }
	    }
	    mol = mol->next;
	  }
	  if(debug_level)
	    fprintf(stderr,"DISPLAY %s\n",xv_get(mi, MENU_STRING));
	  return(allops);
	} else {
	  sprintf(notice_msg, "Problems in make_op_list %d.", allops);
	  show_notice(0, notice_msg);
	}
      }
      break;
    case MENU_DISPLAY_DONE:
      if(debug_level)
	fprintf(stderr,"DISPLAY_DONE %s\n",xv_get(mi, MENU_STRING));
      break;
    case MENU_NOTIFY:
      if(debug_level)
	fprintf(stderr,"NOTIFY: %s\n",xv_get(mi, MENU_STRING));
      break;
    case MENU_NOTIFY_DONE:
      if(debug_level)
	fprintf(stderr,"NOTIFY_DONE %s\n",xv_get(mi, MENU_STRING));
      break;
    }
    return(allops);
  } else
    show_notice(1, "Bad client data in make_op_list().");
  return(0);
}

/*********************************************************************/
Menuop *search_all_menus_but(excluded, item_name)
     char *excluded, *item_name;
{
  Menuop  *find_operator(), *found = NULL, *get_null_op();
  Moplist  *menu_get_op_lists(), *mol = menu_get_op_lists();

  if(item_name && *item_name && excluded && *excluded) {
    while(mol) {
      if(strcmp(mol->name,excluded) &&
	 (found = find_operator(mol->reserve_ops,item_name)))
	return(found);
      mol = mol->next;
    }

    sprintf(notice_msg,
	    "Can't find operator (%s) in search_all_menus_but(%s).",
	    item_name, excluded);
    show_notice(1, notice_msg);
  }

  return(get_null_op());

}

/*********************************************************************/
Menuop *find_op_list(name)
     char *name;
{
  if(name && *name) {
    Moplist *mol = menu_get_op_lists();

    while(mol) {
      if(!strcmp(mol->name, name))
	return(mol->first_op);
      mol = mol->next;
    }
  }
  return(NULL);
}

/*********************************************************************/
Menuop *find_op_in_list(opname, listname)
     char *opname, *listname;
{
  if(opname && *opname) {
    Menuop *mo = find_op_list(listname);
    
    while(mo) {
      if(!strcmp(mo->name, opname))
	return(mo);
      mo = mo->next;
    }
  }
  return(NULL);
}
    
/*********************************************************************/
Menu make_window_menu(menuname, but_menu, mid_ops, left_ops)
    char		*menuname;
    Menuop		*but_menu, *mid_ops, *left_ops;
{
  Menu	menu,
  mb = MENU_NULL;
  struct named_menu	*menus;
  char		*name;
  Menuop		*mo;
  int			n;
  Button_items *bl, *br;

  bl = (Button_items*)malloc(sizeof(Button_items));
  br = (Button_items*)malloc(sizeof(Button_items));
  bl->name = savestring(menuname);
  bl->ops = (Menu)NULL;
  br->name = savestring(menuname);
  br->ops = (Menu)NULL;

  menu = make_generic_menu(but_menu, do_menu);

  mb = xv_create(XV_NULL, MENU,
		 MENU_ITEM,
		 MENU_GEN_PULLRIGHT_ITEM,    "middle",   make_op_list,
		 MENU_CLIENT_DATA, br,
		 0,
		 MENU_ITEM,
		 MENU_GEN_PULLRIGHT_ITEM,    "left",	make_op_list,
		 MENU_CLIENT_DATA, bl,
		 0,
		 0);
  name = "Button Modes";
  wave_submenu = mb;

  for (n = 0; menl[n].name && strcmp(menl[n].name, menuname); )
    n++;
  if (menl[n].name && (menus = menl[n].submenus)) {
    menus[0].name = name;
    *menus[0].menu = wave_submenu;
  }
    
  if (wave_submenu != MENU_NULL)
    xv_set(menu,
	   MENU_PULLRIGHT_ITEM,	name, wave_submenu,
	   MENU_PULLRIGHT_DELTA,	30,
	   0);

  return menu;
}

/***********************************************************************/
Menu 
make_wave_menu()
{
    return make_window_menu("wave", &right_but_menu,
			    aux_but_ops, aux_but_ops);
}

/***********************************************************************/
void menu_change(strold, strnew, val, menuname)
     char	*strold, *strnew;
     Xv_opaque	val;
     char	*menuname;
{
  int		n;
  Menu	menu;
  Menu_item   item;

  if(menuname) {
    if(!*menuname || !strcmp(menuname, "all"))
      menuname = NULL;
    else
      if(*menuname && !strcmp(menuname, "none"))
	return;
  }

  for(n = 0; menl[n].name && menl[n].menu; n++)
    if((!menuname || !strcmp(menuname, menl[n].name))
       && (menu = *menl[n].menu) != MENU_NULL) {
      if(strold && *strold) {

	item = find_menu_item(menu, strold);

	if(strnew && *strnew) {
	  if (item != MENU_ITEM_NULL) 
	    xv_set(item,
		   MENU_STRING,	    savestring(strnew),
		   MENU_NOTIFY_PROC,   do_menu,
		   MENU_VALUE,	    val,
		   0);
	  else
	    xv_set(menu,
		   MENU_ITEM,
		   MENU_STRING,	savestring(strnew),
		   MENU_NOTIFY_PROC,	do_menu,
		   MENU_VALUE,		val,
		   0,
		   0);
	} else {
	  if(item != MENU_ITEM_NULL) {
	    xv_set(menu,
		   MENU_REMOVE_ITEM,   item,
		   0);
	    dt_xv_destroy(6,item);
	  }
	}
      }
      else
	if(strnew && *strnew)
	  xv_set(menu,
		 MENU_ITEM,
		 MENU_STRING,	savestring(strnew),
		 MENU_NOTIFY_PROC,	do_menu,
		 MENU_VALUE,		val,
		 0,
		 0);
    }
}

/*********************************************************************/
void
menu_clear(menuname)
    char	*menuname;
{
  int		i, n;
  Menu	menu;
  Menu_item	item;

  if(menuname && (!*menuname || !strcmp(menuname, "all")))
    menuname = NULL;

  for(n = 0; menl[n].name && menl[n].menu; n++)
    if((!menuname || !strcmp(menuname, menl[n].name))
       && (menu = *menl[n].menu) != MENU_NULL) {
      for(i = (int) xv_get(menu, MENU_NITEMS); i >= 1; i--) {
	item = (Menu_item) xv_get(menu, MENU_NTH_ITEM, i);
	xv_set(menu,
	       MENU_REMOVE_ITEM,   item,
	       0);
	if(item != MENU_ITEM_NULL)
	  dt_xv_destroy(7,item);
      }
    }
}

/*********************************************************************/
char *meth_save_menus(ob, str)
     Object *ob;
     char *str;
{
  static char menuname[50], output[NAMELEN];
  static Selector s2 = {"menu", "%s", menuname, NULL},
                  s1 = {"output", "%s", output, &s2};
  int		i, n, didone = 0, ni;
  Menu	menu;
  Menu_item	item;
  FILE *of;
  extern char ok[], null[];

  CHECK_QUERY(str, &s1)

  strcpy(menuname,"all");
  *output = 0;
  get_args(str, &s1);
  if(*output) {
    char scrat[NAMELEN];
    Menuop *mo;

    (void) build_filename(scrat, "", output); 

    if((of = fopen(scrat, "w"))) {
      for(n = 0; menl[n].name && menl[n].menu; n++)
	if((!strcmp(menuname, "all") || !strcmp(menuname, menl[n].name))
	   && (menu = *menl[n].menu) != MENU_NULL) {
	  for(ni = (int) xv_get(menu, MENU_NITEMS), i = 1; i <= ni; i++) {
	    item = (Menu_item) xv_get(menu, MENU_NTH_ITEM, i);
	    if(item && (mo = (Menuop*)xv_get(item, MENU_VALUE))) {
	      char *mn = (char*)xv_get(item, MENU_STRING);
	      if(mn) {
		if(!didone)
		  fprintf(of,"delete_all_items menu %s\n", menuname);
		didone = TRUE;
		if(strcmp(mn,"Button Modes"))
		  fprintf(of,"add_waves menu %s name \"%s\" op %s\n",
			  menl[n].name,mn,mo->name);
		else
		  fprintf(of, "add_waves menu %s submenu t\n",menl[n].name);
	      }
	    }
	  }
	}
      if(didone)
	fprintf(of,"return\n");
      fclose(of);
      return(ok);
    } else {
      sprintf(notice_msg, "Can't open %s for output in meth_save_menus().",
	      output);
      show_notice(1, notice_msg);
    }
  } else
    show_notice(1, "Output file must be specified in meth_save_menus().");
  return(null);
}

/*********************************************************************/
int
add_button_submenu(name, menuname)
    char	*name, *menuname;
{
  int		n;
  struct named_menu
    *menus;
  Menu	menu, submenu;
  Menu_item	item;

  if (menuname && (!*menuname || !strcmp(menuname, "all")))
    menuname = NULL;

  for (n = 0; menl[n].name && menl[n].menu; n++)
    if ((!menuname || !strcmp(menuname, menl[n].name))
	&& (menu = *menl[n].menu) != MENU_NULL
	&& (menus = menl[n].submenus)
	&& (menus[0].menu)
	&& (submenu = *menus[0].menu) != MENU_NULL) {

      if (!(name && *name))
	name = menus[0].name;

      if (name && *name) {
	if ((item = find_menu_item(menu, name)) != MENU_ITEM_NULL) {
	  xv_set(item,
		 MENU_NOTIFY_PROC,	(Xv_opaque (*)())  NULL,
		 MENU_PULLRIGHT,		submenu,
		 MENU_PULLRIGHT_DELTA,	30,
		 0);
	} else {
	  if (name != menus[0].name)
	    name = savestring(name);

	  xv_set(menu,
		 MENU_PULLRIGHT_ITEM,	name, submenu,		
		 MENU_PULLRIGHT_DELTA,	30,
		 0);
	}
      }
    }
}

/*********************************************************************/
menu_get_n(me, mo)
     Menu me;
     Menuop *mo;
{
  if(me && mo) {
    int i;
    Menu_item item;
    for(i = (int) xv_get(me, MENU_NITEMS); i >= 1; i--) {
	item = (Menu_item) xv_get(me, MENU_NTH_ITEM, i);
	if(mo == (Menuop*)xv_get(item, MENU_VALUE))
	  return(i);
      }
  }
  return(-1);

}

/*********************************************************************/
void menu_operate(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  int menu_def;
  Menuop *mo;
  Menu me = (Menu) xv_get(canvas, WIN_MENU);
  
  mo = (Menuop*) xv_get(canvas, XV_KEY_DATA, menu_item_key);
  if ((menu_def = menu_get_n(me, mo)) < 0)
    menu_def = 1;	/* last selected item no longer exists */
  xv_set(me,
	 MENU_DEFAULT, menu_def,
	 0);

  menu_show(me, canvas, event, NULL);
}

/*********************************************************************/
void 
e_play_window(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  View *v;
  
  SINGLE_HIT {
    v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    play_file(get_playable_signal(v), v->start_time, ET(v));
  }
}

/*********************************************************************/
void 
e_play_between_marks(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  View *v;
  
  SINGLE_HIT {
    v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    play_file(get_playable_signal(v), v->lmarker_time, v->rmarker_time);
  }  
}

/*********************************************************************/
void 
e_play_file(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT {
    View *v;
    Signal *s;
    
    v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    if((s = get_playable_signal(v)))
      play_file(s, s->start_time, s->end_time); /* should use SIG_END_TIME() */
  }
}

/*********************************************************************/
void 
e_play_from_cursor(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  View *v;
  
  v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
  SINGLE_HIT
    play_file(get_playable_signal(v), v->cursor_time, v->sig->end_time); /* should use SIG_END_TIME() */
}

/*********************************************************************/
void 
e_page_ahead(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT {
    View *v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    Signal *s = v->sig;
    
    if(s && v)
      page(v, s, v->start_time + v->page_step);
  }
  return;
}

/*********************************************************************/
void 
e_page_back(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT {
    View *v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    Signal *s = v->sig;
    
    if(s && v)
      page(v, s, v->start_time - v->page_step);
  }
  return;
}

/*********************************************************************/
void 
e_output_bitmap(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  return;
}

/*********************************************************************/
void
e_forward_window(canvas, event, arg) 
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT {
    double end_time;

    View *v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    Signal *s = v->sig;
    
    /* note ET sensitive to PIX_PER_CM (see Objects.h) */
    
    if(s && v)
      page(v, s, ET(v));
  }

}


/*********************************************************************/
void
e_backward_window(canvas, event, arg) 
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT {
    View *v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    Signal *s = v->sig;
    double width;
    
    /* note ET sensitive to PIX_PER_CM (see Objects.h) */
    
    width = ET(v) - v->start_time;
    
    if(s && v)
      page(v, s, (v->start_time) - width);
  }
}

/*********************************************************************/
void
e_zoom_full_out(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT {
    View *v;
    static double time;
    
    v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    
    zoom_view(v,v->cursor_time, ref_size/(ET(v) - v->start_time));
  }
} 

/*********************************************************************/
void 
e_position_view(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT {
    View *v;
  
    v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    position_view(v, v->lmarker_time, v->rmarker_time);
  }
}

/*********************************************************************/
void 
e_zoom_in(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT {
    View *v;
    
    v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    
    zoom_view(v,v->cursor_time,v->zoom_ratio);
  }
}

/*********************************************************************/
void 
e_blow_up(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  View *v;
  extern int zoom_ganged;
  int holdit;

  v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
  if(event_id(event) != LOC_DRAG) {
    holdit = zoom_ganged;
    zoom_ganged = 0;
    if(event_is_down(event))
      zoom_view(v,v->cursor_time,0.1);
    else
      zoom_view(v,v->cursor_time,10.0);
    zoom_ganged = holdit;
  } else
    transform_xy(v, event_x(event), event_y(event));
}

/*********************************************************************/
void 
e_blow_up_call_function(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  View *v;
  extern int zoom_ganged;
  int holdit;

  v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    
  if(event_id(event) != LOC_DRAG) {
    holdit = zoom_ganged;
    zoom_ganged = 0;
    if(event_is_down(event))
      zoom_view(v,v->cursor_time,0.1);
    else {
      zoom_view(v,v->cursor_time,10.0);
      if(blowup_op && blowup_op->proc) {
	event_set_down(event);
	blowup_op->proc(canvas, event, blowup_op->data);
      }
    }
    zoom_ganged = holdit;
  } else
    transform_xy(v, event_x(event), event_y(event));
}

/*********************************************************************/
void 
e_zoom_out(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT {
    View *v;
    
    v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    
    zoom_view(v,v->cursor_time,1.0/v->zoom_ratio);
  }
}


/*********************************************************************/
void 
e_align_views(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  View *v;
  Object *obj;

  v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
  if (!v || !v->sig || !(obj = (Object *)v->sig->obj))  return;
  
  SINGLE_HIT do_align_views(obj,v);
}

/*********************************************************************/
void 
e_save_segment(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT {
    View *v, *v2;
    extern int edit_ganged;
    Signal *s, *s2;
    double start,duration;
    extern char outputname[];
    char next[200];
    int itsok = TRUE;

    v2 = v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    start = v->lmarker_time;
    duration = v->rmarker_time - v->lmarker_time;
    if(!save_segment(v->sig,start,duration)) itsok = FALSE;
    if(itsok) {
      get_output_file_names(outputname, next);
      update_filename_display(next);
    }

    if(edit_ganged) {
      s = v->sig;
      s2 = ((Object*)(s->obj))->signals;
      while(s2) {
	if((s2 != s) && (s2->views) && (s2->views->sig == s2)) {
	  if(!save_segment(s2,start,duration)) itsok = FALSE;
	  if(itsok) {
	    get_output_file_names(outputname, next);
	    update_filename_display(next);
	  }
	}
	s2 = s2->others;
      }
    }
  }
  return;
}



/*********************************************************************/
void 
e_insert_file(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT {
    View *v;
    Signal *s, *s2;
    double start,end;
  
    v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    paste(v->sig,v->cursor_time);
  }
  return;
}

/*********************************************************************/
void
e_kill(canvas, event, arg)
    Canvas  canvas;
    Event   *event;
    caddr_t arg;
{
  SINGLE_HIT {
    Frame   frame;

    frame = (Frame) xv_get(canvas, XV_OWNER);
    xv_set(frame,
		FRAME_NO_CONFIRM, TRUE,
		0);
    dt_xv_destroy_safe(8,frame);
  }
}

/*********************************************************************/
void 
e_delete_segment(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT  {
    View *v, *v2;
    extern int edit_ganged;
    Signal *s, *s2;
    double start,end;
  
    v2 = v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
    start = v->lmarker_time;
    end = v->rmarker_time;
    do_deletions(v,start,end);
    if(edit_ganged) {
      s = v->sig;
      s2 = ((Object*)(s->obj))->signals;
      while(s2) {
	if((s2 != s) && (s2->views) && (s2->views->sig == s2))
	  do_deletions(s2->views,start,end);
	s2 = s2->others;
      }
    }
  }
  return;
}


/*********************************************************************/
void 
e_wb_spectrogram(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  View *v;

  v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
  
  SINGLE_HIT spectrogram(v->sig,"wideband",v->lmarker_time,v->rmarker_time,NULL);
}

/*********************************************************************/
void 
c_print_graphic(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  SINGLE_HIT e_print_graphic(canvas, event, arg);
}

/*********************************************************************/
void 
e_print_ensemble(canvas, event, arg)
    Canvas  canvas;
    Event   *event;
    caddr_t arg;
{
  View *v;
  Object *obj;

  v = (View *) xv_get(canvas, WIN_CLIENT_DATA);
  if (!v || !v->sig || !(obj = (Object *) v->sig->obj)) return;
  
  SINGLE_HIT do_print_ensemble(obj);
}

/*********************************************************************/
void 
e_nb_spectrogram(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  View *v;

  v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
  
  SINGLE_HIT spectrogram(v->sig,"narrowband",v->lmarker_time,v->rmarker_time,NULL);
}

/*********************************************************************/
void 
e_up_down_move_marks(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  View *v;

  v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
  if(event_id(event) != LOC_DRAG) {
    if(!event_shift_is_down(event))
      move_markers(v, v->cursor_time, event_is_down(event));
    else
      move_ord_markers(v, event_x(event), event_y(event), event_is_down(event));
  } else
    transform_xy(v, event_x(event), event_y(event));

  if (write_common && event_is_up(event)) 
    markers_to_common(v);
}

/*********************************************************************/
void 
e_move_closest_mark(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  View *v = (View *)xv_get(canvas, WIN_CLIENT_DATA);

  if(event_is_down(event)) {
    if(event_shift_is_down(event)) {
      int do_top = 1, y = event_y(event), yt, yb, x = event_x(event);

      v->cursor_channel = v->tmarker_chan;
      yt = v->yval_to_y(v, v->tmarker_yval);
      v->cursor_channel = v->bmarker_chan;
      yb = v->yval_to_y(v, v->bmarker_yval);

      if(iabs(yt - y) > iabs(yb - y))
	do_top = 0;
      v->cursor_channel = v->xy_to_chan(v,x,y);
      move_ord_markers(v, x, y, do_top);
    } else {
      double time = v->cursor_time;
      if(fabs(v->lmarker_time - time) < fabs(v->rmarker_time - time))
	move_markers(v, time, 1);
      else
	move_markers(v, time, 0);
    }
  }

  if (write_common && event_is_up(event)) 
    markers_to_common(v);
}

/*********************************************************************/
void
e_repeat_previous(canvas, event, arg)
    Canvas	canvas;
    Event	*event;
    caddr_t	arg;
{
    Menuop *mo;

    SINGLE_HIT {
      mo = (Menuop*) xv_get(canvas, XV_KEY_DATA, menu_item_key);
      if(mo && mo->proc)
	mo->proc(canvas, event, mo->data);
    }
}

/*********************************************************************/
void
e_exec_waves(canvas, event, arg)
    Canvas	canvas;
    Event	*event;
    caddr_t	arg;
{
    extern char	*dispatch(),
		*receiver_prefixed();
    extern void exec_waves();

    if (debug_level)
	fprintf(stderr, "e_exec_waves: command \"%s\"\n", (char *) arg);

    SINGLE_HIT exec_waves((char *) arg);
}

/**************************************************************/
char **get_ops(mop)
     Menuop *mop;
{
  Menuop *mo = mop;
  int no = 0;
  char **wopl = NULL, **sort_a_list();
  
  while(mo) {
    no++;
    mo = mo->next;
  }
  if(no) {
    wopl = (char**) malloc(sizeof(char**) * (no+1));
    no = 0;
    while(mop) {

      if(mop->name && mop->name[0] && mop->proc)
	wopl[no++] = mop->name;

      mop = mop->next;
    }
    wopl[no] = NULL;
  }
  return(sort_a_list(wopl));
}

/**************************************************************/
char **get_full_op_list()
{
  char **wops = NULL;
  char **fop = NULL, **sort_a_list(), **uniq_list(), **fopp = NULL;
  int n = 0, ns = 0;
  Moplist  *menu_get_op_lists(), *mol = menu_get_op_lists();

  while(mol) {
    if((wops = get_ops(mol->first_op))) {

      n = 0;
      while(wops[n]) n++;
      
      ns += n;
      
      if(!fop)
	fop = (char**)malloc(sizeof(char**) * (ns+1));
      else
	fop = (char**)realloc(fop, sizeof(char**) * (ns+1));

      fopp = fop + ns - n;

      for(n=0; wops && wops[n]; n++)
	*fopp++ = wops[n];

      free(wops);

      *fopp = NULL;
    }
    mol = mol->next;
  }
  return(uniq_list(sort_a_list(fop)));
}

/**************************************************************/
char *pack_up_list_items(ol)
     char **ol;
{
  static char *the_list = NULL;
  static int llen = 0;

  if(ol) {
    int i, slen, j;
    for(i = 0, slen = 0; ol[i]; i++)
      slen += strlen(ol[i]) + 3;

    slen++;
    
    if(slen > llen) {
      if(the_list)
	free(the_list);
      the_list = NULL;
      llen = 0;
      if(!(the_list = malloc(slen))) {
	show_notice(1 ," allocation failure in pack_up_list_items().");
	return(NULL);
      } else
	if(debug_level)
	  fprintf(stderr,"Allocated %d bytes in pack_up_list_items()\n",slen);
     
      llen = slen;
    }
    for(j=0, *the_list = 0; j < i; j++)
      if(strchr(ol[j], ' '))	/* embedded space? */
	sprintf(the_list + strlen(the_list), "\"%s\" ", ol[j]);
      else
	sprintf(the_list + strlen(the_list), "%s ", ol[j]);

    free(ol);
    return(the_list);
  }
  return(NULL);
}

/**************************************************************/
char *get_string_list(op_type)
     char *op_type;
{
  static char *the_list = NULL;
  static int llen = 0;
  char **ol;
  extern char **waves_get_commands(), **object_get_commands(),
              **view_get_variable_names(), **get_new_files_list(),
              **get_objects_list();

  if(!strcmp(op_type,"view"))
    ol = get_full_op_list();

  if(!strcmp(op_type,"waves"))
    ol = waves_get_commands();

  if(!strcmp(op_type,"object"))
    ol = object_get_commands();

  if(!strcmp(op_type,"settables"))
    ol = get_ops(view_get_settable_list());

  if(!strcmp(op_type,"variables"))
    ol = view_get_variable_names();

  if(!strcmp(op_type,"new_files"))
    ol = get_new_files_list();

  if(!strcmp(op_type,"objects")) 
   ol = get_objects_list();

  return(pack_up_list_items(ol));

}

