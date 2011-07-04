/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin
 *
 *  view_setter.c
 *  a collection of routines for setting view attributes in "xwaves"
 *
 */

static char *sccs_id = "@(#)view_setter.c	1.12	12/6/95	ATT/ERL";

#include <Objects.h>
#include <spectrogram.h>
#include <file_ext.h>
#include <esps/esps.h>
#include <esps/fea.h>

extern Object program;
extern int 	debug_level, w_verbose;
extern char *get_line_item(), *get_next_item(), *read_all_types();


/* The rule is this: If the channel is not specified, the setting will
   be applied to all active channels. */
static  char view_channels[MES_BUF_SIZE] = ""; /* used for channel-specific attr. */
static Object *view_object = NULL;
static View *the_view;

/***********************************************************************/
view_assimilate_args(alist, adscr)
     register char *alist;
     Menuop *adscr;
{
  register int gotsome = 0, gotone, n;
  char name[NAMELEN], buffer[MES_BUF_SIZE];
  char *parsed_as;
  register Menuop *ad, *ad2;

  if(alist && *alist && adscr) {
    *view_channels = 0;
    while((*alist == ' ') || (*alist == '\t'))
      alist++;			/* skip initial blanks */
    while(*alist && (*alist != '\n')) {
      strnscan(alist,name,sizeof(name));
      if(! *(alist = get_line_item(alist))) {
	sprintf(notice_msg,"No argument for selector %s",name);
        show_notice(1,notice_msg);
	if (debug_level > 1 || w_verbose > 1)
	    fprintf(stderr,"Got %d args\n", gotsome);
	return(gotsome);
      }
      n = strlen(name);
      ad = adscr;
      gotone = 0;
      while(ad) {
	if(!strncmp(name,ad->name,n)) {
	  parsed_as = ad->name;
	  if(strlen(ad->name) != n) {
	    ad2 = ad->next;		/* check for uniqueness */
	    while(ad2) {
	      if(!strncmp(name,ad2->name,n)) /* found another match... */
		break;
	      ad2 = ad2->next;
	    }
	  } else
	    ad2 = NULL;
	  if(!ad2) {		/* reached end with no dups. */
	    alist = read_all_types(alist,ad->data,buffer);
/*	    if(ad->proc && *buffer) { changed 7/27/95 dt */
	    if(ad->proc) {
	      ad->proc(the_view,buffer);
	      gotsome++;
	      gotone = 1;
	    }
	  }
	  break;
	} else
	  ad = ad->next;
      }

      if (debug_level > 1 || w_verbose > 1) {
	if(gotone)
	  fprintf(stderr,"View parsed attribute %s as %s\n", name, parsed_as);
	else
	  fprintf(stderr,"View attribute %s not found in Selector list.\n", name);
      }
	
      if(! gotone) {		/* keyword not found in adscr list */
	strnscan(alist,buffer,sizeof(buffer));
	if((*buffer == '%') || (*buffer == '#')) { /* looks like a format spec. */
	  alist = get_line_item(alist);	/* skip the non-standard item */
	  alist = read_all_types(alist,buffer,NULL);
	} else {		/* punt */
	  sprintf(notice_msg,
                 "Unknown or non-unique selector %s; argument %s ignored\n",
		 name,buffer);
          show_notice(1,notice_msg);
	  alist = get_line_item(alist);
	}
      }
    }
  }
  if (debug_level > 1 || w_verbose > 1) 
        fprintf(stderr,"Got %d args\n", gotsome);

  return(gotsome);
}

static char *cur_ch;
/***********************************************************************/
channel_access(str)
     char *str;
{
  int tst;
  if(str && *str) {
    cur_ch = str;
    if(sscanf(cur_ch,"%d",&tst))
      return(TRUE);
  }
  cur_ch = NULL;
  return(FALSE);
}

/***********************************************************************/
next_chan()
{
int tmp;
if(cur_ch && *cur_ch && sscanf(cur_ch,"%d", &tmp)) {
    cur_ch = get_next_item(cur_ch);
    return(tmp);
  }
  cur_ch = NULL;
  return(-1);
}

/***********************************************************************/
void view_set_background(v, val)
     View *v;
     char *val;
{
  v->background = atoi(val);
}

/***********************************************************************/
void view_set_zoom_ratio(v, val)
     View *v;
     char *val;
{
  v->zoom_ratio = atof(val);
}
  
/***********************************************************************/
void view_set_cross_level(v, val)
     View *v;
     char *val;
{
  v->cross_level = atof(val);
}
  
/***********************************************************************/
void view_set_page_step(v, val)
     View *v;
     char *val;
{
  v->page_step = atof(val);
}

/* These need to be done (sometime...) 
start_time
(end_time)
loc_y
loc_x
height
width
*/

/***********************************************************************/
void view_set_invert_dither(v, val)
     View *v;
     char *val;
{
  v->invert_dither = atoi(val);
}

/***********************************************************************/
void view_set_overlay_as_number(v, val)
     View *v;
     char *val;
{
  v->overlay_as_number = atoi(val);
}

/***********************************************************************/
void set_view_channels(v, val)
     View *v;
     char *val;
{
  strcpy(view_channels,val);
}

/***********************************************************************/
void view_set_redraw_on_release(v, val)
     View *v;
     char *val;
{
  v->redraw_on_release = atoi(val);
}

/***********************************************************************/
void view_set_rewrite_after_edit(v, val)
     View *v;
     char *val;
{
  v->rewrite_after_edit = atoi(val);
}

/***********************************************************************/
void view_set_edit_ganged(v, val)
     View *v;
     char *val;
{
  if(v->sig && v->sig->obj)
    ((Object*)(v->sig->obj))->edit_ganged = atoi(val);
}

/***********************************************************************/
void view_set_zoom_ganged(v, val)
     View *v;
     char *val;
{
  if(v->sig && v->sig->obj)
    ((Object*)(v->sig->obj))->zoom_ganged = atoi(val);
}

/***********************************************************************/
void view_set_find_crossing(v, val)
     View *v;
     char *val;
{
    v->find_crossing = atoi(val);
}

/***********************************************************************/
void view_set_scroll_ganged(v, val)
     View *v;
     char *val;
{
  if(v->sig && v->sig->obj)
    ((Object*)(v->sig->obj))->scroll_ganged = atoi(val);
}

/***********************************************************************/
void view_set_scrollbar_height(v, val)
     View *v;
     char *val;
{
  if(v->scrollbar)
    v->scrollbar->height = atoi(val);
}


/***********************************************************************/
void view_set_readout_height(v, val)
     View *v;
     char *val;
{
  v->readout_height = atoi(val);
}

/***********************************************************************/
void view_set_line_color(v, val)
     View *v;
     char *val;
{
  int chan, ival = atoi(val);

  if(channel_access(view_channels)) {
    while((chan = next_chan()) >= 0) {
      if(v->sig && (chan < v->sig->dim))
	v->colors[view_invert_display_index(v,chan)] = ival;
    }
  } else
    for(chan = 0; chan < v->dims; chan++)
      v->colors[chan] = ival;
}

/***********************************************************************/
void view_set_line_type(v, val)
     View *v;
     char *val;
{
  int chan, ival = atoi(val);

  if(channel_access(view_channels)) {
    while((chan = next_chan()) >= 0) {
      if(v->sig && (chan < v->sig->dim))
	v->line_types[view_invert_display_index(v,chan)] = ival;
    }
  } else
    for(chan = 0; chan < v->dims; chan++)
      v->line_types[chan] = ival;
}

/***********************************************************************/
void view_set_val_offset(v, val)
     View *v;
     char *val;
{
  int chan;
  double dval = atof(val);

  if(isa_spectrogram_view(v))
    v->val_offset[0] = dval;
  else
    if(channel_access(view_channels)) {
      while((chan = next_chan()) >= 0) {
	if(v->sig && (chan < v->sig->dim))
	  v->val_offset[view_invert_display_index(v,chan)] = dval;
      }
    } else
      for(chan = 0; chan < v->dims; chan++)
	v->val_offset[chan] = dval;
}

/***********************************************************************/
void view_set_val_scale(v, val)
     View *v;
     char *val;
{
  int chan;
  double dval = atof(val);

  if(isa_spectrogram_view(v))
    v->val_scale[0] = dval;
  else
    if(channel_access(view_channels)) {
      while((chan = next_chan()) >= 0) {
	if(v->sig && (chan < v->sig->dim))
	  v->val_scale[view_invert_display_index(v,chan)] = dval;
      }
    } else
      for(chan = 0; chan < v->dims; chan++)
	v->val_scale[chan] = dval;
}

/***********************************************************************/
void view_set_show_vals(v, val)
     View *v;
     char *val;
{
  int chan, ival = atoi(val);

  if(channel_access(view_channels)) {
    while((chan = next_chan()) >= 0) {
      if(v->sig && (chan < v->sig->dim))
	v->show_vals[view_invert_display_index(v,chan)] = ival;
    }
  } else
    for(chan = 0; chan < v->dims; chan++)
      v->show_vals[chan] = ival;
}

/***********************************************************************/
void view_set_show_current_chan(v, val)
     View *v;
     char *val;
{
  int chan, ival = atoi(val);

  v->show_current_chan = ival;
}

/***********************************************************************/
void view_set_show_labels(v, val)
     View *v;
     char *val;
{
  int chan, ival = atoi(val);

  if(channel_access(view_channels)) {
    while((chan = next_chan()) >= 0) {
      if(v->sig && (chan < v->sig->dim))
	v->show_labels[view_invert_display_index(v,chan)] = ival;
    }
  } else
    for(chan = 0; chan < v->dims; chan++)
      v->show_labels[chan] = ival;
}

/***********************************************************************/
void view_set_reticle_on(v, val)
     View *v;
     char *val;
{
  int chan, ival = atoi(val);

  if(channel_access(view_channels)) {
    while((chan = next_chan()) >= 0) {
      if(v->sig && (chan < v->sig->dim))
	v->reticle_on[view_invert_display_index(v,chan)] = ival;
    }
  } else
    for(chan = 0; chan < v->dims; chan++)
      v->reticle_on[chan] = ival;
}

/***********************************************************************/
void view_set_spect_interp(v, val)
     View *v;
     char *val;
{
  v->spect_interp = atoi(val);
}

/***********************************************************************/
void view_set_h_rescale(v, val)
     View *v;
     char *val;
{
  v->h_rescale = atoi(val);
}


/***********************************************************************/
void view_set_rescale_scope(v, val)
     View *v;
     char *val;
{
  v->rescale_scope = scope_to_number(val);
}

/***********************************************************************/
void view_set_v_rescale(v, val)
     View *v;
     char *val;
{
  int chan, ival = atoi(val);

  if(channel_access(view_channels)) {
    while((chan = next_chan()) >= 0) {
      if(v->sig && (chan < v->sig->dim))
	v->v_rescale[view_invert_display_index(v,chan)] = ival;
    }
  } else
    for(chan = 0; chan < v->dims; chan++)
      v->v_rescale[chan] = ival;
}

/***********************************************************************/
void view_set_plot_max(v, val)
     View *v;
     char *val;
{
  int chan, i;
  double ival = atoi(val);

  if(channel_access(view_channels)) {
    while((chan = next_chan()) >= 0) {
      if(v->sig && (chan < v->sig->dim)) {
	v->plot_max[i = view_invert_display_index(v,chan)] = ival;
	v->v_rescale[i] = FALSE;
      }
    }
  } else
    for(chan = 0; chan < v->dims; chan++) {
      v->plot_max[chan] = ival;
      v->v_rescale[chan] = FALSE;
    }
}

/***********************************************************************/
void view_set_plot_min(v, val)
     View *v;
     char *val;
{
  int chan, i;
  double ival = atoi(val);

  if(channel_access(view_channels)) {
    while((chan = next_chan()) >= 0) {
      if(v->sig && (chan < v->sig->dim)) {
	v->plot_min[i = view_invert_display_index(v,chan)] = ival;
	v->v_rescale[i] = FALSE;
      }
    }
  } else
    for(chan = 0; chan < v->dims; chan++) {
      v->plot_min[chan] = ival;
      v->v_rescale[chan] = FALSE;
    }
}

/***********************************************************************/
void view_set_middle_op(v, val)
     View *v;
     char *val;
{
  char *cs;
  Menuop *search_all_menus_but();

  if(isa_spectrogram_view(v))
    cs = "wave";
  else
    cs = "spect";
  
  v->mid_but_proc = search_all_menus_but(cs,val);
  
  if(v->sig)
    update_window_titles(v->sig);
}

/***********************************************************************/
void view_set_move_op(v, val)
     View *v;
     char *val;
{
  char *cs;
  Menuop *search_all_menus_but();

  if(isa_spectrogram_view(v))
    cs = "wave";
  else
    cs = "spect";
  
  v->move_proc = search_all_menus_but(cs,val);
  
  if(v->sig)
    update_window_titles(v->sig);
}

/***********************************************************************/
void view_set_left_op(v, val)
     View *v;
     char *val;
{
  char *cs;
  Menuop *search_all_menus_but();

  if(isa_spectrogram_view(v))
    cs = "wave";
  else
    cs = "spect";
  
  v->left_but_proc = search_all_menus_but(cs,val);
  
  if(v->sig)
    update_window_titles(v->sig);
}

/***********************************************************************/
void view_set_mark_reference(v, val)
     View *v;
     char *val;
{
  char *savestring();

  if(val && *val) {
    if(v->mark_reference) {
      if(strlen(v->mark_reference) >= strlen(val)) {
	strcpy(v->mark_reference,val);
	return;
      } else
	free(v->mark_reference);
    }
    v->mark_reference = savestring(val);
  }
}

/***********************************************************************/
void view_set_shorten_header(v, val)
     View *v;
     char *val;
{
  v->shorten_header = atoi(val);
  if(v->sig)
    update_window_titles(v->sig);
}

/***********************************************************************/
void view_set_cursor_yval(v, val)
     View *v;
     char *val;
{
  v->cursor_yval = atof(val);
}

/***********************************************************************/
void view_set_cursor_chan(v, val)
     View *v;
     char *val;
{
  v->cursor_channel = atoi(val);
}

/***********************************************************************/
void view_set_cursor_time(v, val)
     View *v;
     char *val;
{
  v->cursor_time = atof(val);
}

/***********************************************************************/
void view_set_lmarker_time(v, val)
     View *v;
     char *val;
{
  v->lmarker_time = atof(val);
}

/***********************************************************************/
void view_set_rmarker_time(v, val)
     View *v;
     char *val;
{
  v->rmarker_time = atof(val);
}

/***********************************************************************/
void view_set_tmarker_yval(v, val)
     View *v;
     char *val;
{
  v->tmarker_yval = atof(val);
}

/***********************************************************************/
void view_set_tmarker_chan(v, val)
     View *v;
     char *val;
{
  v->tmarker_chan = atoi(val);
}

/***********************************************************************/
void view_set_bmarker_yval(v, val)
     View *v;
     char *val;
{
  v->bmarker_yval = atof(val);
}

/***********************************************************************/
void view_set_bmarker_chan(v, val)
     View *v;
     char *val;
{
  v->bmarker_chan = atoi(val);
}

/***********************************************************************/
void view_set_view(v, val)
     View *v;
     char *val;
{
  Signal *s, *find_signal();

  if(apply_waves_input_path(val, val) &&
     (s = find_signal(view_object, val)) &&
     s->views)
    the_view = s->views;
}

static Menuop
  vi35 = {"b_marker_chan", view_set_bmarker_chan , "#qstr", NULL},
  vi34 = {"b_marker_yval", view_set_bmarker_yval , "#qstr", &vi35},
  vi33 = {"t_marker_chan", view_set_tmarker_chan , "#qstr", &vi34},
  vi32 = {"t_marker_yval", view_set_tmarker_yval , "#qstr", &vi33},
  vi31 = {"r_marker_time", view_set_rmarker_time , "#qstr", &vi32},
  vi30 = {"l_marker_time", view_set_lmarker_time , "#qstr", &vi31},
  vi29 = {"channels", set_view_channels , "#qstr", &vi30},
  vi28 = {"plot_max", view_set_plot_max , "#qstr", &vi29},
  vi27 = {"plot_min", view_set_plot_min , "#qstr", &vi28},
  vi26 = {"left_op", view_set_left_op , "#strq", &vi27},
  vi25b = {"move_op", view_set_move_op , "#strq", &vi26},
  vi25 = {"middle_op", view_set_middle_op , "#strq", &vi25b},
  vi24 = {"cursor_time", view_set_cursor_time , "#qstr", &vi25},
  vi23 = {"cursor_channel", view_set_cursor_chan , "#qstr", &vi24},
  vi22 = {"cursor_yval", view_set_cursor_yval , "#qstr", &vi23},
  vi21 = {"shorten_header", view_set_shorten_header , "#qstr", &vi22},
  vi20 = {"rescale_scope", view_set_rescale_scope , "#qstr", &vi21},
  vi19b = {"v_rescale", view_set_v_rescale , "#qstr", &vi20},
  vi19 = {"h_rescale", view_set_h_rescale , "#qstr", &vi19b},
  vi18 = {"spect_interp", view_set_spect_interp , "#qstr", &vi19},
  vi17 = {"reticle_on", view_set_reticle_on , "#qstr", &vi18},
  vi16b = {"mark_reference", view_set_mark_reference , "#qstr", &vi17},
  vi16 = {"show_labels", view_set_show_labels , "#qstr", &vi16b},
  vi15a = {"show_current_chan", view_set_show_current_chan , "#qstr", &vi16},
  vi15 = {"show_vals", view_set_show_vals , "#qstr", &vi15a},
  vi14c = {"value_scale", view_set_val_scale , "#qstr", &vi15},
  vi14b = {"value_offset", view_set_val_offset , "#qstr", &vi14c},
  vi14 = {"line_type", view_set_line_type , "#qstr", &vi14b},
  vi13 = {"line_color", view_set_line_color , "#qstr", &vi14},
  vi12 = {"readout_bar_height", view_set_readout_height , "#qstr", &vi13},
  vi11 = {"scrollbar_height", view_set_scrollbar_height , "#qstr", &vi12},
  vi10 = {"scroll_ganged", view_set_scroll_ganged , "#qstr", &vi11},
  vi9a = {"zoom_ganged", view_set_zoom_ganged , "#qstr", &vi10},
  vi9 = {"find_crossing", view_set_find_crossing , "#qstr", &vi9a},
  vi8 = {"edit_ganged", view_set_edit_ganged , "#qstr", &vi9},
  vi7 = {"rewrite_after_edit", view_set_rewrite_after_edit , "#qstr", &vi8},
  vi6 = {"redraw_on_release", view_set_redraw_on_release , "#qstr", &vi7},
  vi5 = {"overlay_as_number", view_set_overlay_as_number , "#qstr", &vi6},
  vi4 = {"invert_dither", view_set_invert_dither , "#qstr", &vi5},
  vi3 = {"page_step", view_set_page_step , "#qstr", &vi4},
  vi2b = {"file", view_set_view , "#qstr", &vi3},
  vi2a = {"zoom_ratio", view_set_zoom_ratio , "#qstr", &vi2b},
  vi2 = {"cross_level", view_set_cross_level , "#qstr", &vi2a},
  vi1 = {"background", view_set_background , "#qstr", &vi2};

/***********************************************************************/
update_view(v)
     View *v;
{
  if(v && v->canvas)
    redoit(v->canvas);
}

/***********************************************************************/
char *set_view_attributes(o,str)
     Object *o;
     char *str;
{
  extern char ok[], null[];
  char *cret, *checking_activators();

  if((cret = checking_activators(str, &vi1)))
    return(cret);

  if(o && str && *str) {
    View *v;
    Signal *s;
    
    if((s = o->signals)) {
      view_object = o;
      while(s) {
	v = s->views;
	
	while(v) {
	  if(v->extra_type != VIEW_OVERLAY) {
	    int got;
	    the_view = v;
	    if((got = view_assimilate_args(str, &vi1))) {
	      update_view(the_view);
	      return(ok);
	    } else
	      return(null);
	  }
	  v = v->next;
	}
	s = s->others;
      }
    }
  }
  return(null);
}

Menuop *view_get_settable_list()
{
  return(&vi1);
}

/***********************************************************************/
void set_all_view_attributes(o,str)
     Object *o;
     char *str;
{
  if(o && str && *str) {
    View *v;
    Signal *s;

    if((s = o->signals)) {
      view_object = o;
      while(s) {
	v = s->views;
	
	while(v) {
	  if(v->extra_type != VIEW_OVERLAY) {
	    int got;
	    the_view = v;
	    if((got = view_assimilate_args(str, &vi1)))
	      update_view(the_view);
	  }
	  v = v->next;
	}
	s = s->others;
      }
    }
  }
  return;
}

/***********************************************************************/
void set_wave_view_attributes(o,str)
     Object *o;
     char *str;
{
  if(o && str && *str) {
    View *v;
    Signal *s;

    if((s = o->signals)) {
      view_object = o;
      while(s) {
	v = s->views;
	
	while(v) {
	  if((v->extra_type != VIEW_OVERLAY) && !isa_spectrogram_view(v)) {
	    int got;
	    the_view = v;
	    if((got = view_assimilate_args(str, &vi1)))
	      update_view(the_view);
	  }
	  v = v->next;
	}
	s = s->others;
      }
    }
  }
  return;
}

/***********************************************************************/
void set_spect_view_attributes(o,str)
     Object *o;
     char *str;
{
  if(o && str && *str) {
    View *v;
    Signal *s;

    if((s = o->signals)) {
      view_object = o;
      while(s) {
	v = s->views;
	
	while(v) {
	  if((v->extra_type != VIEW_OVERLAY) && isa_spectrogram_view(v)) {
	    int got;
	    the_view = v;
	    if((got = view_assimilate_args(str, &vi1)))
	      update_view(the_view);
	  }
	  v = v->next;
	}
	s = s->others;
      }
    }
  }
  return;
}
