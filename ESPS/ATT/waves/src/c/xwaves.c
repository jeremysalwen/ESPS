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
 * Written by:  David Talkin
 * Checked by:
 * Revised by:  John Shore, Rod Johnson, Alan Parker, ERL
 *
 *  xwaves.c
 *  a general-purpose speech parameter display program
 *
 */

static char *sccs_id = "@(#)xwaves.c	1.36 28 Oct 1999 ERL/ATT";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <Objects.h>
#if !defined(HP400) && !defined(APOLLO_68K) && !defined(DS3100) && !defined(LINUX_OR_MAC)
#include <malloc/malloc.h>
#endif
#include <xview/scrollbar.h>
#include <xview/font.h>
#include <xview/notice.h>
#include <esps/exview.h>
#ifndef hpux
#include <sys/param.h>
#else
#define MAXPATHLEN 1024
#endif

#define SYNTAX USAGE("xwaves [-w wave_pro] [-s] [-n<alternate name>]  [-c] [file(s)]...") ;

#define REACT_TIME 0.2


extern int  optind;		/* for use of getopt() */
extern char *optarg;		/* for use of getopt() */
extern char *wave_pro;		/* .wave_pro file name from command line */

extern void repaint();
extern void identify_visual();

View *create_new_signal_view();

char	*get_helpfile();
static double get_next_time(), get_prev_time();
void	quit_proc(), doit(), command_proc(), newText_proc(), doub_proc(),
	show_output_proc(), main_proc();
int	pause_proc(), resume_proc();
int	exv_get_help();
Notify_value iocatcher(), stdiocatcher();
Notify_value destroy_func();
Notify_value sigint_func();

char *setup_ipc();

#if defined(DS3100) || defined(hpux)
/* This is an ugly hack because I can't figure out how to fix this
   correctly on the DS3100 XView port.    Without this hack, the
   aux panels in xvwaves come up black and white.
*/
Xv_singlecolor ffg, fbg;   /* frame foreground/background colors */
int             dffg = 0x000000,        /* default frame foreground color */
                dfbg = 0xf0ffff;        /*    "      "   background   " */
#endif



/* THE FOLLOWING GLOBALS MAY BE SET FROM <a profile file> */

extern char inputname[],	/* name of signal input file */
	    outputname[],	/* name of signal output file */
	    objectname[],	/* user-defined object name */
	    commandname[],	/* command name */
	    init_file[],	/* command script for initialization */
	    funcname[],		/* linkage to external process */
	    wb_spect_params[],
	    nb_spect_params[],
	    overlayname[],
	    def_cm[],
	    def_left_op[],
	    def_middle_op[],
	    def_move_op[];

extern int  w_verbose;
char	    *savestring(), *basename();
extern char wb_spect_params[], nb_spect_params[], cmsname[], def_cm[];
FILE	    *fp_command = NULL;	/* file from which commands
				 * are being executed */
int	    command_line = 0;	/* line in file being processed */

extern	    scroll_ganged,
	    zoom_ganged,
	    edit_ganged,
	    line_type,		/* for plotting waveforms */
	    ctlwin_x,
	    ctlwin_y,
	    ctlwin_iconized;

/* some display creation globals */
extern double ref_size,		/* size for def. waveform disp.*/
              ref_step,		/* amount to step in file */
	      ref_start,	/* where reference window display begins */
	      zoom_ratio;
extern int  def_w_height,		/* dim. of waveform window */
            def_w_width,
	    scrollbar_height, readout_bar_height;

/* window position parameters */
extern int  next_x,		/* upper left-hand corner of new window */
            next_y,
	    w_x_incr,		/* increment for next window */
	    w_y_incr;

/* END OF GLOBALS SET FROM CONFIGURATION FILE */

/* Args of make command. */
extern int  new_width,		/* dimensions of new window */
	    new_height;

/* Flag to enable debug printout. */
int	    debug_level;

/* Waves+ version number */
extern char *Version;

/* A flag to indicate whether to use the dsp32/VME buss board: */
extern int  use_dsp32;

extern int do_color;

Xv_Font def_font;
int     def_font_height, def_font_width;
int	menu_item_key;		/* key for storing last selected menu
				   item as XV_KEY_DATA for canvas */
int	file_name_key;		/* key for storing selected file name from
				   browser menu as XV_KEY_DATA for canvas */
int	list_elem_key;		/* key for storing list element selected from
				   browser menu as XV_KEY_DATA for canvas */
int	auxpanel_key;		/* key for storing auxiliary control panel
				   list element as XV_KEY_DATA for panel
				   frame */

Frame	daddy = (Frame)NULL;
Panel	control_panel;
Panel_item attach_panel;

Object	program = {
    (char*)NULL,
    (Signal*)NULL,
    (Methods*)NULL,
    (Marks*)NULL,
    0,
    0,
    0,
    0,
    (Object *)NULL,
};

int	    child = 0;
char	    *host;		/* basename of this program */
char *thisprog;			/* full runtime name of this program */

Notify_value kill_program();

Menu	    spect_menu = (Menu)0, wave_menu = (Menu)0,
	    make_wave_menu(), make_spect_menu();

Panel_item  help_item, quit_item, newFile_item, newFunct_item, newObj_item,
	    newControl_item, pause_item, resume_item, overlay_item,
	    outputFile_item;

/* flags, etc. for interaction with realtime and DSP-32 operations. */
extern int  da_location, da_done;
extern void e_up_down_move_marks(), e_play_between_marks(), menu_operate();

int server_mode = 0;
char *meth_enable_server();
extern int socket_port;
extern int use_static_cmap;
extern int fullscreendebug;

static unsigned int prev_attach_value=0;

/*********************************************************************/
static void attach_callback(item, value, event)
     Panel_item	item;
     unsigned int	value;
     Event		*event;
{
  short	i, num;
  unsigned int change, new_value;
  char *attachment;
	
  change = value ^ prev_attach_value;
  prev_attach_value = value;

  num = (int) xv_get(item, PANEL_NCHOICES);

  for (i = 0; i < num; i++) {
    if (change & 01)
    {
      attachment = (char *) xv_get(item, PANEL_CHOICE_STRING, i);

      if (01 & value) {
	start_external_function(attachment);
      } else  {
	terminate(attachment);
      }
    }
    value >>=1;
    change >>=1;
  }

}
/*********************************************************************/

void reset_attach_choice(func)
char *func;
{
  unsigned int mask = ~01;
  unsigned int value, num, i;
  char *attachment;

  if (attach_panel) {
   num = (int) xv_get(attach_panel, PANEL_NCHOICES);
   value = (unsigned int) xv_get(attach_panel, PANEL_VALUE);

   for (i = 0; i< num; i++) {
	attachment = (char *)xv_get(attach_panel, PANEL_CHOICE_STRING, i);
        if (strcmp(func, attachment) == 0) {
		value = value & mask;
		prev_attach_value = value;
		xv_set(attach_panel, PANEL_VALUE, value, NULL);
		break;
	} else
		mask = (mask << 1) | 1;
   }
   return;
  }
}

/*********************************************************************/
static Panel_item add_attach_choices(panel, xvx, xvy, items)
     Xv_opaque panel;	
     int xvx, xvy;
     char *items;
{
  char *cp, *get_next_item(), choice[100];
  Panel_item pi;
  int i = 0;

  if(!(items && *items))
    items = "xlabel xspectrum";

  if(!strcmp(items,"none"))
    return (int)NULL;

  cp = items;
  pi = (Panel_item)xv_create(panel, PANEL_TOGGLE,
	    XV_X, xvx,
	    XV_Y, xvy,
	    PANEL_CHOICE_NROWS, 1,
	    PANEL_LAYOUT, PANEL_HORIZONTAL,
	    PANEL_LABEL_STRING, "Attach:",
	    PANEL_NOTIFY_PROC, attach_callback,
	    PANEL_VALUE, 0,
	    NULL);
  while(cp && *cp) {

    sscanf(cp,"%s",choice);
    xv_set(pi, PANEL_CHOICE_STRING, i, choice,
	   NULL);
    cp = get_next_item(cp);
    i++;
  }
  return pi;
}
	
/*********************************************************************/
main(ac, av)
     int ac;
     char **av;
{
  int i;
  Object *ob;
  extern Methods bmeth1;
  extern int	command_paused;
  char		frame_label[200];
  int		ch;		/* option letter read by getopt */
#ifndef NO_LIC
  extern void	lm_quit();
#endif
  int p_flag=0;
  extern char attachments[];
  extern char *registry_name;
  char *default_name = "xwaves";

#if defined(DS3100) || defined(hpux)
  ffg.red = (dffg >> 16) & 0xff; /* unpack some colormap values */
  ffg.green = (dffg >> 8 ) & 0xff;
  ffg.blue = dffg & 0xff;
  fbg.red = (dfbg >> 16) & 0xff;
  fbg.green = (dfbg >> 8 ) & 0xff;
  fbg.blue = dfbg & 0xff;
#endif


  check_version();	/* be sure we are running in the correct e/w version*/

  fullscreendebug = 1; /* this global inhibits server grabs that cause
			  problems on the SGI */
  xv_init(XV_INIT_ARGC_PTR_ARGV, &ac, av, NULL);

  while ((ch = getopt(ac, av, "n:w:sp:cP:V:x:")) != EOF)
      switch (ch)
      {
      case 'n':
          default_name = optarg;
          break;
      case 'w':
	  wave_pro = optarg;	/* initialized to ".wave_pro" in globals.c */
	  break;
      case 's':
	  server_mode = 1;
	  break;
      case 'p':
	  server_mode = 1;
	  socket_port = atoi(optarg);
          p_flag++;
	  break;
      case 'c':
	  use_static_cmap = 1;
	  break;
      case 'P':
      case 'V':
	  fprintf(stderr, "%s%s%s",
		"The -P and -V options are no longer used, since RPC",
                " is no longer used in\nxwaves.  See the xwaves manual",
                "and the -n option.\n");
      case 'x':
	  debug_level = atoi(optarg);
	  break;
      default:
	  SYNTAX
	      ;
	  break;
      }

  if(!p_flag && getenv("WAVES_PORT"))
	socket_port = atoi(getenv("WAVES_PORT"));

  def_font = (Xv_Font) xv_find(XV_NULL, FONT,
				FONT_FAMILY, FONT_FAMILY_DEFAULT_FIXEDWIDTH,
				0);

  def_font_width = (int) xv_get(def_font, FONT_DEFAULT_CHAR_WIDTH);
  def_font_height = (int) xv_get(def_font, FONT_DEFAULT_CHAR_HEIGHT);

  if (debug_level)
      fprintf(stderr, "default font height %d, width %d.\n",
	      def_font_height, def_font_width);

  waves_initialize();

/*!*//* xv_unique_key unreliable; find better way to pick keys. */
  menu_item_key = xv_unique_key();
  file_name_key = xv_unique_key();
  list_elem_key = xv_unique_key();
  auxpanel_key = xv_unique_key();

  wave_menu = make_wave_menu();	 /* one menu for each generic window */
  spect_menu = make_spect_menu();

  esps_initialize();

  host = basename(av[0]);
  thisprog = av[0];
  child = FALSE;


  if(do_color)
    daddy = xv_create(XV_NULL, FRAME,
#if defined(DS3100) || defined(hpux)
                     FRAME_FOREGROUND_COLOR, &ffg,
                     FRAME_BACKGROUND_COLOR, &fbg,
#endif
		      FRAME_INHERIT_COLORS, FALSE,
		      XV_X, ctlwin_x,
		      XV_Y, ctlwin_y,
		      XV_WIDTH, 665,
		      XV_HEIGHT, 141,
		      FRAME_CLOSED, FALSE,
		      FRAME_SHOW_FOOTER, FALSE,
		      0);
  else
    daddy = xv_create(XV_NULL, FRAME,
		      XV_X, ctlwin_x,
		      XV_Y, ctlwin_y,
		      XV_WIDTH, 665,
		      XV_HEIGHT, 141,
		      FRAME_CLOSED, FALSE,
		      FRAME_SHOW_FOOTER, FALSE,
		      0);

  if(!daddy) {
    printf("Either X needs to be started, or you are out of memory!\n");
#ifndef NO_LIC
    lm_quit();
#endif
    exit(-1);
  }

  if (debug_level > 3)
    XSynchronize((Display *)xv_get(daddy,XV_DISPLAY), True);

  install_signal_handlers(daddy);
  notify_interpose_destroy_func(daddy, destroy_func);

  registry_name = setup_ipc(daddy, default_name);

/* export my registry name */
  if (registry_name) {
    char *ptr = (char *)malloc(strlen("WAVES_NAME=")+strlen(registry_name)+3);
    sprintf(ptr, "WAVES_NAME=%s", registry_name);
    putenv(ptr);
  }

  sprintf(frame_label,
	  "xwaves Multidimensional Signal Display, Version %s (%s)",
		Version, registry_name);
  xv_set(daddy, XV_LABEL, frame_label, NULL);

  (void) exv_attach_icon(daddy, ERL_NOBORD_ICON, "xwaves", TRANSPARENT);

  control_panel = xv_create(daddy, PANEL,
			    XV_X,		    0,
			    XV_Y,		    0,
			    XV_WIDTH,		    WIN_EXTEND_TO_EDGE,
			    XV_HEIGHT,		    WIN_EXTEND_TO_EDGE,
			    OPENWIN_SHOW_BORDERS,   FALSE,
			    0);
  window_wash(control_panel);

  pause_item = xv_create(control_panel, PANEL_BUTTON,
			 PANEL_LABEL_STRING,	"PAUSE",
			 PANEL_NOTIFY_PROC,	pause_proc,
			 XV_X,			206,
			 XV_Y,			16,
/* 			 XV_HELP_DATA,		"xwaves:pause_item", */
			 XV_WIDTH,		57,
			 XV_HEIGHT,		19,
			 0);

  resume_item = xv_create(control_panel, PANEL_BUTTON,
			  PANEL_LABEL_STRING, "CONTINUE",
			  PANEL_NOTIFY_PROC, resume_proc,
			  XV_X, 310,
			  XV_Y, 16,
			  XV_HELP_DATA, "xwaves:resume_item",
			  XV_WIDTH, 83,
			  XV_HEIGHT, 25,
			  0);

  newFile_item = xv_create(control_panel, PANEL_TEXT,
			   XV_HELP_DATA, "xwaves:newFile_item",
			   XV_X, 12,
			   XV_Y, 52,
			   XV_WIDTH, 306,
			   XV_HEIGHT, 15,
			   PANEL_VALUE_X, 94,
			   PANEL_VALUE_Y, 52,
			   PANEL_LAYOUT, PANEL_HORIZONTAL,
			   PANEL_VALUE_DISPLAY_LENGTH, 29,
			   PANEL_VALUE_STORED_LENGTH, 120,
			   PANEL_READ_ONLY, FALSE,
			   PANEL_LABEL_STRING, "INPUT file:",
			   PANEL_VALUE, inputname,
			   PANEL_NOTIFY_PROC, newText_proc,
			   0);

  outputFile_item = xv_create( control_panel, PANEL_TEXT,
			      XV_HELP_DATA, "xwaves:outputFile_item",
			      XV_X, 344,
			      XV_Y, 52,
			      XV_WIDTH, 297,
			      XV_HEIGHT, 15,
			      PANEL_LABEL_STRING, "OUTPUT file:",
			      PANEL_VALUE_X, 441,
			      PANEL_VALUE_Y, 52,
			      PANEL_LAYOUT, PANEL_HORIZONTAL,
			      PANEL_VALUE_DISPLAY_LENGTH, 25,
			      PANEL_VALUE_STORED_LENGTH, 120,
			      PANEL_READ_ONLY, FALSE,
			      PANEL_VALUE, outputname,
			      PANEL_NOTIFY_PROC, newText_proc,
			      PANEL_EVENT_PROC, show_output_proc,
			      0);

  newObj_item = xv_create( control_panel, PANEL_TEXT,
			  XV_HELP_DATA, "wtest:newObj_item",
			  XV_X, 12,
			  XV_Y, 20,
			  XV_WIDTH, 170,
			  XV_HEIGHT, 15,
			  PANEL_LABEL_STRING, "OBJECT name:",
			  PANEL_VALUE_X, 118,
			  PANEL_VALUE_Y, 20,
			  PANEL_LAYOUT, PANEL_HORIZONTAL,
			  PANEL_VALUE_DISPLAY_LENGTH, 8,
			  PANEL_VALUE_STORED_LENGTH, 80,
			  PANEL_READ_ONLY, FALSE,
			  PANEL_VALUE, objectname,
			  0);

  attach_panel = add_attach_choices(control_panel, 344, 80, attachments);

  overlay_item = xv_create(control_panel, PANEL_TEXT,
			   XV_HELP_DATA, "xwaves:overlay_item",
			   XV_X, 12,
			   XV_Y, 80,
			   XV_WIDTH, 316,
			   XV_HEIGHT, 15,
			   PANEL_LABEL_STRING, "Overlay name:",
			   PANEL_VALUE_X, 120,
			   PANEL_VALUE_Y, 80,
			   PANEL_LAYOUT, PANEL_HORIZONTAL,
			   PANEL_VALUE_DISPLAY_LENGTH, 26,
			   PANEL_VALUE_STORED_LENGTH, 120,
			   PANEL_READ_ONLY, FALSE,
			   PANEL_NOTIFY_PROC, newText_proc,
			   PANEL_VALUE, overlayname,
			   0);

/*  help_item = xv_create(control_panel, PANEL_BUTTON,
			XV_HELP_DATA,		"xwaves:help_item",
			XV_X,			382,
			XV_Y,			16,
			XV_WIDTH,		51,
			XV_HEIGHT,		19,
			PANEL_LABEL_STRING,	"HELP!",
			XV_KEY_DATA,
			    EXVK_HELP_NAME,	FIND_WAVES_LIB(NULL,
							      "xwaves.help"),
			XV_KEY_DATA,
			    EXVK_HELP_TITLE,	"xwaves help",
			PANEL_NOTIFY_PROC,	exv_get_help,
			0);
*/
  help_item = xv_create(control_panel, PANEL_BUTTON,
			XV_HELP_DATA,		"xwaves:man_help_item",
			XV_X,			451,
			XV_Y,			16,
			XV_WIDTH,		119,
			XV_HEIGHT,		25,
			PANEL_LABEL_STRING,	"xwaves MANUAL",
			XV_KEY_DATA,
			    EXVK_HELP_NAME,	FIND_WAVES_LIB(NULL,
							      "xwaves.man"),
			XV_KEY_DATA,
			    EXVK_HELP_TITLE,	"xwaves manual page",
			PANEL_NOTIFY_PROC,	exv_get_help,
			0);

  quit_item = xv_create(control_panel, PANEL_BUTTON,
			XV_HELP_DATA,		"xwaves:quit_item",
			XV_X,			588,
			XV_Y,			16,
			XV_WIDTH,		52,
			XV_HEIGHT,		25,
			PANEL_LABEL_STRING,	"QUIT!",
			PANEL_NOTIFY_PROC,	quit_proc,
			0);

  newControl_item = xv_create(control_panel, PANEL_TEXT,
			      XV_HELP_DATA, "xwaves:newControl_item",
			      XV_X, 12,
			      XV_Y, 108,
			      XV_WIDTH, 631,
			      XV_HEIGHT, 15,
			      PANEL_VALUE_X, 163,
			      PANEL_VALUE_Y, 108,
			      PANEL_LAYOUT, PANEL_HORIZONTAL,
			      PANEL_VALUE_DISPLAY_LENGTH, 60,
			      PANEL_VALUE_STORED_LENGTH, 200,
			      PANEL_READ_ONLY, FALSE,
			      PANEL_NOTIFY_PROC, newText_proc,
			      PANEL_LABEL_STRING, "COMMAND (or @file):",
			      PANEL_VALUE, commandname,
			      0);

  init_print_setup_window();

  identify_visual();

/*    change to hard wire in waves as the initial object name */
  if((program.name = savestring("waves"))) {
    program.methods = &bmeth1;
    window_fit(control_panel);
    window_fit(daddy);

    /* Process funcname before init_file; otherwise, if init_file starts
       an attachment, funcname gets set as a side effect, and the attachment
       gets started twice. */

    if(*funcname)
	start_external_function(funcname);

    if(server_mode)
      (void)meth_enable_server(NULL,NULL);


    if(*init_file && strcmp(init_file, "/dev/null")) {
      char *act_init_file = malloc(MAXPATHLEN);
      if (FIND_WAVES_COMMAND(act_init_file,init_file)) {
	strcpy(commandname, "@");
	strcat(commandname, act_init_file);
	panel_set_value(newControl_item, commandname);
	command_proc(newControl_item, EVENT_NULL);
      } else  {
	sprintf(notice_msg, "Cannot find init_file %s", init_file);
        show_notice(0,notice_msg);
      }
      free(act_init_file);
    }

    if(ac > optind + 50) ac = optind + 50;   /* some protection against	*/
					    /* too many windows...	*/
    for(i = optind; i < ac; i++)
	if((av[i][0] == '-') && !av[i][1]) {
	    strcpy(commandname, "@stdin");
	    panel_set_value(newControl_item, commandname);
	    command_proc(newControl_item, EVENT_NULL);
	    break;
	} else {
	    char *cmd_file = FIND_WAVES_COMMAND(NULL, av[i]);
	    if(cmd_file && is_a_command_file(cmd_file)) {
	      strcpy(commandname, "@");
	      strcat(commandname, cmd_file);
	      panel_set_value(newControl_item, commandname);
	      command_proc(newControl_item, EVENT_NULL);
	    } else {
	      strcpy(inputname, av[i]);
	      (void)apply_waves_input_path(inputname, inputname);
	      panel_set_value(newFile_item, inputname);
	      create_new_signal_view(inputname);
	    }
	if(cmd_file)
	    free(cmd_file);
        }


    if (ctlwin_iconized) {
      xv_set(daddy, FRAME_CLOSED, TRUE, 0);
    }

    window_main_loop(daddy);

  } else {
    printf("Problems initializing main program object\n");
#ifndef NO_LIC
    lm_quit();
#endif
    exit(-1);
  }
}

/*************************************************************************/
void
doub_proc(item, event)
     Panel_item item;
     Event *event;
{
  double val;

#define PNZ_VAL(x) (x = (val > 0.0)? val : x)

  sscanf((char*)panel_get_value(item),"%lf", &val);

}

/*********************************************************************/
void
rescale_it(c, w, h)
    Canvas  c;
    int	    w, h;
{
  View *v;

  if (debug_level)
  {
      Frame frame;
      Xv_Window pw;

      fprintf(stderr, "rescale_it(c, %d, %d)\n", w, h);
      pw = canvas_paint_window(c);
      fprintf(stderr, "Paint window has XV_WIDTH %d,  XV_HEIGHT %d.\n",
	  (int) xv_get(pw, XV_WIDTH), (int) xv_get(pw, XV_HEIGHT));
      fprintf(stderr, "Canvas has XV_WIDTH %d,  XV_HEIGHT %d.\n",
	  (int) xv_get(c, XV_WIDTH), (int) xv_get(c, XV_HEIGHT));
      frame = (Frame) xv_get(c, XV_OWNER);
      fprintf(stderr, "Frame has XV_WIDTH %d,  XV_HEIGHT %d.\n",
	  (int) xv_get(frame, XV_WIDTH), (int) xv_get(frame, XV_HEIGHT));
  }

  if (w && (v = (View*)xv_get(c, WIN_CLIENT_DATA))) {
    *(v->x_scale) *= ((double) (v->width - *v->x_offset))/(w - *v->x_offset);
    v->width = w;
    v->height = h;
  }
}

/*********************************************************************/
View *create_new_signal_view(name)
    char *name;
{
  static Frame    frame, sfr;
  Canvas          canvas;
  Scrollbar       scrollbar;
  Xv_Cursor       cursor;
  Rect            *rec;
  char            head[100];
  Signal          *sig, *get_any_signal();
  Object          *ob;
  int             fd;
  extern int cmap_depth;
  double          dur;
  View            *v;
  int             width, height;
  Object *find_object();
  S_bar *new_scrollbar();
  char *remove_ext();
  extern Visual   *visual_ptr;

  if (debug_level)
    (void) fprintf(stderr, "create_new_signal_view: function entered\n");
  /* get object name (if any) */
/*  objectname[0] = '\0';		/* re-init name string */
  sscanf((char *)panel_get_value(newObj_item),"%s",objectname);
  if (!objectname[0]) {		/* defaults filename - extension */
    strncpy(objectname,remove_ext(name),99);
    panel_set_value(newObj_item,objectname);
  }

  if((sig = get_any_signal(expand_name(NULL,name),ref_start,ref_size,NULL))) {
    close_sig_file(sig);	/* to conserve open file numbers */
    if(!(ob = find_object(objectname))) {
      if(!(ob = new_object(objectname, sig))) {
	show_notice(1,"Cannot create a new object in create_new_signal_view");
	return(NULL);
      }
      link_new_object(ob);
    } else
      link_new_signal(ob,sig);

    strcpy(inputname,name);
    /* check for special types */
    if ((sig->type & SPECIAL_SIGNALS) == SIG_SPECTROGRAM) {
      return((View*)new_spect_display(sig));
    }

    width = (new_width > 0) ? new_width : def_w_width;
    height = (new_height > 0) ? new_height : def_w_height;

    if (debug_level)
      fprintf(stderr, "Creating frame with XV_WIDTH %d,  XV_HEIGHT %d.\n",
	      width, height);

    if(do_color)
      frame = xv_create(XV_NULL, FRAME,
			XV_VISUAL,	        visual_ptr,
			XV_WIDTH,		width + 2*FRAME_MARGIN,
			XV_HEIGHT,		height + FRAME_HEADER
			+ FRAME_MARGIN,
			FRAME_INHERIT_COLORS,	FALSE,
			XV_SHOW,		FALSE,
			FRAME_NO_CONFIRM,	TRUE,
			XV_X,			next_x,
			XV_Y,			next_y,
			0);
    else
      frame = xv_create(XV_NULL, FRAME,
			XV_WIDTH,		width + 2*FRAME_MARGIN,
			XV_HEIGHT,		height + FRAME_HEADER
			+ FRAME_MARGIN,
			XV_LABEL,		head,
			XV_SHOW,		FALSE,
			FRAME_INHERIT_COLORS,	TRUE,
			FRAME_NO_CONFIRM,	TRUE,
			XV_X,			next_x,
			XV_Y,			next_y,
			0);
    if(!window_check_return(frame)) return(NULL);

    next_x += w_x_incr;
    next_y += w_y_incr;
    if((next_y+def_w_height) > SCREEN_HEIGHT) {
      next_y = 120;
      next_x = 10;
    }

    canvas = xv_create(frame, CANVAS,
		       CANVAS_RETAINED, FALSE,
		       CANVAS_FIXED_IMAGE, FALSE,
/*			WIN_DEPTH,              cmap_depth, */
		       CANVAS_AUTO_SHRINK, TRUE,
		       CANVAS_AUTO_EXPAND, TRUE,
		       OPENWIN_NO_MARGIN,	TRUE,
		       XV_WIDTH,		WIN_EXTEND_TO_EDGE,
		       XV_HEIGHT,		WIN_EXTEND_TO_EDGE,
		       WIN_MENU, wave_menu,
		       XV_KEY_DATA,	menu_item_key,
		         find_operator(wave_get_ops(),"play between marks"),
		       CANVAS_CMS_REPAINT, FALSE,
		       CANVAS_PAINTWINDOW_ATTRS,
			XV_VISUAL,	        visual_ptr,
			WIN_DEPTH,              cmap_depth,
		       0,
		       CANVAS_NO_CLIPPING,	TRUE,
		       0);

    xv_set(canvas_paint_window(canvas),
	   WIN_CONSUME_EVENTS,
	   LOC_DRAG,
	   WIN_IN_TRANSIT_EVENTS,
	   WIN_MOUSE_BUTTONS,
	   LOC_MOVE,
	   LOC_WINEXIT,
	   LOC_WINENTER,
	   KBD_USE,
	   KBD_DONE,
	   WIN_ASCII_EVENTS,
	   0,
	   WIN_EVENT_PROC, doit,
	   WIN_BIT_GRAVITY, ForgetGravity,
	   0);

    if(!window_check_return(canvas )) {
      dt_xv_destroy_safe(16,frame);
      return(NULL);
    }

    cmap(canvas);

    /* kill_signal_view wasn't getting called when interposed on canvas, so
     * interpose on frame.  But function has to be able to find canvas to
     * get its WIN_CLIENT_DATA.     */
    notify_interpose_destroy_func(frame, kill_signal_view);
    xv_set(frame, WIN_CLIENT_DATA, (caddr_t) canvas, 0);

    if((v = setup_view(sig, canvas))) {
      v->next = sig->views;
      sig->views = v;
      update_window_titles(sig);
      xv_set(canvas,
	     WIN_CLIENT_DATA,	sig->views,
	     /* Don't set repaint and resize procs in original xv_create
		since xv_create might call the repaint proc prematurely. */
	     CANVAS_REPAINT_PROC,	repaint,
	     CANVAS_RESIZE_PROC,	rescale_it,
	     0);

      (void) exv_attach_icon(frame, SINE_ICON, basename(name), TRANSPARENT);

      xv_set(frame, XV_SHOW, TRUE, 0);
      sig->views->scrollbar = new_scrollbar(sig->views);
      return(v);
    } else
      show_notice(1, "Cannot setup signal views in create_new_signal_view");
  } else {
    sprintf(notice_msg,"Cannot open file %s in create_new_signal_view",name);
    show_notice(1,notice_msg);
  }
  return(NULL);
}

/*********************************************************************/
void
doit(canvas_pw, event, arg)
    Pixwin  *canvas_pw;
    Event   *event;
    caddr_t arg;
{
/* Event proc called on paint window, not canvas itself. */
  Canvas      canvas = xv_get((Xv_opaque)canvas_pw, CANVAS_PAINT_CANVAS_WINDOW);
  View	      *v;
  double      time, freq;
  static int  x;
  int	      i, id;
  void	      (*e_proc)();
  extern Event *last_event;

  last_event = event;

  v = (View *)xv_get((Xv_opaque)canvas, WIN_CLIENT_DATA);

  id = event_id(event);

  /* This section handles events in the scrollbar region. */
  if(event_y(event) < v->readout_height + v->scrollbar->height) {
    if(v->handle_scrollbar)
      v->handle_scrollbar(v,event);
    return;
  }

  /* This code segment handles interruption of d/a conversion. */
  if(event_is_down(event) && (event_id(event) == MS_LEFT) && (!da_done)) {
    handle_da_interruption(v);
    return;
  }

  switch (id) {

  case LOC_DRAG:
    if (event_left_is_down(event) &&
	v->left_but_proc && v->left_but_proc->proc)
      v->left_but_proc->proc(canvas, event, v->left_but_proc->data);
    else
      if (event_middle_is_down(event) &&
	  v->mid_but_proc && v->mid_but_proc->proc)
	v->mid_but_proc->proc(canvas, event, v->mid_but_proc->data);
    break;

  case LOC_MOVE:
    win_set_kbd_focus(canvas_pw,xv_get((Xv_opaque)canvas_pw,XV_XID));
    transform_xy(v, event_x(event), event_y(event));
    if(v->move_proc && v->move_proc->proc) {
      v->move_proc->proc(canvas, event, v->move_proc->data);
    }
    break;

  case MS_LEFT:
    if(v->left_but_proc && v->left_but_proc->proc)
      v->left_but_proc->proc(canvas, event, v->left_but_proc->data);
    break;

  case MS_MIDDLE:
    if(v->mid_but_proc && v->mid_but_proc->proc)
      v->mid_but_proc->proc(canvas, event, v->mid_but_proc->data);
    break;

  case MS_RIGHT:
    if(v->right_but_proc)
      v->right_but_proc(canvas, event, arg);
    break;

  default:
    if((id >= ASCII_FIRST) && (id <= META_LAST)) {
      keymap_command(canvas, event, arg);
      if(debug_level) {
	/*	if(id == '') id = '\n'; */
	printf("%d %c", id, id);
	fflush(stdout);
      }
    }
    break;
    }
  return;
}

/*********************************************************************/
move_view_to_loc(v, da_loc)
     View *v;
     int da_loc;
{
  extern int da_stop_pos_view;

  if(da_stop_pos_view && v && (da_loc > 0)) {
    double t, size;
    Signal *s;
    Signal *get_playable_signal();
    if(debug_level)
      fprintf(stderr,"move_view_to_loc: da_loc %d\n", da_loc);

    if((s = get_playable_signal(v))) {
      size = ET(v) - v->start_time;
      t = (s->start_time + ((double)da_loc)/s->freq) - (0.5 * size) - REACT_TIME;
      if(t < s->start_time) t = s->start_time;
      page(v,v->sig,t);
    }
  }
  return;
}

/*********************************************************************/
handle_da_interruption(v)
     View *v;
{
  int     da_loc = 0;
  extern int play_pid;
  Signal *get_playing_signal(), *s;

  if (use_dsp32 && play_pid == 0) /* means that an external play was not used */
     da_loc  = da_location - da_samps_remaining();
  if(debug_level)
     fprintf(stderr,"da_done %d  da_loc %d\n", da_done, da_location);
  if((s = get_playing_signal()) && (s->views))
    v = s->views;
  if(stop_da(v))
     move_view_to_loc(v,da_loc);

  return;
}

/*********************************************************************/
/* If a command file is open and paused and the command file name
   has not been changed, resume reading from the (already open)
   stream.  If the name has been changed, close the open file, if
   any, and open and begin reading the new file.
*/
void
command_proc(item, event)
    Panel_item	item;
    Event	*event;
{
  extern char	commandname[];
  extern int	command_paused;
  extern char	*dispatch();

  char    string[501];

  if (debug_level)
    (void) fprintf(stderr, "command_proc: function entered\n");

  if((!command_paused) && fp_command) {
    fclose(fp_command);
    fp_command = NULL;
    commandname[0] = 0;
  }

  strcpy(string, (char*)panel_get_value(item));

  if(*string != '@') {		/* it's a direct command */
    if(*string)
      (void) dispatch(receiver_prefixed(string));
    /* Need to restore the actual command file name
       (because of a direct command during pause)? */
    if(command_paused && *commandname) {
      sprintf(string,"@%s",commandname);
      xv_set(newControl_item, PANEL_VALUE, string, 0 );
    }
    return;
  }

  if(!command_paused) {
    strip_newline_at_etc(commandname,string);
    if(strcmp(commandname,"stdin")) {
      char *cmd_res;

      cmd_res = FIND_WAVES_COMMAND(NULL,commandname);

      if (cmd_res == NULL) {
	sprintf(notice_msg, "Couldn't find readable command file %s",
		commandname);
        show_notice(1,notice_msg);
	fp_command = NULL;
      } else {
	strcpy(commandname,cmd_res);
	sprintf(string,"@%s",commandname);
	xv_set(newControl_item, PANEL_VALUE, string, 0 );
	fp_command = fopen(commandname,"r");
	free(cmd_res);
      }
    }
    else
      fp_command = stdin;

    command_line = 0;
  } else {			/* it WAS in the paused state */
    if(strcmp(string+1,commandname)) { /* start a new file? */
      char *cmd_res;
      if(fp_command) {
	fclose(fp_command);
	fp_command = NULL;
	commandname[0] = 0;
      }
      strcpy(commandname,string+1);
      cmd_res = FIND_WAVES_COMMAND(NULL,commandname);
      if (cmd_res == NULL) {
	sprintf(notice_msg, "Couldn't find readable command file %s",
		commandname);
        show_notice(1,notice_msg);
	fp_command = NULL;
      } else {
	strcpy(commandname,cmd_res);

	sprintf(string,"@%s",commandname);
	xv_set(newControl_item, PANEL_VALUE, string, 0 );
	fp_command = fopen(commandname,"r");
	free(cmd_res);
	command_line = 0;
      }
    }
    command_paused = FALSE;	/* toggle on (in either case) */
  }

  if(fp_command) {
    char *tst, *fgets(), *clobber_cr();
    while((tst = clobber_cr(fgets(string,501,fp_command))) ||
	  (fp_command == stdin)) {
      if(tst) {
	command_line++;
	if(*string != '#')
	  (void) dispatch(receiver_prefixed(string));
	if(!fp_command) {
	    command_paused = FALSE;
	    return;
	}
	if(command_paused) return;
      } else {
	meth_sleep(&program,"seconds .2");
	return;
      }
    }
    if(!command_paused && (fp_command != stdin)) {
      fclose(fp_command);
      fp_command = NULL;
      commandname[0] = 0;
    }
  }
  command_paused = FALSE;
}

/*********************************************************************/
int
pause_proc(item, event)
    Panel_item	item;
    Event	*event;
{
    extern int	command_paused;

    command_paused = TRUE;

    return XV_OK;
}

/*********************************************************************/
int
resume_proc(item, event)
     Panel_item item;
     Event *event;
{

  resume_ipc_if_stepping();

  command_proc(newControl_item, NULL);

  check_ipc_response_pending("ok");

  return XV_OK;
}

/*********************************************************************/
void
quit_proc(item, event)
     Panel_item item;
     Event *event;
{
  cleanup();
  check_ipc_response_pending("nogood");
  kill_proc();
}

/*************************************************************************/
void
newText_proc(item, event)
     Panel_item item;
     Event *event;
{
  FILE *fdt, *fopen();
  char name[MAXPATHLEN], next[MAXPATHLEN], *get_output_file_names(), *cp;
  int n;

  /* Output filename (files to be created/overwritten) entry handler: */
  /* Proposals for new output filenames are generated by:
     (1) explicit type-in of a complete name (no leading @ or trailing ..)
     (2) auto-increment of numbers embedded in a filename
          (optional trailing ..)
     (3) reading a file list from a text file; elements may optionally
         have trailing ..
     (4) if the filename has a trailing .. an extention will be substituted
         for the final .; the extension will be determined from a table
	 which relates signal types (as specified in Signals.h) to extensions.
	 This table will be compiled in for now...
     (5) If the filename does not have trailing .. the above extensions will
         be appended directly to the name if the name does not already
	 have the correct extension (as .ext) for the file type being
	 generated.
     (6) If the filename is preceded by @ the file will be read as a source
         of actual output filenames.
     (7) As new output files are generated, their pathnames will be added
         to a list which may be browsed and accessed as a source of input
	 filenames for display and signal splicing.
  */

  *next = 0;
  sscanf((char*)panel_get_value(item),"%s %s",name,next);

  expand_name(name, name);

  if(item == outputFile_item) {
    strcpy(outputname, name);
    if(! (cp = get_output_file_names(outputname, next))) {
      *outputname = 0;
      xv_set(item, PANEL_VALUE, outputname, 0);
      return;
    }
    update_filename_display(cp, cp);
    return;
  }

  if(item == newControl_item) {
    command_proc(item, event);
    return;
  }

  /* Generic handler for receivers of existing filenames. */
  if(its_a_partial_pathname(name)) {
    fix_path_end(name);
    select_from_alternatives(name,item);
    return;
  }

/* The following items will receive the results from the generic handler
   (above) if the target location (results_to) is specified in
   select_from_alternatives() in menus.c */

  if(item == newFile_item) {
    (void)apply_waves_input_path(name,name);
    xv_set(item, PANEL_VALUE, name, 0);
    create_new_signal_view(name);
  }

  if(item == overlay_item) {
    if(*next)
      expand_name(next, next);
    (void)apply_waves_input_path(name,name);
    xv_set(item, PANEL_VALUE, name, 0);
    setup_overlay(objectname, name, next);
  }

  return;
}

/************************************************************************/
Notify_value
kill_program(canvas, status)
     Canvas canvas;
     Destroy_status status;
{
#ifndef NO_LIC
  extern void lm_quit();
#endif

  stop_da(NULL);
  meth_detach(&program,NULL);
#ifndef NO_LIC
  lm_quit();
#endif
  exit(0);
}

/************************************************************************/
/* When a canvas is destroyed, all views which refer to it must also
   be destroyed.  If the destroyed view is a signal's only view, the
   signal is also destroyed.  If the signal is an object's only signal,
   the object is unlinked and destroyed.  This is implemented assuming that
   different objects do not share canvases.  This doesn't seem too restrictive... */
Notify_value
kill_signal_view(frame, status)
    Frame           frame;
    Destroy_status  status;
{
    Canvas canvas;

    if (status == DESTROY_CLEANUP)
    {
	canvas = (Canvas) xv_get(frame, WIN_CLIENT_DATA);
	clobber_repaint_entry(canvas);
	if (! free_canvas_views(canvas))
	  printf("Problems with free_canvas_views() in kill_signal_views()\n");
	xv_set(canvas, WIN_CLIENT_DATA, NULL, 0);
    }
    return(notify_next_destroy_func(frame, status));
}


Notify_value
destroy_func(client, status)
    Notify_client client;
    Destroy_status status;
{

#define DEBUG(x) if(debug_level > x) fprintf

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
