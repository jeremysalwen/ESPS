/*
 * This material contains proprietary software of Entropic Speech,
 * Inc.  Any reproduction, distribution, or publication without the
 * prior written permission of Entropic Speech, Inc. is strictly
 * prohibited.  Any public distribution of copies of this work
 * authorized in writing by Entropic Speech, Inc.  must bear the
 * notice 
 *
 * "Copyright (c) 1986, 1987 Entropic Speech, Inc. All rights reserved."
 *
 *
 * init.c - initialize program for mlplot(1-ESPS) and genplot(1-ESPS)
 *
 * Author: Ajaipal S. Virdy, Entropic Speech, Inc.
 */
#ifndef lint
    static char *sccs_id = "@(#)init.c	3.4	3/29/89	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>


void
initialize()
{

/* Global Variables Referenced: */

    extern int	debug_level;
    extern int	nflag;
    extern int	gps;
    extern int	tek;
    extern int	imagen;
    extern char	*device;
    extern char	*ProgName;
    extern char	*outdir;
    extern int	(*plotting_func)();
    extern int	gps_plotline();
    extern int	tek_plotline();

    if (strcmp(device, "gps") == 0)
    {
	gps = YES;
	tek = NO;
	imagen = NO;
	plotting_func = gps_plotline;

	if ((outdir == NULL) && nflag)
	{
	    Fprintf(stderr,
		"%s: conflicting options, no output will be generated.\n",
		ProgName);
	    exit(1);
	}

	if (outdir != NULL)	/* output will be sent into a directory */
	{
	    if (debug_level > 2)
	    {
		Fprintf(stderr, "%s: making %s directory.\n",
		    ProgName, outdir);
		(void) fflush(stderr);
	    }
	    if (mkdir(outdir, 0777) != 0)
	    {
		Fprintf(stderr, "%s: could not create %s directory.\n",
		    ProgName, outdir);
		exit(1);
	    }
	}

	if (!nflag)
	{
	    if (debug_level > 2)
	    {
		Fprintf(stderr,
		    "%s: outputing in MASSCOMP universe coordinate system.\n",
		    ProgName);
		(void) fflush(stderr);
	    }
	}
    }
    else if (strcmp(device, "imagen") == 0 || strcmp(device, "hardcopy") == 0)
    {
	imagen = YES;
	tek = YES;
	gps = NO;
	plotting_func = tek_plotline;

	if (debug_level > 1)
	{
	    Fprintf(stderr,
		"%s: output in Tektronix 4010 format for the IMAGEN.\n",
		ProgName);
	    (void) fflush(stderr);
	}

	if (outdir != NULL)
	{
	    Fprintf(stderr,
		"%s: this version doesn't support writing into a directory.\n",
		ProgName);
	    exit(1);
	}

	if (debug_level > 2)
	{
	    Fprintf(stderr, "%s: initializing IMAGEN Laser printer...\n",
		ProgName);
	    (void) fflush(stderr);
	}

	init_tek_plot();
    }
    else if (strcmp(device, "tek") == 0)
    {
	tek = YES;
	imagen = NO;
	gps = NO;
	plotting_func = tek_plotline;

	if (debug_level > 1)
	{
	    Fprintf(stderr,
		"%s: output in Tektronix 4010 format.\n",
		ProgName);
	    (void) fflush(stderr);
	}

	if (outdir != NULL)
	{
	    Fprintf(stderr,
		"%s: this version doesn't support writing into a directory.\n",
		ProgName);
	    exit(1);
	}
    }
    else
    {	/* incorrect device name */
	Fprintf(stderr,
	    "%s: can only support gps, hardcopy (imagen), and tek devices for now.\n",
	    ProgName);
	exit(1);
    }

}  /* end initialize() */
