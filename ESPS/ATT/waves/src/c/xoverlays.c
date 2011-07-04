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
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)xoverlays.c	1.19	9/28/98	ATT/ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <Objects.h>
#include <fcntl.h>
#include <spectrogram.h>
#include <tracks.h>
#include <xview/scrollbar.h>
#include <xview/font.h>

#define ALMOST 0.95
#define P_F_Y_LOC (17 + v->readout_height)
#define P_F_X_LOC 0

extern char  overlayname[];	/* in globals.c */
extern char objectname[];
static void	dotospect(), set_spect_scale();
void cmap_spect();
static o_yoff = 5, o_xoff = -4;	/* x and y offsets required to center "*" */
static o_off = -2, o_siz = 5;	/* x & y offset to center box; side of box */
extern int options;
extern int scrollbar_height, readout_bar_height;
extern Xv_Font def_font;
extern int     def_font_height, def_font_width;
extern void set_pvd();
extern double assign_value(),  signal_get_value(), get_displayed_value();
extern int debug_level;
char *expand_name();

#define BW_OVERLAYS 1
View *setup_general_overlay();
View *setup_formant_overlay(), *setup_pole_overlay(), *create_dummy_sf();

/*************************************************************************/
/*
  How can spectrogram overlays be supported in a reasonably principled way?
  Several signals need to be overlaid on spectrograms. It would be highly
  desirable if all elements of all overlaid signals could be plotted
  in unique colors AND using xor so that rapid changes could be made
  without redrawing the whole image.  Some of the plotted signals
  will be immutable, while others will be modified interactively.
  If modifications are performed, the modified signal must be renamed
  and written out to disc at some point.  

  Overlays could have roughly the same status as reticles except that they
  are based on signals and can be modified.  The signals used must be linked
  to the "host" signal so that when the host must be redisplayed, all
  overlays are also replotted.  This linkage can be through the views.
  The host's view list will contain views which refer to signals other
  than those at the head of the list.  These data for the overlays may have
  null view lists, or may also be displayed in other views.

  The procedure:
    Generate the host view as usual.
    Get the name(s) of the file(s) to be overlaid.
    Get these signals linked into the Object list.
    Create the views describing how to plot the overlay.
    Link these views into the host's view list.
    Call redoit().


  NON-SPECTROGAM OVERLAYS    

*/

/*********************************************************************/
double 
o_x_to_time(s,v,x)
     register View *v;
     register Signal *s;
     register int x;
{
  register int i;
  register double time, t;

  x -= *(v->x_offset);
  if(x < 0) x = 0;
  time = v->start_time + (double)x * *(v->x_scale) / PIX_PER_CM;
  if (time > (t = ET(v)))  time = t;
  i = 0.5 + ((time - s->start_time) * s->freq);
  return(s->start_time + ((double)i)/s->freq);
}

/*********************************************************************/
#define DIST(d) \
    ((!(d)) ? -1 : \
     iabs(y - (vh->y_offset[k] - (int)((d)[ind]*PIX_PER_CM/vh->y_scale[k]))))
overlay_xy_to_chan(v,x,y)
     register View *v;
     int x, y;
{
  register Signal *s;

  if(v && (s = v->sig)) {

    if(v->dims == 1) return(v->elements[0]); /* special-case for speed */
    else {
      int i, mini, mind, j, ind, k, isspect;
      View *vh = v;
      double time;

      if(v->extra && (v->extra_type == VIEW_OVERLAY))
	vh = (View*) v->extra;

      isspect = isa_spectrogram_view(vh);

      time = (vh->x_to_time)? vh->x_to_time(vh, x) :
	generic_x_to_time(vh, x);

      if (!s->data)
      {
	  fprintf(stderr,
		  "NULL signal data array in overlay_xy_to_chan.\n");
	  return 0;
      }

      ind = time_to_index(s, time);

      mind = 5000;     /* needs to be bigger than biggest y screen in pixels */
      mini = 0;
      for (i = 0; i < v->dims; i++) {

	if((k=i) >= vh->dims) k = vh->dims-1;

	if(isspect)   /* sgram views only use first scale and offset element */
	  k = 0;

	switch(s->type & VECTOR_SIGNALS) { 
	case P_SHORTS:
	  j = DIST(((short**)(s->data))[v->elements[i]]);
	  break;
	case P_DOUBLES:
	  j = DIST(((double**)(s->data))[v->elements[i]]);
	  break;
	case P_FLOATS:
	  j = DIST(((float**)(s->data))[v->elements[i]]);
	  break;
	case P_INTS:
	  j = DIST(((int**)(s->data))[v->elements[i]]);
	  break;
	case P_CHARS:
	  j = DIST(((char**)(s->data))[v->elements[i]]);
	  break;
	case P_MIXED:
	  {
	    caddr_t	s_data;
	    if(! s->types) {
	      fprintf(stderr,
		      "s->types null for P_MIXED in overlay_xy_to_chan\n");
	      return(0);
	    }
	    s_data = ((caddr_t *) s->data)[v->elements[i]];
	    switch (s->types[v->elements[i]])
	      {
	      case P_SHORTS:
		j = DIST((short *) s_data);
		break;
	      case P_DOUBLES:
		j = DIST((double *) s_data);
		break;
	      case P_FLOATS:
		j = DIST((float *) s_data);
		break;
	      case P_INTS:
		j = DIST((int *) s_data);
		break;
	      case P_CHARS:
		j = DIST((char *) s_data);
		break;
	      }
	  }
	  break;
	}
#undef DIST(d)
	if (j < 0)
	{
	    fprintf(stderr,
		    "NULL signal data in overlay_xy_to_chan.\n");
	    return 0;
	}
	else if (j < mind)
	{
	    mind = j;
	    mini = i;
	}
      }
      return(v->elements[mini]);
    }
  }

  if(debug_level)
    fprintf(stderr,"Bad args to overlay_xy_to_chan %x\n", v);
  return(0);
}
      
/****************************************************************************/
overlay_general(v)
    View    	*v;
{
  return(plot_waves(v));
}

/****************************************************************************/
overlay_spectrograms(v)
    View    	*v;
{
  View    	*vh;
  int	    	x, y, i, j, dim;
  double  	time, tinc, freq;
  caddr_t 	data;
  Pixwin  	*pw;
  Signal  	*s;
  struct rect	*rect;
  int		color;
  char	plotstr[2];

  if(v && (vh = (View*)v->extra) && (s = v->sig)) {
    if (((s->type & SPECIAL_SIGNALS) == SIG_GENERAL)
	&& s->data && (s->buff_size > 0)) {
      pw = canvas_pixwin(v->canvas);
      rect = (struct rect *) xv_get(v->canvas, WIN_RECT);
      strcpy(plotstr, "*");
      tinc = 1.0/s->freq;
      for (i = 0; i < s->buff_size; i++) {
	if (((time = (tinc*i) + BUF_START_TIME(s)) >= vh->start_time)
	    && (time <= ET(vh))) {
	  x = o_time_to_x(vh, time);
	  for(j = 0; j < v->dims ; j++) {
	    freq = get_displayed_value(s, v, j, i);
	    y = vh->yval_to_y(vh, freq);

	    if(vh->overlay_as_number) {
	      plotstr[0] = '1' + j;
	      pw_ttext(pw, x + o_xoff, y + o_yoff,
		       PIX_SRC|PIX_COLOR(FOREGROUND_COLOR),
		       def_font, plotstr);
	    } else {
	      color = YA1_COLOR + v->elements[j] + 1;
	      pw_write(pw, x + o_off, y + o_off, o_siz, o_siz,
		       PIX_SRC|PIX_COLOR(color), NULL, 0, 0);
	    }
	  }
	}
      }
      return TRUE;
    }
    else
      show_notice(1,"Apparently bogus SIG_GENERAL in overlay_spectrograms");
  }
  else
    show_notice(1,"Bad argument(s) to overlay_spectrograms");
  return FALSE;
}

/****************************************************************************/
dummy_cursor(v)
    View *v;
{
    return(TRUE);
}


/****************************************************************************/
general_overlay_print(v)
    View    *v;
{
    static int char_width = -1;
    int	    i, j, k, xloc;
    caddr_t data;
    View    *vh = (View *) v->extra;
    double  time, value, tm;
    char    tmp[200];
    Frame   frm;
    Pixwin  *pw;
    Signal  *s = v->sig;
    List    *li;

    time = vh->cursor_time;
    if (time < vh->start_time) time = vh->start_time;
    if (time > (tm = ET(vh))) time = tm;
    i = time_to_index(s, time);
    if(v->xy_to_chan)
      k =  v->cursor_channel;
    else
      k = overlay_xy_to_chan(v, vh->time_to_x(vh,time),
			     vh->yval_to_y(vh, vh->cursor_yval));

    value = signal_get_value(s, k, i);

    if(char_width <= 0)
      char_width = (int) xv_get(def_font, FONT_DEFAULT_CHAR_WIDTH);

    if(v->overlay_n <= 0) {	/* For the first overlay... */
      for (j = 0, li = s->idents;
	   j < k && li;
	   j++, li = li->next) { }
      
      sprintf(tmp, "%10s:%8.2f ", li ? li->str : "", value);
      xloc = P_F_X_LOC;
    } else {
      xloc = P_F_X_LOC + (char_width * ((v->overlay_n * 11) + 15));
      sprintf(tmp,"%8.2f ",value);
    }
    pw = (Pixwin *) canvas_paint_window(v->canvas);
    pw_text(pw, xloc, P_F_Y_LOC, PIX_SRC|PIX_COLOR(YA1_COLOR+v->overlay_n+1),
	    def_font, tmp);
}
      
/****************************************************************************/
overlay_formants(v)
     View *v;
{
  View *vh;
  int x, y, i, j, fe, chan;
  double time, tinc;
  Pixwin *pw;
  Signal *s;
  struct rect *rect;
  
  if (v && (vh = (View*)v->extra) && (s = v->sig)) {
    if (((s->type & VECTOR_SIGNALS) == P_DOUBLES
         || (s->type & SPECIAL_SIGNALS) == SIG_FORMANTS)
	&& (s->buff_size > 0))
    {
      pw = canvas_pixwin(v->canvas);
      rect = (struct rect*)xv_get(v->canvas, WIN_RECT);

      fe = v->dims; /* s->dim/2;		 number of formants */
      if(s->dim && !fe) fe = 1;	/* in case single formant and no bw. */
      if(vh->overlay_as_number) { /*  B&W for screen dump hardcopy */
	char fnum[2];

	fnum[1] = 0;
	for (tinc = 1.0/s->freq, i=0; i < s->buff_size; i++) {
	  if (((time = (tinc*i)+BUF_START_TIME(s)) >= vh->start_time) &&
	      (time <= ET(vh))) {
	    for (j=0, x=o_time_to_x(vh,time) + o_xoff; j < fe; j++) {
	      vh->cursor_channel = chan = v->elements[j];
	      y = vh->yval_to_y(vh, signal_get_value(s,j,i)) + o_yoff;
	      *fnum = '1' + chan;
	      pw_text(pw, x, y,
		      PIX_SRC|PIX_COLOR(FOREGROUND_COLOR), def_font, fnum);
	    }
	  }
        }
      } else {			/* regular color plots */
	for(tinc = 1.0/s->freq, i=0; i < s->buff_size; i++) {
	  if (((time = (tinc*i)+BUF_START_TIME(s)) >= vh->start_time)
	      && (time <= ET(vh)))
	  {
	      x = o_time_to_x(vh, time);
	      for (j = 0; j < fe; j++)
	      {
		
		vh->cursor_channel = chan = v->elements[j];
		y = vh->yval_to_y(vh, signal_get_value(s,j,i));
		pw_write(pw, x + o_off, y + o_off, o_siz, o_siz,
			   PIX_SRC|PIX_COLOR(YA1_COLOR + chan + 1), NULL, 0, 0);
	      }
	  }
	}
      }
      return(TRUE);
    } else
      show_notice(1,"Apparently bogus signal in overlay_formants");
  } else
    show_notice(1,"Bad argument(s) to overlay_formants");
  return(FALSE);
}

/****************************************************************************/
formant_cursor(v)
     View *v;
{
  return(TRUE);
}

/****************************************************************************/
formant_print(v)
     View *v;
{
  int i,j,k,fe;
  register View *vh = (View*)v->extra;
  double time, freq, amin, tm, val, tv;
  char tmp[100];
  /* Frame     frm; */
  Pixwin    *pw;
  
  time = vh->cursor_time;
  if (time < vh->start_time) time = vh->start_time;
  if (time > (tm = ET(vh))) time = tm;
  freq = vh->cursor_yval;
  if (freq < vh->start_yval) freq = vh->start_yval;
  if (freq > vh->end_yval)   freq = vh->end_yval;
  i = time_to_index(v->sig, time);
  fe = v->sig->dim/2;
  for(amin=1.0e8,j=0,k=0; j < fe; j++) {
    if((tm = fabs((tv = signal_get_value(v->sig,j,i)) - freq)) < amin) {
      val = tv;
      amin = tm;
      k = j;
    }
  }
  sprintf(tmp, "F:%5.0f   B:%5.0f", val, signal_get_value(v->sig,k+fe,i));
  /* pw = (Pixwin*)xv_get(v->canvas, WIN_PIXWIN); */
  pw = canvas_pixwin(v->canvas);
  pw_text(pw, 200, P_F_Y_LOC, PIX_SRC|PIX_COLOR(YA1_COLOR+k+1), def_font, tmp);
}  

/****************************************************************************/
overlay_poles(v)
  View *v;
{
  View *vh;
  int x, y, i, j;
  double time, tinc;
  POLE **po;
  Pixwin    *pw;
  Signal    *s;
  struct rect *rect;
  int	color;
  
  if(v && (vh = (View*)v->extra) && (s = v->sig)) {
    if(((s->type & SPECIAL_SIGNALS) == SIG_LPC_POLES) &&
       (po = (POLE**)s->data) && (s->buff_size > 0)) {
      pw = canvas_pixwin(v->canvas);
      rect = (struct rect*)xv_get(v->canvas, WIN_RECT);
      if(vh->overlay_as_number) color = FOREGROUND_COLOR;
      else color = YA1_COLOR;
      for(tinc = 1.0/s->freq, i=0; i < s->buff_size; i++) {
	if(((time = (tinc*i)+BUF_START_TIME(s)) >= vh->start_time) &&
	   (time <= ET(vh))) {
	  if (vh->overlay_as_number) {
	    for(j=0,x=o_time_to_x(vh,time) + o_xoff; j < po[i]->npoles; j++) {
	      y = vh->yval_to_y(vh,po[i]->freq[j]) + o_yoff;
	      pw_ttext(pw, x, y, PIX_SRC|PIX_COLOR(color), def_font, "*");
	    }
	  } else {
	    x = o_time_to_x(vh, time);
	    for(j = 0; j < po[i]->npoles; j++) {
	      y = vh->yval_to_y(vh, po[i]->freq[j]);
	      pw_write(pw, x + o_off, y + o_off, o_siz, o_siz,
		       PIX_SRC|PIX_COLOR(color), NULL, 0, 0);
	    }
	  }
	}
      }
      return(TRUE);
    } else
      show_notice(1,"Apparently bogus SIG_LPC_POLES in overlay_poles");
  } else
    show_notice(1,"Bad argument(s) to overlay_poles");
  return(FALSE);
}

/****************************************************************************/
pole_cursor(v)
     View *v;
{
  return(TRUE);
}

/****************************************************************************/
pole_print(v)
     View *v;
{
  int i,j,k;
  POLE **po;
  View *vh = (View*)v->extra;
  double time, freq, amin, tm;
  char tmp[100];
  /* Frame     frm; */
  Pixwin    *pw;
  
  time = vh->cursor_time;
  if (time < vh->start_time) time = vh->start_time;
  if (time > (tm = ET(vh))) time = tm;
  freq = vh->cursor_yval;
  if (freq < vh->start_yval) freq = vh->start_yval;
  if (freq > vh->end_yval)   freq = vh->end_yval;
  po = (POLE**)v->sig->data;
  i = time_to_index(v->sig, time);
  if(po && po[i] && po[i]->npoles) {
    for(amin=1.0e8,j=0,k=0; j < po[i]->npoles; j++) {
      if((tm = fabs(po[i]->freq[j] - freq)) < amin) {
	amin = tm;
	k = j;
      }
    }
    sprintf(tmp,"pF:%5.0f  pB:%5.0f",po[i]->freq[k],po[i]->band[k]);
    /* pw = (Pixwin*)xv_get(v->canvas, WIN_PIXWIN); */
    pw = canvas_pixwin(v->canvas);
    pw_text(pw, 0, P_F_Y_LOC, PIX_SRC|PIX_COLOR(YA1_COLOR), def_font, tmp);
  }
}  
  
/*********************************************************************/
Signal *
find_host_signal(ob,name)
     Object *ob;
     char *name;
{
  Signal *s;

  if(ob && name && (s = ob->signals)) {
    while(s) {
      if(!strcmp(expand_name(NULL,name),s->name) && s->views) return(s);
      s = s->others;
    }
  }
  return(NULL);
}

/****************************************************************************/
View *
setup_overlay(object,name,onname)
     char *name, *object, *onname;
{
  Object *o, *find_object();
  Signal *s, *so, *get_any_signal(), *find_signal();
  extern int read_poles();
  double mpexc_buf_start(), mpexc_buf_end();
  
  if(name && *name) {
    if(!(o = find_object(object))) object = objectname;
    if((o = find_object(object))) {
      if(!(s = find_host_signal(o,onname))) {
	s = o->signals;

	/* If no "onname" can be found, default to either the most
	   recent spectrogram or the most recently loaded signal. */
	for ( ; s->others; s = s->others ) {
	  if ((!s->views) ||
	      (s->views && (s->views->extra_type == VIEW_OVERLAY)))
	    continue; /* (no valid view to overlay onto) */
	  if ((s->type & SPECIAL_SIGNALS) == SIG_SPECTROGRAM ) break;
	}
      }
      if(s) {
	if((so=get_any_signal(expand_name(NULL,name),0.0,0.0,NULL))) {
	  switch(so->type & SPECIAL_SIGNALS)
	  {
	  case SIG_GENERAL:
	    if (!IS_GENERIC(so->type))
	    {
		sprintf(notice_msg,
			"Specified overlay signal type %x not supported",
		        so->type);
                show_notice(1,notice_msg);
		free_signal(so);
		return(NULL);
	    }
            if (so->utils->read_data(so, s->start_time,
                                     s->end_time - s->start_time))
            {                           /* use SIG_END_TIME()? */
                close_sig_file(so);
                return setup_general_overlay(so, s->views);
            }
	    else
	    {
		show_notice(1,"Problems reading general signal in setup_overlay");
		return(NULL);
	    }
	  case SIG_LPC_POLES:
	    so->utils->read_data = read_poles;
	    if(so->utils->read_data(so,s->start_time,
				    s->end_time - s->start_time)) { /* should use SIG_END_TIME() */
	      close_sig_file(so);
	      return(setup_pole_overlay(so,s->views));
	    } else {
	      show_notice(1,"Problems reading poles in setup_overlay");
	      return(NULL);
	    }
	  default:
	    if(((s->type & SPECIAL_SIGNALS) != SIG_SPECTROGRAM ) &&
	       IS_GENERIC(s->type))
	      return(setup_general_overlay(so,s->views));

	    if(((s->type & SPECIAL_SIGNALS) == SIG_SPECTROGRAM ) &&
	       ((so->type & VECTOR_SIGNALS) != P_DOUBLES)) {
	      sprintf(notice_msg,
		"Specified overlay signal type %x not supported",so->type);
              show_notice(1,notice_msg);
	      free_signal(so);
	      return(NULL);
	    }			/* else fall through and try to overlay */
	  case SIG_FORMANTS:
	    if(so->utils->read_data(so,s->start_time,
				    s->end_time - s->start_time)) { /* should use SIG_END_TIME() */
	      close_sig_file(so);
	      return(setup_formant_overlay(so,s->views));
	    } else {
	      show_notice(1,"Problems reading formants in setup_overlay");
	      return(NULL);
	    }
	  }
	} else {
	  sprintf(notice_msg,"Can't get signal %s in setup_overlay",name);
          show_notice(1,notice_msg);
	  return(NULL);
	}
      }
      sprintf(notice_msg,
		"Object %s has no views on which to overlay",objectname);
      show_notice(1,notice_msg);
    } else {
      sprintf(notice_msg,
		"Object %s doesn't exist, or has no views",objectname);
      show_notice(1,notice_msg);
    }
  }
  return(NULL);
}
  
/****************************************************************************/

View *
setup_general_overlay(s, v)
    Signal  *s;			/* signal to overlay */
    View    *v;			/* usually the view of the "host" signal */
{
    Object  *o;
    Signal  *sh;
    View    *vo, *vh;
    Rect    *rec;
    extern int YA1_COLOR;
    int overlays, i;

    if((vh = v) && (sh = v->sig) && (o = (Object*)sh->obj) && v->canvas) {
	s->others = o->signals;
	o->signals = s;
	s->obj = (caddr_t)o;
	if((vo = new_waves_view(s, v->canvas))) {
	    rec = (Rect*)xv_get(v->canvas,WIN_RECT);
	    vo->width = rec->r_width;
	    vo->height = rec->r_height;
	    vo->extra = (caddr_t)v;
	    vo->extra_type = VIEW_OVERLAY;
	    vo->cursor_plot = dummy_cursor;
	    vo->reticle_plot = NULL;
	    vo->x_print = general_overlay_print;
	    vo->y_print = NULL;
	    vo->time_to_x = generic_time_to_x;
	    vo->x_to_time = generic_x_to_time;
	    vo->yval_to_y = generic_yval_to_y;
	    vo->y_to_yval = generic_y_to_yval;
	    vo->xy_to_chan = overlay_xy_to_chan;
	    if(isa_spectrogram_view(vh)) {
	      vo->data_plot = overlay_spectrograms;
	    } else {
	      if(IS_TAGGED_FEA(s)) {
		int tag_time_to_x(), tag_plot_waves();
		double tag_x_to_time();
		
		vo->data_plot = tag_plot_waves;
		vo->time_to_x = tag_time_to_x;
		vo->x_to_time = tag_x_to_time;
	      } else {
		vo->data_plot = overlay_general;
	      }
	    }
	    overlays = 0;
	    while(v->next) {
	      v = v->next; /* put new view at end of list */
	      if(v->extra_type & VIEW_OVERLAY)
		v->overlay_n = overlays++;
	    }
	    vh->overlay_n = overlays;
	    vo->overlay_n = overlays;
	    for(i=0; i < s->dim; i++)
	      vo->colors[i] = YA1_COLOR + overlays + 1;

	    {
	      double start, size;

	      start = vh->start_time;
	      size = ET(vh) - vh->start_time;
	      get_view_segment(vo,&start,&size);
	    }

	    if(vh->plotted) {	/* do overlay now if host plot is visible */
		vo->data_plot(vo);
		vo->cursor_plot(vo);
		vo->x_print(vo);
	    }

	    vo->next = v->next;
	    v->next = vo;	/* (redisplay is done in list order) */
	    return(vo);
	} else
	    show_notice(1,"Can't make new_waves_view in setup_general_overlay");
    } else {
	show_notice(1,"Bad argument(s) to setup_general_overlay");
    }
    return(NULL);
}

/****************************************************************************/
/* The overlay is implemented by sharing the canvas of the "host" signal view.
Things like display offsets, scale factors, come from host by dereferencing
overlaying_view->extra in the context of VIEW_OVERLAY.  overlaying_view->sig
refers to the overlaying signal.  The overlaying signal does not reference
ANY views directly (s->views == NULL). */
View *
setup_formant_overlay(s,v)
     Signal *s;			/* the formant signal */
     View *v;			/* the "host" view */
{
  Object *o;
  Signal *sh;
  View *vo;
  Rect *rec;

  if((sh = v->sig) && (o = (Object*)sh->obj) && v->canvas) {
    int overlays;
    View *vh = v;

    s->others = o->signals;
    o->signals = s;
    s->obj = (caddr_t)o;
    if((vo = new_waves_view(s, v->canvas))) {
      rec = (Rect*)xv_get(v->canvas,WIN_RECT);
      vo->width = rec->r_width;
      vo->height = rec->r_height;
      vo->extra = (caddr_t)v;
      vo->extra_type = VIEW_OVERLAY;
      vo->data_plot = overlay_formants;
      vo->cursor_plot = formant_cursor;
      vo->reticle_plot = NULL;
      vo->x_print = formant_print;
      vo->y_print = NULL;
      vo->time_to_x = generic_time_to_x;
      vo->x_to_time = generic_x_to_time;
      vo->yval_to_y = generic_yval_to_y;
      vo->y_to_yval = generic_y_to_yval;
      vo->xy_to_chan = overlay_xy_to_chan;
      vo->dims = s->dim/2;
      if(v->plotted) { /* do overlay now if host plot is visible */
	vo->data_plot(vo);
	vo->cursor_plot(vo);
	vo->x_print(vo);
      }
      overlays = 0;
      while(v->next) {
	v = v->next; /* put new view at end of list */
	if(v->extra_type & VIEW_OVERLAY)
	  v->overlay_n = overlays++;
      }
      vh->overlay_n = overlays;
      vo->overlay_n = overlays;
      vo->next = v->next;
      v->next = vo;		/* (redisplay is done in list order) */
      return(vo);
    } else
      show_notice(1,"Can't make new_waves_view in setup_formant_overlay");
  } else
    show_notice(1,"Bad argument(s) to setup_formant_overlay");
  return(NULL);
}

/****************************************************************************/
View *
setup_pole_overlay(s,v)
     Signal *s;			/* the POLE signal */
     View *v;			/* the "host" view */
{
  Object *o;
  Signal *sh;
  Rect *rec;
  View *vo;

  if((sh = v->sig) && (o = (Object*)sh->obj) && v->canvas) {
    s->others = o->signals;
    o->signals = s;
    s->obj = (caddr_t)o;
    if((vo = new_waves_view(s, v->canvas))) {
      rec = (Rect*)xv_get(v->canvas,WIN_RECT);
      vo->width = rec->r_width;
      vo->height = rec->r_height;
      vo->extra = (caddr_t)v;
      vo->extra_type = VIEW_OVERLAY;
      vo->data_plot = overlay_poles;
      vo->cursor_plot = pole_cursor;
      vo->reticle_plot = NULL;
      vo->x_print = pole_print;
      vo->y_print = NULL;
      vo->time_to_x = generic_time_to_x;
      vo->x_to_time = generic_x_to_time;
      vo->yval_to_y = generic_yval_to_y;
      vo->y_to_yval = generic_y_to_yval;
      if(v->plotted) { /* do overlay now if host plot is visible */
	vo->data_plot(vo);
	vo->cursor_plot(vo);
	vo->x_print(vo);
      }
      while(v->next) v = v->next; /* put new view at end of list */
      vo->next = v->next;
      v->next = vo;		/* (redisplay is done in list order) */
      return(vo);
    } else
      show_notice(1,"Can't make new_waves_view in setup_pole_overlay");
  } else
    show_notice(1,"Bad argument(s) to setup_pole_overlay");
  return(NULL);
}

/*********************************************************************/
o_time_to_x(v,time)
     register View *v;
     register double time;
{
  return((int)(.5 + (PIX_PER_CM *
		     ((time - v->start_time) / *(v->x_scale)))) + *(v->x_offset));
}

/*******************************************************************/
static finish_overlay_edit(sf, vh, event, active)
  Signal *sf;
  View *vh;
  Event *event;
  int active;
{
  int id;
  if (event_is_up(event)
      && ((id = event_id(event)) != LOC_WINENTER)
      && (id != LOC_WINEXIT)
      && (id != LOC_MOVE)) {
    if((active >= 0) && vh->redraw_on_release)
      redoit(vh->canvas);
    else
      if(vh->cursor_plot) vh->cursor_plot(vh, CURSOR_COLOR);
    /* Optionally, save the signal after each edit. */
    if(sf && vh->rewrite_after_edit &&
       (sf->file_size == sf->buff_size)) {
      if(sf->file == SIG_NEW)
	put_waves_signal(sf);
      else
	put_signal(sf);
    }
    return(TRUE);
  } else
    return(FALSE);
}

/****************************************************************************/
/*
  Scan view list for this canvas looking for LPC_POLE and FORMANTS
  file overlays.  If both of these are found, check the state of the
  FORMANTS file descriptor.  If it is not SIG_NEW, change the name of the
  FORMANTS file (append "2" or something) and change s->file to SIG_NEW.
  Get the cursor time, index and frequency coordinates. If the button was just
  depressed (event_down and !LOC_DRAG) determine which formant the cursor is
  nearest and establish that as the "working" formant.  When LOC_DRAG events
  are detected, the POLE which is closest in time-frequency is assigned to the
  working formant and the FORMANTS signal is updated to reflect this.  The
  screen displays are adjusted accordingly. */

e_reassign_formant(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  static int active = -1, indo = -1;
  Signal  *s, *sp = NULL, *sf = NULL, *sg = NULL;
  Object  *o;
  POLE	  **ppp;
  View    *v, *vh, *vsp, *vsf, *vsg;
  int     type, id, x, y, color;
  register int indf, indp, i, j, k;
  register double time, freq, d_freq, amin, dtmp;
  char      *cp, *ct;
  Pixwin    *pw;
  Rect	    *rect;
  
  vh = v = (View*)xv_get(canvas, WIN_CLIENT_DATA);

/* Want to accomplish the following:
   If no formant tracks are overlaid (.pole tracks are optional):
       Else, create a .fb signal with all formant frequencies set to
             nominal values.  Overlay this signal on the spectrogram.
   Else, 
       If .fb and .pole signls are overlaid, modify the .fb file by
           moving the nearest formant to the pole nearest the cursor.
       if .pole signals only are overlaid, assign current formant according to
           cursor location re nominal formant overlay.
       If .fb file is overlaid, modify nearest formant
           to match cursor frequency location.
   If the name of a file to be modified ends in .ed, overwrite it, else
       change its name to end in .ed and write a new file.
*/

  
  while(v) {			/* try to find .pole and .fb overlay signals */
    if((type = v->sig->type & SPECIAL_SIGNALS) == SIG_LPC_POLES) {
      sp = v->sig;
      vsp = v;
    }
    if(type == SIG_FORMANTS) {
      sf = v->sig;
      vsf = v;
    }
    if (type == SIG_GENERAL) {
      sg = v->sig;
      vsg = v;
    }
    if(sp && sf) {		/* if both .fb and .pole exist... */

	/* make cursor time an integral number of data samples*/
      time = o_x_to_time(sf,vh, event_x(event));
      x = o_time_to_x(vh,time) + o_off;

      freq = vh->y_to_yval(vh, event_y(event));
      indf = vec_time_to_index(sf, time);
      indp = vec_time_to_index(sp, time);
      j = sf->dim/2;
/* INITIAL BUTTON PRESS */
      if(((id = event_id(event)) != LOC_DRAG) && (event_is_down(event))) {

	active = vsf->cursor_channel;
	indo = indf;		/* for interpolator */

	if(vh->cursor_plot) vh->cursor_plot(vh,CURSOR_COLOR);
	return(TRUE);
      }

/* DRAWING ON WINDOW */

      ppp = (POLE**)sp->data;
	  
      if((active >= 0)
	 && ppp && ppp[indp] && ppp[indp]->npoles && (id == LOC_DRAG))
      {
	int nin, inc;

	if(vh->overlay_as_number)
	  color = FOREGROUND_COLOR;
	else
	  color = YA1_COLOR;

	if((sf->file != SIG_NEW) && check_file_name(sf,".ed")) {
	  close_sig_file(sf);
	  sf->file = SIG_NEW;
	}
	
	/* Find the closest "pole" (candidate) frequency. */
	for(amin=fabs(freq - ppp[indp]->freq[0]),j=1,k=0;
	    j < ppp[indp]->npoles; j++)
	  if((dtmp = fabs(ppp[indp]->freq[j] - freq)) < amin) {
	    amin = dtmp;
	    k = j;
	  }

	assign_value(sf, vsf, active, indf, (double)(ppp[indp]->freq[k]));
	assign_value(sf, vsf, (sf->dim/2) + active, indf,
		     (double)(ppp[indp]->band[k]));
	if(indo >= 0) {
	  fill_in(sf, vsf, active,  (double)(ppp[indp]->freq[k]), indo, indf);
	  fill_in(sf, vsf, (sf->dim/2) + active,
		  (double)(ppp[indp]->band[k]), indo, indf);
	  nin = indf - indo;
	} else
	  nin = 0;
	indo = indf;
	if(nin != 0) {		/* replot all points traversed */
	  inc = nin/iabs(nin);
	  indp -= nin;
	  indf -= nin;
	  nin = iabs(nin);
	  pw = canvas_pixwin(v->canvas);
	  rect = (struct rect*)xv_get(v->canvas, WIN_RECT);
	  do {
	    indp += inc;
	    indf += inc;
	    time = sf->utils->index_to_time(sf, indf);
	    x = o_time_to_x(vh,time) + o_off;
	    for(j=0; j < ppp[indp]->npoles; j++) {
	      y = vh->yval_to_y(vh,ppp[indp]->freq[j]) + o_off;
	      pw_write(pw, x, y, o_siz, o_siz,
		       PIX_SRC|PIX_COLOR(color), NULL, 0, 0);
	    }
	    k = vsf->dims;
	    if(vh->overlay_as_number) {
	      char fnum[2];

	      x += o_xoff - o_off;
	      fnum[1] = 0;
	      for(j=0; j < k; j++) {
		*fnum = '1' + vsf->elements[j];
		y = vh->yval_to_y(vh,get_displayed_value(sf,vsf,j,indf)) + o_yoff;
		pw_ttext(pw, x, y,
			 PIX_SRC|PIX_COLOR(FOREGROUND_COLOR), def_font, fnum);
	      }
	    } else {
	      for(j=0; j < k; j++) {
		y = vh->yval_to_y(vh,get_displayed_value(sf,vsf,j,indf)) + o_off;
		pw_write(pw, x, y, o_siz, o_siz,
			 PIX_SRC|PIX_COLOR(YA1_COLOR+vsf->elements[j]+1), NULL, 0, 0);
	      }
	    }
	  } while(--nin > 0);
	}
	return(TRUE);
      }

/* BUTTON RELEASED IN WINDOW?? */
      if(finish_overlay_edit(sf,vh,event,active)) {
	active = -1;
	indo = -1;
      }
      return(TRUE);
    } /* end if(sp && sf) */
    v = v->next;
  }	    /* finished searching view list for pole and formant signals */

  if(sf) {			/* got here because there is no sp... */

    time = o_x_to_time(sf,vh, event_x(event));
    x = o_time_to_x(vh,time);
    freq = vh->y_to_yval(vh, event_y(event));
    indf = vec_time_to_index(sf, time);
    
/* INITIAL BUTTON PRESS */
    if(((id = event_id(event)) != LOC_DRAG) && (event_is_down(event))) {

      active = vsf->cursor_channel;

      if(vh->cursor_plot) vh->cursor_plot(vh, CURSOR_COLOR);
      return(TRUE);
    }
    
/* DRAWING ON WINDOW */
    if((active >= 0)  && (id == LOC_DRAG)) {
      double val;

      if(sf->file != SIG_NEW) {
	if (check_file_name(sf,".ed")) {
	  close_sig_file(sf);
	  sf->file = SIG_NEW;
	} else
	  if(sf->header && sf->header->magic == ESPS_MAGIC)
	    add_comment(sf->header->esps_hdr, "xwaves modify formants\n");
      }

      val = assign_value(sf, vsf, active, indf, freq);

      if(indo >= 0)
	fill_in(sf, vsf, active, freq, indo, indf);

      indo = indf;

      overlay_replot_value(vh, x, val, freq, active);

      return(TRUE);
    }

/* BUTTON RELEASED IN WINDOW?? */
    if(finish_overlay_edit(sf,vh,event,active)) {
      active = -1;
      indo = -1;
    }
    return(TRUE);
  } else

    if(sg) {  /* got here because there is no formant signal (sf) */
      time = o_x_to_time(sg, vh, event_x(event));
      x = o_time_to_x(vh, time);
      freq = vh->y_to_yval(vh, event_y(event));
      indf = sg->utils->time_to_index(sg, time);

      /* INITIAL BUTTON PRESS */
      if ((id = event_id(event)) != LOC_DRAG && event_is_down(event)) {

	active = vsg->cursor_channel;

	if(vh->cursor_plot) vh->cursor_plot(vh, CURSOR_COLOR);
	return TRUE;
      }

      /* DRAWING ON WINDOW */
      if ((active >= 0) && id == LOC_DRAG)  {
	if (sg->file != SIG_NEW) {
	  if (check_file_name(sg,".ed")) {
	    close_sig_file(sg);
	    sg->file = SIG_NEW;
	  }
	  else
	    if (sg->header && sg->header->magic == ESPS_MAGIC)
	      add_comment(sg->header->esps_hdr, "xwaves modify formants\n");
	}


	d_freq = assign_value(sg, vsg, active, indf, freq);

	if(indo >= 0)
	  fill_in(sg, vsg, active, freq, indo, indf);
	
	indo = indf;
	
	overlay_replot_value(vh, x, d_freq, freq, active);

	return TRUE;
      }

/* BUTTON RELEASED IN WINDOW?? */
      if(finish_overlay_edit(sg,vh,event,active)) {
	active = -1;
	indo = -1;
      }
      return(TRUE);

    } else {	/* got here because no sf and no sg */

      if(!sp) sp = vh->sig;	/* use the spectrogram signal if no poles */
      if((v = (View*)create_dummy_sf(sp,vh))) {

	sf = v->sig;
	time = o_x_to_time(sf,vh, event_x(event));
	x = o_time_to_x(vh,time) + o_xoff;

	/* INITIAL BUTTON PRESS */
	if(((id = event_id(event)) != LOC_DRAG) && (event_is_down(event))) {

	  active = (v->xy_to_chan)? v->xy_to_chan(v, x, event_y(event)) :
	    overlay_xy_to_chan(v, x, event_y(event));
	  
	  if(debug_level)
	    fprintf(stderr,"active:%d sf->dim:%d sf->type:%x\n",
		  active,sf->dim,sf->type);

	  if(vh->cursor_plot) vh->cursor_plot(vh, CURSOR_COLOR);

	  return(TRUE);
	}
      }
    }

  return(FALSE);
}

/*******************************************************************/
overlay_replot_value(vh, x, val, freq, active)
  View *vh;
  double val, freq;
  int x, active;
{
  int y;
  Pixwin *pw = canvas_pixwin(vh->canvas);

  if(vh->overlay_as_number) { /* numbers distinguish formants */
    char fnum[2];
    
    x += o_xoff;
    fnum[1] = 0;
    *fnum = '1' + active;
    y = vh->yval_to_y(vh,val) + o_yoff;
    pw_ttext(pw, x, y, PIX_SRC|PIX_COLOR(BACKGROUND_COLOR), def_font, fnum);
    y = vh->yval_to_y(vh,freq) + o_yoff;
    pw_ttext(pw, x, y, PIX_SRC|PIX_COLOR(FOREGROUND_COLOR), def_font, fnum);
  } else {			/* use colors to distinguish formants */
    x += o_off;
    y = vh->yval_to_y(vh,val) + o_off;
    pw_write(pw, x, y, o_siz, o_siz,
	     PIX_SRC|PIX_COLOR(FOREGROUND_COLOR), NULL, 0, 0);
    y = vh->yval_to_y(vh,freq) + o_off;
    pw_write(pw, x, y, o_siz, o_siz,
	     PIX_SRC|PIX_COLOR(YA1_COLOR+active+1), NULL, 0, 0);
  }
}

/*******************************************************************/
View *
create_dummy_sf(sp,vh)
     Signal  *sp; /* a pole signal currently overlaid on a spectrogram */
     View    *vh;		/* the spectrogram view */
{
  Signal  *sf;
  View    *v;
  int	  dim, i, j, k, size;
  double  **dpp, *dp, freq, band;
  caddr_t *cpp;
  char	  temp[200], *cp;

  if(sp && vh) {
    dim = 2 * (1 + ((sp->band - 500)/1000));
    if(dim > 8) dim = 8;
    if((sp->type & SPECIAL_SIGNALS) != SIG_LPC_POLES) { 
     freq = 100.0;		/* best guess at frame rate for formants */
      size = BUF_DURATION(sp) * freq;
    } else {
      freq = sp->freq;
      size = sp->file_size;
    }
    if((sf = new_signal(new_ext(sp->name,"fb.ed"),SIG_NEW,
        dup_header(sp->header), NULL,size,freq,dim))) {
      sf->file_size = sf->buff_size = size;
      if(freq != sp->freq)
	  head_printf(sf->header,"frequency",&(sf->freq));
      if(size != sp->file_size)
	  head_printf(sf->header,"samples",&(sf->file_size));

      if(!(dpp = (double**)malloc(sizeof(double*) * dim)))
      {
	printf("Problems allocating pointer array in create_dummy_sf()\n");
	return NULL;
      }
      for(i=0; i<dim; i++) {
	if(!(dpp[i] = (double*)malloc(sizeof(double) * sf->file_size))) {
	  printf("Problems allocating arrays for dummy formants\n");
	  return(NULL);
	}
      }
      if (sp->header && sp->header->magic == ESPS_MAGIC
	  && sp->header->esps_hdr)
      {
	  struct header	*hdr;
	  long	    	fld_dim[1];

	  hdr = new_header(FT_FEA);
	  sf->header->esps_hdr = hdr;
	  set_pvd(hdr);
	  sprintf(temp, "xwaves create_dummy_sf %s %s\n", sp->name, sf->name);
	  add_comment(hdr, temp);
	  *add_genhd_d("start_time", (double *) NULL, 1, hdr) = sp->start_time;
	  *add_genhd_d("record_freq", (double *) NULL, 1, hdr) = freq;
	  fld_dim[0] = dim/2;
	  add_fea_fld("fm", (long)dim/2, 1, fld_dim, DOUBLE, (char **) NULL, hdr);
	  add_fea_fld("bw", (long)dim/2, 1, fld_dim, DOUBLE, (char **) NULL, hdr);
	  
	  if (!(cpp = (caddr_t *) malloc(sizeof(caddr_t) * dim)))
	  {
	      printf("Can't allocate pointer array in create_dummy_sf()\n");
	      return NULL;
	  }
	  for (i = 0; i < dim; i++)
	      cpp[i] = (caddr_t) dpp[i];

	  if (!(sf->types = (int *) malloc(sizeof(int) * dim)))
	  {
	      printf("Can't allocate types array in create_dummy_sf()\n");
	      return NULL;
	  }
	  for (i = 0; i < dim; i++)
	      sf->types[i] = P_DOUBLES;

	  sf->data = (caddr_t) cpp;
	  sf->type = P_MIXED | SIG_FORMANTS | SIG_BINARY;
      }
      else
      {
	  sf->data = (caddr_t)dpp;
	  sf->type =  P_DOUBLES | SIG_FORMANTS | SIG_ASCII;
      }


      dim /= 2;
      for(i=0; i<dim; i++) {	/* create the dummy data */
	k = i + dim;
	for(j=0, freq = 500 + i*1000, band = 60 + 0.05*freq;
	    j < sf->file_size; j++) {
	      dpp[i][j] = freq;
	      dpp[k][j] = band;
	    }
      }
      head_printf(sf->header,"type_code",&(sf->type));
      head_printf(sf->header,"dimensions",&(sf->dim));
      cp = temp;
      for(i=0; i< dim; i++) {
	sprintf(cp,"F%d ",i+1);
	cp = temp + strlen(temp);
      }
      for(i=0; i< dim; i++) {
	sprintf(cp,"B%d ",i+1);
	cp = temp + strlen(temp);
      }
      head_ident(sf->header,temp);
      if((sp->type & SPECIAL_SIGNALS) == SIG_LPC_POLES) 
	head_printf(sf->header,"remark",
	    "xwaves: created by hand-assignment of frequency candidates");
      else
	head_printf(sf->header,"remark",
	    "xwaves: created by hand-marking formant frequencies");
      return(setup_formant_overlay(sf,vh,1));
    } else
      printf("Can't create_dummy_sf()\n");
  } else
    printf("Bad arguments to create_dummy_sf()\n");
  return(NULL);
}    
