/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin
 *
 */

static char *sccs_id = "@(#)labelprocs.c	1.9	9/26/95	ATT/ERL";

#ifndef hpux
#include <sys/param.h>
#else
#define MAXPATHLEN 1024
#endif
#include <Objects.h>
#include <labels.h>
#ifndef hpux
#include <sys/param.h>
#else
#define MAXPATHLEN 1024
#endif

#define DELETE '\177'

#if !defined(hpux) && !defined(DS3100) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
char *malloc();
#endif
Objects *objlist = NULL;
#ifdef FOR_XVIEW
Pixfont *get_default_label_font();
#endif

/************************************************************************/
Labelfile *
find_labelfile(o,file)
     Objects *o;
     char *file;
{
  Labelfile *lf;

  if(o && file && *file) {
    lf = o->labels;
    while(lf)
      if(!strcmp(lf->label_name,file))
	return(lf);
      else
	lf = lf->next;
  }
  return(NULL);
}
  
/************************************************************************/
char *
make_label_filename(name)
char *name;
{
  char *p;

  if(name && *name) {
    if(!(p = malloc(MAXPATHLEN))) {
      printf("Can't allocate a label file name\n");
      return(NULL);
    }
    sprintf(p,"%s.lab",name);
    return(p);
  }
  return(NULL);
}
      
  
/************************************************************************/
Labelfile *
get_labelfile(ob, name)
     register Objects *ob;
     register char *name;
{
  register Labelfile   *lf;

  if(name && *name && ob && (lf = ob->labels)) {
    while(lf) {
      if(!strcmp(lf->label_name,name)) return(lf);
      lf = lf->next;
    }
  }
  return(NULL);
}

/************************************************************************/
free_label(l)
     Label *l;
{
  Fields *f, *ft;

  if(l) {
    f = l->fields;
    while(f) {
      if(f->str) free(f->str);
      ft = f->next;
      free(f);
      f = ft;
    }
    free(l);
  }
}

/************************************************************************/
destroy_labelfile(o,lf,send_to_host)
     Objects *o;
     Labelfile *lf;
     int send_to_host;
{
  Label *l, *l2;
  Labelfile *lf2;

  if((lf2 = o->labels) == lf)
    o->labels = lf->next;
  else {
    while(lf2 && (lf2->next != lf)) lf2 = lf2->next;
    if(!lf2) return(FALSE);	/* lf is not part of o! */
    lf2->next = lf->next;
  }
  if((l = lf->first_label)) {
    while(l) {
      if(send_to_host)
	waves_send(o->name, "unmark", l->color, l->time);
      l2 = l;
      l = l->next;
      free_label(l2);
    }
  }
  if(lf->sig_name)
    free(lf->sig_name);
  if(lf->label_name)
    free(lf->label_name);
  if(lf->comment)
    free(lf->comment);
#ifdef FOR_XVIEW
  if(lf->menu)
    menu_destroy(lf->menu);
#endif
  /* should free font resources, if used... */

  free(lf);
  return(TRUE);
}

/************************************************************************/
kill_object(o,send_to_host)
     Objects *o;
     int send_to_host;
{
  Label *l, *l2;
  Labelfile *lf, *lf2;
  Objects *o2;
  extern Objects *objlist;

  if(o) {
    lf = o->labels;
    while(lf) {
      lf2 = lf->next;
      rewrite_labels(lf);
      destroy_labelfile(o,lf,send_to_host);
      lf = lf2;
    }
    if((o2 = objlist) == o)
      objlist = o->next;
    else
      while(o2->next) {
	if(o2->next == o) {
	  o2->next = o->next;
	  break;
	}
	o2 = o2->next;
      }
    if(o->name) free(o->name);
    kill_wobject(get_wobject(o));
    free(o);
    return(TRUE);
  }
  return(FALSE);
}
    
	
/************************************************************************/
Label *
new_label(time, color)
     double time;
     int color;
{
  Label *l;

  if(!(l = (Label*)malloc(sizeof(Label)))) {
    printf("Can't allocate a Label structure\n");
    return(NULL);
  }
  l->time = time;
  l->color = color;
  l->fields = NULL;
  l->prev = NULL;
  l->next = NULL;
  return(l);
}

/************************************************************************/
Fields *
new_field(str, id, color, font)
     char *str;
     int id, color, font;
{
  Fields *f;

  if(!(f = (Fields*)malloc(sizeof(Fields)))) {
    printf("Can't allocate a Fields structure\n");
    return(NULL);
  }
  f->str = str;
  f->field_id = id;
  f->color = color;
  f->font = font;
  f->next = NULL;
  f->xloc = f->yloc = 0;
  return(f);
}

/************************************************************************/
Label *
get_next_label(lf,time)
     Labelfile *lf;
     double time;
{
  Label *l;

  l = lf->last_label;
  while(l) {
    if(l->time >= time) {
      if(l->prev) {
	if(l->prev->time < time) return(l);
      } else return(l);
    } else return(NULL);
    l = l->prev;
  }
  return(NULL);
}

/************************************************************************/
Label *
get_prev_label(lf,time)
     Labelfile *lf;
     double time;
{
  Label *l;

  l = lf->first_label;
  while(l) {
    if(l->time < time) {
      if(l->next) {
	if(l->next->time >= time) return(l);
      } else return(l);
    } else return(NULL);
    l = l->next;
  }
  return(NULL);
}

/************************************************************************/
double 
get_prev_time(lf,time)
     Labelfile *lf;
     double time;
{
  Label *l;

  if((l = get_prev_label(lf,time)))
    return(l->time);
  else
    return(0.0);
}

/************************************************************************/
double 
get_next_time(lf,time)
     Labelfile *lf;
     double time;
{
  Label *l;

  if((l = get_next_label(lf,time)))
    return(l->time);
  else
    return(-1.0);
}

/************************************************************************/
Label *
get_nearest_label(lf, time)
     Labelfile *lf;
     double time;
{
  Label *lp, *ln;
  double tp= - 1.0, tn = 1.0;;

  if(!(lp = get_prev_label(lf,time)))
    return(get_next_label(lf,time));
  if((ln = lp->next)) {
    if((time - lp->time) < (ln->time - time)) return(lp);
    else return(ln);
  }
  return(lp);
}
    
/************************************************************************/
remove_nearest_label(lf,time,color)
     Labelfile *lf;
     double time;
     int color;
{
  Label *l;

  if((l = get_nearest_label(lf,time))) {
    if(l == lf->first_label)
      lf->first_label = l->next;
    else
      l->prev->next = l->next;
    if(l == lf->last_label)
      lf->last_label = l->prev;
    else
      l->next->prev = l->prev;
    lf->changed = 1;
    if(rewrite_labels(lf)) {
      waves_send(lf->obj->name, "unmark", l->color, l->time);
      free_label(l);
      return(TRUE);
    }
  }
  return(FALSE);
}
  
/************************************************************************/
unlink_label(l,lf)
     Labelfile *lf;
     Label *l;
{
  if(lf && l) {
    if(l == lf->first_label)
      lf->first_label = l->next;
    else
      l->prev->next = l->next;
    if(l == lf->last_label)
      lf->last_label = l->prev;
    else
      l->next->prev = l->prev;
    lf->changed = 1;
    return(TRUE);
  }
  return(FALSE);
}
  
/************************************************************************/
fields_destroy(f)
     Fields *f;
{
  Fields *fp;

  while((fp = f)) {
    if(fp->str) free(fp->str);
    f = fp->next;
    free(fp);
  }
}


/************************************************************************/
Label *
append_label(label,labelfile)
     Label *label;
     Labelfile *labelfile;
{
  Label *l;

  labelfile->changed = 1;
  if(!(l = labelfile->last_label)) {
    labelfile->last_label = labelfile->first_label = label;
    return(label);
  }
  while(l) {
    if(l->time <= label->time) {
      if(labelfile->last_label == l) labelfile->last_label = label;
      label->prev = l;
      label->next = l->next;
      if(l->next) l->next->prev = label;
      l->next = label;
      return(l);
    }
    if(l->prev) l = l->prev;
    else {
      l->prev = label;
      label->next = l;
      label->prev = NULL;
      labelfile->first_label = label;
      return(l);
    }
  }
  labelfile->changed = 0;
  return(NULL);
}

/*********************************************************************/
char_to_label(lf,l,c,field)
     Labelfile *lf;
     Label* l;
     int c, field;
{
  Fields *f, *f2;
  char *s1, *s2, s[2];
  int id;			/* This is the ID of the NEXT field (should one need to be created). */

  f = f2 = l->fields;
  if(field < 1) {		/* The field to change was not specified. */
    while(f) {
      f2 = f;			/* assume the final field is the active one. */
      f = f->next;
    }
    if(f2)
      id = f2->field_id + 1;
    else
      id = 1;		/* NOTE: id is one greater than field pointed to by f2. */
  } else {	/* A field to be changed WAS specified. */
    while(f) {
      f2 = f;
      if(f->field_id == field) break;
      f = f->next;
    }
    if(!f) {			/* Requested field doesn't exist (yet). */
      if(field > 1) {
	if(!f2)		 /* I.e. were ANY fields previously defined? */
	  l->fields = f2 = new_field(NULL,1,lf->color,NULL);
	while(f2->field_id < field) {
	  f = new_field(NULL,f2->field_id + 1,lf->color,NULL);
	  f2->next = f;
	  f2 = f;
	}
	id = f2->field_id + 1;
      } else
	id = 1;
    } else
      id = f2->field_id + 1;
  }
  /* At this point f2 should point to a field with (field_id == field), when field > 0, and field_id ==
     last field otherwise.  If there were no fields previously defined and (field < 1), then f2 is NULL,
     indicating that the first field should be defined. */

  lf->changed = TRUE;

  if(lf->separator[0] == c) {	/* create a new field? */
    f = new_field(NULL,id,lf->color,NULL);
    if(!f2) {			/* This is the second field in the list! */
      l->fields = f;
      f->next = new_field(NULL,id+1,lf->color,NULL);
    } else {
      f->next = f2->next;
      f2->next = f;		/* Link in the new field. */
      /* Increment the IDs of any following fields. */
      f = f->next;
      while(f) {
	f->field_id += 1;
	f = f->next;
      }
    }
    return(TRUE);
  }		/* end if(field separator) */

  if(f2 && (c == DELETE)) {	/* erase last character of last field? */
    int i;

    if(f2->str) {
      if((i = strlen(f2->str)) > 1)
	*(f2->str + i - 1) = 0;
      else {
	free(f2->str);
	f2->str = NULL;
      }
      return(TRUE);
    } else {			/* must free this field */
	f = l->fields;
	if(f == f2)		/* is it the first in the list? */
	  l->fields = f2->next;
	else {
	  while(f->next != f2) f = f->next;
	  f->next = f2->next;
	}
	f = f->next;
	free(f2);
	while(f) {
	  f->field_id -= 1;
	  f = f->next;
	}
      return(TRUE);
    }
  }	/*	end if(delete character) */

  /* char. was not a delete or a field separator; insert it in the current field*/
  if(f2) {			/* is there an active field? */
    if(f2->str) {		/* an active string? */
      s2 = (char*)malloc(strlen(f2->str) + 2);
      sprintf(s2,"%s%c",f2->str,c);
      free(f2->str);
      f2->str = s2;
    } else {
      f2->str = (char*)malloc(2);
      f2->str[0] = c;
      f2->str[1] = 0;
    }
  } else {			/* it is the first label field */
    s2 = (char*)malloc(2);
    s2[0] = c;
    s2[1] = 0;
    f= new_field(s2,id,lf->color,NULL);
    l->fields = f;
  }
  return(TRUE);
}
