/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 *  M A R K E R I O
 *
 *     read and write routines for marker files
 */
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
 * Written by:  Mark Liberman
 * Checked by:
 * Revised by: David Talkin
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)markerio.c	1.2	9/26/95	ATT/ERL";


#include <stdio.h>
#include <ctype.h>
#include <marker.h>

char *fgets();

Sentence *
read_markers(infd)
FILE *infd;
{
  if(infd) {
    Word *lastword = NULL;
    Marker *lastmarker = NULL;
    Sentence *sentence = get_sentence();
    Word *w;
    Marker *m;
    char line[4000];
    int allspace;
    char temp[4000], temp1[4000];
    int nwords = 0, nmarkers = 0;
    register char *ptr;

    lastword = get_word();
    lastword->spelling[0] = '\0';
    sentence->first = lastword;

    while(fgets(line,3999,infd) != NULL){
      if(line[0] == '*' && line[1] == '*'){
	strncpy(sentence->text,line+2,NTEXT);
	ptr = sentence->text + strlen(sentence->text) - 1;
	if(*ptr == '\n')*ptr = '\0';
	continue;
      }
      if(strlen(line) == 0)continue;
      for(allspace=1,ptr=line;*ptr;ptr++){
	if(!isspace(*ptr)){
	  allspace = 0;
	  break;
	}
      }
      if(allspace)continue;
      if(isspace(line[0])){	/* ordinary marker */
	m = get_marker();
	sscanf(line,"%s%f",temp,&(m->time));
	strncpy(m->label,temp,NLABEL);
	insert_marker(lastmarker,m,NULL);
	m->word = lastword;
	if(lastword->first == NULL)
	  lastword->first = m;
	lastmarker = m;
	nmarkers++;
      }
      else {			/* word-type marker */
	w = get_word();
	*temp = *temp1 = 0;
	sscanf(line,"%s%s",temp,temp1);
	strncpy(w->spelling,temp,NSPELLING);
	strncpy(w->transcription,temp1,NSPELLING);
	insert_word(lastword,w,NULL);
	if(lastword != NULL)lastword->last = lastmarker;
	lastword = w;
	if(nmarkers == 0 && nwords == 0)sentence->first = w;
	nwords++;
      }
    }
    if(lastword->first != NULL)lastword->last = lastmarker;
    sentence->last = lastword;
    return(sentence);
  } else
    return(NULL);
}

print_markers(s,outfd)
     Sentence *s;
     FILE *outfd;
{
  if(s && outfd) {
    Marker *m;
    Word *w;
    Word *lastword = NULL;
    int i,n;

    fprintf(outfd,"**%s\n",s->text);
    for(w=s->first;w && w->left != s->last;w = w->right){
      fprintf(outfd,"%s\t%s\n",w->spelling,w->transcription);
      for(m=w->first;m && m->left != w->last;m = m->right){
	fprintf(outfd,"  %10s",m->label);
	if(m->time > 0.)
	  fprintf(outfd,"%18.8g\n",m->time);
	else fprintf(outfd,"\n");
      }
    }
  }
}


    
