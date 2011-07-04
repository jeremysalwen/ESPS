/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1988 Entropic Speech, Inc.; All rights reserved"

  get_sf: get sampling frequency from sampled data file

  written by: David Burton
*/
#ifndef lint
static char *sccs_id = "@(#)get_sf.c	1.2 1/6/93 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/sd.h>
#include <esps/unix.h>

extern char *optarg;
extern int optind;

int debug_level = 0;
main (argc, argv)
char **argv;
{
    char *in_file = NULL, *eopen ();
    float sf;
    struct header *h;
    FILE *strm;

    if (argc != 2) {
	Fprintf (stderr, "Usage: %s  file.sd\n", *argv);
	exit (1);
    }

    in_file = argv[1];
    in_file = eopen (*argv, in_file, "r", 0, 0, &h, &strm);
    (void) fclose (strm);

    sf = h->hd.sd->sf;
    Fprintf (stdout, "%d", (int)sf);
    return(0);
}
