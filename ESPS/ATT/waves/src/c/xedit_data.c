/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:   David Talkin
 * Checked by:
 * Revised by:   Alan Parker, David Talkin
 *
 * Brief description:
 *
 */
/* xedit_data.c */
/* The beginnings of a waveform data editor for waves.  Two editing domains
   are supported: (1) modification of existing signals by changing vector
   element values by "drawing" with the mouse; (2) cutting and splicing
   waveform fragments.  The first domain is well developed for all vector
   signals; the second is under development...
   */

static char *sccs_id = "@(#)xedit_data.c	1.11	9/28/98	ATT/ESI/ERL";

#include <Objects.h>
#include <spectrogram.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/limits.h>

Header	*dup_header();
char	*get_date();
void	free_signal();
double assign_value(), get_displayed_value(), signal_get_value();

extern void set_pvd();

/*********************************************************************/
#define y_to_val(v,y,chan)  (((v)->y_offset[chan] - (y)) * (v)->y_scale[chan]/PIX_PER_CM)
#undef MIN
#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )  
#undef MAX
#define MAX(a,b) ( ((a)>(b)) ? (a) : (b) )  


/*********************************************************************/
/* In situ modification of data element values for all types of vector
   signals.  If a signal is modified, ".ed" is appended to its filename
   and it is marked to be saved.  The channel (element) to be modified is
   determined when the button is first pressed in the window.  Mouse movements
   with button depressed reassign data values.  Button release optionally
   causes the display to be redrawn to reflect the changes.
*/
void 
e_modify_signal(canvas, event, arg)
     Canvas canvas;
     Event *event;
     caddr_t arg;
{
  View *v;
  Signal *s;
  static double time;
  static int x, y, active = -1, indo = -1, dactive = -1;
  int ind, id, color, type;
  double val;
  register int nf, nfs;
  Xv_Cursor cursor;
  Pixwin *pw;
  Rect *rect;

  v = (View *)xv_get(canvas, WIN_CLIENT_DATA);
  s = v->sig;

/* Is it a regular VECTOR_SIGNAL? */
  if(IS_GENERIC(s->type)) {
    x = event_x(event);
    y = event_y(event);
    time = v->x_to_time(v,x);
    ind = time_to_index(s,time);

    /* When BUTTON IS FIRST PRESSED: */
    if(((id = event_id(event)) != LOC_DRAG) && (event_is_down(event))) {
      active = (v->xy_to_chan)? v->xy_to_chan(v,x,y) :
	                           generic_xy_to_chan(v,x,y);
      dactive = view_invert_display_index(v,active); /* to save time later.. */
      if(v->cursor_plot) v->cursor_plot(v, CURSOR_COLOR);
      return;
    }

    /* If MOVEMENT WITH BUTTON PRESSED: */
    if((active >= 0) && (id == LOC_DRAG)) {

      color = YA1_COLOR;
      /* WHAT IF IT IS NOT ALL IN MEMORY?!?!?!?! */
      if((s->file != SIG_NEW) &&
	  !check_file_name(s,".ed")) {	/* (new) edited signal saved on exit */
	update_hdr_for_mod(s);
      }

      close_sig_file(s);
      s->file = SIG_NEW;	/* indicate a modified signal */

      val = y_to_val(v,y,dactive); /* retrieve virtual data value at cursor*/

      assign_value(s, v, active, ind, val);

      fill_in(s, v, active, val, indo, ind);

      pw = canvas_pixwin(v->canvas);
      rect = (struct rect*)xv_get(v->canvas, WIN_RECT);
      pw_vector((Xv_opaque)pw,x-1,y,x+1,y,PIX_SRC|PIX_COLOR(color),color);
      v->cursor_time = time;
      if(v->y_print) v->y_print(v);
      indo = ind;
      return;
    }

    /* When BUTTON IS RELEASED: */
    if (event_is_up(event)
	&& (id != LOC_WINENTER)
	&& (id != LOC_WINEXIT)
	&& (id != LOC_MOVE))
    {
      if((active >= 0) && v->redraw_on_release)
	    redoit(v->canvas);
	else
	    if (v->cursor_plot) v->cursor_plot(v, CURSOR_COLOR);
	 /* Optionally, save the signal after each edit. */
	if(v->rewrite_after_edit &&
	   (v->sig->file_size == v->sig->buff_size)) {
	    if(v->sig->file == SIG_NEW)
		put_waves_signal(v->sig);
	    else
		put_signal(v->sig);
	}
	active = -1;
	indo = -1;
    }
    return;
  }
  return;
}

/*********************************************************************/
/* Given the Signal s, represented in View v, modify displayed element
   chan at index ind to the new value val.  The old value is returned
   by this function.  */
double  assign_value(s, v, chan, ind, val)
     Signal *s;
     View *v;
     int ind, chan;
     double val;
{
  if(s && v && s->data && ind >= 0) {
    double oldv;

    switch(s->type & VECTOR_SIGNALS)
    {
    case P_SHORTS:
      {
	short	*s_data = ((short **) s->data)[chan];

	if (!s_data)
	    return 0.0;
	oldv = s_data[ind];
	s_data[ind] = ROUND(val);
      }
      break;
    case P_INTS:
      {
	int	*s_data =  ((int **) s->data)[chan];

	if (!s_data)
	    return 0.0;
	oldv = s_data[ind];
	s_data[ind] = ROUND(val);
      }
      break;
    case P_DOUBLES:
      {
	double	*s_data =  ((double **) s->data)[chan];

	if (!s_data)
	    return 0.0;
	oldv = s_data[ind];
	s_data[ind] = val;
      }
      break;
    case P_CHARS:
      {
	char	*s_data =  ((char **) s->data)[chan];

	if (!s_data)
	    return 0.0;
	oldv = s_data[ind];
	s_data[ind] = ROUND(val);
      }
      break;
    case P_FLOATS:
      {
	float	*s_data =  ((float **) s->data)[chan];

	if (!s_data)
	    return 0.0;
	oldv = s_data[ind];
	s_data[ind] = val;
      }
      break;
    case P_MIXED:
      {
	int	s_type = s->types[chan];
	caddr_t	s_data = ((caddr_t *) s->data)[chan];

	if (!s_data)
	    return 0.0;

	switch (s_type)
	{
	case P_SHORTS:
	  oldv = ((short *) s_data)[ind];
	  ((short *) s_data)[ind] = ROUND(val);
	  break;
	case P_INTS:
	  oldv = ((int *) s_data)[ind];
	  ((int *) s_data)[ind] = ROUND(val);
	  break;
	case P_DOUBLES:
	  oldv = ((double *) s_data)[ind];
	  ((double *) s_data)[ind] = val;
	  break;
	case P_CHARS:
	  oldv = ((char *) s_data)[ind];
	  ((char *) s_data)[ind] = ROUND(val);
	  break;
	case P_FLOATS:
	  oldv = ((float *) s_data)[ind];
	  ((float *) s_data)[ind] = val;
	  break;
	default:
	  return(0.0);
	}
      }
      break;
    default:
      return(0.0);
    }
    return(oldv);
  }
  return(0.0);
}

/*********************************************************************/
/* Given the Signal s, retrieve element chan at index ind. */
double  signal_get_value(s, chan, ind)
     Signal *s;
     int ind, chan;
{
  if(s && s->data && ind >= 0) {
    double val;

    if(chan >= s->dim)
      chan = s->dim - 1;
    else
      if(chan < 0) chan = 0;

    switch(s->type & VECTOR_SIGNALS) {
    case P_SHORTS:
      {
	short	*s_data = ((short**)(s->data))[chan];

	if (!s_data)
	    return 0.0;
	val = s_data[ind];
      }
      break;
    case P_INTS:
      {
	int	*s_data = ((int**)(s->data))[chan];

	if (!s_data)
	    return 0.0;
	val = s_data[ind];
      }
      break;
    case P_DOUBLES:
      {
	double	*s_data = ((double**)(s->data))[chan];

	if (!s_data)
	    return 0.0;
	val = s_data[ind];
      }
      break;
    case P_CHARS:
      {
	char	*s_data = ((char**)(s->data))[chan];

	if (!s_data)
	    return 0.0;
	val = s_data[ind];
      }
      break;
    case P_FLOATS:
      {
	float	*s_data = ((float**)(s->data))[chan];

	if (!s_data)
	    return 0.0;
	val = s_data[ind];
      }
      break;
    case P_MIXED:
      {
	int	s_type = s->types[chan];
	caddr_t	s_data = ((caddr_t *)(s->data))[chan];

	switch (s_type)
	{
	case P_SHORTS:
	  val = ((short *) s_data)[ind];
	  break;
	case P_INTS:
	  val = ((int *) s_data)[ind];
	  break;
	case P_DOUBLES:
	  val = ((double *) s_data)[ind];
	  break;
	case P_CHARS:
	  val = ((char *) s_data)[ind];
	  break;
	case P_FLOATS:
	  val = ((float *) s_data)[ind];
	  break;
	default:
	  return(0.0);
	}
      }
      break;
    default:
      return(0.0);
    }
    return(val);
  }
  return(0.0);
}

/*********************************************************************/
/* Given the Signal s, represented in View v, retrieve displayed
   element chan at index ind. */

double  get_displayed_value(s, v, chan, ind)
     Signal *s;
     View *v;
     int ind, chan;
{
  if(s && v) {
    if(chan >= v->dims)
      chan = v->dims - 1;
    else
      if(chan < 0) chan = 0;
    return(signal_get_value(s, v->elements[chan], ind));
  } else
    return(0.0);
}
	   
/*********************************************************************/
/* Interpolate any skipped frames.  For channel chan in Signal s,
   shown in View v; interpolate from the value with index indo to the
   value val at index ind.  Intermediate samples are linearly
   interpolated.
*/
fill_in(s, v, chan, val, indo, ind)
     Signal *s;
     View *v;
     double val;
     int chan, indo, ind;
{
  if(s && v) {
    double valf, valo, dv;
    int j, k, nf, nfs;

    if((indo >= 0) && ((nf = iabs((nfs = indo - ind))) > 1)) {
      
      valo = signal_get_value(s, chan, indo);
      
      if(nfs > 0) {
	dv = ((double)(valo-val))/nf;
	valf = val;
	k = ind;
      } else {
	dv = ((double)(val-valo))/nf;
	valf = valo;
	k = indo;
      }

      for(j=1 ; j < nf; j++) {	/* Interpolate. */
	valf += dv;
	assign_value(s, v, chan, j+k, valf);
      }

    }
  }
}

/*********************************************************************/
/*
 * The following routines permit modification or generation of signals
 * by cutting and pasting waveform segments.
 */
/*********************************************************************/

/*********************************************************************
  Segment selection: Segments are selected by positioning the left and
  right markers using mouse buttons in either "up/down" or "move closest"
  mode.

  Operations on selected segments: Save in file; delete; (eventually
  expand; contract {by either interpolation or delete/iterate}).

  Filename selection: Four modes: (1) explicit name type-in;
  (2) auto-increment of numeric part of filename; (3) names read from a
  file-list file; (4) mktemp filenames based on the name of the signal
  from which the segment was excised.

  In all cases a brouseable list of these file names will be maintained,
  with mouse selection of any list element for: D/A playback;
  insertion into another signal; header printout; deletion; removal from
  the list.
*/


/*******************************************************************/
element_size(s, i)
    register Signal *s;
    int	    	    i;
{
  if(s)
    switch(s->type & VECTOR_SIGNALS) {
    case P_INTS:
    case P_UINTS:
      return(sizeof(int));
    case P_SHORTS:
    case P_USHORTS:
      return(sizeof(short));
    case P_CHARS:
    case P_UCHARS:
      return(sizeof(char));
    case P_FLOATS:
      return(sizeof(float));
    case P_DOUBLES:
      return(sizeof(double));
    case P_MIXED:
      switch (s->types[i])
      {
      case P_INTS:
      case P_UINTS:
	  return(sizeof(int));
      case P_SHORTS:
      case P_USHORTS:
	  return(sizeof(short));
      case P_CHARS:
      case P_UCHARS:
	  return(sizeof(char));
      case P_FLOATS:
	  return(sizeof(float));
      case P_DOUBLES:
	  return(sizeof(double));
      }
    default:
      sprintf(notice_msg, 
           "Unknown VECTOR_SIGNAL in element_size(%x, %d)\n", s->type, i);
      show_notice(1,notice_msg);
    }
  return(0);
}

/*******************************************************************/

int
samp_size(s)
    Signal  *s;
{
    int	    i;
    int	    size = 0;

    for (i = 0; i < s->dim; i++)
	size += element_size(s, i);
    return size;
}

/*******************************************************************/
add_to_new_files_browser(realname)
     char *realname;
{
  extern Pending_input new_files;
  /* Put the new file at the head of the list and "activate" it for
       display purposes. */
    if(new_files.list) ((Menu_list*)new_files.list)->active = FALSE;
  if(add_to_menu_list(&new_files.list, realname))
    ((Menu_list*)new_files.list)->active = TRUE;
  if(new_files.canvas) menu_redoit(new_files.canvas, NULL, NULL);
}

/*******************************************************************/
save_segment(s,start_time,duration)
     Signal *s;
     double start_time, duration;
{
  char *ext, *name, next[200], realname[200], *cp;
  extern int dont_save_sgrams;
  extern char outputname[];

  if (((s->type & SPECIAL_SIGNALS) == SIG_SPECTROGRAM) &&
     dont_save_sgrams) return(TRUE);

  if(!make_edited_filename(s,realname))
    return(FALSE);

  /*
   * Might want to put in a check here for existing files and/or previously
   * used names.  At this point, ``realname'' has been set by by some means.
   */
 
  if(save_seg_in_file(s, start_time, duration, realname)) {
    add_to_new_files_browser(realname);
    return(TRUE);
  } else
    show_notice(1,"Problems in save_seg_in_file in save_segment");
}

/*********************************************************************/
do_deletions(v,start,end)
  View *v;
      double start, end;
{
  double dt;
  Signal *s = v->sig;
  char newname[256];

  if (v->sig->file != SIG_NEW)
    insert_numeric_ext(v->sig->name,++v->sig->version,newname);
  else
    *newname = '\0';

  if (delete_segment(s, start, end, newname)) {
    if(*newname)
      update_window_titles(v->sig);
    v->rmarker_time = v->lmarker_time;
    if(v->start_time < (dt = BUF_START_TIME(v->sig))) {
      v->start_time = dt;
      view_fit_duration(v);
    }
    if(ET(v) > (dt = BUF_END_TIME(v->sig)))
      view_fit_duration(v);
    
    redoit(v->canvas);
  }
}

#define RETURN(val,msg) \
  { if(msg) fprintf(stderr,"save_seg_in_file: %s\n",msg); return(val); }


/*******************************************************************/
save_generic_seg(s, start_time, duration, file)
     Signal *s;
     double start_time, duration;
     char *file;
{
  Signal *so;
  char comment[200];
  char **dpp, **dppi;
  int n, i, j, size;
  double etr, ets;

  if(!((start_time >= s->start_time) &&
       ((etr = start_time+duration) <=
	(ets = (s->start_time + SIG_DURATION(s)))))) {
    if((start_time >= ets) || (etr <= s->start_time)) {
      printf("Bogus times passed to save_seg_in_file(%s %f %f)\n",
	     s->name, start_time, duration);
      return(FALSE);
    }
    if(start_time < s->start_time) start_time = s->start_time;
    if(ets < etr) etr = ets;
    duration = etr - start_time;
    sprintf(notice_msg, 
    "Limits requested in save_seg_in_file exceeded signal limits.\nAdjusted to: start_time=%lf  duration=%lf",start_time,duration);
    show_notice(1,notice_msg);
  }

  n = 0.5 + s->freq * duration;

  if((so=new_signal(file, SIG_NEW, dup_header(s->header),
		    (caddr_t) NULL, n, s->freq, s->dim))) {
    sprintf(comment,"save_seg_in_file: start_time %f duration %f signal %s",
	    start_time, duration, s->name);
    clone_methods(so,s);
    so->start_time = start_time;
    so->file_size = n;
    so->end_time = BUF_END_TIME(so);

    if (so->header
	&& so->header->magic == ESPS_MAGIC
	&& so->header->esps_hdr)
    {
	struct header  *new_hdr;

	new_hdr = copy_header(so->header->esps_hdr);
	so->header->esps_hdr = new_hdr;
	set_pvd(new_hdr);
	add_comment(new_hdr, "xwaves ");
	strcat(comment, "\n");
	add_comment(new_hdr, comment);

	*(genhd_type("start_time", (int *) NULL, new_hdr) == DOUBLE
	  ? get_genhd_d("start_time", new_hdr)
	  : add_genhd_d("start_time", (double *) NULL, 1, new_hdr)
	  ) = start_time;

	*(genhd_type("end_time", (int *) NULL, new_hdr) == DOUBLE
	  ? get_genhd_d("end_time", new_hdr)
	  : add_genhd_d("end_time", (double *) NULL, 1, new_hdr)
	  ) = so->end_time;
    }

    if((BUF_START_TIME(s) <= start_time) &&
       (BUF_END_TIME(s) >= (start_time + duration))) { /* it's all in memory */
      if(!(so->data =
	   (caddr_t)(dpp = (char**)malloc(sizeof(char*) * s->dim)))) {
	printf("Allocation problems in save_seg_in_file()\n");
	free_signal(so);
	return(FALSE);
      }
      i = time_to_index(s,start_time);
      dppi = (char**)(s->data);
      for(j=0; j < s->dim; j++)	/* new pointer array for signal subsegment */
	dpp[j] = dppi[j] + element_size(s, j) * i;

      so->start_samp = 0;
      if(s->x_dat)
	so->x_dat = &(s->x_dat[i]);
      get_maxmin(so);
      head_printf(so->header,"operation",comment);
      head_printf(so->header,"samples", (char *) &n);
      head_printf(so->header,"version", (char *) &(so->version));
      head_printf(so->header,"start_time", (char *) &start_time);
      head_printf(so->header,"end_time", (char *) &(so->end_time));
      head_printf(so->header,"maximum", (char *) &(*so->smax));
      head_printf(so->header,"minimum",(char *) &(*so->smin));
      head_printf(so->header,"time",get_date());
      if(!(i = put_signal(so)))
	printf("put_signal() problems in save_seg_in_file()\n");
      free(so->data);
      so->data = NULL;
      so->x_dat = NULL;
      free_signal(so);
      return(i);
    } else {			/* must do read/write transfers */
      extern int max_buff_bytes;	/* defined in globals.c */
      double      chunk, ts, dur, amax, amin, tstep, st;
      int         bsize;

      chunk = ((double) max_buff_bytes)/(samp_size(s) * s->freq);

      bsize = (int)((duration < chunk)? duration * s->freq : chunk * s->freq);
      tstep = ((double)bsize)/s->freq;
      /* First, read over the data and find max and min. */
      for(amax = *s->smin, amin = *s->smax, dur=duration, ts=tstep,
	  st=start_time ;
	  dur > 0; dur -= ts, st += ts) {
	if(ts > dur) ts = dur;
	if(ts > 0.5/so->freq) { /* more than 1/2 sample interval? */
	  if(read_data(s,st,ts)) {
	    get_maxmin(s);
	    if(*s->smin < amin) amin = *s->smin;
	    if(*s->smax > amax) amax = *s->smax;
	  } else {
	    printf("read_data() error in save_seg_in_file()\n");
	    return(FALSE);
	  }
	}
      }
      head_printf(so->header,"operation",comment);
      head_printf(so->header,"samples", (char *) &n);
      head_printf(so->header,"version", (char *) &(so->version));
      head_printf(so->header,"start_time",  (char *) &start_time);
      head_printf(so->header,"end_time",  (char *) &(so->end_time));
      head_printf(so->header,"maximum", (char *) &amax);
      head_printf(so->header,"minimum", (char *) &amin);
      head_printf(so->header,"time",get_date());
      if(!output_header(so)) {
	printf("Problems with header or file open in save_seg_in_file()\n");
	return(FALSE);
      }
    /* Now do the real read/write transfers. */
      for(so->start_samp = 0, so->buff_size = bsize, dur=duration, ts=tstep ;
	  dur > 0; dur -= ts, start_time += ts) {
	if(ts > dur) ts = dur;
	if(ts > 0.5/so->freq) { /* more than 1/2 sample interval? */
	  if(read_data(s,start_time,ts)) {
	    so->data = s->data;
	    so->x_dat = s->x_dat;
	    if(so->utils->write_data(so,start_time,ts))
	      so->start_samp += bsize;
	    else {
	      printf("write_data() error in save_seg_in_file()\n");
	      return(FALSE);
	    }
	  } else {
	    printf("read_data() error in save_seg_in_file()\n");
	    return(FALSE);
	  }
	}
      }
      so->data = NULL;	/* NOTE: so->data was == s->data (don't free)*/
      so->x_dat = NULL;
      free_signal(so);
      if(s->views) 
	read_data(s,s->views->start_time,
		  ET(s->views) - s->views->start_time);
      close_sig_file(s);
      return(TRUE);
    }
  } else
    show_notice(1,"Can't create a scratch buffer signal in save_seg_in_file");
  return(FALSE);
}

/*******************************************************************/
save_seg_in_file(s, start_time, duration, file)
     Signal *s;
     double start_time, duration;
     char *file;
{
  Signal *so;
  char comment[200];
  char **dpp, **dppi;
  int n, i, j;

  if(IS_TAGGED_FEA(s))
      return(tag_save_seg(s, start_time, duration, file));
    
  if(s && (duration > 0.0)) {
    if (IS_GENERIC(s->type))
      return(save_generic_seg(s, start_time, duration, file));
  }
  sprintf(notice_msg,
           "Bad signal type (%x) or duration (%f) in save_seg_in_file()\n",
	   ((s)? s->type : 0), duration);
  show_notice(1,notice_msg);
  return(FALSE);
}

#undef RETURN  

/*******************************************************************/
delete_generic_segment(s, start_time, end_time, newname)
    Signal  *s;
    double  start_time, end_time;
    char    *newname;
{
#define MOVE_TYPES(type_1, type_2) { \
    type_1          *dpp = (type_1 *) s->data; \
    type_2          *dp = (type_2 *) dpp[dim]; \
    register type_2 *p, *q, *r; \
    if (!dp) { \
        show_notice(1, "NULL data array in delete_segment."); \
        return FALSE; \
    } \
    for (p = dp+is, q = dp+ie+1, r = dp+s->buff_size; q < r; ) \
        *p++ = *q++; \
    dpp[dim] = (type_1) realloc((char *) dp, new_size * sizeof(type_2)); \
  }

  int new_size, is, ie, dim;
  char comment[200];
  struct header	*hdr;
  
  if((start_time < end_time) && (start_time >= s->start_time) &&
     (end_time <= (s->start_time + ((double)s->file_size)/s->freq))) {

    if (!s->data)
    {
      show_notice(1, "NULL data array pointer in delete_segment.");
      return FALSE;
    }

    if(s->buff_size == s->file_size) { /* it's all in memory */

      is = time_to_index(s,start_time);
      ie = time_to_index(s,end_time);
      new_size = s->buff_size - (ie - is + 1);
      if(new_size <= 0) {
	show_notice(1,"Edited signal would have zero length; no op performed.");
	return(FALSE);
      }
      if (s->file != SIG_NEW)
	  rename_signal(s, newname);

      for(dim=0; dim < s->dim; dim++) {
	switch(s->type & VECTOR_SIGNALS) {
	case P_SHORTS:
	case P_USHORTS:
	    MOVE_TYPES(short *, short);
	    break;
	case P_INTS:
	case P_UINTS:
	    MOVE_TYPES(int *, int);
	    break;
	case P_FLOATS:
	    MOVE_TYPES(float *, float);
	    break;
	case P_DOUBLES:
	    MOVE_TYPES(double *, double);
	    break;
	case P_CHARS:
	case P_UCHARS:
	    MOVE_TYPES(char *, char);
	    break;
        case P_MIXED:
	    switch (s->types[dim])
	    {
	    case P_SHORTS:
	    case P_USHORTS:
		MOVE_TYPES(caddr_t, short);
		break;
	    case P_INTS:
	    case P_UINTS:
		MOVE_TYPES(caddr_t, int);
		break;
	    case P_FLOATS:
		MOVE_TYPES(caddr_t, float);
		break;
	    case P_DOUBLES:
		MOVE_TYPES(caddr_t, double);
		break;
	    case P_CHARS:
	    case P_UCHARS:
		MOVE_TYPES(caddr_t, char);
		break;
	    default:
		sprintf(notice_msg,"Bad data type (%x) in delete_segment",
		       s->types[dim]);
                show_notice(1,notice_msg);
		return(FALSE);
	    }
	    break;
	default:
	    sprintf(notice_msg, 
                       "Bad data type (%x) in delete_segment()\n",s->type);
            show_notice(1,notice_msg);
	    return(FALSE);
	}
      }
      s->buff_size = s->file_size = new_size;
      sprintf(comment,"delete_segment: start_time %f end_time %f signal %s",
	      start_time, end_time, s->name);
      head_printf(s->header,"operation",comment);
      head_printf(s->header,"samples",(char*)&new_size);
      head_printf(s->header,"version", (char *) &(s->version));
      if (!is) {		/* deleted leading segment */
	s->start_time += ((double)(ie+1))/s->freq;
	head_printf(s->header,"start_time",(char*)&s->start_time);
      }
      s->end_time = BUF_END_TIME(s);
      head_printf(s->header,"end_time", (char *) &(s->end_time));
      get_maxmin(s);
      head_printf(s->header,"maximum", (char *) &(*s->smax));
      head_printf(s->header,"minimum", (char *) &(*s->smin));
      head_printf(s->header,"time",get_date());

      if (s->header
	  && s->header->magic == ESPS_MAGIC
	  && (hdr = s->header->esps_hdr))
      {
	  strcat(comment, "\n");
	  add_comment(hdr, comment);

	  *(genhd_type("start_time", (int *) NULL, hdr) == DOUBLE
	    ? get_genhd_d("start_time", hdr)
	    : add_genhd_d("start_time", (double *) NULL, 1, hdr)
	    ) = s->start_time;

	  *(genhd_type("end_time", (int *) NULL, hdr) == DOUBLE
	    ? get_genhd_d("end_time", hdr)
	    : add_genhd_d("end_time", (double *) NULL, 1, hdr)
	    ) = s->end_time;
      }

      return(TRUE);
    } else
      show_notice(1,"Can't handle partially buffered signals");
  } else {
    sprintf(notice_msg,
           "Absurd boundary times passed to delete_segment(%f,%f)\n",
	   start_time, end_time);
    show_notice(1,notice_msg);
  }
  return(FALSE);

#undef MOVE_TYPES
}

/*******************************************************************/
/* Delete a segment from a signal.  Create a new signal as a result (the
   original signal will remain unchanged).  If the original signal is
   entirely in memory, delete the requested segment and simply mark the
   signal to be saved.  If it is a "large" signal (i.e. not entirely in
   memory), perform read/writes as required to delete the signal within
   the memory constraints imposed by "page_size." */

delete_segment(s, start_time, end_time, newname)
    Signal  *s;
    double  start_time, end_time;
    char    *newname;
{
  if(s && IS_GENERIC(s->type))
    return(delete_generic_segment(s, start_time, end_time, newname));
  printf("Bad signal type in delete_segment\n.");
  return(FALSE);
}
	   

/* 11-13-87 mcb
 *			File Naming Schemes
 * Here are some facts to keep in mind when reading the commentary below:
 * (1) When a signal is modified (waveform redrawn with cursor), the
 *     extension ".ed" is included in the current name as follows:
 *     "abc"        -> "abc.ed"
 *     "abc.x"      -> "abc.ed.x"
 *     "abc.x.y.z"  -> "abc.x.y.ed.z"
 * (2) When a segment is saved and NO OUTPUT NAME IS GIVEN, the extension
 *     ".#<number>" is combined with the parent signal's name to form a name
 *     for the new segment signal.  <number> is a counter (separate for each
 *     signal) that is incremented for each save operation.  Extension
 *     placement is the same as for ".ed" above, e.g. "abc.#1.x".
 * (3) When a segment is saved and there is an output name, things work
 *     exactly as before (which I will not try to explain just now).
 * (4) When a segment is deleted, the original name will be changed according
 *     to the rules for saving segments in (2) above, e.g. "abc.#2.x".
 */
  
/* When segments are copied from a signal to a separate file the problem
   of how to name the new file arises.  This problem is more severe
   when "edit_ganged" is set and there are several elements in the display
   ensemble.  Some possible alternatives: (1) Use the original filename made
   unique by mktemp(); the extension, if present, would remain the same,
   else, the last character would remain the same.  (2) Use a simplified
   version of the original file name made unique by mktemp; an extension
   would be generated by table lookup on the file type.  (3) Append
   incrementing digits to the filename to maintain uniqueness leaving the
   extension, if any, unchanged.  (4) Read a basename from a list of
   files to be generated; maintain uniqueness with incrementing digits
   (or mktemp()); append extensions which indicate file type.

   It is desirable that all files generated by editing operations be
   listed and readily accessible by mousing the list.  The listing window
   could have a menu providing the following operations on listed objects:
   (1) Read into and display by waves
   (2) Remove form list
   (3) Delete file (and remove form list)
   (4) D/A convert (possibly after synthesis), if possible

   A list of file objects may also be generated by the unix ls command,
   or reading from a file.  These lists should also be manipulable as
   above.  Each list can have a constant pathname prefix which is
   not repeated for each list element in the listing, but is prepended as
   necessary when operations are performed.

   A general procedure for accessing existing files could be:
   If a complete, correct filename corresponding to a signal is typed to
   a panel item, the file is accessed directly as usual.  If the
   pathname is terminated with a "/" or a "?" the trailing "? or "/" will be
   removed and a listing of the corresponding path will be generated.
   Selection of an item from the resulting list will then cause the panel
   item name to be completed and the list window to be destroyed.  A structure
   "pending_input" could be set to indicate what item is to get the selection.
   typedef struct pending_input {
   char *results_to;		/* target array to be filled 
   char *path_prefix;		/* the pathname to be completed 
   Panel_item item;		/* the text item, if any, to be updated 
   caddr_t list;		/* pointer to alternatives list
   Canvas canvas;               /* alternatives display window
   void (*proc)();	     /*  to be called with item and full name as arg.
   int destroy_on_select;	/* boolean for window destroy 
   char *banner;		/* optional string for window label 
   struct pending_input *next;
   struct pending_input *prev;
   } Pending_input;

*/

