/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
 *
 * Program: xhelp.c
 *
 * Written by:  John Shore
 * Checked by:
 *
 * functions for creating help window
 */

#ifndef lint
static char *sccs_id = "@(#)ehelp.c	1.2	3/23/90	ESI";
#endif

/*
 * system include files
 */
#include <stdio.h>
#include <esps/esps.h> 

extern int debug_level;

#define TEMPATH "/tmp"		/* location of temporary files */

char *make_temp();
char *mktemp();
char *savestring();
char *sprintf();

char *
get_helpfile(progfile)
char *progfile;
{
  /*This function decodes the progfile; if it's a bare file, all we 
    do is check that we can read it. If it represents an ESPS program,
    we put cleaned eman output in a temp file and return that name
    */

  char *filename;		/* filename for help */

  /* command  to clean an ESPS man page; this just removes underlining
     and the copyright line (which has lots of control characters embedded 
     into it; it would be better to leave in a cleaned up copyright and 
     last-changed string, as well as to delete large groups of empty lines;
     We use an sed script; probably should change to awk.
   */

/*  char *clean_com = "sed -e \"s/_//g\" -e \"/Copyright c/d\" -e \"/^$/d\" ";*/


  char *clean_com = "sed -e \"s/_//g\" -e \"/Copyright c/d\" ";

  FILE *hfd; 
  int status = 0;
  char    *template = "xpromptXXXXXX";
  char command[500];

  if ((progfile[0] == '.') || (progfile[0] == '/'))

    filename = progfile; 

  else { /*assume it's the name of an ESPS program*/

    filename = make_temp(TEMPATH, template); 
    sprintf(command, "eman %s | %s > %s", progfile, clean_com, filename);

    if (debug_level)
      Fprintf(stderr, "get_helpfile: running shell command: %s\n", command);
    status = system(command);
  }

  if (((hfd = fopen(filename, "r")) == NULL) || (status != 0)) {
    return((char *) NULL);
  }
  else {
    fclose(hfd);
    return(savestring(filename));
  }
}

/* routine for making temp files*/ 
static 
char *
make_temp(path, template)
char *path;
char *template;
{
  char tpath[100];
  (void) sprintf(tpath, "%s/%s", path, template);  
  return(savestring(mktemp(tpath)));
}
