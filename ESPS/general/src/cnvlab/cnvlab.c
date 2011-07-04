/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1991  Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  John Shore
 * Checked by:
 * Revised by:
 *
 * Brief description: convert TIMIT (sphere) label file to waves+ label file
 *
 */

static char *sccs_id = "@(#)cnvlab.c	1.2	3/5/92	ERL";

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>

#define SYNTAX \
USAGE("cnvlab [-m mode] [-s sampling_rate] sphere_lab waves_lab")

#define Fprintf (void)fprintf

/*
 * fileOK() - Local function to check and see if the file permissions
 * (from Ken Nelson)
 */

static int fileOK(name, perm )
char *name;
char *perm;
{
 FILE *file;
 int result;

 file = fopen(name, perm);
 if (file != NULL)
 {
   result = TRUE;
   (void) fclose(file);
 }
 else
 {
   result = FALSE;
 }
 return result;
}

main(argc,argv)
int argc;
char **argv;
{
  int             c;
  extern char    *optarg;
  extern          optind;
  int mode = 1;
  double sf = 8000;

  while ((c = getopt(argc, argv, "bs:")) != EOF) {
      switch (c) {
	case 'b':
	     mode = 2;
	     break;
	case 's':
	     sf = atof(optarg);
	     break;
	default:
	    SYNTAX;
	   }	
    }

  if ((argc - optind) != 2)
    SYNTAX;

  if (strcmp(argv[optind], "-") != 0 && !fileOK(argv[optind],"r")) {
      Fprintf(stderr, "cnvlab: can't open input file %s\n", argv[optind]);
      SYNTAX;
    }

  optind++;

  if (strcmp(argv[optind], "-") != 0 && !fileOK(argv[optind],"w")) {
      Fprintf(stderr, "cnvlab: can't open output file %s\n", argv[optind]);
      SYNTAX;
    }

  (void) convertlab(argv[optind - 1], argv[optind], sf, mode, -1, -1);

  exit(0);
}
