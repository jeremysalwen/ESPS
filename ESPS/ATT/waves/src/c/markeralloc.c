/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*
 *  M A R K E R A L L O C
 *
 *    storage allocation for marker information
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
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)markeralloc.c	1.2	9/26/95	ATT/ERL";

#include <stdio.h>
#include <marker.h>

#define NWORDS 300
#define NMARKERS 2000
#define NSENTENCES 15

Word wordlist[NWORDS];
Marker markerlist[NMARKERS];
Sentence sentencelist[NSENTENCES];

Word nullword;

Marker nullmarker;

Sentence nullsentence;

Word *nextword = wordlist;
Marker *nextmarker = markerlist;
Sentence *nextsentence = sentencelist;

/* (re-)initialize all marker and word lists */

init_markers()
{

  nextword = wordlist;
  nextmarker = markerlist;
  nextsentence = sentencelist;
  nullword.spelling[0] =
    nullword.transcription[0] =
      nullmarker.label[0] =
	nullsentence.text[0] = '\0';
  nullmarker.time = -1;

}

Word *
get_word()
{
  Word *w = nextword;
  if(w >= wordlist + NWORDS){
    fprintf(stderr,"ran out of words\n");
    return(NULL);
  }
  nextword++;
  *w = nullword;
  return(w);
}

Marker *
get_marker()
{
  Marker *m = nextmarker;
  if(m > markerlist + NMARKERS){
    fprintf(stderr,"ran out of markers\n");
    return(NULL);
  }
  nextmarker++;
  *m = nullmarker;
  return(m);
}

Sentence *
get_sentence()
{
  Sentence *s = nextsentence;
  if(s > sentencelist + NSENTENCES){
    fprintf(stderr,"ran out of sentences\n");
    return(NULL);
  }
  nextsentence++;
  *s = nullsentence;
  return(s);
}

insert_word(left,thing,right)
Word *left, *thing, *right;
{
  thing->left = left;
  thing->right = right;
  if(left != NULL)left->right = thing;
  if(right != NULL)right->left = thing;
}

delete_word(w)
Word *w;
{
  Word *l = w->left;
  Word *r = w->right;

  if(l != NULL)l->right = r;
  if(r != NULL)r->left = l;
}

insert_marker(left,thing,right)
Marker *left, *thing, *right;
{
  thing->left = left;
  thing->right = right;
  if(left != NULL)left->right = thing;
  if(right != NULL)right->left = thing;
}

Marker *
delete_marker(m)
Marker *m;
{
  Marker *l=m->left, *r=m->right;

  if(m->word->first == m && m->word->last == m){
    m->word->first = m->word->last = NULL;
  }
  else if(m->word->first == m){
    m->word->first = r;
  }
  else if(m->word->last == m){
    m->word->last = l;
  }

  if(l != NULL)l->right = r;
  if(r != NULL)r->left = l;
  if(r != NULL)return(r);
  else  if(l != NULL)return(l);
  else return(m);
}
