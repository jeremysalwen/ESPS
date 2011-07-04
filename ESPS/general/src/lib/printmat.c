/*	
 This material contains proprietary software of Entropic Speech, Inc.
 Any reproduction, distribution, or publication without the the prior	   
 written permission of Entropic Speech, Inc. is strictly prohibited.
 Any public distribution of copies of this work authorized in writing by
 Entropic Speech, Inc. must bear the notice			

	Copyright 1986, Entropic Speech, Inc; All Rights Reserved

									    
			    PRINT_MAT

 prints a matrix on the history file if the debug_level is greater than 3 
Inputs:								    
    mat: matrix pointer to the matrix to print			    
    size: size of the matrix						    
									    
Written by: Bernard G. Fraenkel

----------------------------------------------------------------------------*/


#ifndef lint
	static char *sccsid = "@(#)printmat.c	1.4	5/3/91 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>

extern int debug_level;

print_mat (mat, size)
double  **mat;
int     size;

{
    int     i, j, n;

    spsassert(mat != NULL, "print_mat: mat NULL");
    n = size - 1;

    if (debug_level >= 3) {
	for (i = 0; i < size; i++)
	    for (j = 0; j < size; j++)
		(void)fprintf (stderr, "%-14.8lg%c", 
			mat[i][j], (j == n ? '\n' : ' '));

    }
    return (0);
}


