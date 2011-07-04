/*
 * This material contains proprietary software of Entropic Speech,
 * Inc.  Any reproduction, distribution, or publication without the
 * prior written permission of Entropic Speech, Inc. is strictly
 * prohibited.  Any public distribution of copies of this work
 * authorized in writing by Entropic Speech, Inc. must bear the
 * notice
 *
 * "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."
 *
 *
 * gpsplot.c - gps plotting routines for genplot(1-ESPS) and mlplot(1-ESPS)
 *
 * Authors:  Rodney Johnson & Ajaipal S. Virdy, Entropic Speech, Inc.
 */
#ifndef lint
    static char *sccs_id = "@(#)gpsplot.c	3.4	6/17/88	ESI";
#endif

#include <stdio.h>
#include <esps/unix.h>
#include <esps/esps.h>

/*
 * The following are macros and functions for writing Masscomp
 * UNIX Graphic Primitive String (GPS) commands.
 */

/* Macros to pack data into the three graphic primitive word types that
   occur in a GPS metafile.  Byte order is not necessarily correct for
   machines other than Masscomp.  (Documentation on the GPS format is
   not clear on that point.) */

/* PUT? writes to stdout */

#define PUT1(A,B)  {short sh_P = (A)<<12 | (B)&0x0fff; \
		    (void) fwrite((char*) &sh_P, sizeof(short), 1, stdout);}
#define PUT2(A)	   {short sh_P = (A); \
		    (void) fwrite((char*) &sh_P, sizeof(short), 1, stdout);}
#define PUT3(A,B)  {short sh_P = (A)<<8 | (B)&0x00ff; \
		    (void) fwrite((char*) &sh_P, sizeof(short), 1, stdout);}

/* FD_PUT? writes to file specified by outfp file pointer */

#define FD_PUT1(A,B) {short sh_P = (A)<<12 | (B)&0x0fff; \
		    (void) fwrite((char*) &sh_P, sizeof(short), 1, outfp);}
#define FD_PUT2(A)   {short sh_P = (A); \
		    (void) fwrite((char*) &sh_P, sizeof(short), 1, outfp);}
#define FD_PUT3(A,B) {short sh_P = (A)<<8 | (B)&0x00ff; \
		    (void) fwrite((char*) &sh_P, sizeof(short), 1, outfp);}

/* Generate Masscomp Unix GPS line-drawing command. */

#define	MAX_POINTS  2046

#define	FACTOR	    4	/* make plot 4 times as big as IMAGEN plot */

/*
 * G L O B A L
 *  V A R I A B L E S
 *   R E F E R E N C E D
 */

extern int  debug_level;
extern long P0;
extern long Q0;
extern int  nflag;
extern char *outdir;
extern FILE *outfp;
extern int  gps_bundle;


#define	PATH_LEN  200	/* directory path length */

init_gps_page()
{
    char    name[PATH_LEN];

/* GLOBAL Variables referenced: */

    extern int	page_num;
    extern char	*ProgName;

    if (outdir != NULL)
    {
	Sprintf(name, "%s/page%d", outdir, page_num);
	if ( (outfp = fopen(name, "w")) == NULL )
	{
	    Fprintf(stderr, "%s: could not open %s\n", ProgName, name);
	    exit (1);
	}

	if (debug_level > 3)
	    Fprintf(stderr, "%s: init_gps_page: opening %s\n",
		    ProgName, name);

	gps_null_comment(outfp);
    }
}

int
gps_plotline(n, u, v)
    long    n;			/* Number of points. */
    long    u[], v[];		/* Vectors of horiz. & vert. coords. */
{
    int	    i, residue;
    long    j, loop;
    int	    npoints;		/* number of points to plot */

/*
 * GLOBAL Variables referenced: nflag, outdir, outfp, P0, Q0
 */

    loop = n / MAX_POINTS + 1;
    residue = n % MAX_POINTS;

    for (j = 1; j <= loop; j++)
    {
	if (j == loop)
	    npoints = residue;
	else
	    npoints = MAX_POINTS;

	if ( !nflag )
	    PUT1(0, 2 + 2*npoints)	/* Command code & length */

	if ( outdir != NULL )
	    FD_PUT1(0, 2 + 2*npoints)	/* Command code & length */

	for (i = 1; i <= npoints; i++, u++, v++)
	{
	    if ( !nflag )		/* suppress output to stdout */
	    {
		PUT2(*u * FACTOR + P0)	/* x Point coordinates. */
		PUT2(*v * FACTOR + Q0)	/* y Point coordinates. */
	    }

	    if ( outdir != NULL )	/* write output into directory */
	    {
		FD_PUT2(*u * FACTOR + P0)	/* x Point coordinates. */
		FD_PUT2(*v * FACTOR + Q0)	/* y Point coordinates. */
	    }
	}

	if ( !nflag )
	    PUT3(gps_bundle, 0)			/* Bundle number. */

	if ( outdir != NULL )
	    FD_PUT3(gps_bundle, 0)		/* Bundle number. */

    } /* end for loop */

}  /* end procedure gps_plotline() */


/*
 * void
 * gps_plotpoints(n, u, v)
 * int	n;	
 * int	u[], v[];
 * {
 *     int i;
 *     PUT1(0, 2 + 2*n)
 * 			
 *     for (i = 1; i <= n; i++) {PUT2(*u++) PUT2(*v++)}
 *     PUT3(gps_bundle, 0)
 * }
 */

gps_null_comment(f)
    FILE *f;
{
    if(f == stdout) PUT1(15, 1)
    else FD_PUT1(15, 1)
}
