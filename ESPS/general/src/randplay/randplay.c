/*
 * This material contains proprietary software of Entropic Processing, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Processing, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Processing, Inc. must bear the notice
 *
 *              "Copyright 1986, 1989 Entropic Processing, Inc."
 *
 * Program:	testsd
 *
 * Written by:  John Shore
 * Checked by:
 *
 * This is the randplay program, which generates a script of play commands
 * for randomized "A-B" listening comparisons.  
 */

#ifndef lint
static char *sccs_id = "@(#)randplay.c	3.6 10/7/96 ERL";
#endif
/*
 * include files
 */
#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
/*
 * defines
 */
#define Fprintf (void)fprintf
#define SYNTAX USAGE ("randplay [-a] [-s] [-d path] [-n program] compfile [play-options]")
#define OPTSIZ 256	/*maximum string length for play options*/
#define MAXLINE 256	/*max length of input or output line*/
#define MAXNAMES 50	/*maximum number of files in one group*/
#define MAXTOKEN 100	/*maximum length of input file name*/
#define MAXPLAY 1000	/*maximum number of A-B comparisons*/
#define MAXDIR 100	/*maximum length of directory name*/
#define TRUE 1
/*
 * system functions and variables
 */
int getopt ();
extern  optind;
extern	char *optarg;
long time();
char *ctime();

long rand_int(), rand_intnr();

/*
 * external SPS functions
 */

/*
 * global declarations
 */
char *gfiles[MAXNAMES];	    /*pointers to individual file names in group*/
/*
 * main program
 */
main (argc, argv)
int argc;
char **argv;
{
/*
 * setup and initialization
 */
void add_gfiles();
char *playname = "play";	    /*name of play program*/
char *scoresheet = "scoresheet";    /*name of file for score sheet*/
static char playpath[MAXDIR] = "";		    /*added path for SD filenames*/
#if !defined(APOLLO_68K) && !defined(IBM_RS6000) && !defined(DEC_ALPHA) && !defined(LINUX_OR_MAC)
static char playoptions[OPTSIZ] = NULL;   /*options for play program*/
#else
static char playoptions[OPTSIZ] = "";   /*options for play program*/
#endif
int c;				    /*for getopt return*/
int i, j;			    /*loop counters*/
long tloc;			    /*needed for current time*/
int self_flag = 0;		    /*set if files are to compared to themselves*/
int score_flag = 0;		    /*set if scoresheet wanted*/
char *compfile = NULL;		/*file name input file*/
FILE *istrm = stdin;		/*input file stream*/
FILE *scorestrm;		/*stream for scoresheet*/
int eog;			/*condition for end of input group*/
char *groupend = "#\n";		/*contents of line signifying end of group*/
char line[MAXLINE];		/*input line*/
char *mark;			/*for fgets return -- check for EOF*/
char *playlines[MAXPLAY];	/*lines of output -- one per A-B comparison*/
int grpind = 0;	    		/*index into array of file names for group*/
int playind = 0;		/*index into array of output lines*/
long playmax;			/*index of final output line*/
char *token;			/*pointer for one filename*/

/*
 * process command line options
 */

    while ((c = getopt (argc, argv, "ad:n:s")) != EOF) {
	switch (c) {
	    case 'a': 
		self_flag++;
		break;
	    case 'd':
		(void) strcpy(playpath, optarg);
		(void) strcat(playpath, "/");
    		break;
	    case 'n':
		playname = optarg;
		break;
	    case 's':
		score_flag++;
		break;
    	    default:
		SYNTAX;
	}
    }
/*
 * process file argument
 */
    if (optind < argc) {
	compfile = argv[optind++];
	if (strcmp (compfile, "-") == 0)
	    compfile = "<stdin>";
	else
	    TRYOPEN (argv[0], compfile, "r", istrm);
	}
    else {
	Fprintf(stderr, "randplay: no input file specified.\n");
	SYNTAX;
        }
/*
 * get play options
 */
    for (i=optind; i<argc; i++) {
	if((strlen(playoptions)+strlen(argv[i])) > OPTSIZ) break;
	(void) strcat(playoptions,argv[i]);
	(void) strcat(playoptions," ");
    }
/*
 * set random seed -- use system time for random starting place
 */
    (void) srandom(time(0));
/*
 * allocate space for the file names in a group
 */
    for (i = 0; i < MAXNAMES; i++) 
	gfiles[i] = (char *) calloc(MAXTOKEN, sizeof(char));
/*
 * main loop
 */
    mark = fgets(line, MAXLINE, istrm);
    while (TRUE) {
	eog = (mark == NULL) || (strcmp(line, groupend) == 0);
	if (!eog) {
	    /*copy file names to group array*/
	    if ((token = strtok(line, " \t\n")) != NULL) {
		add_gfiles(token, grpind, playpath);
		grpind++;
	    }
	    while ((token = strtok(0, " \t\n")) != NULL) {
		add_gfiles(token, grpind, playpath);
		grpind++;
	    }
	    mark = fgets(line, MAXLINE, istrm);
	    if (grpind > MAXNAMES) {
		Fprintf(stderr, "randplay: too many files in group\n");
		exit(1);
	    }
	}
	else {	/*reached end of group*/
	    /*copy randomized A-B pairs to output list*/
	    for(i=0; i < grpind ; i++)
		for (j = (self_flag ? i : i+1); j < grpind; j++) {
		    playlines[playind] = (char *) calloc(MAXLINE, sizeof(char));
		    if ((int) rand_int(1) == 0) {
			(void) strcpy(playlines[playind], gfiles[i]);
			(void) strcat(playlines[playind], " ");
			(void) strcat(playlines[playind], gfiles[j]);
		    }
		    else {
			(void) strcpy(playlines[playind], gfiles[j]);
			(void) strcat(playlines[playind], " ");
			(void) strcat(playlines[playind], gfiles[i]);
		    }
		    playind++;
		    if (playind > MAXPLAY) {
			Fprintf(stderr, 
			"randplay: you asked for more than %d comparisons\n", MAXPLAY);
			exit(1);
		    }	
		}
	    /*initialize for next group*/
	    grpind = 0;
	    /*EOF after group means done*/
	    if ( fgets(line, MAXLINE, istrm) == NULL) break; 
	}
    }
/*
 * output play list in randomized order with proper name and options
 */
    playmax = playind -1;
    for (i = 0; i <= playmax; i++) {
	(void) strcpy(line, playname);
	(void) strcat(line, " ");
	(void) strcat(line, playoptions);
	(void) strcat(line, playlines[(int) rand_intnr(playmax, 0)]);
 	Fprintf(stdout, "%s\n", line);
    }
/*
 * create scoresheet if it was asked for
 */
    if (!score_flag) exit(0);
    TRYOPEN(argv[0], scoresheet, "w", scorestrm);
    tloc = time(0);
    Fprintf(scorestrm, "randplay: score sheet created %s\n", ctime(&tloc));
    Fprintf(scorestrm, "NAME:\n\n");
    Fprintf(scorestrm, "DATE:\n\n");
    Fprintf(scorestrm, "(A) Pefer-A, (B) Prefer-B, (C) Different, but no preference");
    Fprintf(scorestrm, ", (D) No difference\n\n");
    for (i = 0; i <= playmax; i++) {
	Fprintf(scorestrm, 
	  "%3d. ( )A  ( )B  ( )C  ( )D  Comments:\n\n",i+1);
    }
    exit(0);
}

void
add_gfiles(tok, index, path)
char *tok;  /*filename token to add to group list*/
int index;  /*index in gfiles[] array where tok should be added*/
char *path; /*leading pathname for files*/
{
    (void) strcpy(gfiles[index], path);
    (void) strcat(gfiles[index], tok);
}
