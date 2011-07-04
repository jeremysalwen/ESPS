/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987 through 1993  AT&T,	*/
/*	  Entropic Speech, Inc., and			*/
/*	  Entropic Research Laboratory, Inc.		*/
/*	  All Rights Reserved.				*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC RESEARCH LABORATORY, INC.			*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* browser.c */

/* Provide homebrew lists of selectable items.  The primary use is the
   display of lists of files that may be selected for some operation
   (like being read and displayed by xwaves.).  This should probably be
   replaced by some "standard" GUI technology, since it was originally
   designed to overcome deficits in Sunview...
*/


#ifndef lint
static char *sccs_id = "@(#)browser.c	1.9	9/28/98	ATT/ERL";
#endif

#include <Objects.h>
#include <xview/font.h>
#include <esps/unix.h>
#include <esps/limits.h>

extern int	w_verbose, debug_level;
extern char	play_program[], inputname[], *basename(), *remove_reg_exp();
extern void newText_proc();
extern Xv_Font  def_font;
extern int      def_font_height, def_font_width;

Pending_input new_files = {
    inputname,			/* results_to */
    NULL,			/* path_name_prefix */
    (Panel_item) NULL,		/* item */
    NULL,         /* list (files created during a run of waves) */
    (Canvas) NULL,		/* display window */
    newText_proc,		/* procedure to call on selection */
    FALSE,			/* destroy_on_select */
    "New Files",		/* banner */
    NULL,			/* next */
    NULL };			/* prev */

Pending_input *input_pending = &new_files;

/* The following menu operations are specific to the file browser windows.
   The arguments to the called routine are (canvas, filename, list_ele). */
/* The last two arguments passed are an event and NULL because of a change in
   menu handling upon conversion from SunView to XView.  The filename and
   list element are stored on the canvas as XV_KEY_DATA with keys file_name_key
   and list_elem_key. */
extern int  file_name_key, list_elem_key;

void	m_play_file(), m_remove_list_element(), m_pass_in_name(),
	m_print_file(), m_kill_display(), m_designate_source();

void menu_redoit();
static void menu_doit();

Menuop 
    mmo1 = {"play file", m_play_file, NULL, NULL},
    mmo1a = {"browse file", m_print_file, NULL, &mmo1},
    mmo2 = {"remove from list", m_remove_list_element, NULL, &mmo1a},
    mmo3 = {"pass in name", m_pass_in_name, NULL, &mmo2},
    mmo4 = {"display off", m_kill_display, NULL, &mmo3},
    mmo5 = {"source for insert", m_designate_source, NULL, &mmo4};

/*********************************************************************/
Pending_input *get_new_files()
{
  return(&new_files);
}

/*********************************************************************/
/* Notify_proc called when an item in a browser window
   menu is selected. */
do_browser_menu(menu, item)
    Menu        menu;
    Menu_item   item;
{
    Event       *event;
    Canvas      canvas;
    Menuop *mo;
    extern int	menu_item_key;	/* key for storing last selected menu item
				   as XV_KEY_DATA for canvas. */

    if (debug_level) 
      (void) fprintf(stderr, "do_browser_menu: function entered\n"); 

    event = (Event *) xv_get(menu, MENU_FIRST_EVENT);

    canvas = menu_get_canvas(event);

    if((mo = (Menuop*)xv_get(item, MENU_VALUE)) && mo->proc) {

      xv_set(canvas,
	     XV_KEY_DATA, menu_item_key, mo,
	     0);
      
      xv_set(event_window(event),
	     WIN_MOUSE_XY, event_x(event), event_y(event),
	     0);

      mo->proc(canvas, event, mo->data);
    }

}

/*********************************************************************/
/* Display for an arbitrarily long list of options. */
Canvas 
file_selection_window(p)
     Pending_input *p;
{
  Menu menu;
  extern  Frame daddy;
  extern Panel control_panel;
  extern Panel_item newFile_item;
  Canvas canvas;
  Rect *rect;
  caddr_t obj;
  Pixwin *pw;
  Pending_input *p2;
  int xloc, x, width;
  
  if( p ) {
    if((p->banner && !strcmp("New Files",p->banner)) ||
       (p->item == newFile_item))
      menu = make_generic_menu(&mmo5, do_browser_menu); /* include paste designate opt. */
    else
      menu = make_generic_menu(&mmo4, do_browser_menu);

    obj = (caddr_t)control_panel;
    rect = (Rect*)xv_get(control_panel, WIN_RECT);
    xloc = rect->r_width;
    if((p2 = input_pending)) {
      while(p2) {
	if((p2 != p) && p2->canvas) {
	  rect = (Rect*)xv_get(p2->canvas, WIN_RECT);
	  width = rect->r_width;
	  x = (int)xv_get(p2->canvas, WIN_X) + width;
	  if(x >= xloc) {
	    xloc = x;
	    obj = (caddr_t) p2->canvas;
	  }
	}
	p2 = p2->next;
      }
    }
    rect = (Rect*)xv_get(control_panel, WIN_RECT);
    canvas = xv_create(daddy, CANVAS,
			CANVAS_RETAINED,	FALSE, 
			CANVAS_FIXED_IMAGE,	FALSE,
			CANVAS_AUTO_SHRINK,	TRUE,
			CANVAS_AUTO_EXPAND,	TRUE,
			XV_HEIGHT,		rect->r_height,
			WIN_X,			xloc,
			WIN_Y,			0,
			XV_WIDTH,		200,
			CANVAS_WIDTH,		200,
			WIN_MENU,		menu,
			WIN_CLIENT_DATA,	p,
			0);
    xv_set(canvas_paint_window(canvas),
			WIN_CONSUME_EVENTS,
			    LOC_DRAG,
			    WIN_IN_TRANSIT_EVENTS,
			    LOC_MOVE,
			    0,
			WIN_EVENT_PROC,		menu_doit,
			0);
/*!*//* Why doesn't the following work? */
    xv_set(canvas_paint_window(canvas),
			WIN_IGNORE_EVENTS,
			    KBD_USE,
			    KBD_DONE,
			    0,
			0);
			   
    /* window_fit(canvas); */
    /* xv_set(daddy, XV_WIDTH, xloc + 200, 0); */
    window_fit(daddy);

/* Done here instead of in xv_create above in case of problems with
   call within xv_create. */

    xv_set(canvas,	   CANVAS_REPAINT_PROC, menu_redoit,
			   0);

    return canvas;
  }
  return((Canvas)NULL);
}

/*************************************************************************/
Menu_list *
add_to_menu_list(li,cp)
     Menu_list **li;
     char *cp;
{
  int  n = strlen(cp);
  Menu_list *l2 = *li, *l;
  caddr_t tp = NULL;
  int tap = FALSE;

  while(l2) {
    if(!strcmp(cp,l2->str)) {	/* already in list? */
      if(! l2->prev) return(l2); /* it's at top of list */
      tp = l2->data;
      tap = l2->tapped;
      l2->prev->next = l2->next;
      if(l2->next) l2->next->prev = l2->prev;
      free(l2->str);
      free(l2);
      break;
    }
    l2 = l2->next;
  }
  l2 = *li;
  if((l = (Menu_list*)malloc(sizeof(Menu_list))) &&
     (l->str = malloc(n+1))) {
    strcpy(l->str,cp);
    if(l2) {
      l->next = l2;
      l->prev = NULL;
      l2->prev = l;
      l2 = l;
    } else {
      l2 = l;
      l->next = NULL;
      l->prev = NULL;
    }
    l->active = FALSE;
    l->tapped = tap;
    l->data = tp;
    *li = l2;
    return(l2);
  }
  printf("Problems allocating mem in add_to_menu_list()\n");
  return(NULL);
}

/*********************************************************************/
Canvas 
listing_display(path,p)
     char *path;
     Pending_input *p;
{
  FILE *fs, *fopen();
  int nfi, n;
  char ctemp[500], *ntemp, *cp, *e_temp_name();
  Menu_list *l2=NULL;
  
  if((ntemp = e_temp_name(NULL))) {
    sprintf(ctemp,"/bin/ls -rFa %s > %s",path,ntemp);
    if(debug_level)
      fprintf(stderr, "listing_display: %s\n",ctemp);
    system(ctemp);
    nfi = 0;
    if((fs = fopen(ntemp,"r"))) {
      while(fgets(ctemp,200,fs)) {
        if((n = strlen(ctemp)) > 1) {
          if(ctemp[n-1] == '\n') {
            n--;
            ctemp[n] = 0;
          }
          cp = basename(ctemp);
          nfi++;
          if(!add_to_menu_list(&l2, cp)) {
            fclose(fs);
            unlink(ntemp);
            free(ntemp);
            return((Canvas)NULL);
          }
        }
      }
      fclose(fs);
      unlink(ntemp);
      free(ntemp);
      if(l2) {
        l2->active = TRUE;
        p->list = (caddr_t)l2;
        return(file_selection_window(p));
      }
    } else {
      sprintf(notice_msg,"Cannot open %s\n",ntemp);
      show_notice(1,notice_msg);
      unlink(ntemp);
      free(ntemp);
    }
  }
  return((Canvas)NULL);
}

/*************************************************************************/
destroy_pending_input(p)
     register Pending_input *p;
{
  extern Frame daddy;
  extern Panel control_panel;
  int killed = 0;
  Canvas canvas;
  
  if(p) {
    if(p == input_pending) {
      if( !p->next )
	input_pending = NULL;
      else {
	input_pending = p->next;
	input_pending->prev = NULL;
      }
    } else {
      if(p->prev) p->prev->next = p->next;
      if(p->next) p->next->prev = p->prev;
    }
    if(p->list) free_menu_list(p->list);
    if((canvas = p->canvas))
      killed = 1;
    if(p->path_prefix) free(p->path_prefix);
    free(p);
  }
  if(killed)
      reshuffle_and_kill(canvas);
  
  return;
}

/*************************************************************************/
/* If the Sun window system worked as advertised, this crap wouldn't be
   necessary... */
reshuffle_and_kill(canvas)
     Canvas canvas;
{
  extern Panel control_panel;
  extern Frame daddy;
  caddr_t obj;
  Pending_input *p = input_pending;
  Rect *rect;
  int locx;

  dt_xv_destroy_safe(1,canvas);

  window_fit(control_panel);
  rect = (Rect*)xv_get(control_panel, WIN_RECT);
  locx = rect->r_width;
  obj = (caddr_t)control_panel;

  while(p) {
    if(p->canvas) {
      if(p->canvas == canvas) {
	printf("Canvas still in list in reshuffle_and_kill()!\n");
	p->canvas = (Canvas)0;
      } else {
	xv_set(p->canvas, WIN_X, locx, XV_WIDTH, 200, 0);
	obj = (caddr_t)p->canvas;
	menu_redoit(p->canvas, NULL, NULL);
	locx += 200;
      }
    }
    p = p->next;
  }
  xv_set(daddy, XV_WIDTH, locx, 0);
  /* window_fit(daddy); */
}

/*************************************************************************/
select_from_alternatives(name,item)
     char *name;
     Panel_item item;
{
  Pending_input *p, *p2;
  extern void newText_proc();
  char *results_to;
  extern Panel_item overlay_item, newFile_item, newControl_item;
  extern char overlayname[], inputname[], commandname[];

  results_to = NULL;
  if(item == overlay_item) results_to = overlayname;
  if(item == newFile_item) results_to = inputname;
  /*   if(item == newControl_item) results_to = commandname; */
  if(!results_to) return;
  
  /* see if this string is already looking for input */
  if((p = input_pending))
    while(p) {
      if((p->results_to == results_to) && !p->banner) {
	/* if there's a banner, it wasn't created here! -- don't destroy */
	if((p->item == item) && p->path_prefix &&
	   (!strcmp(p->path_prefix,name)) && p->canvas)
	  return;		/* it's already set up! */
	else {
	  p2 = p->next;
	  destroy_pending_input(p);
	  p = p2;
	}
      } else
	p = p->next;
    }
 
  if((p = (Pending_input*)malloc(sizeof(Pending_input))) &&
     ((p->path_prefix = malloc(strlen(name) + 1))) ) {
    p->results_to = results_to;
    strcpy(p->path_prefix,name);
    p->item = item;
    p->proc = newText_proc;
    p->destroy_on_select = TRUE; /* this is optional */
    p->banner = NULL;
    if(input_pending) {
      input_pending->prev = p;
      p->next = input_pending;
    } else 
      p->next = NULL;
    input_pending = p;
    p->prev = NULL;
    if(*name == '@') name++;
    if(!(p->canvas = listing_display(name,p))) {
      show_notice(1,"Can't generate listing display in select_from_alternatives");
      free(p->path_prefix);
      if(!p->next)
	input_pending = NULL;
      else {
	input_pending = p->next;
	input_pending->prev = NULL;
      }
      free(p);
    }
  }
  return;
}

/*********************************************************************/
void 
menu_redoit(canvas, pwi, repaint_area)
     Canvas canvas;
     Pixwin *pwi;
     Rectlist *repaint_area;
{
  Pixwin *pw;
  Menu_list *l;
  Rect *rect;
  static int fhi;
  register int y, xloc;
  Pending_input *p;
  extern Frame daddy;
  
  p = (Pending_input*)xv_get(canvas, WIN_CLIENT_DATA);
  if (!p)
      return;
  l = (Menu_list*)p->list;
  pw = (Pixwin *)canvas_pixwin(canvas);
  rect = (Rect*)xv_get(canvas, WIN_RECT);

  fhi = def_font_height + 2;

  while(l && (! l->active)) l = l->next; /* find top of displayed section */
  if(l) {
    pw_write((Xv_opaque)pw,0,0,rect->r_width,rect->r_height,
	     PIX_COLOR(0)|PIX_SRC,NULL,0,0);
    y = fhi;
    while(l && (y < rect->r_height)) {
      if(l->tapped)
	pw_text((Xv_opaque)pw, 0, y, PIX_COLOR(1)|PIX_SRC, def_font, "*");
      pw_text((Xv_opaque)pw, 10, y, PIX_COLOR(1)|PIX_SRC, def_font, l->str);
      y += fhi;
      l = l->next;
    }
  }
  if(p->item) {		/* print label of panel item awaiting input */
    xloc = (int)xv_get(canvas, WIN_X);
    pw = (Pixwin*)xv_get(daddy, WIN_PIXWIN);
    if(!p->banner)
      pw_text((Xv_opaque)pw, xloc, 11, PIX_COLOR(FOREGROUND_COLOR)|PIX_SRC, def_font,
	      (char *) xv_get(p->item, PANEL_LABEL_STRING));
    else
      pw_text((Xv_opaque)pw, xloc, 11, PIX_COLOR(FOREGROUND_COLOR)|PIX_SRC, def_font,
	      p->banner);
  }
}

/*********************************************************************/
static void 
menu_doit(pw, event, arg)
    Pixwin  *pw;
    Event   *event;
    caddr_t arg;
{
  Canvas  canvas = xv_get((Xv_opaque)pw, CANVAS_PAINT_CANVAS_WINDOW);
  Menu_list *l, *l2;
  Rect *rect;
  static int fhi;
  register int y, x, height, locy, entry, ndis;
  Pending_input *p;
  Panel_item item;
  void (*proc)();
  char *result;
  Menu me;
  extern char *cleaned_for_input();

  switch (event_id(event))
  {
  case MS_LEFT:
  case MS_RIGHT:
  case LOC_DRAG:
  case LOC_MOVE:
      break;
  default:
      return;
  }
  
  p = (Pending_input*)xv_get(canvas, WIN_CLIENT_DATA);
  if (!p)
      return;
  l2 = l = (Menu_list*)p->list;
  rect = (Rect*)xv_get(canvas, WIN_RECT);

  x = event_x(event);
  y = event_y(event);

  fhi = def_font_height + 2;

  entry = (y - (fhi >> 2))/fhi;

  while(l && (! l->active)) l = l->next; /* find top of displayed section */

  if (l){
    switch(event_id(event)) {
    case MS_LEFT:

      if (event_is_up(event))
      {
	while(entry-- && l) l = l->next;
	if(l && p->results_to) {
	  result = p->results_to;
	  item = p->item;
	  proc = p->proc;
	  if(p->path_prefix)
	    strcpy(result,remove_reg_exp(p->path_prefix));
	  else
	    *result = 0;
	  strcat(result,cleaned_for_input(l->str));
	  if (item) xv_set(item, PANEL_VALUE, result, 0);
	  if(p->destroy_on_select)
	  {
	      xv_set(canvas,	WIN_CLIENT_DATA,	(caddr_t) NULL,
				0);
	      destroy_pending_input(p);
	  }
	  proc(item, NULL); /* assumes it's a PANEL_NOTIFY_PROC */
	  return;
	}
      }
      break;

    case MS_RIGHT:

      if (debug_level)
	fprintf(stderr, "event_y %d.  entry %d.\n", y, entry);

      while(entry-- && l) l = l->next;
      if(l && p->results_to) {
	result = p->results_to;
	if(p->path_prefix)
	  strcpy(result,remove_reg_exp(p->path_prefix));
	else
	  *result = 0;
	strcat(result,cleaned_for_input(l->str));
	if(event_is_down(event)) {
	  me = (Menu)xv_get(canvas, WIN_MENU);

	  xv_set(canvas,
		    XV_KEY_DATA, file_name_key, result,
		    0);
	  xv_set(canvas,
		    XV_KEY_DATA, list_elem_key, l,
		    0);
	  menu_show(me, canvas, event, 0);

	  return;
	}
      }
      break;

    case LOC_DRAG:
    case LOC_MOVE:
      ndis = rect->r_height/(fhi << 1);
      if(y < fhi) {	/* if it`s near the top */
	if(l->prev) {
	  l->active = FALSE;
	  while(ndis-- && l->prev) l = l->prev;
	  l->active = TRUE;
	  xv_set(canvas, WIN_MOUSE_XY, x, y+fhi, 0);
	  menu_redoit(canvas,NULL,NULL);
	  return;
	}
      } else {
	if((rect->r_height - y) < fhi) { /* if it's near the bottom */
	  l2 = l;
	  while(entry-- && l) l = l->next;
	  if(l && l->next) {
	    l2->active = FALSE;
	    while(ndis-- && l2->next) l2 = l2->next;
	    l2->active = TRUE;
	    xv_set(canvas, WIN_MOUSE_XY, x, y - fhi, 0);
	    menu_redoit(canvas,NULL,NULL);
	    return;
	  }
	}
      }
    default:
      return;
    }
  }
}

/*********************************************************************/
void 
m_play_file(canvas, name, le)
     Canvas canvas;
     char *name;
     Menu_list *le;
{
  char play_command[125];

  name = (char *) xv_get(canvas, XV_KEY_DATA, file_name_key);
  (void) sprintf(play_command, "%s -r1:", play_program);
  (void) call_external_play_prog(play_command, name, "", 0, 0, NULL);
  return;
}

/*********************************************************************/
void 
m_print_file(canvas, name, le)
     Canvas canvas;
     char *name;
     Menu_list *le;
{
  Frame fr;
  Textsw  help;
  char head[200];
  register int n;
  char command[500];
  char label[500];
  Textsw espsout;
  int ret;

  name = (char *) xv_get(canvas, XV_KEY_DATA, file_name_key);

  if(name && *name) {
    if(*name == '@') name++;
    n = strlen(name);
    
    if(name[n-1] == '/') m_pass_in_name(canvas, name, NULL);
    else {
      if (is_esps_file(name)) { /* is an ESPS file (or NIST) */
	
	run_esps_prog("xtext epsps -l",name,"",0,0,NULL);
	return;
      }
      else { /* not an ESPS file, just pop up in text window*/
	sprintf(head,"File: %s",name);
	fr = xv_create(XV_NULL,FRAME,
			   XV_LABEL, head,
			   0);
	if(!window_check_return(fr)) return;
	
	help = xv_create(fr, TEXTSW,
			     TEXTSW_FILE, name,
			     0);
	if(!window_check_return(help)) {
	  dt_xv_destroy_safe(2,fr);
	  return;
	}
	
	xv_set(fr,WIN_SHOW, TRUE,
		   FRAME_NO_CONFIRM, TRUE,
		   0);
      }
    }
    return;
  }
}

/*********************************************************************/
void 
m_kill_display(canvas, name, le)
     Canvas canvas;
     char *name;
     Menu_list *le;
{
  Pending_input *p;
  
  if((p = (Pending_input*)xv_get(canvas,WIN_CLIENT_DATA))) {
    p->canvas = (Canvas)0;
    reshuffle_and_kill(canvas);
  }

  return;
}


/*********************************************************************/
void 
m_pass_in_name(canvas, name, le)
     Canvas canvas;
     char *name;
     Menu_list *le;
{
  Pending_input *p;
  
  name = (char *) xv_get(canvas, XV_KEY_DATA, file_name_key);
  p = (Pending_input*)xv_get(canvas,WIN_CLIENT_DATA);

  if(p && p->item && p->proc && p->results_to) {
    strcpy(p->results_to,name);
    xv_set(p->item, PANEL_VALUE, name, 0);
    p->proc(p->item,name);
  }
}

/*********************************************************************/
void 
m_remove_list_element(canvas, name, le)
     Canvas canvas;
     char *name;
     Menu_list *le;
{
  char *cp;
  Menu_list *l;
  Pending_input *p;
  
  if ((l = (Menu_list *) xv_get(canvas, XV_KEY_DATA, list_elem_key))
      && (p = (Pending_input*)xv_get(canvas,WIN_CLIENT_DATA))) {
    if(l->active) {
      if(l->prev) l->prev->active = TRUE;
      else
	if(l->next) l->next->active = TRUE;
    }
    if(l == (Menu_list*)p->list) {
      if(! l->next) {
	if(p->destroy_on_select)
	{
	    xv_set(canvas,	WIN_CLIENT_DATA,	(caddr_t) NULL,
				0);
	    destroy_pending_input(p);
	}
	return;
      }
      p->list = (caddr_t)l->next;
      l->next->prev = NULL;
    } else {
      if(l->next) l->next->prev = l->prev;
      if(l->prev) l->prev->next = l->next;
    }
    if(l->str) free(l->str);
    if(l->data) free(l->data);
    free(l);
    menu_redoit(canvas, NULL, NULL);
    return;
  }
/*!*//* What's the point? */
  l = l->next;
}

/*********************************************************************/
void 
m_designate_source(canvas, name, le)
     Canvas canvas;
     char *name;
     Menu_list *le;
{
  char *cp;
  Menu_list *l;
  Pending_input *p, *get_designated_source();

  le = (Menu_list *) xv_get(canvas, XV_KEY_DATA, list_elem_key);

  /* First, see if another designee should be turned off. */
  if((p = get_designated_source(&l,NULL))) {
    l->tapped = FALSE;
    if(p->canvas)
      menu_redoit(p->canvas,NULL,NULL);
  }
  
  if ((l = le)
      && (p = (Pending_input*)xv_get(canvas,WIN_CLIENT_DATA))) {
    l->tapped = TRUE;
    menu_redoit(canvas, NULL, NULL);
    return;
  }
}

/*********************************************************************/
void 
show_output_proc(item, event)
     Panel_item item;
     Event *event;
{
  extern Panel_item outputFile_item, newFile_item;
  extern Pending_input new_files;
  extern void newText_proc();
  
  if(new_files.list && (item == outputFile_item)) {
    switch(event_id(event)) {
    case MS_RIGHT:
      if(event_is_down(event)) {
	if(!new_files.canvas) {
	  if((new_files.canvas = file_selection_window(&new_files))) {
	    new_files.item = newFile_item;
	    new_files.proc = newText_proc;
	  } else
	    show_notice(1,"Problems creating output file browser window");
	}
      }
      return;
    default:
      panel_default_handle_event(item,event);
      return;
    }
  } else
    panel_default_handle_event(item,event);
  return;
}

/*********************************************************************/
free_menu_list(l)
     Menu_list *l;
{
  Menu_list *l2;
  
  while((l2 = l)) {
    if(l->str) free(l->str);
    if(l->data) free(l->data);
    l = l->next;
    free(l2);
  }
}
