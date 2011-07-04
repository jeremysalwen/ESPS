/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1987, 1990 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  fea_compat.c								|
|  									|
|  Rodney Johnson							|
|  									|
|  check compatibility of feature definitions in file headers		|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
static char *sccs_id = "@(#)fea_compat.c	3.9 3/2/98	ERL";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/spsassert.h>

#ifndef DEC_ALPHA
char	*malloc(), *realloc();
#endif

static int	l_arr_eq();
static int	incompat_codes();
static int	num_strings();

int
fea_compat(ihd, ohd, fields, trans)
    struct header   *ihd, *ohd;
    char    **fields;
    short   ***trans;
{
    int	    c_lvl;		/* returned compatibility level (-1,0,1) */
    int	    tbl_req;		/* translation table required? */
    int	    n_fields;
    char    **ienum, **oenum;
    int	    n_icodes, n_ocodes;
    char    *string;
    int	    ic, oc;
    int	    i, j, k;

    spsassert(ihd, "input header NULL");
    spsassert(ihd->common.type == FT_FEA, "input header not of type FEA.");
    spsassert(ohd, "output header NULL");
    spsassert(ohd->common.type == FT_FEA, "output header not of type FEA.");

    if (fields == NULL) fields = ihd->hd.fea->names;

    /* Check compatibility. */

    tbl_req = NO;
    c_lvl = 1;

    for	(   k = 0;
	    fields[k] != NULL
		&& (i = lin_search2(ihd->hd.fea->names, fields[k])) != -1
		&& (j = lin_search2(ohd->hd.fea->names, fields[k])) != -1
		&& ihd->hd.fea->types[i] == ohd->hd.fea->types[j]
		&& ihd->hd.fea->sizes[i] == ohd->hd.fea->sizes[j]
		&& ihd->hd.fea->ranks[i] == ohd->hd.fea->ranks[j]
		&& (ihd->hd.fea->ranks[i] < 2 || 
                    l_arr_eq(ihd->hd.fea->dimens[i], ohd->hd.fea->dimens[j],
			    ihd->hd.fea->ranks[i]));
	    k++)
	if (incompat_codes(ihd, i, ohd, j)) tbl_req = YES;

    if (fields[k] != NULL) c_lvl = 0;

    for ( ;
	    fields[k] != NULL
		&& (i = lin_search2(ihd->hd.fea->names, fields[k])) != -1
		&& (j = lin_search2(ohd->hd.fea->names, fields[k])) != -1
		&& ihd->hd.fea->types[i] == ohd->hd.fea->types[j]
		&& ihd->hd.fea->sizes[i] <= ohd->hd.fea->sizes[j];
	    k++)
	if (incompat_codes(ihd, i, ohd, j)) tbl_req = YES;

    if (fields[k] != NULL) return -1;

    if (!tbl_req)
	*trans = NULL;
    else
    {
	/* Make translation table and extend enums arrays in output header. */

	n_fields = num_strings(fields);
	*trans = (short **)malloc((unsigned)n_fields*sizeof(short *));
	spsassert(*trans != NULL, "Can't allocate space for table array.");

	for (k = 0; k < n_fields; k++)
	{
	    i = lin_search2(ihd->hd.fea->names, fields[k]);
	    j = lin_search2(ohd->hd.fea->names, fields[k]);
	    if (ihd->hd.fea->types[i] != CODED)
		(*trans)[k] = NULL;
	    else
	    {
		ienum = ihd->hd.fea->enums[i];
		oenum = ohd->hd.fea->enums[j];
		n_icodes = num_strings(ienum);
		n_ocodes = num_strings(oenum);

		(*trans)[k] = malloc_s((unsigned)n_icodes);
		spsassert((*trans)[k] != NULL,
			"Can't allocate space for table array element.");

		for (ic = 0; ic < n_icodes; ic++)
		{
		    string = ienum[ic];
		    oc = lin_search(oenum, string);
		    if (oc == -1)
		    {
			(*trans)[k][ic] = n_ocodes;
			oenum[n_ocodes++] = string;
			oenum = (char **) realloc((char *) oenum,
				   (unsigned)(n_ocodes + 1) * sizeof(char *));
			spsassert(oenum != NULL,
			    "Can't reallocate enums array in output header.");
			oenum[n_ocodes] = NULL;
			ohd->hd.fea->enums[j] = oenum;
		    }
		    else (*trans)[k][ic] = oc;
		}
	    }
	}
    }

    return c_lvl;
}

/* Function to compare two arrays of longs for equality */

static int
l_arr_eq(x, y, n)
    long    *x, *y;
    int	    n;
{
    int	    i;

    for (i = 0; i < n && x[i] == y[i]; i++) {}

    return i >= n;
}

/* Function to check whether coded fields have incompatible enums arrays */

static int
incompat_codes(ihd, i, ohd, j)
    struct header   *ihd, *ohd;
    int	    i, j;
{
    int	    n, k;

    if (ihd->hd.fea->types[i] != CODED) return NO;

    n = num_strings(ihd->hd.fea->enums[i]);
    if (n > num_strings(ohd->hd.fea->enums[j])) return YES;

    for (   k = 0;
	    k < n && strcmp(ihd->hd.fea->enums[i][k],
			    ohd->hd.fea->enums[j][k]) == 0;
	    k++) {}

    return k < n;
}

/* Function to compute length of NULL-terminated array of strings */

static int
num_strings(a)
    char    **a;
{
    int	    k;

    for (k = 0; a[k] != NULL; k++) {}
    return k;
}
