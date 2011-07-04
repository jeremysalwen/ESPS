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
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)labelio.c	1.11 9/16/97 ERL";

#include <sys/param.h>
#include <Objects.h>
#include <labels.h>

extern char active[];
Labelfile *get_labelfile();
#ifdef FOR_XVIEW
Pixfont *open_label_font();
#endif
char *get_next_item();

/**********************************************************************/
/* create a labelfile structure.  If the named file exists, read it in and
   use its contents to build label structure; otherwise, create the named file
   and write out it's header then close the file.
   */
Labelfile *
read_labels(ob, label_file_name)
     char *label_file_name;
     Objects *ob;
{
  FILE *fopen(), *fp;
  Labelfile *lf;
  Label *l;
  Fields *f, *fo;
  char in[MES_BUF_SIZE], *s, *p, *q;
  double time;
  int i, id, color;

  if(!ob) return(NULL);

  if((lf = get_labelfile(ob,label_file_name))) {
    return(NULL);		/* already exists... */
  }

  if((fp = fopen(label_file_name,"r"))) { /* Does the file exist? */
    while(fgets(in,MES_BUF_SIZE,fp)) { /* read label file header */
      if(*in == '#') break;
      get_args(in,xlabel_global_list());
    }
    if((lf = new_labelfile(ob->name, label_file_name))) {
#ifndef EPOCHS
      color=get_default_xlabel_color();
#endif
      while(fgets(in,MES_BUF_SIZE,fp)) { /* read all labels */
	sscanf(in,"%lf%d", &time, &color);
	if(! (l = new_label(time,color))) {
	  printf("Can't create a new label; label MUST EXIT!\n");
	  fclose(fp);
	  return(NULL);
	}
	if(lf->color < 0) lf->color = color; /* if not spec. in header */
	s = get_next_item(get_next_item(in));
	id = 1;
	fo = NULL;
	while(*s) {		/* get all label fields */
	  if((p = (char*)index(s, lf->separator[0])))
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
	  } else
	    q = NULL;
	  f = new_field(q,id,color,NULL);
	  if(!fo) l->fields = f;
	  else fo->next = f;
	  fo = f;
	  id++;
	  if(s[i] == lf->separator[0]) {
	    s = s + i + 1;
	    while(*s && ((*s == ' ') || (*s == '	'))) s++;
	  } else
	    s = get_next_item(s+i);
	}
	append_label(l,lf);
      }
      lf->changed = 0;
    } else {
#ifdef WAVES_BINS
      sprintf(notice_msg,"Problems creating new labelfile %s", label_file_name);
      show_notice(0,notice_msg);
#else
      fprintf(stderr,"Problems creating new labelfile %s", label_file_name);
#endif
    }
    fclose(fp);
  } else {			/* must create a label file */
    if((lf = new_labelfile(ob->name, label_file_name))) {
      /* adjust for output_dir, if it exists */
      build_label_outout_name(lf->label_name);
      if((fp = fopen(lf->label_name,"w"))) {
	output_label_header(lf,fp);
	lf->changed = 1;
	fclose(fp);
      } else {
#ifdef WAVES_BINS
	sprintf(notice_msg,"Can't open an output file %s",lf->label_name);
        show_notice(0,notice_msg);
#else
	fprintf(stderr,"Can't open an output file %s",lf->label_name);
#endif
	free(lf->label_name);
	free(lf->sig_name);
	free(lf);
	return(NULL);
      }
    }
  }
  if(lf) {
#ifdef FOR_XVIEW
    if(*(lf->fontfile)) {
      Pixfont *pf;
      if(!(pf = open_label_font(lf->fontfile))) {
#ifdef WAVES_BINS
	sprintf(notice_msg,"read_labels(): Can't open fontfile %s; using default",
		lf->fontfile);
        show_notice(0,notice_msg);
#else
	fprintf(stderr,"read_labels(): Can't open fontfile %s; using default ",
                lf->fontfile);
#endif
	*lf->fontfile = 0;
      } else
	lf->font = pf;
    }
#endif
    lf->next = ob->labels;	/* link in the new labelfile */
    ob->labels = lf;
    lf->obj = ob;
    update_file_panel(lf);
    return(lf);
  } else
  return(NULL);
}

/************************************************************************/
output_label_header(lf,fp)
     Labelfile *lf;
     FILE *fp;
{
  if(lf && fp) {
    fprintf(fp,"signal %s\ntype %d\n",lf->sig_name, lf->type);
    fprintf(fp,"comment created using xlabel %s\n",get_date());
    if(lf->comment && *lf->comment)
      fprintf(fp,"comment %s\n",lf->comment);
    if(lf->color >= 0)
      fprintf(fp,"color %d\n",lf->color);
    if(*lf->fontfile)
      fprintf(fp,"font %s\n",lf->fontfile);
    fprintf(fp,"separator %s\nnfields %d\n#\n",lf->separator,lf->nfields);
  }
}

/************************************************************************/
build_label_outout_name(name)
     char *name;
{
  if(not_explicitly_named(name))
    setup_output_dir(name);
}

/************************************************************************/
get_maxfields(lf)
     register Labelfile *lf;
{
  if(lf) {
    register Label *l = lf->first_label;
    register Fields *f;
    register int maxf = 0;
    while(l) {
      f = l->fields;
      while(f) {
	if(f->field_id > maxf) maxf = f->field_id;
	f = f->next;
      }
      l = l->next;
    }
    return(maxf);
  }
  return(0);
}
      
/************************************************************************/
rewrite_labels(lf)
     Labelfile *lf;
{
  char name[MAXPATHLEN];
  Label *l;
  char c, in[2];
  int id;
  Fields *f;
  FILE *fp, *fopen();
  
  if(lf && lf->label_name && lf->changed) {
    sprintf(name,"%sBAK", lf->label_name);

    /* adjust for output_dir, if it exists */
    build_label_outout_name(name);
    build_label_outout_name(lf->label_name);

    unlink(name);
    if(!link( lf->label_name, name)) {
      if((!unlink(lf->label_name)) &&
	 ((fp = fopen(lf->label_name,"w")))) {
	lf->nfields = get_maxfields(lf);
	output_label_header(lf,fp);
	l = lf->first_label;
	while(l) {
	  f = l->fields;
	  fprintf(fp,"%12.6lf %4d ",l->time, l->color);
	  id = 1;
	  while(f) {
	    while(f->field_id > id) {
	      fprintf(fp,"%s ",lf->separator);
	      id++;
	    }
	    if(f->str) fprintf(fp,"%s",f->str);
	    f = f->next;
	  }
	  fprintf(fp,"\n"); 
	  l = l->next;
	}
	fclose(fp);
	return(TRUE);
      } else {
	printf("Couldn't open a new label file version (%s).\n",
	       lf->label_name);
	printf("Old version still exists as %s\n",name);
	return(FALSE);
      }
    } else {
#ifdef WAVES_BINS
      sprintf(notice_msg,"Couldn't relink old label file %s to %s. Therefore, no changes have been saved.",
	     lf->label_name, name);
      show_notice(0,notice_msg);
#else
      fprintf(stderr,"Couldn't relink old label file %s to %s. Therefore, no changes have been saved.",
             lf->label_name, name);

#endif
    }
  }
  return(FALSE);
}
      
/**********************************************************************/
file_update(labels, new_label)
	Labelfile *labels;
	Label *new_label;
{
  FILE *fp, *fopen();
  Fields *f;
  char c = labels->separator[0];
  int id = 1;

  if((fp = fopen(labels->label_name,"a"))) {
    f = new_label->fields;
    fprintf(fp,"%12.6lf %4d ",new_label->time, new_label->color);
    while(f) {
      while(f->field_id > id) {
	fprintf(fp,"%c ",c);
	id++;
      }
      if(f->str) fprintf(fp,"%s",f->str);
      f = f->next;
    }
    fprintf(fp,"\n");
    fclose(fp);
    return(TRUE);
  }
#ifdef WAVES_BINS
  sprintf(notice_msg,"Can't open %s for output or append; access privileges?",
	 labels->label_name);
  show_notice(0,notice_msg);
#else
  fprintf(stderr,"Can't open %s for output or append; access privileges?",
         labels->label_name);
#endif
  return(FALSE);
}

