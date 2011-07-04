/* 
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
               "Copyright 1990 Entropic Speech, Inc."
 				
  Module Name:  addsd or multsd

  Written By:  Ajaipal S. Virdy
  Modified for ESPS By: David Burton
  Checked By:  Alan Parker and David Burton
  Converted to ESPS 3.0 by David Burton and John Shore
  Converted to read and write FEA_SD files by David Burton
  Converted to do multiplication (when called as multsd) by David Burton

 */

#ifndef lint
static char *sccs_id = "@(#)addsd.c	3.16	11/15/96 ESI";
#endif

#define VERSION "3.16"
#define DATE "11/15/96"

/* system Includes*/
#include <stdio.h>

/*ESPS Includes*/
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/unix.h>

#define SYNTAX_ADD USAGE \
("addsd [-x debug-level] [-{pr} range] [-g scale] [-z] [-t] file1 file2 file3")

#define SYNTAX_MULT USAGE \
("multsd [-x debug-level] [-{pr} range] [-g scale] [-z] [-t] file1 file2 file3")

#define REQUIRE(test,text1,text2) {if (!(test)) {(void) fprintf(stderr, \
"%s: %s - exiting\n", text1, text2); exit(1);}}

#define SIZE	1024	/* size of data buffers */

/* ESPS Functions */
    void lrange_switch();
    int getopt(); 
    int read_params();
    void symerr_exit();
    void add_comment();
    int putsym_i();
    int putsym_s();
    char *eopen();
    double_cplx realmult();
    double_cplx cadd();
    short cover_type();
    double_cplx cmult();

extern  optind;
extern	char *optarg, *get_cmd_line();
int	debug_level = 0;

main (argc, argv)	/* program to add or multiply two sampled data files */
int     argc;
char  **argv;
{
    int		c;

    FILE 	   *f1p, *f2p, *f3p;		/* file pointers */
    struct header  *f1h, *f2h, *f3h;		/* pointers to structure */
    char	   *f1str, *f2str, *f3str;	/* string pointers */
    struct feasd  *sd_rec1, *sd_rec2, *sd_rec3; /* data record pointers*/

    int		gflag = 0;		/* scale flag */
    int		zflag = 0;		/* inhibit warning message flag */
    int		tflag = 0;		/* truncate flag */
    char 	*range = NULL;		/* string to hold range */

    double	scale;		/* scale file2 before adding to file1 */

    char	combuf[1024];	/* command buffer for writing to header */

    long	s_rec; 		/* starting record position */
    long	e_rec;		/* ending record position */
    long	n_rec;  	/* number of records in file1 to use */
    long	total_points = 0; /* total number of points written to f3*/
    int		npoints1;	/* number of points read from f1 */
    int		npoints2;	/* number of points read from f2 */
    int		rewound = 0;	/* flag to indicate f2 rewound */
    int		more = 1;	/* flag for reading data from f1 */

    double dbuf[SIZE];	        /* buffer for storing scaled data*/
    double_cplx cdbuf[SIZE];    /* buffer for storing scaled data*/

    long	i = 0;		/* indexing variable */
    double      sf;             /* record frequency or sampling frequency*/
    double *data1, *data2, *data3; 
    double_cplx *cdata1, *cdata2, *cdata3;
    short in_type1;              /* holds input type for file 1*/
    short in_type2;              /* holds input type for file 2*/
    int out_data_type;           /* holds output data type*/
    int complex=0;                 /* is either file 1 or 2 complex*/
    int complex1=0;                /* is input file 1 complex: yes or no*/
    int complex2=0;                /* is input file 2 complex: yes or no*/
    int num_channels;            /* number of input channels in fea_sd file*/
    double start_time = 0;       /* start time generic value*/
    int pflag = 0;               /* range flag*/
    
/*
 * Check calling name - must be addsd or multsd
*/
    if(strcmp("addsd", argv[0]) != 0 && strcmp("multsd", argv[0]) != 0){
      Fprintf(stderr, 
"Program name cannot be linked or copied to anything but \"addsd\" or \"multsd\"\n");
      exit(1);
    }

/* get options from command line and set various flags */

    while ((c = getopt (argc, argv, "x:p:r:g:zt")) != EOF) {
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		if(debug_level > 0)
		    Fprintf(stderr, "debug_level = %d\n",
		    debug_level);
		break;
	    case 'p': 
	    case 'r': 
		pflag++;
		range = optarg;
		break;
	    case 'g': 
		gflag++;
		scale = atof (optarg);
		if(scale == 0)
		    Fprintf(stderr, "%s: WARNING - zero gain specified\n", argv[0]);
		if (debug_level > 0)
		   Fprintf (stderr, "%s: scale is %lf\n", argv[0], scale);
		break;
	    case 'z': 
		zflag++;
		break;
	    case 't': 
		tflag++;
		break;
	    default:
		if(strcmp(argv[0], "addsd") == 0)
		   SYNTAX_ADD;
		if(strcmp(argv[0], "multsd") == 0)
		  SYNTAX_MULT;
	      }
      }

/*
 * check for multiple range specifications
*/
    if(pflag > 1){
      Fprintf(stderr, 
	      "%s: range should only be specified once - exiting.\n",argv[0]);
      exit(1);
    }
/* user must supply three file names: file1 file2 file3 */

    if (argc - optind < 3) {
		if(strcmp(argv[0], "addsd") == 0)
		   SYNTAX_ADD;
		if(strcmp(argv[0], "multsd") == 0)
		  SYNTAX_MULT;
	      }
/* open first input file*/

    f1str = eopen(argv[0], argv[optind++], "r", FT_FEA, FEA_SD, &f1h, &f1p);

/* open the second file (it usually represents noise) */

    f2str = eopen(argv[0], argv[optind], "r", FT_FEA, FEA_SD, &f2h, &f2p);

/* if output not stdout, make sure it's different from two input files */

    f3str = argv[argc - 1];

    if (debug_level > 0)
	(void) Fprintf (stderr,
            "%s: file1 = %s, file2 = %s, file3 = %s.\n", argv[0], f1str,
             f2str, f3str);

    if (strcmp(f3str, "-") != 0) {
	if(strcmp(f3str, f2str) == 0){
	    (void) Fprintf(stderr, 
		"Output file %s cannot equal input file 2\n", f3str);
	    exit(1);
	}
	if(strcmp(f3str, f1str) == 0){
	    (void) Fprintf(stderr, 
		"Output file %s cannot equal input file 1\n", f3str);
	    exit(1);
        }
    }

/* open output file*/

    f3str = eopen(argv[0], argv[argc - 1], "w", NONE, NONE, &f3h, &f3p);

/*
 * check for multichannel files
*/    
    if((num_channels = 
        get_fea_siz("samples", f1h,(short *) NULL, (long **) NULL)) != 1){
         Fprintf(stderr, 
   "%s: Multichannel data in file1 (%s); not supported yet\n", argv[0], f1str);
           exit(1);
	 }
    if((num_channels = 
        get_fea_siz("samples", f2h,(short *) NULL, (long **) NULL)) != 1){
           Fprintf(stderr, 
   "%s: Multichannel data in file2 (%s); not supported yet\n", argv[0], f2str);
           exit(1);
	 }
 
/* compare sampling frequencies of the two input files, exit if they differ */

 	if ((sf = get_genhd_val("record_freq", f1h, -1)) != 
	          get_genhd_val("record_freq", f2h, 0)) {
	    (void) Fprintf(stderr,
       "%s: both files must have same record_freq/sampling_freq.\n", argv[0]);
            (void) fclose(f3p);
	    exit(1);
 	}

/*
 * check data types in input files
*/    
    in_type1 = get_fea_type("samples", f1h);
    switch (in_type1)
    {
    case DOUBLE:
    case FLOAT:
    case LONG:
    case SHORT:
    case BYTE:
        complex1 = NO;
        break;
    case DOUBLE_CPLX:
    case FLOAT_CPLX:
    case LONG_CPLX:
    case SHORT_CPLX:
    case BYTE_CPLX:
        complex1 = YES;
        break;
    default:
        Fprintf(stderr, "%s: bad type code (%d) in input file 1 header.\n",
                 argv[0], in_type1);
        exit(1);
        break;
    }

    in_type2 = get_fea_type("samples", f2h);
    switch (in_type2)
    {
    case DOUBLE:
    case FLOAT:
    case LONG:
    case SHORT:
    case BYTE:
        complex2 = NO;
        break;
    case DOUBLE_CPLX:
    case FLOAT_CPLX:
    case LONG_CPLX:
    case SHORT_CPLX:
    case BYTE_CPLX:
        complex2 = YES;
        break;
    default:
        Fprintf(stderr, "%s: bad type code (%d) in input file 2 header.\n",
                 argv[0], in_type2);
        exit(1);
        break;
    }

/* process range*/

	s_rec = 1;
       	e_rec =  ((f1h->common.ndrec > -1) ? f1h->common.ndrec : LONG_MAX);

/*
 * Read ESPS Common, if appropriate
*/
	if(f1p != stdin){
	    (void) read_params((char *)NULL, SC_CHECK_FILE, f1str);
	    if(symtype("start") != ST_UNDEF) 
		s_rec = getsym_i("start");
	    if(symtype("nan") != ST_UNDEF) {
		if (getsym_i("nan") != 0)
		    e_rec = s_rec + getsym_i("nan") - 1; 
	    }
	}

	symerr_exit();

	if (range != NULL) (void) lrange_switch (range, &s_rec, &e_rec, 1);
	n_rec = e_rec - s_rec +1;

	if (debug_level > 1)
	    (void) Fprintf (stderr, 
             "%s: %s range: s_rec = %ld, e_rec = %ld,  n_rec = %ld.\n",
	     argv[0], f1str, s_rec, e_rec, n_rec);

	if ( s_rec > e_rec ) {
	    (void) Fprintf (stderr, 
	     "%s: start record greater than end record.\n", argv[0]);
	    exit(1);
	}

/* put all pertinent information from file1 to file3 */

	f3h = new_header(FT_FEA);
	(void) strcpy (f3h->common.prog, argv[0]);
	(void) strcpy (f3h->common.vers, VERSION);
	(void) strcpy (f3h->common.progdate, DATE);
	f3h->common.tag = NO;

	add_source_file (f3h, f1str, f1h);
	add_source_file (f3h, f2str, f2h);
	add_comment (f3h, get_cmd_line(argc,argv));
        if(gflag)
	  *add_genhd_d("scale", (double *) NULL, 1, f3h) = scale;
        if(strcmp("addsd", argv[0]) == 0)
	  (void) sprintf (combuf, 
	    "  Added samples from %s to samples %ld - %ld of %s.\n",
			 f2str,  s_rec, e_rec, f1str);
        else
	  (void) sprintf (combuf, 
	    "  Multiplied samples from %s with samples %ld - %ld of %s.\n",
			 f2str,  s_rec, e_rec, f1str);

	(void) add_comment (f3h, combuf);

    /* allocate input and initialize output data records */
    if(complex1 || complex2){
      complex = YES;
      sd_rec1 = allo_feasd_recs(f1h, DOUBLE_CPLX, (long)SIZE, (char *) NULL, NO);
      cdata1 = (double_cplx *)sd_rec1->data;
      sd_rec2 = allo_feasd_recs(f2h, DOUBLE_CPLX, (long)SIZE, (char *) NULL, NO);
      cdata2 = (double_cplx *)sd_rec2->data;
    }
    else{
      sd_rec1 = allo_feasd_recs(f1h, DOUBLE, (long)SIZE, (char *) NULL, NO);
      data1 = (double *)sd_rec1->data;
      sd_rec2 = allo_feasd_recs(f2h, DOUBLE, (long)SIZE, (char *) NULL, NO);
      data2 = (double *)sd_rec2->data;
    }

      start_time = (double)(s_rec-1)/sf;
      out_data_type = (int)cover_type(in_type1, in_type2);
      REQUIRE(
	init_feasd_hd(f3h, out_data_type, (int)1, &start_time, NO, sf) == 0,
	argv[0],"Couldn't initialize output FEA_SD header")
      if(complex){
         sd_rec3 = allo_feasd_recs(f3h, DOUBLE_CPLX, (long)SIZE, (char *)NULL, NO);
	 cdata3 = (double_cplx *)sd_rec3->data;
       }
      else{
	sd_rec3 = allo_feasd_recs(f3h, DOUBLE, (long)SIZE, (char *)NULL, NO);
	data3 = (double *)sd_rec3->data;
      }
      REQUIRE(sd_rec3 != NULL, argv[0], "Couldn't allocate FEA_SD data record")

      (void) write_header (f3h, f3p);

    if (debug_level > 0) 
        (void) Fprintf (stderr,
        "%s: record_freq/sampling_ freq is %lf.\n", argv[0], sf);

    (void) fea_skiprec (f1p, (long)s_rec - 1, f1h);

    if (debug_level > 0)
      (void) Fprintf (stderr, "%s: skipped %d records.\n",argv[0], s_rec - 1);
     

/* 
 * now begin adding or multiplying data from file1 and (scaled) file2 to file3 
*/

while (more){
    /*
     * first read data from f2
    */
    npoints2 = get_feasd_recs(sd_rec2, 0L, (long)SIZE, f2h, f2p);

    if(debug_level > 1)
	Fprintf(stderr, "npoints2 = %d\n", npoints2);
    /*
     * Check for EOF on f2 (means 0 points read)
    */

    if(npoints2 == 0 && tflag) {/* if we used all the data and we 
				    are supposed to truncate, then
				    we are done*/
	if(total_points == 0)
	    (void)Fprintf(stderr, "%s: %s is empty\n", argv[0], f2str);
	else
	    (void)Fprintf(stderr, "%s: WARNING - truncating data in %s\n", 
			  argv[0], f1str);
	break;
    }
    else if(npoints2 == 0){/* rewind file and get more data */
	if(total_points == 0){
	    (void)Fprintf(stderr, "%s: %s is empty\n", argv[0], f2str);
	    exit(1);
	}
	rewound = 1;
	rewind(f2p);
	f2h = read_header(f2p);
	npoints2 = get_feasd_recs(sd_rec2, 0L, (long)SIZE, f2h, f2p);
	if(debug_level > 1)
	    Fprintf(stderr, "npoints2 = %d\n", npoints2);
    }
    /*
     * Now get the data from f1
     */
	npoints1 = get_feasd_recs(sd_rec1, 0L, (long)npoints2, f1h, f1p);

	if(debug_level > 1)
	    Fprintf(stderr, "npoints1 = %d\n", npoints1);

    /*
     * Check on f1 range
    */
	if ((npoints1 > 0) && rewound) {
	    if(!zflag) Fprintf(stderr, 
	      "%s: %s does not contain enough records; reusing data\n", 
		 argv[0], f2str);
	    rewound = 0;
	}

        if(npoints1 == 0){
	    if(total_points == 0)
		(void)Fprintf(stderr, "%s is empty\n", f1str);
	    break; /*all done processing f1*/
	}
	total_points += npoints1;

	if(total_points >= n_rec){ /* reached end of specified range*/
	    npoints1 -= (total_points - n_rec);/*correct number of points*/
	    total_points = n_rec; /* reset total points*/
	    more = 0; /*set finished flag*/
	}

	if(debug_level > 1)
	    Fprintf(stderr, "total_points = %ld\n", total_points);
    /* 
     * scale f2, if needed
    */
	if(gflag){
	  if(complex)
	    for(i=0;i<npoints1;i++)
	      cdbuf[i] = realmult(cdata2[i], (double)scale);
	  else
	    for(i=0;i<npoints1;i++){
		dbuf[i] = data2[i]*scale;
	    }
	}
    /*
     * add or multiply f1 and f2 and write output file
     */
        if(complex && (strcmp("addsd", argv[0]) == 0)){
	  if(gflag)
	    for(i=0;i<npoints1;i++)
	      cdata3[i] = cadd(cdata1[i],cdbuf[i]);
	  else
	    for(i=0;i<npoints1;i++)
	      cdata3[i] = cadd(cdata1[i],cdata2[i]);
	}
	else if (complex && (strcmp("multsd", argv[0]) == 0)){
	  if(gflag)
	    for(i=0;i<npoints1;i++)
	      cdata3[i] = cmult(cdata1[i],cdbuf[i]);
	  else
	    for(i=0;i<npoints1;i++)
	      cdata3[i] = cmult(cdata1[i],cdata2[i]);
	}
        else if (!complex && (strcmp("addsd", argv[0]) == 0))
	  {
	  if(gflag)
	    for(i=0;i<npoints1;i++){
	      data3[i] = data1[i]+dbuf[i];
	    }
	  else
	    for(i=0;i<npoints1;i++)
	      data3[i] = data1[i]+data2[i];
	}
       else if (!complex && (strcmp("multsd", argv[0]) == 0)){
	  if(gflag)
	    for(i=0;i<npoints1;i++)
	      data3[i] = data1[i]*dbuf[i];
	  else
	    for(i=0;i<npoints1;i++)
	      data3[i] = data1[i]*data2[i];	 
       }
       else{
	 Fprintf(stderr, 
		 "Neither addsd nor multsd used to run program - exiting\n");
	 exit(1);
       }

	put_feasd_recs(sd_rec3, 0l, (long)npoints1, f3h, f3p);
    
	}


    /*
     * Write Common, if appropriate
    */
	if(strcmp(f3str, "<stdout>") != 0){
	    if((putsym_s("filename", f3str)) == -1){
		Fprintf(stderr, "Filename could not be written to Common");
		exit(1);	    
	    	    }

	    if((putsym_i("start", (int) 1 )) == -1){
		Fprintf(stderr, "Start could not be written to Common");
		exit(1);
	    }

	    if((putsym_i("nan", (int)total_points)) == -1){
		Fprintf(stderr,  "Nan could not be written to Common");
		exit(1);
	    }
	}


    (void) fclose (f1p);
    (void) fclose (f2p);
    (void) fclose (f3p);
    return(0);
}

