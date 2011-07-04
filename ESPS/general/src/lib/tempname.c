/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John shore
 * Checked by:
 * Revised by:
 *
 * Brief description: ESPS utilities for temp files
 */

static char *sccs_id = "@(#)tempname.c	1.5	1/4/96	ERL";

#include <stdio.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif

/* #define DEF_TEMP_PATH "/usr/tmp" /* last resort location of temporary files */
#define DEF_TEMP_PATH "/var/tmp" /* last resort location of temporary files */

#define DEF_TEMPLATE  "espsXXXXXX"

extern char *savestring();

extern int debug_level;

char *
e_temp_name(template)
     char	*template;
{
/* creates and returns a unique temporary file name, replacing trailing 
 * XXXXXX via mktemp.  If there are no path specifiers ('/') in template, 
 * the file name is in ESPS_TEMP_PATH (default /var/tmp).  The template
 * argument is unchanged and can be re-used by the caller.  The function
 * returns NULL if it can't create the name or if the name isn't writable.
 */ 

  char *tmp_path;
  char *tmp_plate;
  char *new_file;
  char *env_tmp;
  int name_length;
  int has_slash = 0;
  FILE *tmp_strm;

  if ((template != NULL) && (strchr(template, '/') != NULL))
      has_slash = 1;
     
  if (has_slash) {

      new_file = savestring(template);
      
    }
  else {

      env_tmp = getenv("ESPS_TEMP_PATH"); 

      tmp_path = ((env_tmp == NULL) ? DEF_TEMP_PATH : env_tmp);

      tmp_plate = ((template == NULL) ? DEF_TEMPLATE : template);

      name_length = strlen(tmp_path) + strlen(tmp_plate) + 2;

      new_file = malloc(name_length); 

      if (new_file != NULL)
	sprintf(new_file, "%s/%s", tmp_path, tmp_plate);  
    }  

  if (new_file == NULL) {
      if (debug_level) 
	(void) fprintf(stderr, "etempname: malloc failed for new name\n");
      return((char *)NULL);
  }
  else {

      (void) mktemp(new_file); 

      tmp_strm = fopen(new_file, "w+");

      if (tmp_strm == NULL) {
	  if (debug_level) 
	    (void) fprintf(stderr, 
			 "e_temp_name: file %s not writable\n", new_file);
	  free(new_file);
	  return((char *)NULL);
	}
      else {
	  (void) fclose(tmp_strm);
	  (void) unlink(new_file); 
	  return(new_file);
	}
    }
}

FILE *
e_temp_file(template, new_file)
     char *template;
     char **new_file;
{
  FILE *tmp_strm = NULL;

  *new_file = e_temp_name(template);

  tmp_strm = fopen(*new_file, "w+"); 

  if (debug_level && (tmp_strm == NULL)) {
    if (*new_file) 
      (void) fprintf(stderr, 
		     "e_temp_file: couldn't open file %s\n", *new_file);
    else
      (void) fprintf(stderr, 
		     "e_temp_file: couldn't open file\n");
  }
  return(tmp_strm); 
}

