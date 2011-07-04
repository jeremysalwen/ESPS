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
 *
 * plot_data.c
 * display methods for all supported data types
 */

static char *sccs_id = "@(#)xplot_data.c	1.16 9/5/97 ERL";

#include <sys/file.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feaspec.h>
#include <Objects.h>
#include <xview/font.h>

extern int	generic_ord_to_y();
extern double	generic_y_to_yval();

extern int debug_level;
extern Xv_Font def_font;
extern int     def_font_height, def_font_width;

extern int doing_print_graphic, print_only_plot;

#define scoff(x) ((int)(yoffs + ((double)(x) * scale)))
#define ALMOST 0.95
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
double 
fmod(num,den)
     register double num, den;
{
  register int q;

  q = num/den;
  return( num - ((double)q)*den);
}

/*********************************************************************/
int 
plot_reticles(v)
     View *v;
{
  int i;

  if (!v || !(v->canvas)) return(FALSE);
  for (i = 0; i < v->dims; ++i)
    if (v->reticle_on[i] && v->ret[i])
      draw_reticle(v->canvas,v->ret[i]);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
print_y(v)
     register View *v;
{
  register int	i, elem, dim, index, slen, w;
  register List	*l, *l2;
  double	time, scale, yoffs;
  char		tmp[100];
  Pixwin	*pw;
  Signal	*sig;
  extern 	View *focus_view;

  if (doing_print_graphic && print_only_plot)
    	return(TRUE);

  if (v && (sig = v->sig)) {

    if (!IS_GENERIC(sig->type)) return(FALSE);

    time = v->cursor_time;
    if (time < v->start_time) time = v->start_time;
    if (time > (scale = ET(v)))   time = scale;

    if((index=time_to_index(sig, time)) > sig->buff_size - 1
       || index < 0
       || (v->dims > 0 && !sig->data))
      return(FALSE);

    w = def_font_width;

    pw = canvas_pixwin(v->canvas);

    for(dim=0; dim < v->dims; dim++) { /* for each dimension... */
      if(v->show_vals[dim]) {
	double offs, scl;

	elem = v->elements[dim];

	if(v->show_labels[dim] && (l = sig->idents)) {
	  /* find the element's ident. string */
	  for(i = 0, l2 = l; i < elem; i++)
	    if(l2->next)
	      l2 = l2->next;
	  slen = strlen(l2->str); /* offset at which to print numeric value */
	} else slen = 0;
	offs = v->val_offset[elem];
	scl = v->val_scale[elem];

	switch (sig->type & VECTOR_SIGNALS) {
	case P_CHARS:
	case P_UCHARS:
	  {
	    char *s_data = ((char **) sig->data)[elem];

	    if (!s_data)
	      return FALSE;
	    sprintf(tmp,"%4.0f",offs + (scl * s_data[index]));
	  } break;
	case P_SHORTS:
	case P_USHORTS:
	  {
	    short *s_data = ((short **) sig->data)[elem];

	    if (!s_data)
	      return FALSE;
	    sprintf(tmp,"%6.0f",offs + (scl * s_data[index]));
	  } break;
	case P_INTS:
	case P_UINTS:
	  {
	    int *s_data = ((int **) sig->data)[elem];

	    if (!s_data)
	      return FALSE;
	    sprintf(tmp,"%12.0f",offs + (scl * s_data[index]));
	  } break;
	case P_FLOATS:
	  {
	    float *s_data = ((float **) sig->data)[elem];

	    if (!s_data)
	      return FALSE;
	    sprintf(tmp,"%13.6e",offs + (scl * s_data[index]));
	  } break;
	case P_DOUBLES:
	  {
	    double *s_data = ((double **) sig->data)[elem];

	    if (!s_data)
	      return FALSE;
	    sprintf(tmp,"%13.6e",offs + (scl * s_data[index]));
	  } break;
	case P_MIXED:
	  {
	    caddr_t s_data = ((caddr_t *) sig->data)[elem];

	    if (!s_data)
	      return FALSE;

	    switch (sig->types[elem])
	    {
	    case P_CHARS:
	    case P_UCHARS:
	      sprintf(tmp,"%4.0f",offs + (scl * ((char *) s_data)[index]));
	      break;
	    case P_SHORTS:
	    case P_USHORTS:
	      sprintf(tmp,"%6.0f",offs + (scl * ((short *) s_data)[index]));
	      break;
	    case P_INTS:
	    case P_UINTS:
	      sprintf(tmp,"%12.0f",offs + (scl * ((int *) s_data)[index]));
	      break;
	    case P_FLOATS:
	      sprintf(tmp,"%13.6e",offs + (scl * ((float *) s_data)[index]));
	      break;
	    case P_DOUBLES:
	      sprintf(tmp,"%13.6e",offs + (scl * ((double *) s_data)[index]));
	      break;
	    default:
	      return(FALSE);
	    }
	  } break;
	default:
	  return(FALSE);
	}

	/*      sigmin = v->sig->smin[v->elements[dim]];
		
		if (plot_min != 0)
		sigmin = plot_min;
		
		if (v->sig->smax[v->elements[dim]] - sigmin == 0.0)
		sigmin -= 0.5; */ /* Cf. line in scale_for_canvas setting
		* dtm = 1.0 when maxval - minval == 0.0.
		* The adjustment here is to keep the baseline
		* of the at the bottom of the allotted strip,
		* rather than the center, when the data are
		* constant
		*/


	scale = - PIX_PER_CM/v->y_scale[dim];
	yoffs = .5 + (double)v->y_offset[dim];
        if (v->show_current_chan && focus_view == v && dim == v->cursor_channel)
	   pw_text(pw, *(v->x_offset) + (slen*w), scoff(v->plot_min[dim]),
		PIX_SRC|PIX_COLOR(YA2_COLOR), def_font, tmp);
        else
	   pw_text(pw, *(v->x_offset) + (slen*w), scoff(v->plot_min[dim]),
		PIX_SRC|PIX_COLOR(TEXT_COLOR), def_font, tmp);
      }
    }
  }
  return(TRUE);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
file_print_x(v,x)
     register View *v;
     int x;
{
  Xv_Window	pw;
  Rect	*rec;
  char	mes[50];
  double	time, sig_dur, fract;
  int		w_msg, w_win;
  
  if (doing_print_graphic && print_only_plot)
    	return(TRUE);

  if (!v || !v->canvas || !v->sig) return FALSE;

  x -= *(v->x_offset);
  if(x < 0) x = 0;
  fract = ((double)x)/(v->width - *(v->x_offset));
  sig_dur = SIG_DURATION(v->sig);
  time = v->sig->start_time + (sig_dur * fract);
  pw = canvas_paint_window(v->canvas);
  rec = (Rect *) xv_get(pw, WIN_RECT);
  w_win = rec->r_width;

  sprintf(mes, "Time(f):%9.5fsec", time);
  w_msg = strlen(mes)*def_font_width;

  x = (w_win/3 >= w_msg) ? w_win/3 - w_msg
    : 0;			/* Move time msg left in small windows to
				   postpone overlap with D: L: R: F: msg
				   (see plot_markers in xcursors.c). */
  pw_text(pw, x, 14,
	  PIX_SRC|PIX_COLOR(FOREGROUND_COLOR), def_font, mes);
    
  return(TRUE);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
print_x(v)
    register View *v;
{
    Xv_Window	pw;
    Rect	*rec;
    char	mes[50];
    double	time, t;
    int		w_msg, w_win, x;
  
    if (!v || !v->canvas) return FALSE;
    if (doing_print_graphic && print_only_plot)
    	return(TRUE);

    time = v->cursor_time;
    if (time < v->start_time) time = v->start_time;
    if (time > (t = ET(v)))   time = t;

    pw = canvas_paint_window(v->canvas);
    rec = (Rect *) xv_get(pw, WIN_RECT);
    w_win = rec->r_width;

    sprintf(mes, "   Time:%9.5fsec", time);
    w_msg = strlen(mes)*def_font_width;

    x = (w_win/3 >= w_msg) ? w_win/3 - w_msg
	: 0;			/* Move time msg left in small windows to
				   postpone overlap with D: L: R: F: msg
				   (see plot_markers in xcursors.c). */
    pw_text(pw, x, 14,
	    PIX_SRC|PIX_COLOR(FOREGROUND_COLOR), def_font, mes);
    
    return(TRUE);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Periodic vector and scalar signals are plotted using a connect-the-dots
   algorithm.  Arbitrary x-axis scaling, y_offsets and y-axis scaling are
   supported. */
plot_waves(view)
  register View *view;
{
  register int y, y2, j, k, npix;
  int index, nsam, maxdim;
  int val, dim, its_an_overlay = FALSE;
  register View *vh;
  Pixwin    *pw;
  Signal    *sig;
  Canvas    canvas;
  double    end, fxoff;
  register double scale, incr, x, yoffs, sump;
/*  double	sigmin; */
  struct rect	*rect;
  extern int	show_labels;
  List *l, *l2;

  if(view && (sig = view->sig) && sig->type) {

    if (IS_GENERIC(sig->type))
    {

      /* Is this an overlaid signal? */
      if(!((view->extra_type & VIEW_OVERLAY) && (vh = (View*)view->extra)))
	vh = view;
      else
	its_an_overlay = TRUE;

      canvas = vh->canvas;
      pw = canvas_pixwin(canvas); /* get the pixwin to write to */

      rect = (struct rect*)xv_get(canvas, WIN_RECT);
  
      /* Determine the number of waveform samples to plot */
      if(vh->start_time < (end = BUF_END_TIME(sig))) {
	if(vh->start_time >= BUF_START_TIME(sig))
	  nsam = (end - vh->start_time) * sig->freq;
	else
	  nsam = sig->buff_size;
      } else {
	return(FALSE);
      }
	
      /* get x-displacement per waveform sample */
      incr = (PIX_PER_CM / *(vh->x_scale))/sig->freq;

      /*compute the number of  pixels in the x-direction */
      npix = (incr * nsam);

      /* clip to size of canvas */
      npix = (npix > (rect->r_width - *(vh->x_offset))) ?
	rect->r_width - *(vh->x_offset) : npix;
      nsam = .5 + (((double)npix)/incr);

      /* Clear window if data is NOT plotted as an overlay. */
      if(!its_an_overlay && !doing_print_graphic)
	pw_write(pw,0,0,rect->r_width,rect->r_height,
		 PIX_COLOR(vh->background)|PIX_SRC,NULL,0,0);

      /* deternine starting sample in the waveform */
      index = time_to_index(sig,vh->start_time);
      /* determine x offset due to initial sample time re view start */
      fxoff = 0.5 + (PIX_PER_CM * (BUF_START_TIME(sig) -
	   vh->start_time + ((double)index)/sig->freq) / *(vh->x_scale));
  
#define PLOT_TYPE(the_type)	{ \
    register the_type *q, *r, *p, imax, imin; \
    int k1=0; \
     \
    if (!s_data) return FALSE; \
     \
    val = view->colors[dim]; \
     \
    scale = - PIX_PER_CM/vh->y_scale[dim]; \
     \
    yoffs = .5 + (double)vh->y_offset[dim]; /* precompute for speed */ \
     \
    switch(view->line_types[dim]) { \
    case 1:			/* standard solid line type */ \
    default: \
      if(incr < 1.0){	/* must do max-min computation? */ \
        register int y2; \
	for(k=0, j = *(vh->x_offset) + fxoff, sump=0.0, \
	    p = (the_type *) s_data + index ;    k < npix; \
	    j++, k++, sump -= 1.0) { \
	      for(imax = imin = *p; sump < 1.0; sump += incr ) { \
                if (++k1 == nsam) break; \
		if(*++p > imax)imax = *p; \
		else \
		  if(*p < imin)imin = *p;\
	      } \
              y = scoff(imax); \
	      y2 = scoff(imin); \
	      if(y2 != y) \
                 pw_vector(pw, j, y, j, y2, \
			PIX_COLOR(val)|PIX_SRC, val); \
              else \
                 pw_vector(pw, j, y, j+1, y2, \
			PIX_COLOR(val)|PIX_SRC, val); \
	    } \
      } else {		/* no need for max-min */ \
	for(k = *(vh->x_offset) + fxoff, x = fxoff + *(vh->x_offset) + incr, \
	    p = (the_type *) s_data + index, \
	    r = p + 1,  q = p + nsam - 1; p < q; x += incr) { \
	      j = x; \
	      pw_vector(pw, k, scoff(*p++), j, scoff(*r++), \
			PIX_COLOR(val)|PIX_SRC, val); \
	      k = j; \
	    } \
      } \
      break; \
    case 2:			/* histogram-style */ \
      if(incr < 1.0){	/* must do max-min computation? */ \
	for(k=0, j = *(vh->x_offset) + fxoff, sump=0.0, \
	    y = scoff(sig->smin[vh->elements[dim]]), \
	    p = (the_type *) s_data + index ;    k < npix; \
	    j++, k++, sump -= 1.0) { \
	      for(imax = *p; sump < 1.0; sump += incr ) {\
                if(++k1 == nsam) break; \
		if(*++p > imax)imax = *p; \
              } \
	      pw_vector(pw, j, scoff(imax), j, y, \
			PIX_COLOR(val)|PIX_SRC, val); \
	    } \
      } else {		/* no need for max-min */ \
	for(k = *(vh->x_offset) + fxoff, x = fxoff + *(vh->x_offset) + incr, \
	    y2 = scoff(sig->smin[vh->elements[dim]]), \
	    p = (the_type *) s_data + index, \
	    r = p + 1,  q = p + nsam; p < q; x += incr) { \
	      j = x; \
	      pw_vector(pw, k, (y = scoff(*p++)), j, scoff(*r++), \
			PIX_COLOR(val)|PIX_SRC, val); \
	      pw_vector(pw, k, y, k, y2, PIX_COLOR(val)|PIX_SRC, val); \
	      k = j; \
	    } \
      } \
      break; \
    case 3:			/* DSP-style */ \
    case 4:		/* DSP-style with connect-the-dots*/ \
      if(incr < 1.0){	/* must do max-min computation? */ \
	for(k=0, j = *(vh->x_offset) + fxoff, sump=0.0, \
	    y = scoff(0.0), \
	    p = (the_type *) s_data + index ;    k < npix; \
	    j++, k++, sump -= 1.0) { \
	      for(imax = imin = *p; sump < 1.0; sump += incr ) { \
                if (++k1 == nsam) break; \
		if(*++p > imax)imax = *p; \
		else \
		  if(*p < imin)imin = *p; \
	      } \
	      if(imax > 0.0) \
		pw_vector(pw, j, scoff(imax), j, y, \
			PIX_COLOR(val)|PIX_SRC, val); \
	      if(imin < 0.0) \
		pw_vector(pw, j, scoff(imin), j, y, \
			PIX_COLOR(val)|PIX_SRC, val); \
	    } \
      } else {		/* no need for max-min */ \
	for(k = *(vh->x_offset) + fxoff, x = fxoff + *(vh->x_offset) + incr, \
	    y2 = scoff(0.0), \
	    p = (the_type *) s_data + index, \
	    r = p + 1,  q = p + nsam; p < q; x += incr) { \
	      j = x; \
	      y = scoff(*p++); \
	      if(view->line_types[dim] == 4) \
		pw_vector(pw, k, y, j, scoff(*r++), \
			PIX_COLOR(val)|PIX_SRC, val); \
	      pw_vector(pw, k, y, k, y2, PIX_COLOR(val)|PIX_SRC, val); \
	      k = j; \
	    } \
      } \
      break; \
    } \
}
  /* >>>  END of PLOT_TYPE() definition  <<< */

      if (nsam > 0)
      {
	  maxdim = (vh->dims > view->dims) ? view->dims : vh->dims;

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
			printf("Unknown component data type in plot_waves(%x)\n",
			       sig->types[vh->elements[dim]]);
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
	printf("Unknown data type in plot_waves(%x)\n", sig->type);
	return(FALSE);
    }

    if ((!its_an_overlay) && (l = sig->idents)) {
      int curi, i;
      
      for(dim=0; dim < vh->dims; dim++) { /* for each dimension... */
	if(vh->show_labels[dim]) {
	  curi = view->elements[dim];
	  for(i = 0, l2 = l; i < curi; i++)
	    if(l2->next)
	      l2 = l2->next;

/*	  sigmin = sig->smin[curi];

	  if (plot_min != 0)
	    sigmin = plot_min;

	  if (sig->smax[curi] - sigmin == 0.0)
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
View *view_initial_values(v)
     View *v;
{
  extern int line_type, reticle_grid, show_labels, show_vals,
    v_spect_rescale, readout_bar_height, scrollbar_height,
    shorten_header, h_spect_rescale, invert_dither,
    overlay_as_number, redraw_on_release, rewrite_after_edit, find_crossing;
  extern double plot_max, plot_min, zoom_ratio, cross_level, ref_step;
  extern char spect_rescale_scope[];
  extern int show_current_chan;

  if(v) {
    int i;
    for(i =0; i < v->dims; i++) {
      v->reticle_on[i] = reticle_grid;
      v->show_labels[i] = show_labels;
      v->show_vals[i] = show_vals;
      v->line_types[i] = line_type;
      if(plot_max > plot_min) {
	v->plot_max[i] = plot_max;
	v->plot_min[i] = plot_min;
	v->v_rescale[i] = FALSE;
      }
    }
    v->readout_height = readout_bar_height;
    if(v->scrollbar)
      v->scrollbar->height = scrollbar_height;
    v->rescale_scope = scope_to_number(spect_rescale_scope);
    v->shorten_header = shorten_header;
    v->h_rescale = TRUE;
    v->invert_dither = invert_dither;
    v->overlay_as_number = overlay_as_number;
    v->redraw_on_release = redraw_on_release;
    v->find_crossing = find_crossing;
    v->rewrite_after_edit = rewrite_after_edit;
    v->zoom_ratio = zoom_ratio;
    v->cross_level = cross_level;
    v->page_step = ref_step;
    v->show_current_chan = show_current_chan;
    return(v);
  }
  return(NULL);
}

/*********************************************************************/
/* Create a view and initialize it to the current settings of the
   relevant xwaves globals. */
View *new_waves_view(s, c)
     Signal *s;
     Canvas c;
{
  View *v = new_view(s,c);
  return(view_initial_values(v));
}

/*********************************************************************/
View *
setup_generic_view(s, c)
     Signal *s;
     Canvas c;
{
  View *v;
  extern int line_type;
  extern plot_markers(), plot_hmarkers(), plot_cursors(),
         plot_spect_cursors();
  extern void operate_wave_scrollbar();
  int i, scale_for_canvas();
  Rect *rec;
  double start, size;
  extern double ref_start, ref_size;
  extern char mark_reference[], *savestring();
  extern void (*right_button_proc)();
  extern Menuop *aux_but_ops;
  extern Menuop *search_all_menus_but();
  extern char def_left_op[], def_middle_op[], def_move_op[];
  extern int line_type;

  if((v = new_waves_view(s,c))) {
    rec = (Rect*)xv_get(c,WIN_RECT);
    v->width = rec->r_width;
    v->height = rec->r_height;
    for(i=0; i <s->dim; i++) v->line_types[i] = line_type;
    v->data_plot = plot_waves;
    v->cursor_plot = plot_cursors;
    v->vmarker_plot = plot_markers;
    v->hmarker_plot = plot_hmarkers;
    v->reticle_plot = plot_reticles;
    v->x_print = print_x;
    v->y_print = print_y;
    v->set_scale = scale_for_canvas;
    v->time_to_x = generic_time_to_x;
    v->x_to_time = generic_x_to_time;
    v->yval_to_y = generic_ord_to_y;
    v->y_to_yval = generic_y_to_yval;
    v->xy_to_chan = generic_xy_to_chan;
    v->handle_scrollbar = operate_wave_scrollbar;
    v->left_but_proc = search_all_menus_but("spect", def_left_op);
    v->mid_but_proc =  search_all_menus_but("spect", def_middle_op);
    v->move_proc =  search_all_menus_but("spect", def_move_op);
    v->right_but_proc = right_button_proc;
    if(*mark_reference)
      v->mark_reference = savestring(mark_reference);
    else
      v->mark_reference = savestring("cursor_time");
    v->tmarker_chan = 0;
    v->bmarker_chan = 0;
    v->background = W_BACKGROUND_COLOR;
    start = ref_start;
    size = ref_size;
    get_view_segment(v,&start,&size);
    get_maxmin(s);
    for(i=0; i < v->dims; i++) {
      v->colors[i] = W_WAVEFORM_COLOR;
      if(v->v_rescale[i]) {
	v->plot_max[i] = s->smax[i];
	v->plot_min[i] = s->smin[i];
      }
    }
    v->bmarker_yval = *s->smin;
    v->tmarker_yval = *s->smax;
    *(v->x_scale) = PIX_PER_CM * size/(v->width - *(v->x_offset));
    v->set_scale(v);

    /* Now correct the x scale, since the reticle numbering on the left
       may have changed the available space. */
    *(v->x_scale) = PIX_PER_CM * size/(v->width - *(v->x_offset));
    
    return(v);
  }
  return(NULL);
}


/*********************************************************************/
View *
setup_view(s, c)
     Signal *s;
     Canvas c;
{
  View *tag_setup_view(), *new_f0_view();

  /* For the tagged abominations */
  if(IS_TAGGED_FEA(s))
    return(tag_setup_view(s,c));
  
  switch(s->type & SPECIAL_SIGNALS) {
  case SIG_SPECTROGRAM:
    return(new_spect_view(s,FIXED_SCALE,c));
  case SIG_F0:
    return(new_f0_view(s,c));
  default:
    if(IS_GENERIC(s->type) && (get_labeled_chan(s,"F0") >= 0))
      return(new_f0_view(s,c));
    else
      return(setup_generic_view(s,c));
    break;
  }
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
Reticle *
prepare_reticle(v)
     View *v;
{
  Bound *b, *b2;
  int width, chan = 0, i;
  double hz_per_pix, sec_per_pix, lospac, hispac, range, range10,
         range5, range2, size, maxbounds;
  int right_of_d, numbers = 0;
  char abform[200], *savestring();
  Reticle *ret;
  Signal *s;
  struct rect *rect;
  extern int readout_bar_height, scrollbar_height;

  if(v && v->canvas) {
    rect = (struct rect*)xv_get(v->canvas, WIN_RECT);
    if(v->scrollbar)
      size = (double)(rect->r_height - v->readout_height - v->scrollbar->height)/
	v->dims;
    else
      size = (double)(rect->r_height - readout_bar_height - scrollbar_height)/
	v->dims;
    for(numbers = 0, chan = 0, maxbounds = 0; chan < v->dims; chan++) {
      if(v->reticle_on[chan]) {
	if(!(ret = v->ret[chan])) {
	  /* Create a reticle (time/amplitude scales), if necessary. */
	  if(!(v->ret[chan] = ret = new_wave_reticle())) {
	    printf("Can't create a new reticle structure\n");
	    return(NULL);
	  }
	}
	if((s = v->sig)) {
	  double arange, val_per_pix;

	  ret->ordi.end = v->plot_max[chan];
	  ret->ordi.start = v->plot_min[chan];
	  ret->absc.start = v->start_time;

	  /* Want at least 30 pixels between amp. numbering and no more than 60. */
	  arange = v->plot_max[chan] - v->plot_min[chan];
	  if(arange == 0) arange = 1;
	  val_per_pix = arange/size;
	  lospac = 30.0*val_per_pix;
	  hispac = 60.0*val_per_pix;
	  range = pow(10.0, floor(log10(lospac)));
	  range10 = 10.0*range;
	  range5 = 5.0*range;
	  range2 = 2.0*range;

	  if (hispac >= range10)
	    ret->ordinate.maj.inter = range10;
	  else if((lospac <= range5) && (hispac >= range5))
	    ret->ordinate.maj.inter = range5;
	  else
	    ret->ordinate.maj.inter = range2;

	  if (ret->ordinate.maj.inter <= 1.0)
	    right_of_d = 1 + (int)(0.5 - log10(ret->ordinate.maj.inter));
	  else
	    right_of_d = 0;

	  sprintf(abform,"%s%df","%8.",right_of_d);
	  reticle_set_ord_precision(ret,abform);
	  ret->ordinate.maj.style = EDGES;
	  ret->ordinate.maj.list = NULL;
	  ret->ordinate.maj.num = 0;
	  ret->ordinate.min1.style =  EDGES;
	  ret->ordinate.min1.inter = ret->ordinate.maj.inter/2.0;
	  ret->ordinate.min1.list = NULL;
	  ret->ordinate.min1.num = 0;
	  ret->ordinate.min2.style = EDGES;
	  ret->ordinate.min2.inter = ret->ordinate.maj.inter/10.0;
	  ret->ordinate.min2.list = NULL;
	  ret->ordinate.min2.num = 0;
	  ret->ordinate.num_inter = ret->ordinate.maj.inter;
	  ret->ordinate.num_loc = NUM_LB;
	  ret->color = RETICLE_COLOR;

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
	  reticle_set_absc_precision(ret,abform);
	  ret->abscissa.maj.style =  EDGES;
	  ret->abscissa.maj.length = ret->ordinate.maj.inter/5.0;
	  ret->abscissa.min1.style =  EDGES;
	  ret->abscissa.min1.length =  ret->ordinate.maj.inter/10.0;
	  ret->abscissa.min1.inter = ret->abscissa.maj.inter/2.0;
	  ret->abscissa.min2.style = EDGES;
	  ret->abscissa.min2.length =  ret->ordinate.maj.inter/20.0;
	  ret->abscissa.min2.inter = ret->abscissa.maj.inter/10.0;
	  ret->abscissa.num_inter = ret->abscissa.maj.inter;
	  ret->ordinate.maj.length =  ret->abscissa.maj.inter/4.0;
	  ret->ordinate.min1.length = ret->abscissa.maj.inter/6.0;
	  ret->ordinate.min2.length = ret->abscissa.maj.inter/12.0;
	  if(v->reticle_on[chan] && !numbers) {	/* time axis on chan 0 only  */
	    numbers = 1;
	    ret->abscissa.num_loc = NUM_LB;
	  } else
	    ret->abscissa.num_loc = 0;

	  /* get margins required by reticle, numerals, etc. */
	  b = reticle_get_margins(ret);
	  /* ret->bounds.top and ret->bounds.bottom are set up in
	     scale_for_canvas() */
	  ret->bounds.right = rect->r_width; /* this is cheating (bit of a kluge) */
	  if(b->left > maxbounds)
	    maxbounds = b->left;
	  ret->bounds.left = b->left;
	}
      }
      *(v->x_offset) = (numbers)? maxbounds : 0;
    }

    for(chan = 0; chan < v->dims; chan++)
      if(v->reticle_on[chan] && v->ret[chan]) {
	v->ret[chan]->absc.end = ET(v);
	v->ret[chan]->bounds.left = maxbounds;
      }

    return(ret);
  }
  return(NULL);
}

/*********************************************************************/
int 
scale_for_canvas(v)
    View *v;
{
  int i, bot, top;
  extern int scrollbar_height, readout_bar_height;
  double maxval, minval, amax, amin;
  double  scale, size, off, dtm, height;
  Signal	*s;
  Canvas	c;
  Xv_Window	pw;
  struct rect	*r;
  Reticle *ret;
  Bound *b;

  if(v) {
    s = v->sig;
    c = v->canvas;
    pw = canvas_paint_window(c);
    r = (struct rect*) xv_get(pw, WIN_RECT);

    for(i = 0; i < v->dims; i++) {
      if(v->v_rescale[i] || (v->plot_max[i] <= v->plot_min[i])) {
	v->plot_max[i] = s->smax[v->elements[i]];
	v->plot_min[i] = s->smin[v->elements[i]];
      } 	
    }
    prepare_reticle(v);

    if(v->scrollbar)
      size = (double)(r->r_height - v->readout_height - v->scrollbar->height)/v->dims;
    else
      size = (double)(r->r_height - readout_bar_height - scrollbar_height)/v->dims;
    for(i=0, off=r->r_height - (size/2); i < v->dims; i++, off -= size) {
      if(v->reticle_on[i] && (ret = v->ret[i])) {
	b = reticle_get_margins(ret);
	bot = b->bottom;
	top = b->top;
      } else {
	v->reticle_on[i] = 0;
	ret = NULL;
	bot = top = 0;
      }

      height = size - (top + bot);

      maxval = v->plot_max[i];
      minval = v->plot_min[i];

      if(!i) {
	amax = maxval;
	amin = minval;
      } else {
	if(maxval > amax)
	  amax = maxval;
	if(minval < amin)
	  amin = minval;
      }
      v->start_yval = amin;
      v->end_yval = amax;
      if((dtm = maxval - minval) == 0.0) dtm = 1.0;
      if(ret)
	scale = - ((double)height)/dtm;
      else
	scale = - ALMOST * ((double)height)/dtm;

      v->y_offset[i] = off - bot + (size/2) - (height/2) -
	               (scale * (maxval + minval) / 2.0);
      v->y_scale[i] = - PIX_PER_CM/scale;
      if(ret) {
	ret->bounds.top = off - (size/2) + top;
	ret->bounds.bottom = ret->bounds.top + height;
      }
    }
    return(TRUE);		/* in case anyone cares */
  }
  return(FALSE);
}
