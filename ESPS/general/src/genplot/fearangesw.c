/*
 * This material contains proprietary software of Entropic Speech,
 * Inc.  Any reproduction, distribution, or publication without the
 * prior written permission of Entropic Speech, Inc. is strictly
 * prohibited.  Any public distribution of copies of this work
 * authorized in writing by Entropic Speech, Inc. must bear the
 * notice
 *
 *  "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."
 *
 *
 *  fea_range_switch() - extract a range of element numbers from field
 *			names and "generic" subranges.
 *
 *  Rodney W. Johnson, Entropic Speech, Inc.
 *
 */

#ifdef SCCS
    static char *sccs_id = "@(#)fearangesw.c	3.3	5/18/88	ESI";
#endif

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/fea.h>

#define PROG Fprintf(stderr, "%s: ", Module)
#define EXIT Fprintf(stderr, "\n"); exit(1);
#define ERROR_EXIT(text) {PROG; Fprintf(stderr, (text)); EXIT}
#define ERROR_EXIT1(fmt,a) {PROG; Fprintf(stderr, (fmt), (a)); EXIT}
#define ERROR_EXIT3(fmt,a,b,c) {PROG; \
	    Fprintf(stderr, (fmt), (a), (b), (c)); EXIT}
#define TRYALLOC(type,num,var,msg) { \
    if (((var) = (type *) calloc((unsigned)(num),sizeof(type))) == NULL) \
    ERROR_EXIT1("Can't allocate memory--%s", (msg))}


/* ESPS functions referenced */

long    *grange_switch();
long    get_fea_siz();
char    *savestring();

char	*Module = "fea_range_switch";


long *
fea_range_switch(fields, hd, n_ele, names, indices)
    char    **fields;
    struct header   *hd;
    long    *n_ele;
    char    ***names;
    long    **indices;
{
    int	    i, j, k;
    char    *tmp_string;	/* string to hold field name and range */
    char    *field_name;	/* string to hold field name */
    char    *gen_range;		/* string to hold generic range */

    long    *item_array;	/* array of items given in gen_range */
    long    n_item;		/* number of items given in gen_range */

    long    total_len = 0;	/* total length of fields */
    long    max_elem;		/* maximum element in gen_range requested */
    long    size;		/* size of feature field in hd */
    short   type;		/* type of feature field in hd */
    long    start;		/* start of field relative to other
				    fields of same type in record */
    long    *evec;		/* vector of element numbers */
    long    *ivec;		/* indices within fields */
    char    **nvec;		/* field names */
    long    index = 0;		/* index into evec, ivec, and nvec */

/* Compute the amount of memory required */

    for (i = 0; fields[i] != NULL; i++)
    {
	/* copy feature field name because strtok() destroys strings */

	tmp_string = savestring(fields[i]);
	field_name = strtok(tmp_string, "[");
	gen_range = strtok((char *) 0, "]");
	size = get_fea_siz(field_name, hd, (short *) NULL, (long **) NULL);

	if (gen_range != NULL)
	{
	    item_array = grange_switch(gen_range, &n_item);

	    max_elem = item_array[n_item - 1];
	    if (max_elem >= size)	/* incorrect generic range */
		ERROR_EXIT3(
		    "range of field %s is 0:%ld, requesting element %ld.",
		    field_name, size - 1, max_elem)

	    total_len += n_item;
	    free((char *) item_array);
	}
	else
	    total_len += size;

	free(tmp_string);

    }	/* end for (i ... ) */

/* Allocate memory for output arrays */

    TRYALLOC(long, total_len, evec, "result vector")
    TRYALLOC(long, total_len, ivec, "vector of item numbers")
    TRYALLOC(char *, total_len, nvec, "array of field names")

/* Get indices */

    for (i = 0; fields[i] != NULL; i++)
    {
	tmp_string = savestring(fields[i]);
	field_name = savestring(strtok(tmp_string, "["));
	gen_range = strtok((char *) 0, "]");

	if (gen_range != NULL)
	    item_array = grange_switch(gen_range, &n_item);
	else
	    n_item = get_fea_siz(field_name, hd,
			(short *) NULL, (long **) NULL);

	if ((k = lin_search2(hd->hd.fea->names, field_name)) == -1)
	    ERROR_EXIT1("field name %s not found in ESPS Feature file.",
		    field_name);

	type = hd->hd.fea->types[k];
	start = hd->hd.fea->starts[k];

	for (j = 0; j < n_item; j++)
	{
	    nvec[index] = field_name;
	    ivec[index] = (gen_range != NULL) ? item_array[j] : j;

	    evec[index] = get_fea_element(field_name,hd) + ivec[index];
	    index++;
	}

	if (gen_range != NULL) free((char *) item_array);
	free(tmp_string);

    }	/* end for (i ...) */

    if (names == NULL) free((char *) nvec); else *names = nvec;
    if (indices == NULL) free((char *) ivec); else *indices = ivec;
    *n_ele = total_len;
    return evec;
}
