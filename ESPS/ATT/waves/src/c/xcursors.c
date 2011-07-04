/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988 AT&T	*/
/*	  and Entropic Speech, Inc.	*/
/*	  All Rights Reserved.		*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC SPEECH, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* xcursors.c */

#ifndef lint
static char *sccs_id = "@(#)xcursors.c	1.13 9/28/98 ERL/ATT";
#endif

#include <Objects.h>
#include <xview/font.h>

extern Xv_Font def_font;
extern int     def_font_height, def_font_width;
extern int     doing_print_graphic, print_only_plot;

View *focus_view;

/*********************************************************************/
transform_xy(v, x, y)
     View *v;
     int x, y;
{
  if(v) {
    Signal *s = v->sig;
    Object *o = (Object*)s->obj;
    double freq = -1.0, time = 0.0, level_crossing_time();
    int chan = 0, sdim = s->dim, sigtype;
    
    if(v->xy_to_chan)
      chan = v->xy_to_chan(v,x,y);

    if(v->find_crossing)
      time = level_crossing_time(v,x,v->cross_level,chan);
    else
      if(v->x_to_time)
	time = v->x_to_time(v,x);

    if(v->y_to_yval)
      freq = v->y_to_yval(v,y);
    if(freq < v->start_yval)
      freq = v->start_yval;
    else
      if(freq > v->end_yval)
	freq = v->end_yval;
    v->cursor_channel = chan;
    focus_view = v;
    sigtype = type_of_signal(s);
    /* Check other views */
    s = o->signals;
    while( s ) {
      View *v2 = s->views;
      while( v2 ) {
	if (v2 != v) {
	  if(v2->xy_to_chan && isa_spectrogram_view(v2))
	    v2->cursor_channel = v2->xy_to_chan(v2,v2->time_to_x(v2,time),
						v2->yval_to_y(v2,freq));
	  else
	    if((type_of_signal(v2->sig) == sigtype) &&
	       (sdim == v2->sig->dim))
	      v2->cursor_channel = chan;
	  else
	    if((v2->extra_type == VIEW_OVERLAY) &&
	       ((View*)(v2->extra) == v) &&
	       v2->xy_to_chan)
	      v2->cursor_channel = v2->xy_to_chan(v2,x,y);
	}
	v2 = v2->next;
      }
      s = s->others;
    }

    move_cursors(v,time,freq);
    return(TRUE);
  } else
    return(FALSE);
}

/*********************************************************************/
link_views(v)
     View *v;
{
  if(v) {
    Signal *s = v->sig;
    Object *o = (Object*)s->obj;
    double freq = v->cursor_yval, time = v->cursor_time;
    int chan = v->cursor_channel, sdim = s->dim, sigtype;
    

    sigtype = type_of_signal(s);
    /* Check other views */
    s = o->signals;
    while( s ) {
      View *v2 = s->views;
      while( v2 ) {
	if (v2 != v) {
	  if(v2->xy_to_chan && isa_spectrogram_view(v2))
	    v2->cursor_channel = v2->xy_to_chan(v2,v2->time_to_x(v2,time),
						v2->yval_to_y(v2,freq));
	  else
	    if((type_of_signal(v2->sig) == sigtype) &&
	       (sdim == v2->sig->dim))
	      v2->cursor_channel = chan;
	  else
	    if((v2->extra_type == VIEW_OVERLAY) &&
	       ((View*)(v2->extra) == v))
	      v2->cursor_channel = v2->xy_to_chan(v2,v2->time_to_x(v2,time),
						  v2->yval_to_y(v2,freq));
	}
	v2 = v2->next;
      }
      s = s->others;
    }

    move_cursors(v,time,freq);
    return(TRUE);
  } else
    return(FALSE);
}

/*********************************************************************/
move_cursors(v, time, freq)
     register View   *v;
     double time, freq;
{
  extern int options;

  if(v) {
    Signal *s;
    Object *o;

    /* This bit of magic "fixes" the
       swapped-out-process-on-a-sparc2-crashing bug! (dt 7/29/92)! */
    if(options & ENABLE_CURSOR_HACK) {
      if(v && v->canvas) {
	Xv_Window	pw;
	pw = canvas_paint_window(v->canvas);
	pw_text(pw, 0, 800,
		PIX_SRC|PIX_COLOR(FOREGROUND_COLOR), def_font, ".");
      }
    }
    
    s = v->sig;
    o = (Object*)s->obj;

    /* Display local cursors first. */
    if (v->cursor_plot)	v->cursor_plot(v, CURSOR_COLOR);
    v->cursor_yval = freq;
    v->cursor_time = time;
    if (v->cursor_plot)	v->cursor_plot(v, CURSOR_COLOR);
    if (v->x_print)	v->x_print(v);
    if (v->y_print)	v->y_print(v);

    /* display other cursors */
    s = o->signals;
    while( s ) {
      View *v2 = s->views;
      while( v2 ) {
	if (v2 != v) {
	  if (v2->cursor_plot)	v2->cursor_plot(v2, CURSOR_COLOR);
	  v2->cursor_time = v->cursor_time;
	  if(v->cursor_yval >= v2->start_yval)
	    v2->cursor_yval = v->cursor_yval;
	  if (v2->cursor_plot)	v2->cursor_plot(v2, CURSOR_COLOR);
	  if (v2->x_print)	v2->x_print(v2);
	  if (v2->y_print)	v2->y_print(v2);

	}
	v2 = v2->next;
      }
      s = s->others;
    }
  }
}

/*********************************************************************/
move_markers(v, time, do_left)
     View    *v;
     double  time;
     int do_left;	/* 0=>right; 1=>left; 2=>both */
{
  Signal *s;
  Object *o;
  double temp;

  s = ((Object*)((Signal*)v->sig)->obj)->signals;

  while( s ) {
    v = s->views;
    while( v ) {

      if (v->vmarker_plot)	v->vmarker_plot(v,do_left);
	
      if ((do_left & 1))
	v->lmarker_time = time;
      else
	v->rmarker_time = time;

      if((!(do_left & 1)) &&
	 (v->lmarker_time > v->rmarker_time)) { /* swap left & right */
	   if(v->vmarker_plot) 
	     v->vmarker_plot(v, 1); /* turn off left marker */
	   temp = v->lmarker_time; /* do the swap */
	   v->lmarker_time = v->rmarker_time;
	   v->rmarker_time = temp;
	   if(v->vmarker_plot) 
	     v->vmarker_plot(v, 1); /* turn on (new) left marker */
	 }
	
      if (v->vmarker_plot)	v->vmarker_plot(v, do_left);
      v = v->next;
    }
    s = s->others;
  }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
move_ord_markers(v, x, y, do_top)
     View *v;
     int x, y, do_top;
{
  Signal *s, *s0;
  View *v0;
  Object *o;
  double temp;
  int sigtype, itmp;

  s = ((Object*)((Signal*)v->sig)->obj)->signals;
  s0 = v->sig;
  v0 = v;
  sigtype = type_of_signal(s0);


  /* First do the target view, since it's values will be needed. */

  if (v->hmarker_plot)		/* XOR off old marker */
    v->hmarker_plot(v,do_top);
  
  if ((do_top & 1)) {
    v->tmarker_chan = v->cursor_channel;
    v->tmarker_yval = v->y_to_yval(v,y);
  } else {
    v->bmarker_chan = v->cursor_channel;
    v->bmarker_yval = v->y_to_yval(v,y);
  }

  /* Now, do all other views in the ensemble. */
  while( s ) {

    /* The semantics of moving horizontal markers across views is very
       unclear.  The following heuristic will fail in some cases.  This
       situation can be improved, but probably not fixed completely. */

    if((type_of_signal(s) == sigtype) && (v = s->views)) {
      if(v != v0) { /* if this is a different view... */

	if (v->hmarker_plot)		/* XOR off old marker */
	  v->hmarker_plot(v,do_top);
  
	if ((do_top & 1)) {
	  if(v->xy_to_chan && isa_spectrogram_view(v)) {
	    v->tmarker_yval = v0->tmarker_yval;
	    v->tmarker_chan = v->xy_to_chan(v,v->time_to_x(v,v0->cursor_time),
					    v->yval_to_y(v,v->tmarker_yval));
	  } else
	    if(s0->dim == s->dim) {
	      v->tmarker_yval = v0->tmarker_yval;
	      v->tmarker_chan = v0->tmarker_chan;
	    }
	} else {
	  if(v->xy_to_chan && isa_spectrogram_view(v)) {
	    v->bmarker_yval = v0->bmarker_yval;
	    v->bmarker_chan = v->xy_to_chan(v,v->time_to_x(v,v0->cursor_time),
					    v->yval_to_y(v,v->bmarker_yval));
	  } else
	    if(s0->dim == s->dim) {
	      v->bmarker_yval = v0->bmarker_yval;
	      v->bmarker_chan = v0->bmarker_chan;
	    }
	}

	if((!(do_top & 1)) &&
	   (v->tmarker_yval < v->bmarker_yval)) { /* swap top & bottom */
	  if(v->hmarker_plot) 
	    v->hmarker_plot(v, 1); /* turn off top marker */
	  temp = v->tmarker_yval; /* do the swap */
	  itmp = v->tmarker_chan;
	  v->tmarker_yval = v->bmarker_yval;
	  v->tmarker_chan = v->bmarker_chan;
	  v->bmarker_yval = temp;
	  v->bmarker_chan = itmp;
	  if(v->hmarker_plot) 
	    v->hmarker_plot(v, 1); /* turn on (new) top marker */
	}
	
	if (v->hmarker_plot)	v->hmarker_plot(v, do_top);
	v = v->next;
      }
    }
    s = s->others;
  }

  v = v0;			/* Now finish up the original. */
  if((!(do_top & 1)) &&
     (((v->tmarker_chan == v->bmarker_chan) &&
      (v->tmarker_yval < v->bmarker_yval)) ||
      (v->tmarker_chan < v->bmarker_chan))) { /* swap top & bottom */
    if(v->hmarker_plot) 
      v->hmarker_plot(v, 1);	/* turn off top marker */
    temp = v->tmarker_yval;	/* do the swap */
    itmp = v->tmarker_chan;
    v->tmarker_yval = v->bmarker_yval;
    v->tmarker_chan = v->bmarker_chan;
    v->bmarker_yval = temp;
    v->bmarker_chan = itmp;
    if(v->hmarker_plot) 
      v->hmarker_plot(v, 1);	/* turn on (new) top marker */
  }
	
  if (v->hmarker_plot)	v->hmarker_plot(v, do_top);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
plot_cursors(v,color)
    View    *v;
    int	    color;
{
  Pixwin  *pw;
  Rect    *rec;
  int     x, y1, y2;

  if(doing_print_graphic && print_only_plot)
    return(TRUE);
  if(!v || !v->canvas)
    return(FALSE);
  x = v->time_to_x(v, v->cursor_time);
  pw = canvas_pixwin(v->canvas);
  rec = (Rect *) xv_get((Xv_opaque)pw, WIN_RECT);
  if (v->scrollbar && (v->scrollbar->height > 0) && v->scrollbar->is_on)
   y1 = v->scrollbar->y + v->scrollbar->height;
  else
   y1 = 0;
  y2 = rec->r_height - 1;
  pw_vector(pw, x, y1, x, y2,
	    PIX_COLOR(color)|(PIX_SRC^PIX_DST), color);
  return(TRUE);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
plot_markers(v, which)
    View	    *v;
    int		    which;	/* 0=>right; 1=>left; 2=>both */
{
    Xv_Window	    *pw;
    Rect	    *rec;
    register int    x, y, w_msg, w_win,
		    len = 20,
		    step = 10,
		    offset = (len + step) >> 1;
    int		    y1, y2;
    char	    mes[100];
    double	    dur;

    if(doing_print_graphic && print_only_plot)
      return(TRUE);

    which++;			/* make it easier to decode */
    if(!v || !v->canvas)
	return FALSE;
  
    pw = (Xv_Window*)canvas_pixwin(v->canvas);
    rec = (Rect *) xv_get((Xv_opaque)pw, WIN_RECT);
    if (v->scrollbar && (v->scrollbar->height > 0) && v->scrollbar->is_on)
     y1 = v->scrollbar->y + v->scrollbar->height;
    else
     y1 = 0;
    y2 = rec->r_height - 1;

    if (which & 2) {
	x = v->time_to_x(v, v->lmarker_time);	/* left marker */
	for (y = y2; y >= y1; y -= step+len) {
	    pw_vector(pw, x, y, x, (y - len),
		      PIX_COLOR(MARKER_COLOR)|(PIX_SRC^PIX_DST), MARKER_COLOR);
	}
    }
    if (which & 1) {
	x = v->time_to_x(v, v->rmarker_time);	/* right marker */
	for (y=y2-offset; y >= y1; y -= step+len) {
	    pw_vector(pw, x, y, x, (y - len),
		      PIX_COLOR(MARKER_COLOR)|(PIX_SRC^PIX_DST), MARKER_COLOR);
	}
    }
    dur = v->rmarker_time - v->lmarker_time;
    if(dur > 0.0)
	sprintf(mes, "D:%8.5f   L:%9.5f   R:%9.5f (F:%8.2f)",
	    dur, v->lmarker_time, v->rmarker_time, 1.0/dur);
    else
	sprintf(mes, "D:%8.5f   L:%9.5f   R:%9.5f (F:--------)",
	    dur, v->lmarker_time, v->rmarker_time);

    w_win = rec->r_width;
    w_msg = 38*def_font_width;	/* Part of mes thru R:%9.5f.  Try to keep
				   visible in small windows while avoiding
				   overlap with Time:... messages (see
				   print_x in xplot_data.c and print_spect_x
				   in xspect.c). */
    x = (w_win/2 >= w_msg) ? w_win/2
	: (w_win >= w_msg) ? w_win - w_msg
	: 0;

    pw_text(pw, x, 14,
	    PIX_SRC|PIX_COLOR(FOREGROUND_COLOR), def_font, mes);

    return TRUE;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
plot_hmarkers(v, which)
    View	    *v;
    int		    which;	/* 0=>bottom; 1=>top; 2=>both */
{
  Xv_Window	    *pw;
  Rect	    *rec;
  register int    x, y, w_msg, w_win,
  len = 20,
  step = 10,
  offset = (len + step) >> 1;
  int		    x1, x2, chanh;
  char	    mes[100];
  double	    dur;

  if(doing_print_graphic && print_only_plot)
    return(TRUE);

  which++;			/* make it easier to decode */
  if(!v || !v->canvas)
    return FALSE;
  
  pw = (Xv_Window*)canvas_pixwin(v->canvas);
  rec = (Rect *) xv_get((Xv_opaque)pw, WIN_RECT);
  x1 = 0;
  x2 = rec->r_width - 1;
  chanh = v->cursor_channel;

  if (which & 2) {
    v->cursor_channel = v->tmarker_chan;
    y = v->yval_to_y(v, v->tmarker_yval); /* top marker */
    for (x = x1; x < x2; x += step+len) {
      pw_vector(pw, x, y, x + len, y,
		PIX_COLOR(MARKER_COLOR)|(PIX_SRC^PIX_DST), MARKER_COLOR);
    }
  }
  if (which & 1) {
    v->cursor_channel = v->bmarker_chan;
    y = v->yval_to_y(v, v->bmarker_yval); /* bottom marker */
    for (x = x1+offset; x < x2; x += step+len) {
      pw_vector(pw, x, y, x + len, y,
		PIX_COLOR(MARKER_COLOR)|(PIX_SRC^PIX_DST), MARKER_COLOR);
    }
  }

  v->cursor_channel = chanh;

  return TRUE;
}

  
/*********************************************************************/
object_xor_mark(o, time, color)
     Object *o;
     double time;
     int color;
{
  Signal *s;
  View *v;
  double tsave;

  if(o && (s = o->signals)) {
    while(s) {
      v = s->views;
      while(v) {
	if(v->cursor_plot) {
	  tsave = v->cursor_time;
	  v->cursor_time = time;
	  v->cursor_plot(v,color);
	  v->cursor_time = tsave;
	}
	v = v->next;
      }
      s = s->others;
    }
  }
}

/*********************************************************************/
view_xor_marks(v)
     View *v;
{
  Signal *s = v->sig;
  Object *o;
  Marks *m;
  double tsave;
  int count, col;

  if(s && (o = (Object*)s->obj) && (m = o->marks)) {
    count = 0;			/* keep track of odd-even */
    while(m) {
      tsave = v->cursor_time;
      v->cursor_time = m->time;
      v->cursor_plot(v,(col = m->color));
      v->cursor_time = tsave;
      m = m->next;
      count++;
    }
    if(count & 1) {		/* an odd number of marks? */
      tsave = v->cursor_time;
      v->cursor_time = ET(v);
      v->cursor_plot(v,col);
      v->cursor_time = tsave;
    }
  }
}
      

/*********************************************************************/
/* HOMEBREW SCROLLING FACILITY:
   The problem is that Sun doesn't have a generally useful scrollbar
   object: the "slider" has limited capability and takes yet another
   precious file descriptor; the Sun "scrollbar" is inextricably coupled
   to the subwindow in which it appears and can't be used in a general way.
   
   The homebrew scrolling facility looks like this:  There will be a
   region near the top of the canvas which will be sensitive to scrolling
   requests by the left, middle and right buttons.  Left will move the
   waveform point at the cursor to the left edge of the window; right
   will move the wafeform point at the left edge to the cursor position;
   middle will center the display at the point in the file proportional
   to the ratio of the cursor position re the left edge to the window
   width.  A narrow bar indicating the location and proportion of the
   file displayed will be plotted at the top edge of the window.
   Waveform cursor motion will not occur when the pointer is in the
   scroll-sensitive region (??).
   */

/*********************************************************************/
plot_scrollbar(sb)
    register S_bar *sb;
{
  Pixwin      *pw;
  Rect        *rec;
  extern int doing_print_graphic, print_only_plot;
  
/* do not draw scroll bar if doing print graphic and global 
   print_only_plot is non-zero 
*/
  if (doing_print_graphic && print_only_plot)
	return;

  if (sb && (sb->height > 0) && sb->is_on && sb->view &&
      sb->view->canvas) {
    pw = (Pixwin *) canvas_paint_window(sb->view->canvas);
    rec = (Rect *) xv_get((Xv_opaque)pw, WIN_RECT);

    pw_write(pw, sb->x, sb->y, sb->width, sb->height,
	     PIX_COLOR(sb->color)|PIX_SRC, NULL, 0, 0);

    pw_vector(pw, *(sb->view->x_offset), sb->height + sb->y,
	      rec->r_left + rec->r_width - 1, sb->height + sb->y,
              PIX_COLOR(sb->color)|PIX_SRC, sb->color);

    pw_vector(pw, *(sb->view->x_offset), sb->y,
	      rec->r_left + rec->r_width - 1, sb->y,
              PIX_COLOR(RETICLE_COLOR)|PIX_SRC, RETICLE_COLOR);
    if (*(sb->view->x_offset)) 
        pw_vector(pw, *(sb->view->x_offset), sb->height + sb->y,
		  *(sb->view->x_offset), 0,
                  PIX_COLOR(RETICLE_COLOR)|PIX_SRC, RETICLE_COLOR);
    sb->fresh = -1;
    return(TRUE);
  }
  return(FALSE);
}

/*********************************************************************/
S_bar *
new_scrollbar(v)
     register View *v;
{
  register S_bar *sb;
  register double sig_dur;
  extern int scrollbar_height, readout_bar_height;

  if(!(sb = (S_bar*)malloc(sizeof(S_bar)))) {
    printf("Allocation problems in new_scrollbar()\n");
    return(NULL);
  }
  sb->height = 5;		/* about the width of the frame border */
  if(v)
    sb->y = v->readout_height;		/* at top of canvas */
  else
    sb->y = readout_bar_height;
  sb->color = RETICLE_COLOR;	/* change this at will */
  sb->view = v;
  sb->is_on = TRUE;
  sb->fresh = -1;
  sb->height = scrollbar_height;

  if(v && v->sig) {
    sb->x = *v->x_offset + ((v->width - *v->x_offset) *
			   (v->start_time - v->sig->start_time)/
		       (sig_dur = SIG_DURATION(v->sig)));
    sb->width = (v->width - *v->x_offset) * (ET(v) - v->start_time)/sig_dur;
    if(! v->scrollbar) v->scrollbar = sb;
  }
  return(sb);
}

/*********************************************************************/
set_scrollbar(v)
     register View *v;
{
  register S_bar *sb;
  register double sig_dur;
  
  if(v && (sb = v->scrollbar)) {
    sb->x = *v->x_offset + ((v->width - *v->x_offset) *
			   (v->start_time - v->sig->start_time)/
		       (sig_dur = SIG_DURATION(v->sig)));
    sb->width = (v->width - *v->x_offset) * (ET(v) - v->start_time)/sig_dur;
    return(TRUE);
  }
  return(FALSE);
}

/*********************************************************************/
operate_scrollbar(but, x, view)
     register int but, x;
     register View *view;
{
  double page_time;
  Signal *sig;
  double sig_dur, wind_dur, whereto, fract;
  register int id;

  if(view && (sig = view->sig)) {
    x -= *(view->x_offset);
    if(x < 0) x = 0;
    fract = ((double)x)/(view->width - *(view->x_offset));
    sig_dur = SIG_DURATION(sig);
    wind_dur = ET(view) - view->start_time;
    switch(but) {
    case MS_LEFT:
      whereto = view->start_time + (wind_dur * fract);
      break;
    case MS_MIDDLE:
      whereto = sig->start_time + ((sig_dur * fract) - (0.5 * wind_dur));
      break;
    case MS_RIGHT:
      whereto = view->start_time - (wind_dur * fract);
      break;
    default:
      return(FALSE);
    }
    page(view, view->sig, whereto);
  }
  return(FALSE);
}

void plot_time_bar(v,s,sample)
     View *v;
     Signal *s;
     int sample;
{
  int x;
  extern double play_time;
  double fraction;

  fraction = ((double)sample)/(SIG_DURATION(s) * s->freq);
  x = *v->x_offset + ((v->width - *v->x_offset) * fraction);

  if( v->scrollbar && v->scrollbar->is_on) {
    S_bar *sb = v->scrollbar;
    Pixwin      *pw;
    Rect        *rec;

    pw = (Pixwin *) canvas_paint_window(v->canvas);
    rec = (Rect *) xv_get((Xv_opaque)pw, WIN_RECT);

    if((x + 4) > v->width)
      x = v->width - 4;
    if(sb->fresh >= 0)		/* XOR off the old bar */
      pw_write(pw, sb->fresh, sb->y, 4, sb->height,
	       PIX_COLOR(sb->color)|(PIX_SRC^PIX_DST), NULL, 0, 0);

    pw_write(pw, x, sb->y, 4, sb->height,
	     PIX_COLOR(sb->color)|(PIX_SRC^PIX_DST), NULL, 0, 0);
    sb->fresh = x;
  }
  play_time = s->start_time + (((double)sample)/s->freq);
  if((play_time > v->start_time) &&
     (play_time < ET(v))) {
    if (v->cursor_plot)	v->cursor_plot(v, CURSOR_COLOR);
    v->cursor_time = play_time;
    if (v->cursor_plot)	v->cursor_plot(v, CURSOR_COLOR);
    if (v->x_print)	v->x_print(v);
  } else
    file_print_x(v,x);

  return;
}
