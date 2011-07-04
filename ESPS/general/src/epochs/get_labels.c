/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by: David Talkin 
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)get_labels.c	1.2	2/17/94	ERL";

/* get_labels.c */

#include <Objects.h>
#include <wave_colors.h>
#include <labels.h>

#define AMP 10000
#define min(x,y) ((x > y)? y : x)

typedef struct d_class {
  int type;
  char *name;
} D_CLASS;

D_CLASS dclass[] = {
  0, "O",
  1, "V",
  2, "H",
  3, "D",
  4, "F",
  5, "M",
  1, "aa",
  1, "ae",
  1, "ah",
  1, "ao",
  1, "aw",
  1, "ax",
  0, "ax-h",
  1, "axr",
  1, "ay",
  1, "b",
  1, "bcl",
  0, "ch",
  0, "d",
  1, "dcl",
  1, "dh",
  1, "dx",
  1, "eh",
  1, "el",
  1, "em",
  1, "en",
  1, "eng",
  0, "epi",
  1, "er",
  1, "ey",
  0, "f",
  1, "g",
  1, "gcl",
  0, "h#",
  0, "hh",
  1, "hv",
  1, "ih",
  1, "ix",
  1, "iy",
  1, "j",
  0, "jh",
  0, "k",
  0, "kcl",
  1, "l",
  1, "m",
  1, "n",
  1, "ng",
  1, "nx",
  1, "ow",
  1, "oy",
  0, "p",
  0, "pau",
  0, "pcl",
  1, "q",
  1, "r",
  0, "s",
  0, "sh",
  0, "t",
  0, "tcl",
  0, "th",
  1, "uh",
  1, "uw",
  1, "ux",
  1, "v",
  1, "w",
  1, "y",
  0, "z",
  1, "zh",
  -1, "the end" };

D_CLASS dclass0[] = {
  0, "O",
  1, "V",
  2, "H",
  3, "D",
  4, "F",
  5, "M",
  1, "aa",
  1, "ae",
  1, "ah",
  1, "ao",
  1, "aw",
  1, "ax",
  0, "ax-h",
  1, "axr",
  1, "ay",
  1, "b",
  0, "bcl",
  0, "ch",
  1, "d",
  0, "dcl",
  1, "dh",
  1, "dx",
  1, "eh",
  1, "el",
  1, "em",
  1, "en",
  1, "eng",
  0, "epi",
  1, "er",
  1, "ey",
  0, "f",
  1, "g",
  0, "gcl",
  0, "h#",
  0, "hh",
  1, "hv",
  1, "ih",
  1, "ix",
  1, "iy",
  1, "j",
  0, "jh",
  0, "k",
  0, "kcl",
  1, "l",
  1, "m",
  1, "n",
  1, "ng",
  1, "nx",
  1, "ow",
  1, "oy",
  0, "p",
  0, "pau",
  0, "pcl",
  1, "q",
  1, "r",
  0, "s",
  0, "sh",
  0, "t",
  0, "tcl",
  0, "th",
  1, "uh",
  1, "uw",
  1, "ux",
  1, "v",
  1, "w",
  1, "y",
  1, "z",
  1, "zh",
  -1, "the end" };


Fields *new_field();
Label *new_label();
Label *append_label();
char *get_next_item();



/************************************************************************/
Labelfile *
new_lf(label_name)
     char *label_name;
{
  Labelfile *l;
  char *name;
  int i;
  
  if(!((l = (Labelfile*)malloc(sizeof(Labelfile))) &&
       (name = malloc(strlen(label_name)+1)))) {
    printf("Can't allocate a Labelfile structure\n");
    return(NULL);
  }
  strcpy(name,label_name);
  l->label_name = name;
  l->sig_name = NULL;
  l->type = 1;
  l->color = MARKER_COLOR;
  l->changed = 0;
  l->font = 0;
  l->fontfile[0] = 0;
  l->menu = 0;
  l->comment = NULL;
  strcpy(l->separator,";");
  l->nfields = 1;
  for(l->active[0] = 1, i=1; i<LAB_MAXFIELDS; i++) l->active[i] = 0;
  l->first_label = l->last_label = NULL;
  l->next = NULL;
  return(l);
}

/**********************************************************************/
/* create a labelfile structure.  If the named file exists, read it in and
   use its contents to build label structure; otherwise, create the named file
   and write out it's header then close the file.
   */
Labelfile *
simply_read_labels(label_file_name)
     char *label_file_name;
{
  static int type, nfields, color;
  static char sep, signal[150], comment[501];
  static Selector  a1 = {"separator", "%c", &sep, NULL},
                   a2 = {"type", "%ld", (char*)&type, &a1},
                   a3 = {"nfields", "%ld", (char*)&nfields, &a2},
                   a4 = {"signal", "%s", signal, &a3},
                   a5 = {"color","%ld", (char*)&color, &a4},
                   a6 = {"comment", "#str", comment, &a5};
  FILE *fopen(), *fp;
  Labelfile *lf;
  Label *l;
  Fields *f, *fo;
  char in[501], *s, *p, *q;
  double time;
  int i, id;

  type = 1;
  sep = ';';
  nfields = 4;
  color = -1;
  *signal = 0;
  if((lf = (Labelfile*)new_lf(label_file_name)) &&
     (fp = fopen(lf->label_name,"r"))) {
    while(fgets(in,500,fp)) {	/* read label file header */
      if(*in == '#') break;
      get_args(in,&a6);
    }
    lf->color = color;
    lf->type = type;
    lf->separator[0] = sep;
    lf->nfields = nfields;
    while(fgets(in,500,fp)) {	/* read all labels */
      sscanf(in,"%lf%ld", &time, &color);
      if(! (l = new_label(time,color))) {
	printf("Can't create a new label; label MUST EXIT!\n");
	exit(-1);
      }
      if(lf->color < 0) lf->color = color; /* if not spec. in header */
      s = (char*)get_next_item(get_next_item(in));
      id = 1;
      fo = NULL;
      while(*s) {		/* get all label fields */
	if((p = (char*)index(s, sep)))
	  i = p - s;
	else i = strlen(s);
	if(s[i-1] == '\n') {
	  s[i-1] = 0;
	  i--;
	}
	if(i > 0) {
	  q = (char*)malloc((unsigned)(i + 1));
	  strncpy(q,s,i);
	  q[i] = 0;
	  f = new_field(q,id,color,NULL);
	  if(!fo) l->fields = f;
	  else fo->next = f;
	  fo = f;
	}
	id++;
	if(s[i] == sep) {
	  s = s + i + 1;
	  while(*s && ((*s == ' ') || (*s == '	'))) s++;
	} else
	  s = (char*)get_next_item(s+i);
      }
      append_label(l,lf);
    }
    lf->changed = 0;
    fclose(fp);
    return(lf);
  }
    return(NULL);
}

/*------------------------------------------------------*/
is(cp,lab)
     register char *lab, **cp;
{
  if(lab && *lab) {
    for( ; *cp; cp++)
      if(!strcmp(*cp,lab)) return(TRUE);
  }
  return(FALSE);
}

/*------------------------------------------------------*/
Vlist *read_label(name,of_interest)
     char *name, **of_interest;
{
  double start, end;
  Labelfile *lf;
  Label *l, *l2;
  Vlist *vl = NULL, *vlt, *vl0 = NULL;

  start = 0.0;
  if((lf = simply_read_labels(name)) && (l = lf->first_label)) {
    if(of_interest) {		/* select particular segments? */
      while(l) {
	if(l->fields && is(of_interest,l->fields->str)) {
	  end = l->time;
	  while(l) {
	    if(! l->next) {
	      vlt = (Vlist*)malloc(sizeof(Vlist));
	      vlt->start = start;
	      if(vl)
		vl->next = vlt;
	      if(l->fields && is(of_interest,l->fields->str)) end = l->time;
	      vlt->end = end;
	      vlt->next = NULL;
	      if(!vl0) vl0 = vlt;
	      vl = vlt;
	      return(vl0);
	    }
	    l = l->next;
	    if(l->fields && !is(of_interest,l->fields->str)) {
	      vlt = (Vlist*)malloc(sizeof(Vlist));
	      vlt->start = start;
	      vlt->end = end;
	      vlt->next = NULL;
	      if(vl) vl->next = vlt;
	      vl = vlt;
	      if(!vl0) vl0 = vlt;
	      start = l->time;
	      break;
	    }
	    end = l->time;
	  }
	} else
	  start = l->time;
	if(l)
	  l = l->next;
      }
    } else {			/* keep all segments */
      while(l) {
	end = l->time;
	vlt = (Vlist*)malloc(sizeof(Vlist));
	vlt->start = start;
	vlt->end = end;
	if(vl)
	  vl->next = vlt;
	vlt->next = NULL;
	if(!vl0) vl0 = vlt;
	vl = vlt;
	if(! l->next)
	  return(vl0);
	start = end;
	l = l->next;
      }
    }
    return(vl0);
  }
  return(NULL);
}  
