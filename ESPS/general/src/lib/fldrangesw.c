/*
 * This material contains proprietary software of Entropic Speech,
 * Inc.  Any reproduction, distribution, or publication without the
 * prior written permission of Entropic Speech, Inc. is strictly
 * prohibited.  Any public distribution of copies of this work
 * authorized in writing by Entropic Speech, Inc. must bear the
 * notice
 *
 *  "Copyright (c) 1988 Entropic Speech, Inc. All rights reserved."
 *
 *
 *  fld_range_switch() - parse a string conaining a feature-file field name
 *  and a "generic" subrange.
 *
 *  Rodney W. Johnson, Entropic Speech, Inc.
 *
 */

#ifndef lint
    static char *sccs_id = "%W 1/10/90 ESI";
#endif

#include <stdio.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/esps.h>

long    *grange_switch();
char    *savestring();
short	get_fea_type();
long	get_fea_siz();

long *
fld_range_switch(text, name, array_len, hd)
    char    *text;
    char    **name;
    long    *array_len;
    struct header   *hd;
{
    char    *string;		/* copy of text argument */
    char    *gen_range;		/* bracketed substring of text argument */
    long    *elements;		/* array of element numbers (return value) */
    long    i;			/* loop index */

    string = savestring(text);
    spsassert(string,
	    "can't allocate space for copy of input string");
    *name = savestring(strtok(string, "["));
    spsassert(*name,
	    "can't allocate space for field name");
    gen_range = strtok((char *) 0, "]");

    if (gen_range)
    	elements = grange_switch(gen_range, array_len);
    else
    if (get_fea_type(*name, hd) == UNDEF)
    {
	*array_len = 0;
	elements = NULL;
    }
    else
    {
	*array_len = get_fea_siz(*name, hd, (short *)NULL, (long **)NULL);
	elements = malloc_l((unsigned) *array_len);
	spsassert(elements,
		"can't allocate space for array of element numbers");
	for (i = 0; i < *array_len; i++)
	    elements[i] = i;
    }

    free(string);
    return elements;
}
