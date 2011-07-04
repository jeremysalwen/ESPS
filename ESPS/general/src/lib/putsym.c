/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"

  This module contains the functions used to put symbols into the esps 
  common file.
*/
 				

#ifndef lint
	static char *sccs_id = "@(#)putsym.c	1.14	12/22/93 ESI";
#endif
 	

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <esps/esps.h>

#define	LINE_SIZE	256
#define	MAXLEN		256
#define	TRUE		1
#define	FALSE		0

#define	TMP_FILE	"/tmp/comXXXXXX"

char	*getenv(), *strcpy(), *strtok(), *mktemp();
char	*strcat();
void rewind();
double atof();

static FILE	*com_fp = NULL;		/* file pointer to ESPS Common file */
static char	filename[80];
static FILE	*tmp_fp;
static char	template[20];

static int
setup_common(file)
char *file;
{
char	*home;
char	*com;
struct stat combuf;

    com_fp = NULL;
    filename[0] = '\0';
    tmp_fp = NULL;
    strcpy(template,TMP_FILE);
    if(file == NULL) {
      if(getenv("USE_ESPS_COMMON") != NULL && 
         strcmp(getenv("USE_ESPS_COMMON"),"off") == 0)
	  return 0;

      if((com = getenv("ESPSCOM")) == NULL) {
        if( (home = getenv("HOME")) == NULL )
	  home = ".";
        (void) strcpy(filename,home);
        (void) strcat(filename,"/");
        (void) strcat(filename,SPS_COMMON);
       }
       else
        (void) strcpy(filename,com);
     } else {
       (void) strcpy(filename,file);
     }

    if ((com_fp = fopen (filename, "r")) == NULL) {
	if ((com_fp = fopen(filename, "w")) == NULL) {
	   (void) fprintf (stderr,
	   "putsym: could not create ESPS Common file: %s.\n",filename);
	   return -1;
	}
    } else {
	  if (stat(filename, &combuf) != 0) {
	     fprintf(stderr,"putsym: cannot stat common file %s\n",
		     filename);
	     fprintf(stderr,"This should not happen.\n");
	     exit (1);
	  }

	  if (combuf.st_mode & S_IFDIR) {
	    fprintf(stderr,
              "putsym: Warning, ESPS Common file %s is a directory.\n",
	      filename);
	    fprintf(stderr, "It is not being processed.\n");
	    fclose(com_fp);
	    return -1;
	  }
    }

    if ((tmp_fp = fopen (mktemp(template), "w+")) == NULL ) {
	(void) fprintf (stderr,
	"putsym: could not open temporary file %s for writing.\n",
	template);
	return -1;
    }
    return 1;
}


int
putsym_i (symbol, value)
char	*symbol;
int	value;
{



char	buffer[LINE_SIZE];	/* buffer to hold output text */
char	*text = buffer;		/* pointer to buffer */
char	*name = buffer;		/* string for holding name of symbol */
char	line[LINE_SIZE];
char	*type;
char	*val;

int	found = FALSE;
int     ret;

    spsassert(symbol != NULL, "putsym_i: symbol is NULL");

    if((ret = setup_common(NULL)) != 1) return ret;

/*
 * we've opened the file, now update it.
 * But first check if symbol already exists in the ESPS Common file.
 *
 */

#ifdef DEBUG
   (void) fprintf(stderr,"putsym_i: value: %d, symbol: %s\n",value,symbol);
#endif

     while (fgets(line, LINE_SIZE, com_fp) != NULL) {

	(void) strcpy (buffer, line);
 	if ( strcmp("int", (type = strtok(buffer, " \t"))) != 0 ) {
 
#ifdef DEBUG
   (void) fprintf (stderr, "putsym_i: skipping (%s) type\n", type);
#endif
	   (void) fputs (line,tmp_fp);
	   continue;

	}

	name = strtok(NULL, "\t =");
 	val = strtok(NULL, "\t ;");

#ifdef DEBUG
	(void) fprintf (stderr, "putsym_i: type: %s\n", type);
	(void) fprintf (stderr,	"          name: %s\n", name);
	(void) fprintf (stderr,	"         value: %d\n", atoi(val));
#endif


	if (strcmp(name, symbol) != 0) {
#ifdef DEBUG
	   (void) fprintf (stderr, "putsym_i: skipping (%s) symbol\n", name);
#endif
	   (void) fputs (line,tmp_fp);
	   continue;
	}

#ifdef DEBUG
	(void) fprintf (stderr, "putsym_i: updating (%s) symbol\n", name);
#endif

	(void)sprintf (text, "int %s = %d;\n", symbol, value);
	(void) fputs (text,tmp_fp);
	found = TRUE;

     } /* end while */

     if ( !found ) {
	/* Looks like the symbol didn't exist in the ESPS Common file
	 * so let's append it to the end of the file. */
#ifdef DEBUG
	(void) fprintf (stderr, "putsym_i: new symbol (%s)\n", name);
#endif
	(void)sprintf (text, "int %s = %d;\n", symbol, value);
	(void) fputs (text,tmp_fp);
     }

/*
 * Now copy contents of TMP_FILE to ESPS Common file,
 * but rewind both files first
 *
 */
    (void) rewind (tmp_fp);
    (void) fclose (com_fp);

    if ((com_fp = fopen (filename, "w")) == NULL) {
	(void) fprintf (stderr,
	"putsym_i: could not open ESPS Common file %s for writing.\n",
	filename);
	return -1;
    }

    while (fgets(line, LINE_SIZE, tmp_fp) != NULL) {
	(void)fprintf (com_fp, "%s", line);
#ifdef DEBUG
	(void)fprintf (stderr, "%s", line);
#endif
    }

    (void) fclose (tmp_fp);
    (void) fclose (com_fp);
    (void) unlink (template);

    return 0;

} /* end putsym_i() */

int
fputsym_i (symbol, value, file)
char	*symbol;
int	value;
char	*file;
{

char	buffer[LINE_SIZE];	/* buffer to hold output text */
char	*text = buffer;		/* pointer to buffer */
char	*name = buffer;		/* string for holding name of symbol */
char	line[LINE_SIZE];


char	*type;
char	*val;

int	found = FALSE;
int     ret;

    spsassert(symbol != NULL, "putsym_i: symbol is NULL");
    spsassert(file, "fputsym_i: filename is NULL");

    if((ret = setup_common(file)) != 1) return ret;
/*
 * we've opened the file, now update it.
 * But first check if symbol already exists in the ESPS Common file.
 *
 */

#ifdef DEBUG
   (void) fprintf(stderr,"fputsym_i: value: %d, symbol: %s\n",value,symbol);
#endif

     while (fgets(line, LINE_SIZE, com_fp) != NULL) {

	(void) strcpy (buffer, line);
 	if ( strcmp("int", (type = strtok(buffer, " \t"))) != 0 ) {
 
#ifdef DEBUG
   (void) fprintf (stderr, "fputsym_i: skipping (%s) type\n", type);
#endif
	   (void) fputs (line,tmp_fp);
	   continue;

	}

	name = strtok(NULL, "\t =");
 	val = strtok(NULL, "\t ;");

#ifdef DEBUG
	(void) fprintf (stderr, "fputsym_i: type: %s\n", type);
	(void) fprintf (stderr,	"          name: %s\n", name);
	(void) fprintf (stderr,	"         value: %d\n", atoi(val));
#endif


	if (strcmp(name, symbol) != 0) {
#ifdef DEBUG
	   (void) fprintf (stderr, "fputsym_i: skipping (%s) symbol\n", name);
#endif
	   (void) fputs (line,tmp_fp);
	   continue;
	}

#ifdef DEBUG
	(void) fprintf (stderr, "fputsym_i: updating (%s) symbol\n", name);
#endif

	(void)sprintf (text, "int %s = %d;\n", symbol, value);
	(void) fputs (text,tmp_fp);
	found = TRUE;

     } /* end while */

     if ( !found ) {
	/* Looks like the symbol didn't exist in the ESPS Common file
	 * so let's append it to the end of the file. */
#ifdef DEBUG
	(void) fprintf (stderr, "putsym_i: new symbol (%s)\n", name);
#endif
	(void)sprintf (text, "int %s = %d;\n", symbol, value);
	(void) fputs (text,tmp_fp);
     }

/*
 * Now copy contents of TMP_FILE to ESPS Common file,
 * but rewind both files first
 *
 */
    (void) rewind (tmp_fp);
    (void) fclose (com_fp);

    if ((com_fp = fopen (filename, "w")) == NULL) {
	(void) fprintf (stderr,
	"fputsym_i: could not open file %s for writing.\n",
	filename);
	return -1;
    }

    while (fgets(line, LINE_SIZE, tmp_fp) != NULL) {
	(void)fprintf (com_fp, "%s", line);
#ifdef DEBUG
	(void)fprintf (stderr, "%s", line);
#endif
    }

    (void) fclose (tmp_fp);
    (void) fclose (com_fp);
    (void) unlink (template);

    return 0;

} /* end putsym_i() */



int
fputsym_s (symbol, value, file)
char	*symbol;
char	*value;
char	*file;
{

char	buffer[LINE_SIZE];	/* buffer to hold output text */
char	*text = buffer;		/* pointer to buffer */
char	*name = buffer;		/* string for holding name of symbol */
char	line[LINE_SIZE];


char	*type;
char	*val;
int	found = FALSE;
int     ret;

char	DQUOTE = '"';
    spsassert(symbol != NULL, "fputsym_s: symbol is NULL");
    spsassert(file, "fputsym_s: filename is NULL");

    if((ret = setup_common(file)) != 1) return ret;

/*
 * we've opened the file, now update it.
 * But first check if symbol already exists in the ESPS Common file.
 *
 */

#ifdef DEBUG
    (void) fprintf (stderr,
    "\nfputsym_s: ESPS Common file %s has been opened for updating.\n",
    filename);
#endif

     while (fgets(line, LINE_SIZE, com_fp) != NULL) {

	(void) strcpy (buffer, line);
 	if ( strcmp("string", (type = strtok(buffer, " \t"))) != 0 ) {
 
#ifdef DEBUG
   (void) fprintf (stderr, "fputsym_s: skipping (%s) type\n", type);
#endif
	   (void)fprintf (tmp_fp, "%s", line);
	   continue;

	}

	name = strtok(NULL, "\t =");
 	val = strtok(NULL, "\t ;");

#ifdef DEBUG
	(void) fprintf (stderr, "fputsym_s: type: %s\n", type);
	(void) fprintf (stderr,	"          name: %s\n", name);
	(void) fprintf (stderr,	"         value: %s\n", val);
#endif


	if ( strcmp(name, symbol) != 0) {
#ifdef DEBUG
	   (void) fprintf (stderr, "fputsym_s: skipping (%s) symbol\n", name);
#endif
	   (void) fprintf (tmp_fp, "%s", line);
	   continue;
	}

#ifdef DEBUG
	(void) fprintf (stderr, "fputsym_s: updating (%s) symbol\n", name);
#endif

	(void)sprintf (text, "string %s = %c%s%c;\n", symbol, DQUOTE, value, DQUOTE);
	(void) fprintf (tmp_fp, "%s", text);
	found = TRUE;

     } /* end while */

     if ( !found ) {
	/* Looks like the symbol didn't exist in the ESPS Common file
	 * so let's append it to the end of the file. */
#ifdef DEBUG
	(void) fprintf (stderr, "fputsym_s: new symbol (%s)\n", name);
#endif
	(void)sprintf (text, "string %s = %c%s%c;\n", symbol, DQUOTE, value, DQUOTE);
	(void) fprintf (tmp_fp, "%s", text);
     }

/*
 * Now copy contents of TMP_FILE to ESPS Common file,
 * but rewind both files first
 *
 */
    (void) rewind (tmp_fp);
    (void) fclose (com_fp);

    if ((com_fp = fopen (filename, "w")) == NULL) {
	(void) fprintf (stderr,
	"fputsym_s: could not open file %s for writing.\n",
	filename);
	return -1;
    }

    while (fgets(line, LINE_SIZE, tmp_fp) != NULL) {
	(void)fprintf (com_fp, "%s", line);
#ifdef DEBUG
	(void)fprintf (stderr, "%s", line);
#endif
    }

    (void) fclose (tmp_fp);
    (void) fclose (com_fp);
    (void) unlink (template);

    return 0;

} /* end fputsym_s() */

int
putsym_s (symbol, value)
char	*symbol;
char	*value;
{

char	buffer[LINE_SIZE];	/* buffer to hold output text */
char	*text = buffer;		/* pointer to buffer */
char	*name = buffer;		/* string for holding name of symbol */
char	line[LINE_SIZE];


char	*type;
char	*val;
int	found = FALSE;
int     ret;

char	DQUOTE = '"';
    spsassert(symbol != NULL, "putsym_s: symbol is NULL");

    if((ret = setup_common(NULL)) != 1) return ret;


/*
 * we've opened the file, now update it.
 * But first check if symbol already exists in the ESPS Common file.
 *
 */

#ifdef DEBUG
    (void) fprintf (stderr,
    "\nputsym_s: ESPS Common file %s has been opened for updating.\n",
    filename);
#endif

     while (fgets(line, LINE_SIZE, com_fp) != NULL) {

	(void) strcpy (buffer, line);
 	if ( strcmp("string", (type = strtok(buffer, " \t"))) != 0 ) {
 
#ifdef DEBUG
   (void) fprintf (stderr, "putsym_s: skipping (%s) type\n", type);
#endif
	   (void)fprintf (tmp_fp, "%s", line);
	   continue;

	}

	name = strtok(NULL, "\t =");
 	val = strtok(NULL, "\t ;");

#ifdef DEBUG
	(void) fprintf (stderr, "putsym_s: type: %s\n", type);
	(void) fprintf (stderr,	"          name: %s\n", name);
	(void) fprintf (stderr,	"         value: %s\n", val);
#endif


	if ( strcmp(name, symbol) != 0) {
#ifdef DEBUG
	   (void) fprintf (stderr, "putsym_s: skipping (%s) symbol\n", name);
#endif
	   (void) fprintf (tmp_fp, "%s", line);
	   continue;
	}

#ifdef DEBUG
	(void) fprintf (stderr, "putsym_s: updating (%s) symbol\n", name);
#endif

	(void)sprintf (text, "string %s = %c%s%c;\n", symbol, DQUOTE, value, DQUOTE);
	(void) fprintf (tmp_fp, "%s", text);
	found = TRUE;

     } /* end while */

     if ( !found ) {
	/* Looks like the symbol didn't exist in the ESPS Common file
	 * so let's append it to the end of the file. */
#ifdef DEBUG
	(void) fprintf (stderr, "putsym_s: new symbol (%s)\n", name);
#endif
	(void)sprintf (text, "string %s = %c%s%c;\n", symbol, DQUOTE, value, DQUOTE);
	(void) fprintf (tmp_fp, "%s", text);
     }

/*
 * Now copy contents of TMP_FILE to ESPS Common file,
 * but rewind both files first
 *
 */
    (void) rewind (tmp_fp);
    (void) fclose (com_fp);

    if ((com_fp = fopen (filename, "w")) == NULL) {
	(void) fprintf (stderr,
	"putsym_s: could not open ESPS Common file %s for writing.\n",
	filename);
	return -1;
    }

    while (fgets(line, LINE_SIZE, tmp_fp) != NULL) {
	(void)fprintf (com_fp, "%s", line);
#ifdef DEBUG
	(void)fprintf (stderr, "%s", line);
#endif
    }

    (void) fclose (tmp_fp);
    (void) fclose (com_fp);
    (void) unlink (template);

    return 0;

} /* end putsym_s() */


int
putsym_f (symbol, value)
char	*symbol;
float	value;
{

char	buffer[LINE_SIZE];	/* buffer to hold output text */
char	*text = buffer;		/* pointer to buffer */
char	*name = buffer;		/* string for holding name of symbol */
char	line[LINE_SIZE];


char	*type;
char	*val;

int	found = FALSE;
int     ret;

    spsassert(symbol != NULL, "putsym_f: symbol is NULL");
    if((ret = setup_common(NULL)) != 1) return ret;

/*
 * we've opened the file, now update it.
 * But first check if symbol already exists in the ESPS Common file.
 *
 */

#ifdef DEBUG
   (void) fprintf(stderr,"putsym_f: value: %f, symbol: %s\n",value,symbol);
#endif

     while (fgets(line, LINE_SIZE, com_fp) != NULL) {

	(void) strcpy (buffer, line);
 	if ( strcmp("float", (type = strtok(buffer, " \t"))) != 0 ) {
 
#ifdef DEBUG
   (void) fprintf (stderr, "putsym_f: skipping (%s) type\n", type);
#endif
	   (void) fputs (line,tmp_fp);
	   continue;

	}

	name = strtok(NULL, "\t =");
 	val = strtok(NULL, "\t ;");

#ifdef DEBUG
	(void) fprintf (stderr, "putsym_f: val (string): %s\n",val);
	(void) fprintf (stderr, "putsym_f: type: %s\n", type);
	(void) fprintf (stderr,	"          name: %s\n", name);
	(void) fprintf (stderr,	"         value: %f\n", (float)atof(val));
#endif


	if (strcmp(name, symbol) != 0) {
#ifdef DEBUG
	   (void) fprintf (stderr, "putsym_f: skipping (%s) symbol\n", name);
#endif
	   (void) fputs (line,tmp_fp);
	   continue;
	}

#ifdef DEBUG
	(void) fprintf (stderr, "putsym_f: updating (%s) symbol\n", name);
#endif

	(void)sprintf (text, "float %s = %f;\n", symbol, value);
	(void) fputs (text,tmp_fp);
#ifdef DEBUG
        (void) fprintf(stderr,"text: %s\n",text);
#endif
	found = TRUE;

     } /* end while */

     if ( !found ) {
	/* Looks like the symbol didn't exist in the ESPS Common file
	 * so let's append it to the end of the file. */
#ifdef DEBUG
	(void) fprintf (stderr, "putsym_f: new symbol (%s)\n", name);
#endif
	(void)sprintf (text, "float %s = %f;\n", symbol, value);
	(void) fputs (text,tmp_fp);
     }

/*
 * Now copy contents of TMP_FILE to ESPS Common file,
 * but rewind both files first
 *
 */
    (void) rewind (tmp_fp);
    (void) fclose (com_fp);

    if ((com_fp = fopen (filename, "w")) == NULL) {
	(void) fprintf (stderr,
	"putsym_f: could not open ESPS Common file %s for writing.\n",
	filename);
	return -1;
    }

    while (fgets(line, LINE_SIZE, tmp_fp) != NULL) {
	(void)fprintf (com_fp, "%s", line);
#ifdef DEBUG
	(void)fprintf (stderr, "%s", line);
#endif
    }

    (void) fclose (tmp_fp);
    (void) fclose (com_fp);
    (void) unlink (template);

    return 0;

} /* end putsym_f() */


int
fputsym_f (symbol, value, file)
char	*symbol;
float	value;
char	*file;
{


char	buffer[LINE_SIZE];	/* buffer to hold output text */
char	*text = buffer;		/* pointer to buffer */
char	*name = buffer;		/* string for holding name of symbol */
char	line[LINE_SIZE];


char	*type;
char	*val;

int	found = FALSE;
int     ret;

    spsassert(symbol != NULL, "fputsym_f: symbol is NULL");
    spsassert(file, "fputsym_f: filename is NULL");

    if((ret = setup_common(file)) != 1) return ret;


/*
 * we've opened the file, now update it.
 * But first check if symbol already exists in the ESPS Common file.
 *
 */

#ifdef DEBUG
   (void) fprintf(stderr,"fputsym_f: value: %f, symbol: %s\n",value,symbol);
#endif

     while (fgets(line, LINE_SIZE, com_fp) != NULL) {

	(void) strcpy (buffer, line);
 	if ( strcmp("float", (type = strtok(buffer, " \t"))) != 0 ) {
 
#ifdef DEBUG
   (void) fprintf (stderr, "putsym_f: skipping (%s) type\n", type);
#endif
	   (void) fputs (line,tmp_fp);
	   continue;

	}

	name = strtok(NULL, "\t =");
 	val = strtok(NULL, "\t ;");

#ifdef DEBUG
	(void) fprintf (stderr, "val (string): %s\n", val);
	(void) fprintf (stderr, "putsym_f: type: %s\n", type);
	(void) fprintf (stderr,	"          name: %s\n", name);
	(void) fprintf (stderr,	"         value: %f\n", (float)atof(val));
#endif


	if (strcmp(name, symbol) != 0) {
#ifdef DEBUG
	   (void) fprintf (stderr, "fputsym_f: skipping (%s) symbol\n", name);
#endif
	   (void) fputs (line,tmp_fp);
	   continue;
	}

#ifdef DEBUG
	(void) fprintf (stderr, "putsym_f: updating (%s) symbol\n", name);
#endif

	(void)sprintf (text, "float %s = %f;\n", symbol, value);
#ifdef DEBUG
        (void) fprintf(stderr,"text: %s\n",text);
#endif
	(void) fputs (text,tmp_fp);
	found = TRUE;

     } /* end while */

     if ( !found ) {
	/* Looks like the symbol didn't exist in the ESPS Common file
	 * so let's append it to the end of the file. */
#ifdef DEBUG
	(void) fprintf (stderr, "putsym_f: new symbol (%s)\n", name);
#endif
	(void)sprintf (text, "float %s = %f;\n", symbol, value);
	(void) fputs (text,tmp_fp);
     }

/*
 * Now copy contents of TMP_FILE to ESPS Common file,
 * but rewind both files first
 *
 */
    (void) rewind (tmp_fp);
    (void) fclose (com_fp);

    if ((com_fp = fopen (filename, "w")) == NULL) {
	(void) fprintf (stderr,
	"putsym_f: could not open ESPS Common file %s for writing.\n",
	filename);
	return -1;
    }

    while (fgets(line, LINE_SIZE, tmp_fp) != NULL) {
	(void)fprintf (com_fp, "%s", line);
#ifdef DEBUG
	(void)fprintf (stderr, "%s", line);
#endif
    }

    (void) fclose (tmp_fp);
    (void) fclose (com_fp);
    (void) unlink (template);

    return 0;

} /* end putsym_f() */
