/*----------------------------------------------------------------------------
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
 	

			D_MAT_ALLOC  and D_MAT_FREE

memory management subroutines for pointers to matrices containing double
precison numbers

Written by: Bernard G. Fraenkel
----------------------------------------------------------------------------*/



#ifndef lint
	static char *sccsid = "@(#)matalloc.c	1.4	7/30/91 ESI";
#endif
#include <stdio.h>
#include <math.h>


/*----------------------------------------------------------------------------

			D_MAT_ALLOC

 memory allocater for a pointer to a matrix of doubles   
Inputs:						   
    nb_row: number of rows in the matrix		   
    nb_col: number of columns in the matrix		   
Outputs:						   
    d_mat_alloc returns a pointer to a matrix of doubles 
							   

Written by: Bernard G. Fraenkel
----------------------------------------------------------------------------*/

double **d_mat_alloc (nb_row, nb_col)
int     nb_row, nb_col;

{
    double **mat;
    int     i;

    if ((mat = (double **) malloc (nb_row * sizeof (double *))) == NULL) {
	(void)fprintf(stderr,"row: %d, col: %d\n",nb_row, nb_col);
	return (NULL);
    }

    for (i = 0; i < nb_row; i++)
	if ((mat[i] = (double *) malloc (nb_col * sizeof (double))) == NULL){
	  (void)fprintf(stderr,"row: %d, col: %d\n",nb_row, nb_col);
	  return (NULL);
    }

    return (mat);
}


/* ------------------------------------------------------------------------ */

/*----------------------------------------------------------------------------

			D_MAT_FREE

 frees a pointer to a matrix of doubles   
Inputs:				    
    mat: pointer to a matrix of doubles   
    nb_row: number of rows in that matrix 

Written by: Bernard G. Fraenkel
----------------------------------------------------------------------------*/

d_mat_free (mat, nb_row)
double **mat;
int     nb_row;

{
    int     i;

    for (i = 0; i < nb_row; i++)
	free ((char *)mat[i]);

    free ((char *)mat);

    return (0);
}

/* ------------------------------------------------------------------------ */

