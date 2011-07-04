/*
  This material contains proprietary software of Entropic Speech, Inc.
  Any reproduction, distribution, or publication without the prior
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice

      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"

   grange_switch() - extract a "generic" range from text.

   author:  Ajaipal S. Virdy, Entropic Speech, Inc.

*/

#ifndef lint
    static char *sccs_id = "@(#)grngswitch.c	1.6 11/9/92 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif


/* ESPS functions referenced */

void	lrange_switch();
char	*savestring();


long *
grange_switch (text, n_ele)
char	*text;
long	*n_ele;
{
	long	i;

	long	start=-1;
	long	end=-1;

	long	n_elements;
	long	npoints;

	char	*range;
	long	*array;

	char	*Module = "grange_switch";

		/* string to hold range between commas ','  */
	char	*token;
	
	char	*save_text;

/*
 * First determine how much memory we need to allocate by
 * parsing the text string, then fill in the array by
 * re-parsing the text string.
 */

	save_text = savestring (text);

	range = savestring(text);
	token = strtok (range, ", ");
	lrange_switch (token, &start, &end, 1);
	if (token && *token && token[strlen(token)-1] == ':') {
		fprintf(stderr,
		    "grange_switch: argument of type \"a:\" not allowed.\n");
	 	exit(-1);
	}
	npoints = end - start + 1;

	while (token = strtok (0, ",")) {
		lrange_switch (token, &start, &end, 1);
		if (token && *token && token[strlen(token)-1] == ':') {
		    fprintf(stderr,
		    "grange_switch: argument of type \"a:\" not allowed.\n");
	 	    exit(-1);
		}
		npoints += end - start + 1;
	}
	free (range);

/*
 * Now allocate memory for npoints.
 */
	if ((array = (long *) calloc ((unsigned int) npoints, sizeof (long))) == NULL)
	{
	   (void) fprintf (stderr,
	   "%s: calloc: could not allocate memory for array.\n",
	   Module);
	   exit (1);
	}

/*
 * Go back to original string, text, and re-parse the ranges
 * and fill in the array with the element values.
 *
 */

	range = save_text;
	token = strtok (range, ", ");
	lrange_switch (token, &start, &end, 1);
	npoints = end - start + 1;

	n_elements = 0;
	for (i = 0; i < npoints; i++)
	    array[n_elements++] = start + i;

	while (token = strtok (0, ",")) {
		lrange_switch (token, &start, &end, 1);
		npoints = end - start + 1;
		for (i = 0; i < npoints; i++)
		    array[n_elements++] = start + i;
	}
	free (range);

	*n_ele = n_elements;
	return (array);
}
