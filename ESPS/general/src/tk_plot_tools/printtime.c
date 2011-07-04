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
    static char *sccs_id = "@(#)printtime.c	1.1 9/19/97    ERL";
#endif

#include <stdio.h>

print_time(x, y)
    int     x, y;		/* coordinates of beginning of string */
{
    long    time (), tloc;
    char    *ctime (), *tptr, tout[26];
    int     i;
    char s[80];

    tk_snd_plot_cmd ("c 2");
    sprintf (s,"m %d %d",x,y);
    tk_snd_plot_cmd (s);
    tk_snd_plot_cmd ("t 5 1");

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
    tout[17] = 0;
    tk_snd_plot_cmd (tout);
}
