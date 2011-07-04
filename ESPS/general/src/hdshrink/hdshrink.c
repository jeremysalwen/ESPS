/*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
*/


#ifndef lint
static char *sccs_id = "@(#)hdshrink.c	3.6 12/21/93	ESI";
#endif
 	
#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define PROG "hdshrink"

char *get_cmd_line();
char *savestring();
void recursive_get_comment();
extern int optind;
int debug_level=0;

#ifdef DEC_ALPHA
static char pad[10000];
#endif


#define SYNTAX USAGE ("hdshrink [-c] infile output (or '-' for stdio)")

main(argc,argv)
int argc;
char **argv;
{
	FILE *f1strm = stdin;
	FILE *f2strm = stdout;
	char *f1name = "<stdin>";
	char *f2name = "<stdout>";
	int c, i, cflag=0;
	struct header *f1hd;
	struct header *f2hd;
	char *save_comment = NULL;


    while ((c = getopt(argc, argv, "c")) != EOF) {
      switch (c) {
	case 'c':
	  cflag=1;
	  break;
	default:
	  SYNTAX;
	  break;
	}
      }

/* there must be two file arguments */
	if (argc-optind != 2) SYNTAX;

	f1name = argv[optind++];
	f2name = argv[optind];
	if (strcmp(f1name,f2name) == 0) {
	 (void)fprintf(stderr,"infile and outfile cannot be the same\n");
	 exit(1);
	}

	f1name = eopen(PROG, f1name, "r", NONE, NONE, &f1hd, &f1strm);
	f2name = eopen(PROG, f2name, "w", NONE, NONE, &f2hd, &f2strm);

	if(f1hd->variable.comment)
		save_comment = savestring(f1hd->variable.comment);
	f1hd->variable.comment = NULL;
	add_comment(f1hd,get_cmd_line(argc,argv));
	if (save_comment)
		add_comment(f1hd,save_comment);

	if (cflag)
		recursive_get_comment(f1hd, f1hd);

/* zap all the source files and headers */
    	for (i = 0; i < MAX_SOURCES; i++) 
	 f1hd -> variable.srchead[i] = NULL;
    	f1hd -> variable.nnames = f1hd -> variable.nheads = 0;

	write_header(f1hd,f2strm);

/* copy the data from input to output */
	while ((c = getc(f1strm)) != EOF)
		(void) putc(c,f2strm);
	return 0;
	
}

void
recursive_get_comment (hd1, hd2)
struct header *hd1;
struct header *hd2;
{
	int i;

	if (hd2 == NULL) return;
	if (hd2 != hd1) 
		add_comment(hd1, hd2->variable.comment);
    	for (i = 0; hd2->variable.srchead[i] && i < MAX_SOURCES; i++) 
		recursive_get_comment(hd1, hd2->variable.srchead[i]);
	return;
}
    	
