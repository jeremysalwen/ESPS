/*
 
  This material contains proprietary software of Entropic Processing, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Processing, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Processing, Inc. must bear the notice			
 								
               "Copyright 1986 Entropic Processing, Inc."
*/

#ifndef lint
static char *sccs_id = "@(#)playtest.c	3.4 10/1/93	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

void play();
extern char *optarg;
extern optind;
int debug;

main (argc, argv)
char  **argv;
int     argc;
{
    int c;
    FILE * in, *fopen ();
    char    buf[256],
            ans[256],
	    buf1[256];
    int     dorest = 0, item = 0;

    
    debug=0;
    while ((c = getopt (argc, argv, "x:n")) != EOF) {
	switch (c) {
	    case 'x': 
		debug = atoi(optarg);
		break;
	    default:
		USAGE("playtest [playfile]");
	}
    }

    if (argc - optind < 1)
	in = stdin;
    else
    	TRYOPEN (argv[0], argv[optind], "r", in);

    while (fgets (buf, 256, in) != NULL) {
	item++;
        (void)strcpy(index(buf,'\n')," 2>/dev/null \n");
	play(buf,item);
	play(buf,item);
	if (!dorest) {

    sw: 
	    (void)printf ("Command? [<cr>,r,c,s,q,?] ");
	    if (gets (ans) == NULL) {
		Fprintf (stderr, "playtest: you lose big.\n");
		exit (1);
	    }

	    switch (ans[0]) {
		case '\0': 	/* run next line */
		    break;
		case 'r': 	/* repeat */
		    play(buf,item);
		    goto sw;
		case 'c': 
		    dorest = 1;
		    break;
		case 'q': 
		    (void)printf ("quit\n");
		    exit(0);
		case 's':
		    (void)printf("Running sdcomp..\n");
		    (void)strcpy(buf1,"sdcomp ");
		    (void)strcat(buf1,index(buf,' '));
		    play(buf1,item);
		    goto sw;
		case '?':
		    (void)printf("return for next item\n");
		    (void)printf("c - continue without stopping\n");
		    (void)printf("s - run sdcomp on item\n");
		    (void)printf("q - quit\n");
		    (void)printf("? - this info\n");
	   	    goto sw;
		default: 
		    (void)printf ("Never heard of that command.\n");
		    goto sw;
	    }
	}
	else 
	 (void)printf("\n");
    }
    return 0;
}

void
play(s,item)
char *s;
int item;
{
   (void)printf("playtest: Item: %d, ",item);
   (void)fflush(stdout);
    if(debug) 
      (void)printf(s);
   system(s);
}

