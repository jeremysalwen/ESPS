/*----------------------------------------------------------------------+
|									|
|   This material contains proprietary software of Entropic Speech,	|
|   Inc.  Any reproduction, distribution, or publication without the	|
|   prior written permission of Entropic Speech, Inc. is strictly	|
|   prohibited.  Any public distribution of copies of this work		|
|   authorized in writing by Entropic Speech, Inc. must bear the	|
|   notice								|
|									|
| "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."	|
|									|
+-----------------------------------------------------------------------+
|									|
|  print_time -- print time in a form suitable for plotas		|
|									|
|  Shankar Narayan, EPI							|
|  Adapted by Joseph T. Buck.		 				|
|									|
+----------------------------------------------------------------------*/
#ifndef lint
    static char *sccs_id = "@(#)printtime.c	3.1	10/20/87	ESI";
#endif

#define Printf (void) printf

print_time(x, y)
    int     x, y;		/* coordinates of beginning of string */
{
    long    time (), tloc;
    char    *ctime (), *tptr, tout[26];
    int     i;

    Printf ("\nc 2");
    Printf ("\nm %d %d\nt 5 1\n", x, y);

    (void) time (&tloc);
    tptr = ctime (&tloc);
    for (i = 0; i < 3; i++)
	tout[i] = tptr[i + 8];	/* day */
    for (i = 3; i < 7; i++)
	tout[i] = tptr[i + 1];	/* month */
    for (i = 7; i < 11; i++)
	tout[i] = tptr[i + 13];	/* year */
    for (i = 11; i < 17; i++)
	tout[i] = tptr[i - 1];	/* time of day */
    tout[17] = '\n';
    tout[18] = '\0';
    Printf ("%s\n", tout);
    Printf ("\m 0 0\n");
}
