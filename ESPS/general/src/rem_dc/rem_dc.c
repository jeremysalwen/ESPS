/********************************************************
*
*  This material contains proprietary software of Entropic Speech, Inc.   
*  Any reproduction, distribution, or publication without the prior	   
*  written permission of Entropic Speech, Inc. is strictly prohibited.
*  Any public distribution of copies of this work authorized in writing by
*  Entropic Speech, Inc. must bear the notice			
* 								
*      "Copyright (c) 1987, 1990 Entropic Speech, Inc.; All rights reserved"
* 				
*
*  Module Name: rem_dc.c
*
*  Written By:   Brian Sublett   8/8/86
*  
*  Checked by: David Burton
*
*  Purpose:  This program reads data from an SPS sampled data
*	     computes the dc value, subtracts it from the input
*	     data, and prints this data to the output file.
*
*  Secrets:  None.
*
*********************************************************/

/*
 * Include Files
*/

# include <stdio.h>
# include <esps/esps.h>
# include <esps/fea.h>
# include <esps/feasd.h>
# include <esps/unix.h>

/*
 * Defines
*/
# define AR_LIM 4096    /* Store this many outputs before
			    printing to a file.  */
# define SYNTAX USAGE ("rem_dc [-x debug_level] [-r range] [-p range] [-l yrange] [-a offset] in_file out_file");
/*
 * System Functions
*/

double fabs();

/*
 * ESPS Functions
*/
char *eopen();
void lrange_switch();
void frange_switch();
void add_comment();
char *get_cmd_line();
void put_sd_recd();
char *e_temp_name();

/*
 * Global Variables
*/
    extern char *optarg;
    extern optind;
    
    int debug_level = 0;
/*
 * SCCS Stuff
*/
static char *sccs_id = "@(#)rem_dc.c	3.10 3/23/97 ESI";
char *Version="3.10", *Date="3/23/97"; 


main (argc, argv)
int argc;
char *argv[];
    {
    
    FILE  *fpin;		    /*input sampled data file stream ptr*/
    FILE *fpout = stdout;	    /*output sampled data file stream ptr*/
    char *in_file = "<stdin>";	    /*input file name*/
    char *out_file = "<stdout>";    /*output filename*/
    struct header *ih;		    /*input sampled data file header*/
    struct header *oh;		    /*output sampled data file header*/
    FILE *fptemp;		    /*stream ptr for temporary file*/
    char *temp_file;		/*temporary file name*/
    int i, c, ngot;		    
    double high = 0.0;		    /*maximum value for dc computation*/
    double low = 0.0;		    /*minimum value for dc computation*/
    double offset = 0.0;	    /*desired dc offset*/
    double dc = 0.0;		    /* computed dc level*/
    double *y;			    /*array to hold input data*/
    long start = 1;		    /*starting point*/
    long nleft = 0;		    /*number of points left*/
    long end = LONG_MAX;	    /*last point*/
    long nan;			    /* number of points to process*/
    long ncount;		    /*number of data points*/
    long total_pts;		    /* total points read*/
    int num_of_files = 0;	    /* holds # cmd line file names*/
    int common_read = NO;	    /* flag for common reading*/
    int r;

    long num_channels;              /* num of channels in samples field*/
    double max = -DBL_MAX;          /* holds maximum value */
    double min = DBL_MAX;           /*holds minimum value */
    double value;                   /*new max magnitude value */
    double start_time = 0;          /* holds value for genric header item*/
    double record_freq;             /*holds sampling freq. of input file*/
    int input_type;

/* Check the command line options. */

    while ((c = getopt (argc, argv, "x:r:p:l:a:")) != EOF)
	{
	switch (c)
	    {
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
            case 'r':
	    case 'p':
		lrange_switch (optarg, &start, &end, 1);
		break;
	    case 'l':
		frange_switch (optarg, &low, &high);
		break;
	    case 'a':
		offset = atof (optarg);
		break;
	    default:
		USAGE ("rem_dc [-x debug_level] [-r range] [-p range] [-l yrange] [-a offset] in_file out_file");
	    }
	}


/* 
 * Get the number of filenames on the command line
*/ 
    if((num_of_files = argc - optind) > 2){
	Fprintf(stderr, "Only two file names allowed\n");
	SYNTAX;
    }
    if(debug_level > 0)
	Fprintf(stderr, "rem_dc: num_of_files = %d\n", num_of_files);

    if(num_of_files == 0){
	Fprintf(stderr, "No output filename specified\n");
	SYNTAX;
    }

/*
 * Only output file specified on command line
*/
    if(num_of_files == 1){
	
    /* check common for input filename*/
       if((r=read_params((char *)NULL, SC_CHECK_FILE, (char *)NULL) ) != 0){
	    if(debug_level > 0)
		Fprintf(stderr, "Read_params returned %d\n", r);
	    Fprintf(stderr, 
           "No input file specified on command line and no Common exists\n");
	    exit(1);
	}
	if(symtype("filename") == ST_UNDEF){
	    Fprintf(stderr, 
	    "No input file specified on command line or in Common\n");
	    exit(1);
	}
	else{
	    if(debug_level > 0)
		Fprintf(stderr, "Input file name from Common is %s\n",
		getsym_s("filename"));
	    in_file = eopen("rem_dc", getsym_s("filename"), "r",
    		    FT_FEA, FEA_SD, &ih, &fpin);
	}

    	out_file = argv [optind];

       if ( strcmp(out_file, in_file) == 0 ) {
	   Fprintf(stderr, "rem_dc: Fatal Error. Identical input and output names.\n");
	   exit(1);
	 }
	 

	if (strcmp (out_file, "-") == 0) 
	    {
	    out_file = "<stdout>";
	    }
	else 
	    {
	    TRYOPEN("rem_dc", out_file, "w", fpout);
	    }
	if (debug_level) 
	    {
	    Fprintf (stderr,"Output file is %s\n", out_file);
	    }


	common_read = YES;
    }

/*
 * Both input and output file names specified on command line
*/

    if (num_of_files == 2)
	{
	in_file = eopen("rem_dc", argv[optind++], "r", FT_FEA, FEA_SD,
			&ih, &fpin);
	if (debug_level) Fprintf (stderr,"Input file is %s\n", in_file);


    	out_file = argv [optind];
	if (strcmp (out_file, "-") == 0) 
	    {
	    out_file = "<stdout>";
	    }
	else 
	    {
	    TRYOPEN("rem_dc", out_file, "w", fpout);
	    }
	if (debug_level) 
	    {
	    Fprintf (stderr,"Output file is %s\n", out_file);
	    }
	}


/* Open a temporary file.  */

    temp_file = e_temp_name(NULL);
    
    TRYOPEN ("rem_dc", temp_file, "w+", fptemp);
    if (debug_level) 
        {
        Fprintf (stderr,"Temporary file is %s\n", temp_file);
        }

/*
 * is data multichannel or complex - exit if so
*/
    if((num_channels = 
        get_fea_siz("samples", ih,(short *) NULL, (long **) NULL)) != 1){
           Fprintf(stderr, "rem_dc: Multichannel data not supported yet - exiting.\n");
           exit(1);
         }

    if(is_field_complex(ih, "samples") == YES)
      {
           Fprintf(stderr, "rem_dc: Complex data not supported - exiting.\n");
           exit(1);
	 }    

/* Create the output header.  */

    oh = new_header(FT_FEA);
    if((record_freq = get_genhd_val("record_freq", ih, (double)-1.))== -1){
      Fprintf(stderr, 
      "rem_dc: invalid record_freq (sf) in input file - exiting.\n");
      exit(1);
    }
    input_type = get_fea_type("samples", ih);
    init_feasd_hd(oh,input_type ,1, &start_time, 0, record_freq);
    add_source_file (oh, in_file, ih);
    (void)add_comment(oh, get_cmd_line(argc, argv));

/* Set other key values.  */

    (void)strcpy (oh->common.prog, "rem_dc");
    (void)strcpy (oh->common.vers, Version);
    (void)strcpy (oh->common.progdate, Date);

/* Set the range variables. */
    
    /* Check Common for start and nan, if appropriate */

    if(common_read == NO)
	(void)read_params((char *)NULL, SC_CHECK_FILE, in_file);

    if(symtype("start") != ST_UNDEF)
	    start = getsym_i("start");
	    
    if(symtype("nan") != ST_UNDEF)
	    end = start + getsym_i("nan") - 1;


    if (end <= start)
	{
	Fprintf (stderr, "rem_dc: start = %ld end = %ld  Exiting.\n", start, end);
	(void)unlink (temp_file);
	exit (1);
	}

    nleft = end - start + 1;

    if (debug_level)
	{
	Fprintf (stderr, "rem_dc: start = %ld end = %ld  nleft = %ld\n", start, end, nleft);
	Fprintf (stderr, "rem_dc: debug_level = %d offset = %f low = %f high = %f\n", debug_level,offset,low,high);
	}

/* Allocate and initialize the arrays. */

    y = (double *) calloc ((unsigned)AR_LIM, sizeof(double));
    spsassert(y != NULL, "Cannot allocate space for input data");

/* Find starting point.  */

    fea_skiprec (fpin, start - 1, ih);

    write_header(oh, fptemp);

/* Read in the data and compute the DC component. */

    ncount = 0;
    total_pts = 0;
    nan = end + 1 - start;
    while ((ngot = get_sd_recd(y, AR_LIM, ih, fpin)) != 0 && total_pts < nan)
	{
	total_pts += ngot;
	if(total_pts > nan){
	    ngot = ngot - (total_pts - nan);
	}
	for (i=0; i<ngot; i++)
	    {
	    if (y[i] > max)
	      max = y[i];
	    else if (y[i] < min)
	      min = y[i];
	    if (high == 0.0 && low == 0.0)
		{
		dc += y[i];
		ncount ++;
		}
	    else
		{
		if (y[i] <= high && y[i] >= low)
		    {
		    dc += y[i];
		    ncount ++;
		    }
		}
	    }
	(void)put_sd_recd (y, ngot, oh, fptemp);
	}

    if (ncount > 0)
	{
	dc /= ncount;
	dc -= offset;
	}
    else
	{
	Fprintf (stderr,"rem_dc: non-positive count.\n");
        (void)unlink (temp_file);
	exit (1);
	}


    if (debug_level)
	{
	Fprintf (stderr,"rem_dc: dc - offset = %g\n", dc);
	Fprintf (stderr,"rem_dc: comp range is start=%d end=%d\n", start, end);
	Fprintf (stderr,"rem_dc: number of samples used is ncount = %d\n", ncount);
	}

/* Write the header to the output file. */

  /*
   * first compute the new max_value generic
  */
      {
       if (dc > 0. && min < 0)/*max > 0*/
	    value = MAX((fabs(max - dc)), (fabs(min) + dc));
       else if (dc > 0. && min >= 0)/*max >0*/
	    value = MAX((fabs(max - dc)), (fabs(min - dc)));
       else if(dc <= 0. && max > 0)/*min < 0*/
	    value = MAX((max + fabs(dc)), (fabs(min - dc)));
       else /*dc <= 0 && max <=0 && min < 0*/
	    value = MAX((fabs(max - dc)), (fabs(min - dc)));
       (void)add_genhd_d("max_value", &value, 1, oh);
     }

    (void) add_genhd_d("dc_removed", &dc, 1, oh);

    (void)update_waves_gen(ih, oh, (float)start, (float)1);

    write_header (oh, fpout);
	    
/* Transfer data from temp to output. */

    rewind (fptemp);
    ih = read_header(fptemp);
    while ((ngot = get_sd_recd(y, AR_LIM, ih, fptemp)) != 0)
	{
	for (i=0; i<ngot; i++) y[i] -= dc;
	(void)put_sd_recd (y, ngot, oh, fpout);
	}


/*
 * Write Common, if appropriate
*/
    if(strcmp(out_file, "<stdout>") != 0){
	if(putsym_s("filename", out_file) != 0)
	    Fprintf(stderr, "Could not write into ESPS Common\n");
	(void)putsym_s("prog", argv[0]);
	(void)putsym_i("start", (int)1);
	(void)putsym_i("nan", (int)ncount);
    }

/* Close up.   */

    (void) fclose (fpin);
    (void) fclose (fpout);
    (void) unlink (temp_file);

    exit(0);
    return(0);
    }
