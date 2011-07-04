/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	
|			
| Written by:  John Shore
|  				
| Module:	fmatalloc.c - contains f_mat_alloc and f_mat_free
|
| memory management subroutines for pointers to matrices containing 
| floating point numbers; The code was modified from the matalloc.c 
| code (doubles) by B. Fraenkel.  
*/

#ifndef lint
static char *sccs_id = "@(#)fmatalloc.c	1.5	5/21/91 ESI";
#endif

#include <stdio.h>

char *malloc();
void free();

/*
|			F_MAT_ALLOC
|
| memory allocater for a pointer to a matrix of floats 
|Inputs:						   
|    nb_row: number of rows in the matrix		   
|    nb_col: number of columns in the matrix		   
|Outputs:						   
|    f_mat_alloc returns a pointer to a matrix of floats
|
*/

float **f_mat_alloc (nb_row, nb_col)
unsigned     nb_row;
unsigned nb_col;

{
    float **mat;
    unsigned    i;

    if ((mat = (float **) malloc (nb_row * sizeof (float *))) == NULL)
	return (NULL);

    for (i = 0; i < nb_row; i++)
	if ((mat[i] = (float *) malloc (nb_col * sizeof (float))) == NULL)
	    return (NULL);

    return (mat);
}



/*
|
|			F_MAT_FREE
|
| frees a pointer to a matrix of floats   
|Inputs:				    
|    mat: pointer to a matrix of floats   
|    nb_row: number of rows in that matrix 
*/

f_mat_free (mat, nb_row)
float **mat;
int     nb_row;

{
    int     i;

    for (i = 0; i < nb_row; i++)
	free ((char *)mat[i]);

    free ((char *)mat);

    return (0);
}

