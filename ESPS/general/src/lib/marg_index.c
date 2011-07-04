/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
|    "Copyright (c) 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  marg_index.c								|
|									|
|  by Rodney Johnson							|
|									|
|  The function marg_index() creates an array of pointers that permits	|
|  accessing a linear block of storage as a multidimensional array.	|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)marg_index.c	1.5	12/19/95	ESI";
#endif

#include <stdio.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/esps.h>

#define ERROR(text) \
{(void) fprintf(stderr, "%s: %s\n", PROG, text); exit(1);}

#define SWITCH(star) \
        switch (typ) \
        { \
        case DOUBLE:	    CASE(double star) \
        case FLOAT:	    CASE(float star) \
        case LONG:	    CASE(long star) \
        case SHORT:	    \
        case CODED:	    CASE(short star) \
        case CHAR:	    \
        case BYTE:	    CASE(char star) \
	case DOUBLE_CPLX:   CASE(double_cplx star) \
	case FLOAT_CPLX:    CASE(float_cplx star) \
	case LONG_CPLX:	    CASE(long_cplx star) \
	case SHORT_CPLX:    CASE(short_cplx star) \
	case BYTE_CPLX:	    CASE(byte_cplx star) \
        default:	    ERROR("unrecognized type") \
        } \
        break;

/* Provide a way to give SWITCH an empty argument. */

#define VOID

#define PROG "marg_index"
#define CASE(type) \
	    if ((ind = calloc((unsigned) prod, sizeof(type *))) == NULL) \
		ERROR("unable to allocate storage"); \
	    { \
		type **a = (type **) ind; \
		type *p = (type *) arr; \
		for (i = 0; i < prod; i++) \
		{ \
		    a[i] = p; \
		    p += step; \
		} \
	    } \
	    break;

char *
marg_index(arr, rk, dim, typ, lvl)
    char    *arr;
    int	    rk;
    long    *dim;
    int	    typ, lvl;
{
    int     i, prod, step;
    char    *ind;

    if (rk < 1) ERROR("rank < 1")
    if (lvl < 0) ERROR("level < 0")
    if (rk == 1) return arr;

    prod = 1;
    for (i = 0; i < rk-1; i++) prod *= dim[i];
    step = dim[rk-1];

    switch (lvl)
    {
    case 0:	SWITCH(VOID)
    case 1:	SWITCH(*)
    default:	CASE(char **)
    }
    return marg_index(ind, rk-1, dim, typ, lvl+1);
}

#undef CASE
#undef PROG


#define PROG "arr_alloc"
#define CASE(type) \
	    if ((arr = calloc((unsigned) prod, sizeof (type))) == NULL) \
		ERROR("unable to allocate storage"); \
	    break;

char *
arr_alloc(rk, dim, typ, lvl)
    int     rk;
    long    *dim;
    int	    typ, lvl;
{
    int	    i, prod;
    char    *arr;

    if (rk < 1) ERROR("rank < 1")
    if (lvl < 0) ERROR("level < 0")

    prod = 1;
    for (i = 0; i < rk; i++) prod *= dim[i];
    
    switch (lvl)
    {
    case 0:	SWITCH(VOID)
    case 1:	SWITCH(*)
    default:	CASE(char **)
    }
    return marg_index(arr, rk, dim, typ, lvl);
}

#undef CASE
#undef PROG


#define PROG "arr_free"
#define CASE(type) \
	    p = (char *) * (type **) arr; \
	    break;
    
void
arr_free(arr, rk, typ, lvl)
    char    *arr;
    int     rk, typ, lvl;
{
    char    *p;

    if (rk < 0) ERROR("rank < 0")
    if (lvl < 0) ERROR("level < 0")
    if (rk == 0) return;

    switch (rk + lvl)
    {
    case 1:	{ free(arr); return; }
    case 2:	SWITCH(VOID)
    case 3:	SWITCH(*)
    default:	CASE(char **)
    }

    free(arr);
    arr_free(p, rk-1, typ, lvl);
}
