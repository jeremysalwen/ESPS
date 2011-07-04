/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1993-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin
 *
 *  view_utils.c
 *  a collection of routines for manipulation of Views in "xwaves"
 */

static char *sccs_id = "@(#)view_utils.c	1.24 8/12/98 ERL";

#include <Objects.h>
#include <spectrogram.h>
#include <file_ext.h>
#include <esps/esps.h>
#include <esps/fea.h>

#define WhiteSpace(ch) ((ch) == ' ' || (ch) == '\t' || (ch) == '\n')

extern Object program;
extern int 	debug_level;
extern char time_range_prefix[], samp_range_prefix[];
extern int c_range_offset;	/* in globals.c */
extern int s_range_offset;

static Selector va11 = {"file", "%s", NULL, NULL},
 va10 = {"name", "%s", NULL, &va11},
 va9b = {"cursor_yval", "%lf", NULL, &va10},
 va9 = {"cursor_time", "%lf", NULL, &va9b},
 va8b = {"l_marker_time", "%lf", NULL, &va9},
 va7b = {"r_marker_time", "%lf", NULL, &va8b},
 va8 = {"t_marker_yval", "%lf", NULL, &va7b},
 va7 = {"b_marker_yval", "%lf", NULL, &va8},
 va6 = {"width", "%d", NULL, &va7},
 va5 = {"height", "%d", NULL, &va6},
 va4 = {"view_start_yval", "%lf", NULL, &va5},
 va3 = {"view_end_yval", "%lf", NULL, &va4},
 va2b = {"mark_reference", "#qstr", NULL, &va3},
 va2a = {"find_crossing", "%d", NULL, &va2b},
 va2 = {"cross_level", "%lf", NULL, &va2a},
 va1 = {"view_start_time", "%lf", NULL, &va2};

static char *view_get_overlays(), *view_get_is_iconized(),
  *view_get_cursor_samp(), *view_get_l_marker_samp(),
  *view_get_l_marker_samp_from_start(), *view_get_objects(),
  *view_get_r_marker_samp(), *view_get_win_x(),
  *view_get_loc_x(), *view_get_loc_y(),
  *view_get_sec_cm(), *view_get_val_cm(), *view_get_cursor_value(),
  *view_get_cursor_values(), *view_get_view_channels(),
  *view_get_view_end_time(), *view_get_range_samp(), *view_get_dur_samp(),
  *view_get_dur_time(), *view_get_f_value(),
  *view_get_range_time(), *view_get_range_yval(), *view_get_range_chan(),
  *view_get_cursor_channels(), *view_get_cursor_chan(), *view_get_cursor_label(),
  *view_get_bmarker_chan(), *view_get_tmarker_chan(), *view_get_operators(),
  *view_get_object_commands(), *view_get_waves_commands(),
  *view_get_settables(), *view_get_variables(), *view_get_files(),
  *view_get_new_files(), *view_get_cursor_element();
static char *view_get_mark_range_time(), *view_get_mark_range_samp(),
  *view_get_r_mark_time(), *view_get_l_mark_time(),
  *view_get_r_mark_samp(), *view_get_l_mark_samp(), *view_get_cursor_labels();

extern char *get_string_list();

typedef struct activator {
  char *name;
  char *(*proc)();
  struct activator *next;
} Activator;



static Activator
  vb14 = {"new_files", view_get_new_files, NULL},
  vb13 = {"operators", view_get_operators, &vb14},
  vb12g = {"objects", view_get_objects, &vb13},
  vb12e = {"files", view_get_files, &vb12g},
  vb12f = {"settables", view_get_settables, &vb12e},
  vb12d = {"variables", view_get_variables, &vb12f},
  vb12c = {"waves_commands", view_get_waves_commands, &vb12d},
  vb12b = {"object_commands", view_get_object_commands, &vb12c},
  vb12 = {"overlays", view_get_overlays, &vb12b},
  vb11 = {"is_iconized", view_get_is_iconized, &vb12},
  vb10 = {"cursor_samp", view_get_cursor_samp, &vb11},
  vb9e = {"f_value", view_get_f_value, &vb10},
  vb9d = {"dur_seconds", view_get_dur_time, &vb9e},
  vb9c = {"dur_time", view_get_dur_time, &vb9d},
  vb9b = {"dur_samp", view_get_dur_samp, &vb9c},
  vb9 = {"l_marker_samp_from_start", view_get_l_marker_samp_from_start, &vb9b},
  vb8c = {"lmsfs", view_get_l_marker_samp_from_start, &vb9},
  vb8b = {"l_marker_samp", view_get_l_marker_samp, &vb8c},
  vb8 = {"r_marker_samp", view_get_r_marker_samp, &vb8b},
  vb7g = {"win_x", view_get_win_x, &vb8},
  vb7f = {"l_mark_samp", view_get_l_mark_samp, &vb7g},
  vb7e = {"r_mark_samp", view_get_r_mark_samp, &vb7f},
  vb7d = {"l_mark_time", view_get_l_mark_time, &vb7e},
  vb7c = {"r_mark_time", view_get_r_mark_time, &vb7d},
  vb7b = {"mark_range_samp", view_get_mark_range_samp, &vb7c},
  vb7a = {"mark_range_time", view_get_mark_range_time, &vb7b},
  vb7 = {"loc_x", view_get_loc_x, &vb7a},
  vb6 = {"loc_y", view_get_loc_y, &vb7},
  vb5 = {"sec/cm", view_get_sec_cm, &vb6},
  vb4 = {"val/cm", view_get_val_cm, &vb5},
  vb3f= {"cursor_element", view_get_cursor_element, &vb4},
  vb3e1 = {"cursor_label", view_get_cursor_label, &vb3f},
  vb3e = {"cursor_labels", view_get_cursor_labels, &vb3e1},
  vb3d = {"cursor_channel", view_get_cursor_chan, &vb3e},
  vb3c = {"b_marker_chan", view_get_bmarker_chan, &vb3d},
  vb3b = {"t_marker_chan", view_get_tmarker_chan, &vb3c},
  vb3 = {"cursor_value", view_get_cursor_value, &vb3b},
  vb2b = {"cursor_values", view_get_cursor_values, &vb3},
  vb2 = {"cursor_channels", view_get_cursor_channels, &vb2b},
  vb1e = {"range_yval", view_get_range_yval, &vb2},
  vb1d = {"range_chan", view_get_range_chan, &vb1e},
  vb1c = {"range_time", view_get_range_time, &vb1d},
  vb1b = {"range_samp", view_get_range_samp, &vb1c},
  vb1 = {"view_channels", view_get_view_channels, &vb1b},
  vb0 = {"view_end_time", view_get_view_end_time, &vb1};

/*************************************************************************/
static double left_mark_time(s, tr)
     double tr;
     Signal *s;
{
  if(s) {
    Object *o;
    Marks *m;
    
    if((o = (Object*)(s->obj)) && (m = o->marks)) {
      if(m->time > tr)
	return(SIG_START_TIME(s));
      while(m) {
	if((! m->next) || ((m->next->time > tr) && (m->time <= tr)))
	  return(m->time);
	m = m->next;
      }
    }
    return(SIG_START_TIME(s));
  }
  return(0.0);
}

/*************************************************************************/
static double right_mark_time(s, tr)
     double tr;
     Signal *s;
{
  if(s) {
    Object *o;
    Marks *m;
    
    if((o = (Object*)(s->obj)) && (m = o->marks)) {
      if(m->time > tr)
	return(m->time);
      while(m && m->next) {
	if(((m->next->time > tr) && (m->time <= tr)))
	  return(m->next->time);
	m = m->next;
      }
    }
    return(SIG_END_TIME(s));
  }
  return(0.0);
}

/*************************************************************************/
static double mark_time_reference(v)
     View *v;
{
  char *ap;

  if(v->mark_reference && (strlen(v->mark_reference) > 0) &&
     strcmp("(null)", v->mark_reference) &&
    (ap = view_get_value(v, v->mark_reference)))
    return(atof(ap));
  else
    return(atof(view_get_value(v,"cursor_time")));

}

/*************************************************************************/
static char *view_get_l_mark_time(v)
     View *v;
{
  static char rv[20];

  if(v && v->sig) {
    sprintf(rv,"%.7f", left_mark_time(v->sig, mark_time_reference(v)));
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_mark_range_time(v)
     View *v;
{
  static char rv[40];

  if(v && v->sig) {
    sprintf(rv,"%s%.7f:%.7f", time_range_prefix,
	    left_mark_time(v->sig, mark_time_reference(v)),
	     right_mark_time(v->sig, mark_time_reference(v)));
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_r_mark_time(v)
     View *v;
{
  static char rv[20];

  if(v && v->sig) {
    sprintf(rv,"%.7f", right_mark_time(v->sig, mark_time_reference(v)));
    return(rv);
  } else
    return("");
}

/*************************************************************************/
int time_to_sample(sig, time)
     Signal *sig;
     double time;
{

  if(sig) {
    if(IS_GENERIC(sig->type))
      return(s_range_offset + (int)((time - sig->start_time) * sig->freq));
    else {
      return(sig->start_samp + s_range_offset + time_to_index(sig,time));
    }
  } else
    return(0);
}

/*************************************************************************/
static char *view_get_mark_range_samp(v)
     View *v;
{
  static char rv[21];

  if(v && v->sig) {
    sprintf(rv,"%s%d:%d", samp_range_prefix,
	    time_to_sample(v->sig,left_mark_time(v->sig,mark_time_reference(v))),
	    time_to_sample(v->sig,right_mark_time(v->sig,mark_time_reference(v))));
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_dur_samp(v)
     View *v;
{
  static char rv[10];

  if(v && v->sig) {
    sprintf(rv,"%d",
    (int)(0.5 + (v->sig->freq * (v->rmarker_time - v->lmarker_time))));
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_f_value(v)
     View *v;
{
  static char rv[10];
  double dur;

  if(v) {
    dur = v->rmarker_time - v->lmarker_time;
    if (dur > 0.0)
       sprintf(rv,"%8.2f", 1.0/(v->rmarker_time - v->lmarker_time));
    else
       sprintf(rv,"--------");
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_dur_time(v)
     View *v;
{
  static char rv[10];

  if(v) {
    sprintf(rv,"%.7f", v->rmarker_time - v->lmarker_time);
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_r_mark_samp(v)
     View *v;
{
  static char rv[10];

  if(v && v->sig) {
    sprintf(rv,"%d",
	    time_to_sample(v->sig,right_mark_time(v->sig,mark_time_reference(v))));
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_l_mark_samp(v)
     View *v;
{
  static char rv[10];

  if(v && v->sig) {
    sprintf(rv,"%d",
	    time_to_sample(v->sig,left_mark_time(v->sig,mark_time_reference(v))));
    return(rv);
  } else
    return("");
  
}

/*************************************************************************/
char **view_get_variable_names()
{
  Activator *mo = &vb0;
  Selector *se = &va1;
  int no = 0;
  char **wopl = NULL, **sort_a_list();
  
  while(mo) {
    if(mo->name && *mo->name && mo->proc)
      no++;
    mo = mo->next;
  }
  while(se) {
    if(se->name && *se->name)
      no++;
    se = se->next;
  }
  if(no) {
    wopl = (char**) malloc(sizeof(char**) * (no+1));
    no = 0;
    mo = &vb0;
    while(mo) {

      if(mo->name && mo->name[0] && mo->proc)
	wopl[no++] = mo->name;

      mo = mo->next;
    }

    se = &va1;
    while(se) {

      if(se->name && se->name[0])
	wopl[no++] = se->name;

      se = se->next;
    }

    wopl[no] = NULL;
  }
  return(sort_a_list(wopl));
}

/*************************************************************************/
static Selector *view_setup_access(v)
     register View *v;
{
  if(v) {
    if(v->sig) {
      va11.dest = v->sig->name;
      va10.dest = ((Object*)v->sig->obj)->name;
    } else {
      va11.dest = NULL;
      va10.dest = NULL;
    }
    va2b.dest = v->mark_reference;
    va2a.dest = (char*)&(v->find_crossing);
    va2.dest = (char*)&(v->cross_level);
    va9.dest = (char*)&(v->cursor_time);
    va9b.dest = (char*)&(v->cursor_yval);
    va8b.dest = (char*)&(v->lmarker_time);
    va7b.dest = (char*)&(v->rmarker_time);
    va8.dest = (char*)&(v->tmarker_yval);
    va7.dest = (char*)&(v->bmarker_yval);
    va6.dest = (char*)&(v->width);
    va5.dest = (char*)&(v->height);
    va4.dest = (char*)&(v->start_yval);
    va3.dest = (char*)&(v->end_yval);
    va1.dest = (char*)&(v->start_time);
    return(&va1);
  } else
    return(NULL);
}

/**************************************************************/
char **object_get_files(o)
     Object *o;
{
  Signal *s, *mo;

  if(o && (mo = s = o->signals)) {
    int no = 0;
    char **wopl = NULL, **sort_a_list();
  
    while(mo) {
      if(mo->views)
	no++;
      mo = mo->others;
    }
    if(no) {
      wopl = (char**) malloc(sizeof(char**) * (no+1));
      no = 0;
      while(s) {
	if(s->views)
	  wopl[no++] = s->name;
	
	s = s->others;
      }
      wopl[no] = NULL;
    }
    return(sort_a_list(wopl));
  }
  return(NULL);
}

/*************************************************************************/
static char *view_get_files(v, str)
     View *v;
     char *str;
{
  extern char *pack_up_list_items();

  if(v && v->sig && v->sig->obj)
    return(pack_up_list_items(object_get_files(v->sig->obj)));
  else
    return("");
}
       
/*************************************************************************/
static char *view_get_settables(v, str)
     View *v;
     char *str;
{
  return(get_string_list("settables"));
}
       
/*************************************************************************/
static char *view_get_objects(v, str)
     View *v;
     char *str;
{
 char *cp;

 if((cp = get_string_list("objects")))
   return(cp);
 else
   return("");
}
       
/*************************************************************************/
static char *view_get_variables(v, str)
     View *v;
     char *str;
{
  return(get_string_list("variables"));
}
       
/*************************************************************************/
static char *view_get_operators(v, str)
     View *v;
     char *str;
{
  return(get_string_list("view"));
}
       
/*************************************************************************/
static char *view_get_new_files(v, str)
     View *v;
     char *str;
{
  return(get_string_list("new_files"));
}
       
/*************************************************************************/
static char *view_get_object_commands(v, str)
     View *v;
     char *str;
{
  return(get_string_list("object"));
}
       
/*************************************************************************/
static char *view_get_waves_commands(v, str)
     View *v;
     char *str;
{
  return(get_string_list("waves"));
}
       
/*************************************************************************/
Event *view_state_to_event(v)
     View *v;
{
  static Event e;

  event_init(&e);
  event_set_x(&e,v->time_to_x(v,v->cursor_time));
  event_set_y(&e,v->yval_to_y(v,v->cursor_yval));
  event_set_window(&e,v->canvas);
  return(&e);
}
		     
/*********************************************************************/
/* Should be called vodo op.  Makes it possible to issue simple-minded
   (and under-specified) commands like: oname op, where oname is a
   display ensemble name, and op is any named operator in any of the
   Menuop lists. */
void view_do_op(v,op)
     View *v;
     char *op;
{
  Canvas canvas = v->canvas;
  if(v && canvas && op && *op) {
    Event *event = (Event*)view_state_to_event(v);
    extern Menuop  *find_operator();
    Moplist  *menu_get_op_lists(), *mol = menu_get_op_lists();
    Menuop *mo;
    
    if(event) {
      char *ce = op + strlen(op)-1;
      while(*op && WhiteSpace(*op)) op++;
      if(*op) {
	while(WhiteSpace(*ce) && (ce > op)) ce--;
	*(++ce) = 0;
	while(mol) {
	  if((mo = find_operator(mol->first_op,op)) &&
	     mo->proc) {
	    mo->proc(canvas, event, mo->data);
	    return;
	  }
	  mol = mol->next;
	}
      }
    }
  }
  fprintf(stderr,"Problems in view_do_op(%s) %x\n",op,v);
}

/*************************************************************************/
arg_to_simple_string(str, n, se)
     char **str;
     int *n;
     Selector *se;
{
  char *get_next_item();
  if(arg_to_string(str, n, se)) {
    char *cp = get_next_item(*str);
    int i;
    cp[(i = strlen(cp))-1] = 0;
    strcpy(*str, cp);
    return((i) ? i : -1);	/* indicate success, but null value */
  }
  return(0);
}
  
/*************************************************************************/
char *value_as_string(sp, name)
     Selector *sp;
     char *name;
{
  static char *cp = NULL;
  static int nstr = 0;

  if(!cp) {
    cp = malloc(MES_BUF_SIZE);
    nstr = MES_BUF_SIZE;
  }
  while(sp) {
    if(!strcmp(sp->name,name)) {
      if(arg_to_simple_string(&cp, &nstr, sp))
	return(cp);
    }
    sp = sp->next;
  }
  return(NULL);
}

/*************************************************************************/
static char *view_get_overlays(v)
     View *v;
{
  if(v) {
    static char *rv = NULL;
    static int svl = 0;
    char *c;
    if(v->overlay_n >= 0) {	/* does it have any overlays? */
      int slen = ((NAMELEN+1) * (v->overlay_n + 1)) + 1;
      if(slen > svl) {
	if(rv)
	  free(rv);
	svl = 0;
	if(!(rv = malloc(slen))) {
	  fprintf(stderr,"Allocation problems in view_get_overlays\n");
	  return("");
	}
	svl = slen;
      }
      *rv = 0;
      
      while(v = v->next) {
	if((v->extra_type == VIEW_OVERLAY) && (v->sig))
	  sprintf(rv+strlen(rv),"%s ",v->sig->name);
      }
      return(rv);
    }
    return("");
  }
  return("");
}

/*************************************************************************/
static char *view_get_is_iconized(v)
     View *v;
{
  if(!v)
    return("");
  else
    if(!v->canvas || xv_get(xv_get(v->canvas,XV_OWNER),FRAME_CLOSED))
      return("1");
    else
      return("0");
}

/*************************************************************************/
static char *view_get_cursor_samp(v)
     View *v;
{
  static char rv[10];

  if(v && v->sig) {
    sprintf(rv,"%d", time_to_sample(v->sig,v->cursor_time));
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_l_marker_samp(v)
     View *v;
{
  static char rv[10];

  if(v && v->sig) {
    sprintf(rv,"%d", time_to_sample(v->sig,v->lmarker_time));
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_l_marker_samp_from_start(v)
     View *v;
{
  static char rv[10];

  if(v && v->sig) {
    sprintf(rv,"%d", time_to_sample (v->sig,v->lmarker_time) - s_range_offset);
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_r_marker_samp(v)
     View *v;
{
  static char rv[10];

  if(v && v->sig) {
    sprintf(rv,"%d", time_to_sample(v->sig,v->rmarker_time));
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_tmarker_chan(v)
     View *v;
{
  static char rv[10];

  if(v) {
    sprintf(rv,"%d", v->tmarker_chan + c_range_offset);
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_bmarker_chan(v)
     View *v;
{
  static char rv[10];

  if(v) {
    sprintf(rv,"%d", v->bmarker_chan + c_range_offset);
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_cursor_label(v)
     View *v;
{
  static char rv[100];
  char *cp;
  List *l, *l2;

  if(v && v->sig && (l = v->sig->idents)) {
    int dim, curi, i;

    curi = v->cursor_channel;
    for(i = 0, l2 = l; i < curi; i++)
      if(l2->next)
	l2 = l2->next;

    strcpy(rv,l2->str);

    if((cp = strchr(rv,'[')))
      *cp = 0;

    return(rv);
  } else
    return("");
}
/*************************************************************************/
static char *view_get_cursor_element(v)
     View *v;
{
  static char rv[100];
  char *cp, *ce;
  List *l, *l2;

  if(v && v->sig && (l = v->sig->idents)) {
    int dim, curi, i;

    curi = v->cursor_channel;
    for(i = 0, l2 = l; i < curi; i++)
      if(l2->next)
	l2 = l2->next;

    strcpy(rv,l2->str);

    if((cp = strchr(rv,'['))) {
	if((ce = strchr(cp,']'))) 
		*ce = 0;
	return (cp+1);
    } else
	return "0";
  } else
    return("");
}

/*************************************************************************/
static char *view_get_cursor_chan(v)
     View *v;
{
  static char rv[10];

  if(v) {
    sprintf(rv,"%d", v->cursor_channel + c_range_offset);
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_range_time(v)
     View *v;
{
  static char rv[20];
  if(v) {
    sprintf(rv,"%s%6f:%6f", time_range_prefix, v->lmarker_time,v->rmarker_time);
    return(rv);
  }
  return("");
}

/*************************************************************************/
static char *view_get_range_samp(v)
     View *v;
{
  static char rv[20];
  if(v && v->sig) {
    sprintf(rv,"%s%d:%d",samp_range_prefix,
	    time_to_sample(v->sig,v->lmarker_time),
	    time_to_sample(v->sig,v->rmarker_time));
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_range_yval(v)
     View *v;
{
  static char rv[20];

  if(v && v->sig) {
    if(v->tmarker_yval < v->bmarker_yval)
      sprintf(rv,"%6f:%6f",v->tmarker_yval, v->bmarker_yval);
    else
      sprintf(rv,"%6f:%6f",v->bmarker_yval, v->tmarker_yval);
    return(rv);
  } else
    return("");
}

/*************************************************************************/
static char *view_get_range_chan(v)
     View *v;
{
  static char rv[20];
  if(v) {
    sprintf(rv,"%d:%d",v->bmarker_chan + c_range_offset,
	    v->tmarker_chan + c_range_offset);
    return(rv);
  }
  return("");
}

/*************************************************************************/
/* Returns the displayed element numbercounting from zero at bottom of
   display. */
view_invert_display_index(v,ind)
     View *v;
     int ind;			/* signal element number */
{
  if(v && (v->dims > 1)) {
    int i;

    if(isa_spectrogram_view(v))
      return(ind);

    for(i=0; i < v->dims; i++)
      if(v->elements[i] == ind)
	return(i);
  }
  return(0);
}

/*************************************************************************/
static char *view_get_cursor_channels(v)
     View *v;
{
  static char *rv = NULL;
  static char nch = 0;
  if(v && (v->dims >= 1)) {
    int is = view_invert_display_index(v,v->bmarker_chan),
        ie = view_invert_display_index(v,v->tmarker_chan), need = (ie-is+1) * 4;
    if(need > nch) {
      if(rv)
	free(rv);
      nch = 0;
      if(!(rv = malloc(need))) {
	fprintf(stderr,"Allocation problems in view_get_range_chan()\n");
	return("");
      }
      nch = need;
    }
    *rv = 0;
    if(!isa_spectrogram_view(v))
      for(; is <= ie; is++)
	sprintf(rv+strlen(rv),"%d ",v->elements[is] + c_range_offset);
    else
      for(; is <= ie; is++)
	sprintf(rv+strlen(rv),"%d ",is + c_range_offset);
    return(rv);
  }
  return("");
}

/*************************************************************************/
static char *
view_get_win_x(v)
    View *v;
{
    if (v && v->canvas) {
	static char   info[10];
	Frame frm = (Frame) xv_get(v->canvas, XV_OWNER);
	Rect  rec;

	frame_get_rect(frm, &rec);
	sprintf(info, "%d", rec.r_left);
	return(info);
    } else
	return("");
}

/*************************************************************************/
static char *
view_get_loc_x(v)
    View *v;
{
    if (v && v->canvas) {
	static char   info[10];
	Frame frm = (Frame) xv_get(v->canvas, XV_OWNER);
	Rect  rec;

	frame_get_rect(frm, &rec);
	sprintf(info, "%d", rec.r_left + *(v->x_offset));
	return(info);
    } else
	return("");
}

/*************************************************************************/
static char *
view_get_loc_y(v)
    View *v;
{
    if (v && v->canvas) {
	static char   info[10];
	Frame frm = (Frame) xv_get(v->canvas, XV_OWNER);
	Rect  rec;

	frame_get_rect(frm, &rec);
	sprintf(info, "%d", rec.r_top);
	return(info);
    } else
	return("");
}

/*************************************************************************/
static char *view_get_sec_cm(v)
     View *v;
{
  static char rv[10];

  if(v) {
    sprintf(rv, "%lf", *(v->x_scale));
    return(rv);
  }
  return("");
}

/*************************************************************************/
static char *view_get_val_cm(v)
     View *v;
{
  static char rv[10];
  if(v) {
    int i;
    for(*rv = 0, i = 0; i < v->dims; i++)
      if(v->elements[i] == v->cursor_channel) {
	sprintf(rv,"%lf",v->y_scale[i]);
	break;
      }
    return(rv);
  }
  return("");
}

/*************************************************************************/
static char *view_get_cursor_value(v)
     View *v;
{
  static char rv[10];
  double signal_get_value();

  if(v) {
    *rv = 0;
    if(v->sig) {
      sprintf(rv,"%.6e",
	      signal_get_value(v->sig,v->cursor_channel,time_to_index(v->sig,v->cursor_time)));
    }
    return(rv);
  }
  return("");
}

/*************************************************************************/
static char *view_get_cursor_labels(v)
     View *v;
{
  static char *rv = NULL;
  static int nrv = 0;
  int i;
  List *l, *l2;

  if(v && v->sig && (l = v->sig->idents)) {
    int needed = v->sig->dim * 15;
    if(needed > nrv) {
      if(rv)
	free(rv);
      nrv = 0;
      if(!(rv = malloc(needed))) {
	fprintf(stderr,"Allocation problems in view_get_cursor_labels()\n");
	return("");
      }
      nrv = needed;
    }
    *rv = 0;

    for(i = 0, l2 = l; i < v->sig->dim; i++) {
      sprintf(rv+strlen(rv),"%s ",l2->str);
      if(l2->next)
	l2 = l2->next;
    }

    return(rv);
  }
  return("");
}

/*************************************************************************/
static char *view_get_cursor_values(v)
     View *v;
{
  static char *rv = NULL;
  static int nrv = 0;
  double signal_get_value();
  int i;

  if(v && v->sig) {
    int ch, ind = time_to_index(v->sig, v->cursor_time), needed = v->sig->dim * 15;
    if(needed > nrv) {
      if(rv)
	free(rv);
      nrv = 0;
      if(!(rv = malloc(needed))) {
	fprintf(stderr,"Allocation problems in view_get_cursor_values()\n");
	return("");
      }
      nrv = needed;
    }
    *rv = 0;

    for(i = 0; i < v->sig->dim; i++)
      sprintf(rv+strlen(rv),"%.6e ",signal_get_value(v->sig,i,ind));
    return(rv);
  }
  return("");
}

/*************************************************************************/
static char *view_get_view_channels(v)
     View *v;
{
  static char *rv = NULL;
  static int nrv = 0;
  int i;

  if(v && (v->dims > 1)) {
    int needed = v->dims * 5;
    if(needed > nrv) {
      if(rv)
	free(rv);
      nrv = 0;
      if(!(rv = malloc(needed))) {
	fprintf(stderr,"Allocation problems in view_get_view_channels()\n");
	return("");
      }
      nrv = needed;
    }
    *rv = 0;
    for(i=0; i < v->dims; i++)
      sprintf(rv+strlen(rv),"%d ",v->elements[i] + c_range_offset);

    return(rv);
  } else
    return("0");
}

/*************************************************************************/
static char *view_get_view_end_time(v)
     View *v;
{
  static char rv[10];
  if(v) {
    sprintf(rv,"%lf",ET(v));
    return(rv);
  }
  return("");
}

/*************************************************************************/
static char *view_get_computed_value(v, name)
     View *v;
     char *name;
{
  Activator *a = &vb0;
  
  while(a) {
    if(!strcmp(name, a->name))
      return(a->proc(v));
    a = a->next;
  }
  return(NULL);
}

/*************************************************************************/
char *view_get_value(v, name)
     char *name;
     View *v;
{
  if( name && *name) {
    char *rv;

    if((rv = view_get_computed_value(v, name)))
      return(rv);
    else {
      Selector *vs = view_setup_access(v);
      if((rv = value_as_string(vs, name)))
	return(rv);
      else {
	extern Selector g1;
	if((rv = value_as_string(&g1, name)))
	  return(rv);
      }
    }
  }
  return("null");
}

/*************************************************************************/
/* This destroys all views of this signal that are NOT in this signal's
   view list,  but refer to it. */
View *find_and_destroy_overlay_views(s)
     Signal *s;
{
  View *vr = NULL;

  if(s && s->obj) {
    Signal *so = ((Object*)s->obj)->signals;
    while(so) {
      if(so != s) {
	View *v = so->views, *v2;
	while(v) {
	  if((v->sig == s) &&	/* found a view to be destroyed == v */
	    (v->extra_type == VIEW_OVERLAY) && v->extra) {
	    vr = (View*)v->extra;
            v2 = so->views;
	    
	    if(v2 == v)
	      so->views = v->next;
	    else
	      while(v2) {
		if(v2->next == v)
		  v2->next = v2->next->next;
		v2 = v2->next;
	      }
	    v2 = v->next;
	    v->sig = NULL;
	    v->next = NULL;
	    free_view(v);
	    v = v2;
	  } else
	    v = v->next;
	}
      }
      so = so->others;
    }
  }
  return(vr);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Return the "active element" number whose identification string matches
   str.  If a channel label can not be found to match str, -1 is returned.
   */
get_active_labeled_chan(v,str)
     View *v;
     char *str;
{
  Signal *s = v->sig;
  register List *l, *l2;

  if(s && (l = s->idents)) {
    register int curi, i, dim;
      
    for(dim=0; dim < v->dims; dim++) { /* for each active dimension... */
      curi = v->elements[dim];
      for(i = 0, l2 = l; i < curi; i++) { /* get label for this chan. */
	if(l2->next)
	  l2 = l2->next;
      }				/* falls through with l2 set */
      if(!strcmp(l2->str,str)) return(dim);
    }
  }
  return(-1);
}

/*********************************************************************/
char *
get_channels_name(v,n)
     register View *v;
     register int n;
{
  register List *l;

  if(v && v->sig && (n < v->sig->dim) && (l = v->sig->idents)) {
    while((n-- > 0) && l->next) l = l->next;
    return(l->str);
  }
  return(NULL);
}

/*********************************************************************/
clear_active_channels(v)
     View *v;
{
  extern char active_ids[], active_numbers[];
  int i, dim;
  Signal *s;
  char *cp, name[50], *get_next_item();
  
  if(v && (s = v->sig)) {
    cp = active_ids;
    while(*cp) {
      sscanf(cp,"%s",name);
      if((i = get_active_labeled_chan(v,name)) >= 0) {
	for(dim=i+1; (v->dims > 1) && (dim < v->dims); dim++)
	  v->elements[dim-1] = v->elements[dim];
	v->dims--;
      }
      cp = get_next_item(cp);
    }
    cp = active_numbers;
    while(*cp) {
      sscanf(cp,"%d",&dim);
      if(dim < s->dim) {
	for(i=0; i < v->dims; i++)
	  if(v->elements[i] == dim) {
	    for(dim=i+1; (v->dims > 1) && (dim < v->dims); dim++)
	      v->elements[dim-1] = v->elements[dim];
	    v->dims--;
	    break;
	  }
      } 
      cp = get_next_item(cp);
    }

    *active_numbers = *active_ids = 0;
    for(i=0, cp = active_numbers; i < v->dims; i++) {
      sprintf(cp," %d",v->elements[i]);
      cp = active_numbers + strlen(active_numbers);
    }
  }
}

/*********************************************************************/
add_active_channels(v)
     View *v;
{
  extern char active_ids[], active_numbers[];
  int i, dim;
  Signal *s;
  char *cp, name[50];
  
  if(v && (s = v->sig)) {
    if(*active_ids && s->idents)
      for(i=0; i < v->dims; i++) {
	strcat(active_ids," ");
	strcat(active_ids,get_channels_name(v,v->elements[i]));
      }

    if(*active_numbers)
      for(i=0; i < v->dims; i++) {
	cp = active_numbers + strlen(active_numbers);
	sprintf(cp," %d",v->elements[i]);
      }
    
    set_active_channels(v);
  }
}

/*********************************************************************/
set_active_channels(v)
     View *v;
{
  extern char active_ids[], active_numbers[];
  int i, dim;
  Signal *s;
  char *cp, name[50], *get_next_item();
  
  if(v && (s = v->sig) && (*active_ids || *active_numbers)) {

    if(isa_spectrogram_view(v))
      return;

    for(i=0; i < s->dim; i++) v->elements[i] = 0;

    cp = active_ids;
    while(*cp) {
      sscanf(cp,"%s",name);
      if((i = get_labeled_chan(s,name)) >= 0) v->elements[i] = 1;
      cp = get_next_item(cp);
    }

    cp = active_numbers;
    while(*cp) {
      sscanf(cp,"%d",&i);
      if((i >= 0) && (i < s->dim)) v->elements[i] = 1;
      cp = get_next_item(cp);
    }

    for(i=0, v->dims = 0; i < s->dim; i++) {
      if(v->elements[i])
	v->elements[v->dims++] = i;
    }

    if(! v->dims) {   /* activate all, if none were correctly specified */
      for(i=0; i < s->dim; i++) v->elements[i] = i;
      v->dims = s->dim;
    }
  }
}

/*********************************************************************/
void operate_wave_scrollbar(v, event)
     View *v;
     Event *event;
{
  int id = event_id(event);
  switch (id) {
  case MS_LEFT:
  case MS_MIDDLE:
  case MS_RIGHT:
    if(event_is_down(event)) {
      operate_scrollbar(id, event_x(event), v);
    }
    break;
  default:
    break;
  }
  file_print_x(v,event_x(event));
}

/*********************************************************************/
generic_time_to_x(v,time)
     register View *v;
     register double time;
{
  register int i;
  register double freq, t;

  freq = v->sig->freq;
  /* "integerize" time */
  i = 0.5 + ((time - v->sig->start_time) * freq);
  time = v->sig->start_time + ((double)i)/freq;
  if (time < v->start_time) time = v->start_time; /* clamp */
  if (time > (t = ET(v)))   time = t;
  return((int)(.5 + (PIX_PER_CM *
     ((time - v->start_time) / (*v->x_scale)))) + (*v->x_offset));
}
	 
/*********************************************************************/
generic_yval_to_y(v, yval)
    register View   *v;
    register double yval;
{
    register int    i;
    for(i=0; i < v->dims; i++)
      if(v->cursor_channel == v->elements[i]) {
	if (yval < v->start_yval) yval = v->start_yval;
	if (yval > v->end_yval)   yval = v->end_yval;
	yval -= v->start_yval;
	i = (v->y_offset[i]) - (int) (0.5 + yval * PIX_PER_CM / (v->y_scale[i]));
	return i;
      }
    return(0);
}
	 
/*********************************************************************/
double generic_x_to_time(v,x)
     register View *v;
     register int x;
{
  register int i;
  register double time, freq, t;

  freq = v->sig->freq;
  x -= *(v->x_offset);
  if(x < 0) x = 0;
  time = v->start_time + (double)x * *(v->x_scale) / PIX_PER_CM;
  if (time > (t = ET(v)))   time = t;
  i = 0.5 + ((time - v->sig->start_time) * freq);
  return(v->sig->start_time + ((double)i)/freq);
}

/*********************************************************************/
double generic_y_to_yval(v, y)
     register View   *v;
     int    y;
{
  double val;
  register int i, chan = v->cursor_channel;

  for(i=0; i < v->dims; i++)
    if(chan == v->elements[i]) {
      val = (v->y_offset[i] - y) * v->y_scale[i] / PIX_PER_CM;
      return( (val < v->sig->smin[chan]) ? v->sig->smin[chan]
	: ((val > v->sig->smax[chan]) ? v->sig->smax[chan]
	  : val));
    }
  return(0.0);
}

/*********************************************************************/
double nonlin_y_to_yval(v, y)
    View    *v;
    int	    y;
{
    double  *y_dat, u;
    int	    d, i;

    y_dat = v->sig->y_dat;
    d = v->sig->dim - 1;

    u = d*(*v->y_offset - y)*(*v->y_scale)
	/ (PIX_PER_CM*(v->end_yval - v->start_yval));

    if (u < 0.0) return y_dat[0];
    i = u;
    if (i >= d) return y_dat[d];
    u -= i;
    return y_dat[i] + u*(y_dat[i+1] - y_dat[i]);
}

/*********************************************************************/
generic_x_to_index(v, x)
     View *v;
     int x;
{
  Signal *s;
  
  if(v && (s = v->sig)) {
    double time = (v->x_to_time)? v->x_to_time(v,x) :
                                  generic_x_to_time(v,x);

    return((s->utils->time_to_index)? s->utils->time_to_index(s,time) :
	                       vec_time_to_index(s,time));
  }
  return(0);
}

/*********************************************************************/
/* Given a view v, a buffer index, ind, a crossing level val, and a
channel, chan, find the nearest positive level crossing within the
view. */
int level_crossing_index(v, ind, val, chan)
     int ind, chan;
     View *v;
     double val;
{
  if(v) {
    int is, ie, i, j, k;
    double oldv, dv, signal_get_value();
    Signal *s;

    s = v->sig;
    is = time_to_index(s,v->start_time);
    ie = time_to_index(s,ET(v));
    if((is <= ind) && (ie >= ind)) {
      if(is < ind) {
	i = ind;
      } else {
	i = ind + 1;
      }
      oldv = signal_get_value(s, chan, i-1);
      for(j = -1 ; i <= ie; i++) {
	if(((dv = signal_get_value(s, chan, i)) >= val) && (oldv < val)) {
	  j = i;
	  break;
	}
	oldv = dv;
      }
      if(ie > ind)
	i = ind;
      else
	i = ind-1;
      
      oldv = signal_get_value(s, chan, i);

      for(k = -1; i > is; i--) {
	if(((dv = signal_get_value(s, chan, i-1)) < val) && (oldv >= val)) {
	  k = i;
	  break;
	}
	oldv = dv;
      }

      if(k >= 0) {
	if((j >= 0) && ((j-ind) < ind - k))
	  return(j);
	else
	  return(k);
      } else
	if(j >= 0)
	  return(j);
	else
	  return(ind);
    }
  }
  return(ind);
}


/*********************************************************************/
/* Given a view v, view Xposition x, a crossing level val, and a
channel, chan, find the time of the nearest positive level crossing within the
view and return the corresponding time. */
double level_crossing_time(v, x, val, chan)
     int x, chan;
     View *v;
     double val;
{
  if(v) {
    int newind, ind;

    ind = generic_x_to_index(v, x);
    newind = level_crossing_index(v, ind, val, chan);
    return(v->sig->utils->index_to_time(v->sig, newind));
  }
  return(0.0);
}

/*********************************************************************/
int generic_ord_to_y(v, val)
     register    View    *v;
    double  val;
{
  register int chan = v->cursor_channel, i;

  if (val < v->sig->smin[chan])
    val = v->sig->smin[chan];
  else
    if (val > v->sig->smax[chan])
      val = v->sig->smax[chan];

  for(i=0; i < v->dims; i++)
    if(v->elements[i] == chan) {
      val *= PIX_PER_CM / (v->y_scale[i]);
      return(v->y_offset[i] - ROUND(val));
    }
}

/*********************************************************************/
#define DIST(d) \
	((!(d)) ? -1 : \
	 iabs(y - (v->y_offset[i] - (int)((d)[ind]*PIX_PER_CM/v->y_scale[i]))))
generic_xy_to_chan(v,x,y)
     register View *v;
     int x, y;
{
  register Signal *s;

  if(v && (s = v->sig)) {

    if(v->dims == 1) return(v->elements[0]); /* special-case for speed */
    else {
      int i, mini, mind, j, ind;
      double time = (v->x_to_time)? v->x_to_time(v, x) :
	generic_x_to_time(v, x);

      if (!s->data)
      {
	  fprintf(stderr,
		  "NULL signal data array in generic_xy_to_chan.\n");
	  return 0;
      }

      ind = time_to_index(s, time);
      mind = 5000;   /* needs to be bigger than biggest y screen in pixels */
      mini = 0;
      for (i = 0; i < v->dims; i++) {

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
		      "s->types null for P_MIXED in generic_xy_to_chan\n");
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
		    "NULL signal data in generic_xy_to_chan.\n");
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
    fprintf(stderr,"Bad args to generic_xy_to_chan %x\n", v);
  return(0);
}
      
/*********************************************************************/
plot_view(v)
     View *v;
{
  Frame fr;

  if(v->canvas && (fr = (Frame)xv_get(v->canvas,XV_OWNER)) &&
     !xv_get(fr, FRAME_CLOSED)) {
    if (v->set_scale)   v->set_scale(v);
    if (v->data_plot) {
      v->data_plot(v);
      v->plotted = TRUE;
    }
    if (v->reticle_plot)	v->reticle_plot(v);
    if (v->x_print)	v->x_print(v);
    if (v->y_print)	v->y_print(v);
    if (v->cursor_plot)  {
      v->cursor_plot(v,CURSOR_COLOR); /* do frequent x-or ops last */
      view_xor_marks(v);
    }
    if (v->vmarker_plot)	v->vmarker_plot(v,2);
    if (v->hmarker_plot)	v->hmarker_plot(v,2);
    /* update scrollbar for new configuration */
    set_scrollbar(v);
    plot_scrollbar(v->scrollbar);
  }
}

/*********************************************************************/
/* Search for view referring to a signal, not necessarily on signal's
 * own view list (e.g. overlays).
 */
View *find_view(sig)
    Signal  *sig;
{
    Object  *o;
    Signal  *s;
    View    *v;

    if (sig->views) return sig->views;
    else if ((o = (Object *) sig->obj))
	for (s = o->signals; s; s = s->others)
	    for (v = s->views; v; v = v->next)
		if (v->sig == sig) return v;
    return NULL;
}

/*********************************************************************/
/* Get a segment of signal.  Adjust view start and end times to match
segment actually obtained.  Return TRUE if the viewable segment
actually changed (i.e. if redisplay is necessary).  Clip start and
page_time, if necessary. */

#define FUZZ 0.000001

get_view_segment(view, start, page_time)
     register View *view;
     register double *start, *page_time;
{
  double end, resolution;
  int     wasopen, redo;
  double  buf_start, buf_end;

  end = *start + *page_time;
  redo = FALSE;
  resolution = *view->x_scale/PIX_PER_CM; /* display time resolution per pixel */

  /* Clip requested interval to lie within the signal. */
  if (*start < view->sig->start_time) { /* does this seem clumsey? */
    *start = view->sig->start_time;
    end = *start + *page_time;
  }
  if (end > SIG_END_TIME(view->sig)) {
    end = SIG_END_TIME(view->sig);
    *start = end - *page_time;
  }
  if (*start < view->sig->start_time) /* must duration be clipped? */
    *start = view->sig->start_time;

  *page_time = end - *start;	/* return corrected page time */

  while(view) {
    if((SIG_END_TIME(view->sig) > *start) && (view->sig->start_time < end)) {
      /* If either view->start_time or ET(view) changes, redisplay must be done. */
      if(view->start_time != *start) {
	if((view->sig->freq == 0.0) || ((view->sig->freq > 0.0) &&
			(fabs(*start - view->start_time) >= resolution)))
	  redo = TRUE;
	view->start_time = *start;
      }
      if((ET(view) != end) &&
	 ((view->sig->freq == 0.0) || ((view->sig->freq > 0.0) &&
				       (fabs(ET(view) - end) >= resolution)))) {
	redo = TRUE;
      }
      /* if region not already in buffer, read data from file */
      buf_start = BUF_START_TIME(view->sig);
      buf_end = BUF_END_TIME(view->sig);
      if ((view->sig->file != SIG_NEW)
	  && (*start < buf_start - fabs(FUZZ*buf_start)
	      || end > buf_end + fabs(FUZZ*buf_end))) {
	wasopen = (view->sig->file >= 0);
	view->sig->utils->read_data(view->sig, *start, end - *start);

	if (view->sig->buff_size == 0) {
	  sprintf(notice_msg,
                 "%s: No records read (requested segment length %g sec).\n",
		 "get_view_segment", end - *start);
	  show_notice(1,notice_msg);
        }
	
	if(!wasopen)
	  close_sig_file(view->sig);
	if((view->sig->type & SPECIAL_SIGNALS) != SIG_SPECTROGRAM) /* for speed... */
	  get_maxmin(view->sig);
	redo = TRUE;
      }
    }
    view = view->next;
  }
  return(redo);		/* return "redisplay is required" boolean */
}

#undef FUZZ

/*********************************************************************/
view_fit_duration(v)
     View* v;
{
  if(v && v->canvas && v->sig) {
    double page_time = ((double)(v->sig->buff_size))/v->sig->freq;
    *(v->x_scale) = PIX_PER_CM * page_time /
      (v->width - *(v->x_offset));
  }
}

/*********************************************************************/
position_view(view,start,end)
  View *view;
  double start, end;
{
  double page_time;
  
  page_time = end - start;
  if (start >= end) return;
  get_view_segment(view, &start, &page_time);
  *(view->x_scale) = PIX_PER_CM * page_time /
    (view->width - *(view->x_offset));
  redoit(view->canvas);
}

/*********************************************************************/
page(view,sig,start)
  View *view;
  Signal *sig;
  double start;
{
  double page_time;
  Signal *sig0 = sig;
  extern int scroll_ganged;

  page_time = ET(view) - view->start_time;
  if(get_view_segment(view, &start, &page_time))
    redoit(view->canvas);

  if(scroll_ganged) {
    sig = ((Object*)sig0->obj)->signals;
    while(sig) {
      if((sig0 != sig) && (view = sig->views)) {
	start = sig0->views->start_time; /* borrow new start time */
	page_time = ET(view) - view->start_time;
	if(get_view_segment(view,&start,&page_time))
	  redoit(view->canvas);
      }
      sig = sig->others;
    }
  }
}

/*********************************************************************/
void
zoom_view(v, time, ratio)
    View    *v;
    double  time, ratio;
{
  Signal *s, *so;
  extern int zoom_ganged;
  double ptime;		/* start time & page duration */
  double start;
  extern int h_spect_rescale;

  /* current time (e.g. of cursor) remains fixed; endpoints float */
  if (!v || !(s = v->sig)) return;
  if (v->h_rescale) {
    start = time - (time - v->start_time)*ratio;
    ptime = (ET(v) - v->start_time)*ratio;
    if(get_view_segment(v, &start, &ptime)) {
      *(v->x_scale) = PIX_PER_CM * ptime / (v->width - *(v->x_offset));
      redoit(v->canvas);
    }
  }

  if(zoom_ganged)  /* zoom all the other views in the object (if possible) */
    zoom_others(s, *(v->x_scale), v->start_time, ET(v));

}

/*************************************************************************/
zoom_others(s, x_scale, startr, endr)
    Signal  *s;
    double  x_scale, startr, endr;
{
  Signal *so;
  View *view;
  double ptime, start, end;
  extern int h_spect_rescale;

  so = ((Object*)s->obj)->signals;
  while(so) {
    if((so != s) && (view = so->views)) {
      start = startr;		/* borrow new start time and scale*/
      ptime =   x_scale * (view->width - *(view->x_offset))/PIX_PER_CM;
      if (view->h_rescale) {
	get_view_segment(view, &start, &ptime);
	*(view->x_scale) = PIX_PER_CM * ptime /
	  (view->width - *(view->x_offset));
	redoit(view->canvas);
      } else {
	if(((start < view->start_time) && (start >= so->start_time)) ||
	   (((end = start + ptime) > endr) && (end <= so->end_time))) {	/* should use SIG_END_TIME() */
	   if(get_view_segment(view, &start, &ptime))
	     redoit(view->canvas);
	 }
      }
    }
    so = so->others;
  }
}

/*********************************************************************/
do_align_views(obj, v)
    Object	*obj;
    View	*v;
{
  Signal	*s;
  View		*view_to_match = v;
  int		maxoff;
  struct rect	r;
  Frame		frm;

  /* See if the view needs to be moved to accommodate views with greater
     x_offset. */

  for (s = obj->signals, maxoff = *v->x_offset; s; s = s->others)
    if ((v = s->views) && (v->start_time < ET(view_to_match)) &&
	(ET(v) > view_to_match->start_time)) 
      if (*v->x_offset > maxoff) maxoff = *v->x_offset;

  frm = (Frame) xv_get(view_to_match->canvas, XV_OWNER);
  frame_get_rect(frm,&r);
  maxoff += WMGR_MARGIN - r.r_left - *view_to_match->x_offset;

/*!*//* DEBUG */
  if (debug_level >= 2)
      fprintf(stderr,
	      "do_align_views: r_top %d\tr_left %d\tr_width %d\tr_height %d.\n",
	      r.r_top, r.r_left, r.r_width, r.r_height);

  if (maxoff > 0) {
    int trim;

    trim = r.r_left + maxoff + r.r_width + WMGR_MARGIN - SCREEN_WIDTH;
    if (trim < 0) trim = 0;

	/* The align_view's in the loop below are (or may be) done
	   before the resize triggered by the xv_set, and align_view
	   refers to the width of the model view.  So set this value now. */

    view_to_match->width = r.r_width - trim - 2*FRAME_MARGIN;
    xv_set(frm,
		XV_X,	    r.r_left + maxoff,
		XV_WIDTH,   r.r_width - trim,
		0);
  }

  for (s = obj->signals; s; s = s->others)
    for (v = s->views; v; v = v->next)
      if (v != view_to_match)
	align_view(v,view_to_match);
}

/*********************************************************************/
align_view(v, vmodel)
    View	*v, *vmodel;
{
  /* Goal: change screen location, x-scale and view times so that the curor in
   * ``v'' is horizontally aligned on the screen with the cursor in the model.
   * The window will be narrowed if required by scale and signal end time.
   */
  /* There are two major cases: views that can change horizontal scale,
     and those that can't. */

  Signal	*s = v->sig;
  Frame		frm = (Frame)xv_get(v->canvas,XV_OWNER);
  Frame		frmmodel = (Frame)xv_get(vmodel->canvas,XV_OWNER);
  extern int h_spect_rescale;
  struct rect	r;
  double	x_scale = PIX_PER_CM / (*vmodel->x_scale); /* pixels/sec */
  double	dt, start, page, end;
  int		x_scalable = v->h_rescale;
  int		offset = *v->x_offset,
		m_left, m_right, x_loc, x_end, v_left, gotnew, v_width;

#define ET_PROPOSED(v) \
    ((v)->start_time + (*(v)->x_scale * \
          (x_end - x_loc - 2*FRAME_MARGIN - *(v)->x_offset) / PIX_PER_CM))

  if((end = ET(vmodel)) > BUF_END_TIME(vmodel->sig))
    end = BUF_END_TIME(vmodel->sig);
  start = vmodel->start_time;
  page = end - start;

  /* check for temporal overlap of signals.  */
  if (v->extra_type == VIEW_OVERLAY
      || s->start_time > end || SIG_END_TIME(s) < start)
    return(TRUE);	/* Views have nothing in common! */

    /* Get screen location and width of model and ancillary views. */
  frame_get_rect(frmmodel, &r);
  m_left = r.r_left;		/* right and left model bounds */
  m_right = m_left + r.r_width;
  frame_get_rect(frm, &r);
  v_left = r.r_left;		/* current right and left view bounds */
  v_width = r.r_width;
  x_loc = m_left + *vmodel->x_offset - *v->x_offset; /* need to move here */
  x_end = m_right; /* assumes zero-width rh border */

  if(! x_scalable) {	    /* spectrogram (or other unscalable view)? */
    if((start < v->start_time) ||
       (end > ET(v)))	/* If it's all on screen, don't scroll */
      gotnew = get_view_segment(v,&start,&page);
    else
      gotnew = FALSE;

  /* Window of ancillary view is moved only if necessary to align its
     left edge or BUF_START or BUF_END with model: */

    if((dt = start - vmodel->start_time) > 0.0)
      x_loc += (0.5 + (dt * x_scale));	/* match left edges (starting times) */

    if((end > BUF_END_TIME(s)) || (ET_PROPOSED(v) > BUF_END_TIME(s)))
      x_end = x_loc
	      + (0.5 + (BUF_END_TIME(s)-start) * PIX_PER_CM/(*v->x_scale))
	      + *v->x_offset + 2*FRAME_MARGIN;
  
    if ((x_loc != v_left) || (x_end != v_left + v_width)) {
      v->start_time = start;

      xv_set(frm,
		XV_WIDTH,   x_end - x_loc,
		XV_X,	    x_loc,
		0);
      /* xv_set() forces a repaint() as part of the resize sequence ... */
    }

    if (gotnew && x_end - x_loc == v_width)	/* ... if the size changes. */
      redoit(v->canvas);

  } else {			/* view IS time-scalable */

    gotnew = get_view_segment(v,&start,&page); /* Get as much as possible. */

    /* Window of ancillary view is moved only if necessary to align its
      left edge or BUF_START or BUF_END with model: */

    *v->x_scale = *vmodel->x_scale;

    if((dt = start - vmodel->start_time) > 0.0)
      x_loc += (0.5 + (dt * x_scale));

    if((end > BUF_END_TIME(s)) || (ET_PROPOSED(v) > BUF_END_TIME(s)))
      x_end = x_loc
	      + (0.5 + (BUF_END_TIME(s)-start) * x_scale)
	      + *v->x_offset + 2*FRAME_MARGIN;

    if ((x_loc != v_left) || (x_end != v_left + v_width)) {
      v->start_time = start;

      v->width = x_end - x_loc - 2*FRAME_MARGIN; /* prevents auto-rescale */

      xv_set(frm,
		XV_WIDTH,   x_end - x_loc,
		XV_X,	    x_loc,
		0);
      /* xv_set() forces a repaint() as part of the resize sequence ... */
    }

    if (gotnew && x_end - x_loc == v_width)
      redoit(v->canvas);
  }

  return(TRUE);
}

