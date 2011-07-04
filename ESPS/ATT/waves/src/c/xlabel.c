/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc.
 *    "Copyright (c) 1990-1995  Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  David Talkin
 * Checked by:
 * Revised by:  Alan Parker for ERL
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)xlabel.c	1.42 2/2/98 ERL";

/* xlabel.c */
/* a waveform labeler to be used with programs like "xwaves" */
/* (designed to be used as an "attachment") */

#ifdef hp400
#include <string.h>
#endif
#ifndef hpux
#include <sys/param.h>
#else
#define MAXPATHLEN 1024
#endif
#include <Objects.h>
#include <wave_colors.h>
#include <labels.h>
#include <charts.h>
#include <xview/font.h>
#include <xview/notice.h>
#include <esps/esps.h>
#include <esps/exview.h>
#include <esps/epaths.h>
#define _NO_PROTO
#include <xprint_util.h>

#define DELETE '\177'
#define DEBUG(x) if(debug_level > x) fprintf

#define SYNTAX USAGE("xlabel [-w wave_pro] [-n<waves or other host program>] [-c<registry name of host>] ");

static char *wave_pro = ".wave_pro";	/* .wave_pro file name from command line */

Event pseudo_event;
int    debug_level = 0;
int    command_paused = 0, da_location = 0;

Canvas global_canvas;	/* used by meth_print routine */

/* from globals.c */
extern	int	 print_graphic_printer;
extern	int	 print_graphic_resolution;
extern	char	 print_graphic_orientation[];
extern	char	 print_graphic_type[];
extern	char	 print_graphic_file[];
extern	char	 print_graphic_command[];
extern	double	 print_graphic_scale;
extern  char     *Version;
extern char    *registry_name;

char *meth_print(), *basename();
Label *get_nearest_label();
Labelfile *get_labelfile();
Pixfont  *xv_pf_open();
static void doit(), repaint(), resize();
static Notify_value iocatcher();

extern Visual *visual_ptr;

/*********************************************************************/
Panel_item temp_item, file_item, object_item, active_item, menu_item,
  chart_item, top_word_item;

void quit_proc(), newFile();

Frame daddy = XV_NULL;	/* global reference in xprint_setup.c and xnotice.c */
Panel panel;

/* These globals (in globals.c) are a means of control via the .wave_pro */

int xlab_ctlwin_y = -1;		/* initial Y pos of xlabel control window */
int xlab_ctlwin_x = -1;		/* initial X pos of xlabel control window */


extern char inputname[], objectname[];
extern u_char red[], blue[], green[];
static int  wind_height = 80;	/* Vertical space allocated for each
				   label file. */

/* Fudge factors to allow user control of frame offsets to compensate
for unmanageable window manager decorations. */
static int xlabel_frame_voff = 33, xlabel_frame_hoff = 0;

static int insert_mode = 1; /* 0==REPLACE; 1==INSERT; 2==MOVE */
int replace_field = 1;

/*global declarations for default font*/
Xv_Font def_font = XV_NULL;
int     def_font_height, def_font_width;

int	use_dsp32;              /* referred to in globals.c */
double	image_clip = 7.0, image_range = 40.0;

char        *Ok="ok", *Null="null";
char fontfile[MAXPATHLEN]= "";
char  active[200] = "1";
char menufile[MAXPATHLEN] = "labelmenu.def";
char chartname[MAXPATHLEN] = "";
char topword[NAMELEN] = "";
char chartmenu[MAXPATHLEN] = "chartmenu";
char *host = "waves", *thisprog = "xlabel";

#if !defined(hpux) && !defined(DS3100) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
char *malloc();
#endif
int	exv_get_help();
void remove_cursor();
void add_cursor();
void label_menu_proc(), done_proc();
Notify_value kill_signal_view();
Objects *program;
static Selector
  g70 = {"xlabel_ctlwin_y", "%d", (char *) &xlab_ctlwin_y, NULL},
  g69 = {"xlabel_ctlwin_x", "%d", (char *) &xlab_ctlwin_x, &g70},
  gg0 = {"xlabel_label_height", "%d", (char*)&wind_height, &g69},
  gg1 = {"xlabel_menufile", "#qstr", menufile, &gg0},
  gg2c = {"xlabel_frame_voff", "%d", (char*) &xlabel_frame_voff, &gg1},
  gg2b = {"xlabel_frame_hoff", "%d", (char*) &xlabel_frame_hoff, &gg2c},
  gg2 = {"xlabel_fields", "#strq", active, &gg2b},
  gg3b = {"xlabel_name", "#qstr", objectname, &gg2},
  gg3d = {"xlabel_font", "#qstr", fontfile, &gg3b},
  gg3 = {"xlabel_insert_mode", "%d", (char*)&insert_mode, &gg3d},
  gg4c = {"xchart_top_word", "#qstr", topword, &gg3},
  gg4b = {"xchart_chartfile", "#qstr", chartname, &gg4c},
  gg4a = {"xchart_chartmenu", "#qstr", chartmenu, &gg4b},
  gg4 = {"xlabel_labelfile", "#qstr", inputname, &gg4a},
  g0 = {"label_height", "%d", (char*)&wind_height, &gg4},
  g1 = {"menufile", "#qstr", menufile, &g0},
  g2b = {"active_fields", "#strq", active, &g1},
  g2 = {"fields", "#strq", active, &g2b},
  g3c = {"insert_mode", "%d", (char*)&insert_mode, &g2},
  g3b = {"object", "#qstr", objectname, &g3c},
  g3d = {"name", "#qstr", objectname, &g3b},
  g3 = {"font", "#qstr", fontfile, &g3d},
  g4c = {"top_word", "#qstr", topword, &g3},
  g4b = {"chartfile", "#qstr", chartname, &g4c},
  g4a = {"chartmenu", "#qstr", chartmenu, &g4b},
  g4 = {"labelfile", "#qstr", inputname, &g4a};

  static double m_time, sec_cm, start, rend, rstart;
  static int color, width, height, loc_x, loc_y;
  static char signame[NAMELEN];
  static int type = 0, nfields = 1;
  static char sep;
  static char comment[500];
  static Selector
  a12 = {"signal", "#qstr", signame, &g4},
  la1 = {"separator", "%c", &sep, &a12},
  la2 = {"type", "%d", (char*)&type, &la1},
  la3b = {"nfields", "%d", (char*)&nfields, &la2},
  la6 = {"comment", "#strq", comment, &la3b},
  a11 = {"rend", "%lf", (char*)&rend, &la6},
  a10 = {"rstart", "%lf", (char*)&rstart, &a11},
  a9 = {"sec/cm", "%lf", (char*)&sec_cm, &a10},
  a8 = {"start", "%lf", (char*)&start, &a9},
  a7 = {"width", "%d", (char*)&width, &a8},
  a6 = {"height", "%d", (char*)&height, &a7},
  a5 = {"loc_y", "%d", (char*)&loc_y, &a6},
  a4 = {"loc_x", "%d", (char*)&loc_x, &a5},
  a3 = {"color", "%d", (char*)&color, &a4},
  a2 = {"time", "%lf", (char*)&m_time, &a3},
  a0 = {"file", "#qstr", inputname, &a2};

/*********************************************************************/
Pixfont*
get_default_label_font()
{
  Pixfont *xv_pf_default();

  return(xv_pf_default());
}

/*********************************************************************/
Pixfont*
open_label_font(name)
     char *name;
{
  return(xv_pf_open(name));
}

/*********************************************************************/
window_wash(handle)
     caddr_t handle;
{
  if(handle) return;
  fprintf(stderr,"Xview can't create a window; sorry...\n");
  exit(-1);
}

/*********************************************************************/
Menu make_menu(menufile, nitems, proc)
     void (*proc)();
     char *menufile;
     int *nitems;
{
  static char text[MES_BUF_SIZE];
  Menu menu;
  int i, nargs;
  char item[200], str[NAMELEN], *it, *st;
  FILE *fp, *fopen();
  Pixfont *pf = NULL;
  static int rows, cols;
  static Selector a3 = {"font", "#qstr", fontfile, NULL},
  a2 = {"rows", "%d", (char*)&rows, &a3},
  a1 = {"columns", "%d", (char*)&cols, &a2};

  if((fp = fopen(menufile,"r")) && fgets(text,MES_BUF_SIZE,fp)) {
    Pixfont *xv_pf_open(), *xv_pf_default();

    rows = cols = -1;
    *nitems = 0;

    nargs = get_args(text,&a1);	/* ASSUMES ALL ATTRIBUTES ON FIRST LINE! */

    if(*fontfile) {
      if(!(pf = xv_pf_open(fontfile)))
      {
	sprintf(notice_msg,
		"make_menu(): Can't open fontfile %s; using default.",
		fontfile);
  	show_notice(0, notice_msg);
      }
    }
    if(!pf)
      pf = xv_pf_default();

    menu = xv_create(XV_NULL, MENU,
		     XV_FONT, pf,
		     MENU_NOTIFY_PROC, proc,
		     MENU_DONE_PROC, done_proc,
		     0);
    window_wash(menu);

    do {
      if(!nargs) { /* !nargs means first row was menu data (not col/row spec.) */
	sscanf(text,"%s%s",item,str);
	if(!((it = malloc(strlen(item)+1)) && (st = malloc(strlen(str)+1)))) {
	  show_notice(1, "Can't allocate mem for menu strings.");
	  return(FALSE);
	}
	strcpy(it,item);
	strcpy(st,str);
	xv_set(menu, XV_FONT, pf, MENU_STRING_ITEM, it, st, 0, 0);
	/* Knowing Sunview, this probably results in lots of little lost memory
	   fragments, even after menu_destroy()... i.e. the it's and str's
	   need to be freed!  What a crock. */
	(*nitems)++;
      }
      nargs = 0;
    } while(fgets(text,MES_BUF_SIZE,fp));
    fclose(fp);

    if(*nitems) {
      if(cols == -1) {
	if(rows > 0) {
		cols = *nitems/rows;
		if (*nitems%rows)
			cols++;
        }
	else cols = (int)sqrt((double)*nitems);
      }
      if(rows == -1)
	rows = *nitems/cols;
      while((cols * rows) < *nitems) rows++;
      xv_set(menu, MENU_NCOLS, cols, MENU_NROWS, rows, 0);
    }
    return(menu);
  }
  return(0);
}


/*********************************************************************/
make_labels_menu(lf)
     Labelfile *lf;
{
  char text[256];
  int i, j, nitems = 0, itsok = FALSE;
  int rows, cols;
  Menu mtmp = (Menu)0;

  if (!FIND_WAVES_MENU(menufile,menufile)) {
      (void) sprintf(notice_msg,
		     "xlabel: couldn't find menufile %s.", menufile);
      show_notice(1, notice_msg);
      return(FALSE);
  }

  if((mtmp = make_menu(menufile,&nitems,label_menu_proc))) {
    itsok = TRUE;
    if(lf->menu)
      menu_destroy(lf->menu);
    lf->menu = mtmp;
  } else
    return(FALSE);

  /* Insure that menu font matches the labelfile font.  If both were specified,
     menu font overrides. */
  if(*fontfile && strcmp(fontfile,lf->fontfile)) {
    Pixfont *xv_pf_open();

    strcpy(lf->fontfile,fontfile);
    lf->font = xv_pf_open(fontfile);
    plot_labels(lf);
  } else			/* make menu look like label file */
    if((! *fontfile) && *(lf->fontfile))
      xv_set(lf->menu,XV_FONT,lf->font,0);

  if(!nitems) {			/* make a default (probably useless) menu */
    itsok = FALSE;
    rows = 8;
    cols = 8;
    for(i = 0, j = 65; i < 127;) {
      text[i] = j++;
      text[i+1] = 0;
      xv_set(lf->menu, MENU_STRING_ITEM, &text[i], &text[i],0,0);
      i += 2;
    }
    xv_set(lf->menu, MENU_NCOLS, cols, MENU_NROWS, rows, 0);
  }
  return(itsok);
}

/*********************************************************************/
time_to_x(ob,time)
     Objects *ob;
     double time;
{
  return((int)(.5 + (PIX_PER_CM * ((time - ob->start)/ob->sec_cm))) +
	 ob->x_off);
}

/*********************************************************************/
double
x_to_time(ob,x)
     Objects *ob;
     int x;
{
  x -= ob->x_off;
  if(x < 0) x = 0;
  return(ob->start + (double)x * ob->sec_cm / PIX_PER_CM);
}

/*********************************************************************/
char *exec_waves(str)
     char *str;
{
  extern char *dispatch();
  return(dispatch(str));
}

/*********************************************************************/
char *
get_receiver_name(ob)
     Objects *ob;
{
  return(ob->name);
}

/*********************************************************************/
char *
get_methods(ob)
     Objects *ob;
{
  extern Methods base_methods;

  if(ob) return((char*)(ob->methods));

  return((char*)&base_methods);
}

/*********************************************************************/
char *
get_receiver(str)
     char *str;
{
  Objects *ob;
  static  char name[NAMELEN];
  extern Objects *objlist;

  ob = objlist;
  if(str && strlen(str)) {
    sscanf(str,"%s",name);
    while(ob) {
      if(ob->name &&
	 (! strcmp(ob->name, name))) {
	   return((char*)ob);
	 }
      ob = ob->next;
    }
  }
  return(NULL);
}

Notify_value
destroy_func(client, status)
Notify_client client;
Destroy_status status;
{

    DEBUG(1) (stderr,"Inside of destroy_func.\n");
    if (status == DESTROY_CHECKING) {
	}
    else  if (status == DESTROY_CLEANUP) {
    DEBUG(1) (stderr,"Inside of destroy_func. CLEANUP\n");
	quit_proc();
        return notify_next_destroy_func(client, status);
	}
    else if (status == DESTROY_SAVE_YOURSELF) {
	}
    else {
    DEBUG(1) (stderr,"Inside of destroy_func. DEATH\n");
	quit_proc();
	}
	
    return NOTIFY_DONE;
}

extern int  optind;		/* for use of getopt() */
extern char *optarg;		/* for use of getopt() */
extern int fullscreendebug;



/*********************************************************************/
char *generate_startup_command(registry)
  char *registry;
{
  static char com[MES_BUF_SIZE];

  sprintf(com,"add_op name %s op #send function %s registry %s command _name mark signal _file time _cursor_time",
	   basename(thisprog), thisprog, registry);
  return(com);
}

static char env_str[MAXPATHLEN+20];
/*********************************************************************/
/*********************************************************************/
main(ac, av)
     int ac;
     char **av;
{
  Frame frame;
  extern Objects *objlist, *new_objects();
  extern Methods base_methods;
  extern int attachment;
  char mess[MES_BUF_SIZE];
  int i, locx, locy;
  extern int optind;
  extern char *optarg;
  int ch;
  char *server_name = "xwaves";
  extern Display *X_Display;
  extern Window   comm_window;
  extern void identify_visual();


  fullscreendebug = 1; /* this global inhibits server grabs that cause
			  problems on the SGI */
  thisprog = av[0];

  while ((ch = getopt(ac, av, "w:n:c:")) != EOF)
    switch (ch)
      {
      case 'n':
	host = optarg;
	break;
      case 'w':
	wave_pro = optarg;
	break;
      case 'c':
	server_name = optarg;
	break;
      default:
	SYNTAX
	  if(debug_level)
	    for(ch = 0; ch < ac; ch++)
	      fprintf(stderr,"%s ",av[ch]);
	fprintf(stderr,"\n");
	exit(-1);
      }

  attachment = TRUE;		/* make cmap create static colormap segment */
  event_init(&pseudo_event);
  event_id(&pseudo_event) = MS_RIGHT;

  def_font = (Xv_Font) xv_find(XV_NULL, FONT,
			FONT_FAMILY,	FONT_FAMILY_DEFAULT_FIXEDWIDTH,
			0);
  def_font_width = (int) xv_get(def_font, FONT_DEFAULT_CHAR_WIDTH);
  def_font_height = (int) xv_get(def_font, FONT_DEFAULT_CHAR_HEIGHT);

  get_all_globals(wave_pro, &g4);
  sprintf(env_str,"XPPATH=%s/lib/Xp",get_esps_base(NULL));
  putenv(env_str);

  get_color_depth();
  setup_colormap();
  *inputname = 0;	/* Don't get this directly from .wave_pro */
  program = objlist = new_objects(av[0]);
  objlist->methods = &base_methods;
  color = MARKER_COLOR;
  locx = (xlab_ctlwin_x >= 0) ? xlab_ctlwin_x : 700;
  locy = (xlab_ctlwin_y >= 0) ? xlab_ctlwin_y : 0;

  daddy = 	/* global reference in xprint_setup.c and xnotice.c */
      frame = xv_create(NULL, FRAME,
		        XV_X, locx,
		        XV_Y, locy,
		        XV_WIDTH, 400,
			0);

  /* set up communications with the host (xwaves) */
  if (!setup_attach_comm(frame, server_name, "xlabel")) {
      fprintf(stderr, "Failed to setup ipc communications\n");
      exit(1);
  }
  if (is_chart())
     sprintf(mess, "Chart Display and Labeler %s (%s)", Version, registry_name);
  else
     sprintf(mess, "Labeler %s (%s)", Version, registry_name);
  xv_set(frame, XV_LABEL, mess, NULL);

  if (debug_level)
      fprintf(stderr, "registry name: %s\n", registry_name);

  send_start_command(generate_startup_command(registry_name));

  set_blowup_op();

  window_wash(frame);
  notify_interpose_destroy_func(frame, destroy_func);

  panel = xv_create(frame, PANEL, 0);
  window_wash(panel );

  file_item = xv_create(panel, PANEL_TEXT,
				PANEL_LABEL_STRING, "Label File:",
				PANEL_VALUE, inputname,
				PANEL_VALUE_DISPLAY_LENGTH, 30,
				PANEL_NOTIFY_PROC, newFile,
				   0);
  window_wash(file_item );

    object_item = xv_create(panel, PANEL_TEXT,
				PANEL_LABEL_STRING, "Object:",
				PANEL_VALUE, objectname,
				PANEL_VALUE_DISPLAY_LENGTH, 8,
			  PANEL_VALUE_STORED_LENGTH, 80,
				PANEL_NOTIFY_PROC, newFile,
				   0);
    window_wash(object_item );
  if(child_of_ensig()) {
                 xv_set(object_item, PANEL_INACTIVE, 1, NULL);
  }

  if(!is_chart()) {
    temp_item = xv_create(panel, PANEL_BUTTON,
			PANEL_LABEL_STRING,	    "xlabel manual",
		        XV_KEY_DATA,
		         EXVK_HELP_NAME,	    FIND_WAVES_LIB(NULL,
							   "xlabel.man"),
			XV_KEY_DATA,
			 EXVK_HELP_TITLE,    "xlabel manual page",
			PANEL_NOTIFY_PROC,	    exv_get_help,
			0);
    window_wash(temp_item);
  }

  active_item = xv_create(panel, PANEL_TEXT,
				PANEL_LABEL_STRING, "Active fields:",
				PANEL_VALUE, active,
				PANEL_VALUE_DISPLAY_LENGTH, 27,
			  PANEL_VALUE_STORED_LENGTH, 80,
				PANEL_NOTIFY_PROC, newFile,
				   0);
  window_wash(active_item);

  if(is_chart()) {
    temp_item = xv_create(panel, PANEL_BUTTON,
			  PANEL_LABEL_STRING,	    "xchart manual",
			  XV_KEY_DATA,
			  EXVK_HELP_NAME,	    FIND_WAVES_LIB(NULL,
							   "xchart.man"),
			  XV_KEY_DATA,
			  EXVK_HELP_TITLE,    "xchart manual page",
			  PANEL_NOTIFY_PROC,	    exv_get_help,
			  0);
    window_wash(temp_item);
  }

  menu_item = xv_create(panel, PANEL_TEXT,
				PANEL_LABEL_STRING, "Label Menu File:",
				PANEL_VALUE, menufile,
				PANEL_VALUE_DISPLAY_LENGTH, 20,
			  PANEL_VALUE_STORED_LENGTH, 80,
				PANEL_NOTIFY_PROC, newFile,
				   0);
  window_wash(menu_item );

  temp_item = xv_create(panel, PANEL_BUTTON,
				PANEL_LABEL_STRING, "  QUIT  ",
				PANEL_NOTIFY_PROC, quit_proc,
				0);
  window_wash(temp_item );

  if(is_chart()) {
    chart_item = xv_create(panel, PANEL_TEXT,
				   PANEL_LABEL_STRING, "Chart File:",
				   PANEL_VALUE, chartname,
				   PANEL_VALUE_DISPLAY_LENGTH, 30,
				   PANEL_NOTIFY_PROC, newFile,
				   0);
    window_wash(chart_item);
    top_word_item = xv_create(panel, PANEL_TEXT,
				   PANEL_LABEL_STRING, "Top Word:",
				   PANEL_VALUE, topword,
				   PANEL_VALUE_DISPLAY_LENGTH, 30,
			  PANEL_VALUE_STORED_LENGTH, 80,
				   PANEL_NOTIFY_PROC, newFile,
				   0);
    window_wash(top_word_item);
  }

  if (is_chart())
  	exv_attach_icon(frame, ERL_NOBORD_ICON, "xchart", TRANSPARENT);
  else
    exv_attach_icon(frame, ERL_NOBORD_ICON, "xlabel", TRANSPARENT);

  identify_visual();
  window_fit(panel);
  window_fit(frame);

  window_main_loop(frame);

}


/*********************************************************************/
set_blowup_op()
{
  char mess[MES_BUF_SIZE];

  sprintf(mess,"set blowup_op %s",basename(thisprog));
  mess_write(mess);
}

/*********************************************************************/
Labelfile *
get_label_level(ob,y)
     Objects *ob;
     register int y;
{
  Rect *rec;
  register int n, at, yoffset, height;
  register Labelfile *lf;
  Wobject *wob;

  if(ob && (lf = ob->labels)) {
    n=1;
    while ((lf = lf->next)) n++;
    rec = (Rect*)xv_get(ob->canvas, WIN_RECT);
    if(plotting_charts(wob = (Wobject*)get_wobject(ob))) {
      yoffset = (wob->label_loc)? rec->r_height - wob->label_size : 0;
      height = wob->label_size;
    } else {
      yoffset = 0;
      height = rec->r_height;
    }
    if((y < yoffset) || (y > yoffset+height)) return(NULL);
    if(n==1) return(ob->labels);
    at = (n * (y-yoffset))/height;
    if(at == n) at--;
    lf = ob->labels;
    while(lf && at--) lf = lf->next;
    return(lf);
  }
  return(NULL);
}

/*********************************************************************/
static void
doit(canvas_pw, event, arg)
     Canvas canvas_pw;
     Event *event;
     caddr_t arg;
{
  /* Event proc called on point window, not canvas itself. */
  Canvas    canvas = xv_get(canvas_pw, CANVAS_PAINT_CANVAS_WINDOW);

  Objects *ob;
  Wobject *wob;
  Labelfile *lf;
  Label *l;
  static double time, time0 = 0.0, ptime, ntime;
  double get_next_time(), get_prev_time();
  static int x, y;
  int id;
  Pixwin *pw;
  Rect *rec;
  Frame frm;
  char mes[MES_BUF_SIZE];
  void mark_window();

  if(canvas && (ob = (Objects *)xv_get(canvas, WIN_CLIENT_DATA))) {
    wob = (Wobject*)get_wobject(ob);
    x = event_x(event);
    y = event_y(event);
    time = x_to_time(ob,x);
    id = event_id(event);
    global_canvas = canvas;

    if(event_is_up(event)) { /* a button was released... */
      Label *l;
      switch(event_id(event)) {
      case MS_MIDDLE:
	if(((insert_mode == 2) || event_shift_is_down(event) ||
	    event_meta_is_down(event))) {
	  lf = ob->labels;
	  remove_cursor(canvas, ob);
	  while(lf) {
	    if((l = lf->move_pending)) {
	      if(unlink_label(l,lf)) {
		waves_send(ob->name,"unmark",l->color,l->time);
		l->time = time;
		lf->move_pending = NULL;
		waves_send(ob->name, "mark", l->color, l->time);
		if(append_label(l,lf))
		  rewrite_labels(lf);
		else
		  fprintf(stderr,"Error reinserting label after move in doit()\n");
		plot_labels(lf);
	      } else
		fprintf(stderr,"Can't unlink label to be moved in doit()\n");
	    }
	    lf = lf->next;
	  }
	  add_cursor(canvas, ob, x, y);
	}
	return;
      case MS_LEFT:
	return;
      }
    }

    switch(id) {
    case MS_LEFT:
      if(lf = get_label_level(ob,y)) {
	if(event_ctrl_is_down(event)) {
	  lf->time_t = time;
	  do_labels(event,lf,"*EDIT*");
	  return;
	}
	ptime = get_prev_time(lf, time);
	ntime = get_next_time(lf, time);
	if(ntime > 0.0) {
	  mark_window(ob, ptime, ntime);
	  send_play_seg(ob,ptime,ntime);
	}
      } else
	play_chart_entry(wob,time,y);
      break;

    case LOC_MOVE:
    case LOC_DRAG:
      win_set_kbd_focus(canvas_pw,xv_get(canvas_pw,XV_XID));
      {
	char mess[MES_BUF_SIZE], *iop;
	Chartfile *cf;

	pw = canvas_pixwin(ob->canvas);
	if((lf = get_label_level(ob,y)))
	  sprintf(mess,"%s",lf->label_name);
	else {
	  if((cf = (Chartfile*)get_chart_level(get_wobject(ob),y)))
	    sprintf(mess,"%s",cf->chart_name);
	  else
	    sprintf(mess,"%s","<no active file>");
	}

	switch(insert_mode) {
	case 0:
	  iop = "REPLACE MODE";
	  break;
	default:
	case 1:
	  iop = "INSERT MODE";
	  break;
	case 2:
	  iop = "MOVE MODE";
	  break;
	}
	sprintf(mes,"%30s  T:%9.5f  %12s",mess,time,iop);
	pw_text(pw,0,xv_get(ob->canvas, XV_HEIGHT) - 8,
		PIX_COLOR(TEXT_COLOR)|PIX_SRC,def_font,mes);
	send_cursor(ob->name,"cursor", time);
	remove_cursor(canvas, ob);
	add_cursor(canvas, ob, x, y);
      }
      break;
    case LOC_WINEXIT:		/*get rid of cursor*/
      remove_cursor(canvas, ob);
      break;
    case MS_MIDDLE:
      if((insert_mode != 2) && !event_shift_is_down(event) &&
	 !event_meta_is_down(event)) {
	if((lf = get_label_level(ob,y))) {
	  ptime = get_prev_time(lf, time);
	  mark_window(ob, ptime, time);
	  sprintf(mes,"%s play start %f end %f\n",ob->name,ptime,time);
	  mess_write(mes);
	}
      } else {			/* MOVE MODE */
	if(event_meta_is_down(event)) {	/* move boundaries in all tiers */
	  lf = ob->labels;
	  while(lf) {
	    if((l = get_nearest_label(lf,time)))
	      lf->move_pending = l;
	    lf = lf->next;
	  }
	} else			/* must be shift is down... */
	  if((lf = get_label_level(ob,y)) &&
	     (l = get_nearest_label(lf,time))) {
	    lf->move_pending = l;
	  }
      }
      break;
    case MS_RIGHT:
      {
	Chartfile *cf;
	if((lf = get_label_level(ob,y)))
	  do_label_menu(lf, time, y, lf->color, event, arg);
	else
	  if((cf = get_chart_level(get_wobject(ob),y)))
	    do_chart_menu(cf,event,NULL);
      }
      break;
    default:
      /* If ASCII event: get label level; find nearest label; if further
	 away than one pixel, create a new label at time; append characters to
	 last field; create new fields as delimiters are encountered; if id ==
	 DELETE, remove characters from end of last field; cross fields as
	 necessary; rewrite file when a RETURN is hit. */

      if(((id == DELETE) || ((id >= ASCII_FIRST) && (id <= META_LAST))) &&
	 event_is_down(event)) {

/* if( event_meta_is_down(event)) {
  fprintf(stderr,"meta:%c %c %d %d\n",id, id & 0x7f ,id, id & 0x7f);
}
*/
	if((lf = get_label_level(ob,y))) {
	  double dt = ob->sec_cm / PIX_PER_CM; /* min. resolvable time */
	  int field = (insert_mode == 0)? replace_field : -1;

	  if(id == '\r') {
	    rewrite_labels(lf);
	    return;
	  }
	  l = get_nearest_label(lf,time);
	  if(!l)
	    if(!(l = new_label(time, lf->color))) {
	      fprintf(stderr,"Can't make new_label in doit()\n");
	      return;
	    } else {
	      append_label(l,lf);
	      waves_send(ob->name,"mark",lf->color,time);
	    }
	  /* If in not in REPLACE mode, add a new label if cursor
	     is not exactly on the old mark. */
	  if(insert_mode != 0) {
	    if(fabs(l->time - time) >= dt) {
	      if(!(l = new_label(time, lf->color))) {
		fprintf(stderr,"Can't make new_label in doit()\n");
		return;
	      }
	      append_label(l,lf);
	      waves_send(ob->name,"mark",lf->color,time);
	    }
	  }
	  char_to_label(lf,l,id,field);
	  remove_cursor(canvas, ob);
	  plot_labels(lf);
	  add_cursor(canvas, ob, x, y);
	}
      }
      break;
    }
  }
  return;
}

/*********************************************************************/
void
quit_proc(item, event)
     Panel_item item;
     Event *event;
{
  cleanup();
  kill_proc();
}

/*********************************************************************/
waves_send(name,command,color,time)
     char *name, *command;
     int color;
     double time;
{
  char mes[MES_BUF_SIZE];

  sprintf(mes,"%s %s time %f color %d\n",name,command,time,color);
  mess_write(mes);
  return(TRUE);

}

/*********************************************************************/
send_cursor(name,command,time)
     char *name, *command;
     double time;
{
  char mes[MES_BUF_SIZE];

  sprintf(mes,"%s %s time %f\n",name,command,time);
  mess_write(mes);
  return(TRUE);

}

/*********************************************************************/
do_label_menu(lf, time, y, color, event, arg)
     Labelfile *lf;
     double time;
     int y, color;
     Event *event;
     caddr_t arg;
{
  Event *ep;

  if(!lf)
    return(FALSE);
  if(lf->menu) {
    if(!event) {
      event_x(&pseudo_event) = time_to_x(lf->obj,time);
      event_y(&pseudo_event) = y;
      ep = &pseudo_event;
    } else
      ep = event;
    lf->color_t = color;
    lf->time_t = time;
    remove_cursor(lf->obj->canvas,lf->obj);
    xv_set(lf->menu, MENU_CLIENT_DATA, lf, 0);
    menu_show(lf->menu,lf->obj->canvas,ep,NULL);
    return(TRUE);
  } else
    fprintf(stderr,"NULL lf->menu in do_label_menu()\n");
  return(FALSE);
}

static int evrx = 0, evry = 0;
#ifndef DEC_ALPHA
static int evrw = 0;
#else
static long evrw = 0;
#endif
	
/*********************************************************************/
/* This is run when a label menu item is selected. */
void
label_menu_proc(men, item)
    Menu	men;
    Menu_item	item;
{
  Labelfile *lf = (Labelfile*)xv_get(men, MENU_CLIENT_DATA);
  char *str;
  Event *evr;

  if (debug_level > 1)
    (void) fprintf(stderr, "entered label_menu_proc()\n");

  if((evr = (Event *) xv_get(men, MENU_FIRST_EVENT))){
    evrx = event_x(evr);
    evry = event_y(evr);
    evrw = event_window(evr);
  }

  if((str = (char*)xv_get(item,MENU_VALUE))) {
    Label *label, *la;
    Fields *fld;
    Objects *ob = lf->obj;
    char *c;
    Event *ev = (Event *) xv_get(men, MENU_FIRST_EVENT);

    if (debug_level > 1)
      (void) fprintf(stderr, "menu item = %s\n", str);

    if(!strcmp(str,"*SHELL*")) { /* Fork a process? */
      char command[MES_BUF_SIZE];
      if((la = get_nearest_label(lf,lf->time_t))) {
	c = (char*)xv_get(item, MENU_STRING);
	sprintf(command,"%s %s %s %f &\n",c,lf->label_name,la->fields->str,la->time);
	system(command);
      }
      return;
    }
    do_labels(ev, lf, str);
  }
}


/*********************************************************************/
do_labels(ev, lf, str)
     Event *ev;
     Labelfile *lf;
     char *str;
{
  Label *label, *la;
  Fields *fld;
  Objects *ob = lf->obj;
  char *c;
  int edit_string = FALSE;

  if(!strcmp(str,"*PRINT*")) { /* print graphic */
    meth_print(ob,NULL);
    return;
  }

  if(!strcmp(str,"*DEL*")) {	/* delete the label? */
    if((la = get_nearest_label(lf,lf->time_t)) &&
       remove_nearest_label(lf, lf->time_t, lf->color_t)) {
      redo_display(ob);
      waves_send(ob->name,"unmark",la->color,lf->time_t);
    }
    return;
  }

  if(!strcmp(str,"*UNLOAD*")) { /* remove the label file? */
    rewrite_labels(lf);
    destroy_labelfile(ob,lf,1);
    set_display_size(ob);
    redo_display(ob);
    return;
  }

  if(!strcmp(str,"*REPLACE*")) {
    insert_mode = 0;
    return;
  }

  if(!strcmp(str,"*INSERT*")) {
    insert_mode = 1;
    return;
  }

  if(!strcmp(str,"*MOVE*")) {
    insert_mode = 2;
    return;
  }

  if(!strcmp(str,"*EDIT*"))
    edit_string = TRUE;
  else
    edit_string = FALSE;


  /* else, it is a label to be applied */
  if(!edit_string && !event_shift_is_down(ev) && insert_mode) {
    if(! (label = new_label(lf->time_t, lf->color_t))) {
      fprintf(stderr,"Can't allocate another label structure!\n");
      return;
    }
    if(!append_label(label,lf)) {
      fprintf(stderr,"Error from append_label() in label_menu_proc()\n");
      return;
    }
    if(!(fld = (Fields*)new_field(NULL, 1, lf->color_t, 0))) {
      fprintf(stderr,"Can't get new_field() in label_menu_proc()\n");
      return;
    }
    label->fields = fld;
  }
  if(edit_string || event_shift_is_down(ev) ||
     (insert_mode == 0)) {	/* replace or edit nearest label */
    Fields *f;
    if(!(label = get_nearest_label(lf,lf->time_t)))
      return;			/* nothing to replace */
    fld = f = label->fields;
    while(f && (f->field_id != replace_field)) {
      fld = f;
      f = f->next;
    }
    if(!f) {			/* must create missing fields */
      if(!fld)
	label->fields = fld = (Fields*)new_field(NULL, 1, lf->color_t, 0);
      while(fld->field_id < replace_field) {
	fld->next = new_field(NULL,fld->field_id + 1, lf->color_t, 0);
	if(!(fld = fld->next)) {
	  fprintf(stderr,"Allocation failure in label_menu_proc\n");
	  return;
	}
      }
    } else
      fld = f;

    if(edit_string) {
      set_up_editor(ob,lf,label,fld);
      return;
    }

    if(fld->str) free(fld->str);
  }
  c = malloc(strlen(str)+1);
  strcpy(c,str);
  fld->str = c;
  lf->changed = TRUE;
  if(insert_mode == 1)
    if(!file_update(lf,label)) {
      fprintf(stderr,"Problems doing file_update() in label_menu_proc()\n");
      return;
    }
    else
      if(!rewrite_labels(lf)) { /* Must rewrite whole file. */
	fprintf(stderr,"Problems doing rewrite_labels() in label_menu_proc()\n");
	return;
      }
  plot_labels(lf);
  if(insert_mode && !event_shift_is_down(ev))
    waves_send(ob->name, "mark", lf->color_t, lf->time_t);
}

/*********************************************************************/
void
done_proc(men, result)
    Menu	men;
    Xv_opaque	result;
{
  if(evrx || evry)
    xv_set(evrw,
	   WIN_MOUSE_XY, evrx, evry, 0);
  evrx = evry = 0;
  evrw = NULL;
}

/*********************************************************************/
plot_labels(lf)
     Labelfile *lf;
{
  Pixwin *pw;
  Pixfont *pf;
  Rect *rec;
  Wobject *wob;
  Objects *ob;
  Labelfile *lf2;
  Label *label;
  Fields *f;
  double end;
  int x, y, n, top, bot, old_x[100], xl, yl, f_plotted;
  int ystep, ymin, ymax, yspace, width, lev, oldf, yoffset;
  char mes[MES_BUF_SIZE];

  if(!((ob = lf->obj) && (lf2 = ob->labels)))
    return(FALSE);
  n = 1;
  x = 1;
  while((lf2 = lf2->next)) {
    n++;
    if(lf2 == lf) x = n;
  }
  wob = get_wobject(ob);
  pw = canvas_pixwin(ob->canvas);
  rec = (Rect*)xv_get(ob->canvas, WIN_RECT);

  if(wob && wob->charts && wob->charts->first_word) {
    lf->height = wob->label_size/n;
    yoffset = (wob->label_loc)? rec->r_height - wob->label_size : 0;
  } else {
    lf->height = rec->r_height/n;
    yoffset = 0;
  }
  top = (x-1)*lf->height + yoffset;
  bot = top + lf->height + yoffset;
  end = ob->start + ((ob->sec_cm * rec->r_width) /PIX_PER_CM);

  if (debug_level > 1)
    (void) fprintf(stderr,
		   "plot_labels: n;%d x:%d height:%d top:%d bot:%d yoffset:%d\n",
		   n, x, lf->height,top,bot,yoffset);

    if(!doing_print_graphic)
      pw_rop(pw,0,top,rec->r_width,lf->height,
         PIX_SRC|PIX_COLOR(W_BACKGROUND_COLOR), NULL,0,W_BACKGROUND_COLOR);

  if(lf->next)
    pw_vector(pw, 0, bot-1, rec->r_width, bot-1, PIX_SRC, lf->color);

  if((label=lf->first_label)){
    if((pf = lf->font)) { /* set sizes */
      yspace = (int)xv_get(pf,FONT_DEFAULT_CHAR_HEIGHT)*0.2;
      ystep = ymin = (int)xv_get(pf,FONT_DEFAULT_CHAR_HEIGHT) + yspace;
      ymax = bot - ystep;
    }
    else {
      fprintf(stderr,"plot_labels: cannot get window font\n");
      return(FALSE);
    }
    width = (int)xv_get(pf,FONT_DEFAULT_CHAR_WIDTH);
    if (debug_level)
	(void) fprintf(stderr, "yspace = %d, ystep = %d, width = %d\n",
		       yspace, ystep, width);

    for(lev = 0; lev < 100; lev++) old_x[lev] = 0;
    lev = 0;
    y = top;
    while(label) {
      if((label->time >= ob->start) && (label->time <= end)) {
	xl = time_to_x(ob,label->time);
	f = label->fields;
	*mes = 0;
	x = xl - width;
	oldf = 1;
	f_plotted = FALSE;
	while(f) {
	  if(lf->active[f->field_id-1]) {
	    if(f->str) x -= (strlen(f->str) * width);
	    if(f_plotted)
	      while(f->field_id > oldf){/* place marks for blank active fields */
		if(lf->active[oldf-1]) {
		  x -= width;
		  strcat(mes,lf->separator);
		}
		oldf++;
	      }
	    if(f->str) {
	      strcat(mes,f->str);
	      f_plotted = TRUE;
	      oldf = f->field_id;
	    }
	  }
	  f = f->next;
	}
	y = top;
	lev = 0;
	while(old_x[lev] && (x <= old_x[lev])) {
	  lev++;
	  y += ystep;
	}
	x += width;
	if(y > ymax) y = ymax;
	old_x[lev] = xl;
	yl = y + ystep;
	pw_text(pw,x,yl - yspace,PIX_COLOR(TEXT_COLOR)|PIX_SRC,pf,mes);
	pw_vector(pw, xl, yl, x, yl, PIX_SRC, label->color);
	if(xl <= rec->r_width)
	  pw_vector(pw, xl, top, xl, yl, PIX_SRC, label->color);
      }
      label = label->next;
    }
  }
  return(TRUE);
}

/*********************************************************************/
redo_display(ob)
     Objects *ob;
{
  Label *l;
  Labelfile *lf;
  double end;
  Rect *rec;
  Pixwin *pw;
  int x, n;
  Wobject *w;
  Chartfile *cf;

  if(!(ob && ob_exists(ob))) return(FALSE);
  if(! ob->canvas) {
    fprintf(stderr,"Null canvas in redo_display()\n");
    return(FALSE);
  }

  ob->oldcursor_x = -1;
  ob->oldcursor_y = -1;

  w = get_wobject(ob);
  if(!(lf = ob->labels) && (!w || (w && !w->charts))) {
    rec = (Rect*)xv_get(ob->canvas, WIN_RECT);
    pw = canvas_pixwin(ob->canvas);
      pw_rop(pw,0,0,rec->r_width,rec->r_height,
	   PIX_SRC|PIX_COLOR(W_BACKGROUND_COLOR),NULL,0,W_BACKGROUND_COLOR);
    return(TRUE);
  }

  while(lf) {
    plot_labels(lf);
    lf = lf->next;
  }

  if(w) {
    cf = w->charts;
    while(cf) {
      plot_chart(cf);
      cf = cf->next;
    }
  }
  return(TRUE);
}

/*********************************************************************/
char *
meth_unload(ob, args)
     Objects *ob;
     char *args;
{
  static char file[NAMELEN],object[NAMELEN];
  static Selector a0 = {"file","%s",file,NULL},
                  a1 = {"name","%s",object,&a0};
  Labelfile *lf, *find_labelfile();
  extern Objects *objlist;

  *file = *object = 0;
  get_args(args,&a1);
  if((ob == program)) {
    if((! *object) && (objlist != program))
      strcpy(object,objlist->name);
    ob = (Objects*)get_receiver(object);
  }
  if(ob && (! *file) && ob->labels)
    strcpy(file,ob->labels->label_name);
  if(ob && (lf = find_labelfile(ob,file))) {
      rewrite_labels(lf);
      destroy_labelfile(ob,lf,1);
      set_display_size(ob);
      redo_display(ob);
      return(Ok);
    }
  return(Null);
}

/*********************************************************************/
char *
meth_kill(ob, args)
     Objects *ob;
     char *args;
{
  static char file[NAMELEN],object[NAMELEN], chart[NAMELEN];
  static Selector a0 = {"file","%s",file,NULL},
                  a1 = {"name","%s",object,&a0},
                  a2b = {"chartfile","%s",chart,&a1},
                  a2 = {"chart","%s",chart,&a2b};
  Labelfile *lf;
  Frame fr;
  Objects *o;
  int got, sendit;

  *file = 0;
  *object = 0;
  *chart = 0;
  sendit = 0;
  got = get_args(args,&a2);
  if(ob == program) {		/* Was this a message to the program? */
    if(!(got && (o = (Objects*)get_receiver(object))))
      return(Null);
    if(*file) { /* label file specified?*/
      if((lf = (Labelfile*)get_labelfile(o,file))) {
	rewrite_labels(lf);
	destroy_labelfile(o,lf,1);
	set_display_size(o);
	redo_display(o);
	
	xv_set(file_item, PANEL_VALUE, "", 0);
	*inputname = 0;
	return(Ok);
      } else return(Null);
    } else			/* chart file specified? */
      if(kill_chartfile(chart,(o = (Objects*)get_receiver(object))))
	redo_display(o);
    /* If neither label or chart was specified prepare to kill entire object */
      else {
	ob = o;
	sendit = 1;		/* send "unmark" commands to host */
      }
  } else {		/* The message was sent to a display object. */
    /* "file" will be the name of a signal file in this case */
    if(*file)	     /* label doesn't care which signames are displayed */
      return(Ok);  /*(assumes that something remains in display ensemble)*/
    sendit = 0;
  }
  /* Getting here implies that the message was sent to a display object
     either implicitly or explicitly.
     The whole display ensemble will be distroyed */
  if((fr = (Frame)xv_get(ob->canvas, XV_OWNER))) {
    Canvas    canvas = xv_get(ob->canvas, CANVAS_PAINT_CANVAS_WINDOW);
    save_shared_colormap(ob->canvas);
    xv_set(fr, WIN_CLIENT_DATA, NULL, 0);
    xv_set(ob->canvas, WIN_CLIENT_DATA, NULL, 0);
    if(canvas)
      xv_set(canvas, WIN_CLIENT_DATA, NULL, 0);
    kill_object(ob,sendit);
    xv_set(fr, FRAME_NO_CONFIRM, TRUE, 0);
    xv_destroy_safe(fr); /* causes kill_object in kill_signal_view */
    xv_set(file_item, PANEL_VALUE, "", 0);
    *inputname = 0;

    if(is_chart()) {
      xv_set(chart_item, PANEL_VALUE, "", 0);
      *chartname = 0;
    }

    xv_set(object_item, PANEL_VALUE, "", 0);
    *objectname = 0;
    return(Ok);
  }
  return(Null);
}

/*********************************************************************/
is_chart()
{
  extern Objects *program;
  return(!strcmp("xchart",basename(program->name)));
}

/*********************************************************************/
char *
meth_set(ob, args)
     Objects *ob;
     char *args;
{
  get_args(args, &g4);

  xv_set(object_item, PANEL_VALUE, objectname,0);
  if(*objectname)
    newFile(object_item,NULL);

  if(is_chart()) {

    xv_set(chart_item, PANEL_VALUE, chartname,0);
    newFile(chart_item,NULL);

    xv_set(top_word_item, PANEL_VALUE, topword,0);
    newFile(top_word_item,NULL);
  }

  xv_set(file_item, PANEL_VALUE, inputname,0);
  if(*inputname)
    newFile(file_item,NULL);

  xv_set(active_item, PANEL_VALUE, active,0);
  newFile(active_item, NULL);

  xv_set(menu_item, PANEL_VALUE, menufile,0);
  newFile(menu_item,NULL);

  return(Ok);
}

/*********************************************************************/
initial_setup()
{

  xv_set(file_item, PANEL_VALUE, inputname,0);

  xv_set(menu_item, PANEL_VALUE, menufile,0);

  xv_set(object_item, PANEL_VALUE, objectname,0);
  if(*objectname)
    newFile(object_item,NULL);

  if(is_chart()) {

    xv_set(chart_item, PANEL_VALUE, chartname,0);
    newFile(chart_item,NULL);

    xv_set(top_word_item, PANEL_VALUE, topword,0);
    newFile(top_word_item,NULL);
  }

  xv_set(active_item, PANEL_VALUE, active,0);
  newFile(active_item, NULL);

}

/*********************************************************************/
char *
meth_mark(ob, args)
     Objects *ob;
     char *args;
{
  static double time, rstart, rend;
  static int color;
  static char file[NAMELEN], label[MES_BUF_SIZE];
   static Selector
     a4 = {"rend", "%lf", (char*)&rend, NULL},
     a3 = {"rstart", "%lf", (char*)&rstart, &a4}, /* recognize & ignore */
     a2c = {"label", "#strq", label, &a3},
     a2b = {"replace_field", "%d", (char*)&replace_field, &a2c},
     a2 = {"time", "%lf", (char*)&time, &a2b },
     a1 = {"color", "%d", (char*)&color, &a2},
     a0b = {"signal", "%s", signame, &a1},
     a0 = {"file", "%s", file, &a0b};
  Labelfile *lf;

  if(ob_exists(ob)) {
    time = 0.0;
    *file = 0;
    *label = 0;
    color = -1;

    if(get_args(args, &a0) ) {
      if(!(lf = get_labelfile(ob,file))) lf = ob->labels;
      if((color < 0) && lf) color = lf->color;
      if(*label) {
	event_x(&pseudo_event) = time_to_x(lf->obj,time);
	event_y(&pseudo_event) = wind_height/2;
	lf->time_t = time;
	lf->color_t = color;
	do_labels(&pseudo_event, lf, label);
	return(Ok);
      } else
	if(do_label_menu(lf,time,0,color,NULL, 0)) {
	  return(Ok);
	}
    }
  }
     return(Null);
}

/*********************************************************************/
cleanup()
{
  extern Objects *program;
  Objects *obj;
  Labelfile *lf;
  int dumcol=1;
  double dumtime = 0.0;
  extern Objects *objlist;

  obj = objlist;
  while(obj) {
    if((lf = obj->labels)) {
      while(lf) {
	if(lf->first_label) rewrite_labels(lf);
	lf = lf->next;
      }
    }
    if(strcmp(obj->name,program->name))
      waves_send(obj->name,"unmark all t",dumcol,dumtime);
    obj = obj->next;
  }
}

/*********************************************************************/
char *
meth_quit(ob, args)
     Objects *ob;
     char *args;
{
  cleanup();
  terminate_communication(registry_name);
  exit(0);
}

/**********************************************************************/
get_decor_size(height, width)
     int *height, *width;
{
  static Rect  rect, *rec;
  Frame fr;

  if(panel && daddy) {
    frame_get_rect(daddy, &rect);
    rec = (Rect*)xv_get(panel, WIN_RECT);
    *height = rect.r_height - rec->r_height;
    *width = rect.r_width - rec->r_width;
    /* One sleazy hack deserves another... */
    if((*height < 0) || (*width < 0))
      *height = *width = 0;
    if(debug_level){
      fprintf(stderr,"decor: h:%d w:%d\n",*height, *width);
      fprintf(stderr,"fr.height:%d panel.height:%d\n",rect.r_height,rec->r_height);
    }
    return(TRUE);
  } else
    return(FALSE);
}


/**********************************************************************/
set_display_size(ob)
     Objects *ob;
{
  if(ob) {
    int height, width, nf, head;
    Wobject *w;
    Labelfile *lf;
    Frame fr;
    Rect	rect;

    lf = ob->labels;
    nf = 1;
    get_decor_size(&head, &width);
    while(lf && (lf = lf->next))nf++;
    if(plotting_charts(w = (Wobject*)get_wobject(ob)))
      height = (SCREEN_HEIGHT - ob->yloc - ob->height) - head;
    else
      height = (nf * wind_height);
    if(w)
      w->label_size = (nf * wind_height);
    if(debug_level)
      fprintf(stderr,
       "set_display_size: height=%d width=%d ob_height=%d ob_yloc=%d wind_height=%d N=%d dec_w:%d dec_y:%d\n",
              height,ob->width,ob->height,ob->yloc,wind_height,nf,width,head);

    if(height > 0) {
      ob->resize_status |= RESIZE_STATUS_FORCED_RESIZE;
      fr = (Frame)xv_get(ob->canvas,XV_OWNER);
      frame_get_rect(fr, &rect);
#ifdef SG
      rect_construct(&rect,
		     ob->xloc + (width/2) + xlabel_frame_hoff,
		     ob->yloc + ob->height + head + xlabel_frame_voff,
                     ob->width, height + head);
#else
      rect_construct(&rect,
	 ob->xloc + xlabel_frame_hoff, ob->yloc + ob->height + xlabel_frame_voff,
                     ob->width, height + head);
#endif
      frame_set_rect(fr, &rect);
    }
    return(TRUE);
  }
  return(FALSE);
}

/**********************************************************************/
initial_bad_values()
{
  m_time = start = sec_cm = -1.0;
  color = -1;
  width = height = loc_x = loc_y = -123;
  *signame = 0;
  type = 0;
  sep = ';';
  nfields = 1;
  *comment = 0;
}

/**********************************************************************/
install_signal_name(ob,name)
     Objects *ob;
     char *name;
{
  if(*name) {
    if(ob->signal_name) free(ob->signal_name);
    ob->signal_name = malloc(MAXPATHLEN);
    strcpy(ob->signal_name,name);
  }
}

/**********************************************************************/
char *
meth_make_object(ob,args)
     Objects *ob;
     char *args;
{
  Objects *ob2, *new_objects();
  Labelfile *lf;
  Frame frame;
  Canvas canvas;
  Rect rect;
  extern Objects *objlist, *new_objects();
  extern Methods base_methods;
  extern int cmap_depth;
  char mes[MES_BUF_SIZE];

  initial_bad_values();

  if(get_args(args,&a0) && *objectname) {
    if(debug_level)
      fprintf(stderr,"xlabel.meth_make_object(%s)\n",args);
    if(!(ob2 = (Objects *)get_receiver(objectname))) { /* does it already exist? */
      if((ob2 = new_objects(objectname))) { /* create one */

	frame = xv_create(XV_NULL, FRAME,
			  XV_VISUAL, visual_ptr,
			  XV_LABEL,		objectname,
			  XV_WIDTH,		ob2->width + 5,
			  XV_HEIGHT,		wind_height,
			  WIN_X,			ob2->xloc,
			  WIN_Y,			ob2->yloc + ob2->height,
			  XV_SHOW,		FALSE,
			  FRAME_INHERIT_COLORS,	FALSE,
			  WIN_CLIENT_DATA,    ob2,
			  0);

	window_wash(frame );

	exv_attach_icon(frame, ERL_NOBORD_ICON, objectname, TRANSPARENT);

	/* fill window code in pseudo_event */

	event_window(&pseudo_event) = (Xv_object) frame;

	canvas = xv_create(frame, CANVAS,
			   XV_X,		    0,
			   XV_Y,		    0,
			   CANVAS_RETAINED,    FALSE,
			   CANVAS_FIXED_IMAGE, FALSE,
			   CANVAS_AUTO_SHRINK, TRUE,
			   CANVAS_AUTO_EXPAND, TRUE,
			   CANVAS_CMS_REPAINT, FALSE,
			   CANVAS_PAINTWINDOW_ATTRS,
                              WIN_DEPTH,              cmap_depth,
			      XV_VISUAL,	      visual_ptr,
			      0,
			   OPENWIN_NO_MARGIN, TRUE,
			   XV_WIDTH,	    WIN_EXTEND_TO_EDGE,
			   XV_HEIGHT,	    WIN_EXTEND_TO_EDGE,
			   CANVAS_NO_CLIPPING,	TRUE,
			   WIN_CLIENT_DATA,    ob2,
			   0);

    xv_set(canvas_paint_window(canvas),
	   WIN_CONSUME_EVENTS,
	   LOC_DRAG,
	   WIN_IN_TRANSIT_EVENTS,
	   LOC_MOVE,
	   LOC_WINEXIT,
	   LOC_WINENTER,
	   WIN_ASCII_EVENTS,
	   0,
	   WIN_IGNORE_EVENTS,
	   KBD_USE,
	   KBD_DONE,
	   0,
	   WIN_EVENT_PROC, doit,
	   WIN_BIT_GRAVITY, ForgetGravity,
	   0);
	window_wash(canvas );
	window_fit(frame);

	notify_interpose_destroy_func(frame, kill_signal_view);
	cmap(canvas);
	ob2->next = objlist;
	objlist = ob2;
	ob2->canvas = canvas;
	/* When these were set in the xv_create above,
	   XViews would bomb calling repaint within xv_create
	   (in 1.0 beta at least). */
	xv_set(canvas, CANVAS_REPAINT_PROC, repaint,
	       CANVAS_RESIZE_PROC, resize, 0);
	window_fit(frame);
	xv_set(frame, XV_SHOW,TRUE, 0);
      } else {			/* couldn't allocate a new_object() */
	fprintf(stderr,
	 "meth_make_object: Allocation failure in new_object()\n.");
	return(Null);
      }
      update_object_panel(ob2);
    }  /* completed new Objects creation */

    if(!have_all_display_attributes(ob2))
      get_display_attributes(ob2);

    if(*inputname || *chartname) {

      install_signal_name(ob2,signame);

      set_display_size(ob2);

      initial_setup();

      if((lf = ob2->labels)) {
	if(*fontfile && strcmp(fontfile,lf->fontfile)) {
	  strcpy(lf->fontfile, fontfile);
	  lf->font = xv_pf_open(lf->fontfile);
	  xv_set(lf->menu,XV_FONT,lf->font,0);
	  if(lf->first_label)
	    plot_labels(lf);
	}
	if((! lf->first_label) && (color >= 0)) lf->color = color;
	if(m_time >= 0.0) {	/* was time specified in make command? */
	  if(color < 0) color = lf->color;
	  if(! do_label_menu(lf, m_time, 0, color, NULL, 0)) {
	    return(Null);
	  }
	}
      }
      return(Ok);
    }
  }
  return(Null);
}

/*********************************************************************/
update_menu_panel(menu)
     char *menu;
{


  xv_set(menu_item, PANEL_VALUE, menu, 0);
  strcpy(menufile,menu);

  return;
}

/*********************************************************************/
update_object_panel(ob)
     Objects *ob;
{


  xv_set(object_item, PANEL_VALUE, ob->name, 0);
  strcpy(objectname,ob->name);

  return;
}

/*********************************************************************/
update_file_panel(lf)
     Labelfile *lf;
{


  xv_set(file_item, PANEL_VALUE, lf->label_name, 0);
  strcpy(inputname,lf->label_name);

  return;
}

/*********************************************************************/
send_all_marks(lf)
     Labelfile *lf;
{
  Label *l;
  Objects *ob;

  if(lf && (ob = lf->obj)) {
    l = lf->first_label;
    while(l) {
      waves_send(ob->name,"mark",l->color,l->time);
      l = l->next;
    }
    return(TRUE);
  }
  return(FALSE);
}

char *
meth_print(ob,str)
     Objects *ob;
     char *str;
{
#ifdef HELL_FREEZES_OVER
    Display	*printer, *theDisp;
    XColor	*colorcell_defs;
    int		cells, i;
    int		theScreen;
    Colormap	theCmap;
    char	command[MAXPATHLEN];
    XpOrientation_t  graphic_orientation;

    static char	file[NAMELEN],object[NAMELEN], chart[NAMELEN];
    static Selector a0 = {"file","%s",file,NULL},
		    a1 = {"name","%s",object,&a0},
		    a2 = {"chart","%s",chart,&a1};

    Labelfile	*lf;
    Frame	fr;
    Objects	*o;
    int		got, sendit;


    *file = 0;
    *object = 0;
    *chart = 0;
    sendit = 0;
    got = get_args(str,&a2);
    if(ob == program) {		/* Was this a message to the program? */
	if(!(got && (o = (Objects*)get_receiver(object))))
	    return(Null);
	ob = o;
    }
    if(!ob) return(Null);

    printer = setup_xp_from_globals(ob->canvas);
    if (!printer)
	return Null;
    start_xp(printer, ob->canvas);

/* re-render image here */
    redo_display(ob);
/**/

    finish_xp(printer, ob->canvas);
#endif
    return(Ok);
}


/*********************************************************************/
char *
meth_print_EPS_temp(ob, str)
    Objects *ob;
    char    *str;
{
#ifdef HELL_FREEZES_OVER
    static char	    filename[NAMELEN];
    static int	    id;
    static Selector s1 = {"output", "#qstr", filename, NULL},
		    s0 = {"return_id", "%d", (char *) &id, &s1};
    Frame	    frame;
    int		    x, y;
    char	    reply[200];
    Display	    *printer;
    Rect	    rect;

    *filename = '\0';
    id = 0;
    get_args(str, &s0);

    if (ob && ob->canvas)
    {
	if (debug_level)
	    fprintf(stderr, "meth_print_EPS_temp(%s, \"%s\")\n",
		    (ob->name) ? ob->name : "<NULL>",
		    (str) ? str : "<NULL>");

	frame = xv_get(ob->canvas, XV_OWNER);

	if (*filename && id && frame)
	{
	    if (!xv_get(frame, FRAME_CLOSED))
	    {
		frame_get_rect(frame, &rect);
		x = rect.r_left;
		y = rect.r_top;

		sprintf(reply, "%s completion %d loc_x %d loc_y %d",
			host, id, x, y);

		printer =
		    setup_xp_EPS_temp(ob->canvas, filename, NULL, 0);
		if (!printer)
		    return;

		start_xp(printer, ob->canvas);
		redo_display(ob);
		finish_xp(printer, ob->canvas);

		mess_write(reply);
		return Ok;
	    }
	}
    }

    sprintf(reply, "%s completion %d", host, id);

    if (debug_level)
	fprintf(stderr, "meth_print_EPS_temp: Null return.\n");

    mess_write(reply);
    return Null;
#else
    return Ok;
#endif
}


/*********************************************************************/
char *
meth_redisplay(ob,str)
     Objects *ob;
     char *str;
{
  if(ob && ob_exists(ob) && str && ob->canvas) {
    initial_bad_values();

    if(get_args(str,&a0) &&
       have_all_display_attributes(ob)) {
      set_display_size(ob);
      redo_display(ob);
      return(Ok);
    } else
      get_display_attributes(ob);

  } else
    if(debug_level)
      fprintf(stderr,"Null object or arg list to meth_redisplay()\n");
  return(Null);
}

/*********************************************************************/
get_display_attributes(ob)
     Objects *ob;
{
  char mes[MES_BUF_SIZE];
  int n, id;

  if(ob && ob_exists(ob) && ob->name) {
    start = sec_cm = -1.0;
    width = height = loc_x = loc_y = -1;
    id = set_return_value_callback(meth_redisplay,ob);
    if(ob->signal_name && *(ob->signal_name)) {
      sprintf(mes,"%s get attributes display function %s file %s return_id %d\n",
	      ob->name, thisprog, ob->signal_name, id);
    } else {
      sprintf(mes,"%s get attributes display function %s return_id %d\n",ob->name,
	      thisprog, id);
    }
    mess_write(mes);
    return(TRUE);
  }
  return(FALSE);
}

/*********************************************************************/
have_all_display_attributes(ob)
     Objects *ob;
{
  if(ob) {
/*    if(!(ob->signal_name && *ob->signal_name))
      install_signal_name(ob,signame); */
    if((width > 0) &&
       (height > 0) &&
       (start != -1.0) &&
       (sec_cm > 0) &&
       (loc_x != -123) &&
       (loc_y != -123)) {
      ob->width = width;
      ob->height = height;
      ob->start = start;
      ob->sec_cm = sec_cm;
      ob->xloc = loc_x;
      ob->yloc = loc_y;
      if(debug_level)
	fprintf(stderr,
		"have_disp_attr: x:%d y:%d height:%d width:%d\n",
		loc_x,loc_y,height,width);
      return(TRUE);
    }
  }
  return(FALSE);
}

ob_exists(ob)
     Objects *ob;
{
  extern Objects *objlist;
  Objects *op = objlist;

  while(op) {
    if(op == ob)
      return(TRUE);
    op = op->next;
  }
  if(debug_level)
    fprintf(stderr,"Object no longer exists!\n");
  return(FALSE);
}

static void
repaint(canvas, pw, repaint_area)
     Canvas canvas;
     Pixwin *pw;
     Rectlist *repaint_area;
{
  Objects *ob = (Objects*)xv_get(canvas, WIN_CLIENT_DATA);

  if(ob && ob_exists(ob)) {
    Labelfile *lf = ob->labels;
    redo_display(ob);
    if(lf && (ob->resize_status & RESIZE_STATUS_RESIZE) &&
       !(ob->resize_status & RESIZE_STATUS_FORCED_RESIZE)) {
      wind_height = lf->height;
    }
    ob->resize_status = RESIZE_STATUS_RESET;
  }

  return;
}

/*********************************************************************/
static void
resize(canvas, width, height)
     Canvas canvas;
     int width, height;
{
  Objects *ob;

  if(canvas && (ob = (Objects *)xv_get(canvas, WIN_CLIENT_DATA)))
    ob->resize_status |= RESIZE_STATUS_RESIZE;
}

/*********************************************************************/
char *
meth_new_labelmenu(ob,str)
     Objects *ob;
     char *str;
{
  static Selector a1 = {"menufile", "%s", menufile, NULL};

  if(get_args(str,&a1)) {

    xv_set(menu_item, PANEL_VALUE, menufile, 0);
    newFile(menu_item,NULL);
    return(Ok);
  }
  return(Null);
}

/*********************************************************************/
char *
meth_set_active_fields(ob,str)
     Objects *ob;
     char *str;
{
  static Selector a1 = {"file", "%s", inputname, NULL},
  a2 = {"name", "%s", objectname, &a1},
  a3 = {"fields", "#strq", active, &a2};
  Labelfile *lf;

  if(ob_exists(ob) && get_args(str,&a3)) {

    xv_set(active_item, PANEL_VALUE, active, 0);
    if(ob == program)
      if(!(ob = (Objects*)get_receiver(objectname))) return(Null);
    if(*inputname)
      set_active_fields(ob->name,inputname,active);
    else
      if((lf = ob->labels))
	while(lf) {
	  set_active_fields(ob->name,lf->label_name,active);
	  lf = lf->next;
	}
    return(Ok);
  }
  return(Null);
}

/***********************************************************************/
char *
meth_return(ob,str)
     Objects *ob;
     char *str;
{
  if(str && *str) {
    int id;
    char *get_next_item();

    sscanf(str,"%d",&id);
    do_return_callback(id,get_next_item(str));
    return(Ok);
  }
  return(Null);
}

/*********************************************************************/
char *meth_save_globals(ob,str)
     Objects *ob;
     char *str;
{
  static char name[NAMELEN];
  static Selector s1 = {"output", "%s", name, NULL};

  *name = 0;
  get_args(str, &s1);

  if(dump_local_variables(name, &gg4))
    return(Ok);
  else
    return(Null);
}
	

/*********************************************************************/
/*********************************************************************/
Methods
meth7 = {"print_EPS_temp", meth_print_EPS_temp, NULL},
meth6 = {"print", meth_print, &meth7},
meth5 = {"kill", meth_kill, &meth6},
meth4 = {"activate", meth_set_active_fields, &meth5},
meth3 = {"redisplay", meth_redisplay,&meth4},
meth2 = { "mark", meth_mark, &meth3},
meth1 = { "quit", meth_quit, &meth2};

Methods
bm9 = {"print", meth_print, NULL},
bm8 = {"completion", meth_return, &bm9},
bm7 = {"unload", meth_unload, &bm8},
bm6 = {"labelmenu", meth_new_labelmenu, &bm7},
bm5 = {"kill", meth_kill, &bm6},
bm4b = {"save_globals", meth_save_globals, &bm5},
bm4 = {"activate",meth_set_active_fields, &bm4b},
bm3 = {"labelfile", meth_make_object, &bm4},
bm2 = {"quit", meth_quit, &bm3},
bm1 = {"set", meth_set, &bm2},
base_methods = {"make", meth_make_object, &bm1};

/*********************************************************************/
Notify_value
kill_signal_view(client, status)
     Canvas client;
     Destroy_status status;
{
  Objects *o;
  Canvas    canvas = xv_get(client, CANVAS_PAINT_CANVAS_WINDOW);

  DEBUG(1)(stderr,"kill_signal_view()\n");
  if(status == DESTROY_CLEANUP) {
    DEBUG(1)(stderr,"DESTROY_CLEANUP\n");
    if((o = (Objects*)xv_get(client, WIN_CLIENT_DATA))) {
      DEBUG(1)(stderr,"Killing object %s\n",o->name);
      kill_object(o,1);
    }
    DEBUG(1)(stderr,"setting client data null\n");
    xv_set(client, WIN_CLIENT_DATA, NULL, 0);
    if(canvas)
      xv_set(canvas, WIN_CLIENT_DATA, NULL, 0);
  }
  DEBUG(1)(stderr,"Leaving kill_signal_view()\n");
  return(notify_next_destroy_func(client, status));
}

/*********************************************************************/
Objects *
make_new_object(str)
     char *str;
{
  char name[NAMELEN], command[MES_BUF_SIZE];

  sscanf(str,"%s",name);
  if(debug_level)
    fprintf(stderr,"make_new_object(%s)\n",str);
  if(strlen(name)) {
    sprintf(command,"name %s",name);
    if(!strcmp(Ok,meth_make_object(NULL,command))) {
      return((Objects *)get_receiver(name));
    }
  }
  return(NULL);
}

/*********************************************************************/
Objects *
new_objects(name)
     char *name;
{
  Objects *ob;
  char *c;

  if((ob = (Objects*)malloc(sizeof(Objects))) &&
     (c = malloc(MAXPATHLEN))) {
       strcpy(c,name);
       ob->name = c;
       ob->signal_name = NULL;
       ob->methods = &meth1;
       ob->cursor = -1.0;
       ob->oldcursor_x = ob->oldcursor_y = -1;
       ob->sec_cm = .05;
       ob->start = 0.0;
       ob->xloc = 10;
       ob->yloc = 5000;
       ob->width = 1000;
       ob->height = 200;
       ob->x_off = 0;
       ob->resize_status = RESIZE_STATUS_NEW;
       ob->labels = NULL;
       ob->next = NULL;
       ob->canvas = XV_NULL;
       return(ob);
     }
  fprintf(stderr,"Can't allocate space for another object\n");
  return(NULL);
}


/*********************************************************************/
kill_proc()
{
  char mess[MES_BUF_SIZE];

  sprintf(mess,"%s disconnect function %s\n",host,thisprog);
  terminal_message( mess );
  terminate_communication(registry_name);
  exit(0);
}

/*************************************************************************/
Objects *obj_exists(input)
     char *input;
{
  Objects *ob;
  if(!*objectname || (!(ob = (Objects*)get_receiver(objectname)) &&
		      !(ob = (Objects*)make_new_object(objectname)))) {
    sprintf(notice_msg,"Object %s doesn't exist (can't add file %s)\n",
	   objectname,input);
    show_notice(0,notice_msg);
    return(NULL);
  }
  return(ob);
}

/*************************************************************************/
void
newFile(item, event)
     Panel_item item;
     Event *event;
{
  Objects *ob;
  Labelfile *lf;

  if(item == file_item) {
    strcpy(inputname,(char*)panel_get_value(item));
    build_label_outout_name(inputname);
    if(*inputname && (ob = obj_exists(inputname))) {
      if((lf = read_labels(ob,inputname))) {
	set_display_size(ob);
        set_active_fields(ob->name,lf->label_name,active);
	send_all_marks(lf);
	make_labels_menu(lf);
      }
      set_active_fields(objectname,inputname,active);
    }
  }

  if(item == top_word_item) {
    strcpy(topword,(char*)panel_get_value(item));
    if(*chartname && (ob = obj_exists(chartname))) {
      Chartfile *cf;
      Wobject *wob;
      if((cf = get_chartfile(get_wobject(ob),chartname))) {
	top_word(topword,cf);
	return;
      }
    }
  }

  if(item == chart_item) {
    strcpy(chartname,(char*)panel_get_value(item));
    if(*chartname && (ob = obj_exists(chartname))) {
      Chartfile *cf;
      Wobject *wob;
      if(!(wob = get_wobject(ob)) && !(wob = new_wobject(ob))) {
	fprintf(stderr,"Problems creating new chart object\n");
	return;
      }
      if((cf = read_charts(wob,chartname))) {
	set_display_size(ob);
	redo_display(ob);
      }
    }
  }

  if(item == object_item) {
    strcpy(objectname,(char*)panel_get_value(item));
    if(!(ob = (Objects*)get_receiver(objectname)) &&
       !(ob = (Objects*)make_new_object(objectname))){
      sprintf(notice_msg,"Object %s doesn't exist.\n",objectname);
      show_notice(0,notice_msg);
    }
    else {
      if(*inputname && ((lf = get_labelfile(ob,inputname)) ||
	((lf = read_labels(ob,inputname)) && redo_display(ob) &&
	  send_all_marks(lf)))) {
	make_labels_menu(lf);
	set_display_size(ob);
	set_active_fields(objectname,inputname,active);
      }
    }
  }

  if(item == menu_item) {
    strcpy(menufile,(char*)panel_get_value(item));
    ob = (Objects*)get_receiver(objectname);
    lf = get_labelfile(ob,inputname);
    if(lf)
      make_labels_menu(lf);
    else
      if(ob && (lf = ob->labels))
	while(lf) {
	  make_labels_menu(lf);
	  lf = lf->next;
	}
  }

  if(item == active_item) {
    strcpy(active,(char*)panel_get_value(item));
    if(*inputname)
      set_active_fields(objectname,inputname,active);
    else
      if((ob = (Objects*)get_receiver(objectname)) && (lf = ob->labels))
	while(lf) {
	  set_active_fields(objectname,lf->label_name,active);
	  lf = lf->next;
	}

  }
}


/*********************************************************************/
set_active_fields(o,f,a)
     char *o, *f, *a;
{
  Objects *ob;
  Labelfile *lf;
  char *s, *get_next_item();
  int af;

  if((ob = (Objects*)get_receiver(o)) && (lf = ob->labels)) {
    while(lf) {
      if(!strcmp(lf->label_name,f)) {
	s = a;
	replace_field = 1;
	for(af=0; af < LAB_MAXFIELDS; af++) lf->active[af] = 0;
	while(*s) {
	  if(*s == '*') {	/* make this active for REPLACE mode */
	    if(*(++s)) {
	      sscanf(s,"%d",&af);
	      replace_field = af;
	    }
	  } else
	    sscanf(s,"%d",&af);
	  if(af && (af < LAB_MAXFIELDS)) lf->active[af-1] = 1;
	  s = get_next_item(s);
	}
        redo_display(ob);
	return(TRUE);
      }
      lf = lf->next;
    }
  }
  return(FALSE);
}

/*********************************************************************/
void remove_cursor(canv, obj)
Canvas canv;
Objects *obj;
{
  Pixwin *pw;
  Rect *rec;
  int bot, top, left, right;
  rec = (Rect *) xv_get(canv, WIN_RECT);
  bot = 0;
  top = rec->r_height;
  left = 0;
  right = rec->r_width;
  pw = canvas_pixwin(canv);

  if (obj->oldcursor_x != -1)
    pw_vector(pw, obj->oldcursor_x, bot, obj->oldcursor_x, top,
	      PIX_COLOR(CURSOR_COLOR)|(PIX_SRC^PIX_DST), CURSOR_COLOR);

  if (obj->oldcursor_y != -1)
    pw_vector(pw, left, obj->oldcursor_y, right, obj->oldcursor_y,
		PIX_COLOR(CURSOR_COLOR)|(PIX_SRC^PIX_DST), CURSOR_COLOR);

  obj->oldcursor_x = -1;
  obj->oldcursor_y = -1;
}

/*********************************************************************/
void add_cursor(canv, obj, x, y)
Canvas canv;
Objects *obj;
int x, y;
{
  Pixwin *pw;
  Rect *rec;
  int bot, top, left, right;
  rec = (Rect *) xv_get(canv, WIN_RECT);
  bot = 0;
  top = rec->r_height;
  left = 0;
  right = rec->r_width;
  pw = canvas_pixwin(canv);

  pw_vector(pw, x, bot, x, top,
	    PIX_COLOR(CURSOR_COLOR)|(PIX_SRC^PIX_DST), CURSOR_COLOR);

  pw_vector(pw, left, y, right, y,
		PIX_COLOR(CURSOR_COLOR)|(PIX_SRC^PIX_DST), CURSOR_COLOR);

  obj->oldcursor_x = x;
  obj->oldcursor_y = y;
}


/*********************************************************************/
void mark_window(ob,left, right)
Objects *ob;
double left, right;
{
	char mess[MES_BUF_SIZE];
	sprintf(mess,"%s marker time %f do_left 1\n",ob->name,left);
  	mess_write(mess);
  	sprintf(mess,"%s marker time %f do_left 0\n",ob->name,right);
  	mess_write(mess);
}

/*********************************************************************/
typedef struct cur_ed {
  Objects *ob;
  Labelfile *lf;
  Label *l;
  Fields *f;
  int active;
} Editing;

static Editing ed = {NULL, NULL, NULL, NULL, FALSE};
static Textsw textwin = XV_NULL;
static Frame editor = XV_NULL;

/*********************************************************************/
still_lives(e)
     Editing *e;
{
  if(e && e->active) {
    extern Objects *objlist;
    Objects *o = objlist;
    while(o) {
      if(o == e->ob) {
	Labelfile *lf = o->labels;
	while(lf) {
	  if(lf == e->lf) {
	    Label *l = lf->first_label;
	    while(l) {
	      if(l == e->l) {
		Fields *f = l->fields;
		while(f) {
		  if(f == e->f)
		    return(TRUE);
		  f = f->next;
		}
	      }
	      l = l->next;
	    }
	  }
	  lf = lf->next;
	}
      }
      o = o->next;
    }
  }
  return(FALSE);
}

/*********************************************************************/
finish_edit()
{
  if(still_lives(&ed)) {
    int ncom_char = xv_get(textwin, TEXTSW_LENGTH);

    if(ed.f->str)
      free(ed.f->str);
    ed.f->str = NULL;

    ed.lf->changed = TRUE;

    if(ncom_char > 0) {
      char *newstr = malloc(ncom_char + 1);

      if(newstr) {
	register char *cp = newstr;
	if(xv_get(textwin, TEXTSW_CONTENTS, 0, newstr, ncom_char)
	   != ncom_char) {
	  Fprintf(stderr, "Error getting textsw contents.\n");
	} else
	  newstr[ncom_char] = '\0';

	ed.f->str = newstr;
	do {
	  if(*cp == '\n')
	    *cp = ' ';
	} while(*cp++);
      } else
	fprintf(stderr,"Allocation problems in finish_edit()\n");
    }

    if(!rewrite_labels(ed.lf)) /*  Rewrite the whole file. */
      fprintf(stderr,"Problems doing rewrite_labels() in finish_edit()\n");

    plot_labels(ed.lf);
  }

  ed.active = FALSE;

  if(editor)
    xv_set(editor, XV_SHOW, FALSE, NULL);
}

/*********************************************************************/
ignore_edit()
{
  ed.active = FALSE;

  if(editor)
    xv_set(editor, XV_SHOW, FALSE, NULL);
}

/*********************************************************************/
static void open_edit_win(str, xloc, yloc)
     char *str;
     int xloc, yloc;
{
  Panel lpanel;

  /* Create frame. */

  if(!editor) {
    editor =  xv_create(NULL, FRAME, NULL);

  xv_set(editor,
	 XV_LABEL, "LONG STRING EDITOR",
	 XV_WIDTH, 882,
	 XV_HEIGHT, 80,
	 XV_SHOW, FALSE,
	 FRAME_SHOW_FOOTER, FALSE,
	 FRAME_SHOW_RESIZE_CORNER, TRUE,
	 NULL);

  /* Create left control panel. */
  lpanel = (Panel) xv_create(editor, PANEL,
			     XV_X, 0,
			     XV_Y, 0,
			     XV_WIDTH, WIN_EXTEND_TO_EDGE,
			     XV_HEIGHT, 25,
			     WIN_BORDER, FALSE,
			     NULL);

    (void) xv_create(lpanel, PANEL_MESSAGE,
		XV_X, 32,
		PANEL_LABEL_STRING,
		   "Left click to edit; then press Apply or Ignore.         ",
		PANEL_LABEL_BOLD, TRUE,
		NULL);


  (void) xv_create(lpanel, PANEL_BUTTON,
		PANEL_LABEL_STRING,
		"Apply changes",
		PANEL_NOTIFY_PROC, finish_edit,
		NULL);

  (void) xv_create(lpanel, PANEL_BUTTON,
		PANEL_LABEL_STRING,
		   "Ignore changes",
		PANEL_NOTIFY_PROC, ignore_edit,
		NULL);

  /* Create text window. */

  textwin = xv_create(editor, TEXTSW,
		XV_X, 0,
		XV_Y, (int)xv_get(lpanel, XV_Y) +
		      (int)xv_get(lpanel, XV_HEIGHT),
		XV_WIDTH, WIN_EXTEND_TO_EDGE,
		XV_HEIGHT, WIN_EXTEND_TO_EDGE,
		OPENWIN_SHOW_BORDERS, TRUE,
		NULL);
  }

  textsw_reset(textwin, 0,0);

  if(str && *str)
    textsw_insert(textwin, str, strlen(str));

  xv_set(editor, XV_SHOW, TRUE,
	 XV_X, xloc,
	 XV_Y, yloc,
	 NULL);
}


/*********************************************************************/
set_up_editor(ob,lf,label,fld)
     Objects *ob;
     Labelfile *lf;
     Label *label;
     Fields *fld;
{
  if(ed.active)			/* was previous edit in progress? */
    finish_edit();

  if((ed.ob = ob) && (ed.lf = lf) && (ed.l = label) && (ed.f = fld)) {
    open_edit_win(fld->str, ob->xloc + ob->x_off, ob->yloc + ob->height);
    ed.active = TRUE;
    return(TRUE);
  }
  return(FALSE);
}

/**********************************************************************/
int get_default_xlabel_color()
{
  return(color);
}

/**********************************************************************/
Selector *xlabel_global_list()
{
  return(&a0);
}

/************************************************************************/
Labelfile *
new_labelfile(sig_name,label_name)
     char *sig_name, *label_name;
{
  Labelfile *l;
  char *name;
  int i;


  if(!((l = (Labelfile*)malloc(sizeof(Labelfile))) &&
       (name = malloc(MAXPATHLEN)))) {
    printf("Can't allocate a Labelfile structure\n");
    return(NULL);
  }
  if(*signame)
    strcpy(name,signame);
  else
    strcpy(name,sig_name);
  l->sig_name = name;
  if((!label_name)||(! *label_name))
    l->label_name = (char*)make_label_filename(name);
  else {
    l->label_name = name = malloc(MAXPATHLEN);
    strcpy(name,label_name);
  }
  l->type = type;
  l->color = color;
  l->height = height;
  l->changed = 0;
  if(*fontfile) {
    strcpy(l->fontfile, fontfile);
    l->font = NULL;
  } else {
    l->font = get_default_label_font();
    l->fontfile[0] = 0;
  }
  l->menu = (Menu)0;
  if(*comment) {
    l->comment = malloc(strlen(comment)+1);
    strcpy(l->comment,comment);
  } else
    l->comment = NULL;
  l->separator[0] = sep;
  l->separator[1] = 0;
  l->nfields = nfields;
  for(l->active[0] = 1, i=1; i<LAB_MAXFIELDS; i++) l->active[i] = 0;
  l->first_label = l->last_label = l->move_pending = NULL;
  l->next = NULL;
  return(l);
}
