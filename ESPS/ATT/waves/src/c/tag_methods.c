/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1993-1997  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin
 *
 * tag_methods.c
 *
 * A collection of methods for displaying and editing so-called
 *                    "tagged" ESPS files.
 *
 */

static char *sccs_id = "@(#)tag_methods.c	1.9	1/18/97	ATT/ERL";

#include <stdio.h>
#ifndef hpux
#include <sys/param.h>
#else
#define MAXPATHLEN 1024
#endif
#include <esps/esps.h>
#include <Objects.h>
#if !defined(HP400) && !defined(APOLLO_68K) && !defined(DS3100) && !defined(LINUX_OR_MAC)
#include <malloc/malloc.h>
#endif
#include <esps/exview.h>

extern int debug_level;
extern Xv_Font def_font;
extern int     def_font_height, def_font_width;

#define scoff(x) ((int)(yoffs + ((double)(x) * scale)))
#define ALMOST 0.95

/*********************************************************************/
tag_time_to_x(v,time)
     register View *v;
     register double time;
{
  register int i;
  register double freq, t;

  /* "integerize" time */
  i = time_to_index(v->sig,time);
  time = v->sig->x_dat[i];
  if (time < v->start_time) time = v->start_time; /* clamp */
  if (time > (t = ET(v)))   time = t;
  return((int)(.5 + (PIX_PER_CM *
     ((time - v->start_time) / (*v->x_scale)))) + (*v->x_offset));
}
	 
/*********************************************************************/
double 
tag_x_to_time(v,x)
     register View *v;
     register int x;
{
  double time, t;

  x -= *(v->x_offset);
  if(x < 0) x = 0;
  time = v->start_time + (double)x * *(v->x_scale) / PIX_PER_CM;
  if (time > (t = ET(v)))   time = t;
  return(v->sig->x_dat[time_to_index(v->sig,time)]);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Aperiodic vector and scalar signals are plotted using a connect-the-dots
   algorithm.  Arbitrary x-axis scaling, y_offsets and y-axis scaling are
   supported. */
tag_plot_waves(view)
     register View *view;
{
  register int y, y2, j, k, npix, x;
  View *vh = view;
  int index, ind, nsam, maxdim, its_an_overlay = FALSE;
  int val, dim;
  Pixwin *pw;
  Signal *sig;
  Canvas canvas;
  double end;
  register double scale, yoffs;
  struct rect	*rect;
  extern int	show_labels, doing_print_graphic, print_only_plot;
  List *l, *l2;

  if(view && (sig = view->sig) && sig->type) {

    if(view->extra && (view->extra_type == VIEW_OVERLAY)) {
      vh = (View*)view->extra;
      its_an_overlay = TRUE;
    }
    
    if(IS_GENERIC(view->sig->type)) {

      canvas = vh->canvas;
  
      pw = canvas_pixwin(canvas); /* get the pixwin to write to */

      rect = (struct rect*)xv_get(canvas, WIN_RECT);

      /* Determine the number of waveform samples to plot */
      if(vh->start_time < BUF_END_TIME(sig)) {
	end = ET(vh);
	/* deternine starting sample in the waveform */
	index = time_to_index(sig,vh->start_time);
	for(nsam=0,j=index; (nsam < sig->buff_size) &&
	    (sig->x_dat[j] <= end); j++) nsam++;
      } else {
	return(FALSE);
      }

      if(index+nsam >= sig->buff_size-1)
	nsam = sig->buff_size - 1 - index;

      /* clear window if data is NOT plotted as an overlay */
      if( !its_an_overlay && !doing_print_graphic)
	pw_write(pw,0,0,rect->r_width,rect->r_height,
		 PIX_COLOR(BACKGROUND_COLOR)|PIX_SRC,NULL,0,0);

#define PLOT_TYPE(the_type) { \
  register the_type *q, *r, *p, imax, imin; \
   \
  if (nsam > 0 && !s_data) \
    return FALSE; \
   \
  val = view->colors[dim]; \
  scale = - PIX_PER_CM/vh->y_scale[dim]; \
  yoffs = .5 + (double)vh->y_offset[dim]; /* precompute for speed */ \
  switch(view->line_types[dim]) { \
  case 1:		/* standard solid line type */ \
  default: \
      k = *(vh->x_offset) + 0.5 + \
	((PIX_PER_CM * (sig->x_dat[index] - vh->start_time) / *(vh->x_scale))); \
    for(ind = index+1, p = (the_type *) s_data + index, \
	r = p + 1,  q = p + nsam; p < q; ind++) { \
      x = *(vh->x_offset) + 0.5 + \
	((PIX_PER_CM * (sig->x_dat[ind] - vh->start_time) / *(vh->x_scale))); \
      pw_vector(pw, k, scoff(*p++), x, scoff(*r++), \
		PIX_COLOR(val)|PIX_SRC, val); \
      k = x; \
    } \
    break; \
  case 2:			/* histogram-style */ \
    k = *(vh->x_offset) + 0.5 + \
        ((PIX_PER_CM * (sig->x_dat[index] - vh->start_time) / *(vh->x_scale))); \
    for(ind = index+1, y2 = scoff(sig->smin[view->elements[dim]]), \
	p = (the_type *) s_data + index, \
	r = p + 1,  q = p + nsam; p < q; ind++) { \
      x = *(vh->x_offset) + 0.5 + \
	((PIX_PER_CM * (sig->x_dat[ind] - vh->start_time) / *(vh->x_scale))); \
      pw_vector(pw, k, (y = scoff(*p++)), x, scoff(*r++), \
		PIX_COLOR(val)|PIX_SRC, val); \
      pw_vector(pw, k, y, k, y2, PIX_COLOR(val)|PIX_SRC, val); \
      k = x; \
    } \
    break; \
  case 3:			/* DSP-style */ \
  case 4:			/* DSP-style with connect the dots */ \
    k = *(vh->x_offset) + 0.5 + \
        ((PIX_PER_CM * (sig->x_dat[index] - vh->start_time) / *(vh->x_scale))); \
    for(ind = index+1, y2 = scoff(0.0), \
	p = (the_type *) s_data + index, \
	r = p + 1,  q = p + nsam; p < q; ind++) { \
      x = *(vh->x_offset) + 0.5 + \
	((PIX_PER_CM * (sig->x_dat[ind] - vh->start_time) / *(vh->x_scale))); \
      y = scoff(*p++); \
      if(view->line_types[dim] == 4) \
	pw_vector(pw, k, y, x, scoff(*r++), \
		PIX_COLOR(val)|PIX_SRC, val); \
      pw_vector(pw, k, y, k, y2, PIX_COLOR(val)|PIX_SRC, val); \
      k = x; \
    } \
    break; \
  } \
}
  /* >>>  END of PLOT_TYPE() definition  <<< */

      if (nsam > 0)
      {
	  maxdim = MIN(view->dims, vh->dims);

	  if (maxdim > 0 && !sig->data)
	      return FALSE;

	  for (dim=0; dim < maxdim; dim++) /* for each dimension... */
	      switch(sig->type & VECTOR_SIGNALS)
	      {
	      case P_CHARS:
	      case P_UCHARS:
		{
		    char    *s_data = ((char **) sig->data)[vh->elements[dim]];

		    PLOT_TYPE(char);
		}
		break;
	      case P_SHORTS:
		{
		    short   *s_data = ((short **) sig->data)[vh->elements[dim]];

		    PLOT_TYPE(short);
		}
		break;
	      case P_INTS:
	      case P_UINTS:
		{
		    int     *s_data = ((int **) sig->data)[vh->elements[dim]];

		    PLOT_TYPE(int);
		}
		break;
	      case P_FLOATS:
		{
		    float   *s_data = ((float **) sig->data)[vh->elements[dim]];

		    PLOT_TYPE(float);
		}
		break;
	      case P_DOUBLES:
		{
		    double  *s_data = ((double **) sig->data)[vh->elements[dim]];

		    PLOT_TYPE(double);
		}
		break;
	      case P_MIXED:
		{
		    caddr_t s_data = ((caddr_t *) sig->data)[vh->elements[dim]];

		    switch (sig->types[vh->elements[dim]])
		    {
		    case P_CHARS:
		    case P_UCHARS:
			PLOT_TYPE(char);
			break;
		    case P_SHORTS:
			PLOT_TYPE(short);
			break;
		    case P_INTS:
		    case P_UINTS:
			PLOT_TYPE(int);
			break;
		    case P_FLOATS:
			PLOT_TYPE(float);
			break;
		    case P_DOUBLES:
			PLOT_TYPE(double);
			break;
		    default:
			sprintf(notice_msg,
				"Unknown component data type in plot_waves(%x)\n",
				sig->types[vh->elements[dim]]);
			show_notice(1,notice_msg);
			return(FALSE);
		    }
		}
		break;
	      default:
		goto arrgh;
	      } /* end switch (sig->type ...) */
      } /* end if (nsam ...) */

    } else {
    arrgh:
      sprintf(notice_msg,"Unknown data type in plot_waves(%x)\n", 
                         view->sig->type );
      show_notice(1,notice_msg);
      return(FALSE);
    }    

    if((!its_an_overlay) && (l = sig->idents)) {
      int curi, i;
/*      double sigmin; */
      
      for(dim=0; dim < view->dims; dim++) { /* for each dimension... */
	if(view->show_labels[dim]) {
	  curi = view->elements[dim];
	  for(i = 0, l2 = l; i < curi; i++)
	    if(l2->next)
	      l2 = l2->next;
/*	  sigmin = view->sig->smin[curi];

	  if (plot_min != 0)
	    sigmin = plot_min;

	  if (view->sig->smax[curi] - sigmin == 0.0)
	      sigmin -= 0.5; */	/* Cf. comment on similar adjustment
				 * in print_y.
				 */
	  scale = - PIX_PER_CM/view->y_scale[dim];
	  yoffs = .5 + (double)view->y_offset[dim];	  
	  if(!(doing_print_graphic && print_only_plot))
	      pw_text(pw, *(view->x_offset), scoff(view->plot_min[dim]),
		  PIX_SRC|PIX_COLOR(TEXT_COLOR), def_font, l2->str);
	}
      }
    }
    return(TRUE);
  }
  return(FALSE);
#undef PLOT_TYPE
}

/*********************************************************************/
View *
tag_setup_view(s, c)
     Signal *s;
     Canvas c;
{
View *v, *setup_generic_view();
if(s && c) {
  if((v = setup_generic_view(s,c))) {
    v->data_plot = tag_plot_waves;
    v->time_to_x = tag_time_to_x;
    v->x_to_time = tag_x_to_time;
    }
    return(v);
  }
  return(NULL);
}


/*******************************************************************/
tag_save_seg(s, start_time, duration, file)
     Signal *s;
     double start_time, duration;
     char *file;
{
  Signal *so;
  char comment[200];
  char **dpp, **dppi;
  int n, i, j, size;
  double etr, ets;
  char *ptr;
  /* Pixrect   *pr, *pro; */

  if(!((start_time >= s->start_time) &&
       ((etr = start_time+duration) <=
	(ets = (s->start_time + SIG_DURATION(s)))))) {
    if((start_time >= ets) || (etr <= s->start_time)) {
      sprintf(notice_msg, "Bogus times passed to tag_save_seg(%s %f %f)",
	     s->name, start_time, duration);
      show_notice(1,notice_msg);
      return(FALSE);
    }
    if(start_time < s->start_time) start_time = s->start_time;
    if(ets < etr) etr = ets;
    duration = etr - start_time;
    sprintf(notice_msg, 
             "Limits requested in tag_save_seg exceeded signal limits.\n");
    ptr = notice_msg + strlen(notice_msg);
    sprintf(ptr,"adjusted to: start_time=%lf  duration=%lf",
	   start_time,duration);
    show_notice(1,notice_msg);
  }

  if((so=new_signal(file, SIG_NEW, dup_header(s->header),
		    (caddr_t) NULL, 0, s->freq, s->dim))) {
    sprintf(comment,"tag_save_seg: start_time %f duration %f signal %s",
	    start_time, duration, s->name);
    so->start_time = start_time;

    so->end_time = etr;
    clone_methods(so,s);
    if (so->header
	&& so->header->magic == ESPS_MAGIC
	&& so->header->esps_hdr)
    {
	struct header  *new_hdr;
	int points;
	double ssf;
	
	if((ssf = get_genhd_val("src_sf",s->header->esps_hdr,-1.0)) <= 0.0) {
	  show_notice(0,
	     "Warning: src_sf not present in tagged file; guessing 8kHz");
	  ssf = 8000.0;
	}
	
	if(ssf != s->freq)
	  points = 0.5 + (ssf * duration);
	else
	  points = s->file_size;
	
	new_hdr = copy_header(so->header->esps_hdr);

	so->header->esps_hdr = new_hdr;
	set_pvd(new_hdr);
	add_comment(new_hdr, "xwaves ");
	strcat(comment, "\n");
	add_comment(new_hdr, comment);

	*(genhd_type("nan", (int *) NULL, new_hdr) == LONG
	  ? get_genhd_l("nan", new_hdr)
	  : add_genhd_l("nan", (long *) NULL, 1, new_hdr)
	  ) = points;
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
       (BUF_END_TIME(s) >= (start_time + duration))) {
      if(!(so->data =
	   (caddr_t)(dpp = (char**)malloc(sizeof(char*) * s->dim)))) {
	printf("Allocation problems in tag_save_seg()\n");
	free_signal(so);
	return(FALSE);
      }
      i = time_to_index(s,start_time);
      dppi = (char**)(s->data);
      for(j=0; j < s->dim; j++)	/* new pointer array for signal subsegment */
	dpp[j] = dppi[j] + element_size(s, j) * i;
      so->x_dat = s->x_dat + i;
      so->start_samp = 0;
      so->file_size = so->buff_size = time_to_index(s,etr) - i + 1;
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
	printf("put_signal() problems in tag_save_seg()\n");
      free(so->data);
      so->data = NULL;
      so->x_dat = NULL;
      free_signal(so);
      return(i);
    } else {			/* must do read/write transfers */
      extern int max_buff_bytes;	/* defined in globals.c */
      double      ts, dur, amax, amin, tstep, st;
      int         bsize;

      bsize = s->buff_size; /* make chunks approx. equal to current source */
      tstep = BUF_DURATION(s);
      /* First, read over the data and find max and min. */
      for(amax = *s->smin, amin = *s->smax, dur=duration, ts=tstep,
	  st=start_time ;
	  dur > 0;) {
	if(ts > dur) ts = dur;
	if(ts > 0.5/so->freq) { /* more than 1/2 sample interval? */
	  if(s->utils->read_data(s,st,ts)) {
	    get_maxmin(s);
	    if(*s->smin < amin) amin = *s->smin;
	    if(*s->smax > amax) amax = *s->smax;
	  } else {
	    printf("read_data() error in tag_save_seg()\n");
	    return(FALSE);
	  }
	  dur -= BUF_DURATION(s);
	  st += BUF_DURATION(s);
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
	printf("Problems with header or file open in tag_save_seg()\n");
	return(FALSE);
      }
    /* Now do the real read/write transfers. */
      for(so->start_samp = 0, dur=duration, ts=tstep ; dur > 0; ) {
	if(ts > dur) ts = dur;
	if(ts > 0.5/so->freq) { /* more than 1/2 sample interval? */
	  if(s->utils->read_data(s,start_time,ts)) {
	    so->data = s->data;
	    so->x_dat = s->x_dat;
	    so->buff_size = s->buff_size;
	    if(so->utils->write_data(so,start_time,ts))
	      so->start_samp += so->buff_size;
	    else {
	      printf("write_data() error in tag_save_seg()\n");
	      return(FALSE);
	    }
	  } else {
	    printf("read_data() error in tag_save_seg()\n");
	    return(FALSE);
	  }
	  dur -= BUF_DURATION(so);
	  start_time += BUF_DURATION(so);
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
    printf("Can't create a scratch buffer signal in tag_save_seg()\n");
  return(FALSE);
}

/*********************************************************************/
Signal *
tag_insert_signal(s1,s2,time)
     Signal *s1, *s2;
     double time;
{
  /* Assumes s1 and s2 are both on disc and that both are tagged filed;
     creates a new signal for the
     paste-up and returns a pointer to this new signal. s1 and s2 remain
     unchanged, except for their buffer positions. */
  if(compatable(s1,s2) && (time >= s1->start_time) &&
     (time <= SIG_END_TIME(s1))) {
    char    tname[300], mess[200];
    Signal  *stmp;
    double  rtime, wtime, rinc, smax, smin, amax, amin, half_samp,
            ssf = 8000.0;
    int	    fd_tmp, skip_tmp, bs, s2start;
    Header  *hdr_tmp;
    
    sprintf(tname,"%sXXXXXX",s1->name);	/* temp name for constructing output */
    /* Duplicate header and structure of sink file. */
#if !defined(LINUX)
    if((stmp = new_signal(mktemp(tname),SIG_NEW,dup_header(s1->header),NULL,
#else
    if((stmp = new_signal(mkstemp(tname),SIG_NEW,dup_header(s1->header),NULL,
#endif
			  s1->file_size+s2->file_size,s1->freq,s1->dim))) {
      clone_methods(stmp,s1);
      head_scanf(s1->header,"maximum",&smax);
      head_scanf(s1->header,"minimum",&smin);
      head_scanf(s2->header,"maximum",&amax);
      head_scanf(s2->header,"minimum",&amin);
      if(smax > amax) amax = smax;
      if(smin < amin) amin = smin;
      head_printf(stmp->header,"maximum",&amax);
      head_printf(stmp->header,"minimum",&amin);
      sprintf(mess,"tag_insert_signal: source %s sink %s time %f",s2->name,
	      s1->name,time);
      head_printf(stmp->header,"operation",mess);
      /* Output size is combined size of inputs. */
      stmp->file_size = stmp->buff_size;
      stmp->buff_size = 0;	/* Indicate current buffer is empty. */
      head_printf(stmp->header,"samples",&(stmp->file_size)); 
      if(time <= s1->start_time) { /* Initial samples from source or sink? */
	time = s1->start_time;
	stmp->start_time = s2->start_time;
	s2start = TRUE;		/* from source */
	head_printf(stmp->header,"start_time",&(stmp->start_time));
      } else
	s2start = FALSE;	/* from sink */
      stmp->end_time = stmp->start_time + SIG_DURATION(s1) + SIG_DURATION(s2);
      head_printf(stmp->header,"end_time",&(stmp->end_time));
      if (stmp->header
	  && stmp->header->magic == ESPS_MAGIC
	  && stmp->header->esps_hdr)
	{
	  struct header	*hdr;
	  int points;

	  if((ssf = get_genhd_val("src_sf",s1->header->esps_hdr,-1.0)) <= 0.0) {
	    show_notice(0,
	     "Warning: src_sf not present in tagged file; guessing 8kHz");
	    ssf = 8000.0;
	  }
	  if(ssf != stmp->freq)
	    points = 0.5 + (ssf * (stmp->end_time - stmp->start_time));
	  else
	    points = s1->file_size + s2->file_size;
	  hdr = stmp->header->esps_hdr = copy_header(stmp->header->esps_hdr);
	  set_pvd(hdr);
	  add_comment(hdr, "xwaves ");
	  strcat(mess, "\n");
	  add_comment(hdr, mess);
	  *(genhd_type("nan", (int *) NULL, hdr) == LONG
	    ? get_genhd_l("nan", hdr)
	    : add_genhd_l("nan", (long *) NULL, 1, hdr)
	    ) = points;
	  *(genhd_type("start_time", (int *) NULL, hdr) == DOUBLE
	    ? get_genhd_d("start_time", hdr)
	    : add_genhd_d("start_time", (double *) NULL, 1, hdr)
	    ) = stmp->start_time;
	  *(genhd_type("end_time", (int *) NULL, hdr) == DOUBLE
	    ? get_genhd_d("end_time", hdr)
	    : add_genhd_d("end_time", (double *) NULL, 1, hdr)
	    ) = stmp->end_time;
	}
      if(output_header(stmp)) {
	if(s1->file_size > 1)
	  half_samp = 0.5 * SIG_DURATION(s1)/s1->file_size;
	else
	  half_samp = 1.0/ssf;

	stmp->start_samp = 0;
	bs = 0;
	if(!s2start)	/* Output starts with samples from sink file. */
	  for(rtime=stmp->start_time, rinc=10.0;
	      rtime+half_samp < time; rtime += rinc) {
	    if((rtime+rinc) > time) rinc = time - rtime;
	    if(rinc > 0.0) {
	      if(s1->utils->read_data(s1,rtime,rinc)) {
		bs = s1->buff_size;
		if((rinc = BUF_DURATION(s1)) > 0.0) {
		  fd_tmp = s1->file;
		  s1->file = stmp->file;
		  skip_tmp = s1->bytes_skip;
		  s1->bytes_skip = stmp->bytes_skip;
		  /* keep s1->header->strm consistent with s1->file */
		  hdr_tmp = s1->header;
		  s1->header = stmp->header;
		  if(debug_level)
		    fprintf(stderr,"w1:rtime:%f rinc:%f st:%f t1:%f et:%f\n",
			  rtime,rinc,s1->start_time,s1->x_dat[0],s1->end_time);
		  if(!s1->utils->write_data(s1,rtime,rinc)) {
		    close_sig_file(s1);
		    s1->file = fd_tmp;
		    s1->bytes_skip = skip_tmp;
		    /* keep s1->header->strm consistent with s1->file */
		    s1->header = hdr_tmp;
		    printf("write_data() error1 in tag_insert_signal()\n");
		    free_signal(stmp);
		    return(NULL);
		  }
		  s1->file = fd_tmp;
		  s1->bytes_skip = skip_tmp;
		  /* keep s1->header->strm consistent with s1->file */
		  s1->header = hdr_tmp;
		} else
		  break;
	      } else {
		printf("read_data() problems1 in tag_insert_signal()\n");
		free_signal(stmp);
		return(NULL);
	      }
	    }
	  }	/* Finished transferring initial part of sink file. */
	close_sig_file(s1);
	
	/* Now transfer all of the source file. */
	stmp->start_samp = s1->start_samp + bs;	/* = tot of s1 sams written */
	for(wtime=stmp->start_time + (time - s1->start_time),
	    rtime=s2->start_time, rinc=10.0;
	    rtime+half_samp < SIG_END_TIME(s2); rtime += rinc, wtime += rinc) {
	  if(s2->utils->read_data(s2,rtime,rinc)) {
	    if((rinc = BUF_DURATION(s2)) > 0.0) {
	      if(s2->x_dat) {	/* Offset the tags correctly */
		register double correct = wtime - s2->start_time, *dp = s2->x_dat,
		*tp = (double*)malloc(sizeof(double)*s2->buff_size);
		int i;
		for(i=0, stmp->x_dat = tp; i < s2->buff_size; i++)
		  *tp++ = *dp++ + correct;
	      }
	      stmp->data = s2->data;
	      stmp->buff_size = s2->buff_size;
	      if(debug_level)
		fprintf(stderr,"w2:wtime:%f rinc:%f st:%f t1:%f et:%f\n",
		      wtime,rinc,stmp->start_time,stmp->x_dat[0],stmp->end_time);
	      if(!stmp->utils->write_data(stmp,wtime,rinc)) {
		printf("write_data() error2 in tag_insert_signal()\n");
		stmp->data = NULL;
		free_signal(stmp);
		return(NULL);
	      }
	      if(stmp->x_dat) {
		free(stmp->x_dat);
		stmp->x_dat = NULL;
	      }
	      stmp->start_samp += stmp->buff_size;
	    } else
	      break;
	  } else {
	    printf("read_data() problems2 in tag_insert_signal()\n");
	    stmp->data = NULL;
	    free_signal(stmp);
	    return(NULL);
	  }
	}
	close_sig_file(s2);
	/* Finished transferring source file. */

	/* Finally, transfer the remainder of the sink file. */
	for(rtime=time, rinc=10.0;
	    rtime+half_samp < SIG_END_TIME(s1);
	    rtime += rinc, wtime += rinc) {
	  if(s1->utils->read_data(s1,rtime,rinc)) {
	    if((rinc = BUF_DURATION(s1)) > 0.0) {
	      stmp->data = s1->data;
	      stmp->buff_size = s1->buff_size;
	      if(s1->x_dat) {	/*Offset the tags correctly*/
		register double correct = wtime - s1->x_dat[0], *dp = s1->x_dat,
		*tp = (double*)malloc(sizeof(double)*s1->buff_size);
		int i;
		for(i=0, stmp->x_dat = tp; i < s1->buff_size; i++)
		  *tp++ = *dp++ + correct;
	      }
	      if(debug_level)
		fprintf(stderr,"w3:wtime:%f rinc:%f st:%f t1:%f et:%f\n",
		      wtime,rinc,stmp->start_time,stmp->x_dat[0],stmp->end_time);
	      if(!stmp->utils->write_data(stmp,wtime,rinc)) {
		printf("write_data() error3 in tag_insert_signal()\n");
		stmp->data = NULL;
		free_signal(stmp);
		return(NULL);
	      }
	      if(stmp->x_dat) {
		free(stmp->x_dat);
		stmp->x_dat = NULL;
	      }
	      stmp->start_samp += stmp->buff_size;
	    } else
	      break;
	  } else {
	    printf("read_data() problems3 in tag_insert_signal()\n");
	    stmp->data = NULL;
	    free_signal(stmp);
	    return(NULL);
	  }
	}
	stmp->buff_size = 0;
	stmp->data = NULL;
	stmp->start_samp = 0;
	close_sig_file(s1);
	close_sig_file(stmp);
	return(stmp);
      } else
	printf("output_header() failure in tag_insert_signal()\n");
    } else
      show_notice(0,"Can't create a new signal in tag_insert_signal");
  } else {
    sprintf(notice_msg,
    "Incompatible signals (%s %s) or bad insert time (%f) in tag_insert_signal",
	   s1->name,s2->name,time);
    show_notice(1,notice_msg);
  }
  return(NULL);
}

