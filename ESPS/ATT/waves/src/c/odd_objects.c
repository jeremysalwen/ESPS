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
 *  odd_object.c
 *  implementation of objects to perform specialized system functions
 */

static char *sccs_id = "@(#)odd_objects.c	1.5	12/6/95	ATT/ERL";

#include <Objects.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/unix.h>
#include <xview/scrollbar.h>

extern void set_all_view_attributes(), set_wave_view_attributes(),
       set_spect_view_attributes();

static Menuop
  q1 =  {"zoom_ratio", set_all_view_attributes, "zoom_ratio", NULL},
  q2a = {"ref_step", set_all_view_attributes, "page_step", &q1},
  q2 = {"cross_level", set_all_view_attributes, "cross_level", &q1},
  q3 = {"invert_dither", set_all_view_attributes, "invert_dither", &q2},
  q4b = {"move_op", set_wave_view_attributes, "move_op", &q3},
  q4 = {"middle_op", set_wave_view_attributes, "middle_op", &q4b},
  q5 = {"left_op", set_wave_view_attributes, "left_op", &q4},
  q6 = {"spec_middle_op", set_spect_view_attributes, "middle_op", &q5},
  q7 = {"spec_left_op", set_spect_view_attributes, "left_op", &q6},
  q8 = {"scrollbar_height", set_all_view_attributes, "scrollbar_height", &q7},
  q9 = {"readout_bar_height", set_all_view_attributes, "readout_bar_height", &q8},
  q10 = {"line_type", set_all_view_attributes, "line_type", &q9},
  q11 = {"show_vals", set_all_view_attributes, "show_vals", &q10},
  q12a = {"show_current_chan", set_all_view_attributes, "show_current_chan", 
		&q11},
  q12b = {"show_labels", set_all_view_attributes, "show_labels", &q12a},
  q12 = {"find_crossing", set_all_view_attributes, "find_crossing", &q12b},
  q13 = {"reticle_grid", set_all_view_attributes, "reticle_on", &q12},
  q14 = {"spect_interp", set_spect_view_attributes, "spect_interp", &q13},
  q15 = {"h_spect_rescale", set_spect_view_attributes, "h_rescale", &q14},
  q16 = {"v_spect_rescale", set_spect_view_attributes, "v_rescale", &q15},
  q17 = {"spect_rescale_scope", set_spect_view_attributes, "rescale_scope", &q16},
  q18 = {"plot_min", set_wave_view_attributes, "plot_min", &q17},
  q19 = {"plot_max", set_wave_view_attributes, "plot_max", &q18},
  q20 = {"shorten_header", set_all_view_attributes, "shorten_header", &q19};

change_objects_globally(changes)
     Selector **changes;
{
  extern Object program;
  char buildit[MES_BUF_SIZE];
  Menuop *mo, *find_operator();

  if(changes) {
    while(*changes) {
      if(( mo = find_operator(&q20, (*changes)->name))) {
	  Object *o = program.next;
	  while(o) {
	    sprintf(buildit,"%s %s", mo->data,
		    value_as_string(*changes, (*changes)->name));
	    mo->proc(o, buildit);
	    o = o->next;
	  }
	}
      changes++;
    }
  }
}

char *checking_selectors(str, sl)
     char *str;
     Selector *sl;
{
  static char *oblock = NULL;
  static int oblocksize = 0;
  char tmp[100];

  if(str && (*str == '?')) {
    char *cp;
    int used = 0;

    if(oblock)
      *oblock = 0;
    while(sl) {
      if((cp = sl->name) && *cp) {
	if( ! used )
	  block_build(&oblock,&used,&oblocksize,"returned ");
	sprintf(tmp,"%s ", cp);
	block_build(&oblock,&used,&oblocksize, tmp);
      }
      sl = sl->next;
    }
    if(oblock && *oblock)
      return(oblock);
    else
      return("null");
  }
  return(NULL);
}

char *checking_activators(str, sl)
     char *str;
     Menuop *sl;
{
  static char *oblock = NULL;
  static int oblocksize = 0;
  char tmp[100];

  if(str && (*str == '?')) {
    char *cp;
    int used = 0;

    if(oblock)
      *oblock = 0;
    while(sl) {
      if((cp = sl->name) && *cp) {
	if( ! used )
	  block_build(&oblock,&used,&oblocksize,"returned ");
	sprintf(tmp,"%s ", cp);
	block_build(&oblock,&used,&oblocksize, tmp);
      }
      sl = sl->next;
    }
    if(oblock && *oblock)
      return(oblock);
    else
      return("null");
  }
  return(NULL);
}
    
char **get_attr_list(com, type)
     char *com, *type;
{
  char **mb = NULL, **sort_a_list();
  
  if(com && *com) {
    Methods *get_methods_list(), *m = get_methods_list(type);
    char *res, *get_next_item(), *savestring();

    while(m) {
      if(m->name && m->meth && !strcmp(com,m->name)) {

	res = m->meth(NULL, "?");
	if(res && *res && !strncmp(res,"returned",strlen("returned"))) {
	  char *cp, tb[100];
	  int len = 0;

	  if(!(res = get_next_item(res)) && (res && *res))
	    res = "<NONE>";

	  cp = res;
	  while(cp && *cp) {
	    len++;
	    cp = get_next_item(cp);
	  }
	  if(!(mb = (char**)malloc(sizeof(char*)*(len+1)))) {
	    fprintf(stderr,"Allocation problems in get_attr_list()\n");
	    return(NULL);
	  }
	  cp = res;
	  len = 0;
	  while(cp && *cp) {
	    sscanf(cp, "%s", tb);
	    if(!(mb[len] = savestring(tb))) {
	      fprintf(stderr,"Allocation problems (2) in get_attr_list()\n");
	      return(NULL);
	    }
	    cp = get_next_item(cp);
	    len++;
	  }
	  mb[len] = NULL;
	  return(sort_a_list(mb));
	}
      }
      m = m->next;
    }
  }
  return(NULL);
}

char **get_vc_attr_list(com)
     char *com;
{
  return(get_attr_list(com,"view"));
}
	 

free_string_list(sl)
     char **sl;
{
  char **slp = sl;
  if(sl) {
    while(*slp) {
      free(*slp);
      slp++;
    }
    free(sl);
  }
}
