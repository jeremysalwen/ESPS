/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988, 1989 AT&T	*/
/*	  1990-1993 Entropic Research Laboratory, Inc.		*/
/*	  All Rights Reserved.			*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC RESEARCH LABORATORY, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* paste.c */
/*
   Segment Concatenation:
   ----------------------
   Three interactive operations will be supported: insert segment at
   cursor; prepend segment to file; and append segment to file.

   How should the source and resulting signals be named?  Note that it may be
   desirable to operate on ensembles of signals.

   For now, ensemble operations will not be available for splice/paste
   operations.  The "file browser menus" will used as a source of file
   names for the paste operation.  Segments will be set aside using the
   "save segment in file" operation.  The resulting files may then be
   pasted into any other file of the same type at the cursor location.
   The paste operation will be added to the right mouse menu; the ability to
   flag a file as paste source will be added to the browser menus.

   The file which is the sink for the paste operation will maintain its
   original start time unless the paste is a prepend in which case the
   inserted file's start time will be adopted.  The file name for the
   sink will be taken from the OUTPUTfile: item as usual.  When the paste
   operation is performed a disc image of the file will be created and then
   the display will be updated to reflect the changes.  So, the procedure is:

   Select a source filename from a browser window (entire file will be source).
   If no name is selected, most recently created "New File" will be used.
   Enter desired name for sink file in OUTPUTfile: item.
   If output file is null, quit.
   Move cursor to desired insert point in sink file display.
   Select "paste" item from waveform menu.

   Details of paste operation:
   --------------------------
   Generate an OUTPUTfile name for the sink file.
   If sink file is SIG_NEW, do a put_signal() (source will always be on disc).
   Create a temp SIG file for result.
   Except for ascii/binary differences, source and sink files must agree in
   	type, dimensions and sampling frequency; Source type will be changed
	to ascii or binary to match sink.
   Set header start_time, file_size, operator...
   Put_header() to temp file.
   Transfer initial part (if any) of sink file to temp file.
   Transfer source file to temp file.
   Transfer remainder of sink file to temp file and close it.
   If all operations were successful, relink the temp file to the desired name,
   		else delete the temp file, yell and quit.
   If the new filename doesn't appear in the "New Files" list, add it.
*/

#ifndef lint
static char *sccs_id = "@(#)paste.c	1.7	9/28/98	ATT/ERL";
#endif

#include <Objects.h>
#include <file_ext.h>
#include <esps/esps.h>
#include <esps/fea.h>


extern void set_pvd();
extern int debug_level;
extern char *remove_reg_exp();
extern char *cleaned_for_input();
/*********************************************************************/
static char designated_source[NAMELEN] = "";
set_designated_source(name)
     char *name;
{
  if(name && *name && (strlen(name) < NAMELEN))
    strcpy(designated_source, name);
  else
    *designated_source = 0;
  return;
}

/*********************************************************************/
Pending_input *
get_designated_source(le,name)
     Menu_list **le;
     char *name;
{
  Pending_input *pi;
  extern Pending_input *input_pending;
  extern Panel_item newFile_item;
  Menu_list *ml;
  char *cp;

  if(*designated_source) {
    strcpy(name, designated_source);
    *designated_source = 0;
    return((Pending_input*)TRUE);
  }

  pi = input_pending;
  while(pi) {
    if(((pi->banner && !strcmp("New Files",pi->banner)) ||
	 (pi->item == newFile_item)) &&
	 (ml = (Menu_list*)pi->list) && pi->canvas) {
      while(ml) {
	if(ml->str && *(ml->str) && ml->tapped) {
	  if(name) {
	    if(pi->path_prefix)
	      strcpy(name,remove_reg_exp(pi->path_prefix));
	    else
	      *name = 0;
	    strcat(name,cleaned_for_input(ml->str));
	  }
	  if(le)
	    *le = ml;
	  return(pi);
	}
	ml = ml->next;
      }
    }
    pi = pi->next;
  }
  return(NULL);
}

/*********************************************************************/
/*
  Look at the input pending lists for New Files and INPUTfile for a designated
  source.  If neither list contains entries, return NULL.  If neither list
  contains a designated source file, use the top entry in the New Files list,
  if it exists, else use the top entry in the INPUTfile list.  If the top
  entry option is taken, make the next item in the host list the top. */
Signal *
find_source_signal()
{
  extern Pending_input *input_pending;
  Pending_input *pi;
  extern Panel_item newFile_item;
  char *cp, result[200];
  Signal *get_any_signal();
  Menu_list *ml;

  	/* first look for "designated" source */
  if((pi = get_designated_source(&ml,result))) {
    return(get_any_signal(expand_name(NULL,result),0.0,0.0,NULL));
  }

  pi = input_pending;
  while(pi) {	/* just find top of New Files or INPUTfile: list */
    if(((pi->banner && !strcmp("New Files",pi->banner)) ||
	(pi->item == newFile_item)) &&
	  (ml = (Menu_list *)pi->list) && pi->canvas) {
      while(ml) {
	if(ml->str && *(ml->str) && ml->active) {
	  if(pi->path_prefix)
	    strcpy(result,remove_reg_exp(pi->path_prefix));
	  else
	    *result = 0;
	  strcat(result,cleaned_for_input(ml->str));
	  if(ml->next) {
	    ml->active = FALSE;
	    ml->next->active = TRUE;
	    if(pi->canvas)
	      menu_redoit(pi->canvas,NULL,NULL);
	  }
	  return(get_any_signal(expand_name(NULL,result),0.0,0.0,NULL));
	}
	ml = ml->next;
      }
    }
    pi = pi->next;
  }
  return(NULL);
}


/*********************************************************************/
paste(s,time)
     Signal *s;
     double time;
{
  Signal *s2, *snew, *insert_signal();
  View *v;
  double start, page_time, diff, lc_time, rc_time;
  extern Pending_input new_files;
  int i;
  char mes[400];

  if(!s)
    return(FALSE);

  if(! IS_GENERIC(s->type)) {
    sprintf(notice_msg, "Signals of type %x are not supported by paste",s->type);
    show_notice(1,notice_msg);
    return(FALSE);
  }
  if(s2 = find_source_signal()) {
    if(time < s->start_time) time = s->start_time;
    if(time > SIG_END_TIME(s)) time = SIG_END_TIME(s);
    if(s->file == SIG_NEW)  put_waves_signal(s);
    if(s2->file == SIG_NEW) put_waves_signal(s2);

    if((snew = insert_signal(s,s2,time))) {
      char newname[200];
      if(make_edited_filename(s,newname)) {
	close_sig_file(snew);
	unlink(newname);	/* in case a file of this name exists */
	sprintf(mes,"/bin/mv %s %s\n",snew->name,newname);
	if(i = system(mes))
	  sprintf(mes,"/bin/cp %s %s; rm %s",snew->name,newname,snew->name);
	if(!i || !system(mes)) {
	  char *na = malloc(strlen(newname) + 1);
	  strcpy(na, newname);
	  free(snew->name);
	  snew->name = na;
	} else {
	  sprintf(notice_msg, "Couldn't rename %s to %s in paste",
                              snew->name,newname);
	  show_notice(1,notice_msg);
        }
      } else {
	sprintf(notice_msg, "make_edited_filename() failed (%s %s) in paste",
	       s->name,newname);
        show_notice(1,notice_msg);
      }
      if(time <= s->start_time)
	lc_time = s2->start_time;
      else
	lc_time = time;
      rc_time = lc_time + SIG_DURATION(s2);
      snew->views = v = s->views;
      s->views = NULL;
      snew->obj = s->obj;
      unlink_signal(s);
      snew->others = ((Object*)snew->obj)->signals;
      ((Object*)snew->obj)->signals = snew;
      s->views = NULL;
      free_signal(s);
      while(v) {
	v->sig = snew;
	v = v->next;
      }
      update_window_titles(snew);
      v = snew->views;
      while(v) {
	v->lmarker_time = lc_time;
	v->rmarker_time = rc_time;
	v->cursor_time = lc_time;
	if(v->start_time < snew->start_time)
	  v->start_time = snew->start_time;
	if((diff = ET(v) - SIG_END_TIME(snew)) > 0.0)
	  v->start_time -= diff;
	start = v->start_time;
	page_time = ET(v) - start;
	get_view_segment(v,&start,&page_time);
	plot_view(v);
	v = v->next;
      }
      if(new_files.list) ((Menu_list*)new_files.list)->active = FALSE;
      if(add_to_menu_list(&new_files.list, snew->name))
	((Menu_list*)new_files.list)->active = TRUE;
      if(new_files.canvas) menu_redoit(new_files.canvas, NULL, NULL);
      return(TRUE);
    }
  } else {
    show_notice(1,"No destination signal, or source signal is not defined in paste");
  }
  return(FALSE);
}
	  
compatable(s1,s2)
     Signal *s1, *s2;
{
  if(((s1->type & ~SIG_FORMAT) == (s2->type & ~SIG_FORMAT)) &&
     ((s1->freq == s2->freq) || (IS_TAGGED_FEA(s1) && IS_TAGGED_FEA(s2))) &&
     (s1->dim == s2->dim) ) return(TRUE);
  return(FALSE);
}

/*********************************************************************/
Signal *
insert_signal(s1,s2,time)
     Signal *s1, *s2;
     double time;
{
  Signal * tag_insert_signal();

  /* Assumes s1 and s2 are both on disc; creates a new signal for the
   paste-up and returns a pointer to this new signal. s1 and s2 remain
   unchanged, except for their buffer positions. */

  if(IS_TAGGED_FEA(s1))
    return(tag_insert_signal(s1,s2,time));

  if(compatable(s1,s2) && (time >= s1->start_time) &&
     (time <= SIG_END_TIME(s1))) {
    char    tname[300], mess[200];
    Signal  *stmp;
    double  rtime, wtime, rinc, smax, smin, amax, amin, half_samp;
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
      sprintf(mess,"insert_signal: source %s sink %s time %f",s2->name,
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

	  hdr = stmp->header->esps_hdr = copy_header(stmp->header->esps_hdr);
	  set_pvd(hdr);

	  add_comment(hdr, "xwaves ");
	  strcat(mess, "\n");
	  add_comment(hdr, mess);
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
	half_samp = 0.5/stmp->freq; /* Find a reasonable error margin. */
	stmp->start_samp = 0;
	bs = 0;
	if(!s2start)	/* Output starts with samples from sink file. */
	  for(rtime=stmp->start_time, rinc=10.0;
	      rtime+half_samp < time; rtime += rinc) {
	    if((rtime+rinc) > time) rinc = time - rtime;
	    if(rinc > 0.0) {
	      if(s1->utils->read_data(s1,rtime,rinc)) {
		bs = s1->buff_size;
		rinc = BUF_DURATION(s1);
		fd_tmp = s1->file;
		s1->file = stmp->file;
		skip_tmp = s1->bytes_skip;
		s1->bytes_skip = stmp->bytes_skip;
			/* keep s1->header->strm consistent with s1->file */
		hdr_tmp = s1->header;
		s1->header = stmp->header;
		if(debug_level)
		  fprintf(stderr,"w1:rtime:%f rinc:%f st:%f et:%f\n",
			rtime,rinc,s1->start_time,s1->end_time);
		if(!s1->utils->write_data(s1,rtime,rinc)) {
		  close_sig_file(s1);
		  s1->file = fd_tmp;
		  s1->bytes_skip = skip_tmp;
			/* keep s1->header->strm consistent with s1->file */
		  s1->header = hdr_tmp;
		  printf("write_data() error1 in insert_signal()\n");
		  free_signal(stmp);
		  return(NULL);
		}
		s1->file = fd_tmp;
		s1->bytes_skip = skip_tmp;
			/* keep s1->header->strm consistent with s1->file */
		s1->header = hdr_tmp;
	      } else {
		printf("read_data() problems1 in insert_signal()\n");
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
	    rinc = BUF_DURATION(s2);
	    stmp->data = s2->data;
	    stmp->buff_size = s2->buff_size;
	    if(debug_level)
	      fprintf(stderr,"w2:wtime:%f rinc:%f st:%f et:%f\n",
		    wtime,rinc,stmp->start_time,stmp->end_time);
	    if(!stmp->utils->write_data(stmp,wtime,rinc)) {
	      printf("write_data() error2 in insert_signal()\n");
	      stmp->data = NULL;
	      free_signal(stmp);
	      return(NULL);
	    }
	    stmp->start_samp += stmp->buff_size;
	  } else {
	    printf("read_data() problems2 in insert_signal()\n");
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
	    rinc = BUF_DURATION(s1);
	    stmp->data = s1->data;
	    stmp->buff_size = s1->buff_size;
	    if(debug_level)
	      fprintf(stderr,"w3:wtime:%f rinc:%f st:%f et:%f\n",
	           wtime,rinc,stmp->start_time,stmp->end_time);
	    if(!stmp->utils->write_data(stmp,wtime,rinc)) {
	      printf("write_data() error3 in insert_signal()\n");
	      stmp->data = NULL;
	      free_signal(stmp);
	      return(NULL);
	    }
	    stmp->start_samp += stmp->buff_size;
	  } else {
	    printf("read_data() problems3 in insert_signal()\n");
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
	printf("output_header() failure in insert_signal()\n");
    } else
      show_notice(1,"Can't create a new signal in insert_signal");
  } else {
    sprintf(notice_msg,
        "Incompatible signals (%s %s) or bad insert time (%f) in insert_signal",
	s1->name,s2->name,time);
    show_notice(1,notice_msg);
    }
  return(NULL);
}

