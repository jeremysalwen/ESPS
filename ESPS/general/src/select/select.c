/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by: Ken Nelson
 *
 * Brief description: 
 *
 */

static char *sccs_id = "@(#)select.c	3.8	1/6/93	ERL";


#ifndef lint
	char *select_sccs = "@(#)select.c	3.8	1/6/93 ESI";
#endif

#include "select.h"
#include <signal.h>

#define SYNTAX USAGE("select [-n] [-q query] [-l log] [-o output] [input1 input2 ...]")

#define DEF_FORMAT	"%lg\n"

char	*ifile[MAXIFILES];	/* array of input file names */
char  	*ofile=NULL;		/* name of output file */
FILE 	*istrm[MAXIFILES]; 	/* array of input file streams */
FILE	*ostrm=NULL;		/* stream for output file */
struct header *ihd[MAXIFILES];	/* array of input headers */
struct fea_data *irec[MAXIFILES]; /* array of input records */
char    *eval_format = DEF_FORMAT;/* default format for eval output */

char *query=NULL, *lfile=NULL;	/* query and log file file from command line */
int nflag=0, n_ifiles=0;	/* nflag means don't expand the header */
int qflag=0;			/* 1 if q option used */
int cflag=0;			/* 1 if coded comparisons are case 
				   insensitive */
int eflag=0;			/* eval expression on command line */
int debug_level=0;			/* global debug flag */

extern int optind;
extern char *optarg;
void init_output();
void push_back_line();
int interrupt = 0; 
static int_handler(); 		/* interrupt is incremented on ^C */
extern int query_error;		/* indicates an error in query parsing */
extern int fatal_error;		/* indicates bad run-time error */
extern int run_error;		/* indicates a run-time error */
static int fpe_handler();	/* fpe handler routine */
int Argc;			/* global for argc */
char **Argv;			/* global for argv */

char *process_controls();	

main(argc,argv)
int argc;
char **argv;
{
   int c;

   Argc = argc;
   Argv = argv;
   init_output();
   while ((c=getopt(argc,argv,"ncq:e:l:o:d:f:")) != EOF) {
	switch (c) {
	  case 'n':
	  	nflag++;
		break;
	  case 'c':
		cflag=1;
		break;
	  case 'q':
		qflag=1;
		query = optarg;
		break;
	  case 'e':
		eflag=1;
		query = optarg;
		break;
	  case 'l':
		lfile = optarg;
		break;
	  case 'o':
		ofile = optarg;
		break;
	  case 'd':
		debug_level = atoi(optarg);
		break;
	  case 'f':
		eval_format = process_controls(optarg);
		break;
	  default:
		SYNTAX;
		exit(1);
	}
   }

/* if input files given, collect them with in_files.  If a file
   argument is bad, then in_files will print message and return zero.
*/

   while (optind < argc)
	if (in_files(argv[optind++]) == 0)
	  exit (1); 

/* open log file if specified 
*/
   if (lfile != NULL)
	if (open_logfile(lfile) == 0) exit(1);

/* open the output file if specified
*/
   if (ofile != NULL)
	if (open_outfile(ofile) == 0) exit(1);

   (void) signal(SIGINT, int_handler);
   (void) signal(SIGFPE, fpe_handler);

/* if a query is given on the command line, use push_back_line
   to feed it to the lexical analyzer
*/
   if (query != NULL) {
	if (qflag && ofile == NULL) {
		errmsg("select: no output file open\n");
		exit(1);
	}
	if (n_ifiles == 0) {
		errmsg("select: no input files open\n");
		exit(1);
	}
	if (qflag)
		push_back_line("select ",0);
	else
		push_back_line("eval ",0);
	push_back_line(query,1);
	if (qflag)
		push_back_line("write",1);
	push_back_line("quit",1);
   }

/* run the parser
*/
   while (1) {
	query_error = 0;
	interrupt = 0;
	fatal_error = 0;
	(void) do_command();
   }
}

static
int_handler() {
/* Just record that we got an interrupt, and re-enable the signal */
    (void) signal(SIGINT,int_handler);
    interrupt++;
    if (interrupt >= 3) exit(1);
    errmsg("Interrupt\n");
}


/* the fpe handler just sets the run_error flag */

static
fpe_handler() {
    errmsg("Floating point exception; ");
    run_error++;
}

/* This function processes a string containing controls, such as \n, \t,
   etc and produces a binary string - as the C compiler does at compile
   time. */

char *
process_controls(s)
char *s;
{
	if(s && s[0]) {
		char *p1 = s;
		char *p2 = s;
		while(*p1) {
		  if(*p1 == '\\') {
		    p1++;
		    switch (*(p1++)) {
			case 'n':
			 *p2++ = '\n';
			 break;
			case 't':
			 *p2++ = '\t';
			 break;
			case 'b':
			 *p2++ = '\b';
			 break;
			case 'r':
			 *p2++ = '\r';
			 break;
			case 'f':
			 *p2++ = '\f';
			 break;
			case '\\':
			 *p2++ = '\\';
			 break;
			case '\'':
			 *p2++ = '\'';
			 break;
			default:
			 fprintf(stderr,
			  "select: Error in -f option string, starting at: %s\n",
			   p1-2);
			 fprintf(stderr,"Using default format.\n");
			 return DEF_FORMAT;
		    }
		  }
		  else
		    *p2++ = *p1++;
		}
              *p2 = '\0';
	      return s;
	}
	return DEF_FORMAT;
}

			

