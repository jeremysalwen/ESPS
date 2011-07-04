/*********************************************************
*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	
*
*  Module Name: notch_filt.c
*
*  Written By:   Brian Sublett   8/13/86
*  ESPS 3.0 modifications by David Burton
*  mods for FEAFILT by Bill Byrne, 5/24/91
*  Checked by: 
*
*  DESCRIPTION:  This program designs a two pole, two zero
*		 notch filter, and prints the coefficients
*		 to a FILT file.  The user enters the notch
*		 frequency, the notch bandwidth, and the
*		 sampling frequency either from the command
*		 line or in the parameter file.
*
*  Secrets:  None.  Pretty straight forward program.
*
*********************************************************/

static char *sccs_id = "@(#)notch_filt.c	3.12 8/8/91 ESI";

char *Version = "3.12", *Date="8/8/91";
int debug_level = 0;
/*
 * System Includes
*/
# include <stdio.h>
# include <math.h>

/* 
 * ESPS Includes
*/
# include <esps/esps.h>
# include <esps/fea.h>
# include <esps/feafilt.h>
# include <esps/unix.h>
# include <esps/constants.h>

/* 
 * Defines
*/
# define NUM_CO 3   /* Number of numerator coefficients. */
# define DEN_CO 3   /* Number of denominator coefficients. */
# define NO_B 3     /* Number of frequency bands. */
# define SYNTAX	USAGE ("notch_filt [-P param_file][-x debug_level][-s sf][-n nf][-b bw][-g gain] filt_file");

/* 
* ESPS Functions
*/
    char *get_cmd_line();
    void co_to_pz_local();

main (argc, argv)
int argc;
char *argv[];
    {
    
    FILE *fpout;		    /*output stream pointer*/
    struct header *oh;		    /*output file header pointer*/
    struct feafilt *frec;	    /*output data rescord structure*/
    char *param_file = NULL;    /*parameter file name*/
    char *filt_file;		    /*output file name*/
    filtdesparams *fdp;

    float a[NUM_CO], b[DEN_CO], edges[2*NO_B], bgains[NO_B];
    static float wts[NO_B] = {1., 1., 1.}; /* equal wts for this filter*/
    double alpha, beta, lambda; 
    double gain = 1.0;		    /*scaling gain*/
    double tgain;
    double nf = -1;		    /*notch frequency*/
    double sf = -1;		    /*sampling frequency*/
    double bw = -1;		    /*notch bandwidth*/
    double sqrt (), cos (), atof ();
    int i, c;

    short pnco;
    int nroots;
    double rr[5], ii[5];

    extern char *optarg;
    extern optind;


/* Check the command line options. */

    while ((c = getopt (argc, argv, "P:x:s:n:b:g:")) != EOF)
	{
	switch (c)
	    {
	    case 'P':
		param_file = optarg;
		break;
	    case 'x':
		if (sscanf(optarg,"%d", &debug_level) != 1)
		    {
		    Fprintf (stderr, "Error reading -x arg. Exitting.\n");
		    exit (-1);
		    }
		if (debug_level != 0 && debug_level != 1)
		    {
		    debug_level = 1;
		    }
		break;
	    case 's':
		sf = atof (optarg);
		break;
	    case 'n':
		nf = atof (optarg);
		break;
	    case 'b':
		bw = atof (optarg);
		break;
	    case 'g':
		gain = atof (optarg);
		break;
	    default:
		SYNTAX
	    }
	}

/* Get the filenames. */

    if (optind < argc)
	{
	filt_file = argv [optind++];
	if (strcmp (filt_file, "-") == 0) 
	    {
	    filt_file = "<stdout>";
	    fpout = stdout;
	    }
	else TRYOPEN("notch_filt", filt_file, "w", fpout);
	if (debug_level) Fprintf (stderr,"notch_filt: Output file is %s\n", filt_file);
	}
    else
	{
	Fprintf (stderr,"notch_filt: No output file specified.\n");
	SYNTAX
	exit (1);
	}

/* Read parameter file if any necessary parameters are not yet set. */

    if (sf == -1 || nf == -1 || bw == -1)
	{
	if (debug_level)
	    {
	    Fprintf (stderr,"notch_filt: Reading parameter file %s\n", param_file);
	    }
	if (read_params (param_file, SC_NOCOMMON, (char *)NULL) != 0)
	    {
	    Fprintf (stderr,
		"notch_filt: read_params returned an error code.\n");
	    Fprintf(stderr, "\tNo params file specified\n");
	    exit(1);
	    }

	if (sf == -1){
	    if(symtype("samp_freq") != ST_UNDEF)
		sf = getsym_d ("samp_freq");
	    else{
		Fprintf(stderr, 
	          "notch_filt: Sampling frequency not specified - exiting.\n");
		exit(1);
	    }
	}

	if (nf == -1){
	    if(symtype("notch_freq") != ST_UNDEF)
		nf = getsym_d ("notch_freq");
	    else{
		Fprintf(stderr, 
	          "notch_filt: Notch frequency not specified - exiting.\n");
		exit(1);
	    }
	}
	    
	if (bw == -1){
	    if(symtype("band_width") != ST_UNDEF)
		bw = getsym_d ("band_width");
	    else{
		Fprintf(stderr, 
	          "notch_filt: Notch band width not specified - exiting.\n");
		exit(1);
	    }
	}
	    
    }

/* Check the values. */

    if (nf > sf/2)
        {
        Fprintf (stderr,"notch_filt: notch frequency=%f greater than the folding frequency=%f\n", nf, sf/2);
        exit (1);
        }
    if (nf - bw/2 < 0 || nf + bw/2 > sf/2)
        {
        Fprintf (stderr,"notch_filt: notch frequency +/- bw/2 exceeds 0-sf/2 limits.\n");
        exit (1);
        }

/* Create the output header and set key values. */

    oh = new_header (FT_FEA);
    (void)strcpy (oh->common.prog, "notch_filt");
    (void)strcpy (oh->common.vers, Version);
    (void)strcpy (oh->common.progdate, Date);
    (void)add_comment(oh, get_cmd_line(argc, argv));/*store command line*/
    oh->common.tag = YES;

    fdp = (filtdesparams *) calloc( 1, sizeof(filtdesparams));

    fdp->define_pz = YES;
    fdp->func_spec = (short)BAND;
    fdp->nbands = (short)NO_B;

    fdp->type = FILT_BS;
    fdp->method = PZ_PLACE;

    if (debug_level)
	{
	Fprintf (stderr,"notch_filt: max_num=%d max_den=%d nbands=%d\n", NUM_CO, DEN_CO, fdp->nbands);
	}

    edges[0] = 0;
    edges[1] = (nf - bw/2)/sf;
    edges[2] = nf/sf;
    edges[3] = nf/sf;
    edges[4] = (nf + bw/2)/sf;
    edges[5] = 0.5;
    fdp->bandedges = edges;

    bgains[0] = gain;
    bgains[1] = 0;
    bgains[2] = gain;
    fdp->gains = bgains;
    fdp->wts = wts;

/* Calculation of the coefficients. */

/* Calculate beta for the zero and pole locations. */

    beta = 2*PI*nf/sf;

/* Calculate lambda to get alpha. */

    lambda = cos (2*PI*(bw/2)/sf);

/* Calculate alpha for the bandwidth.  */

    alpha = lambda - sqrt (lambda*lambda - 8*lambda + 7);

/* Calculate the initial coefficients. */

    a[0] = a[2] = b[0] = 1.0;
    a[1] = -2*cos (beta);
    b[1] = alpha*a[1];
    b[2] = alpha*alpha;

/* Calculate the gain. */

tgain = (a[0] + a[1] + a[2])/(b[0] + b[1] + b[2]);

/* Normalize the filter gain. */

    for (i=0; i<NUM_CO; i++)
	{
	a[i] = gain*a[i]/tgain;
	} 

/* Print out the preliminary results. */

    if (debug_level)
	{
        Fprintf (stderr,"Input parameters:\n\n");
        Fprintf (stderr,"sampling frequency = %f, notch frequency = %f, -6db bandwidth = %f\n\n", sf, nf, bw);
        Fprintf (stderr,"Calculated parameters:\n\n");
        Fprintf (stderr,"alpha = %.16g, beta = %.16g, lambda = %.16g, tgain = %.16g\n", alpha, beta, lambda, tgain);
        for (i=0; i<NUM_CO; i++)
	    Fprintf(stderr,"notch_filt: a[%d] = %.16g\n", i, a[i]);
        for (i=0; i<DEN_CO; i++)
	    Fprintf(stderr,"notch_filt: b[%d] = %.16g\n", i, b[i]);
	}

/* Allocate the data record and fill in the values. */

    init_feafilt_hd( oh, NUM_CO, DEN_CO, fdp);
/*
 * Add generic header items: samp_freq, notch_freq, and band_width
*/

    (void)add_genhd_d("samp_freq", &sf, 1, oh);
    (void)add_genhd_d("notch_freq", &nf, 1, oh);
    (void)add_genhd_d("band_width", &bw, 1, oh);

    write_header (oh, fpout);

    frec = allo_feafilt_rec (oh);
    *frec->num_size = (long) NUM_CO;
    *frec->denom_size = (long) DEN_CO;
    for (i=0; i<NUM_CO; i++)
      frec->re_num_coeff[i] = a[i];
    for (i=0; i<DEN_CO; i++)
      frec->re_denom_coeff[i] = b[i];

    /* find zeros */
    co_to_pz_local( a, (short) NUM_CO, rr, ii, &nroots);
    *frec->zero_dim = nroots;
    for (i=0; i<nroots; i++) {
	frec->zeros[i].real = rr[i];
	frec->zeros[i].imag = ii[i];
    }

    co_to_pz_local( b, (short) NUM_CO, rr, ii, &nroots);
    *frec->pole_dim = nroots;
    for ( i=0; i<nroots; i++) {
	frec->poles[i].real = rr[i];
	frec->poles[i].imag = ii[i];
    }


/* Write the data to the output file. */

    put_feafilt_rec (frec, oh, fpout);

    (void) fclose (fpout);
    exit(0);
    return(0); /*lint pleasing*/
  }


void
co_to_pz_local( co, pnco, rr, ii, nroots)
float *co;
short pnco;
double *rr, *ii;
int *nroots;
{
    double a, b, c;
    double disc;
    double sqrt();

    float aa[5];
    short lpnco;
    int i;

    if (debug_level) {
	(void) fprintf(stderr, "factoring polynomial a: ");
	for (i=0; i<pnco; i++)
	    (void) fprintf(stderr, "a[%d] %e ", i, co[i]);
	fprintf(stderr,"\n");
    }

    if ( co[0] == 0.0 ) {
	if ( co[1] == 0.0 ) 
	    *nroots = 0;
	else {
	    *nroots = 1;
	    rr[0] = -co[2]/co[1];
	    ii[0] = 0.0;
	}
    } else { 
	c = co[2]/co[0];
	b = co[1]/co[0];
	a = 1.0;
	
	disc = b * b - 4.0 * c;
	
	if ( disc < 0.0 ) {
	    rr[0] = - b / 2.0;
	    ii[0] = sqrt( (double) (- disc) ) / 2.0;
	    *nroots = 1;
	} else {
	    *nroots = 2;
	    ii[0] = ii[1] = 0.0;
	    disc = sqrt(disc);
	    rr[0] = (- b + disc) / 2.0;
	    rr[1] = (- b - disc) / 2.0;
	}
    }
	
    if (debug_level) {
	(void)fprintf(stderr, "found %d distinct root(s)\n", *nroots);
	for (i=0; i<*nroots; i++)
	    fprintf(stdout, "root[%d] %e  %ei magnitude %e\n", 
		    i, rr[i], ii[i], sqrt(rr[i]*rr[i] + ii[i]*ii[i]));
	
	pz_to_co( *nroots, rr, ii, &lpnco, aa);
	    
	(void) fprintf(stderr, "reconstructed polynomial: a ");
	for (i=0; i<lpnco; i++)
	    (void) fprintf(stderr, "a[%d] %e ", i, aa[i]*co[0]);
	fprintf(stderr,"\n");
    }

    return;
}

