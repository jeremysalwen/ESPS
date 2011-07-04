/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin
 * Checked by:
 * Revised by:  Alan Parker, Rod Johnson, John Shore, David Talkin
 *
 *  xmethods.c
 *  implementation of methods accessible by external processes
 */

static char *sccs_id = "@(#)xmethods.c	1.39 28 Oct 1999 ERL/ATT";
#include <Objects.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/unix.h>
#include <xview/scrollbar.h>

#ifndef hpux
#include <sys/param.h>
#else
#define MAXPATHLEN 1024
#endif

#define ATTACHING 0x4000
#define SLEEPING  0x2000

#define HI_SPECT_LIM  127-22
  /* This fudge factor of 22 is to translate the image_clip semantics
     for monochrome dither to a reasonable value for color/greyscale.
     Without this fudge factor, all image_clip entries in various
     environments worldwide would need to be changed!) */

extern char *checking_selectors(), *savestring();
extern double image_clip, image_range;
extern char *dispatch(), *receiver_prefixed(), *get_next_item();

#define CHECK_QUERY(a,b) { char *cret; if((cret = checking_selectors(a,b))) return(cret); }

static FILE *fopen_command_file();
extern Selector g1;
Signal *find_signal();
Selector **get_changed_items();
Object *find_object();

char *meth_add_espst(), *meth_add_espsf(), *meth_add_operator(),
     *meth_add_espsn(), *meth_start_attachment(), *meth_send(),
     *meth_add_keymap(), *meth_delete_keymap(), *meth_dump_keymaps();
char *meth_enable_server(),  *meth_disable_server(),
       *meth_dump_add_ops(), *meth_save_menus(),
       *meth_save_panels(), *meth_setenv(), *meth_stop_play();
void	    menu_change();
char *meth_get_attach_list();

extern void update_control_panel_names(), menu_set_blowup_op(),
  distribute_options_settings(), set_default_header(),
  set_old_sphere_format();

char *meth_print_setup();

char *meth_open_ctlwin(), *meth_close_ctlwin();

char *meth_auto_plotlims();
char *meth_print_graphic();
char *meth_print_ensemble();


extern char *build_filename();

extern int  debug_level; 
extern Frame daddy; /* the xwaves control panel */

extern char ok[], null[];	/* in message.c */


/* THE FOLLOWING GLOBALS MAY BE SET FROM <a profile file> */

extern char inputname[],	/* name of signal input file */
     outputname[] ,	/* name of signal output file */
     objectname[] ,	/* user-defined object name */
     commandname[] ,	/* control script */
     funcname[] ,	/* linkage to external process */
     def_left_op[],
     def_middle_op[],
     def_move_op[],
     def_sleft_op[],
     def_smiddle_op[];

extern int
    line_type;			/* for plotting waveforms */

/* some display creation globals */
extern double ref_size,		/* size for def. waveform disp.*/
              ref_step,		/* amount to step in file */
	      ref_start;	/* where reference window display begins */
extern int def_w_height,	/* dim. of waveform window */
           def_w_width;

/* window position parameters */
extern int  next_x,		/* upper left-hand corner of new window */
            next_y;

extern int w_verbose;

/* END OF GLOBALS SET FROM CONFIGURATION FILE */

extern Panel_item quit_item, newFile_item, newObj_item, newFunct_item;
extern Object program;
extern Frame daddy;

int command_paused = 0;		/* pause flag for command file */
typedef struct com_stk {
  char name[200];
  int line_num;
  struct com_stk *next;
} Com_stack;

Com_stack *command_stack = NULL;

int	new_width=0, new_height=0; /* arguments to make command */

#ifdef STARDENT_3000
extern char WAVES_MISC[];
#endif

/*********************************************************************/
char *
get_receiver(str)
     char *str;
{
  Object *ob;
  static  char name[200];

  ob = &program;
  if(str && strlen(str) && sscanf(str,"%s",name) == 1) {
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

/*********************************************************************/
char *
get_receiver_name(ob)
     Object *ob;
{
  return(ob->name);
}

/*********************************************************************/
char *
get_methods(ob)
     Object *ob;
{
  extern Methods bmeth1;
  
  if(ob) return((char*)(ob->methods));
  else
    return((char*)&bmeth1);
}

/***********************************************************************/
char *
augment_external_command(s,v,mess)
     Signal *s;
     View *v;
     char *mess;
{
  if(mess && s && (v || (v = s->views)) && v->canvas) {
    double start, seccm;
    Frame frm = (Frame)xv_get(v->canvas, XV_OWNER);
    Rect  rec;
    char info[200];

    frame_get_rect(frm, &rec);
    
    seccm = *(v->x_scale);
    start = v->start_time;
    if(debug_level)
      fprintf(stderr,"augment_external_command: loc_x:%d loc_y:%d height:%d width:%d start:%f sec/cm:%f\n",
            rec.r_left + *(v->x_offset),rec.r_top,rec.r_height ,
            rec.r_width - *(v->x_offset),start,seccm);
    sprintf(info," loc_x %d loc_y %d height %d width %d start %f sec/cm %f",
            rec.r_left + *(v->x_offset),rec.r_top,rec.r_height,
	    rec.r_width - *(v->x_offset),start,seccm);
    strcat(mess,info);
    return(mess);
  }
  show_notice(1, "Bad args to augment_external_command().");
  return(NULL);
}

/***********************************************************************/
char *
meth_return(ob,str)
     Object *ob;
     char *str;
{
  if(ob && str && *str) {
    int id;
    sscanf(str,"%d",&id);
    do_return_callback(id,get_next_item(str));
    return(ok);
  }
  return(null);
}

/***********************************************************************/

char *
meth_delete_item(ob, str)
    Object	*ob;
    char	*str;
{
    static char	menu[50];
    static char	name[100];
    static Selector
		a1 = {"menu",	"%s",	    menu,   NULL},
		a0 = {"name",	"#strq",    name,   &a1};

    *menu = '\0';
    *name = '\0';
    CHECK_QUERY(str, &a0)
      get_args(str, &a0);

    if (*name)
    {
	menu_change(name, "", XV_NULL, menu);
	return ok;
    }
    else
    {
	return null;
    }
}

/***********************************************************************/

char *
meth_delete_all(ob, str)
    Object	*ob;
    char	*str;
{
    static char menu[50];
    static Selector
		a0 = {"menu",	"%s",	    menu,   NULL};

    *menu = '\0';
    CHECK_QUERY(str, &a0)
      get_args(str, &a0);
    menu_clear(menu);
    return ok;
}

static void *ipc_callback_data = NULL;
void *in_a_ipc_dispatch();

/***********************************************************************/
char *
meth_pause(o,str)
     Object *o;
     char *str;
{
  CHECK_QUERY(str,NULL)
  command_paused = TRUE;
  ipc_callback_data = in_a_ipc_dispatch();
  return(ok);
}

/***********************************************************************/
char *
meth_detach(o,str)
     Object *o;
     char *str;
{
  static char name[NAMELEN];
  static Selector s1 = {"function", "%s", name, NULL};

  strcpy(name,"all");
  CHECK_QUERY(str,&s1)
    get_args(str,&s1);
  terminate(name);
  return(ok);
}
    
/***********************************************************************/
char *
meth_attach(o,str)
     Object *o;
     char *str;
{
  static char fname[NAMELEN];
  static Selector a1 = {"function", "%s", fname, NULL};

  CHECK_QUERY(str,&a1)
    if(get_args(str,&a1))
    start_external_function(fname);

  return(ok);
}

  
/***********************************************************************/
View *
get_first_view(o)
     Object *o;
{
  Signal *s;
  View *v = NULL;
  
  if(o) {
    s = (Signal*)o->signals;
    while(s && (!(v = s->views))) s = s->others;
  }
  return(v);
}

/***********************************************************************/
char *
meth_align(o,str)
     Object *o;
     char *str;
{
  extern char inputname[];
  static char file[MAXPATHLEN];
  static Selector a1= {"file", "%s", file, NULL};
  char *cp;
  Signal *s;

  *file = 0;
  /* Try to get the name of a data view to use as alignment model */
  CHECK_QUERY(str, &a1)
    if(get_args(str,&a1) && *file) {
    (void) apply_waves_input_path(file,file);
    cp = file;
  } else
    if(o && (o->signals)) cp = o->signals->name;

  if(o && (s = find_signal(o,cp)) && s->views &&
     do_align_views(o, s->views))
    return(ok);

  return(null);
}

/***********************************************************************/
char *
meth_active_channels(o,str)
     Object *o;
     char *str;
{
  extern char active_ids[], active_numbers[];
  Signal *s;
  View *v, *find_view();
  static char name[MAXPATHLEN], op[20];
  static Selector a0 = {"op", "%s", op, NULL},
                  a1 = {"file", "%s", name, &a0},
                  a2 = {"numbers", "#strq", active_numbers, &a1},
                  a3 = {"identifiers", "#strq", active_ids, &a2};

  *name = *active_numbers = *active_ids = 0;
  CHECK_QUERY(str, &a3)
    get_args(str,&a3);
  if (*name) 
    (void) apply_waves_input_path(name,name);

  if(*name && (s = find_signal(o,name)) && (v = find_view(s))) {
    if(*op && (strcmp(op, "set"))) {
      if(!strcmp(op,"clear")) {
	clear_active_channels(v);
      } else
	if(!strcmp(op,"add")) {
	  add_active_channels(v);
	}
    }
    set_active_channels(v);
    redoit(v->canvas);
    *active_numbers = *active_ids = 0;
    return(ok);
  }
  *active_numbers = *active_ids = 0;
  return(null);
}

/***********************************************************************/
char *
meth_move_markers(o,str)
     Object *o;
     char *str;
{
  static double time;
  static int do_left;
  static Selector a2 = {"time", "%lf", (char*)&time, NULL },
                  a1 = {"do_left", "%d", (char*)&do_left, &a2};
  View *v;

  CHECK_QUERY(str, &a1)
  if(v = get_first_view(o)) {
    time = 0.0;
    do_left = 1;
    
    if(get_args(str, &a1)) {
      move_markers(v, time, do_left);
      return(ok);
    }
  }
  return(null);
}

/***********************************************************************/
char *
meth_move_cursors(o,str)
     Object *o;
     char *str;
{
  static double time, freq;
  static int chan;
  static Selector a2 = {"time", "%lf", (char*)&time, NULL },
                  a1b = {"yval", "%lf", (char*)&freq, &a2},
                  a1 = {"frequency", "%lf", (char*)&freq, &a1b};

  View *v;
  
  CHECK_QUERY(str, &a1)
  if((v = get_first_view(o))) {
    time = v->cursor_time;
    freq = v->cursor_yval;

    if(get_args(str, &a1)) {
      move_cursors(v,time,freq);
      return(ok);
    }
  }
  return(null);
}

/***********************************************************************/
char *
meth_remap_colors(o,str)
     Object *o;
     register char *str;
{
  static double threshold;
  static char name[MAXPATHLEN];
  static  Selector a3 = {"file", "%s", name, NULL},
                   a2b = { "image_clip", "%lf", (char*)&image_clip, &a3 },
                   a1b = { "image_range", "%lf", (char*)&image_range, &a2b },
                   a2 = { "threshold", "%lf", (char*)&threshold, &a1b },
                   a1 = { "range", "%lf", (char*)&image_range, &a2 };
  Signal *s;
  View *v;
  char *p;
  
  CHECK_QUERY(str,&a1)
  threshold = -234.5;
  *name = 0;
  for(s = (Signal*)o->signals; s; s = s->others) /* find a spectrogram */
    if(s->views && ((s->type & SPECIAL_SIGNALS) == SIG_SPECTROGRAM)) break;
    get_args(str,&a1);
  if (*name) (void) apply_waves_input_path(name,name);
  if(s && ! *name) strcpy(name, s->name);
  if(*name && (s = find_signal(o,name)) && (v = s->views) &&
     v->canvas) {
    if(threshold != -234.5)
      image_clip = HI_SPECT_LIM - (threshold + image_range);
    limit_z_range(MAX_CMAP_SIZE - (cmap_size-CMAP_RESERVED));
    cmap_spect(v->canvas);
    return(ok);
  }
  return(null);
}

/***********************************************************************/
limit_z_range(total_range)
     int total_range;
{
  if(image_clip < 0)
    image_clip = 0;
  if(image_range < 1)
    image_range = 1;
  if((image_clip + image_range) >= total_range)
    image_clip = total_range - image_range - 1;
}

/***********************************************************************/
char *
meth_spectrogram(o,str)
     Object *o;
     register char *str;
{
  static double start, end, threshold;
  static char name[NAMELEN], type[200], output[NAMELEN];
  extern int next_x, next_y;
  extern double image_clip, image_range;
  extern char inputname[];
  static  Selector   b2b = { "image_clip", "%lf", (char*)&image_clip, NULL },
                     b1b = { "image_range", "%lf", (char*)&image_range, &b2b },
                     b2 = { "threshold", "%lf", (char*)&threshold, &b1b },
                     b1 = { "range", "%lf", (char*)&image_range, &b2 },
                     a7 = {"width", "%d", (char *) &new_width, &b1},
                     a6b = {"height", "%d", (char *) &new_height, &a7},
                     a6 = {"file", "%s", name, &a6b},
                     a5 = {"loc_y", "%d", (char*)&next_y, &a6},
                     a4 = {"loc_x", "%d", (char*)&next_x, &a5},
                     a3b = {"type", "%s", type, &a4},
                     a3 = {"output", "%s", output, &a3b},
                     a2 = { "start", "%lf", (char*)&start, &a3 },
                     a1 = { "end", "%lf", (char*)&end, &a2 };
  double temp;
  Signal *s;

  strcpy(type, "wideband");
  *name = 0;
  *output = 0;
  start = -10.0;
  end = -1.0;
  new_width = new_height = 0;
  threshold = -234.5;
  CHECK_QUERY(str,&a1)
    get_args(str,&a1);

  if(*name) {
    (void)apply_waves_input_path(name,name);
    s = find_signal(o,name);
  }
  else  /* Try to find a .sd or .d file or the first  P_SHORTS file. */
    if(!((s = find_signal(o,new_ext(((Signal*)o->signals)->name,"d"))) ||
	 (s = find_signal(o,new_ext(((Signal*)o->signals)->name,"sd"))))) {
      s = o->signals;
      while(s) {
	if(IS_GENERIC(s->type) && ((s->type & VECTOR_SIGNALS) == P_SHORTS) &&
	   (s->dim == 1))
	  break;
	s = s->others;
      }
    }

  if(s) {
    if(start < 0.0)
      start = BUF_START_TIME(s);

    if(end < 0.0)
      end = BUF_END_TIME(s);

    if(start < (temp = SIG_START_TIME(s))) 
      start = temp;

    if(end > (temp = 	SIG_END_TIME(s)))
      end = temp;

    if(threshold != -234.5)
      image_clip = HI_SPECT_LIM - (threshold + image_range);

    limit_z_range(MAX_CMAP_SIZE - (cmap_size-CMAP_RESERVED));

    spectrogram(s, type, start, end, output);

/* Note that "spectrogram()" has the side effect of setting new_height
   and new_width to zero. */

    return(ok);
  }
  new_width = new_height = 0;
  return(null);
}

/***********************************************************************/
char *
meth_page(o,str)
     Object *o;
     register char *str;
{
  static double step;
  static char file[MAXPATHLEN];
  static  Selector a3 = {"file", "%s", file, NULL},
                   a2 = { "step", "%lf", (char*)&step, &a3 },
                   a1 = { "ref_step", "%lf", (char*)&ref_step, &a2 };
  Signal *s;
  
  step = 0.0;
  strcpy(file,inputname);
  CHECK_QUERY(str, &a1)
    get_args(str,&a1);
  if (*file) 
    (void)apply_waves_input_path(file,file);
  if((s = find_signal(o,file)) && s->views) {
    if(step == 0.0)
      step = s->views->page_step;
    s->views->lmarker_time += step;
    s->views->rmarker_time += step;
    page(s->views, s, s->views->start_time + step);
    return(ok);
  }
  return(null);
}

/***********************************************************************/
char *
meth_bracket_markers(o, str)
    Object	    *o;
    register char   *str;
{
    extern int	    h_spect_rescale;
    static double   start, end;
    static char	    file[MAXPATHLEN];
    static Selector a3 = {"file",   "%s",  file,	    NULL},
		    a2 = { "start", "%lf", (char *) &start, &a3},
		    a1 = { "end",   "%lf", (char *) &end,   &a2};
    Signal	    *s;

  
  start = end = -1.0;
  strcpy(file, inputname);
    CHECK_QUERY(str, &a1)
      get_args(str, &a1);

  if (*file) 
    apply_waves_input_path(file,file);
  
    if ((s = find_signal(o, file))
      && s->views && s->views->h_rescale)
  {
    if (start == -1.0) start = s->views->lmarker_time;
    if (end == -1.0) end = s->views->rmarker_time;
    if (start < end)
    {
	s->views->rmarker_time = end;
	s->views->lmarker_time = start;
	position_view(s->views, start, end);
	return(ok);
    }
  }
  return(null);
}

/***********************************************************************/
char *
meth_save_seg(o,str)
     Object *o;
     register char *str;
{
  static double start, end;
  static char name[MAXPATHLEN];
  static  Selector a3 = {"file", "%s", name, NULL},
  a2 = { "start", "%lf", (char*)&start, &a3 },
  a1b = {"outputname", "%s", outputname, &a2},
  a1 = { "end", "%lf", (char*)&end, &a1b };
  double duration;
  char next[200];
  Signal *s;
  View *v;
  
  *name = 0;
  start = -1.0;
  end = -1.0;

  CHECK_QUERY(str, &a1)
    get_args(str,&a1);

  if(*name && apply_waves_input_path(name, name) && (s = find_signal(o,name))) {
    if(start < end) { /* specified in command */
      if(start < s->start_time)
	start = s->start_time;
       if(end > SIG_END_TIME(s))
	 end = SIG_END_TIME(s);
      duration = end - start;
    } else {
      if((start == end) && (end == -1.0) && (v = s->views)) {
	duration = v->rmarker_time - v->lmarker_time;
	start = v->lmarker_time;
	end = v->rmarker_time;
      } else return(null);
    }
    if(save_segment(s,start,duration)) {
      get_output_file_names(outputname, next);
      update_filename_display(next);
      return(ok);
    }
  } else {
    if(! *name) strcpy(name,null);
    fprintf(stderr,"Can't find_signal(%s) in meth_save_segment().\n",name);
  }
  return(null);
}

/***********************************************************************/
char *
meth_play_seg(o, str)
    Object	    *o;
    char	    *str;
{
    Signal	    *s;
    Header	    *h;
    struct header   *espsh;

    static double   start, end, gain_0, gain_1;
    static int pchannel;
    static char	    name[MAXPATHLEN];
    static Selector a3 = {"file",  "%s",  name,		   NULL},
		    a2d = {"channel", "%d", (char *) &pchannel, &a3},
		    a2c = {"gain_1", "%lf", (char *) &gain_1, &a2d},
		    a2b = {"gain_0", "%lf", (char *) &gain_0, &a2c},
		    a2 = {"start", "%lf", (char *) &start, &a2b},
                    a1 = {"end",   "%lf", (char *) &end,   &a2};
  
    *name = 0;
    start = 0.0;
    end = 0.0;
    gain_0 = gain_1 = 1.0;
    pchannel = -1;

    CHECK_QUERY(str, &a1)
      get_args(str, &a1);


    if(*name) {
      (void)apply_waves_input_path(name,name);
	s = find_signal(o, name);
    }
    else
    {
        for (s = o->signals;
	     s && !(IS_GENERIC(s->type)
		    && ((s->type & VECTOR_SIGNALS) == P_SHORTS)
		    && playable_dimension(s->dim))
	       && !((h = s->header)
		    && h->magic == ESPS_MAGIC
		    && (espsh = h->esps_hdr)
		    && (espsh->common.type == FT_FEA)
		    && (espsh->hd.fea->fea_type == FEA_SD));
	     s = s->others)
        { }
    }

    if (s)
    {
	if (start < s->start_time) start = s->start_time;
	if (end <= start) end = s->start_time + s->file_size/s->freq;
	if(pchannel == 0)
	  gain_1 = 0.0;
	else
	  if(pchannel == 1)
	    gain_0 = 0.0;
	set_channel_gains(gain_0, gain_1);
	play_file(s, start, end);
	return(ok);
    }
    return(null);
}

/***********************************************************************/
char *
meth_stop_play(o, str)
    Object	    *o;
    char	    *str;
{
    Signal	    *s, *get_playing_signal();
    Header	    *h;
    struct header   *espsh;
    extern int da_stop_pos_view;
    int repo_hold;

    static char	    name[MAXPATHLEN];
    static int      reposition;
    static Selector a2 = {"reposition_view",  "%d",  (char *) &reposition, NULL},
                    a1 = {"file",  "%s",  name,	&a2};
  
    *name = 0;
    reposition = da_stop_pos_view;
    CHECK_QUERY(str, &a1)
      get_args(str, &a1);


    if(*name) {
      (void)apply_waves_input_path(name,name);
	s = find_signal(o, name);
    }
    else
      if( ! (s = get_playing_signal())) {
        for (s = o->signals;
	     s && !(s->views && IS_GENERIC(s->type)
		    && ((s->type & VECTOR_SIGNALS) == P_SHORTS)
		    && playable_dimension(s->dim))
	       && !(s->views && (h = s->header)
		    && h->magic == ESPS_MAGIC
		    && (espsh = h->esps_hdr)
		    && (espsh->common.type == FT_FEA)
		    && (espsh->hd.fea->fea_type == FEA_SD));
	     s = s->others)
        { }
      }

    if (s && s->views) {
      repo_hold = da_stop_pos_view;
      da_stop_pos_view = reposition;
      handle_da_interruption(s->views);
      da_stop_pos_view = repo_hold;
      return(ok);
    }
    return(null);
}

/***********************************************************************/
char *
meth_play_window(o,str)
     Object *o;
     register char *str;
{
  double start, end;
  static char name[MAXPATHLEN];
  static  Selector a1 = {"file", "%s", name, NULL};
  double temp;
  Signal *s;
  View *v;
  
  *name = 0;
  start = 0.0;
  end = 0.0;

  CHECK_QUERY(str, &a1)
    get_args(str,&a1);

  if(*name) {
      (void)apply_waves_input_path(name,name);
      s = find_signal(o,name);
    }
  else  /* Try to find a .d file or the first available P_SHORTS file. */
    if(!(s = find_signal(o,new_ext(((Signal*)o->signals)->name,"d")))) {
      s = o->signals;
      while(s) {
	if(IS_GENERIC(s->type) && ((s->type & VECTOR_SIGNALS) == P_SHORTS) &&
	   (s->dim == 1))
	  break;
	s = s->others;
      }
    }

  if(s) {
    if((v = s->views)) {
      start = v->start_time;
      end = ET(v);
    } else {
      start = s->start_time;
      end = SIG_END_TIME(s);
    }
    play_file(s, start, end);
    return(ok);
  }
  return(null);
}

/***********************************************************************/
char *
meth_branch_to_script(o, str)
     Object *o;
     char *str;
{
  extern FILE *fp_command;
  char tmp[500];
  extern Panel_item newControl_item;

  CHECK_QUERY(str,&g1)
    if(get_args(str,&g1) && *commandname) {
    if(fp_command)
      fclose(fp_command);
    
    if(!(fp_command = fopen_command_file(commandname,"r")))
      *commandname = 0;
    sprintf(tmp,"@%s",commandname);
    panel_set_value(newControl_item,tmp);
    if(*commandname)
      return(ok);
  }
  return(null);
}

/***********************************************************************/
FILE *pop_command_file()
{
  extern FILE *fp_command;
  extern int command_line;
  Com_stack *ct;
  char junk[501];
  extern Panel_item newControl_item;

  if(fp_command && (fp_command != stdin))
    fclose(fp_command);
  *commandname = 0;
  command_line = 0;
  fp_command = NULL;
  *junk = 0;
  if((ct = command_stack)) {
    if(!strcmp(ct->name,"stdin"))
      fp_command = stdin;
    else
      fp_command = fopen(ct->name,"r");
    if(fp_command) {
      for ( ; command_line < ct->line_num; command_line++)
	fgets(junk, 500, fp_command);
      strcpy(commandname,ct->name);
      sprintf(junk,"@%s",ct->name);
      command_stack = ct->next;
      free((char *)ct);
    }
  }
  panel_set_value(newControl_item,junk);
  return(fp_command);
}

/***********************************************************************/
char *
meth_call_script(o, str)
     Object *o;
     char *str;
{
  extern FILE *fp_command;
  extern int command_line;
  char tmp[500];
  extern Panel_item newControl_item;

  CHECK_QUERY(str,NULL)
  if(push_command_file()) {
    strcpy(tmp,commandname);
    *commandname = 0;
    if(get_args(str,&g1) && *commandname && strcmp(tmp,commandname)) {
	if(!(fp_command = fopen_command_file(commandname,"r")))
	  *commandname = 0;
    }
    command_line = 0;
    sprintf(tmp,"@%s",commandname);
    panel_set_value(newControl_item,tmp);
    if(! *commandname)
      pop_command_file();
    else
      return(ok);
  }
  return(null);
}

/***********************************************************************/
/* Only necessary because of bugs in mbuttons and send_xwaves. */
strip_trailing_spaces(cmd)
     char *cmd;
{
  int i;

  for (i = strlen(cmd) - 1; i >= 0 && cmd[i] == ' '; i--)
    cmd[i] = '\0';
}

/***********************************************************************/
static char *find_double_colon(str)
     char *str;
{
  if(str) {
    while(*str) {
      if((*str == ':') && (str[1] == ':'))
	return(str);
      str++;
    }
  }
  return(NULL);
}
	
/***********************************************************************/
char *separate_wcommands(cmd)
     char *cmd;
{
  char *cp, *index(), tomatch;

  if(cmd) {
    while((*cmd == ' ') || (*cmd == '	'))
      *cmd++;
    if(((tomatch = *cmd) == '"') || (tomatch == '\'')) { /* first nonblank quote? */
      cp = ++cmd;
      while(*cp) {
	if(*cp == tomatch) {
	  *cp = ' ';
	  break;
	}
	cp++;
      }
    }
    if(*cmd && (cp = find_double_colon(cmd))) {
      char *cpp = cmd;

      for( ; cpp < cp;)
	if(((tomatch = *cpp++) == '"') || (tomatch == '\'')) {
	  for(; *cpp;)
	    if(*cpp++ == tomatch)
	      return(separate_wcommands(cpp));
	}
      if(cpp >= cp) {
	*cp++ = 0;
	*cp++ = 0;
	if(*cp)
	  return(cp);
      }
    }
  }
  return(NULL);
}

/***********************************************************************/
char *exec_waves(cmd)
    char    *cmd;
{
  extern FILE		*fp_command;
  extern Panel_item	newControl_item;
  extern int command_line;

  if(debug_level)
    fprintf(stderr,"exec_waves() entered with |%s|\n",cmd);

  if (!cmd || !*cmd)
    return(null);
    
  strip_trailing_spaces(cmd);

  if (*cmd != '@') {
    char *nextcom, *comstore, *rval;

    comstore = savestring(cmd);
    cmd = comstore;
    while (nextcom = separate_wcommands(cmd)) {
      dispatch(receiver_prefixed(cmd));
      cmd = nextcom;
    }
    rval = dispatch(receiver_prefixed(cmd));
    free(comstore);
    return(rval);
  } else {
    if(!*(cmd + 1))
      return(null);
    if(push_command_file()) {
      strcpy(commandname, cmd + 1);
      fp_command = fopen_command_file(commandname, "r");
      if (!fp_command)
	pop_command_file();
      else {
	command_line = 0;
	panel_set_value(newControl_item, cmd);
	/* command_proc(newControl_item, EVENT_NULL); */
      }
      return(ok);
    } else {
      if (fp_command)
	fclose(fp_command);
      strcpy(commandname, cmd + 1);
      fp_command = fopen_command_file(commandname, "r");
      if (!fp_command) *commandname = '\0';
      panel_set_value(newControl_item, cmd);
      command_proc(newControl_item, EVENT_NULL);
      return(ok);
    }
  }
}

/***********************************************************************/
char *
meth_return_from_script(o, str)
     Object *o;
     char *str;
{
  extern FILE *fp_command;
  extern int command_line;
  char tmp[500];
  extern Panel_item newControl_item;

  CHECK_QUERY(str,NULL)
  if(pop_command_file())
    return(ok);
  else
    return(null);
}

/***********************************************************************/
char *menu_op_access_proc(o, str)
     Object *o;
     char *str;
{
  if(o && str && *str) {
    Signal *s;
    Canvas canvas;
    View *v;
    int failed = FALSE;
    extern Object program;
    
    if(o == &program) {
      sprintf(notice_msg, "Unknown command sent to xwaves:\n%s.", str);
      show_notice(1, notice_msg);
      return(null);
    }
    if((s = o->signals) && (v = s->views) && (canvas = s->views->canvas)) {
      view_do_op(v,str);
      return(ok);
    }
  }
  sprintf(notice_msg, "Bad args to xwaves command processor:\n%s.", str);
  show_notice(1, notice_msg);
  return(null);
}

/***********************************************************************/
install_dispatch_hook()
{
  install_default_execute_proc(menu_op_access_proc);
}

/***********************************************************************/
push_command_file()
{
  extern FILE *fp_command;
  extern int command_line;
  extern Panel_item newControl_item;

  if(*commandname && fp_command) {
    Com_stack *ct;
    if((ct = (Com_stack*)malloc(sizeof(Com_stack)))) {
      if(fp_command != stdin)
        fclose(fp_command);
      else
	command_line = 0;
      fp_command = NULL;
      strcpy(ct->name,commandname);
      ct->line_num = command_line;
      command_line = 0;
      ct->next = command_stack;
      command_stack = ct;
      *commandname = 0;
      panel_set_value(newControl_item,commandname);
      return(TRUE);
    } else
      fprintf(stderr,"Can't allocate memory in push_command_file()\n");
  }
  return(FALSE);
}

/***********************************************************************/
char *
meth_close_frame(o, str)
     Object *o;
     char *str;
{
  static char file[MAXPATHLEN];
  static int invisible = 0;
  static Selector a1 = {"file", "%s", file, NULL},
                  a2 = {"invisible", "%d", (char*)&invisible, &a1};
  View *v;
  Signal *s;
  Frame fr;
  
  invisible = 0;
  CHECK_QUERY(str, &a2)
    get_args(str, &a2);

  if(*file && apply_waves_input_path(file,file)
     && (s = find_signal(o,file)) && (v = s->views) && v->canvas) {
      
      fr = (Frame)xv_get(v->canvas,XV_OWNER);
      xv_set(fr, FRAME_CLOSED, TRUE, 0);
      if(invisible)
	xv_set(fr, XV_SHOW, FALSE, 0);
      return(ok);
  }
  return(null);
}

/***********************************************************************/
char *
meth_auto_plotlims(o, str)
     Object *o;
     char *str;
{
  extern double       plot_max;	/* forced max and min for every trace */
  extern double       plot_min;
  char *meth_set_attr();

  CHECK_QUERY(str, NULL)
  return(meth_set_attr(o,"all t plot_max 0 plot_min 0"));
}

/***********************************************************************/
char*
meth_print_graphic(o, str)
     Object *o;
     char *str;
{
  static char file[MAXPATHLEN];
  static Selector a1 = {"file", "%s", file, NULL};
  View *v;
  Signal *s;
  Frame fr;
  
  CHECK_QUERY(str, &a1)
    get_args(str, &a1);

  if(*file && apply_waves_input_path(file,file)
     && (s = find_signal(o,file)) && (v = s->views) && v->canvas) {
	e_print_graphic(v->canvas, (Event *)NULL, (caddr_t)NULL);
  }
  return(null);
}
      
/***********************************************************************/
char *
meth_print_ensemble(o, str)
    Object  *o;
    char    *str;
{
    CHECK_QUERY(str, NULL);
    do_print_ensemble(o);
    return null;
}
      
/***********************************************************************/
char *
meth_open_frame(o, str)
     Object *o;
     char *str;
{
  static char file[MAXPATHLEN];
  static Selector a1 = {"file", "%s", file, NULL};
  View *v;
  Signal *s;
  Frame fr;
  
  CHECK_QUERY(str, &a1)
    get_args(str, &a1);

  if(*file && apply_waves_input_path(file, file) 
     && (s = find_signal(o,file)) && (v = s->views) && v->canvas) {
    fr = (Frame)xv_get(v->canvas,XV_OWNER);
    xv_set(fr, FRAME_CLOSED, FALSE, 0);
    xv_set(fr, XV_SHOW, TRUE, 0);
    return(ok);
  }
  return(null);
}

void reset_color_maps()
{
  extern Object program;
  extern use_static_cmap;

  setup_colormap();
  if (!use_static_cmap) 
    install_colormap(&program);
  cmap_spect(NULL);
}

/***********************************************************************
  This list defines procedures to be called when any of the named items
  are changed via a "set" command.
***********************************************************************/
static Menuop   si1 = {"colormap", reset_color_maps, NULL, NULL},
                si2 = {"image_range", reset_color_maps, NULL, &si1},
                si3b = {"image_clip", reset_color_maps, NULL, &si2},
                si3 = {"options", distribute_options_settings, NULL, &si3b},
                si4 = {"inputname", update_control_panel_names, NULL, &si3},
                si5b = {"objectname", update_control_panel_names, NULL, &si4},
                si5 = {"name", update_control_panel_names, NULL, &si5b},
                si6 = {"overlayname", update_control_panel_names, NULL, &si5},
                si7 = {"outputname", update_control_panel_names, NULL, &si6},
                si8c = {"old_sphere_format", set_old_sphere_format, NULL, &si7},
                si8b = {"def_header", set_default_header, NULL, &si8c},
                si8 = {"blowup_op", menu_set_blowup_op, NULL, &si8b};

/***********************************************************************/
post_set_process()
{
  Selector **mo = get_changed_items();

  while(*mo) {
    Menuop *mo2 = &si8;
    while(mo2) {
      if(!strcmp((*mo)->name,mo2->name)) {
	mo2->proc((*mo)->dest);
	break;
      }
      mo2 = mo2->next;
    }
    mo++;
  }
}
    
/***********************************************************************/
char *
meth_set_attr(o, str)
     Object *o;
     char *str;
{
  extern Selector gm1;
  extern Object program;
  static char do_all[20];
  /* Put things in this list that you don't ever want to see printed out! */
  static Selector  a0 = {"all", "%s", do_all, &gm1};
  char tmp[500];
  char *set_view_attributes();

  *do_all = 0;

  if(o == &program) {
    CHECK_QUERY(str,&a0)
      if(get_args(str,&a0)) {
      post_set_process();
      if(*do_all)
	change_objects_globally(get_changed_items());
      return(ok);
    }
  } else { /* it's a set command directed to a particular ensemble */
    return(set_view_attributes(o, str));
  }
  sprintf(notice_msg, "Problems in set( %s ).", str);
  show_notice(1, notice_msg);
  return(null);
}

/***********************************************************************/
char *
  meth_call_operator(o, str)
Object *o;
char *str;
{
  static char file[MAXPATHLEN], op[MES_BUF_SIZE];
  static double yval, time;
  static int chan;
  static Selector a1 = {"c_time", "%lf", (char*)&time, NULL},
  a2 = {"c_yval", "%lf", (char*)&yval, &a1},
  a3 = {"c_chan", "%d", (char*)&chan, &a2},
  a4b = {"op", "#strq", op, &a3},
  a4 = {"command", "#strq", op, &a4b},
  a5 = {"file", "%s", file, &a4};
  Signal *s;
  View *v = NULL;
  
  *file = *op = 0;
  time = yval = -1234.5;
  chan = -1;
  CHECK_QUERY(str, &a5)
    if(get_args(str, &a5) && *op) {
    if( ! (*file && apply_waves_input_path(file, file) &&
	   (s = find_signal(o,file)))) {
      s = o->signals;
      while(s) {
	if((v = s->views) && v->sig && v->canvas) {
	  strcpy(file, v->sig->name);
	  break;
	}
	v = NULL;
	s = s->others;
      }
    } else
      v = s->views;
    if( !( s && v && v->sig && v->canvas)) {
      sprintf(notice_msg,  "Bad command sent to meth_op: %s.", str);
      show_notice(1, notice_msg);
      return(null);
    }
    if(chan >= 0)
      v->cursor_channel = chan;
    link_views(v);
    view_do_op(v,op);
    return(ok);
  } else {
    sprintf(notice_msg,  "Bad arguments sent to meth_op: %s.", str);
    show_notice(1, notice_msg);
  }
  return(null);
}

/***********************************************************************/
char *
  meth_overlay(o, str)
Object *o;
char *str;
{
  static char onfile[MAXPATHLEN], file[MAXPATHLEN];
  static Selector a1 = {"on_file", "%s", onfile, NULL},
  a2 = {"file", "%s", file, &a1};
  Object *o2;
  Signal *s;
  
  *onfile = 0;
  *file = 0;
  CHECK_QUERY(str, &a2)
    if(get_args(str, &a2) && *file) {
    (void)apply_waves_input_path(file,file);    
    if (*onfile) 
      (void)apply_waves_input_path(onfile,onfile);    
    setup_overlay(o->name,file,onfile);
    return(ok);
  }
  return(null);
}

/***********************************************************************/
char *
  meth_kill(o, str)
Object *o;
char *str;
{
  static char object[100], file[MAXPATHLEN];
  static Selector a1 = {"name", "%s", object, NULL},
  a2 = {"file", "%s", file, &a1};
  Object *o2;
  Signal *s;
  int killed = FALSE;

  *object = 0;
  
  CHECK_QUERY(str, &a2)
    if(!get_args(str, &a2)) {
    while(o = program.next)
      kill_object(o);
    return(ok);
  } else {
    View *find_and_destroy_overlay_views();

    if(! *object) return(null);
    if((o = find_object(object))) {
      if(*file) {
 	(void)apply_waves_input_path(file,file);
 	while((s = (Signal*)find_signal(o, file))) {
	  killed = TRUE;
	  if(s->views) {
	    Frame fr = (Frame)xv_get((Canvas)s->views->canvas, XV_OWNER);
	    xv_set(fr, FRAME_NO_CONFIRM, TRUE, 0);
	    dt_xv_destroy_safe(9,fr);
	    break;
	  } else {
	    View *vh0 = NULL, *vh;
	    vh = find_and_destroy_overlay_views(s);
	    if((s->file == SIG_NEW) &&
	       ((s->type & SPECIAL_SIGNALS) != SIG_SPECTROGRAM))
	      put_waves_signal(s);
	    unlink_signal(s);
	    free_signal(s);
	    if(!vh0)
	      vh0 = vh;
	    if(vh0)
	      redoit(vh0->canvas);
	  }
	}
      } else
	killed = kill_object(o);
      return((killed)? ok : null);
    } else
      return(null);
  }
}

/***********************************************************************/
char *meth_quit(o, str)
Object *o;
char *str;
{
  CHECK_QUERY(str,NULL)
  quit_proc();
  exit(0);
}

/***********************************************************************/
char *meth_print_setup_cover(o, str)
     Object *o;
     char *str;
{
  CHECK_QUERY(str,NULL)
  return(meth_print_setup(o, str));
}

/*********************************************************************/
mopup()
{
  extern Object program;  Object *o;
  extern Display *X_Display;
  extern char *registry_name;
  
  while(o = program.next)
    kill_object(o);
  
  Close_X_Comm(X_Display, registry_name);
  exit_lm();
}

/*********************************************************************/
exit_lm()
{
#ifndef NO_LIC
  extern void lm_quit();
  
  stop_da(NULL);
  lm_quit();
#endif
  
  exit(0);
}


/*********************************************************************/
kill_proc()
{
  char mess[100];
  extern int child;
  extern char *host, funcname[];

  do_ipc_response_if_any(in_a_ipc_dispatch(), ok);

  if(child) {
    sprintf(mess,"%s disconnect\n",host);
    xwaves_ipc_send("all", mess);
  } else
    terminate("all");
  meth_disable_server(NULL,NULL);
  mopup();
}

/*********************************************************************/
cleanup()
{
  Object *o;
  
  while(o = program.next)    kill_object(o);
  
  return(TRUE);
}

/**********************************************************************/
char *
  meth_make_panel(ob, args)
Object	*ob;
char	*args;
{
  extern int	def_panel_loc_x, def_panel_loc_y;
  static int	loc_x, loc_y, cols, quit, panel_choice, choice_horiz;
  static char	input_file[200], panel_name[50],
  panel_title[100], icon_title[50];
  static Selector
    a7 = {"quit_button", "%d", (char *) &quit,    NULL},
    a6 = {"columns",     "%d", (char *) &cols,    &a7},
    a5 = {"loc_y",      "%d", (char *) &loc_y,   &a6},
    a4 = {"loc_x",      "%d", (char *) &loc_x,   &a5},
    a3 = {"icon_title",  "#qstr", icon_title,    &a4},
    a2 = {"title",      "#qstr", panel_title,    &a3},
    a1b = {"choice_horiz",      "%d", (char*)&choice_horiz,    &a2},
    a1a = {"panel_choice",      "%d", (char*)&panel_choice,    &a1b},
    a1 = {"name",      "%s", panel_name,    &a1a},
    a0 = {"file",      "%s", input_file,    &a1};
  
  input_file[0] = '\0';
  panel_name[0] = '\0';
  panel_title[0] = '\0';
  icon_title[0] = '\0';
  loc_x = def_panel_loc_x;
  loc_y = def_panel_loc_y;
  panel_choice = 0;
  choice_horiz = 0;
  cols = 10;
  quit = 0;
  CHECK_QUERY(args, &a0)
    if (get_args(args, &a0)) {
      if (FIND_WAVES_MENU(input_file, input_file))
	if(make_auxpanel(input_file, panel_name, quit, cols,
			 panel_title, icon_title, loc_x, loc_y, panel_choice,
			 choice_horiz)) 
	  return ok;
    }
  return null;
}

/**********************************************************************/
do_something(cmd)
     char *cmd;
{
  if(cmd && *cmd) {
    strip_trailing_spaces(cmd);
    
    if(!strcmp(dispatch(receiver_prefixed(cmd)), ok))
      return(TRUE);
  }
  return(FALSE);
}

/**********************************************************************/
char *meth_make_add_op_panel(ob, args)
Object	*ob;
char	*args;
{
  extern int	def_panel_loc_x, def_panel_loc_y;
  static int	loc_x, loc_y, ops_win, vars_win;
  static char   panel_name[50],
  help_file[NAMELEN];
  static Selector
    a7 = {"show_ops", "%d", (char *) &ops_win,    NULL},
    a6 = {"show_vars",     "%d", (char *) &vars_win,    &a7},
    a5 = {"loc_y",      "%d", (char *) &loc_y,   &a6},
    a4 = {"loc_x",      "%d", (char *) &loc_x,   &a5},
    a3 = {"help_file",  "#qstr", help_file,    &a4};
  
  help_file[0] = '\0';
  loc_x = def_panel_loc_x;
  loc_y = def_panel_loc_y;
  ops_win = vars_win = 0;

  CHECK_QUERY(args, &a3)
    get_args(args, &a3);
  if(make_add_op_panel(help_file, loc_x, loc_y, ops_win, vars_win, do_something))
	return ok;
  return null;
}

/**********************************************************************/
char *
  meth_close_panel(ob, args)
Object	*ob;
char	*args;
{
  static char	panel_name[50];
  static int inv;
  static Selector
    a0 = {"name",   "%s",   panel_name,     NULL},
    a1 = {"invisible", "%d", (char*)&inv, &a0};

  inv = 0;
  panel_name[0] = '\0';
  CHECK_QUERY(args, &a1)
  if (get_args(args, &a1) && close_auxpanel(panel_name, inv))
    return ok;
  else return null;
}

/**********************************************************************/
char *
  meth_open_panel(ob, args)
Object	*ob;
char	*args;
{
  static char	panel_name[50];
  static Selector
    a0 = {"name",   "%s",   panel_name,	NULL};
  
  panel_name[0] = '\0';
  CHECK_QUERY(args, &a0)
  if (get_args(args, &a0) && open_auxpanel(panel_name))
    return ok;
  else return null;
}

/**********************************************************************/
char *
  meth_kill_panel(ob, args)
Object	*ob;
char	*args;
{
  static char	panel_name[50];
  static Selector
    a0 = {"name",   "%s",   panel_name,	NULL};
  
  panel_name[0] = '\0';
  CHECK_QUERY(args, &a0)
  if (get_args(args, &a0) && kill_auxpanel(panel_name))
    return ok;
  else return null;
}

/***********************************************************************/
char *meth_add_waves(ob, args)
     Object	    *ob;
     char	    *args;
{
  extern void	    e_exec_waves();
  extern Menuop   *wave_get_ops();
  
  static char	    name[50];
  static char	    command[100];
  static char	    menu[50];
  static char	    builtin[50];
  static char	    submenu[50];
  static Selector
    a4 = {"submenu", "#qstr", submenu, NULL},
    a3 = {"builtin", "#strq", builtin, &a4},
    a2 = {"menu",	 "%s",	  menu,	   &a3},
    a1b = {"command", "#strq",  command, &a2},
    a1 = {"op", "#strq",  builtin, &a1b},
    a0 = {"name",	 "#qstr", name,	   &a1};
  
  *name = '\0';
  *command = '\0';
  strcpy(menu,"all");
  *builtin = '\0';
  *submenu = '\0';

  CHECK_QUERY(args, &a0)
  (void) get_args(args, &a0);
  
  if(debug_level) {
    fprintf(stderr,
	    "%s: name \"%s\" menu \"%s\" builtin \"%s\" submenu \"%s\"\n",
	    "meth_add_waves", name, menu, builtin, submenu);
    fprintf(stderr, "\tcommand \"%s\"\n", command);
  }
  
  if(!!*command + !!*builtin + !!*submenu > 1) {
    sprintf(notice_msg, 
	    "%s: can specify only one of command, builtin, and submenu.",
	    "meth_add_waves");
    show_notice(1, notice_msg);
    return null;
  }
  
  if(*command) {

    menu_change(name, name,
		new_menuop(menu, name, e_exec_waves, savestring(command)),
		menu);
    return ok;
  }
  
  if(*builtin) {
    Menuop	*m, *find_op_in_list();
    char    *namep = (*name) ? name : builtin;

    if((!strcmp(menu,"all") || !strcmp(menu,"wave")) &&
       ((m = find_op_in_list(builtin, "wave")) ||
	(m = find_op_in_list(builtin, "all"))) && m->proc)
      menu_change(namep, namep, m, "wave");

    if((!strcmp(menu,"all") || !strcmp(menu,"spect")) &&
       ((m = find_op_in_list(builtin, "spect")) ||
	(m = find_op_in_list(builtin, "all"))) && m->proc)
      menu_change(namep, namep, m, "spect");
    
      return ok;
  }
  
  if(*submenu) {
    if(add_button_submenu(name, menu))
      return ok;
  }
  
  return null;
}

/**********************************************************************/
char *meth_open_ctlwin(ob, str)
     Object *ob;
     char *str;
{
  CHECK_QUERY(str,NULL)

    if (daddy) {
    xv_set(daddy, FRAME_CLOSED, FALSE, 0);  
    xv_set(daddy, XV_SHOW, TRUE, 0); 
   return ok;
  }
  else return null;
}

/**********************************************************************/
char *meth_close_ctlwin(ob, str)
     Object *ob;
     char *str;
{
  static int invisible = 0;
  static Selector  a2 = {"invisible", "%d", (char*)&invisible, NULL};
  Frame fr;
  
  invisible = 0;
  CHECK_QUERY(str, &a2)
    get_args(str, &a2);

  if (daddy) {
    xv_set(daddy, FRAME_CLOSED, TRUE, 0); 
    if(invisible)
      xv_set(daddy, XV_SHOW, FALSE, 0); 
    return ok;
  }
  else return null;
}


/**********************************************************************/
char *meth_make_object(ob,args)
     Object *ob;
     char *args;
{
  extern double ref_size, ref_start;
  extern int def_w_width, def_w_height, next_x, next_y;
  extern char objectname[], inputname[], active_ids[], active_numbers[];
  Object *o;
  Signal *s;
  View *v;
  int oline;
  static Selector
    a11 = {"identifiers", "#strq", active_ids, &g1}, /* note linkage to globals!*/
    a10 = {"numbers", "#strq", active_numbers, &a11},
    a9 = {"duration", "%lf", (char *) &ref_size, &a10},
    a8 = {"start", "%lf", (char *) &ref_start, &a9},
    a7 = {"width", "%d", (char *) &new_width, &a8},
    a6 = {"height", "%d", (char *) &new_height, &a7},
    a5 = {"loc_y", "%d", (char *) &next_y, &a6},
    a4 = {"loc_x", "%d", (char *) &next_x, &a5},
    a1 = {"name", "%s", objectname, &a4},
    a0 = {"file", "%s", inputname, &a1};
  
  oline = line_type;
  *active_ids = *active_numbers = 0;
  new_width = new_height = 0;
  CHECK_QUERY(args,&a0)
    if (get_args(args,&a0)) {
      post_set_process();
      if (*inputname) {
	(void)apply_waves_input_path(inputname,inputname);
	panel_set_value(newFile_item,inputname);
	create_new_signal_view(inputname);
	new_width = new_height = 0;
	if ((o = find_object(objectname))
	    && (s = find_signal(o,inputname))
	    && (v = s->views))
	  set_active_channels(v);
      }
      *active_ids = *active_numbers = 0;
      line_type = oline;
      return(ok);
    }
  line_type = oline;
  new_width = new_height = 0;
  *active_ids = *active_numbers = 0;
  return(null);
}

/***********************************************************************/
insert_mark(o, mark)
     Object *o;
     register Marks *mark;
{
  register Marks *ma;
  
  if(! (ma = o->marks)) {
    o->marks = mark;
    return TRUE;
  }
  if(ma->time >= mark->time) {
    if((ma->time == mark->time) && (ma->color == mark->color))
      return(FALSE);
    o->marks = mark;
    mark->next = ma;
    return(TRUE);
  }
  while( ma->next && (ma->next->time < mark->time)) ma = ma->next;
  while( ma->next && ma->next->time == mark->time) {
    if(ma->next->color == mark->color) return(FALSE);
    ma = ma->next;
  }
  mark->next = ma->next;
  ma->next = mark;
  return(TRUE);
}

/***********************************************************************/
object_enter_mark(o,time,color)
     Object *o;
     double time;
     int color;
{
  Marks *mark, *new_mark();
  
  if(! (mark = new_mark(time, color))) {
    fprintf(stderr,"Can't allocate another mark structure!\n");
    return(FALSE);
  }
  
  if(! insert_mark(o, mark)) {
    free((char *)mark);
    return;
  }
  object_xor_mark(o, time, color);
}

/***********************************************************************/
Marks *
  new_mark(time, color)
double time;
int color;
{
  Marks *m;
  
  if( ! (m = (Marks *)malloc(sizeof(Marks)))) return(NULL);
  
  m->next = NULL;
  m->time = time;
  m->color = color;
  return(m);
}


/***********************************************************************/
kill_all_marks(o)
     Object *o;
{
  Marks *m, *m2;
  
  if(o && (m = o->marks)) {
    while(m2 = m) {
      object_xor_mark(o,m->time, m->color);
      m = m->next;
      free((char *)m2);
    }
    o->marks = NULL;
  }
}

/***********************************************************************/
object_kill_mark(o, time, color)
     Object *o;
     double time;
     int color;
{
  Marks *ma, *map = NULL;
  
  if((ma = o->marks)) {
    while(ma) {
      if((ma->time == time) && (ma->color == color)) {
	object_xor_mark(o, time, color);
	if(ma == o->marks)
	  o->marks = ma->next;
	else
	  map->next = ma->next;
	free((char *)ma);
	return(TRUE);
      }
      map = ma;
      ma = ma->next;
    }
  }
  return(FALSE);
}

/***********************************************************************/
char *meth_chdir(o,str)
     Object *o;
     char *str;
{
  static char path[NAMELEN];
  static Selector a0 = {"path", "%s", path, NULL};

  *path = 0;
  CHECK_QUERY(str,&a0)
  get_args(str,&a0);
  if(*path) {
    char scrat[NAMELEN];
    (void)build_filename(scrat,"", path);
    if( !chdir(scrat))
      return(ok);
  }
  return(null);
}

/***********************************************************************/
char *meth_setenv(o,str)
     Object *o;
     char *str;
{
  static char *val, var[MAXPATHLEN];
  static int specsiz = 0;
  static Selector a0 = {"variable", "%s", var, NULL},
                  a1 = {"value", "#strq", NULL, &a0};
  int nc;

  if(str && strlen(str)) {
    if((nc = strlen(str)) > (specsiz)) {
      if(val)
	free(val);
      if(!(val = malloc(nc+1))) {
	show_notice(1, "Allocation problems in meth_putenv().");
	specsiz = 0;
	return(null);
      }
      specsiz = nc;
      a1.dest = val;
    }
    *val = *var = 0;
    CHECK_QUERY(str,&a1)
      get_args(str,&a1);
    if(*var) {
#if defined(SONY_RISC) || defined(CONVEX)
      setenv(var,val,1);
#else
      char *tch = malloc(strlen(val)+strlen(var)+3); /* NOTE that this is a memory leak! */
      sprintf(tch,"%s=%s",var,val);
      putenv(tch);
#endif
      return(ok);
    }
  }
  return(null);
}

/***********************************************************************/
char *meth_get(o,str)
Object *o;
char *str;
{
  static char attr[20], file[MAXPATHLEN], foutput[MAXPATHLEN], func[MAXPATHLEN],
  aname[MES_BUF_SIZE];
  static int id = 0;
  static Selector
    a2 = {"output", "%s", foutput, NULL},
    a1b = {"aname", "#strq", aname, &a2},
    a1 = {"attributes", "%s", attr, &a1b},
    a0b = {"function", "%s", func, &a1},
    a0 = {"file", "%s", file, &a0b},
    am1 = {"return_id", "%d", (char*)&id, &a0};
  static char reply[200];
  static char *oblock = NULL;
  static int oblocksize = 0;
  char scrat[MAXPATHLEN];
  double start, seccm;
  Frame frm;
  Rect  rec;
  FILE *fp = stdout, *fopen();
  Signal *s;
  View *v;
  
  id = 0;
  *aname = *foutput = *file = *func = 0;
  strcpy(attr,"view");
  CHECK_QUERY(str, &am1)
    if(get_args(str,&am1)) {
    
    if(*foutput && (*foutput != '-')) {
      /* expand any environment variables */
      (void) build_filename(scrat, "", foutput); 
      if( !(fp = fopen(scrat,"w"))) {
	sprintf(notice_msg, 
		"Can't open file %s to output global parameters.",
		scrat);
	show_notice(1, notice_msg);
	return(null);
      }
    }
    
    if(!strcmp("global",attr)) { /* GLOBAL ATTRIBUTES (.wave_pro) */
      Selector *sp = &g1;
      char  *cpp = (char*)malloc(MES_BUF_SIZE);
      int nstr = MES_BUF_SIZE;
      
      if(*aname) {
	char attname[MAXPATHLEN], *c = aname, *cp, *value_as_string();
	
	while(*c) {
	  sscanf(c,"%s",attname);
	  if(strcmp(attname,"objects")) {
	    cp = value_as_string(&g1,attname);
	  } else {		/* special case for "objects" */
	    cp = view_get_value(NULL, "objects");
	  }
	  if(cp)
	    fprintf(fp,"%s %s\n",attname,(*cp)? cp : "\"\"");
	  c = get_next_item(c);
	}
      } else		/* just print them all */
	while(sp) {
	  if(arg_to_string(&cpp, &nstr, sp))
	    fprintf(fp,"%s",cpp);
	  sp = sp->next;
	}
      if(fp != stdout)
	fclose(fp);
      if(cpp)
	free(cpp);
      return(ok);
    }

    if (*file) (void) apply_waves_input_path(file, file);
    if((! *file) || (!(s = find_signal(o,file))) || !s->views ||
       !s->views->canvas || (!strcmp(attr,"display") &&
			     xv_get(xv_get(s->views->canvas,XV_OWNER),FRAME_CLOSED))) {
      /* must try to guess file */
      /* This is the simplest method of choosing which display to sync
	 the external function to.  It could stand considerable improvement. */
      s = o->signals;
      while(s) {		/* Just find the first signal with a view! */
	if((v = s->views) && v->canvas &&
	   !xv_get(xv_get(v->canvas,XV_OWNER),FRAME_CLOSED)) break;
	s = s->others;
      }
      if(!s) {                  /* Maybe all views were closed.  Try again. */
        s = o->signals;
        while(s) {              /* Just find the first signal with a view! */
          if((v = s->views) && v->canvas) break;
          s = s->others;
        }
      }
 
    }

    if(s)
      v = s->views;
    else
      v = NULL;

    if(!strcmp("view",attr)) { /* Attributes of a signal view */
      int used = 0;
      if(*aname) {
	char attname[MAXPATHLEN], *c = aname, *cp, *view_get_value();
	
	while(*c) {
	  sscanf(c,"%s",attname);
	  if((cp = view_get_value(v,attname))) {
	    if(*foutput) {
	      fprintf(fp,"%s",cp);
	      if(*cp && (cp[strlen(cp)-1] != '\n'))
		fprintf(fp,"\n");
	    }

	    if( ! used )
	      block_build(&oblock,&used,&oblocksize,"returned ");
	    else
	      block_build(&oblock,&used,&oblocksize," "); /* space between items */
	      block_build(&oblock,&used,&oblocksize,(*cp)? cp : "\"\"");
	  } else {
	    sprintf(notice_msg, "Can't get value of variable named \"%s\".",
		    attname);
	    show_notice(1, notice_msg);
	  }
	  c = get_next_item(c);
	}
      }
      if(fp != stdout)
	fclose(fp);
      if(used)
	return(oblock);
      else
	return(null);
    }

    if(!(s && (v = s->views))) {
      fprintf(stderr,"Signal %s does not have a view in object %s\n",file,o->name);
      return(null);
    }
    
    if(!strcmp("display",attr) && *func) { /* DISPLAY ATTRIBUTES FOR ATTACHMENT */
      *scrat = 0;
      augment_external_command(s,v,scrat);
      if(!id)
	sprintf(reply,"returned %s\n", scrat);
      else
	sprintf(reply,"%s completion %d %s\n",func,id,scrat);
      xwaves_ipc_send(func, reply);
      return(reply);
    }
    
    
  }
  return(null);
}

/***********************************************************************/
send_kill_file(o,s)
     Object *o;
     Signal *s;
{
  char mess[200];
  int itsok;
  
  if((!s) || (!o) || (!o->name))
    return(FALSE);
  
  sprintf(mess,"%s kill file %s\n", o->name, s->name);
  xwaves_ipc_send("all",mess);
  return(TRUE);
}


/***********************************************************************/
send_kill_object(o)
     Object *o;
{
  char mess[MES_BUF_SIZE];
  
  if((!o) || (!o->name))
    return(FALSE);
  sprintf(mess,"%s kill\n",o->name);
  xwaves_ipc_send("all",mess);
  return(TRUE);
}

/***********************************************************************/
send_redisplay(s)
     Signal *s;
{
  char mess[1000];
  Object *o;
  View *v;
  
  if(!s || !(o = (Object*)s->obj) || !o || !o->name)
    return(FALSE);
  
  sprintf(mess,"%s redisplay signal %s\n",o->name,s->name);
  xwaves_ipc_send("all",mess);
  return(TRUE);
}

#define ITIMER_NULL ((struct itimerval*)0)
/*********************************************************************/
/* Start an interval timer to time a "sleep" in a waves command file.
   Return to the notifier for the duration of the sleep to permit
   completion of all plotting operations and allow interaction with other
   windows during the "sleep." */
char *
  meth_sleep(ob, str)
Object *ob;
char *str;
{
  static double dur = 1.0;
  static Selector a1 = {"seconds", "%lf", (char*)&dur, NULL};
  struct itimerval tim;
  extern int command_paused;
  extern Panel control_panel;
  extern Notify_value waves_timeout_proc();
  
  dur = 1.0;
  CHECK_QUERY(str,&a1)
    get_args(str,&a1);
  if(dur < .2) dur = .2;
  /* Interrupts ("clock ticks") occur every .2 sec. */
  command_paused = (((int)(dur*5.0)) & 0x1fff) | SLEEPING;
  tim.it_interval.tv_usec = 200000;
  tim.it_interval.tv_sec = 0;
  tim.it_value.tv_usec = 200000;
  tim.it_value.tv_sec = 0;
  /* Note that the Control Panel FRAME, rather than window, is specified
     as the "client."  This prevents the window border blinker from
     clobbering my setting of the itimer (not sure why, but it works...). */
  notify_set_itimer_func(daddy,waves_timeout_proc,ITIMER_REAL,&tim,
			 ITIMER_NULL);

  ipc_callback_data = in_a_ipc_dispatch();
  return(ok);
}

/*********************************************************************/
check_ipc_response_pending(str)
char *str;
{
  do_ipc_response_if_any(ipc_callback_data, str);
  ipc_callback_data = NULL;
}

/*********************************************************************/
/* This proc decrements the sleep counter every 200ms as the interval
   timer tics. */
Notify_value 
  waves_timeout_proc(item, which)
Panel_item item;
int which;
{
  extern Panel control_panel;
  extern Panel_item newControl_item;
  
  if(which == ITIMER_REAL) {
    if((command_paused & SLEEPING)) {
      int i = (command_paused ^ SLEEPING) - 1;
      
      if(i <= 0) {
	notify_set_itimer_func(daddy,waves_timeout_proc,ITIMER_REAL,
			       ITIMER_NULL,ITIMER_NULL);
	check_ipc_response_pending(ok);
	command_proc(newControl_item, NULL);
      }
      else
	command_paused = i | SLEEPING;
    }
  }
  return(NOTIFY_DONE);
}
#undef ITIMER_NULL

/*********************************************************************/
char *
  meth_shell(ob, str)
Object *ob;
char *str;
{
  void meth_shell_done();
  CHECK_QUERY(str,NULL)
  if(str && *str) {
    run_esps_prog(str,"","",0,0,meth_shell_done);
    command_paused = TRUE;
  }
  return(ok);
}

/*********************************************************************/
void
  meth_shell_done(pid)
int pid;
{
  extern FILE *fp_command;
  extern Panel_item newControl_item;
  
  if (w_verbose || debug_level) fprintf(stderr,"Shell command completed.\n");
  if(*commandname && fp_command) 
    command_proc(newControl_item, NULL);
}

/*********************************************************************/
char *
  meth_mark(ob, str)
Object *ob;
char *str;
{
  static double time;
  static int color;
  static Selector a2 = {"time", "%lf", (char*)&time, NULL },
  a1 = {"color", "%d", (char*)&color, &a2};
  
  time = 0.0;
  color = MARKER_COLOR;
  
  CHECK_QUERY(str, &a1)
    if(get_args(str, &a1)) {
    object_enter_mark(ob, time, color);
    return(ok);
  }
  return(null);
}


/***********************************************************************/
char *meth_unmark(o,str)
     Object *o;
     char *str;
{
  static double time;
  static int color;
  static char really[6];
  static Selector a3 = {"all", "%s", really, NULL},
  a2 = {"time", "%lf", (char*)&time, &a3},
  a1 = {"color", "%d", (char*)&color, &a2};
  Marks *ma;
  
  time = 0.0;
  color = MARKER_COLOR;
  *really = 'f';
  
   CHECK_QUERY(str, &a1)
  
  if((ma = o->marks)) {
    if(get_args(str, &a1)) {
      if((*really == 't') || (*really == 'T')) {
	kill_all_marks(o);
	return(ok);
      }
      if(object_kill_mark(o, time, color))
	return(ok);
    }
  }
  return(null);
}


/*********************************************************************/
char *
  meth_redisplay(ob,str)
Object *ob;
char *str;
{
  return(ok);
}

/*********************************************************************/
Object *
  make_new_object(str)
char *str;
{
#ifndef SETUP_MESSAGE
  return(NULL);
}
#else
char name[200], command[100];

if (str && sscanf(str,"%s",name) == 1 && strlen(name)) {
  sprintf(command,"name %s",name);
  if(!strcmp(ok,meth_make_object(NULL,command))) {
    return((Object *)get_receiver(name));
  }
}
return(NULL);

}
#endif

/***********************************************************************/
/* This utility expands a filename as a command file.  If a file 
 * is found, the expanded name is substituted for the original name 
 * and we attempt to open the file.  Otherwise we just return null
 * without modifying the original name
 */

static FILE *
fopen_command_file(filename, type)
char *filename, *type;
{
  char *fullname = NULL;

  fullname = FIND_WAVES_COMMAND(NULL, filename); 
  
  if (fullname == NULL) {
    fprintf(stderr, "Couldn't find readable command file %s\n", filename); 
    return(NULL);
  }

  strcpy(filename, fullname); 

  return(fopen(filename, type)); 

}
 
/***********************************************************************/
char *meth_insert(o, str)
     Object *o;
     char *str;
{
  static char file[NAMELEN], source[NAMELEN];
  static double time;
  static Selector s1 = {"file", "%s", file, NULL},
                  s2 = {"source", "%s", source, &s1},
                  s3b = {"output", "%s", outputname, &s2},
                  s3 = {"time", "%lf", (char*)&time, &s3b};

  *file = *source = 0;
  time = -123.0;

  CHECK_QUERY(str, &s3)
  
  get_args(str, &s3);

  if(*file && *source && (time > -122.0)) {
    apply_waves_input_path(source,source);
    set_designated_source(source);
    (void)apply_waves_input_path(file,file);
    if(paste(find_signal(o,file),time))
      return(ok);
    *source = 0;
    set_designated_source(source);
  } else {
    sprintf(notice_msg, "Bad arguments to meth_insert(%s).", str);
    show_notice(1, notice_msg);
  }
  return(null);
}

/***********************************************************************/
char *meth_disconnect(o, str)
     Object *o;
     char *str;
{
  static char fname[NAMELEN];
  static Selector a1 = {"function", "%s", fname, NULL};

  CHECK_QUERY(str, &a1)
  if(get_args(str, &a1)) {
    destroy_function_client(fname);
    return(ok);
  } else
    show_notice(1, "Unspecified function name in meth_disconnect (ignored).");
  return(null);
}

/***********************************************************************/

/* operations on Objects and signals */
Methods
	 meth19 = {"print_ensemble", meth_print_ensemble, NULL},
	 meth18 = {"print_graphic", meth_print_graphic, &meth19},
         meth17 = {"play_wind", meth_play_window, &meth18},
         meth16 = {"page", meth_page, &meth17},
         meth15 = {"save", meth_save_seg, &meth16},
         meth14 = {"bracket", meth_bracket_markers, &meth15},
         meth13 = {"open", meth_open_frame, &meth14},
         meth12 = {"close", meth_close_frame, &meth13},
         meth11 = {"align", meth_align, &meth12},
         meth10 = {"activate", meth_active_channels, &meth11},
         meth9 = {"overlay", meth_overlay, &meth10},
         meth8 = {"get", meth_get, &meth9},
         meth7b = {"cursor", meth_move_cursors, &meth8},
         meth7 = {"marker", meth_move_markers, &meth7b},
	 meth6 = {"colormap", meth_remap_colors, &meth7},
	 meth5a = {"insert", meth_insert, &meth6},
	 meth5 = {"spectrogram", meth_spectrogram, &meth5a},
         meth4 = {"play", meth_play_seg, &meth5},
         meth3b = {"stop_play", meth_stop_play, &meth4},
	 meth3 = {"set", meth_set_attr, &meth3b},
         meth2b = {"op", meth_call_operator, &meth3},
	 meth2c = {"insert", meth_insert, &meth2b},
	 meth2 = {"mark", meth_mark, &meth2c},
	 public_view_methods = {"unmark", meth_unmark, &meth2},
	 meth1 = {"completion", meth_return, &public_view_methods };
/* operations on the main program */
Methods
	bmeth33 = {"get_attach_list", meth_get_attach_list, NULL},
	bmeth32 = {"print_setup",meth_print_setup_cover,&bmeth33},
	bmeth30 = {"auto_plot_limits", meth_auto_plotlims, &bmeth32},
	bmeth29 = {"open_ctlwin", meth_open_ctlwin, &bmeth30},
	bmeth28 = {"close_ctlwin", meth_close_ctlwin, &bmeth29},
	bmeth27 = {"disable_server", meth_disable_server, &bmeth28},
	bmeth26b = {"enable_server", meth_enable_server, &bmeth27},
	bmeth25 = {"add_waves", meth_add_waves, &bmeth26b},
	bmeth24b = {"add_op_panel", meth_make_add_op_panel, &bmeth25},
	bmeth24 = {"make_panel", meth_make_panel, &bmeth24b},
	bmeth23 = {"open_panel", meth_open_panel, &bmeth24},
	bmeth22 = {"close_panel", meth_close_panel, &bmeth23},
	bmeth21 = {"kill_panel", meth_kill_panel, &bmeth22},
	bmeth20b = {"delete_all_items", meth_delete_all, &bmeth21},
	bmeth20 = {"delete_item", meth_delete_item, &bmeth20b},
	bmeth19 = {"add_espsn", meth_add_espsn, &bmeth20},
        bmeth18 = {"add_espst", meth_add_espst, &bmeth19},
        bmeth17c = {"add_op", meth_add_operator, &bmeth18},
        bmeth17b = {"add_espsf", meth_add_espsf, &bmeth17c},
	bmeth17 = {"cd",  meth_chdir, &bmeth17b},
	bmeth13b = {"setenv",  meth_setenv, &bmeth17},
        bmeth13 = {"shell", meth_shell, &bmeth13b},
        bmeth12g = {"save_panels", meth_save_panels, &bmeth13},
        bmeth12f = {"save_menus", meth_save_menus, &bmeth12g},
        bmeth12e = {"save_add_ops", meth_dump_add_ops, &bmeth12f},
        bmeth12d = {"save_keymaps", meth_dump_keymaps, &bmeth12e},
        bmeth12c = {"key_map", meth_add_keymap, &bmeth12d},
        bmeth12b = {"key_unmap", meth_delete_keymap, &bmeth12c},
        bmeth12 = {"set", meth_set_attr, &bmeth12b},
        bmeth11 = {"detach", meth_detach, &bmeth12},
        bmeth10 = {"attach", meth_attach, &bmeth11},
        bmeth9 = {"send", meth_send, &bmeth10},
        bmeth8b ={"get", meth_get, &bmeth9},
        bmeth7 = {"kill", meth_kill, &bmeth8b},
	bmeth5 = {"make", meth_make_object, &bmeth7},
	bmeth4 = {"quit", meth_quit, &bmeth5},
        bmeth8 = {"pause", meth_pause, &bmeth4},
        bmeth16 = {"call", meth_call_script, &bmeth8},
        bmeth15 = {"return", meth_return_from_script, &bmeth16},
        bmeth14 = {"branch", meth_branch_to_script, &bmeth15},
	public_methods = {"sleep", meth_sleep, &bmeth14},

	bmeth2 = {"start", meth_start_attachment, &public_methods},
        bmeth6 = {"redisplay", meth_redisplay, &bmeth2},
        bmeth31 = {"completion", meth_return, &bmeth6 },
        bmeth1 = {"disconnect", meth_disconnect, &bmeth31};
/**************************************************************/
char **get_commands(mop)
     Methods *mop;
{
  Methods *mo = mop;
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

      if(mop->name && mop->name[0] && mop->meth)
	wopl[no++] = mop->name;

      mop = mop->next;
    }
    wopl[no] = NULL;
  }
  return(sort_a_list(wopl));
}

/**************************************************************/
char **waves_get_commands()
{
  return(get_commands(&public_methods));
}

/**************************************************************/
char **object_get_commands()
{
  char **sort_a_list();

  return(get_commands(&public_view_methods));
}

/*************************************************************************/
char **waves_get_variable_names()
{
  Selector *se = &g1;
  int no = 0;
  char **wopl = NULL, **sort_a_list();
  
  while(se) {
    if(se->name && *se->name)
      no++;
    se = se->next;
  }

  if(no) {
    wopl = (char**) malloc(sizeof(char**) * (no+1));
    no = 0;

    se = &g1;
    while(se) {

      if(se->name && se->name[0])
	wopl[no++] = se->name;

      se = se->next;
    }

    wopl[no] = NULL;
  }
  return(sort_a_list(wopl));
}

Methods *get_methods_list(type)
     char *type;
{
  if(type && *type) {
    if(!strcmp(type,"view"))
      return(&public_view_methods);
    else
      return(&public_methods);
  }
  return(NULL);
}

  
    
