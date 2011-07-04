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
 * a collection of routines for dealing with SIG_F0 signals
 */

static char *sccs_id = "@(#)xf0_methods.c	1.10	1/18/97	ATT/ESI/ERL";

#include <Objects.h>

#include <xview/font.h>

#define scoff(x) ((int)(yoffs + ((double)(x) * scale)))
#define ALMOST 0.95

extern double  f0_canvas_use, f0_range, f0_min;
extern char    f0_plot_specs[];
extern Xv_Font def_font;
extern int     def_font_height, def_font_width;

extern char    *savestring();

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
print_f0_y(v)
  register View	*v;
{
  register int	i, elem, dim, index, slen, w;
  register List	*l, *l2;
  int		itsf0;
  double	time, scale, yoffs, t;
  char		tmp[100];
  Pixwin	*pw;
  Signal	*sig;


  if(v && (sig = v->sig)) {

    time = v->cursor_time;
    if (time < v->start_time) time = v->start_time;
    if (time > (t = ET(v)))   time = t;

    if((index = time_to_index(sig, time)) > (sig->buff_size - 1)
       || index < 0
       || (v->dims > 0 && !sig->data))
      return(FALSE);

    w = def_font_width;

    pw = canvas_pixwin(v->canvas);

    for(dim=0; dim < v->dims; dim++) { /* for each dimension... */
      if(v->show_vals[dim]) {
	double offs, scl;

	elem = v->elements[dim];

	itsf0 = FALSE;
	if((v->show_labels[dim]) && (l = sig->idents)) {
	  /* find the element's ident. string */
	  for(i = 0, l2 = l; i < elem; i++)
	    if(l2->next)
	      l2 = l2->next;
	  slen = strlen(l2->str); /* offset at which to print numeric value */
	  if(!strcmp(l2->str,"F0"))
	    itsf0 = TRUE;
	} else slen = 0;
	scale = - PIX_PER_CM/v->y_scale[dim];
	yoffs = .5 + (double)v->y_offset[dim];
	offs = v->val_offset[elem];
	scl = v->val_scale[elem];

	switch(sig_type_hack(sig)) {
	case P_CHARS:
	case P_UCHARS:
	  {
	    char *dp = ((char **) sig->data)[elem];

	    if (!dp)
	      return FALSE;
	    sprintf(tmp, "%4.0f", offs + (scl * dp[index]));
	  } break;
	case P_SHORTS:
	case P_USHORTS:
	  {
	    short *dp = ((short **) sig->data)[elem];

	    if (!dp)
	      return FALSE;
	    sprintf(tmp, "%6.0f", offs + (scl * dp[index]));
	  } break;
	case P_INTS:
	case P_UINTS:
	  {
	    int *dp = ((int **) sig->data)[elem];

	    if (!dp)
	      return FALSE;
	    sprintf(tmp, "%12.0f", offs + (scl * dp[index]));
	  } break;
	case P_FLOATS:
	  {
	    float *dp = ((float **) sig->data)[elem];

	    if (!dp)
	      return FALSE;
	    if(! itsf0)
	      sprintf(tmp, "%13.6e", offs + (scl * dp[index]));
	    else
	      sprintf(tmp, "%6.1f", offs + (scl * dp[index]));
	  } break;
	case P_DOUBLES:
	  {
	    double *dp = ((double **) sig->data)[elem];

	    if (!dp)
	      return FALSE;
	    if(! itsf0)
	      sprintf(tmp, "%13.6e", offs + (scl * dp[index]));
	    else
	      sprintf(tmp, "%6.1f", offs + (scl * dp[index]));
	  } break;
	default:
	  return(FALSE);
	}

	if(itsf0)
	  pw_text(pw, *(v->x_offset) + (slen*w), scoff(v->plot_min[dim]),
		  PIX_SRC|PIX_COLOR(CURSOR_COLOR), def_font, tmp);
	else
	  pw_text(pw, (slen*w), scoff(v->plot_min[dim]),
		  PIX_SRC|PIX_COLOR(TEXT_COLOR), def_font, tmp);
      }
    }
  }
  return(TRUE);
}

/*********************************************************************/
homogeneous(s)
     Signal *s;
{
  if(s && IS_GENERIC(s->type)) {
    if((s->type & VECTOR_SIGNALS) != P_MIXED)
      return(TRUE);
    if(s->types) {
      int i, typ;
      for(i=1, typ = *s->types; i < s->dim; i++)
	if(s->types[i] != typ) return(FALSE);
      return(TRUE);
    }
  }
  return(FALSE);
}

/*********************************************************************/
View *
new_f0_view(s, c)
     Signal *s;
     Canvas c;
{
  View *v, *setup_generic_view();
  int scale_f0_for_canvas(), plot_f0_waves();
  extern double ref_start, ref_size;

  if(!homogeneous(s))
    return(NULL);     /* F0 plotter only knows homogeneous for now... */
  if((v = setup_generic_view(s,c))) {
    int f0chan;
    
    v->data_plot = plot_f0_waves;
    v->y_print = print_f0_y;
    v->start_yval = f0_min;
    v->end_yval = f0_min + f0_range;
    v->find_crossing = FALSE;
    if((f0chan = get_labeled_chan(s,"F0")) >= 0) {
      v->plot_min[f0chan] = f0_min;
      v->plot_max[f0chan] = f0_min + f0_range;
    }
    v->set_scale = scale_f0_for_canvas;
    v->set_scale(v);
    return(v);
  }
  return(NULL);
}

/********************************************************************/
/* This ASSUMES that the signal is homogeneous and that the "P_MIXED"
designation is bogus in the sense that all signal elements are
actually the same atomic type. */
sig_type_hack(sig)
     Signal *sig;
{
  int i;
  if((i = (sig->type & VECTOR_SIGNALS)) == P_MIXED)
    i = sig->types[0];
  return(i);
}

/***************************************************************************/
/*Allocate f0_canvas_use of the available canvas height to the F0 signal.
  The remainder of the space gets divided evenly among the other channels. */
int 
scale_f0_for_canvas(v)
    View *v;
{
  int i, f0chan;
  int	      comb_height;
  double maxval, minval, dtm;
  double  scale, size, off, f0size, height;
  Signal *s;
  Canvas c;
  Bound *b;
  struct rect *r;
  Reticle *f0ret, *prepare_f0_reticle();
  extern int scrollbar_height, readout_bar_height;
  
  if(v && (f0ret = prepare_f0_reticle(v))) {
    /* note side-effects of prepare_f0_reticle() on v!! */
    if(v->scrollbar)
      comb_height = v->scrollbar->height + v->readout_height;
    else
      comb_height = scrollbar_height + readout_bar_height;
    c = v->canvas;
    s = v->sig;
    r = (struct rect*)xv_get(c, WIN_RECT);
    b = reticle_get_margins(f0ret);
    if((f0chan = get_active_labeled_chan(v,"F0")) < 0) f0chan = 0;
    if(v->dims > 1)
      size = (1.0 - f0_canvas_use) *
      ((double)(r->r_height - comb_height))/(v->dims - 1);
    f0size = (f0_canvas_use * (r->r_height - comb_height));
    height = f0size - (b->top + b->bottom);
    for(i=0, off=r->r_height; i < v->dims; i++) {
      if(v->elements[i] != f0chan) {
	if(v->ret[i]) {		/* if these were set by generic view creation */
	  free_reticle(v->ret[i]);
	  v->ret[i] = NULL;
	}
	if(v->v_rescale[i]) {
	  maxval = s->smax[v->elements[i]];
	  minval = s->smin[v->elements[i]];
	} else {
	  maxval = v->plot_max[i];
	  minval = v->plot_min[i];
	}

	if((dtm = maxval - minval) == 0.0) dtm = 1.0;
	scale = - ALMOST * size/dtm;
	v->y_offset[i] = off - (size/2) - (scale * (maxval + minval) / 2.0);
	off -= size;
      } else {
	double f0min = v->plot_min[i],
	       f0range = v->plot_max[i] - v->plot_min[i];
	v->colors[i] = WAVE2_COLOR;
	v->sig->smin[v->elements[i]] = f0min; /* so print_y will work right */
	scale = - height/f0range;
	v->y_offset[i] = off - b->bottom - (height/2) -
	  (scale * (f0min + f0min + f0range) / 2.0);
	/* let reticle know the bounds of actual data */
	f0ret->bounds.top = off - f0size + b->top;
	f0ret->bounds.bottom = f0ret->bounds.top + height;
	off -= f0size;
      }
      if(fabs(scale) > 0.0)
	v->y_scale[i] = - PIX_PER_CM/scale;
      else
	v->y_scale[i] = 1.0e6;
    }
    return(TRUE);		/* in case anyone cares */
  }
  return(FALSE);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Multi-dimensional waveform plotting using a connect-the-dots
scheme.  This routine is tuned to plot signals with one element
labeled "F0" and another labeled "P(voice)" (but both labels are
optional).  The F0 component (or element 0, if no "F0" label is
present) is plotted (with a reticle) so as to use f0_canvas_use of the
vertical plotting area for F0 values starting at f0_min and extending
for f0_range Hz.  The rest of the plotting area is apportioned equally
among all other vector elements.  If an element labeled "P(voice)" is
present, it will be used to gate the plotting of F0 on and off
(threshold 0.5) so as to avoid meaningless F0 values in unvoiced
regions.  If P(voiced) is not labeled, element 0 will be used for the
gating signal, thus a single-channel F0 signal will be plotted
correctly if the values for F0 are set to 0 in unvoiced regions. */

plot_f0_waves(view)
register View *view;
{
  register int nsam, i, j, k, npix;
  int val, dim, f0chan, pvchan;
  Pixwin    *pw;
  Signal    *sig;
  Canvas    canvas;
  double end, start, fxoff, dtm;
  register double scale, incr, x, yoffs, sump;
  char f0c[10];
  static int  dx=4, dy=5, type=0;

  static int  b_siz = 5;

  struct   rect *rect;
  List *l, *l2;

  if(view && (sig = view->sig) && sig_type_hack(sig)) {
    canvas = view->canvas;
    pw = canvas_pixwin(canvas);	/* get the pixwin to write to */

/*!*//* pw_ttext doesn't work right under OpenWindows 1.0.1; */
     /* ignore plot spec, except for type, and use a little box */
     /* instead of the plot char. */

    if(*f0_plot_specs) { 
      strcpy(f0c,"*"); 
      sscanf(f0_plot_specs,"%d%s%d%d",&type,f0c,&dx,&dy); 
    } 
    dx = 2; dy = -2;

    rect = (struct rect*)xv_get(canvas, WIN_RECT);
    if((f0chan = get_active_labeled_chan(view,"F0")) < 0) f0chan = 0;
    if(((pvchan = get_labeled_chan(sig,"P(voice)")) < 0) &&
       ((pvchan = get_labeled_chan(sig,"prob_voice")) < 0))
      pvchan = 0;
    
    /* Determine the number of waveform samples to plot */
    if(view->start_time < (end = BUF_END_TIME(sig))) {
      nsam = (end - view->start_time) * sig->freq;
    } else {
      return(FALSE);
    }
  
    /* get x-displacement per waveform sample */
    incr = (PIX_PER_CM / *(view->x_scale))/sig->freq;

    /*compute the number of  pixels in the x-direction */
    npix = incr * nsam;

    /* clip to size of canvas */
    npix = (npix > (rect->r_width - *(view->x_offset))) ?
           rect->r_width - *(view->x_offset) : npix;
    nsam = .5 + (((double)npix)/incr);

    /* clear window if data is NOT plotted as an overlay */
    if(!(view->extra_type & VIEW_OVERLAY))
      pw_write(pw,0,0,rect->r_width,rect->r_height,
	     PIX_COLOR(BACKGROUND_COLOR)|PIX_SRC,NULL,0,0);

    /* deternine starting sample in the waveform */
    i = time_to_index(sig,view->start_time);
    /* determine x offset due to initial sample time re view start */
    fxoff = 0.5 + (PIX_PER_CM * (BUF_START_TIME(sig) -
		 view->start_time + ((double)i)/sig->freq) / *(view->x_scale));

#define PLOT_TYPE(the_type) { \
  register the_type *q, *r, *p, **dpp, imax, imin, *pv; \
  dpp = (the_type**)sig->data; \
  if (!dpp) return FALSE; \
  if(pvchan >= 0) pv = dpp[pvchan]; \
  else \
    pv = dpp[view->elements[f0chan]]; \
  if (!pv) \
    return FALSE; \
  else \
    pv += i; \
  for(dim=0; dim < view->dims; dim++) { /* for each dimension... */ \
    val = view->colors[dim]; \
    scale = - PIX_PER_CM/view->y_scale[dim]; \
    yoffs = .5 + (double)view->y_offset[dim]; \
    if(dim == f0chan){ \
      int washit; \
      yoffs += (type? dy : 0); \
      if(incr < 1.0){	/* must do max-min computation? */ \
	p = dpp[view->elements[dim]]; \
        if (!p) return FALSE; \
	else    p += i; \
	for(k = 0, j = *(view->x_offset)+(type? -dx : 0), sump=0.0; \
	    k < npix; \
	    j++, k++, sump -= 1.0) { \
	      for(washit=0, imax = imin = *p; sump < 1.0; \
		  sump += incr ) { \
		    if((*pv++ > 0.5) && (*pv > 0.5)) { \
		      washit = 1; \
		      if(*++p > imax)imax = *p; \
		      else \
			if(*p < imin)imin = *p; \
		    } else \
		      p++; \
		  } \
	      if(washit) { \
		if(type) { \
		/* pw_ttext(pw,j,scoff(imax),
			    PIX_SRC|PIX_COLOR(val),def_font,f0c); */\
		pw_write(pw, j, scoff(imax), b_siz, b_siz, \
			 PIX_SRC|PIX_COLOR(val), NULL, 0, 0); \
		/* pw_ttext(pw,j,scoff(imin),
			    PIX_SRC|PIX_COLOR(val),def_font,f0c); */\
		pw_write(pw, j, scoff(imin), b_siz, b_siz, \
			 PIX_SRC|PIX_COLOR(val), NULL, 0, 0); \
		} else \
		  { \
		     long y1 = scoff(imin); \
		     long y2 = scoff(imax); \
                     if (y2 != y1) \
		        pw_vector(pw, j, y2, j, y1, \
				   PIX_COLOR(val)|PIX_SRC, val); \
		     else \
		        pw_vector(pw, j, y2, j+1, y1, \
				   PIX_COLOR(val)|PIX_SRC, val); \
	           } \
	      } \
	    } \
      } else {		/* no need for max-min */ \
	p = dpp[view->elements[dim]]; \
	if (!p) return FALSE; \
	else    p += i; \
	for(k = *(view->x_offset) + (type? -dx : 0), \
	    x = fxoff + k + incr, \
	    r = p + 1,  q = p + nsam - 1; p < q; x += incr, p++, r++) { \
	      j = x; \
	      if(*pv++ > 0.5) { \
		if(type) \
		  /* pw_ttext(pw,k,scoff(*p),PIX_SRC|PIX_COLOR(val),def_font,f0c); */\
		  pw_write(pw, k, scoff(*p), b_siz, b_siz, \
			   PIX_SRC|PIX_COLOR(val), NULL, 0, 0); \
		else \
		  if(*pv > 0.5) \
		    pw_vector(pw, k, scoff(*p), j, scoff(*r), \
			      PIX_COLOR(val)|PIX_SRC, val); \
	      } \
	      k = j; \
	    } \
      } \
    } else {		/* it is a "regular" channel */ \
      if(incr < 1.0){	/* must do max-min computation? */ \
	p = dpp[view->elements[dim]]; \
	if (!p) return FALSE; \
	else    p += i; \
	for(k=0, j = *(view->x_offset), sump=0.0; \
	    k < npix; \
	    j++, k++, sump -= 1.0) { \
	      for(imax = imin = *p; sump < 1.0; sump += incr ) { \
		if(*++p > imax)imax = *p; \
		else \
		  if(*p < imin)imin = *p; \
	      } \
		  { \
		     long y1 = scoff(imin); \
		     long y2 = scoff(imax); \
                     if (y2 != y1) \
		        pw_vector(pw, j, y2, j, y1, \
				   PIX_COLOR(val)|PIX_SRC, val); \
		     else \
		        pw_vector(pw, j, y2, j+1, y1, \
				   PIX_COLOR(val)|PIX_SRC, val); \
	           } \
	    } \
      } else {		/* no need for max-min */ \
	p = dpp[view->elements[dim]]; \
	if (!p) return FALSE; \
	else    p += i; \
	for(k = *(view->x_offset), x = fxoff + *(view->x_offset) + incr, \
	    r = p + 1,  q = p + nsam - 1; p < q; x += incr) { \
	      j = x; \
	      pw_vector(pw, k, scoff(*p++), j, scoff(*r++), \
			PIX_COLOR(val)|PIX_SRC, val); \
	      k = j; \
	    } \
      } \
    } \
  } \
}				/* end of #define PLOT_TYPE() */

    
    switch(sig_type_hack(view->sig)) {
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
      printf("Unknown data type in f0_plot_waves(%d)\n",
	     sig_type_hack(view->sig));
      return(FALSE);
    }
#undef PLOT_TYPE

    if(l = view->sig->idents) {
      int curi, i;
      
      for(dim=0; dim < view->dims; dim++) { /* for each dimension... */
	if(view->show_labels[dim]) {
	  curi = view->elements[dim];
	  for(i = 0, l2 = l; i < curi; i++)
	    if(l2->next)
	      l2 = l2->next;

	  scale = - PIX_PER_CM/view->y_scale[dim];
	  yoffs = .5 + (double)view->y_offset[dim];	  
	  if(dim == f0chan)
	    pw_text(pw, *(view->x_offset), scoff(view->plot_min[dim]),
		    PIX_SRC|PIX_COLOR(CURSOR_COLOR), def_font, l2->str);
	  else
	    pw_text(pw, 0, scoff(view->plot_min[dim]),
		    PIX_SRC|PIX_COLOR(TEXT_COLOR), def_font, l2->str);
	}
      }
    }
    return(TRUE);
  }
  return(FALSE);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
Reticle *
prepare_f0_reticle(v)
     View *v;
{
  Bound *b, *b2;
  int width, f0chan, i;
  double hz_per_pix, sec_per_pix, lospac, hispac, range, range10,
  range5, range2;
  int right_of_d;
  char abform[20];
  Reticle *ret;
  Signal *s;
  struct rect *rect;

  if(v && v->canvas) {
    rect = (struct rect*)xv_get(v->canvas, WIN_RECT);
    if((f0chan = get_active_labeled_chan(v,"F0")) < 0) f0chan = 0;
    if(!(ret = v->ret[f0chan])) {
      /* Create a reticle (time/amplitude scales), if necessary. */
      if(!(v->ret[f0chan] = ret = new_wave_reticle())) {
	printf("Can't create a new reticle structure\n");
	return(NULL);
      }
    }

    if((s = v->sig)) {
      ret->ordi.end = v->plot_max[f0chan];
      ret->ordi.start = v->plot_min[f0chan];
      ret->absc.start = v->start_time;
      ret->absc.end = ET(v);
      ret->color = RETICLE_COLOR;
      ret->abs_label = NULL;
      ret->ordinate.maj.style = EDGES|MAJOR ;
      ret->ordinate.maj.inter = 100.0;
      ret->ordinate.min1.style =  EDGES|MAJOR;
      ret->ordinate.min1.inter = 50.0;
      ret->ordinate.min2.style = EDGES ;
      ret->ordinate.min2.inter =  10.0;
      ret->ordinate.num_inter = 50.0;
      /* Want at least 70 pixels between time numbering and no more than 140. */
      /* Assume one pixel per time step. */
      sec_per_pix = *(v->x_scale)/PIX_PER_CM;
      lospac = 70.0*sec_per_pix;
      hispac = 140.0*sec_per_pix;
      range = pow(10.0, floor(log10(lospac)));
      range10 = 10.0*range;
      range5 = 5.0*range;
      range2 = 2.0*range;
      if(hispac >= range10)
	ret->abscissa.maj.inter = range10;
      else
	if((lospac <= range5) && (hispac >= range5))
	  ret->abscissa.maj.inter = range5;
	else
	  ret->abscissa.maj.inter = range2;

      if( ret->abscissa.maj.inter <= 1.0)
	right_of_d = 1 + (int)(0.5 - log10( ret->abscissa.maj.inter ));
      else
	right_of_d = 0;
      sprintf(abform,"%s%df","%8.",right_of_d);
      reticle_set_absc_precision(ret, abform);
      ret->abscissa.maj.style =  EDGES;
      ret->abscissa.maj.length = ret->ordinate.maj.inter/10.0;
      ret->abscissa.min1.style =  EDGES;
      ret->abscissa.min1.length =  ret->ordinate.maj.inter/20.0;
      ret->abscissa.min1.inter = ret->abscissa.maj.inter/2.0;
      ret->abscissa.min2.style = EDGES;
      ret->abscissa.min2.length =  ret->ordinate.maj.inter/40.0;
      ret->abscissa.min2.inter = ret->abscissa.maj.inter/10.0;
      ret->abscissa.num_inter = ret->abscissa.maj.inter;
      ret->ordinate.maj.length =  ret->abscissa.maj.inter/3.0;
      ret->ordinate.min1.length = ret->abscissa.maj.inter/5.0;
      ret->ordinate.min2.length = ret->abscissa.maj.inter/10.0;
      ret->abscissa.num_loc = NUM_LB;
  
      /* get margins required by reticle, numerals, etc. */
      b = reticle_get_margins(ret);
      /* ret->bounds.top and ret->bounds.bottom are set up in
	 scale_f0_for_canvas() */
      ret->bounds.right = rect->r_width; /* this is cheating (bit of a kluge) */
      ret->bounds.left = b->left;
      /* Align the left margins of all SIG_F0 elements with F0. */
      *(v->x_offset) = b->left;
      return(ret);
    }
  }
  return(NULL);
}
