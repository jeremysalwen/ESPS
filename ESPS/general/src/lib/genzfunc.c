/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1987, 1990 Entropic Speech, Inc.; All rights reserved"
*/
#ifndef lint
static char *sccs_id = "@(#)genzfunc.c	1.7	10/19/93 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>

#ifndef DEC_ALPHA
char *malloc(), *strcat(), *strcpy();
void free();
#endif
void f_copy();


void
add_genzfunc(name, filter, hd)
char *name;			/* base name of filter */
struct zfunc *filter;			/* filter to add */
struct header *hd;		/* SPS file header */
{
/* this function takes a zfunc and a basename (name) and it creates
 * generic header items to store the filter information; the number 
 * of zeros and poles are stored in name_siz, the zeros in name_zeros,
 * and the poles in name_poles; the inverse function is get_genhd_z()
 */
    short *pre_siz;
    float *pre_zeros, *pre_poles;
    char *basename;		/* base name of filter */

/* check arguments
*/
    spsassert(name != NULL,"add_genzfunc: name NULL");
    spsassert(hd != NULL,"add_genzfunc: hd NULL");
    spsassert(filter != NULL,"add_genzfunc: filter NULL");

    basename = malloc((unsigned)strlen(name) + 7);
    spsassert(basename != NULL, "add_genzfunc: memory allocation failed");

    (void) strcpy(basename, name);
    pre_siz = add_genhd_s(strcat(basename, "_siz"), (short *)NULL, 2, hd);
    pre_siz[0] = filter->nsiz;
    pre_siz[1] = filter->dsiz;

    if(pre_siz[0]) {
    (void) strcpy(basename, name);
    pre_zeros = add_genhd_f(strcat(basename, "_zeros"), 
			    filter->zeros, (int)pre_siz[0], hd);
    }

    if(pre_siz[1]) {
    (void) strcpy(basename, name);
    pre_poles = add_genhd_f(strcat(basename, "_poles"), 
			    filter->poles, (int)pre_siz[1], hd);

    }
    free(basename);
}


struct zfunc *
get_genzfunc(name, hd)
char *name;			/* base name of zfunc */
struct header *hd;		/* pointer to SPS header */
{
/* this function accepts the basename (name) of a filter, reads 
 * SPS header items for the number of poles and zeros and also 
 * for the poles and zeros themselves; this information is converted 
 * to a zfunc and returned.
 */
    short *pre_siz;
    float *pre_zero, *pre_pol;
    int size, gotzfunc = 0;
    char *basename_siz, *basename_poles, *basename_zeros;		

/* check arguments
*/
    spsassert(name != NULL,"get_genzfunc: name NULL");
    spsassert(hd != NULL,"get_genzfunc: hd NULL");

    basename_siz = malloc((unsigned)strlen(name) + 5);
    spsassert(basename_siz != NULL, "get_genzfunc: memory allocation failed");

    (void) strcpy(basename_siz, name);
    (void) strcat(basename_siz, "_siz");

    if (genhd_type(basename_siz, &size, hd) == SHORT && size == 2) {
	gotzfunc = 1;
	basename_poles = malloc((unsigned)strlen(name) + 7);
	basename_zeros = malloc((unsigned)strlen(name) + 7);

	spsassert(basename_poles != NULL, 
		"get_genzfunc: memory allocation failed");
	spsassert(basename_zeros != NULL, 
		"get_genzfunc: memory allocation failed");

	(void) strcpy(basename_poles, name);
	(void) strcat(basename_poles, "_poles");
	(void) strcpy(basename_zeros, name);
	(void) strcat(basename_zeros, "_zeros");
    	pre_siz = (short *)get_genhd(basename_siz, hd);

    	if (pre_siz[0] > 0 
	      && genhd_type(basename_zeros, &size, hd) == FLOAT
	      && size == pre_siz[0])
 	    pre_zero = (float *)get_genhd(basename_zeros, hd);
	else
	    gotzfunc = 0;

	free(basename_zeros);

    	if (pre_siz[1] > 0 
	      && genhd_type(basename_poles, &size, hd) == FLOAT
	      && size == pre_siz[1])
	    pre_pol = (float *)get_genhd(basename_poles, hd);
	else
	    gotzfunc = 0;

	free(basename_poles);
    }

    free(basename_siz);

    if (gotzfunc) 
    	return new_zfunc(pre_siz[0], pre_siz[1], pre_zero, pre_pol);
    else
	return NULL;
}


