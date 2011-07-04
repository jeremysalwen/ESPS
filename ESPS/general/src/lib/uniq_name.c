/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1986, 1987 Entropic Speech, Inc.; All rights reserved"
 *
 *
 * Written by:  John Shore
 * Checked by:	Alan Parker
 *
 * This is uniq_name.c, which returns a unique name from a list of names
 * by appending a digit if necessary.  
 */
#ifndef lint
	static char *sccs_id = "@(#)uniq_name.c	1.8	10/19/93 ESI";
#endif

#include <stdio.h>
#include <assert.h>
#include <esps/esps.h>
#define MAX_NUMSTRING 10    /*maximum additional numerical characters*/

/*
 * system functions and variables
 */

int atoi(), strncmp(), strlen();
#ifndef DEC_ALPHA
char *strncpy(), *strcpy();
char *malloc();
#endif
/*
 * external SPS functions
 */
char **genhd_list();

char *
uniq_name(name_in, names)
char *name_in;			/* input name */
char **names;			/* list of existing names */
{
    int n;			/* numerical suffix of existing generic */
    char nstring[MAX_NUMSTRING];/* string for numerical increment */
    char *uniqname;		/* string for returned name*/
    int nums;			/* flag for numerical suffix */
    int found = 0;		/* flag for whether name already exists */
    int maxn = 0;  		/* maximum numerical suffix */
    int in_length;		/* length of input name*/
    int base_length;		/* length of base-name part of input name */
    int gen_length;		/* length of existing generic name */
    register int i, j;

    in_length = strlen(name_in);
    for (i = in_length - 1;
	 i >= 0 && name_in[i] >= '0' && name_in[i] <= '9';
	 i--
	) { }
    base_length = i+1;

    /*
     * get list of existing generic header item names
     */
    for (i = 0; names[i] != NULL; i++) {
	if (strncmp(names[i], name_in, base_length) == 0) {
	    /*
	     * base name of existing generic matches input name;
	     */
	    if (strcmp(names[i], name_in) == 0)
		found++;    /* existing name is exact match */
	    gen_length = strlen(names[i]);
	    if (gen_length > base_length)
	    {
		/* existing name has extra characters;
		 * check to see if they are all digits
		 */
		nums = 1;
		for (j = base_length; j < gen_length; j++) 
		    if (names[i][j] < '0' || names[i][j] > '9') 
			nums = 0;
		/* if extra characters are digits, turn them into
		 * a number and see if it's the biggest so far
		 */
		if (nums) {
		    n = atoi(&names[i][base_length]);
		    if (n > maxn) maxn = n;
		}
	    }
	}
    }
    if (found) {
	/*
	 * match was found in existing names;
	 * increment the largest numerical suffix;
	 */
        maxn++;
	(void) sprintf(nstring, "%d", maxn);
	/*
	 * copy input name as start on unique name
	 */
        uniqname = malloc((unsigned)base_length + strlen(nstring) + 1);
	assert(uniqname != NULL);
        (void) strncpy(uniqname, name_in, base_length);  
	/* 
	 * append suffix to the base name
	 */
	(void) strcpy(uniqname + base_length, nstring);
	return uniqname;
    }
    else
        return name_in;
}
