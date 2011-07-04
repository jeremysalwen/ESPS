/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Derek Lin, a rewrite of previous filtspec
 * Checked by:
 * Revised by:
 *
 * Brief description: Get magnitude and phase by using poles and zeros,
 *     instead of from numerator/denominator coeffs, which cause numerical
 *     problem when the filter order is high
 *
 */

static char *sccs_id = "@(#)filtspec.c	3.17	8/31/95	ERL";

#include <stdio.h>
#include <math.h>

#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feaspec.h>
#include <esps/feafilt.h>

/*
 * Defines
*/
#define SYNTAX USAGE ("filtspec [-x debug_level][-r range]\n [-s sampling_frequency] [-n num_freqs] [-m mode] filt_file spec_file")

#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
					 argv[0], text); exit(1);}

#define SMALL 1.0e-30
#define LOGSMALL -600
#define MAXFREQ 8192
#define LIN 1
#define DB 0
#define COM 2

/*
 * ESPS Functions
*/
void get_fftd();
char *get_cmd_line();
void lrange_switch();

int debug_level = 0;


/*
 * Main
 */
main(argc, argv)
    int     argc;
    char    **argv;
{
    char    *Version = "3.17";
    char    *Date = "8/31/95"; 
    char    *ProgName = "filtspec";
    extern  optind;
    extern char  *optarg;
    char    *inpfile = "<stdin>";	/* input file name */
    char    *out_file = "<stdout>";	/* output file name */
    struct header *ih, *oh;		/* in and out file header ptrs */
    FILE    *istrm = stdin,		/* input file stream ptr */
            *ostrm = stdout;		/* output file steam ptr */
    struct feafilt *filt_rec;		/* FILT data structure */
    struct feaspec *spec_rec;		/* FEA_SPEC data structure */
					/* numerator and denominator */
    long    srec, erec;	                /* starting, ending record numbers */
    long    nrec;			/* record counter */
    long    num_freqs = 2049;	        /* size of fft in disrete frequecies */
    short   mode = DB;			/* data representation */
    int     spec_type;                  /* holds spectral type */


    char    *range = NULL;		/* holds range argument */

    double  sf = 1.0;			/* assumed sampling frequency */
    int sf_flag = NO;
    int fil_spectrum();
    int i,c;
    int status;

/* Interpret the arguments. */

    while ((c = getopt(argc, argv, "x:r:n:m:s:")) != EOF)
    switch (c)
    {
    case 'x': 
        debug_level = atoi(optarg);
        break;
    case 'r': 
        range = optarg;
        break;
    case 'n':
        num_freqs = atoi(optarg);
        break;
    case 'm':
	if (*optarg == 'l') mode = LIN;
	else if (*optarg == 'd') mode = DB;
	else if (*optarg == 'c') mode = COM;
	else ERROR_EXIT("mode options are l, d,  and c.\n");
	break;
    case 's':
	sf_flag = YES;
	sf = atof (optarg);
	if (sf < 0) ERROR_EXIT("specified an illegal sampling frequency");
	break;
    default: 
        SYNTAX;
        break;
    }
    if (optind < argc){
        inpfile = eopen("filtspec", argv[optind++], "r", FT_FEA, FEA_FILT,
			    &ih, &istrm);
    }
    else{
	SYNTAX;	
    }
    if (debug_level)
	Fprintf(stderr,"%s: input file is %s\n", argv[0],inpfile);

    if (optind < argc){
        out_file = eopen("filtspec", argv[optind], "w", NONE, NONE,
			(struct header **)NULL, &ostrm);
    }
    else{
	SYNTAX;
    }
    if (debug_level)
	Fprintf(stderr,"%s: output file is %s\n", argv[0], out_file);

/*
 * Get sampling frequency from header if appropriate
*/
    {
	int size, type;
	if(sf_flag == NO && 
	    (type = genhd_type("samp_freq", &size, ih)) != HD_UNDEF)
		switch(type){
		    case DOUBLE:
			sf = *get_genhd_d("samp_freq", ih);
			break;
		    case FLOAT:
			sf = *get_genhd_f("samp_freq", ih);
			break;
		    case LONG:
			sf = *get_genhd_l("samp_freq", ih);
			break;
		    case SHORT:
			sf = *get_genhd_s("samp_freq", ih);
			break;
		    default:
			Fprintf(stderr, 
	   "filtspec: WARNING - Default sampling frequency = 1 being used.\n");
			break;
		}
    }

    if (num_freqs > MAXFREQ){
        Fprintf(stderr, "%s: number of frequencies %ld ", argv[0], num_freqs);
        Fprintf(stderr, "exceeds %d.\n", MAXFREQ);
        exit(1);                    
      }

/* 
 * Get subrange 
 */
    srec = 1;
    erec = LONG_MAX;
    (void) lrange_switch(range, &srec, &erec, 1);
    if (erec < srec ||  srec < 1) {
      Fprintf(stderr, "%s: bad range specification.\n", argv[0]);
      exit(1);
    }

/* 
 * Make output header 
 */

    oh = new_header(FT_FEA);
    add_source_file(oh, inpfile, ih);
    add_comment(oh, get_cmd_line(argc, argv));
    (void) strcpy(oh->common.prog, ProgName);
    (void) strcpy(oh->common.vers, Version);
    (void) strcpy(oh->common.progdate, Date);
    if(ih->common.tag == YES){
	oh->common.tag = YES;
	oh->variable.refer = ih->variable.refer;
    }
    if (mode == DB)
        spec_type   = SPTYP_DB;
    else if (mode == LIN)
        spec_type   = SPTYP_REAL;
    else
        spec_type   = SPTYP_CPLX;

    init_feaspec_hd(oh, NO, SPFMT_SYM_EDGE, spec_type,
		    YES, num_freqs, SPFRM_NONE, (float *)NULL,
		    (double)sf, (long)NULL, FLOAT);

    if (debug_level){
	Fprintf (stderr,"%s: start=%d,  num_freqs=%ld\n", argv[0],
		 srec, num_freqs);
	Fprintf (stderr,"%s: sf = %g\n",argv[0], sf);
      }

/* Add generic header items */
    (void) copy_genhd(oh, ih, "src_sf");

/*
 * Write header
 */
    write_header(oh, ostrm);

/* 
 * Allocate
 */
    filt_rec = allo_feafilt_rec(ih);
    spec_rec = allo_feaspec_rec(oh, FLOAT);

/* 
 * Skip record
 */

    fea_skiprec( istrm, srec-1, ih);

/*
 * Main Loop - Get each FILT record and convert to magnitude squared spectrum
 */

    nrec = srec;
    while (nrec++ <= erec && get_feafilt_rec(filt_rec, ih, istrm) != EOF){
      if(ih->common.tag == YES)
	*spec_rec->tag = filt_rec->fea_rec->tag;
      
      if (mode == LIN || mode == DB){
	double *mag;
	mag = (double *) malloc (num_freqs *sizeof(double));
	
	status=fil_spectrum(NULL, mag, NULL, filt_rec,
				 ih, num_freqs);
	
	if (mode == DB){
	  for (i=0; i<num_freqs; i++)
	    if (mag[i] < SMALL)
	      spec_rec->re_spec_val[i] = LOGSMALL;
	    else
	      spec_rec->re_spec_val[i] = (float)20.0*log10 (mag[i]);
	}
	else
	  for(i=0;i<num_freqs;i++)
	    spec_rec->re_spec_val[i] = (float) mag[i];
      }	  
      else
	status = fil_spectrum(spec_rec, NULL, NULL, filt_rec, 
				    ih, num_freqs);

      if(status!=0) ERROR_EXIT("error in fil_spectrum");
	
      (void)put_feaspec_rec (spec_rec, oh, ostrm);
      
    }
    exit (0);
  }





