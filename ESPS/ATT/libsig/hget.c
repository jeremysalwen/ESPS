/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988 AT&T	*/
/*	  and Entropic Speech, Inc.	*/
/*	  All Rights Reserved.	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC SPEECH, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* hget.c */
/* Retrieves current data values from signal file headers. */

#ifndef lint
static char *sccs_id = "@(#)hget.c	1.7	9/26/95	ATT/ERL";
#endif

#include <Objects.h>

main(ac, av)
     int ac;
     char **av;
{
  Signal *s, *get_signal();
  char type[200], t2[200], *it, *simpletype();
  double dpv;
  extern Selector head_a0;
  Selector *sl;
  List *lp;
  
  if(ac > 1) {
    if((s = get_signal(av[1],0.0,0.0,NULL))) { /* just get the header */
      if(ac > 2) {		/* any specific items requested? */
	while(ac-- > 2) {	/* selectively print them */
	  /* get_header_item() returns a pointer to the value of the
	     requested item.  The type-specification string is returned
	     in "type." */
	  if((it = (char*)get_header_item(s->header,av[2],type))) {
	    switch(*type) {	/* all types... */
	    case '%':		/* standard C types */
	      sprintf(t2,"%s %s\n",av[2],simpletype(type));
	      if(strcmp(type,"%s")){
		if(! strcmp(type,"%lf"))	/* 8-byte size */
		  printf(t2,*(double*)it);
		if(! strcmp(type,"%f"))		/* floats */
		  printf(t2,*(float*)it);
		if(!(strcmp(type,"%d") &&
		     strcmp(type,"%x") &&
		     strcmp(type,"%o"))) /* 4-byte size */
		  printf(t2,*(int*)it);
		if(!(strcmp(type,"%hd") &&
		     strcmp(type,"%hx") &&
		     strcmp(type,"%ho"))) /* 2-byte size */
		  printf(t2,*(short*)it);
		if(!strcmp(type,"%c")) printf(t2,*it); /* 1-bite size */
	      } else		/* it is just a string */
		printf(t2,it);
	      break;
	    case '#':		/* special types */
	      if(!strcmp(type,"#list")) {
		lp = (List*)it;
		printf("%s",av[2]);
		while(lp) {
		  if(lp->str) printf(" %s",lp->str);
		  lp = lp->next;
		}
		printf("\n");
		break;
	      } else {
		printf("%s %s\n",av[2],it);
		break;
	      }
	    default: break;
	    }
	  } else
	    printf("%s <not found in header>\n",av[2]);
	  av++;
	}
      } else { /* If no fields were specified, just dump the entire header. */
	char *p = s->header->header;
	int n = s->header->nbytes - 1, kluge[2];
	kluge[0] = s->header->magic;
	kluge[1] = 0;
	while(n--) {
	  if(! *p) *p = '@';	/* substitute "@" for nulls in header */
	  p++;	     /* NOTE that this effectively ruins s->header->header! */
	}
	printf("%4s%7d\n%s\n",
	       kluge,s->header->nbytes,s->header->header);
      }
    } else
      printf("Can't get_signal(%s)\n",av[1]);
  } else
    printf("Usage: %s <SIG file> [<keyword(s) of values to be printed>]\n",av[0]);
}

/* Some printfs don't like the size specification in output formats
   (like %lf, %hd, etc.).  This routine fixes the C format types returned by
   get_header_item(). */
char *
simpletype(type)
     char *type;
{
  static char t[10];

  if((type[1] == 'h') || (type[1] == 'l')) {
    *t = *type;
    t[1] = type[2];
    t[2] = NULL;
    return(t);
  }
  return(type);
}
