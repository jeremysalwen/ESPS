/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1996 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:  David Burton
 *
 * Brief description: FIR, IIR filtering
 *
 */

static char *sccs_id = "@(#)filter2.c	1.12	5/1/98	ERL";


#include <stdio.h>
#include <signal.h>
#include <esps/unix.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/feafilt.h>

#define SYNTAX USAGE("filter2 [-P param_file] [-p range] [-r range] [-d data_type] [ -f filter_string] [ -F filt_file] [-z] [-x debug_level] in_file out_file")

#define ERROR_EXIT(text) {(void) Fprintf(stderr, "%s: %s - exiting\n", \
                                         argv[0], text); exit(1);}
#define BUF_LEN 1024
#define MAX_NO_CHANNELS 512
#define NAME_SIZE 20

/*
 * Global declaration
 */

int debug_level = 0;


/*
 * ESPS Functions
 */

char *eopen();
long *grange_switch();
short get_fea_type();
int lin_search();
void fea_skiprec();
int is_field_complex();
char *get_cmd_line();
char *arr_alloc();
void lrange_switch();
void trange_switch();
int getopt();


/*
 * Main program
 */

int
main(argc, argv)
     int argc;
     char **argv;
{
  char *Version = "1.12";
  char *Date = "5/1/98";
  extern char *optarg;
  extern int optind;
  void get_range();
  char *param_file = NULL;       /* default parameter file name */
  FILE *isdfile = stdin,             /* input and output file streams */
       *osdfile = stdout,
       *filfile;                     /* filter file stream */
  char *iname = NULL,                /* input and output file names */
       *oname = NULL,
       *filter_string = "filter",      /* filtername */
       *filter_file_name = NULL,             /* filt_file name */
       *fname = NULL;
  struct header *ihd,                /* input and output file headers */
                *ohd,
                *fhd = NULL;         /* filter file header */
  struct feafilt *filt_rec;           /* filter record */
  struct fdata **frec = NULL;          /* general filter strucure */
  struct sdata *in = NULL, *out = NULL;/* general sd data structure */

  double samp_rate;
  short in_dtype,                     /* input/output data type */
        out_dtype;
  double *ostart_time,            /* input/output start time, delay */
         *istart_time,
         delay = 0;
  char nsiz_name[NAME_SIZE],           /* pole,zero,num,den name for -P */
       den_name[NAME_SIZE],
       dsiz_name[NAME_SIZE],
       num_name[NAME_SIZE],
       psiz_name[NAME_SIZE],
       zsiz_name[NAME_SIZE],
       pole_name[NAME_SIZE],
       zero_name[NAME_SIZE],
       gain_name[NAME_SIZE];
  long nsiz, dsiz, psiz, zsiz;
  double gain = 1;
  double *den_coeff = NULL, *num_coeff = NULL;
  double *parry = NULL, *zarry = NULL;
  double_cplx *poles = NULL, *zeros = NULL;
  long *channels = NULL;
  long num_channels = 1;
  char *range = NULL;
  long s_rec, e_rec;                     /* starting/ending record */
  long samps_left=0, nsamp, actsize, tot_samp_read = 0;
  int rflag = 0;
  int fflag = 0;
  int Fflag = 0;                    
  int zflag = 0;
  int cflag = 0;
  int dflag = 0;
  int common_file = 0;
  int ch, size, i, j;
  int type;
  
  /* channel selection is not implemented */
  while((ch = getopt(argc, argv, "P:p:r:e:d:f:F:x:z")) != EOF)
    switch (ch) {
    case 'P':
      param_file = optarg;
      break;
    case 'p':
    case 'r':
      rflag++;
      range = optarg;
      break;
    case 'c':
      channels = grange_switch(optarg, &num_channels);
      cflag++;
      break;
    case 'd':
      out_dtype = lin_search(type_codes, optarg);
      dflag++;
      break;
    case 'f':
      filter_string = optarg;
      fflag++;
      break;
    case 'F':
      filter_file_name = optarg;
      Fflag++;
      break;
    case 'x':
      debug_level = atoi(optarg);
      break;
    case 'z':
      zflag++;
      break;
    default:
      SYNTAX;
    }

/*
 * Determine and open output file
 */

  if (argc - optind < 1) {
    Fprintf(stderr, "%s: no output file specified.\n",argv[0]);
    SYNTAX;
  }
  oname = eopen(argv[0], argv[argc - 1], "w", NONE, NONE, &ohd, &osdfile);

  if (debug_level)
    Fprintf(stderr, "%s: output file is %s\n", argv[0], oname);

/*
 * Determine input file, either from command line or common file
 */

    if (argc - optind < 2) {
      common_file = 1;
      if (debug_level)
        Fprintf(stderr, "%s: no input file on command line\n",argv[0]);
    }
    else {
      iname = eopen(argv[0],argv[argc-2],"r", FT_FEA, FEA_SD, &ihd, &isdfile);
    }

  (void) read_params(param_file, SC_CHECK_FILE, iname);

  if ( common_file )
    {
      if (symtype("filename") == ST_UNDEF) {
            ERROR_EXIT("no input file name on command line or in common");
        }
        else {
          if (debug_level)
            Fprintf(stderr, "%s: input file name from common is %s\n",
                    argv[0], getsym_s("filename"));
          if (strcmp(oname, getsym_s("filename")) == 0)
            ERROR_EXIT("input name from common same as output file");
          iname = eopen(argv[0], getsym_s("filename"), "r",
                              FT_FEA, FEA_SD, &ihd, &isdfile);
        }
    }

/*
 * Determine parameters
 */

  get_range( &s_rec, &e_rec, range, rflag, 0, ihd );

  if( genhd_type("record_freq",&size,ihd) == HD_UNDEF)
    ERROR_EXIT("record_freq header item is missing from input file");
  samp_rate = *get_genhd_d("record_freq", ihd);

  num_channels = get_fea_siz("samples", ihd, (short *)NULL,(long **)NULL); 
  if( num_channels > MAX_NO_CHANNELS )
    ERROR_EXIT(" Input file exceeds 512 maximum allowable channels");

  if(debug_level) Fprintf(stderr,"main: no. of channels is %ld\n",num_channels);

/*
 * Determine filter data, either from the parameter file or filter file
 */

  if (Fflag || symtype("filter_file_name") != ST_UNDEF) {
    if (!Fflag)
      filter_file_name = getsym_s("filter_file_name");
    if (!fflag)
      filter_string = "1";
    fname = eopen(argv[0], filter_file_name, "r", 
		  FT_FEA, FEA_FILT, &fhd, &filfile);
    filt_rec = allo_feafilt_rec(fhd);
    i = atoi(filter_string);
    if ((fhd->common.ndrec != -1 && i > fhd->common.ndrec) || (i <= 0))
      ERROR_EXIT("specified a non-existing filter record, check -f option");
    fea_skiprec(filfile, (long) (i-1), fhd);
    if (get_feafilt_rec(filt_rec, fhd, filfile) == EOF)
      ERROR_EXIT("can't read specified filter record, check -f option");
  }
  else{
    Sprintf(nsiz_name, "%s_nsiz", filter_string);
    Sprintf(dsiz_name, "%s_dsiz", filter_string);
    Sprintf(num_name, "%s_num", filter_string);
    Sprintf(den_name, "%s_den", filter_string);
    Sprintf(psiz_name, "%s_psiz", filter_string);
    Sprintf(zsiz_name, "%s_zsiz", filter_string);
    Sprintf(pole_name, "%s_poles", filter_string);
    Sprintf(zero_name, "%s_zeros", filter_string);
    Sprintf(gain_name, "%s_gain", filter_string);
    if( symtype(psiz_name) != ST_UNDEF) psiz = getsym_i(psiz_name);
    if( symtype(zsiz_name) != ST_UNDEF) zsiz = getsym_i(zsiz_name);
    if( symtype(gain_name) != ST_UNDEF) gain = getsym_d(gain_name);
    if( symtype(nsiz_name) != ST_UNDEF ) nsiz = getsym_i(nsiz_name);
    if( symtype(dsiz_name) != ST_UNDEF) dsiz = getsym_i(dsiz_name);

    if( symtype(num_name ) != ST_UNDEF || symtype(nsiz_name) != ST_UNDEF){
      num_coeff = (double *) malloc(nsiz * sizeof(double));
      spsassert(num_coeff,"main: num_coeff maolloc failed");
      if( nsiz != getsym_da(num_name, num_coeff, nsiz)) 
	ERROR_EXIT("inconsistent numerator coefficient size");
    }
    if( symtype(den_name ) != ST_UNDEF && symtype(dsiz_name) != ST_UNDEF){
      den_coeff = (double *) malloc(dsiz *sizeof(double));
      spsassert(den_coeff,"main: den_coeff malloc failed");
      if( dsiz != getsym_da(den_name, den_coeff, dsiz)) 
	ERROR_EXIT("inconsistent denomenator coefficient size");
    }
    if( symtype(pole_name) != ST_UNDEF && symtype(psiz_name) != ST_UNDEF){
      parry = (double *) malloc(2*psiz*sizeof(double));
      spsassert(parry, "main: parry malloc failed");
      if( psiz*2 != getsym_da(pole_name, parry, 2*psiz)) 
	ERROR_EXIT("inconsistent poles size");
    }
    if( symtype(zero_name) != ST_UNDEF && symtype(zsiz_name) != ST_UNDEF){
      zarry = (double *) malloc(2*zsiz*sizeof(double));
      spsassert(zarry,"main: zarry maolloc failed");
      if( zsiz*2 != getsym_da(zero_name, zarry, 2*zsiz)) 
	ERROR_EXIT("inconsistent zeros size");
    }
    if(!( (zarry && parry ) || ( !zarry && !parry && den_coeff && num_coeff)))
      ERROR_EXIT("Full zeros/poles or full denominator/numerator coeff should be specified in the parameter file, or check the -f argument.");

    /* IIR gain is the same as 1st element in numerator coefficient array */
    if( zarry && parry ) {
      if(num_coeff == NULL){
	nsiz = 1;
	num_coeff = (double *) malloc(nsiz * sizeof(double));
	spsassert(num_coeff,"main: num_coeff maolloc for gain failed");
	num_coeff[0] = gain;      /* either default 1 or gain_name */
	if(debug_level) Fprintf(stderr,"%s: IIR overall gain, %e\n", argv[0],
				num_coeff[0]);
      }
      else{
	if(num_coeff[0] != gain)
	  if(symtype(gain_name) != ST_UNDEF){ /* gain is defined */
	    num_coeff[0] = gain;
	    if(debug_level)
	      Fprintf(stderr,"%s: use %e for the true overall gain\n",
		      argv[0], num_coeff[0]);
	  }
	  else            /* gain is not defined, but num_coeff exists */
	    if(debug_level) 
	      Fprintf(stderr,"%s: IIR overall gain, %e from numerator[0]\n", 
		      argv[0], num_coeff[0]);
      }
    }

    filt_rec = (struct feafilt *) malloc(sizeof(struct feafilt));

    if( zarry && parry ){

      filt_rec->zero_dim = &zsiz;
      filt_rec->pole_dim = &psiz;
      zeros = (double_cplx *) malloc( zsiz * sizeof(double_cplx));
      spsassert( zeros,"main: zeros malloc failed");
      for( i = 0,j=0; i<zsiz; j+=2, i++){
	zeros[i].real = zarry[j];
	zeros[i].imag = zarry[j+1];
      }
      free(zarry);
      poles = (double_cplx *) malloc( psiz * sizeof(double_cplx));
      spsassert( poles,"main: poles malloc failed");
      for( i = 0,j=0; i<psiz;j+=2, i++){
	poles[i].real = parry[j];
	poles[i].imag = parry[j+1];
      }
      free(parry);
      filt_rec->zeros = zeros;
      filt_rec->poles = poles;

      filt_rec->num_size = &nsiz;        /* num_coeff[0] has the IIRgain info*/
      filt_rec->re_num_coeff = num_coeff;
      filt_rec->denom_size = &dsiz;      /* usually NULL */
      filt_rec->re_denom_coeff = den_coeff;

      if(debug_level) Fprintf(stderr,"main: zeros/poles from parameter file\n");
    }
    else{
      filt_rec->zero_dim = NULL;
      filt_rec->pole_dim = NULL;
      filt_rec->poles = NULL;	/* tested in init_fdata */
      filt_rec->zeros = NULL;

      filt_rec->num_size = &nsiz;
      filt_rec->denom_size = &dsiz;
      filt_rec->re_num_coeff = num_coeff;
      filt_rec->re_denom_coeff = den_coeff;
      if(debug_level)Fprintf(stderr,"%s: num/den coeff from parameter file\n",
			     argv[0]);
    }
    /* init_fdata requires fhd, although it is not used now */
    /* the header usually comes from feafilt file, here since there no
       file, so create fake header.  Without this, passing a NULL
       header to init_fdata, will get runtime error */
    fhd = new_header(FT_FEA);
  }

/*
 * If the FIR filter size is too big, initializing the filter state
 * exceeds array bounds on the input data. We warn and exit below. 
 * We somewhat arbitrarily select 513 as the cut off.
 *
 * Note that no iir filter would have a numerator size greater than
 * 20, so we don't bother verifying that it is an FIR filter.
*/ 

if(*filt_rec->num_size > 513) {
    Fprintf(stderr, "FIR filter size too big for filter(1-ESPS).\n");
    Fprintf(stderr, "\tPlease use fft_filter(1-ESPS).\n");
    exit(1);
}


/*
 * Determine filter type and initialize fdata structure
 */
  
  if( filt_rec->poles || filt_rec->re_denom_coeff ) type = IIR_FILTERING;
  else type = FIR_FILTERING;
  if( debug_level) Fprintf(stderr,"Filter type: %d\n", type);
  frec = (struct fdata **) malloc ( num_channels * sizeof(struct fdata *));
  spsassert(frec,"frec malloc failed");

  if(is_field_complex(ihd, "samples") == NO){
    for(i = 0; i<num_channels; i++)
      frec[i] = (struct fdata *) init_fdata( type, filt_rec, fhd, 0, 0);
  }
  else{
    if(debug_level) Fprintf(stderr,"main: input is complex\n");
    for(i = 0; i<num_channels; i++)
      frec[i] = (struct fdata *) init_fdata( type, filt_rec, fhd, 1, 0);
  }    

/*
 * Determine input signal data and initialize sdata structure
 */

  if((in_dtype = get_fea_type( "samples", ihd)) == UNDEF)
    ERROR_EXIT(" Input file has no field named 'samples'");

  if(!dflag) out_dtype = in_dtype;

  in = (struct sdata *) malloc(sizeof(struct sdata));
  out = (struct sdata *) malloc(sizeof(struct sdata));

/*
 * Prepare output header
 */

  ohd = new_header(FT_FEA);
  (void) strcpy (ohd->common.prog, argv[0]);
  (void) strcpy (ohd->common.vers, Version);
  (void) strcpy (ohd->common.progdate, Date);
  ohd->common.tag = NO;
  add_source_file(ohd,iname,ihd);
  add_comment(ohd,get_cmd_line(argc,argv));

  if(zflag) delay = get_genhd_val("delay_samples", fhd, (double) 0)/samp_rate;


  /* We allow one start time value for each input channel, but do not
     insist on it. So a multichannel file can have a start time
     generic with only one value.

     If there is no input start_time generic, we define the start time to be 0
     and compute an output start time starting sample and filter delay.

     If there is an input start_time generic, we compute an output
     start time value for every value in the generic. Note that the number of
     values in the input start time generic is returned as a side effect
     of determining if the generic is defined in the input file.
  */
     
  if( (type = genhd_type("start_time",&size,ihd)) == HD_UNDEF){

      Fprintf(stderr,"%s: start_time in input file is undefIned.\n", argv[0]);
      Fprintf(stderr,"%s: start_time is set to zero.\n", argv[0]);
      ostart_time = (double *) malloc(sizeof(double));
      spsassert(ostart_time,"malloc failed in main");
      *ostart_time = (s_rec-1)/samp_rate - delay;
    }
  else {

      istart_time = (double *) calloc ((unsigned) size, sizeof(double));
      spsassert(istart_time,"main: istart_time calloc failed");
      istart_time = get_genhd_d( "start_time", ihd);

      ostart_time = (double *) calloc((unsigned)size, sizeof(double));
      spsassert(ostart_time,"main: ostart_time calloc failed");
      for( i = 0; i < size; i++) {
	ostart_time[i] = istart_time[i] + (s_rec-1)/samp_rate - delay; 
      }
    }

  (void) add_genhd_c("source_file", iname, 0, ohd);

/*
 * write header
 */

  /* If the start_time has multiple values, do the init differently.

     We assume that the init_feasd_hd function checks that size and 
     num_channels are equal, if size > 1.
  */
  

  if (size > 1) {
    /* start_time has multiple values */
    init_feasd_hd(ohd, out_dtype ,num_channels,ostart_time ,YES, samp_rate);
  } else {
    init_feasd_hd(ohd, out_dtype ,num_channels,ostart_time ,NO, samp_rate);
  }

  (void) write_header ( ohd, osdfile );

/*
 * Allocate data, data are eiterh double or double_cplx,
 *   although sdata structure allow any data of any type to be passes in
 */

  if(is_field_complex(ihd, "samples") == NO){
    if( num_channels == 1){
      in->rec = allo_feasd_recs( ihd, DOUBLE, BUF_LEN, (char *) NULL, NO);
      out->rec = allo_feasd_recs( ohd, DOUBLE, BUF_LEN, (char *) NULL, NO);
    }
    else{
      in->rec = allo_feasd_recs( ihd, DOUBLE, BUF_LEN, (char *) NULL, YES);
      out->rec = allo_feasd_recs( ohd, DOUBLE, BUF_LEN, (char *) NULL, YES);
    }
  }
  if(is_field_complex(ihd, "samples") == YES){
    if( num_channels == 1){
      in->rec = allo_feasd_recs( ihd, DOUBLE_CPLX, BUF_LEN, (char *) NULL, NO);
      out->rec = allo_feasd_recs( ohd, DOUBLE_CPLX, BUF_LEN, (char *) NULL, NO);
    }
    else{
      in->rec = allo_feasd_recs( ihd, DOUBLE_CPLX,BUF_LEN, (char *) NULL, YES);
      out->rec = allo_feasd_recs( ohd, DOUBLE_CPLX,BUF_LEN, (char *) NULL, YES);
    }
  }

  in->data = NULL;
  out->data = NULL;   /* must set explictly to NULL, block_filte2 checks it */
  
  samps_left = e_rec - s_rec + 1;
  (void) fea_skiprec ( isdfile , s_rec - 1, ihd);
  nsamp = (BUF_LEN > samps_left) ? samps_left : BUF_LEN;

/*
 * Main loop
 */

  while( 0 != (actsize =get_feasd_recs( in->rec, (long) 0L, nsamp, 
				       ihd, isdfile))){
    if( 0 != block_filter2(actsize, in, out, frec))
      ERROR_EXIT("error in block_filter2");
    put_feasd_recs( out->rec, 0L, actsize, ohd, osdfile );

    tot_samp_read += actsize;
    samps_left -=  actsize;
    nsamp = (BUF_LEN > samps_left) ? samps_left : BUF_LEN;
  }

  (void) fclose (isdfile);
  (void) fclose (osdfile);

/*
 * put output file info in ESPS common
 */
   if (strcmp(oname, "<stdout>") != 0) {
       (void)putsym_s("filename", oname);
       (void)putsym_s("prog",argv[0]);
       (void)putsym_i("start",1 );
       (void)putsym_i("nan",(int) tot_samp_read);
   }
   if (debug_level) Fprintf(stderr, "%s: normal exit\n", argv[0]);
   exit(0);
  
}

void get_range(srec, erec, rng, pflag, Sflag, hd)
/*
 * This function facilitates ESPS range processing.  It sets srec and erec
 * to their parameter/common values unless a range option has been used, in
 * which case it uses the range specification to set srec and erec.  If
 * there is no range option and if start and nan do not appear in the
 * parameter/common file, then srec and erec are set to 1 and LONG_MAX.
 * Get_range assumes that read_params has been called; If a command-line
 * range option (e.g., -r range) has been used, get_range should be called
 * with positive pflag and with rng equal to the range specification.
 */
long *srec;                     /* starting record */
long *erec;                     /* end record */
char *rng;                      /* range string from range option */
int pflag;                      /* flag for whether -r or -p used */
int Sflag;                      /* flag for whether -S used */
struct header *hd;              /* input file header */
{
    long common_nan;

    *srec = 1;
    *erec = LONG_MAX;
    if (pflag)
        lrange_switch (rng, srec, erec, 1);
    else if (Sflag)
        trange_switch (rng, hd, srec, erec);
    else {
        if(symtype("start") != ST_UNDEF) {
            *srec = getsym_i("start");
        }
        if(symtype("nan") != ST_UNDEF) {
            common_nan = getsym_i("nan");
            if (common_nan != 0)
                *erec = *srec + common_nan - 1;
        }
    }
}

