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
 * Written by:  Kenneth C. Nelson
 * Checked by:
 * Revised by:
 *
 * Brief description: Search for file along a path given as arg or env variable
 *
 */

static char *sccs_id = "@(#)findespsfi.c	1.11	4/6/93	ERL";

/* INCLUDE FILES */

#include <stdio.h>
#include <string.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/esps.h>
#include <sys/types.h>
#if !defined(APOLLO_68K) && !defined(OS5)
#include <sys/dir.h>
#else
#include <dirent.h>
#endif
#ifndef MAXNAMLEN
# define MAXNAMLEN 255
#endif

/* EXTERNAL VARIABLE DECLARATIONS */

extern int debug_level;  /* Provided by user level programs */
extern char *savestring(); /* ESPS savestring routine */

/* LOCAL FUNCTION DECLARATIONS */

/* 
 * fileReadable() - Local function to check and see if the file is readable
 *
 */

static int fileReadable(name)
char *name;
{
 FILE *file;
 int result;

  file = fopen(name,"r");
  if (file != NULL)
  {
    result = TRUE;
    fclose(file);
  }
  else
  {
    result = FALSE;
  }
  
 return result;

}


/*
 * expandEnvVar() - expands and environment variable
 *                  treats ESPS_BASE specially.
 *
 */

static void expandEnvVar(into_string,envvar)
 char *into_string;
 char *envvar;
{

   /* Use get_esps_base(3) to find value for ESPS_BASE */

   if (strcmp(envvar,"ESPS_BASE") == 0)
   {
     get_esps_base(into_string);
   }
   else  /* get a normal ESPS value */
   {
     if (getenv(envvar) != NULL)
     {
       strcpy(into_string,getenv(envvar));
     }
     else
     {
       strcpy(into_string,"");
     }
   }
}

/*
 * build_filename() - local routine to build up a file name
 *                   given a string to build in, a filename,
 *                   and a path component which can contain
 *                   environment variables to expand.
 *
 */

char *build_filename(into_string,filename,dirname)
char *into_string;
char *filename;
char *dirname;

{

 char envvar[MAXNAMLEN];    /* holds name of env var if found */
 char envexp[MAXNAMLEN];    /* holds env var expansion */
 char pathpart[MAXNAMLEN];  /* holds part part of dirname if found */
 char charstr[2];     /* Place to put, chars in */
 int  envmode = FALSE;
 char *ch;

 ch = dirname;
 strcpy(charstr," ");  /* Build up a 1 character string */
 strcpy(into_string,"");

 while (*ch != '\0')
 {

   switch (*ch)
   {
     case '$':
       if (envmode)
       {
	 expandEnvVar(envexp,envvar);
         strcat(into_string,envexp);
       }
       envmode = TRUE;
       strcpy(envvar,"");
       break;

      case '/':        /* things that terminate paths */
      if (envmode)
       {
	 expandEnvVar(envexp,envvar);
         strcat(into_string,envexp);
       }
       envmode = FALSE;
       strcpy(envvar,"");
       if (*ch == '/')  /* Add the slash */
       {
         charstr[0] = *ch;
         strcat(into_string,charstr);  
       }
       break;

      default:
        charstr[0] = *ch;
        if (envmode)
        {
          strcat(envvar,charstr);
        }
	else
        {
	  strcat(into_string,charstr);
        }
        break;
     }
  ch++ ;
} 
 /* Catch any pesky env vars at the end of the path */

 if (envmode) /* If an enviroment var was being built when the string ended*/
 {
    expandEnvVar(envexp,envvar);
    strcat(into_string,envexp);
 } 

/* Add the file being searched for if one was provided. */

 if (strlen(filename) > 0)
 {
   strcat(into_string,"/");
   strcat(into_string,filename);
 }

 return into_string;
}


/* LOCAL CONSTANTS */

#define EC_SCCS_DATE "4/6/93"
#define EC_SCCS_VERSION "1.11"

/* LOCAL MACROS */

/* Separator in path strings */

#define PATH_FS ":"


/*
 * find_esps_file() - finds a file along a path. Returns full path to
 *                    file if it is readable.
 */

char *find_esps_file(fullpath,filename,defpath,env_var_name)
char* fullpath;
char* filename;
char *defpath;
char *env_var_name;

{

 char *searchPath;   /* Holds the list of paths to search */
 char *currentDir;  /* Holds filepath being checked currently */
 char currentFile[MAXNAMLEN];
 char thePath[MAXNAMLEN];
 char expandedFilename[MAXNAMLEN];  /* holds requested file after ENV var
				     *  expansion.
				     */
 char *returnedString = NULL; 

 int  found = FALSE;

 /* Expand ENV vars in the requested file */

   build_filename(expandedFilename,"",filename);

 /* Check to see if it is a full path, if so then return it */
 
if ( (expandedFilename[0] == '/') || 
      (expandedFilename[0] == '.' && expandedFilename[1] == '/'))
{
    if (fileReadable(expandedFilename))
    {
      found = TRUE;
      if (fullpath == NULL)
       returnedString = savestring(expandedFilename);
      else
      {
        strcpy(fullpath,expandedFilename);
	returnedString =  fullpath;
      }
    }
}
else  /* full path not specified already */
{
 /* First build path string (from default or from env variable. */

 /* If env_var_name specifies a path then use it. */

  if (env_var_name != NULL && getenv(env_var_name) != NULL)
  {
     searchPath = savestring(getenv(env_var_name)); /* use environment var */
  }
  else /* use the default path */
  {
     spsassert(defpath != NULL,"find_esps_file: default path is NULL");
     searchPath = savestring(defpath);
  }


 /* Now for each path field separator, loop and try to find the file */
 
  found = FALSE;
  currentDir = strtok(searchPath,PATH_FS);
  while (currentDir != NULL && !found)
  {    
     build_filename(currentFile,expandedFilename,currentDir);
     found = (fileReadable(currentFile) != NULL);
     currentDir = strtok(NULL,PATH_FS); /* Go to next field */
  }

 free(searchPath);

/* Now if we found it return it else return NULL */

 if (found)
 {
   if (fullpath == NULL) 
   {
     returnedString = savestring(currentFile);
   }
   else
   {
     strcpy(fullpath,currentFile);
     returnedString =  fullpath;
   }
 }
 else
 {
    returnedString = NULL;
 }
}


if (found)
{
  if (debug_level > 1)
  {
    fprintf(stderr,"find_esps_file: %s found as %s\n",filename,returnedString);
  }  
}
else
{
  if (debug_level > 1)
  {
     fprintf(stderr,"find_esps_file: %s not found\n",filename);
  }
}

return returnedString;

}
