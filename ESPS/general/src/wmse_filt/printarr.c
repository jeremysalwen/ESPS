
/*********************************************************
*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
*
*  Module Name: printarr.c
*
*  Written By:   Berny Fraenkel
*  Converted to ESPS by Brian Sublett   5/4/87
*  Modified for ESPS 3.0 by David Burton
*  Checked by: 
*
*  Description: This function prints an array to a file.
*
*  Secrets:  There must be an external variable called
*	     debug_level defined.
*
*********************************************************/

static char *sccsid = "@(#)printarr.c	3.3  8/8/91 ESI ESI";

/*
 * System Includes
*/
#include <stdio.h>

/*
 * ESPS Includes
*/
#include <esps/unix.h>

extern int debug_level;

#define MAX_AR_SIZ 10001    /*Maximim array size that can be printed*/
#define Fprintf (void)fprintf


int
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
    int faterr();    

    Fprintf (stderr, "printing file '%s'  ", filnam);

/*   open file */
    if ((fp = fopen (filnam, "w")) == NULL) {
	(void)strcpy (message, "can't open ");
	(void)strcat (message, filnam);
	(void)strcat (message, " for printing");
	faterr ("printarr", message, 1);
    }

/* print the number of elements to put into the file */
    nb = ar_siz;
    (void)fprintf (stderr, " array size is : %d   ", nb);
    if (fprintf (fp, "%d\n", nb) != 0)
	faterr ("printarr", "can't print the dimension of the array", 1);
    if (nb > MAX_AR_SIZ)
	faterr ("printarr", " size of the array is too large ( > MAX_AR_SIZ)", 1);

/* print the elements of the array depending on the type of data */
/* Fprintf returns 0 when successful */

    nb_print = 0;

    if (strcmp (format, "lf") == 0) {
	Fprintf (stderr, " array type is : DOUBLE \n");
	pdble = (double *) array;
	while ((nb_print < nb) && (fprintf (fp, "%32.16lg\n", *(pdble++)) == 0))
	    nb_print++;
    }
    else if ((strcmp (format, "f") == 0) || (strcmp (format, "g") == 0)) {
	Fprintf (stderr, " array type is : FLOAT \n");
	pfloat = (float *) array;
    /*  warning : the order in the AND expression is important */
	while ((nb_print < nb) && (fprintf (fp, "%24.8g\n", *(pfloat++)) == 0))
	    nb_print++;
    }
    else if (strcmp (format, "ld") == 0) {
	Fprintf (stderr, " array type is : LONG \n");
	plong = (long *) array;
	while ((nb_print < nb) && (fprintf (fp, "%ld\n", *(plong++)) == 0))
	    nb_print++;
    }
    else if (strcmp (format, "d") == 0) {
	Fprintf (stderr, " array type is : INT \n");
	pint = (int *) array;
	while ((nb_print < nb) && (fprintf (fp, "%d\n", *(pint++)) == 0))
	    nb_print++;
    }
    else {
	(void)strcpy (message, format);
	(void)strcat (message, " is not a supported format ");
	faterr ("printarr", message, 1);
    }

/* check that the print operation went fine */
    if (nb_print != ar_siz) {
	if (nb_print == 0) {
	    (void)strcpy (message, "error printing in ");
	    (void)strcat (message, filnam);
	    faterr ("printarr", message, 1);
	}
	else {
	    Fprintf (stderr, " only %d array elements printed into file  %s\n", nb_print, filnam);
	    faterr ("printarr", "error in the dimensions of the array", 1);
	}
    }

/*   use fclose and NOT close   */
    (void)fclose (fp);
    return (nb_print);
}
