/*********************************************************
*
*  This material contains proprietary software of Entropic Speech, Inc.   
*  Any reproduction, distribution, or publication without the the prior	   
*  written permission of Entropic Speech, Inc. is strictly prohibited.
*  Any public distribution of copies of this work authorized in writing by
*  Entropic Speech, Inc. must bear the notice			
* 								
*      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
* 				
*
* 	
*
*  Module Name: atofilt.c
*
*  Written By:   Brian Sublett   8/13/86
*  Modified for ESPS 3.0 by David Burton
*  Modified to FEAFILT by Bill Byrne 5/24/91
*
*  Description: This program reads data from one or two ascii
*	        data files and prints it to a FILT file.  The
*		numerator coefficients must be in one file, and
*		the denominator coefficients can (optionally) be
*		in the other or they can all be in the same file.  
*               The input files are in a format
*		where the first number is an integer showing how
*		many coefficients are in the file, and the
*		coefficients follow.
*
*  Secrets:  None.  Pretty straight forward program.
*
*********************************************************/

static char *sccs_id = "@(#)atofilt.c	3.6 1/7/93 ESI";

/*
 * System Includes
*/

# include <stdio.h>

/*
 * ESPS Includes
*/
# include <esps/esps.h>
# include <esps/fea.h>
# include <esps/feafilt.h>
# include <esps/unix.h>

/*
 * Defines
*/
# define MX_S 2047  /* Number of characters in user entered comments. */
# define COM_S 50  /* Extra characters in default comments. */
# define Date "1/7/93"
# define Version "3.6"
# define SYNTAX	USAGE ("atofilt [-x debug_level] [-c \"comment\"] num_file [den_file] filt_file");

int debug_level = 0;

main (argc, argv)
int argc;
char *argv[];
    {
    
    FILE *fpnum = stdin, *fpden;	        /* Input file stream pntrs*/
    FILE  *fpout;                               /*Output file stream pntr*/
    int i, c;
    int nsiz, dsiz = 0;				/*numerator and
						    denominator sizes */
    short cflag = NO;				/*Comment flag*/
    short  dflag = NO;				/*denominator flag*/
    char *num_file = "<stdin>";			/*numerator file name*/
    char *den_file;				/*denominator file name*/
    char *filt_file;				/*output file name*/
    float *a, *b;				/*input data ptrs*/
    char *comment1, *comment2;
    static char comment3[MX_S + 2];

    struct header  *oh;				/*output file header ptr*/
    struct feafilt *frec;			/*output data structure ptr*/
    filtdesparams *fdp;

    extern char *optarg;
    extern optind;


/* Check the command line options. */

    while ((c = getopt (argc, argv, "x:c:")) != EOF)
	{
	switch (c)
	    {
	    case 'x':
		if (sscanf(optarg,"%d", &debug_level) != 1)
		    {
		    Fprintf (stderr, "Error reading -x arg. Exitting.\n");
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
		    Fprintf (stderr,"atofilt: Comment field has %d characters\n", i);
		    Fprintf (stderr,"atofilt: Maximum exceptable is %d\n", MX_S);
		    exit (1);
		    }
		if (comment3[i-1] != '\n')
		    {
		    comment3[i] = '\n';
		    comment3[i+1] = '\0';
		    }
		cflag = YES;
		break;
	    default:
		SYNTAX;
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
	    TRYOPEN("atofilt", num_file, "r", fpnum);
	    }
	}

    if (debug_level) Fprintf (stderr,
		"atofilt: Numerator file is %s\n", num_file);

   /* Read the numerator coefficients. */

    if (fscanf (fpnum,"%d", &nsiz) != 1)
        {
        Fprintf (stderr,"atofilt: No coefficient number header.\n");
	exit (1);
	}
    if (nsiz < 0)
	{
	Fprintf (stderr,"nsiz = %d must be > zero.\n", nsiz);
	exit (1);
	}
    if (debug_level)
        Fprintf (stderr,"nsiz = %d\n", nsiz);

   /* Allocate space for the coefficients. */

    a = (float*) calloc ((unsigned)nsiz, sizeof (float));
    spsassert(a != NULL, 
		"Couldn't allocate space for numerator coefficients\n");

   /* read the coefficients. */

    for (i=0; i<nsiz; i++)
        {
        if (fscanf (fpnum,"%f", &a[i]) != 1)
	    {
	    Fprintf (stderr,
		"atofilt: Not %d coefficients in %s\n", nsiz, num_file);
	    exit (1);
	    }
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
	    cflag = YES; /* Can't get comment from standard input*/
	    }
	else if (strcmp (den_file, num_file) != 0) /*new file for denom*/
	    {
	    TRYOPEN("atofilt", den_file, "r", fpden);
	    }
        else /*use numerator file for both sets of coeffs*/
	    {
	    fpden = fpnum;
	    }

	if (debug_level) Fprintf (stderr,
			"atofilt: Denominator file is %s\n", den_file);

        if (fscanf (fpden,"%d", &dsiz) != 1)
	    {
	    Fprintf (stderr,"atofilt: No coefficient number header.\n");
	    exit (1);
	    }
	if (dsiz < 0)
	    {
	    Fprintf (stderr,"dsiz = %d; must be > zero.\n", dsiz);
	    exit (1);
	    }
	if (debug_level)
            Fprintf (stderr,"dsiz = %d\n", dsiz);

    /* Allocate space for the coefficients. */

        b = (float*) calloc ((unsigned)dsiz, sizeof (float));
	spsassert(b != NULL, "Couldn't allocate memory for denominator\n");

    /* read the coefficients. */

        for (i=0; i<dsiz; i++)
	    {
            if (fscanf (fpden,"%f", &b[i]) != 1)
	        {
	        Fprintf (stderr,"atofilt: Not %d coefficients in %s\n", dsiz, den_file);
	        exit (1);
	        }
	    }
    }
    else {/*no denominator file name specified*/
	/* allocate space for 1 denominator coefficient*/
        b = (float*) calloc ((unsigned)1, sizeof (float));
	spsassert(b != NULL, "Couldn't allocate memory for denominator\n");

	b[0] = 1.;
	}

 /*
  * Now get output FEAFILT file name
 */
	{
	filt_file = eopen("atofilt", argv [optind], "w", FT_FEA, FEA_FILT,
			    (struct header **)NULL, &fpout);

	if (debug_level) Fprintf (stderr,
				"atofilt: Output file is %s\n", filt_file);
	}


/* Get comments from standard input, if possible. */

    if (cflag == NO)
	{
	(void)printf ("atofilt: Please enter comments.\n");
	(void)printf ("atofilt: Must be less than %d characters and terminated with ^D.\n", MX_S);
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
    Strcpy (oh->common.prog, "atofilt");
    Strcpy (oh->common.vers, Version);
    Strcpy (oh->common.progdate, Date);
    
    fdp = (filtdesparams *) calloc( (unsigned) 1, sizeof(filtdesparams));
    spsassert( fdp!=NULL, "atofilt: can't allocate filter header parameters.");

    fdp->filter_complex = fdp->define_pz = NO;
    fdp->func_spec = (short) NONE;
    fdp->type = FILT_ARB;
    fdp->method = PZ_PLACE;

    init_feafilt_hd( oh, (long) nsiz, (long) dsiz, fdp);

/* Fill in the comment field. */

    comment1 = (char*) calloc ((unsigned)(COM_S+strlen(num_file)), sizeof (char));
    spsassert(comment1 != NULL,
		    "Couldn't allocate memory for numerator comment\n");
    (void)sprintf (comment1,
	    "Numerator coefficients came from the file %s\n", num_file);
    if (strlen (comment1) > MAX_STRING)
	{
	comment1[MAX_STRING - 1] = '\0';
	Fprintf (stderr,"atofilt: Warning, comment field was truncated.");
	}
    else
	{
        oh->variable.comment = comment1;
        if (dflag == YES)
	    {
            comment2 = (char*) calloc((unsigned)(COM_S+strlen(den_file)), sizeof (char));
	    spsassert(comment2 != NULL, 
	    "Couldn't allocate memeory for denominator comment\n");
	    (void)sprintf (comment2,"Denominator coefficients came from the file %s\n", den_file);
	    add_comment (oh, comment2);
	    }
	if (cflag == YES && dflag == NO)
	    {
            add_comment (oh, comment3);
	    }
	}

/* Allocate the data record and fill in the values. */

    frec = allo_feafilt_rec (oh);
    *frec->num_size = nsiz;
    *frec->denom_size = dsiz;
    for (i=0; i<nsiz; i++)
      frec->re_num_coeff[i] = (double)  a[i];
    for (i=0; i<dsiz; i++)
	frec->re_denom_coeff[i] = (double) b[i];

/* Write the data to the output file. */

    write_header (oh, fpout);
    put_feafilt_rec (frec, oh, fpout);

    
    (void) fclose (fpout);
    exit(0);
    return(0); /*lint pleasing*/
    }
