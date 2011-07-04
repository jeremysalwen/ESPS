/*********************************************************
*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
    "Copyright (c) 1987, 1989, 1990 Entropic Speech, Inc.; All rights reserved"
*
*  Module Name: zero_pole.c
*
*  Written By:   Brian Sublett   1-30-87
*  Modified for ESPS 3.0 by David Burton
*  Modified to write FEAFILT files by Bill Byrne, 5/24/91
*  Checked by: 
*
*  Description: This program reads data from one or two ascii
*	        data files representing zeros and poles in the
*		complex plane.  It the computes the filter
*		coefficients and prints the results to a FILT
*		file.
*
*  
*
*********************************************************/

static char *sccs_id = "@(#)zero_pole.c	3.8 1/6/93  ESI";
char *Version="3.8", *Date="1/6/93";

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
# define MX_S 2047  /* Number of characters in user entered comments. */
# define COM_S 50  /* Extra characters in default comments. */
# define SYNTAX	USAGE ("zero_pole [-x debug_level] [-c commnet] [-g gain] [-f freq] num_file [den_file] feafilt_file");
# define SMALL 1.0e-8

int debug_level =0;

main (argc, argv)
int argc;
char *argv[];
    {
    
    FILE *fpnum = stdin, *fpden, *fpout;    /* FILE stream ptrs -
					numerator, denominator, and output*/
    char *num_file = "<stdin>";		/* numerator coeff file name*/
    char *den_file;			/* denominator coeff filename*/
    char *filt_file;			/* output FILT filename*/
    struct header *oh;			/* FILT file header ptr*/
    struct feafilt *frec;		/* FEAFILT data structure ptr*/
    filtdesparams *fdp;
    int i, c;
    int nzero, npole = 0;		/* number of zeros and poles*/
    short cflag = NO;			/* Comment flag*/
    short dflag = NO;			/* denominator flag*/

    int maxnum, maxdenom;
    float *zptr, *pptr;
    short n_dim, d_dim;

    int  gain_flag = 0;  /* option flags*/
    double *za, *zb, *pa, *pb;		/* zero and pole locations*/
    double g = 1.0, f = 0.0, hypot (), sin (), cos ();
    double rpart, ipart, gain;
    char *comment1, *comment2;
    char   comment3[MX_S + 2];


    extern char *optarg;
    extern optind;


/* Check the command line options. */

    while ((c = getopt (argc, argv, "x:c:g:f:")) != EOF)
	{
	switch (c)
	    {
	    case 'x':
		if (sscanf(optarg,"%d", &debug_level) != 1)
		    {
		    Fprintf(stderr, "Error reading -x arg. Exitting.\n");
		    exit (1);
		    }
		if (debug_level != 0 && debug_level != 1)
		    {
		    debug_level = 1;
		    }
		break;
	    case 'c':
		if ((i = strlen (optarg)) <= MX_S)
		    Strcpy (comment3, optarg);
		else
		    {
		    Fprintf (stderr,"zero_pole: Comment field has %d characters\n", i);
		    Fprintf (stderr,"zero_pole: Maximum exceptable is %d\n", MX_S);
		    exit (1);
		    }
		if (comment3[i-1] != '\n')
		    {
		    comment3[i] = '\n';
		    comment3[i+1] = '\0';
		    }
		cflag = YES;
		break;
	    case 'g':
		if (sscanf (optarg, "%lf", &g) != 1)
		    {
		    Fprintf (stderr, "zero_pole: Error reading -g arg. Exitting.\n");
		    exit (1);
		    }
		gain_flag = 1;
		break;
	    case 'f':
		if (sscanf (optarg, "%lf", &f) != 1)
		    {
		    Fprintf (stderr, "zero_pole: Error reading -f arg. Exitting.\n");
		    exit (1);
		    }
		if (f < 0 || f > 1.0)
		    {
		    Fprintf (stderr,"zero_pole:f = %g invalid. f set to 0.\n", f);
		    }
		gain_flag = 1;
		break;
	    default:
		USAGE ("zero_pole [-x debug_level] feafilt_file num_file [den_file]");
	    }
	}

/* Get the filenames. */

    if(argc - optind < 2 || argc - optind > 3){
      SYNTAX;
      exit(1);
    }

 /*
  * Now get input numerator file name
 */

	{
	num_file = argv [optind++];
	if (strcmp (num_file, "-") == 0) 
	    {
	    num_file = "<stdin>";
	    cflag = YES; /*cannot get comment from stdin*/
	    }
	else 
	    {
	    TRYOPEN("zero_pole", num_file, "r", fpnum);
	    }
	}

    if (debug_level) Fprintf (stderr,
		"zero_pole: Numerator file is %s\n", num_file);

/* Read the number of zeros. */

    if (fscanf (fpnum,"%d", &nzero) != 1)
        {
        Fprintf (stderr,"zero_pole: Number of zeros not specified.\n");
	exit (1);
	}
    if (nzero < 0)
	{
	Fprintf (stderr,"nzero = %d is unacceptable - must be > zero.\n", 
				    nzero);
	exit (1);
	}
    if (debug_level)
        Fprintf (stderr,"nzero = %d\n", nzero);

/* Allocate space for the coefficients. */

    za = (double*) calloc ((unsigned)nzero, sizeof (double));
    spsassert(za != NULL, 
		"Couldn't allocate memory for zero coefficients\n");
    zb = (double*) calloc ((unsigned)nzero, sizeof (double));
    spsassert(zb != NULL, 
		"Couldn't allocate memory for zero coefficients\n");

/* read the zeros. */

    for (i=0; i<nzero; i++)
        {
        if (fscanf (fpnum,"%lf %lf", &za[i], &zb[i]) != 2)
	    {
	    Fprintf (stderr,
			"zero_pole: Not %d zeros in %s\n", nzero, num_file);
	    exit (1);
	    }
	}

    if (debug_level)
	{
	for (i=0; i<nzero; i++)
	    Fprintf (stderr,"zero_pole: z%d = %g + %g i\n", i, za[i], zb[i]);
	}

 /*
  * Get denominator coefficients, if appropriate
 */
    if (argc - optind > 1)
      {
	den_file = argv [optind++];
	dflag = YES;
	if (strcmp (den_file, "-") == 0) 
	    {
	    den_file = "<stdin>";
	    fpden = stdin;
	    cflag = YES; /* can't use standard input for comment*/
	    }
	else if (strcmp (den_file, num_file) != 0)
	    {
	    TRYOPEN("zero_pole", den_file, "r", fpden);
	    }
	else
	    {
	    fpden = fpnum;
	    }

	if (debug_level) 
	    Fprintf (stderr,"zero_pole: Pole file is %s\n", den_file);

        if (fscanf (fpden,"%d", &npole) != 1)
	    {
	    Fprintf (stderr,
		"zero_pole: Number of poles not specified.\n");
	    exit (1);
	    }
	if (npole < 0)
	    {
	    Fprintf (stderr,"npole = %d is unacceptable - must be > zero.\n"
			, npole);
	    exit (1);
	    }
	if (debug_level)
            Fprintf (stderr,"npole = %d\n", npole);

/* Allocate space for the coefficients. */

        pa = (double*) calloc ((unsigned)npole, sizeof (double));
    spsassert(pa != NULL, 
		"Couldn't allocate memory for pole coefficients\n");
        pb = (double*) calloc ((unsigned)npole, sizeof (double));
    spsassert(pb != NULL, 
		"Couldn't allocate memory for pole coefficients\n");

/* read the coefficients. */

        for (i=0; i<npole; i++)
	    {
            if (fscanf (fpden,"%lf %lf", &pa[i], &pb[i]) != 2)
	        {
	        Fprintf (stderr,
		    "zero_pole: Not %d poles in %s\n", npole, den_file);
	        exit (1);
	        }
	    }
        if (debug_level)
	    {
	    for (i=0; i<npole; i++)
	        Fprintf (stderr, "zero_pole: p%d = %g + %g i\n", i, pa[i], pb[i]);
	    }

      }

 /*
  * Now get output FEAFILT file name
 */
	{
	filt_file = eopen("zero_pole", argv[optind], "w", FT_FEA, FEA_FILT,
			    (struct header **)NULL, &fpout);

	if (debug_level) Fprintf (stderr,
				"zero_pole: Output file is %s\n", filt_file);
	}


/* Get comments from standard input. */

    if (cflag == NO)
	{
	(void)printf ("zero_pole: Please enter comments.\n");
	(void)printf ("zero_pole: Must be less than %d characters and terminated with ^D.\n", MX_S);
	(void)fflush (stdout);
	i = 0;
	while ((int)(comment3[i] = getchar()) != EOF && i < MX_S)
	    {
	    i ++;
	    }

/* Make sure the comment ends in a \n and a \0. */

	if (comment3[i-1] != '\n')
	    {
	    comment3[i] = '\n';
	    i ++;
	    }
	comment3 [i] = '\0';
	cflag = YES;
	}

/* Create the output header and set key values. */

    oh = new_header (FT_FEA);
    oh->common.tag = YES;
    Strcpy (oh->common.prog, "zero_pole");
    Strcpy (oh->common.vers, Version);
    Strcpy (oh->common.progdate, Date);

    fdp = (filtdesparams *) calloc( (unsigned) 1, sizeof(filtdesparams) );
    spsassert( fdp != NULL, "zero_pole: can't allocate header info memory.");

    fdp->method = PZ_PLACE;
    fdp->define_pz = YES;
    fdp->filter_complex = NO;
    fdp->type = FILT_ARB;
    fdp->func_spec = NONE;


/* Fill in the comment field. */

    comment1 = (char*) calloc ((unsigned)(COM_S+strlen(num_file)), sizeof (char));
    spsassert(comment1 != NULL, "Couldn't allocate memory for comment\n");
    (void)sprintf (comment1,"Zeros came from the file %s.\n", num_file);
    if (strlen (comment1) > MAX_STRING)
	{
	comment1[MAX_STRING - 1] = '\0';
	Fprintf (stderr,"zero_pole: Warning, comment field was truncated.");
	}
    else
	{
        oh->variable.comment = comment1;
        if (dflag == YES)
	    {
            comment2 = (char*) calloc ((unsigned)(COM_S+strlen(den_file)), 
					sizeof (char));
	    spsassert(comment2 != NULL, 
		    "Couldn't allocate memory for pole comment\n");
            (void)sprintf (comment2,
		    "Poles came from the file %s.\n", den_file);
	    add_comment (oh, comment2);
	    }
	}

    add_comment(oh, comment3);

/* Allocate the data record and fill in the values. */


    maxnum = 2*nzero + 1;
    maxdenom = 2*npole + 1;

    zptr = (float *) calloc( (unsigned) maxnum, sizeof(float));
    spsassert( zptr != NULL, "can't allocate memory for numerator poly.");

    if ( npole > 0 ) {
	pptr = (float *) calloc( (unsigned) maxdenom, sizeof(float));
	spsassert ( pptr != NULL, "can't allocate memory for denominator poly.");
      }

    init_feafilt_hd( oh, (long) maxnum, (long) maxdenom, fdp);

    write_header (oh, fpout);

    frec = allo_feafilt_rec (oh);

    (void)pz_to_co (nzero, za, zb, &n_dim, zptr);
    *frec->num_size = (long) n_dim;

    if (npole > 0)
	{
       (void)pz_to_co (npole, pa, pb, &d_dim, pptr);
       *frec->denom_size = (long) d_dim;
	/* Check stability. */
	for (i=0; i < npole; i++)
	    {
	    if (hypot(pa[i], pb[i]) >= 1.0)
		{
		Fprintf (stderr,"zero_pole:Warning, poles not inside the unit circle.\n");
		Fprintf (stderr,"p%d = %g + %g i\n", i, pa[i], pb[i]);
		}
	    }
	}
    else
      *frec->denom_size = (long) 0;

/* Normalize the gain if desired. */

    if (gain_flag)
	{
	rpart = ipart = 0.0;
	for (i=0; i < n_dim; i++)
	    {
	    rpart += zptr[i]*cos (2*PI*f*i);
	    ipart += zptr[i]*sin (2*PI*f*i);
	    }
	gain = hypot (rpart, ipart);
	if (d_dim > 0)
	    {
	    rpart = ipart = 0.0;
	    for (i=0; i < d_dim; i++)
	        {
	        rpart += pptr[i]*cos (2*PI*f*i);
	        ipart += pptr[i]*sin (2*PI*f*i);
	        }
	    if (rpart != 0.0 || ipart != 0.0)
	        gain /= hypot (rpart, ipart);
	    else
		gain = -1;
	    }
	if (debug_level)
	    {
	    Fprintf (stderr,"zero_pole: gain = %g (before normalization)\n", gain);
	    }
	if (gain >= 0.0 && gain < SMALL)
	    {
	    Fprintf (stderr,"zero_pole: gain at f=%lf is zero. Gain will not be changed. \n", f);
	    }
	else if (gain == -1)
	    {
	    Fprintf (stderr,"zero_pole: gain not adjusted.\n");
	    }
	else
	    {
	    for (i=0; i < n_dim; i++)
	        {
	        zptr[i] /= gain;
	        }
	    }
	}
    /* Write the data to the output file. */

    *frec->zero_dim = nzero;
    *frec->pole_dim = npole;

    for (i=0; i<n_dim; i++)
      frec->re_num_coeff[i] = (double) zptr[i];

    for (i=0; i<d_dim; i++)
      frec->re_denom_coeff[i] = (double) pptr[i];

    for (i=0; i<nzero; i++) {
	frec->zeros[i].real = za[i];
	frec->zeros[i].imag = zb[i];
      }

    for (i=0; i<npole; i++) {
	frec->poles[i].real = pa[i];
	frec->poles[i].imag = pb[i];
      }

    put_feafilt_rec (frec, oh, fpout);

    
    (void) fclose (fpout);
    exit(0);
    return(0); /* lint pleasing */
    }
