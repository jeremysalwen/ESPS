/*

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"

*/
 				
 	
/* ---------------------------------------------------------------------------
			PRINT_ARR  &  PRINTARR

print_arr is a new version of printarr, which is kept for compatibility 
purposes

--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------

				PRINT_ARR


Written by: Bernard G. Fraenkel
----------------------------------------------------------------------------*/


#ifdef SCCS
	static char *sccsid = "@(#)printarr.c	1.2	11/19/87";
#endif
#include <stdio.h>

extern int debug_level;


print_arr (array, format, ar_siz, filnam)
char   *filnam;
int     ar_siz;
char   *array;
char    format[3];


{
    double *pdble;
    long   *plong;
    float  *pfloat;
    int    *pint, nb_print;
    char    message[BUFSIZ];
    FILE * fp;

    if (debug_level)
	fprintf (stderr, "printing file '%s'  ", filnam);

/*   open file */
    if ((fp = fopen (filnam, "w")) == NULL) {
	perror ("print_arr: fopen error");
	strcpy (message, "can't open ");
	strcat (message, filnam);
	strcat (message, " for printing");
	faterr ("print_arr", message, -1);
    }

/* print the number of elements to put into the file */
    if (debug_level)
	fprintf (stderr, " array size is : %d   ", ar_siz);
    if (fprintf (fp, "%d\n", ar_siz) != 0) {
	perror ("print_arr: fprintf error");
	faterr ("print_arr", "can't print the dimension of the array", -3);
    }

/* print the elements of the array depending on the type of data */
/* fprintf returns 0 when successful */

    nb_print = 0;

    if (strcmp (format, "lf") == 0 || strcmp (format, "lg") == 0) {
	if (debug_level)
	    fprintf (stderr, " array type is : DOUBLE \n");
	pdble = (double *) array;
	while ((nb_print < ar_siz) && (fprintf (fp, "%-24.15lg\n", *(pdble++)) == 0))
	    nb_print++;
    }
    else if (strcmp (format, "f") == 0 || strcmp (format, "g") == 0) {
	if (debug_level)
	    fprintf (stderr, " array type is : FLOAT \n");
	pfloat = (float *) array;
    /*  warning : the order in the AND expression is important */
	while ((nb_print < ar_siz) && (fprintf (fp, "%-24.8g\n", *(pfloat++)) == 0))
	    nb_print++;
    }
    else if (strcmp (format, "ld") == 0) {
	if (debug_level)
	    fprintf (stderr, " array type is : LONG \n");
	plong = (long *) array;
	while ((nb_print < ar_siz) && (fprintf (fp, "%ld\n", *(plong++)) == 0))
	    nb_print++;
    }
    else if (strcmp (format, "d") == 0) {
	if (debug_level)
	    fprintf (stderr, " array type is : INT \n");
	pint = (int *) array;
	while ((nb_print < ar_siz) && (fprintf (fp, "%d\n", *(pint++)) == 0))
	    nb_print++;
    }
    else {
	strcpy (message, format);
	strcat (message, " is not a supported format ");
	faterr ("print_arr", message, -5);
    }

/* check that the print operation went fine */
    if (nb_print != ar_siz) {
	if (nb_print <= 0) {
	    perror ("print_arr: print error");
	    strcpy (message, "error printing in ");
	    strcat (message, filnam);
	    faterr ("print_arr", message, -7);
	}
	else {
	    perror ("print_arr: print error");
	    if (debug_level)
		fprintf (stderr, " only %d array elements printed into file  %s\n", nb_print, filnam);
	    faterr ("print_arr", "error in the dimensions of the array", -9);
	}
    }

    fclose (fp);
    return (nb_print);
}

/*    -------------------------------------------------------------   */

#define MAX_AR_SIZ 10001

printarr(array,format,ar_siz,filnam)
char *filnam;
int ar_siz;
char *array;
char format[3];

/* ----
PRINT AN ARR INTO A FILE
printarr returns the number of elements actually printed into the file
4 inputs : 
- array : a pointer to the array to be written
- format : format corresponding to the type of the elements of the array
- ar_siz : the number of elements in the array
- filnam : the filename
---- */

{
    double *pdble;
    long   *plong;
    float  *pfloat;
    int    *pint,
            nb,
            nb_print;
    char    message[BUFSIZ];
    FILE * fp;

    fprintf (stderr, "printing file '%s'  ", filnam);

/*   open file */
    if ((fp = fopen (filnam, "w")) == NULL) {
	strcpy (message, "can't open ");
	strcat (message, filnam);
	strcat (message, " for printing");
	faterr ("printarr", message, -1);
    }

/* print the number of elements to put into the file */
    nb = ar_siz;
    fprintf (stderr, " array size is : %d   ", nb);
    if (fprintf (fp, "%d\n", nb) != 0)
	faterr ("printarr", "can't print the dimension of the array", -3);
    if (nb > MAX_AR_SIZ)
	faterr ("printarr", " size of the array is too large ( > MAX_AR_SIZ)", -5);

/* print the elements of the array depending on the type of data */
/* fprintf returns 0 when successful */

    nb_print = 0;

    if (strcmp (format, "lf") == 0) {
	fprintf (stderr, " array type is : DOUBLE \n");
	pdble = (double *) array;
	while ((nb_print < nb) && (fprintf (fp, "%32.16lg\n", *(pdble++)) == 0))
	    nb_print++;
    }
    else if (strcmp (format, "f") == 0) {
	fprintf (stderr, " array type is : FLOAT \n");
	pfloat = (float *) array;
    /*  warning : the order in the AND expression is important */
	while ((nb_print < nb) && (fprintf (fp, "%24.8g\n", *(pfloat++)) == 0))
	    nb_print++;
    }
    else if (strcmp (format, "ld") == 0) {
	fprintf (stderr, " array type is : LONG \n");
	plong = (long *) array;
	while ((nb_print < nb) && (fprintf (fp, "%ld\n", *(plong++)) == 0))
	    nb_print++;
    }
    else if (strcmp (format, "d") == 0) {
	fprintf (stderr, " array type is : INT \n");
	pint = (int *) array;
	while ((nb_print < nb) && (fprintf (fp, "%d\n", *(pint++)) == 0))
	    nb_print++;
    }
    else {
	strcpy (message, format);
	strcat (message, " is not a supported format ");
	faterr ("printarr", message, -7);
    }

/* check that the print operation went fine */
    if (nb_print != ar_siz) {
	if (nb_print == 0) {
	    strcpy (message, "error printing in ");
	    strcat (message, filnam);
	    faterr ("printarr", message, -9);
	}
	else {
	    fprintf (stderr, " only %d array elements printed into file  %s\n", nb_print, filnam);
	    faterr ("printarr", "error in the dimensions of the array", -11);
	}
    }

/*   use fclose and NOT close   */
    fclose (fp);
    return (nb_print);
}

/*    -------------------------------------------------------------   */

