/*
 *  This material contains proprietary software of Entropic Speech,
 *  Inc.  Any reproduction, distribution, or publication without the
 *  prior written permission of Entropic Speech, Inc. is strictly
 *  prohibited.  Any public distribution of copies of this work
 *  authorized in writing by Entropic Speech, Inc. must bear the
 *  notice
 *  
 * "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."
 *
 *
 *  range - Program to select a range using the mouse on a
 *		 MASSCOMP Workstation.
 *  
 *  Written by:  Ajaipal S. Virdy on 9/2/86
 *  Modified for zooming, and playing by Alan Parker 2/88
 *
 */
#ifndef lint
    static char *sccs_id = "@(#)range.c	3.4 3/18/88	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>

#define Printf (void) printf
#define Fflush (void) fflush

#define	TRUE	1
#define	FALSE	0

#define	ESC	'\033'
#define CLEAR	{putchar(ESC);putchar('['); \
		 putchar('2');putchar('K');}
#define	HOME	{putchar(ESC);putchar('[');putchar('H');}
#define BEEP	{putchar(''); Fflush(stdout);}

#define RIGHT	1
#define	MIDDLE	2
#define	LEFT	4

#define	FOREVER	for(;;)

#define	THISPROG "range"


long	beginplot;	/* read from ESPS Common file */
long	endplot;	/* read from ESPS Common file */
long	points;

long	new_start;	/* new start point selected */
long	new_nan;	/* new nan point selected */

static int standard_x = 1;
static int standard_y = 1;	    

static short standard_cursor[16] =	     /* looks like an arrow */
	    {
		0x0000,	0x4000, 0x6000, 0x7000,
		0x7800,	0x7c00, 0x7e00, 0x7f00, 
		0x7f80,	0x7c00, 0x6c00, 0x4600, 
		0x0600,	0x0000, 0x0000, 0x0000
	    };

int	debug=0;	/* debug flag */

char range_arg[500];
extern char *optarg;
extern int optind;

int xl, yb, xr, yt, placed;
void mark_cursor();


main(argc,argv)
int argc;
char **argv;
{
	int	x,	/* x coordinate of current mouse position */ 
		y;	/* y coordinate of current mouse position */

	int	button;		/* button state */

	int	x_start;	/* start point on abscissa */

	int	old_x = 0,	/* previous x location */
		old_y = 0;	/* previous y location */
	
	int	right_mark = 0; /* position of right marker line */

	int	button1_up = TRUE;	/* LEFT button position */
	int	button2_up = TRUE;	/* MIDDLE button position */

	int	click = 0;	/* number of times mouse has been clicked */

	int	x0;		/* origin of graph x-coordinate */

	int	x_final;	/* endpoint of graph x-coordinate */

	int	play=0;		/* if 1 then play segment on middle button */
	int	played=0;
	int	c;
	char	*bflag=NULL;
	char	*channel=NULL;
	char	*s;


/* be sure Common processing is on, otherwise this program
   cannot work correctly
*/

	if((s = getenv("USE_ESPS_COMMON")) != NULL && !strcmp(s,"off")) {
		Fprintf(stderr,"ESPS Common must be on for program.\n");
		Fprintf(stderr,"Environment variable USE_ESPS_COMMON must not equal \"off\"\n");
		exit(1);
	}

	while ((c = getopt(argc, argv, "pb:c:x")) != EOF) {
	  switch (c) {
		case 'p':
		  play=1;
		  break;
		case 'b':
		  bflag=optarg;
		  break;
		case 'c':
		  channel=optarg;
		  break;
		case 'x':
		  debug=1;
		  break;
		default:
		  USAGE("range [-p [-b shift_val] [-c channel]");
		  exit (1);
	  }
	}


 	/* Start a loop to get button input and to
	 * print coordinates.
	 */

	(void) read_params((char *) NULL, SC_CHECK_FILE, (char *) NULL);

	if(symtype("beginplot")==ST_UNDEF || symtype("endplot")==ST_UNDEF) {
	  Fprintf(stderr,"range: no beginplot or endplot in Common file\n");
	  exit(1);
	}
	beginplot = getsym_i ("beginplot");
	endplot = getsym_i  ("endplot");
	points = endplot - beginplot + 1;

	winloadcur16( standard_x, standard_y, standard_cursor );
	wincurlife( 1 );

	(void)mgiasngp(0,0);
	(void)mgimodfunc(3,4,3);
	(void)mgigetvcoor(2,&xl,&yb,&xr,&yt,&placed);
	yb =  yt*.4/2;
	yt = yt-(yt*.4)/2;

	HOME

	while ( button1_up ) {
	  CLEAR
	  if (click == 0)
	     Printf ("Click on the origin (left edge of box) with the LEFT mouse button.");
	  else
	     Printf ("Now click on the endpoint (right edge of box) with the LEFT mouse button.");

	  Fflush (stdout);

	  wincurgetwait( &x, &y, &button );

	  if ( button == LEFT ) {
	     if ( click == 0 ) {	/* first mouse click */
		 x0 = x;
		click = 1;
	     } else {	/* second mouse click */
		 x_final = x;
		button1_up = FALSE;
	     }
	  }
	}

	Fflush (stdout);

RESTART:

	HOME
	CLEAR
	Printf ("LEFT: select start of range, MIDDLE: play entire plot, RIGHT: zoom out\n");
	Fflush (stdout);
	button1_up = TRUE;
	while (button1_up)
	{
		wincurgetwait( &x, &y, &button );
		if ( button == LEFT ) {
			x_start = x;
			button1_up = FALSE;
			mark_cursor(x);
		} else if ( button == MIDDLE ) {
			char buf[80];
			Printf("Playing entire range ...");
			Fflush(stdout);
			(void)strcpy(buf,"play -q ");
			if(getenv("PLAYOPTS"))
				strcat(buf,getenv("PLAYOPTS"));
			else {
				if (bflag) {
					(void)strcat(buf,"-b ");
					(void)strcat(buf,bflag);
				}
				if (channel) {
					(void)strcat(buf,"-c ");
					(void)strcat(buf,channel);
				}
			}
			(void)my_system(buf);
			while (button != 0) wincurget( &x, &y, &button);
			Printf("                        ");
			Fflush(stdout);
		} else if ( button == RIGHT ) {
			Printf("Zooming out ...");
			Fflush(stdout);
			(void)my_system("plotsd -p1: ");
			while (button != 0) wincurget( &x, &y, &button);
	                (void) read_params((char *) NULL, SC_CHECK_FILE, 
				(char *) NULL);
			beginplot = getsym_i ("beginplot");
			endplot = getsym_i  ("endplot");
			points = endplot - beginplot + 1;
			goto RESTART;
		}
	}

	new_start = beginplot + (points * (x_start - x0) / (x_final - x0));

	while (button != 0) wincurget( &x, &y, &button);
	HOME
	CLEAR
	if (!play)
	    Printf ("LEFT: zoom in, MIDDLE: save range and exit, RIGHT: reselect range\n");
	else
	    Printf ("LEFT: zoom in, MIDDLE: play range, RIGHT: reselect range\n");
	Fflush (stdout);

	while (button2_up) {

	    /* Print coordinates at button signal. */

	    wincurget( &x, &y, &button );
	    if  (x != old_x) {
		CLEAR
		if ( (x < x_start) || (x > x_final) )
		   Printf ("OUT OF RANGE");
		else
		{
		   new_nan = beginplot + (points * (x - x0) / (x_final - x0));
		   Printf ("Range selected is from %ld to %ld",
		   	new_start, new_nan);
		   played=0;
		   old_x = x;
		   if(right_mark) {
			mark_cursor(right_mark);
			right_mark=0;
		   }
		}
		Fflush (stdout);
	    }

	    switch (button) {
		case RIGHT:
		   CLEAR
		   played=0;
		   mark_cursor(x_start);
		   goto RESTART;
		   break;
		case MIDDLE: 
		   {
			mark_cursor(old_x); right_mark=old_x;
		   	(void)sprintf(range_arg,"-p%ld:+%ld",new_start,
				new_nan-new_start+1);
			if (play && played != 1) {
			  char buf[512];
			  CLEAR 
			  Printf("Playing points %ld through %ld ...",
				new_start,new_nan);
			  Fflush(stdout);
			  (void)strcpy(buf,"play -q ");
			  (void)strcat(buf,range_arg);
			  (void)strcat(buf," ");
			  if (getenv("PLAYOPTS"))
				strcat(buf,getenv("PLAYOPTS"));
			  else {
			    if (bflag) {
				(void)strcat(buf," ");
				(void)strcat(buf,"-b ");
				(void)strcat(buf,bflag);
			    }
			    if (channel) {
				(void)strcat(buf," ");
				(void)strcat(buf,"-c ");
				(void)strcat(buf,channel);
			    }
			  }
			  (void)my_system(buf);
			  CLEAR
			  Printf("Hit MIDDLE button again to save range and exit");
			  Fflush(stdout);
			  played=1;

			} else {
			  
			  if (putsym_i("start", (int) new_start) == -1)
			   Fprintf (stderr,
			   "%s: could not write into ESPS Common file.\n",
			   THISPROG);
			  if (putsym_i("nan", (int) (new_nan - new_start + 1)) == -1)
			   Printf (stderr,
			   "%s: could not write into ESPS Common file.\n",
			   THISPROG);
			  if (putsym_s("prog", THISPROG) == -1)
			    Fprintf(stderr,
				"%s: could not write into ESPS Common file.\n",
				THISPROG);
			  button2_up = FALSE;
			}
			   			   
		   }
		   break;
		case LEFT:
		   {
			char buf[512];
			CLEAR 
			mark_cursor(old_x); right_mark=0;
			Printf("Replotting points %ld through %ld ...",
				new_start,new_nan);
			Fflush(stdout);
		   	(void)sprintf(range_arg,"-p%ld:+%ld",new_start,
				new_nan-new_start+1);
			(void)strcpy(buf,"plotsd -E.1 ");
			(void)strcat(buf,range_arg);
			(void)my_system(buf);
	                (void)read_params((char *) NULL, SC_CHECK_FILE, 
				(char *) NULL);
			beginplot = getsym_i ("beginplot");
			endplot = getsym_i  ("endplot");
			points = endplot - beginplot + 1;
			goto RESTART;
		   }
		   break;
		default:
		   break;
	    }

	}

	Printf ("\n");
	exit( 0 );
    /*NOTREACHED*/
}

my_system(s)
char *s;
{
	char *msg="ESPS_VERBOSE=0; ";
	char *buf=malloc((unsigned)(strlen(msg)+strlen(s)));
	(void)strcpy(buf,msg);
	(void)strcat(buf,s);
	if(debug) Fprintf(stderr,"%s\n",buf);
	else
		return system(buf);
	return 0;
}
	
void
mark_cursor(x)
int x;
{
	(void)mgil(x,yb,x,yt);
}	

