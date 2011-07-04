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
/* parse.c */
/*  string handling for keyword-value implementation */

#ifndef lint
static char *sccs_id = "@(#)parse.c	1.20	9/26/95	ATT/ERL";
#endif

#include <stdio.h>
#include <ctype.h>
#include <Methods.h>
#include <w_lengths.h>

#define TRUE 1
#define FALSE 0

extern int w_verbose;
extern int debug_level;

typedef struct list {
  char *str;
  struct list *next;
} List ;

char *read_all_types(), *malloc();

#define WhiteSpace(ch) ((ch) == ' ' || (ch) == '\t' || (ch) == '\n')

/***********************************************************************/
strnscan(src,dest,len)
     char *src, *dest;
{
  int i = 0;
  
  if (!src || !dest) return(0);		/* check pointers */
  while (WhiteSpace(*src)) src++;
  len--;			/* leave room for trailing NULL */
  while (i<len && *src && !WhiteSpace(*src)) {
    *dest++ = *src++;		/* don't copy whitespace or NULL */
    i++;
  }
  *dest = '\0';			/* end of string */
  if (i>=len && *src && !WhiteSpace(*src)) {
    fprintf(stderr,"strnscan: truncated word %s\n",dest);
  }
  return(i);			/* return # bytes copied */
}

/***********************************************************************/
char *
get_next_item(list)
     register char *list;
{
  while(*list && ((*list == ' ') || (*list == '\t'))) list++;
  while(*list && (*list != ' ') && (*list != '\t') && (*list != '\n')) list++;
  while(*list && ((*list == ' ') || (*list == '\t') ||
		  (*list == '\n')))
    list++;
  return(list);
}

/***********************************************************************/
char *
get_line_item(list)
     register char *list;
{
  while(*list && ((*list == ' ') || (*list == '\t'))) list++;
  while(*list && (*list != ' ') && (*list != '\t') ) list++;
  while(*list && ((*list == ' ') || (*list == '\t') ))  list++;
  return(list);
}

/***********************************************************************/
char *
get_next_line(list)
     register char *list;
{
  if (!list) return(NULL);
  for ( ; *list; ++list) {
    if (*list == '\n') return(++list);
  }
  return(list);
}

/***********************************************************************/
char *
basename(path)
     char *path;
{
  register char *p;
  register int i;

  if(path && *path) {
    p = path + (i = strlen(path)) - 1;
    while(--i)
      if(*--p == '/') return( ++p);
    return(p);
  }
  return(path);
}

/***********************************************************************/
char *
basename2(path)
     char *path;
{
  static char shold[NAMELEN];
  register char *p;
  register int i;

  if(!path)
    return(NULL);

  if(*path) {
    p = path + (i = strlen(path)) - 1;
    while(--i)
      if(*--p == '/') {
	p++;
	break;
      }
    strcpy(shold,p);
  } else
    *shold = 0;
  return(shold);
}

/***********************************************************************/
static Selector *changed_items[300];
static int items_changed = 0;
/***********************************************************************/
Selector **get_changed_items()
{
  return(changed_items);
}
     
/***********************************************************************/
was_changed(item)
     char *item;
{
  int i;
  for(i=0; (i < items_changed) && changed_items[i]; i++)
    if(!strcmp(item,changed_items[i]->name))
      return(TRUE);
  return(FALSE);
}

/***********************************************************************/
reset_changed_items()
{
  changed_items[0] = NULL;
  items_changed = 0;
}
  
/***********************************************************************/
get_args(alist, adscr)
     register char *alist;
     Selector *adscr;
{
  changed_items[0] = NULL;
  items_changed = 0;
  return(get_args_raw(alist, adscr));
}

/***********************************************************************/
get_args_raw_orig(alist, adscr)
     register char *alist;
     Selector *adscr;
{
  register int gotsome = 0, gotone, n;
  char name[NAMELEN], junk[MES_BUF_SIZE];
  char *parsed_as;
  register Selector *ad, *ad2;


  if(alist && *alist && adscr) {
    while((*alist == ' ') || (*alist == '\t'))
      alist++;			/* skip initial blanks */
    while(*alist && (*alist != '\n')) {
      strnscan(alist,name,sizeof(name));
      if(! *(alist = get_line_item(alist))) {
	fprintf(stderr,"No argument for selector %s\n",name);
	if (debug_level > 1 || w_verbose > 1)
	    fprintf(stderr,"Got %d args\n", gotsome);
	if(items_changed < 300)
	  changed_items[items_changed] = NULL;
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
	    alist = (char*)read_all_types(alist,ad->format,ad->dest);
	    if(items_changed < 300)
	      changed_items[items_changed++] = ad;
	    gotsome++;
	    gotone = 1;
	  }
	  break;
	} else
	  ad = ad->next;
      }

      if (debug_level > 1 || w_verbose > 1) {
	if(gotone)
	  fprintf(stderr,"Parsed attribute %s as %s (raw)\n", name, parsed_as);
	else
	  fprintf(stderr,"Attribute %s not found in Selector list (raw).\n", name);
      }
	
      if(! gotone) {		/* keyword not found in adscr list */
	strnscan(alist,junk,sizeof(junk));
	if((*junk == '%') || (*junk == '#')) { /* looks like a format spec. */
	  alist = get_line_item(alist);	/* skip the non-standard item */
	  alist = (char*)read_all_types(alist,junk,NULL);
	} else {		/* punt */
	  fprintf(stderr,"Unknown or non-unique selector %s; argument %s ignored\n",name,junk);
	  alist = get_line_item(alist);
	}
      }
    }
  }
  if (debug_level > 1 || w_verbose > 1) 
        fprintf(stderr,"Got %d args\n", gotsome);

  if(items_changed < 300)
    changed_items[items_changed] = NULL;
  return(gotsome);
}

/***********************************************************************/
Selector *create_new_variable(sl, input, name)
     Selector *sl;
     char **input, *name;
{
  char *str;

  if(sl && (str = *input) && *str && name && *name) {
    char *mb, *nn, *nf, *nv, *savestring();
    Selector *s;
    int i;
    
    mb = get_line_item(str);
    if((strlen(str) < MES_BUF_SIZE) ||((mb - str) < MES_BUF_SIZE)) {
      s = (Selector*)malloc(sizeof(Selector));
      if(s && (nn = savestring(name))) {
	nf = savestring("#qstr");
	if(nf && (nv = malloc(MES_BUF_SIZE+1))) {
	  s->name = nn;
	  s->format = nf;
	  s->dest = nv;
	  s->next = NULL;
	  mb = (char*)read_all_types(str,s->format,s->dest);
	  while(sl->next) sl = sl->next;
	  sl->next = s;
	  *input = mb;
	  return(s);
	} else
	  fprintf(stderr,"Allocation problems (1) in create_new_variable\n");
	if(nf)
	  free(nf);
	free(nn);
	free(s);
      } else
	fprintf(stderr,"Allocation problems (2) in create_new_variable\n");
    } else
      fprintf(stderr,"Questionable args passed to create_new_variable (%s)\n",str);
  } else
    fprintf(stderr,"Bad args passed to create_new_variable\n");
  *input = get_line_item(*input);
  return(NULL);
}

/***********************************************************************/
get_args_raw(alist, adscr)
     char *alist;
     Selector *adscr;
{
  register int gotsome = 0, gotone, n;
  char name[NAMELEN], junk[MES_BUF_SIZE];
  char *parsed_as;
  register Selector *ad, *ad2;


  if(alist && *alist && adscr) {
    while((*alist == ' ') || (*alist == '\t'))
      alist++;			/* skip initial blanks */
    while(*alist && (*alist != '\n')) {
      strnscan(alist,name,sizeof(name));
      if(! *(alist = get_line_item(alist))) {
	fprintf(stderr,"No argument for selector %s\n",name);
	if (debug_level > 1 || w_verbose > 1)
	    fprintf(stderr,"Got %d args\n", gotsome);
	if(items_changed < 300)
	  changed_items[items_changed] = NULL;
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
	    alist = (char*)read_all_types(alist,ad->format,ad->dest);
	    if(items_changed < 300)
	      changed_items[items_changed++] = ad;
	    gotsome++;
	    gotone = 1;
	  }
	  break;
	} else
	  ad = ad->next;
      }

      if (debug_level > 1 || w_verbose > 1) {
	if(gotone)
	  fprintf(stderr,"Parsed attribute %s as %s\n", name, parsed_as);
	else
	  fprintf(stderr,"Attribute %s not found in Selector list.\n", name);
      }
	
      if(! gotone) {		/* keyword not found in adscr list */
	strnscan(alist,junk,sizeof(junk));
	if((*junk == '%') || (*junk == '#')) { /* looks like a format spec. */
	  alist = get_line_item(alist);	/* skip the non-standard item */
	  alist = (char*)read_all_types(alist,junk,NULL);
	} else {		/* punt */
	  if(debug_level || (w_verbose > 1))
	    fprintf(stderr,
		  "Unknown or non-unique selector %s; creating new entry for: %s\n",
		  name, alist);
	  if((ad = create_new_variable(adscr, &alist, name))) {
	    if(items_changed < 300)
	      changed_items[items_changed++] = ad;
	    gotsome++;
	  }
	}
      }
    }
  }
  if (debug_level > 1 || w_verbose > 1) 
        fprintf(stderr,"Got %d args\n", gotsome);

  if(items_changed < 300)
    changed_items[items_changed] = NULL;
  return(gotsome);
}

/* because some unix's don't have strchr... 
char *strchr_dt(s, it)
     register char *s;
     register int it;
{
  if(s) {
    while((*s != it) && (*s)) s++;
    if(*s == it) return(s);
  }
  return(NULL);
}
*/
/***********************************************************************/
/* Read any data type, specified by format, from the string list.
   Place the decoded result in *dest if the format specifies a standard C
   type.  For special types, *dest points to a structure pointer whose
   memory may be reallocated if a previous value is being replaced (if
   *dest is NULL, memory will be allocated as needed).  In all cases, if
   dest is NULL, the data will simply be skipped over.  Return a pointer
   to the first non-blank character after the end of the input from list.
   New user types may be added to this routine as necessary.  These types
   must have unique names which begin with the "#" character. */
char *
read_all_types(list, format, dest)
    register char *list, *format, *dest;
{
  int n;
  register char *p, *q, *r, **destp;
  char *strchr();
  List *l, *lp, **ldestp;

  if(list && *list) {
    if(*format == '%') {		/* is it a C type? */
      if(dest)
	sscanf(list,format,dest);
      return(get_next_item(list));
    }

    /* Must be one of the locally-defined types: */

    /* The astr type is simply a newline-terminated string */
    if(!strcmp(format, "#astr")) {
      destp = (char **) dest;
      if(destp && *destp) {
	free(*destp);
	*destp = NULL;
      }
      p = list;
      while((p = strchr(p,'\n')) && (*(p-1) == '\\')) p++;
      if(!p) {
	fprintf(stderr,"Newline not found while processing #astr type in read_all_types()\n");
	return(NULL);
      }
      n = 1 + p - list;
      if(destp) {
	if(!(q = malloc(n+1))) {
	  fprintf(stderr,"Can't allocate memory in read_all_types()\n");
	  return(NULL);
	}
	strncpy(q,list,n);
	q[n] = 0;
	*destp = q;
      }
      list += n;
      while(*list && ((*list == ' ') || (*list == '\t') ||
		      (*list == '\n'))) list++;
      return(list);
    }


    /* The cstr type is simply a null terminated string */
    if(!strcmp(format, "#cstr")) {
      destp = (char **) dest;
      if(destp && *destp) {
	free(*destp);
	*destp = NULL;
      }
      n = strlen(list);
      if(destp) {
	if(!(q = malloc(n+1))) {
	  fprintf(stderr,"Can't allocate memory in read_all_types()\n");
	  return(NULL);
	}
	strcpy(q,list);
	*destp = q;
      }
      list += n+1;
      while(*list && ((*list == ' ') || (*list == '\t') ||
		      (*list == '\n'))) list++;
      return(list);
    }


    /* The strq type is exactly like #str, except in the case where
       the first non-white character is a double quote.  In this case, it
       automatically switches to #qstr processing.  This type was invented to
       permit backward compatibility when parsing command arguments in
       xwaves. */
    if(!strcmp(format, "#strq")) {
      if(*list == '"')
	format = "#qstr";
      else
	format = "#str";
    }

    /* The str type is simply a newline or null terminated string.  The newline
       is removed from the returned result. This serves for lines of text
       like a C %s format.  The destination is ASSUMED to be big enough! */
    if(!strcmp(format, "#str")) {
      q = list;
      while((*++q != '\n') && (*q));
      if(*q == '\n') {
	*q = 0;
	n = strlen(list);
	if(dest) 
	  strcpy(dest,list);
	list += n+1;
	while(*list && ((*list == ' ') || (*list == '\t') ||
		      (*list == '\n'))) list++;
      } else {
	n = strlen(list);
	if(dest) 
	  strcpy(dest,list);
	list += n;
      }
      return(list);
    }


    /* The qstr type */
    if(!strcmp(format, "#qstr")) {
      char dummy[MES_BUF_SIZE];

      if(!dest)
	dest = dummy;
      while (WhiteSpace(*list)) list++;

      if(*list == '"') {	/* Quoted string; "" and \ escaped. */
	list++;
	while (*list != '"' && isascii(*list) && isprint(*list)) {
	  if(*list == '\\') {
	    switch (*++list) {
	    case '"':
	    case '\\':
	      *dest++ = *list++;
	      break;
	    default:
	      fprintf(stderr,
		      "read_all_types: unrecognized escape \"\\%c\"\n",
		      *list);
	      break;
	    }
	  } else
	    *dest++ = *list++;
	}

	if (*list == '"')
	  list++;
	else
	  fprintf(stderr, "read_all_types: unmatched quotes.\n");

      } else {	/* String of non-space chars delimited by whitespace. */
	while (*list && !WhiteSpace(*list))
	  *dest++ = *list++;
      }
      *dest = '\0';
      while (WhiteSpace(*list)) list++;
      return list;
    }


    /* The string type consists of an integer byte count followed by
       one blank (tab or space) and then any arbitrary byte string.
       Return a pointer to an array of char containing an exact copy of
       the (string type) with a null byte appended.  The byte count includes
       the count integer, space, etc.*/
    if(!strcmp(format, "#string")) {
      destp = (char **) dest;
      if(destp && *destp) {
	free(*destp);
	*destp = NULL;
      }
      sscanf(list, "%d", &n);
      if(n > 0) {
	/* Skip the byte count (possibly preceeded by whitespace) and the
	   following (single) space or tab. */
	q = list;
	while((*q < '0') || (*q > '9')) q++;
	while((*q >= '0') && (*q <= '9')) q++;
	n += (++q - list);
	if(destp) {
	  if(!(q = malloc(n + 1))) {
	    fprintf(stderr,"Can't allocate string buffer in read_all_types()\n");
	    return(NULL);
	  }
	  for(p = list + n - 1, r = q + n - 1; p >= list; ) *r-- = *p--;
	  q[n] = 0;
	  *destp = q;
        }
      }
      list += n;
      while(*list && ((*list == ' ') || (*list == '\t') ||
		      (*list == '\n'))) list++;
      return(list);
    }
      
  
    /* the list type consists of a separator character (any character except
       space and tab), a blank, a series of blank separated character strings,
       a final blank, and a closing separator character of the same type.
       These list elements (except for the separators) are read into linked
       character structures. */
    if(!strcmp(format, "#list")) {
      char temp[200], sep;

      ldestp = (List **) dest;
       /* need to free up previous allocations? */
      if (ldestp && (lp = *ldestp)) {
	while(lp) {
	  l = lp->next;
	  free(lp->str);
	  free(lp);
	  lp = l;
	}
	*ldestp = NULL;
      }

      strnscan(list,temp,sizeof(temp));
      sep = *temp;
      while(*list++ != sep);
      lp = NULL;
      while(*list) {
	strnscan(list,temp,sizeof(temp));
	list = get_next_item(list);
	if(*temp == sep) return(list);
	if(ldestp) {
	  if(!((l = (List*)malloc(sizeof(List))) &&
	       (l->str = q = malloc(strlen(temp) + 1)))) {
	    fprintf(stderr,"Can't allocate string buffer in read_all_types()\n");
	    return(NULL);
	  }
	  strcpy(q,temp);
	  l->next = NULL;
	  if(!lp) {		/* first list entry? */
	    *ldestp = lp = l;
	  } else {
	    lp->next = l;
	  }
	  lp = l;
	}
      }
    }


    /* The format specification has not been recognized!  Return a pointer
       to the next item (which will probably not make sense either...). */
    fprintf(stderr,"Type %s is unknown to this program.",format);
    fprintf(stderr,"Parsing continues ...\n");
    return(list);
  }
  return(NULL);
}
