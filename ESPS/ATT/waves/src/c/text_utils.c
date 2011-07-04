/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
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
 * a motley collection of more or less specialized text utilities
 */

static char *sccs_id = "@(#)text_utils.c	1.10	9/28/98	ATT/ESI/ERL";

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <esps/unix.h>
#include <esps/epaths.h>
#include <w_lengths.h>
  
#define TRUE  1
#define FALSE 0

extern char output_dir[];  /*defined in globals.c */
extern int debug_level;

char *build_filename(), *basename(); 
void setup_output_dir();

/*********************************************************************/
/* This assumes that the input list is sorted. */
char **uniq_list(l)
     char **l;
{
  if(l && *l && l[1] && *l[0] && *l[1]) {
    int i, j;
    for(i=0, j=1; l[i] && *l[i]; ) {
      while(l[j] && !strcmp(l[i],l[j])) j++;
      i++;
      l[i] = l[j];
    }
  }
  return(l);
}

/*********************************************************************/
strip_newline_at_etc(out,in)
     char *out, *in;
{
  if(out && in) {
    if(*in) {
      register char *pi = in, *pf;

      while(*pi && ((*pi == ' ') || (*pi == '	'))) pi++;
      while(*pi && (*pi == '@')) pi++;
      pf = pi;
      while(*pf && (*pf != ' ') && (*pf != '	') && (*pf != '\n')) pf++;
      *pf = 0;
      if(*pi) {
	strcpy(out,pi);
	return;
      }
    }
    *out = 0;
  }
}
      
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
block_build(buf,used,size,arg)
    char	**buf;	/* buffer into which the header is being built */
    char	*arg;	/* pointer to data destined for block*/
    int		*size,  /* dimension(-1) of *buf */
		*used;  /* number of bytes of buff that have been used */
{
  int n;
#if !defined(HP700) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
  char *realloc(), *malloc();
#endif

  n = strlen(arg);
  if((n + *used) > *size) {   /* bump block size */
    if(*buf && *size) {
      if(!(*buf = realloc(*buf, *size + n + MES_BUF_SIZE + 1))) {
	printf("Can't allocate enough buffer space in block_build()\n");
	return(FALSE);
      }
      *size += MES_BUF_SIZE + n;
    } else {
      if( ! (*buf = malloc(MES_BUF_SIZE + 1))) {
	fprintf(stderr,"Allocation problems in block_build()\n");
	return(FALSE);
      }
      *size = MES_BUF_SIZE;
      *used = 0;
    }
  }
  strcpy((char*)((*buf) + (*used)), arg);
  *used += n;
  return(TRUE);
}


/*************************************************************************/
/* This removes the carriage return inserted by the pty when waves is driven
by standard input with invocations like "xwaves - < /dev/ptyrf&" */
char *
clobber_cr(str)
    register char *str;
{
    register int n;

    if(str && *str && ((str[(n = strlen(str) -2)] == '\r') ||
		       (str[(n = strlen(str) -1)] == '\r')) && (n >= 0)) {
	str[n] = ' ';		/* convert it to a space */
    }

    return(str);
}

/*********************************************************************/
char *expand_name(out, in)
     char *out, *in;
{
  static char name[NAMELEN];
  
  if(in) {
    if(*in) {
      int n;
      build_filename(name,"#",in);
      n = strlen(name) - 2;
      if(name[n+1] == '#')
	name[n] = 0;
      else
	fprintf(stderr,"Weird filename in expand_name(%s, %s)\n",in,name);
    } else {
      if(out) {
	strcpy(out,"./");
	return(out);
      } else {
	strcpy(name,"./");
	return(name);
      }
    }
    if(out) {
      strcpy(out,name);
      return(out);
    }
    return(name);
  }
  fprintf(stderr, "Null input name passed to expand_name()\n");
  return(NULL);
}
    
/*********************************************************************/
/* This eliminates the silly behavior of requiring a ./ before all
   paths starting in the current directory. */
char *
apply_waves_input_path(n1,n2)
     char *n1, *n2;
{
  register char *cp = (char*)FIND_WAVES_INPUT(n1,n2), *c2=cp, *c3=cp+2;
  if(cp && *cp && !strncmp(cp,"./",2))
    while((*c2++ = *c3++));
  return(cp);
}

/*********************************************************************/
/* This eliminates the silly behavior of requiring a ./ before all
   paths starting in the current directory. */
char *
apply_esps_bin_path(n1,n2)
     char *n1, *n2;
{
  if(n2) {
    if(*n2) {
      register char *cp = (char*)FIND_ESPS_BIN(n1,n2), *c2=cp, *c3=cp;
      if(cp && *cp && (*cp == '.')) {
	c3++;
	while(*c3 == '/') c3++;
	while((*c2++ = *c3++));
      }
      return(cp);
    } else {
      if(n1) {
	*n1 = 0;
	return(n1);
      } else
	return(n2);
    }
  } else
    return(NULL);
}

/*********************************************************************/
/* If the terminal node in a pathname contains any of the common regular
   expression characters (like (){}[]* or ?) remove the terminal node.
   Return the modified string in situ. */  
char *
remove_reg_exp(s)
     register char *s;
{
  register int n = strlen(s), found = 0;
  register char *c = s + n, p;

  while(c-- > s) {
    if((p = *c) == '/') {
      if(found) *(c+1) = 0;
      return(s);
    }
    if((p == '*') || (p == '?') || (p == '(') || (p == '{') || (p == '[') ||
       (p == '$'))
      found = 1;
  }
  if(found) *s = 0;
  return(s);
}

/*********************************************************************/
char *
cleaned_for_input(s)
     register char *s;
{
  static char scratch[200];
  register char *c = s + strlen(s) - 1, p;

  if(((p = *c) == '@') || (p == '*')) {
    strncpy(scratch, s, c - s);
    scratch[c - s] = 0;
  } else
    strcpy(scratch,s);
  return(scratch);
}

      
/*********************************************************************/
char *
first_digit(st)
     char *st;
{
  register char *p;

  if(*st) {
    p = st + (strlen(st) - 1);
    while( p >= st) {
      if(*p == '/') return(NULL);
      if((*p >= '0') && (*p <= '9')) {
	while( --p >= st)
	  if((*p < '0') || (*p > '9')) return( ++p );
	return(st);
      }
      p--;
    }
  }
  return(NULL);
}

/*********************************************************************/
char *
after_last_digit(st)
     char *st;
{
  register char *p;

  if(*st) {
    p = st + (strlen(st) - 1);
    while( p >= st) {
      if(*p == '/') return(NULL);
      if((*p >= '0') && (*p <= '9')) return( ++p );
      p--;
    }
  }
  return(NULL);
}

/*********************************************************************/
get_num(st)
     char *st;
{
  int i;
  char t, *p1, *p2;

  if((p1 = first_digit(st))) {
    p2 = after_last_digit(st);
    t = *p2;
    *p2 = 0;
    sscanf(p1,"%d",&i);
    *p2 = t;
    return(i);
  } else
    return(-1);
}
  

/*********************************************************************/
make_next_name(st)
     char *st;
{
  int i, j;
  char sc[100], *p;

  if((i = get_num(st)) >= 0) {
    p = after_last_digit(st);
    strcpy(sc,p);
    p = first_digit(st);
    sprintf(p,"%d%s",i+1,sc);
  }
}
      

/*********************************************************************/
char *
make_x_name(st,ex)
     char *st, *ex;
{
  /* Changed to place new extention BEFORE old one
     e.g. "xyz.d" + ".ed" -> "xyz.ed.d" */
  register int i;
  static   char sc[NAMELEN];

  for (i = strlen(st); i >= 0 && st[i] != '.'; i--);

  if ((i < 0) || 
      ((i == 0) && ((st[0] == '.') || (st[0] == '/')))){  
	  /* no extension, just tack on new one */
	  strcpy(sc,st);
	  strcat(sc,ex);
  }
  else {		/* insert new extension */
    strncpy(sc,st,i);		/* old name without final extension */
    sc[i] = 0;			/* strncpy doesn't do this! */
    strcat(sc,ex);		/* new extention */
    strcat(sc,st+i);		/* replace old final extention */
  }
  setup_output_dir(sc); 
  return(sc);
}

 
/****************************************************************************/
/* if ex is the terminal string of name, or if ex is the string immediately
   preceeding the last "." in name, return TRUE, else FALSE */
is_a_x_file(name,ex)
     register char *name, *ex;
{
  register char *p, q;
  register int i, n, m;

  if (!name || !ex || !*name || !*ex)
    return(FALSE);

  p = name + (n = strlen(name)) - (m = strlen(ex));
  if((n >= m) && (! strcmp(p,ex))) return(TRUE);
  /* does "ex" exist just before an extension? (as might have been put there
     by make_x_name())? */
  for (i = n; i >= 0 && name[i] != '.'; ) i--;
  if(i > m) {		/* could be */
    q = name[i];
    name[i] = 0;
    p = name + (n = strlen(name)) - m;
    n = ((n >= m) && (! strcmp(p,ex)));
    name[i] = q;
    return(n);
  }
  return(FALSE);
}

/*      ----------------------------------------------------------      */
char *
new_ext(oldn,newex)
     char *oldn, *newex;
{
  int	j;
  char *dot;
  static char newn[256];
  static int len = 0;

  dot = strrchr(oldn,'.');
  if(dot != NULL){
    *dot = 0;
    j = strlen(oldn) + 1;
    *dot = '.';
    
  }
  else j = strlen(oldn);
  if((len = j + strlen(newex)+ 1) > 256) {
    printf("Name too long in new_ext()\n");
    return(NULL);
  }
  strncpy(newn,oldn,j);
  newn[j] = 0;
  return(strcat(newn,newex));
}


/*********************************************************************/
char *
remove_ext(name)
     register char *name;
{
  static char str[50];
  register int i;
  register char *c, *d;

  if(name && (i = strlen(name))) {
    c = name + i - 1;
    while( c > name ) {
      if(*c == '.') {
	d = c;
	while((d >= name) && (*d != '/')) d--;
	strncpy(str,d+1, (i = c - d - 1));
	str[i] = 0;
	return(str);
      }
      if(*c == '/') {
	strcpy(str, c+1);
	return(str);
      }
      c--;
    }
    strcpy(str,name);
    return(str);
  }
  return(NULL);
}
		   
/*************************************************************************/
/* Return TRUE if the last node in a pathname looks like a regular exp. */
its_a_reg_exp(s)
     register char *s;
{
  register int n = strlen(s);
  register char *c = s + n, p;

  while(c-- > s) {
    if((p = *c) == '/') return(FALSE);
    if((p == '*') || (p == '?') || (p == '(') || (p == '{') || (p == '[') ||
       (p == '$'))
      return(TRUE);
  }
  return(FALSE);
}

/*************************************************************************/
/* NOTE: This assumes that there is enogh room in name for an extra
   character or two! */
its_a_directory(name)
     char *name;
{
  if(name) {
    struct stat filestat;
    u_short filemode;
    int ret = stat(name, &filestat);

    if((ret == 0) && (filestat.st_mode & S_IFDIR)) {
      int n = strlen(name);
      
      if(n) {
	if (name[n-1] != '/') {
	  name[n] = '/';
	  name[n+1] = 0;
	}
      } else
	strcpy(name,"./");
      return(TRUE);
    }
  }
  return(FALSE);
}

/*************************************************************************/
its_a_partial_pathname(name)
     register char *name;
{
  register int n;

  n = strlen(name);
  return(!n || its_a_reg_exp(name) || its_a_directory(name));/* ||
	 (name[n-1] == '/') ||
     ((name[n-1] == '.') && ((n == 1) || (name[n-2] == '/')))); */
}

/*************************************************************************/
fix_path_end(name)
     register char *name;
{
  register int n;
  register char *c;

  c = name + (n = strlen(name)) - 1;
  if((!n) || (*c == '/')) return;
  if((*c == '.') && ((*(c-1) == '/') || (n == 1)))
    *c = 0;
/*  if((n > 1) && (*(c-1) != '/') && !its_a_reg_exp(name)) *c = '/'; */
  return;
}

/************************************************************************/
not_explicitly_named(name)
     char *name;
{
  if(name && *name) {
    return((*name != '/') && strncmp(name,"./",2));
  }
  return(FALSE);
}

/************************************************************************/
char *
get_output_file_names(outname, next)
     register char *next, *outname;
{
  static  char name[NAMELEN];
  char flist[NAMELEN];
  FILE *fdt, *fopen();
  int i, eof;

  /* If outname begins with "@" this means that the name specified
   is that of a filename list.  Otherwise outname is taken as the name
   of the output file.  In the latter case, if outname contains a numeric
   field (defined as the first occurrence of a contiguous sequence of numbers)
   this sequence will be incremented and a new filename will be created
   containing the incremented field.  The new (next) filename will be returned
   in next in any case. */

  if (debug_level) {
      (void) fprintf(stderr, "entered get_output_file_names: outname = %s\n",
		   outname);
    }

  if(!strlen(outname)) {
    *next = 0;
    return(NULL);
  }
  *name = 0;
  if(*outname == '@') {	/* we are dealing with a list of names */
    sscanf(&outname[1],"%s%s", flist, name);
    if(!(fdt = fopen(flist,"r"))){
      printf("List file %s not found\n",flist);
      *next = 0;
      return(NULL);
    }
    if(strlen(name)) {		/* index into the current filename */
      while((eof = fscanf(fdt, "%s",next)) != EOF) {
	if(!strcmp(name,next)) { /* found current name in list! */
	  if(fscanf(fdt,"%s",next) != EOF) {
	  } else {
	    *next = 0;
	  }
	  fclose(fdt);
	  return(name);
	}
      }
      printf("Name (%s) not found in list %s\n",name,flist);
      *next = 0;
      fclose(fdt);
      return(NULL);
    } else {			/* start with first name in list */
      if(fscanf(fdt, "%s",name) != EOF) {
	if(fscanf(fdt,"%s",next) != EOF) {
	} else {
	  *next = 0;
	}
	fclose(fdt);
	return(name);
      } else {
	printf("No names were found in list file %s\n",flist);
	*next = 0;
	fclose(fdt);
	return(NULL);
      }
    }
  } else {			/* interpret the name directly */
    if(strlen(outname)) {
      if(not_explicitly_named(outname))
	setup_output_dir(outname); 
      sscanf(outname,"%s",name);
      strcpy(next, name);
      make_next_name(next);
      if (debug_level)
	(void) fprintf(stderr, "make_output_file_names: returning next = %s\n",
		   next);
      return(name);
    } else {
      printf("No output file name was specified\n");
      *next = 0;
      return(NULL);
    }
  }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
insert_numeric_ext(old,num,new)
     char *old, *new;
     int num;
{				/* "abc.x",2 --> "abc.2.x"  */
  char *ext, temp[128];
    strcpy(new,old);
    if ( ext = strrchr(new,'.') ) /* has final extension */
      strcpy(temp,ext);		/* save old extension */
    else {
      temp[0] = '\0';		/* no old extension */
      ext = new + strlen(new);
    }
    sprintf(ext,".%d%s",num,temp);
}

/*----------------------------------------*/

/* setup_output_dir is used to adjust output file names so that 
 * all output files are written in the global output_dir if it 
 * has been defined; Since build_filename() is used, environment 
 * variables can be in the output_dir path.  
 */  

void
setup_output_dir(name)
char *name;
{
  /* No processing is needed unless output_dir is defined */

  if (strlen(output_dir) > 0) {
      struct stat filestat;
      char tnams[NAMELEN];
      u_short filemode;
      int ret;

      if(*output_dir)
	expand_name(output_dir,output_dir);

      ret = stat(output_dir, &filestat);
      filemode = filestat.st_mode;

      /* if there's an output_dir, we make sure it exists and has rwx 
	 permissions */

      if (!((ret == 0)
	    && (filemode & S_IFDIR)
	    && (filemode & S_IREAD)
	    && (filemode & S_IWRITE)
	    && (filemode & S_IEXEC))) 
      {
	(void) fprintf(stderr, 
	    "ERROR: output_dir doesn't exist or is without rwx permission\n");
	(void) fprintf(stderr, "    output_dir reset to /var/tmp.\n"); 
	strcpy(output_dir, "/var/tmp");
      }
      strcpy(tnams,basename(name));
      /* now build the full output name in the output directory */
      build_filename(name, tnams, output_dir); 
    }
}

/*********************************************************************/
strcmpcover(s1,s2)
     char **s1, **s2;
{
  return(strcmp(*s1, *s2));
}

/*********************************************************************/
char **sort_a_list(list)
     char **list;
{
  if(list && *list && list[1] && *list[0] && *list[1]) {
    int n = 1;

    while(list[n] && *list[n]) n++;

    qsort(list, n, sizeof(char*), strcmpcover);
  }
  return(list);
}    
