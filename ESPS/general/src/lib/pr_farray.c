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
 * Written by:  John Shore (from refcof code)
 * Checked by:
 * Revised by:
 *
 * Brief description: compute RCs by various methods
 *
 */

static char *sccs_id = "@(#)pr_farray.c	1.2	5/3/91	ERL";

#include <stdio.h>

/*
 * For debug printout of float arrays
 */

void pr_farray(arr, size, title)
    float  *arr;
    long    size;
    char    *title;

{
    int	    i;

    (void) fprintf(stderr, "%s -- %d points:\n", title, size);
    for (i = 0; i < size; i++)
    {
	(void) fprintf(stderr, "%f ", arr[i]);
	if (i%5 == 4 || i == size - 1) (void) fprintf(stderr, "\n");
    }
}

