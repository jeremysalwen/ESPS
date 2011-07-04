/********************************************************
*
*  This material contains proprietary software of Entropic Speech, Inc.   
*  Any reproduction, distribution, or publication without the the prior	   
*  written permission of Entropic Speech, Inc. is strictly prohibited.
*  Any public distribution of copies of this work authorized in writing by
*  Entropic Speech, Inc. must bear the notice			
* 								
*      "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
* 				
*
* 	
*
*  Module Name: ils_esps.c
*
*  Written By:   Brian Sublett
*  Modifed for ESPS3 by David Burton
*  Checked by: 
*
*  Purpose:  This program reads data from an ILS sampled data
*	     and prints it out to an ESPS sampled data file.
*	     The program checks the ILS samplede data code
*	     to determine if it is necessary to do a byte
*	     swapping operation in order to correct different
*	     machine conventions for storing data.
*
*  Secrets:  The program calls the routines long_reverse(3-ESPS)
*	     and short_reverse(3-ESPS) in order to correct for
*	     the machine incompatibilities.
*
*********************************************************/

/*
 * SCCS Stuff
*/

static char *sccs_id = "@(#)ils_esps.c	3.5 1/7/93 ESI";
static char *Version = "3.5";
static char *Date = "1/7/93";



/* 
 * System Includes
*/
# include <stdio.h>
# include <math.h>
/*
 * ESPS INCLUDES
*/
# include <esps/esps.h>
# include <esps/fea.h>
# include <esps/feasd.h>
# include <esps/unix.h>

/*
 * DEFINES
*/
# define SIXTY_THREE 63 /*place of last read header item*/
# define FIFTY_EIGHT 58  /* How far to move in header before reading data*/
# define AR_LIM 4096    /* Store this many outputs before
			    printing to a file.  */
# define SD_CODE -32000L /* ILS sampled data flag. */
#define MU_LAW 50	    /* value that indicates mu law compression*/
# define COMMENT "This file converted from an ILS file by ils_esps (1-ESPS)\n"

			  /* This comment is placed in the output
			     file comment field.   */
# define SYNTAX	USAGE ("ils_esps [-x debug_level] [-r range] [-p range] [-g scale] [-d type] [-h size] [-q equipment] in_file out_file");
/*
 *SYSTEM FUNCTIONS
*/
/* Done via <esps/unix.h>*/

/*
 * ESPS FUNCTIONS
*/
char *eopen();
long long_reverse ();
short short_reverse ();
void lrange_switch();
void set_sd_type();
char *get_cmd_line();
void add_comment();

extern char *optarg;
extern optind;
int debug_level = 0;

main (argc, argv)
int argc;
char *argv[];
    {
    
    FILE  *fpin = stdin;	    /*input stream ptr*/
    FILE *fpout = stdout;	    /*output stream ptr*/
    FILE *fptmp;		    /* temporary file stream ptr*/
    char *in_file = "<stdin>";	    /* input file name*/
    char *out_file = "<stdout>";    /* output file name*/
    char tmp_file[50];		    
    struct header *oh;		    /*ptr to output header*/
    char type = 'f';		    /*holds data type symbol*/
    int data_type = FLOAT;          /*holds data type*/
    int dflag = 0;                  /*flag for d option*/

    int i, c, swap, n;
    float sf;			    /* sampling frequency*/
    double scale = 1.0;		    /* scale factor*/
    double expon;			    
    long start = 1;		    /*starting point*/
    long nleft;			    /*number of points left to get*/
    long end = LONG_MAX;	    /*last point*/


    short sizes = sizeof(short);
    short sized = sizeof(double);
    char *equip = "DSC";
    int size = 128;		    /* number of longs in the header*/
    short *x;			    /*hold short data*/
    double *y;			    /*hold double data*/

    long mu_law = 0;		    /*hold mu quantization code*/
    long code;			    /*holds file type code*/
    long sf_mant;		    /* sampling freq. body*/
    long sf_exp;		    /* sampling freq. exponent*/
    long multiplex = 0;		    /* holds # chans. represented*/

/* Check the command line options. */

    while ((c = getopt (argc, argv, "x:r:p:g:h:d:q:")) != EOF)
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
	    case 'g':
		if (sscanf (optarg, "%lf", &scale) != 1)
		    {
		    Fprintf (stderr,"ils_esps: error reading -g option.\n");
		    exit (1);
		    }
		break;
	    case 'h':
		size = atoi(optarg);
		break;
    	    case 'd':
		type = optarg[0];
		dflag++;
		break;
	    case 'q':
		equip = optarg;
		break;
	    default:
		SYNTAX
	    }
	}

/* Get the filenames. */

    if (optind < argc)
	{
	in_file = argv[optind++];
	if(strcmp(in_file, "-") == 0)
	    in_file = "<stdin>";
	else
	    TRYOPEN("ils_esps", in_file, "r", fpin);
	if (debug_level) Fprintf (stderr,"Input file is %s\n", in_file);
	}
    else
	{
	    Fprintf(stderr, "ils_esps: No input file specified\n");
	    SYNTAX;
	    exit(1);
	}
    if (optind < argc)
	{
	out_file = eopen("ils_esps", argv[optind++], "w", NONE, NONE,
		    (struct header **)NULL, &fpout);
	if (debug_level) Fprintf (stderr,"Output file is %s\n", out_file);
	}
    else
	{
	    Fprintf(stderr, "ils_esps: No output file specified\n");
	    SYNTAX;
	    exit(1);
	}

/* Define the temporary file name. */

    if (strcmp (out_file, "<stdout>") == 0)
	{
	(void)sprintf (tmp_file, "temp_file");
	}
    else
	{
	(void)sprintf (tmp_file, "%s.tmp", out_file);
	}

    TRYOPEN("ils_esps", tmp_file, "w+", fptmp);
    if (debug_level) Fprintf (stderr,"Temporary file is %s\n", tmp_file);

/* Read the ILS file header. */

    if(debug_level > 0){
        for(i=0;i<FIFTY_EIGHT;i++){
	    (void)fread((char *)&sf_exp, sizeof(long), 1, fpin);
		Fprintf(stderr, "%dth item = %ld\n", i,long_reverse(sf_exp));
	}
    }
    else
	 skiprec (fpin, (long)FIFTY_EIGHT, sizeof (long));

/*
 * Get needed parameters
 */
    (void)fread((char *)&multiplex, sizeof(long), 1, fpin);
    (void)fread((char *)&mu_law, sizeof(long), 1, fpin);
    if(mu_law == MU_LAW || long_reverse(mu_law) == MU_LAW){
	Fprintf(stderr, 
	    "ils_esps: Mu law compressed data not supported yet\n");	
	exit(1);
    }
    (void)fread ((char *)&sf_exp, sizeof(long), 1, fpin);
    (void)fread ((char *)&sf_mant, sizeof(long), 1, fpin);
    (void)fread ((char *)&code, sizeof(long), 1, fpin);

    if (debug_level) 
       Fprintf (stderr,
	"ils_esps: In hex: sf_exp=%x sf_mant=%x code=%x, multiplex = %x\n",
			     sf_exp, sf_mant, code, multiplex);


/* Swap bytes if needed */

    if (code == SD_CODE) {
        swap = 0;
	if(debug_level > 0)
	    Fprintf(stderr, "ils_esps:  No byte swap\n");
	}
    else
	{
	code = long_reverse (code);
	if (code == SD_CODE) 
	    {
	    if(debug_level > 0)
		Fprintf(stderr, "ils_esps: Byte swap\n");
            swap = 1;
	    sf_exp = long_reverse (sf_exp);
	    sf_mant = long_reverse (sf_mant);
	    multiplex = long_reverse(multiplex);
	    }
	else
	    {
	    Fprintf (stderr, "ils_esps: %s is not an ILS sampled data file.\n", in_file);
	    if (debug_level)
		{
		Fprintf (stderr, "ils_esps: swapped value of SD_CODE = %ld\n", code);
		code = long_reverse (code);
		Fprintf (stderr, "ils_esps: original value of SD_CODE = %ld\n", code);
		}
	    exit (1);
	    }
	}

    if (multiplex == 0) multiplex = 1;

    if (debug_level) Fprintf(stderr,"sf_mant=%ld sf_exp=%ld multiplex=%ld\n",
			     sf_mant, sf_exp, multiplex);

/* Skip pass rest of header to the beginning of the data.  */

    skiprec (fpin, (long)(size - SIXTY_THREE), sizeof (long));

/* Create the output header. */

    oh = new_header (FT_FEA);

    (void)strcpy (oh->common.prog, "ils_esps");
    (void)strcpy (oh->common.vers, Version);
    (void)strcpy (oh->common.progdate, Date);
    (void)add_comment(oh, COMMENT);
    (void)add_comment(oh, get_cmd_line(argc, argv));

/* Compute the sampling frequency. */

    expon = pow ((double)10.0, (double)sf_exp);
    sf = (float) sf_mant*expon;
    if (debug_level) 
        Fprintf(stderr,"sf_exp=%ld sf_mant=%ld exp=%lf sf=%f\n", sf_exp, sf_mant, expon, sf);

/*
 * Add generics
*/
    (void)add_genhd_c("a2d_equip", equip, 0, oh);
    (void)add_genhd_d("scale", &scale, 1, oh);

/*
 * Check data type
*/
    if(dflag){
        switch (type) {
            case 'B':
            case 'b': 
                data_type = BYTE;
                break;
            case 'S':
            case 's': 
                data_type = SHORT;
		type = 'w';	/* for put_ddata */
                break;
            case 'L':
            case 'l': 
                data_type = LONG;
                break;
            case 'F':
            case 'f': 
                data_type = FLOAT;
                break;
            case 'D':
            case 'd': 
                data_type = DOUBLE;
                break;
            default: 
                Fprintf (stderr, 
                         "filter: -d %c type unknown - exiting.\n", type);
                exit (1);
              }
      }



/* 
 * Initialize fea_sd header
*/

  
    {
      double start_time = 0.;
      if(
	 (init_feasd_hd
	  (oh, data_type, (int)multiplex, &start_time, (int)0, (double)sf) ) 
	  != 0){
	Fprintf(stderr, 
		"ils_esps: Couldn't allocate fea_sd header - exiting.\n");
	exit(1);
      }
    }
      
/* Set the range variables. */

    if (end <= start)
	{
	Fprintf (stderr, 
	"ils_esps: start = %ld end = %ld  Exiting.\n", start, end);
	exit (1);
	}

	nleft = end - start + 1;
	

    if (debug_level)
	{
	Fprintf (stderr, "ils_esps: start = %ld end = %ld  nleft = %ld\n", start, end, nleft);
	Fprintf (stderr, "ils_esps: debug_level = %d\n", debug_level);
	}

/* Allocate and initialize the arrays. */

    x = (short*) calloc ((unsigned)AR_LIM, sizeof(short));
    spsassert(x != NULL, "Cannot allocate memory for input data");
    y = (double*) calloc ((unsigned)AR_LIM, sizeof(double));
    spsassert(y != NULL, "Cannot allocate memory for output data");

/* Find starting point.  */

    skiprec (fpin, start - 1, sizes);

/* Read in and process the data to the temp file. */


    while (nleft > 0 && (n = fread((char *)x, sizes, AR_LIM, fpin)) > 0)
	{
	    nleft -= n;
	    if(nleft < 0) /* only do (nleft + n) points */
		n = nleft + n;

	    if (swap)
		{
		for (i=0; i<n; i++)
		    {
		    y[i] = (double) scale*short_reverse(x[i]);
		    }
		}
	    else
		{
		for (i=0; i<n; i++)
		    {
		    y[i] = (double) scale*x[i];
		    }
		}
	    put_ddata (fptmp,'d', y, n);
	}


/* Write the header to the output file. */

    write_header (oh, fpout);
	    
/* Transfer data from temp to output. */

    rewind (fptmp);
    while ((n = fread((char *)y, sized, AR_LIM, fptmp)) > 0)
	{
	    put_ddata (fpout, type, y, n);
	}

/* Close up.   */

    (void) fclose (fpin);
    (void) fclose (fpout);

    (void) fclose (fptmp);
    (void) unlink (tmp_file);

    exit(0);
    return(0);
    }
