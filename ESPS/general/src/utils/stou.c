/*
 *  This material contains proprietary software of Entropic Speech, Inc.   
 *  Any reproduction, distribution, or publication without the prior	   
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing by
 *  Entropic Speech, Inc. must bear the notice			
 * 								
 *      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 *
 *  stou--translate Stanford plotas commands to Unix GPS commands
 *
 *  Rod Johnson, Entropic Speech, Inc.
 *
 *  This filter reads plot commands in the ``Stanford'' Ascii format
 *  (such as  plotas  input) and writes a translation into Masscomp Unix
 *  ``Graphics Primitive String'' (GPS) format on the standard output.
 *  See Appendix A of Masscomp Data Presentation Programming Manual.
 *  Only the c(olor), d(raw), m(ove), and a simple version of the t(ext)
 *  command are supported:  enough to handle output from plotsd and
 *  plotpit.
 *
 */
#ifndef lint
    static char *sccs_id = "@(#)stou.c	3.1	10/20/87	ESI";
#endif

#include <stdio.h>
#include <esps/unix.h>

#define Sscanf (void) sscanf
#define Fwrite (void) fwrite
#define Fprintf (void) fprintf
#define BUFSIZE	  100		/* See array variables u, v.		*/
#define MAXLINE   500		/* Bound on no. of chars for t command.	*/

#define TEXTSCALE  20		/* Ratio between GPS and Stanford text-	*/
				/*  size indicators.			*/

int	u[BUFSIZE];		/* Consecutive Stanford draw commands	*/
int	v[BUFSIZE];		/*  are buffered and combined into one  */
				/*  Unix line command.			*/
int	siz;			/* Number of points in buffer.		*/
int	color;			/* Current color.			*/
int	bundle = 20;		/* GPS bundle number.			*/

main(argc, argv)
    int	    argc;
    char    **argv;
{
    extern int	optind;		/* used by getopt()			*/
    extern char	*optarg;	/* used by getopt()			*/

    int     x0, y0;		/* Input xy plot coordinates.		*/
    int     ch;			/* Input character.			*/
    int     textsize, textrot;	/* Size and orientation of text.	*/
    int     sinrot, cosrot;	/* Sin & cos of text orientation angle.	*/
    int     hadjust, vadjust;	/* Coord. difference between center of	*/
				/*  char and left edge of baseline.	*/
    char    str[MAXLINE];	/* String for text command.		*/

    while ((ch = getopt(argc, argv, "b:")) != EOF)
	switch (ch)
	{
	case 'b':
	    bundle = atoi(optarg);
	    break;
	default:
	    break;
	}

    comment(0, (int *) NULL);	/* GPS metafile begins with null	*/
				/*  comment field.			*/
    siz = 0;			/* Buffer initially empty.		*/

    /* Read command lines and switch on command character.		*/

    while (fgets(str, MAXLINE, stdin) != NULL)
    {
        switch (str[0])
	{
	case 'c':		/* COLOR --				*/
	    flush();		/* Write out line in current color.	*/
	    Sscanf(str+1, "%d", &color);
				/* Get new color.			*/
            break;

	case 'd':		/* DRAW --				*/
				/* Get input coordinates.		*/
	    Sscanf(str+1, "%d %d", &y0, &x0);
				/* Put output coordinates in buffer.	*/
            draw(x0, -y0);
            break;

	case 'm':		/* MOVE --				*/
				/* Get input coordinates.		*/
	    Sscanf(str+1, "%d %d", &y0, &x0);
				/* Write out buffer & use output coords	*/
				/*  to start new one.			*/
	    move(x0, -y0);
            break;

	case 't':		/* TEXT --				*/
				/* Get size and orientation.		*/
	    Sscanf(str+1, "%d %d", &textsize, &textrot);
            textsize = textsize*TEXTSCALE;
				/* Get line of text.			*/
            if(fgets(str, MAXLINE, stdin) == NULL)
	    {
                Fprintf(stderr, "Unexpected EOF after t(ext) command.");
                exit(1);
            }
				/* Null terminate.			*/
	    str[strlen(str) - 1] = '\0';
				/* Input coords refer to left end of	*/
				/*  baseline; output coords to center.	*/
	    hadjust = (int)(textsize*0.4 + 0.5);
	    vadjust = (int)(textsize*0.5 + 0.5);
				/* Sin & cos of text orientation angle.	*/
	    switch (textrot)
	    {
	    case 1:
	        sinrot = 0; cosrot = 1;
		break;
	    case 2:
		sinrot = 1; cosrot = 0;
		break;
	    case 3:
	        sinrot = 0; cosrot = -1;
		break;
	    case 4:
	        sinrot = -1; cosrot = 0;
		break;
	    }
				/* Write text at current position.	*/
	    text(u[siz-1] + hadjust*cosrot - vadjust*sinrot,
                 v[siz-1] + hadjust*sinrot + vadjust*cosrot,
	         bundle,	/* Map all colors to same bundle no.	*/
				/* Try (color-1)+20 on non-monochrome	*/
				/*  system.				*/
		 textsize,
		 (textrot - 1)*90,
                 str
                );
            break;

	default:		/* NO RECOGNIZED COMMAND --		*/
				/* Treat line as comment.		*/
            break;
	}
    }


   /* End of input.  Write out any partially built line & exit.		*/

    flush();
    exit(0);
    /*NOTREACHED*/
} /* end main */


/* The following are macros and functions for writing Masscomp Unix GPS
    commands. */

/* Macros to pack data into the three graphic primitive word types that
   occur in a GPS metafile.  Byte order is not necessarily correct for
   machines other than Masscomp.  (Documentation on the GPS format is
   not clear on that point.) */

#define PUT1(A,B)  {short sh_P = (A)<<12 | (B)&0x0fff; \
		    Fwrite((char*) &sh_P, sizeof(short), 1, stdout);}
#define PUT2(A)	   {short sh_P = (A); \
		    Fwrite((char*) &sh_P, sizeof(short), 1, stdout);}
#define PUT3(A,B)  {short sh_P = (A)<<8 | (B)&0x00ff; \
		    Fwrite((char*) &sh_P, sizeof(short), 1, stdout);}

/* Write out current buffer; begin new. */

flush()
{
    if (siz > 1)
    {
				/* Write out command to draw any	*/
				/*  current partially built line.	*/
        line(siz, 
	     u, v,
	     bundle);		/* Map all colors to same bundle no.	*/
				/*  Could use (color-1)+20 to get dis-	*/
				/*  tinct styles for distinct colors,	*/
				/*  but resulting dashed lines don't	*/
				/*  look good on monochrome system.	*/

	u[0] = u[siz - 1];	/* Retain current position as first	*/
	v[0] = v[siz - 1];	/*  point of new buffer.		*/
	siz = 1;
    }
}

/* Add another point to current buffer. */

draw(u0, v0)
    int u0, v0;			/* Unix GPS universe coordinates.	*/
{
    if (siz >= BUFSIZE)
        flush();
    u[siz] = u0;
    v[siz] = v0;
    siz++;
}

/* Start new buffer at given position. */

move(u0, v0)
    int u0, v0;			/* Unix GPS universe coordinates.	*/
{
    flush();
    u[0] = u0;
    v[0] = v0;
    siz = 1;
}

/* Generate Masscomp Unix GPS line-drawing command. */

line(n, u, v, b)
    int	n;			/* Number of points.			*/
    int	u[], v[];		/* Vectors of horiz. & vert. coords.	*/
    int	b;			/* Bundle number.			*/
{
    int i;
    PUT1(0, 2 + 2*n)		/* Command code & length		*/
				/* Point coordinates.			*/
    for (i = 1; i <= n; i++) {PUT2(*u++) PUT2(*v++)}
    PUT3(b, 0)			/* Bundle number.			*/
}

/* Generate Masscomp Unix GPS text-writing command. */

text(u0, v0, b, s, r, t)
    int u0, v0;			/* GPS universe coords of first char.	*/
    int b;			/* Bundle number.			*/
    int s, r;			/* Size, rotation.			*/
    char *t;			/* Text string.				*/
{
    int n = (2 + strlen(t))/2;	/* 2 chars per word; 1 or 2 nulls	*/
    r = r % 360;		/* Normalize orientation angle.		*/
    if (r >= 180) r = r - 360;
    else if (r < -180) r = r + 360;
    PUT1(2, 5 + n)		/* Command code & length.		*/
    PUT2(u0) PUT2(v0)		/* Starting coords.			*/
    PUT3(b, 0)			/* Bundle number.			*/
				/* Size * rotation.			*/
    PUT3((s+2)/5, (256*r + 180)/360)
				/* Text characters.			*/
    while (n-- > 1)
    {
	putchar(*t++);
	putchar(*t++);
    }
    {
	putchar(*t);		/* Last char if length is odd; null if	*/
				/*  even.				*/
	putchar('\0');
    }
}

/* Generate Masscomp Unix GPS comment field. */

comment(n, c)
    int n;			/* Number of words			*/
    int c[];			/* Contents.				*/
{
    PUT1(15, n+1)		/* Command code & length.		*/
    while(n--) PUT2(*c++)	/* Contents (short words).		*/
}

/*----------------------------------------------------------------------*/
