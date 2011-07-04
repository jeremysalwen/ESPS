/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1995 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: returns type of feature file in ascii
 *
 */

static char *sccs_id = "@(#)featype.c	1.1	2/19/96	ERL";



#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>

int debug_level = 0;

main(argc, argv)
   int             argc;
   char          **argv;

{

   struct header  *h;
   int             idx_ok();
   extern char    *fea_file_type[];
   FILE           *fp;

   if (argc != 2) {
      printf("NONE\n");
      exit(0);
   }
   if ((fp = fopen(argv[1], "r")) == NULL) {
      printf("NONE\n");
      exit(0);
   }
   if ((h = read_header(fp)) == NULL) {
      printf("NONE\n");
      exit(0);
   }
   if (idx_ok(h->hd.fea->fea_type, fea_file_type))
      printf("%s\n", fea_file_type[h->hd.fea->fea_type]);
   else
      printf("NONE\n");
}
