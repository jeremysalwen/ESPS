/*********************************************************
*
*  This material contains propriety software of Entropic
*  Speech, Inc.  Any reproduction, distribution, or
*  publication without the prior written permission of
*  Entropic Speech, Inc. is strictly prohibited.  Any
*  public distribution of copies of this work authorized
*  in writing by Entropic Speech, Inc. must bear the
*  notice "Copyright 19xx Entropic Speech, Inc."
*
*  Module Name: his.c
*
*  Written By:   Brian Sublett   10-18-86
*  Modified extensively for ESPS 3.0 by David Burton
*  Checked by: 
*
*  Description: This program reads data from the standard
*		input and sorts it into bins for the plotting
*		of histograms.
*
* 
*********************************************************/


static char *sccs_id = "@(#)his.c	3.4 25 Nov 1996 ESI";

/*
 * System Includes
*/
# include <stdio.h>

/*
 * ESPS Includes
*/
# include <esps/esps.h>
# include <esps/unix.h>

/*
 * Defines
*/
#define SYNTAX USAGE ("his [-x debug_level][-r range][-n nbins] out_file");
#define X_GRID 10   /*number of x grid lines*/
#define Y_GRID 10   /*number of y grid lines*/

/*
 * ESPS Functions
*/
void frange_switch();

extern char *optarg;
extern optind;
int  debug_level = 0;

main (argc, argv)
int argc;
char *argv[];
    {
    
    FILE *fpin = stdin,  *fpout;	/*stream pointers*/
    int c;
    long i, nbins = 64, bin;
    long *bin_count;			 /* holds bin counts*/
    double  inc, y;
    double low = -2048.0, high = 2048.0;	/*default limits*/
    char *out_file = "<stdout>";	/*output filename*/
    int    y_inc = 1;				/* y axis step size*/
    double x_inc = 1.;

/* Check the command line options. */

    while ((c = getopt (argc, argv, "x:r:n:")) != EOF)
	{
	switch (c)
	    {
	    case 'x':
		if (sscanf(optarg,"%d", &debug_level) != 1)
		    {
		    Fprintf (stderr, "Error reading -x arg. Exitting.\n");
		    exit (1); 
		    }
		break;
	    case 'r':
		frange_switch (optarg, &low, &high);
		if (low >= high)
		    {
		    Fprintf (stderr, "his: Invalid range specified: %s\n", optarg);
		    exit (1);
		    }
		break;
	    case 'n':
		if (sscanf (optarg, "%d", &nbins) != 1)
		    {
		    Fprintf (stderr, "his: Invalid number of bins specified: %s\n", optarg);
		    exit (1);
		    }
		if (nbins < 1)
		    {
		    Fprintf (stderr, "his: Invalid number of bins specified: \n");
		    Fprintf (stderr, "his: nbins = %d\n", nbins);
		    exit (1);
		    }
		break;
	    default:
		SYNTAX
	    }
	}

/* Get the filenames. */

    if (optind < argc)
	{
	out_file = eopen("his", argv [optind++], "w", NONE, NONE,
			(struct header **)NULL, &fpout);
	}
    else{
	Fprintf(stderr, "his: No output file specified\n");
	SYNTAX
	exit(1);
    }
    if (debug_level) 
	    Fprintf (stderr,"his: Output file is %s\n", out_file);

/* Begin sorting. */

    bin_count = (long *) calloc ((unsigned)nbins, sizeof (long));
    spsassert(bin_count != NULL, "Couldn't allocate space for bin_count");


    inc = (high - low)/nbins;
    /*
     * Get data, sort and count it
    */
    while (fscanf (fpin, "%lf", &y) == 1)
	{
	if (y >= low && y < high)
	    {
	    bin = (y - low)/inc;
	    (bin_count [bin]) ++;
	    if(debug_level > 5){
		Fprintf(stderr, "his: bin_count[%d] = %ld\n", bin, 
				    bin_count[bin]);
	    }
	    }
	}

    /*
     * Data ready to be written - output in aplot format
    */
	/* number of samples*/
	Fprintf(fpout, "%ld\n", 3*nbins);

	/* number of plots*/
	Fprintf(fpout, "1\n");

	/*compute x-increment for grid*/
	x_inc = inc;
	if((high-low)/inc > X_GRID)
	    x_inc = (high - low)/ X_GRID;
	if(debug_level > 2)
	    Fprintf(stderr,
	    "his: high-low = %lf, X_GRID = %d, x_inc = %lf\n",
	    (high-low), X_GRID, x_inc);
	
	/*low, high, and x-increment*/
	Fprintf(fpout, "%lf %lf %lf\n", low, high, x_inc);

	/*find maximum bin count and compute y increment*/
	{
    	    int max_bin = 0;
	    max_bin = bin_count[0];
	    for(i=0; i<nbins; i++){
		if(bin_count[i] > max_bin)
		    max_bin = bin_count[i];
	    }
	    if(debug_level > 0)
		Fprintf(stderr, "his: max_bin = %d\n", max_bin);

	    /*compute y-increment for grid*/
	    if(max_bin > Y_GRID)
		y_inc = ROUND((float)max_bin/(float)Y_GRID + .5);

	    max_bin = Y_GRID*y_inc;

	    if(debug_level > 2)
		Fprintf(stderr, 
		"his: max_bin = %d, Y_GRID = %d, y_inc = %d\n",
		max_bin, Y_GRID, y_inc);

	    /*low, high, y-increment*/
	    Fprintf(fpout, "0 %d %d\n", max_bin, y_inc);
	}

	/* Now write out bin count and bin edges*/
	if(debug_level > 2)
	    Fprintf(stderr, "his: inc = %lf\n", inc);
	for(i = 0; i < nbins; i++){
	    Fprintf(fpout, "%lf %ld\n", (double)(i*inc+low), bin_count[i]);
	    Fprintf(fpout, "%lf %ld\n", (double)((i+1)*inc+low), bin_count[i]);
	    Fprintf(fpout, "%lf 0\n", (double)((i+1)*inc+low));  
	}	
    exit(0);
    return(0); /*lint pleasing*/
}
