/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Burton
 * Modified to FEAFILT and double precision by Bill Bryne
 * Corrected numerical problems, corrected numbers of output zerso, and
 *      added Chebyshev2, elliptical filters by Derek Lin, 12/3/92
 * Checked by:
 * Revised by:
 *
 * Brief description:This program designs a recursive filter.
 *                   Butterworth, Chebyshev1, Chevyshev2, and Elliptical
 *                   filters are supported at this time.
 */

static char *sccs_id = "@(#)iir_filt.c	1.18 01 Oct 1998 ERL";

# include <stdio.h>
# include <math.h>
# include <esps/esps.h>
# include <esps/feafilt.h>
# include <esps/unix.h>
# include <esps/constants.h>

# define SYNTAX	USAGE ("iir_filt [-P param_file][-x debug_level] filt_file");
#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
					 argv[0], text); exit(1);}
# define SQRD(x) ((x)*(x))
# define ARCSINH(x) log((x)+sqrt(SQRD(x)+1))
# define ARCCOSH(x) log((x)+sqrt(SQRD(x)-1))
# define COSH(x) (0.5*(exp(x)+exp(-x)))
char *get_cmd_line();
double_cplx cdiv(), cmult(), cadd(), csub(), realmult(), csqrt();
int pz_to_co_d();

/*
 * Globals
*/
int debug_level = 0;

main (argc, argv)
     int argc;
     char *argv[];
{
  extern char *optarg;
  extern optind;
  char *Version = "2.0";
  char *Date = "12/03/92";
  FILE *fpout;		    /*output stream pointer*/
  struct header *oh;		    /*output file header pointer*/
  struct feafilt *frec;	    /*output data rescord structure*/
  char *param_file = NULL;        /*parameter file name*/
  char *filt_file = NULL;	    /*output file name*/
  filtdesparams *fdp;

  short order;		    /*holds final filter order*/
  int nroots_num, nroots_den;	    /*# roots in final filter*/

  double *num_coeff, *den_coeff;   /*holds designed coefficients*/ 
  double_cplx *poles, *zeros;
  double gain = 1.0;		    /*scaling gain*/
  float sf = 8000;		    /*sampling frequency*/
  char *filt_mthd = NULL;	    /*filter polynomial type*/
  int filter_poly = 0;	    /* integer representation of type*/
  char *filt_resp_type =  NULL;   /*filter response type*/
  int filter_response = 0;	     /* integer representation of type*/
  char *tmp = NULL;		    /* temporary character ptr*/

  int i, c;
  int filter_order = 0;	    /* filter polynomial order */
  float band_edges[4];	    /* holds band edges for ELLIPTICAL*/
  float warped_edges[4];	    /* holds prewarped edges for ELLIPTICAL*/
  int num_of_band_edges;	    /*number of band edge freqs*/
  float pass_loss;		    /*max. pass band loss*/
  float stop_loss;		    /*min. stop band loss*/
  short num_num_co, num_den_co;   /* number of filter coeffs*/
  double gain_factor;		    /*values used to scale numerator coeffs*/
  float val;
  
  float prewarp();
  void low_pass_proto();
  void poles_and_zeros();
  int elliptical_p_and_z();
  void freq_xfrm();
  void blt();
  void pz_to_numden();
  void set_gain();
  int determ_order();

/*
 * Check the command line options. 
 */
  while ((c = getopt (argc, argv, "P:x:")) != EOF)
    {
      switch (c)
	{
	case 'P':
	  param_file = optarg;
	  break;
	case 'x':
	  debug_level = atoi(optarg);
	  break;
	default:
	  SYNTAX
	  }
    }
  
  
  /* allocate space for filt_resp_type */
  filt_resp_type = (char *) calloc((unsigned)8, sizeof(char));

  /* initialize the band edge info arrays */
  for (i=0; i<4; i++) {
      band_edges[i] = warped_edges[i] = 0;
  }
  
/*
 * Get the output filter filename 
 */
  if (optind < argc)
    {
      filt_file = argv [optind];
      if (strcmp (filt_file, "-") == 0) 
	{
	  filt_file = "<stdout>";
	  fpout = stdout;
	}
      else TRYOPEN("iir_filt", filt_file, "w", fpout);
      if (debug_level) 
	Fprintf (stderr,"%s: Output file is %s\n", argv[0],filt_file);
    }
  else
    {
      Fprintf (stderr,"%s: No output file specified.\n", argv[0]);
      SYNTAX
	exit (1);
    }

/* 
 *Read parameter file and get design parameters 
 */
  
  if (debug_level)
    Fprintf (stderr,"%s: Reading parameter file %s\n", argv[0],param_file);
  
  if (read_params (param_file, SC_NOCOMMON, (char *)NULL) != 0)
    ERROR_EXIT("read_params could not read the params file.");
  
  if (symtype("filt_method") != ST_UNDEF)
    filt_mthd = getsym_s ("filt_method");
  else ERROR_EXIT("Filt_method not specified in params file.");
  if((filter_poly = lin_search(filt_method, filt_mthd)) == -1){
    Fprintf(stderr, "%s: Invalid filt_method (%s) specified - exiting.\n",
	    argv[0], filt_mthd);
    exit(1);
  }
  
  if (symtype("filt_type") != ST_UNDEF){
    tmp = getsym_s ("filt_type");
    /* add FILT_ prefix*/
    (void)strcat(filt_resp_type, "FILT_");
    (void)strcat(filt_resp_type, tmp);
  }
  else ERROR_EXIT("Filt_type not specified in params file.");
  if((filter_response = lin_search(filt_type, filt_resp_type)) == -1){
    Fprintf(stderr,"%s: Invalid filt_type (%s) specified - exiting.\n", 
	    argv[0], tmp);
    exit(1);
  }
  
  if (symtype("samp_freq") != ST_UNDEF) sf = (float)getsym_d ("samp_freq");
  else ERROR_EXIT("Sampling frequency not specified in params file.");
  if(sf <= 0.) ERROR_EXIT("Sampling Frequency must be > 0");
  
/*  
 * Get filter design parameters - 
 * 
 */
  if(filter_response == FILT_LP){
    num_of_band_edges = 2;
    if(symtype("p_freq1") != ST_UNDEF) band_edges[0] = getsym_d("p_freq1");
    else ERROR_EXIT("p_freq1 not specified.");
    if(symtype("s_freq1") != ST_UNDEF) band_edges[1] = getsym_d("s_freq1");
    else ERROR_EXIT("s_freq1 not specified.");
  }
  if(filter_response == FILT_HP){
    num_of_band_edges = 2;
    if(symtype("s_freq1") != ST_UNDEF) band_edges[0] = getsym_d("s_freq1");
    else ERROR_EXIT("s_freq1 not specified.");
    if(symtype("p_freq1") != ST_UNDEF) band_edges[1] = getsym_d("p_freq1");
    else ERROR_EXIT("p_freq1 not specified.");
  }
  if(filter_response == FILT_BP){
    num_of_band_edges = 4;
    if(symtype("s_freq1") != ST_UNDEF) band_edges[0] = getsym_d("s_freq1");
    else ERROR_EXIT("s_freq1 not specified.");
    if(symtype("p_freq1") != ST_UNDEF) band_edges[1] = getsym_d("p_freq1");
    else ERROR_EXIT("p_freq1 not specified.");
    if(symtype("p_freq2") != ST_UNDEF) band_edges[2] = getsym_d("p_freq2");
    else ERROR_EXIT("p_freq2 not specified.");
    if(symtype("s_freq2") != ST_UNDEF) band_edges[3] = getsym_d("s_freq2");
    else ERROR_EXIT("s_freq2 not specified.");
  }
  if(filter_response == FILT_BS){
    num_of_band_edges = 4;
    if(symtype("p_freq1") != ST_UNDEF) band_edges[0] = getsym_d("p_freq1");
    else ERROR_EXIT("p_freq1 not specified.");
    if(symtype("s_freq1") != ST_UNDEF) band_edges[1] = getsym_d("s_freq1");
    else ERROR_EXIT("s_freq1 not specified.");
    if(symtype("s_freq2") != ST_UNDEF) band_edges[2] = getsym_d("s_freq2");
    else ERROR_EXIT("s_freq2 not specified.");
    if(symtype("p_freq2") != ST_UNDEF) band_edges[3] = getsym_d("p_freq2");
    else ERROR_EXIT("p_freq2 not specified.");
  }
  for(val=0, i=0; i< num_of_band_edges; i++){
    if(band_edges[i] <= 0)
      ERROR_EXIT("critical frequencies must be >0");
    if(val >= band_edges[i])
      ERROR_EXIT("inconsistent bandedges specified for filter response type selected");
    val = band_edges[i];
  }      

  if(symtype("pass_band_loss") != ST_UNDEF)
    pass_loss = (float)getsym_d("pass_band_loss");
  else ERROR_EXIT("Pass_band_loss not specified.");
  if( pass_loss <= 0.) ERROR_EXIT("Pass band loss must be > 0.");	
  if(symtype("stop_band_loss") != ST_UNDEF)
    stop_loss = (float)getsym_d("stop_band_loss");
  else ERROR_EXIT("Stop_band_loss not specified.");
  if( stop_loss <= 0.) ERROR_EXIT("Stop band loss must be > 0.");

  if (symtype("gain") != ST_UNDEF) gain = (float)getsym_d ("gain");
  else ERROR_EXIT("Pass band gain not specified in params file.");
  if(gain <= 0) ERROR_EXIT("Gain must be > 0");

  if(debug_level > 0){
    Fprintf(stderr, "\n");
    Fprintf(stderr, "\t%s: LISTING OF SPECIFIED PARAMETERS\n\n",argv[0]);
    Fprintf(stderr, "\n\n%s: Samp. freq. = %f, pass band gain = %f\n",
	    argv[0],sf, gain);
    Fprintf(stderr, "%s: filt_method = %s, filt_type = %s\n",argv[0],
	    filt_mthd, filt_resp_type);
    Fprintf(stderr,"%s: pass band loss = %f dB,\nband edges = %f, %f, %f and %f Hz\n",argv[0], pass_loss, band_edges[0], band_edges[1], 
	    band_edges[2], band_edges[3]);
    Fprintf(stderr,"%s: stop band loss = %f dB\n", argv[0],stop_loss);
  }

/*********** Start the filter design processs ***********/
    /*  The basic design process is the following, labelled by A,B...F*/
      /*************************************************************
	A. Prewarp the critical frequencies to the analog frequency
	    domain - this is done by prewarp().

	B. Find critical frequencies for analog low pass
	    prototype filter - this is done by low_pass_proto().
	 
        C. Determine order of the filter.
      **************************************************************/

/* 
 * A. prewarp frequencies 
 */

    for(i=0; i<num_of_band_edges; i++)
      warped_edges[i] = prewarp(2*PI*band_edges[i], sf);

  if (debug_level == 1 || debug_level > 9){
    Fprintf(stderr, "\n\t\tPREWARP FREQUENCIES\n");
    Fprintf(stderr,"%s: Convert from Digital to Analog Frequency Domain\n",
	    argv[0]);
    for(i=0;i<num_of_band_edges; i++)
      Fprintf(stderr,
	      "critical freq[%d] = %f Hz, warped freq[%d] = %f radians\n",
	      i, band_edges[i], i, warped_edges[i]);
  }

/* 
 * B. compute low pass prototype critical frequency 
 */

  low_pass_proto(warped_edges, filter_response, filter_poly);

  if(debug_level == 2 || debug_level >9){
    Fprintf(stderr, "\n\n%s: ANALOG LOWPASS PROTOTYPE FREQUENCIES\n", argv[0]);
      Fprintf(stderr,"pass freq = %g radian, stop freq = %g radian\n",
	      warped_edges[0], warped_edges[1]);
  }

/*
 * C. determine the order of the filter, possible only after A, B 
 */

  filter_order = determ_order(filter_poly, pass_loss, stop_loss, warped_edges);

  if(debug_level){
    Fprintf(stderr,"%s: Lowpass prototype filter order computed by program: %d\n", argv[0], filter_order);
    if( filter_response == FILT_HP || filter_response == FILT_LP )
      Fprintf(stderr,"%s: The final filter order is: %d\n", 
	      argv[0], filter_order);
    else
      Fprintf(stderr,"%s: The final filter order is: %d\n", 
	      argv[0], 2*filter_order);
  }

  if(symtype("filt_order") != ST_UNDEF){
    if( filter_response == FILT_HP || filter_response == FILT_LP )
      Fprintf(stderr,"%s: The optimal filter order by the program is: %d\n", 
	      argv[0], filter_order);
    else
      Fprintf(stderr,"%s: The optimal filter order by the program is: %d\n", 
	      argv[0], 2*filter_order);
    Fprintf(stderr,"%s: The parameter filt_order in the parameter file is present.\n\tThe final filter order is set to it\n",argv[0]);
    filter_order = getsym_i("filt_order");
    if(filter_order <= 0) 
      ERROR_EXIT("filter_order in parameter file must be > 0");
    Fprintf(stderr,"%s: Lowpass prototype filter order is set by filt_order: %d \n", argv[0], filter_order);
    if(filter_response == FILT_BP || filter_response == FILT_BS){
      if(filter_order != 2 * (filter_order/2)){
	Fprintf(stderr,"%s: The parameter filt_order must be an even number for bandpass or bandstop filters", argv[0]);
	ERROR_EXIT(" ");
      }
      else 
	filter_order = filter_order/2;
    }
  }

     /*************************************************************
	D. Find the poles and zeros for the prototype low pass
	    filter - this is done by poles_and_zeros() for 
	    BUTTERWORTH, CHEBYSHEV1, and CHEBYSHEV2 filters and by
	    elliptical_p_and_z() for ELLIPTICAL fitlers.

	E. Find the poles and zeros of the analog version of the
	    desired filter type (LP, HP, BP, or BS) - this is done
	    by freq_xfrm().

	F. Find the corresponding poles and zeros in digital frequency
	    domain -  this is done by blt().

	G. Find the numerator and demoninator coeffs corressponding to
	    the poles and zeros - this is done by pz_to_co_d().

	H. Modify the numerator coefficients so that the gain equal
	    to the value specified - this is done by set_gain().

	F. Write out the appropriate generics and the data record.

     ***************************************************************/


/* 
 * Allocate space for pole and zero coefficients - real and imaginary parts
 */

    poles = (double_cplx *)calloc((unsigned)2*(filter_order), 
				  sizeof(double_cplx));
    spsassert(poles != NULL,"Couldn't allocate space for poles\n");
    zeros = (double_cplx *)calloc((unsigned)2*(filter_order), 
				 sizeof(double_cplx));
    spsassert(zeros != NULL,"Couldn't allocate space for zeros\n");

/* 
 * find poles and zeros for prototype low pass filter 
 */
  switch(filter_poly){
  case BUTTERWORTH:
  case CHEBYSHEV1:
  case CHEBYSHEV2:
    poles_and_zeros(poles, zeros, warped_edges,
		    filter_order, pass_loss, stop_loss, filter_poly, 
		    &nroots_den, &nroots_num );
    break;
  case ELLIPTICAL:
    elliptical_p_and_z(0, poles, zeros, warped_edges, filter_order, 
		       pass_loss, stop_loss, &nroots_den, &nroots_num);
    break;
  default:
    ERROR_EXIT("Invalid filter polynomial type");
  }

  if(debug_level == 3 || debug_level >9){
    Fprintf(stderr, "\n\n%s: ANALOG LOW PASS PROTOTYPE POLES AND ZEROS\n\n",
	    argv[0]);
    Fprintf(stderr,"pole_size %d, zero_size %d\n", nroots_den, nroots_num);
    for(i=0; i<2*filter_order; i++)
	  Fprintf(stderr, 
     "Index %d\treal_p = %lf, imag_p = %lf\n\treal_z = %lf, imag_z = %lg\n\n",
	  i, poles[i].real, poles[i].imag, zeros[i].real, zeros[i].imag);
  }

/* 
 * transform low pass prototype to required response type 
 */
  /*first zeros */
  freq_xfrm(filter_order,filter_response, zeros, 
	    warped_edges[2], &nroots_num);
  /* now poles*/
  freq_xfrm(filter_order, filter_response, poles, 
	    warped_edges[2], &nroots_den);

  if(debug_level == 4 || debug_level > 9){
    Fprintf(stderr, "\n\n%s: Desired Response type = %s\n", 
	    argv[0],filt_resp_type);
    Fprintf(stderr, "Poles and Zeros for ANALOG Filter\n\n");
    Fprintf(stderr,"pole_size %d, zero_size %d\n", nroots_den, nroots_num);
    for(i=0; i<2*filter_order; i++)
      Fprintf(stderr,  "Index = %d, real_p = %f, imag_p = %f,\n\treal_z = %f, imag_z = %f\n", i, poles[i].real, poles[i].imag, zeros[i].real, zeros[i].imag);
  }

/*
 * Warp the frequencies back to the digital domain 
 * This is done by using the digital bilinear transform
 */

  /*first zeros*/
  blt(filter_response, sf, zeros, nroots_num);
  
  /*now poles*/
  blt(filter_response, sf, poles, nroots_den);
  
  if(debug_level == 5 || debug_level >9){
    Fprintf(stderr, "\n\nPoles and ZEROS for DIGITAL FILTER\n\n");
    Fprintf(stderr,"pole_size %d, zero_size %d\n", nroots_den, nroots_num);
    Fprintf(stderr, "\tFirst Poles:\n");
    for(i=0; i< 2*filter_order; i++)
      Fprintf(stderr,"Index = %d:\treal_pole = %lf, imag_pole = %lf\n",
	      i, poles[i].real, poles[i].imag);
    Fprintf(stderr, "\nNow Zeros:\n");
    for(i=0; i< 2*filter_order; i++)
      Fprintf(stderr,"Index = %d:\treal_zero = %lf, imag_zero = %lf\n",
	      i, zeros[i].real, zeros[i].imag);
  }

/* 
 *Allocate space for numerator and denominator coefficients
 */
    num_coeff = (double *)calloc((unsigned)2*filter_order+1, sizeof(double));
    spsassert(num_coeff != NULL, "Couldn't allocate space for numerator");
    den_coeff = (double *)calloc((unsigned)2*filter_order+1, sizeof(double));
    spsassert(den_coeff != NULL, "Couldn't allocate space for denominator");

/* 
 * Convert from pole and zeros to filter cooefficients 
 */

    (void)pz_to_numden(nroots_den, poles, &num_den_co, den_coeff);
    (void)pz_to_numden(nroots_num, zeros, &num_num_co, num_coeff);

/*
 * Create output header 
 */
    oh = new_header (FT_FEA);

    fdp = (filtdesparams *) calloc( 1, sizeof(filtdesparams));
    spsassert(fdp!=NULL, "can't allocate memory for header params");

    oh->common.tag = NO;
    fdp->define_pz = YES; 
    fdp->func_spec = (short)IIR;
    fdp->type = filter_response;
    fdp->method = filter_poly;

    (void)strcpy (oh->common.prog, "iir_filt");
    (void)strcpy (oh->common.vers, Version);
    (void)strcpy (oh->common.progdate, Date);
    (void)add_comment(oh, get_cmd_line(argc, argv));/*store command line*/

    init_feafilt_hd( oh, (long)(2*filter_order+1), (long)(2*filter_order+1),
		    fdp);

/*
 * Add generic header items: samp_freq, notch_freq, and band_width
 */
  (void)add_genhd_f("samp_freq", &sf, 1, oh);
  order = filter_order;
  if(filter_response == FILT_BS || filter_response == FILT_BP)
    order = 2*order;
  (void)add_genhd_s("filt_order", &order, 1, oh);
  (void)add_genhd_f("pass_band_loss", &pass_loss, 1, oh);
  (void)add_genhd_f("stop_band_loss", &stop_loss, 1, oh);
  (void)add_genhd_f("band_edges", band_edges, (int)4, oh);
 
  write_header (oh, fpout);  

  
/* 
 * Allocate space for filter data record 
 */
    frec = allo_feafilt_rec (oh);

/* 
 * Write the data to the output file. 
 */
    *frec->num_size = num_num_co;
    for (i=0; i<num_num_co; i++)
      frec->re_num_coeff[i] = num_coeff[i];

    *frec->denom_size = num_den_co;
    for (i=0; i<num_den_co; i++)
      frec->re_denom_coeff[i] = den_coeff[i];

    *frec->zero_dim = nroots_num;
    for (i=0; i<nroots_num; i++)
      frec->zeros[i] = zeros[i];

    *frec->pole_dim = nroots_den;
    for (i=0; i<nroots_den; i++)
      frec->poles[i] = poles[i];

/* 
 * set gain factor 
 */

  (void)set_gain(filter_response, frec, gain, &gain_factor,
		 band_edges, sf, filter_poly);

  if(debug_level == 6 || debug_level >9){
    Fprintf(stderr, "\n\n%s: Gain factor = %e\n\n", argv[0], gain_factor);
    Fprintf(stderr, "\t\tNumerator and Denominator Coefficients\n\n");
    Fprintf(stderr, "Numerator Coefficients\n");
    for(i=0; i<num_num_co; i++)
      Fprintf(stderr, "\tnum_coeff[%d] = %e\n", i, frec->re_num_coeff[i]);
    Fprintf(stderr, "\nDenominator Coefficients\n");
    for(i=0; i<num_den_co; i++)
      Fprintf(stderr, "\tfrec->denom_coeff[%d] = %e\n", 
	      i, frec->re_denom_coeff[i]);
  }


 
/*
 * Warn if filter order is too high, write output, and exit
 */
  if(order >= 40 )
    Fprintf(stderr, "%s: Filter design order probably too large to reliably design a\nfilter; please check filter design by plotting response.\n",argv[0]);

  put_feafilt_rec (frec, oh, fpout);
  
  (void) fclose (fpout);
  exit(0);
  /*NOTREACHED*/
}

float
  prewarp(rad_freq, samp_freq)
/*assumes that frequencies are expressed in radians/sec*/
/*implements Eqn. 7.112 in DFD by Parks and Burrus*/

float rad_freq;
float samp_freq;
{
  return((float)(2.*samp_freq*tan((double)(rad_freq/(2*samp_freq)))));
}

void
  low_pass_proto(b_edges, filter_response, filter_poly)
/* find critical low pass frequencies corresponding to filter specification*/
float *b_edges;		    /*band edges for elliptical filter*/
int filter_response;	    /* filter response type*/
int filter_poly;	    /* filter design polynomial*/
{
  float ctr;
  float tmp;
  
  switch(filter_response){
	case FILT_LP:
    /*low pass cutoff already defined*/
    break;
  case FILT_HP:
    /* replace S by 1/S */
    tmp = b_edges[1];
    b_edges[1] = 1./b_edges[0];   /* lowpass proto pass */
    b_edges[0] = 1./tmp;   /* lowpass proto stop */
    break;
  case FILT_BP:
    /* use Eqn 7.97 to get center freq (ctr) 
       and Eqn. 7.98 to get cutoff freq */
    
    ctr = (float)sqrt((double)(b_edges[1]*b_edges[2]));
    b_edges[1] = (SQRD(b_edges[2]) - SQRD(ctr))/b_edges[2];
    b_edges[2] = (SQRD(b_edges[3]) - SQRD(ctr))/b_edges[3];
    if(b_edges[2] < 0) b_edges[2] = FLT_MAX;
    b_edges[3] = (SQRD(ctr) - SQRD(b_edges[0]))/b_edges[0];
    if(b_edges[3] < 0) b_edges[3] = FLT_MAX;
    
    if(b_edges[3] < b_edges[2])
      b_edges[2] = b_edges[3];

    b_edges[0] = b_edges[1];    /* lowpass proto pass band */
    b_edges[1] = b_edges[2];    /* lowpass proto stop band */
    b_edges[2] = ctr;           /* orig. band pass center */
    break;
  case FILT_BS:
    /* use Eqn. 7.97 to get center freq (ctr) and Eqn. 7.100
       to get the cutoff frequency */
    
    ctr = (float)sqrt((double)(b_edges[0]*b_edges[3]));
    b_edges[0] = b_edges[0]/(ctr*ctr - SQRD(b_edges[0]));
    b_edges[1] = b_edges[1]/(ctr*ctr - SQRD(b_edges[1]));
    if(b_edges[1] < 0) b_edges[1] = FLT_MAX;
    b_edges[2] = b_edges[2]/(SQRD(b_edges[2]) - ctr*ctr);
    if(b_edges[2] < 0) b_edges[2] = FLT_MAX;
    if(b_edges[2] < b_edges[1])
      b_edges[1] = b_edges[2];
    b_edges[2] = ctr;           /* orig. band pass center */
    break;
  default:
    Fprintf(stderr, "low_pass_pro: Invalid filter response type.\n");
    exit(1);
  }
}

int determ_order(filter_poly, pass_loss, stop_loss, freq)
int filter_poly;
float stop_loss, pass_loss;            /* assumed in dBs */
float *freq;
{
  float c1, c2;
  float epsilon, tmp, junk;
  int order;
  c1 = pow(10., -pass_loss * .05);
  c2 = pow(10., -stop_loss * .05);
  switch(filter_poly){
  case BUTTERWORTH:
    c1 = 1./SQRD(c1) - 1.;
    c2 = 1./SQRD(c2) - 1.;
    tmp =  log(c2/c1) / (2 * log(freq[1]/freq[0])); /*order */
    order =  1 + tmp;                             /*order */
    freq[3] = pow(10., -log10(c1)/(2*tmp)) * freq[0];    /* 3dB frequecny */
    return(order);
    break;
  case CHEBYSHEV1:
    epsilon = sqrt(pow(10., pass_loss*.1) - 1);
    tmp =  sqrt(1-c2*c2) / (epsilon * c2*c2);
    order = 1 + ARCCOSH(tmp)/ARCCOSH(freq[1]/freq[0]);
    return(order);
    break;
  case CHEBYSHEV2:
    epsilon = 1. / sqrt(pow(10., stop_loss*.1) - 1);
    tmp = c1 / ( epsilon * sqrt( 1-c1*c1 ));
    order = 1 + ARCCOSH(tmp)/ARCCOSH(freq[1]/freq[0]);
    return(order);
    break;
  case ELLIPTICAL:
    order = elliptical_p_and_z(1, NULL, NULL, freq, NULL,
			       pass_loss, stop_loss, NULL, NULL);
    return(order);
    break;
  default:
    Fprintf(stderr,"determ_order: Invalid filter polynomial type.\n");
    exit(1);
  }
}

void
poles_and_zeros( poles, zeros, cut_freq, order, p_loss, s_loss, type, np, nz)
/* find poles and zeros corresponding to low pass response*/
double_cplx *poles, *zeros;	/*real and imag poles and zeros*/
float *cut_freq;		/*cutoff frequency*/
int order;			/*desired filter order*/
float p_loss;			/* pass band loss*/
float s_loss;			/*stop band loss*/
int type;			/*filter polynomial type*/
int *np, *nz;                   /* size of poles and zeros, real is a single,
				   complex is as double */
{
    int count, i, half_order;
    double arg, temp_r, temp_i;

    half_order = (order+1)/2; /* take advantage of integer division*/

    if (2*half_order == order)
	count = 1;            /* order is even */
    else
	count = 0;            /* order is odd */

    switch(type){
    case BUTTERWORTH:
      /* Use Eqn. 7.13 for poles; zeros are at frequency = infinity*/
      *nz = order;
      for( i=0; i< *nz; i++ ){      /* treate real zero as a single */
	zeros[i].real = 0;
	zeros[i].imag = FLT_MAX;
      }
      *np = half_order;
      for(i = 0; i < *np; i++){
	arg = (PI/2.)*count/order;
	count += 2;
	poles[i].real = cut_freq[3]*-cos(arg);
	poles[i].imag = cut_freq[3]*sin(arg);
      }
      break;
    case CHEBYSHEV1:
      /* Use Eqn. 7.36 to calculate epsilon, eqn 7.29 to
	 calculate v, and 7.33 to calculate poles.
	 The zeros all occur at infinity. */
      {
	double epsilon, v, sinhm, coshm;
	*nz = order;
	for( i=0; i< *nz; i++ ){      /* treate real zero as a single */
	  zeros[i].real = 0;
	  zeros[i].imag = FLT_MAX;
	}
	epsilon = sqrt(pow((double)10., (.1*p_loss)) - 1);
	v = ARCSINH(1./epsilon)/order;
	sinhm = sinh(v);
	coshm = cosh(v);
	*np = half_order;
	for(i = 0; i < *np; i++){
	  arg = (PI/2.)*count/order;
	  count += 2;
	  poles[i].real = cut_freq[0]*sinhm*-cos(arg);
	  poles[i].imag = cut_freq[0]*coshm*sin(arg);
	}
      }
      break;
    case CHEBYSHEV2:
      {
	double epsilon, v, sinhm, coshm;
	*nz = *np = half_order;
	epsilon = 1.0 / sqrt(pow((double)10., (.1*s_loss)) - 1);
	v = ARCSINH(1./epsilon)/order;
	sinhm = sinh(v);
	coshm = cosh(v);
	for(i = 0; i < half_order; i++){
	  arg = (PI/2.)*count/order;
	  zeros[i].real = 0.;
	  if(count != 0)
	    zeros[i].imag = cut_freq[1]/sin(arg);
	  else	   /* odd order, get a real zero at inf. and a real pole */
	    zeros[i].imag = FLT_MAX;
	  count += 2;

	  temp_r = sinhm*-cos(arg);
	  temp_i = coshm*sin(arg);
	  poles[i].real = cut_freq[1] *temp_r/(SQRD(temp_r)+SQRD(temp_i));
	  poles[i].imag = cut_freq[1] *temp_i/(SQRD(temp_r)+SQRD(temp_i));
	}
      }
      break;
    }
}



int
elliptical_p_and_z(init, poles, zeros, b_edges, order, p_loss, s_loss, np, nz)
int init;                       /* a flag to find order */
double_cplx *poles, *zeros;	/*real and imag poles and zeros*/
float *b_edges;			/*pass and stop band frequencies*/
int order;			/*desired filter order*/
float p_loss;			/* pass band loss*/
float s_loss;			/*stop band loss*/
int *np, *nz;                   /* size of poles and zeros */
{
  static double epsilon, k, kc, K, KC, k1, k1c, K1, K1C;
  double v0, snc, sn, cnc, cn, dnc, dn;
  double cei(), arcsc(), fk();
  void elp();
  int half_order, count, i, j;
  
  if(init){
    epsilon = sqrt(pow((double)10., (.1*p_loss)) - 1);
    k = b_edges[0] / b_edges[1];             /* passband over stop band */
    kc = sqrt(1-k*k);
    k1 = epsilon / sqrt( pow((double)10., (.1*s_loss)) -1 );
    k1c = sqrt(1-k1*k1);
    K = cei(kc);
    KC = cei(k);
    K1 = cei(k1c);
    K1C = cei(k1);
    
    /* recalculate k1 */
    return( (int) K*K1C/(K1*KC) + 1.);
  }
  k1 = fk(order*KC/K);
  k1c = sqrt(1-k1*k1);
  K1 = cei(k1c);

  half_order = (order+1)/2; /* take advantage of integer division*/
  
  if (2*half_order == order)
    count = 1;            /* order is even */
  else
    count = 0;            /* order is odd */
  
  v0 = (K/(K1* order)) * arcsc(1/epsilon, k1);
  elp(v0, k, &snc, &cnc, &dnc);

  if(debug_level > 20)
    fprintf(stderr,"epsilon %f k %f kc %f K %f KC %f\n\tk1 %f k1c %f K1 %f K1C %f\n\tv0 %f snc %f cnc %f dnc %f\n\torder %d \n", epsilon, k, kc, K, KC, k1, k1c, K1, K1C, v0, snc, cnc, dnc, order);
  zeros[0].imag = FLT_MAX;

  for(i=0, j=0; i< half_order; i++, j++){
    elp(K*count/order, kc, &sn, &cn, &dn);
    if(debug_level >20 )
      fprintf(stderr,"sn %f cn %f dn %f\n", sn, cn, dn);
    zeros[j].real = 0;
    if( count != 0 ) zeros[j].imag = b_edges[1]/sn;
/*    else {
      zeros[++j].real = 0;
      zeros[j].imag = FLT_MAX;
    }*/
    poles[i].real = -b_edges[0] * snc*cnc*cn*dn/(1-dn*snc*dn*snc);
    poles[i].imag = b_edges[0] * dnc*sn/(1-dn*snc*dn*snc);
    count += 2;
  }
  *np = i;
  *nz = j;
}

double fk(u)
double u;
{
  double q, a = 1,b=1,c=1,d;
  int i;
  q = exp(-PI*u);
  d = q;
  for(i=0; c >= 1e-7; i++){
    a = a+2*c*d;
    c = c*d*d;
    b = b+c;
    d = d*q;
    if(i>15){
      fprintf(stderr, "fk: fails to converge\n");
      exit(1);
    }
  }
  return(4*sqrt(q)*(b*b)/(a*a));
}
  
void elp(x, k, sn, cn, dn)
double x, k, *sn, *cn, *dn;
{
  int i;
  double aa[16], bb[16];
  double a, b, at, c,d, e;
  if(x == 0){
    *sn = 0;
    *cn = 1;
    *dn = 1;
    return;
  }
  a = 1;
  b = k;
  for(i=0; (a-b)/a >= 1.3e-7 ;){
    aa[i] = a;
    bb[i] = b;
    at = (a+b) /2;
    b = sqrt(a*b);
    a = at;
    if(i>15){
      fprintf(stderr,"elp: elp fails to converge\n");
      exit(1);
    }
    i++;
  }
  c = a/tan(x*a);
  d = 1;
  for(i--; i>=0 ;i--){
    e = c*c/a;
    c = c*d;
    a = aa[i];
    d = (e+bb[i])/(e+a);
  }
  *sn = 1/sqrt(1+c*c);
  *cn = *sn *c;
  *dn = d;
}
    



double arcsc(u, k)
double u,k;
{
  double a, b, y, bt;
  int i, j=0;

  a= 1;
  b= k;
  y = 1./u;
  for(i=0; ; i++){
    bt = a*b;
    a = a+b;
    b= 2*sqrt(bt);
    y = y-bt/y;
    if(y==0) y = sqrt(bt)*1e-10;
    if(fabs(a-b) < a*1.2e-7){
      if(y<0) j++;
      return((atan(a/y) + PI*j)/a);
    }
    j = 2*j;
    if(y<0) j++;
    if(i>15){
      fprintf(stderr,"arcsc: arcsc fails to converge\n");
      exit(1);
    }
  }
}
  
    
    

double
  cei(k)
double k;
{
  double a, b, at;
  int i;
  a = 1;
  b = k;
  for(i = 0; (a-b)/a >= 1.2e-7 ; i++){
    at = (a+b)/2;
    b = sqrt(a*b);
    a = at;
    if( i > 20){
      fprintf(stderr,"cei: complete elliptic integral fails to converge.\n");
      exit(1);
    }
  }
  return(1.570796326794896/a);
}
    

/* change low pass prototype to desired response type*/

void
  freq_xfrm(filt_order,type, coeff, cntr, nroots)
int filt_order;     /* filter order */
int type;	    /* filter response type */
double_cplx *coeff; /* coefficients*/
float cntr;	    /*warped geometric center frequency*/
int *nroots;	    /*number of pole or zero to take care of (either
		     as single real or double complex */
{
  int i,j, upper_idx;
  double_cplx a, b, c;
  
  switch(type){
  case FILT_LP:
    /* No transformation needed - input is low pass */
    break;
  case FILT_HP:
    /* zeros at infinity end up at zero frequency,
       and poles get inverted (S becomes 1/S)*/
    
    for(i = 0; i < *nroots; i++){
      if( coeff[i].imag > 1e20){
	coeff[i].real = 0.;
	coeff[i].imag = 0;
      }
      else{
	a.real = coeff[i].real;
	a.imag = coeff[i].imag;
	b.real = 1.;
	b.imag = 0.;
	if(a.real == 0. && a.imag == 0.){
	  Fprintf(stderr, "freq_xfrm: HP: real_pole[%d] = %lf and imag_pole[%d] = %lf\n", i, coeff[i].real, i, coeff[i].imag);
	  exit(1);
	}
	b = cdiv(b,a);
	coeff[i].real = -fabs(b.real);
	coeff[i].imag = fabs(b.imag);
      }
    }

    break;
  case FILT_BP:
    /*zero at imfinity becomes one at zero frequency and
      one at infinity. 
      Use Eqn 7.99 to get the poles*/
    {
      double_cplx *tmprry, tmp;
      tmprry = (double_cplx *) malloc( 2*filt_order * sizeof(double_cplx));
      spsassert(tmprry, "freq_xfrm: tmprry malloc failed.\n");
      j=0;
      for(i=(*nroots)-1; i>=0 ;i--){
	if(coeff[i].imag > 1e20){
	  tmprry[j].real = 0.;
	  tmprry[j].imag = 1e20;
	  j++;
	  tmprry[j].real = 0.;
	  tmprry[j].imag = 0.;
	  j++;
	}
	else{
	  c.real = 4.*SQRD(cntr);
	  c.imag = 0;
	  a.real = coeff[i].real;
	  a.imag = coeff[i].imag;
	  tmp = csub(cmult(a,a),c);
	  b = csub(a, csqrt( csub(cmult(a, a), c)));
	  tmprry[j].real = -fabs(b.real)/2.;
	  tmprry[j].imag = fabs(b.imag)/2.;
	  j++;

	  /* if not complex conjugate*/
	  if( a.imag !=0 || tmp.imag!=0 || tmp.real >0) 
	    b = cadd(a, csqrt( csub(cmult(a, a), c)));
	  /* when 1 real root -> 1 cplx root, this b will be the same as
	     the last b, but no need to store it twice */
	  if( !(tmp.imag == 0 &&                         /*1. from real root */
		-fabs(b.real)/2 == tmprry[j-1].real &&  /*2. this cplx root */
		fabs(b.imag)/2 == tmprry[j-1].imag && /*=cplx conj. of last */
		((float) b.imag) != 0)){                 /*3. to cplx root */
	    tmprry[j].real = -fabs(b.real)/2.;
	    tmprry[j].imag = fabs(b.imag)/2.;
	    j++;
	  }
	}
      }
      *nroots = j;
      for(i=0;i<*nroots;i++){
	coeff[i].real = tmprry[i].real;
	coeff[i].imag = tmprry[i].imag;
      }
      free(tmprry);
    }
    break;
 case FILT_BS:
    /* Zeros at infinity become double zeros at the center
       frequency (cntr). Use Eqn. 7.101 for the poles*/
    {
      double_cplx *tmprry, tmp;
      tmprry = (double_cplx *) malloc( 2*filt_order * sizeof(double_cplx));
      spsassert(tmprry, "freq_xfrm: tmp malloc failed.\n");
      j=0;
      for(i = *nroots -1; i>=0; i--){
	if(coeff[i].imag > 1e20){
	  tmprry[j].real = 0.;
	  tmprry[j].imag = cntr;
	  j++;
	  }
	else{
	  c.real = 4.*SQRD(cntr);
	  c.imag = 0.;
	  a.real = coeff[i].real;
	  a.imag = coeff[i].imag;
	  b.real = 1.;
	  b.imag = 0.;
	  a = cdiv(b, a);
	  tmp = csub(cmult(a,a),c);
	  b = csub(a, csqrt( csub(cmult(a, a), c)));
	  tmprry[j].real = -fabs(b.real)/2.;
	  tmprry[j].imag = fabs(b.imag)/2.;
	  j++;

	  /* if not complex conjuate pair */
	  if( a.imag !=0 || tmp.imag != 0 || tmp.real>0 )  
	    b = cadd(a, csqrt( csub( cmult(a, a), c )));
	  /* when 1 real root -> 1 cplx root, this b will be the same as
	     the last b, but no need to store it twice */
	  if( !(tmp.imag == 0 &&                         /*1. from real root */
		-fabs(b.real)/2 == tmprry[j-1].real &&  /*2. this cplx root */
		fabs(b.imag)/2 == tmprry[j-1].imag && /*=cplx conj. of last */
		((float) b.imag) != 0)){                 /*3. to cplx root */
	    tmprry[j].real = -fabs(b.real)/2.;
	    tmprry[j].imag = fabs(b.imag)/2.;
	    j++;
	  }
	}
      }
      *nroots = j;
      for(i=0;i<*nroots;i++){
	coeff[i].real = tmprry[i].real;
	coeff[i].imag = tmprry[i].imag;
      }
      free(tmprry);
    }
    break;
  default:
    Fprintf(stderr, "freq_xfrm: Invalid filter reponse type.\n");
    exit(1);
  }
}


void
  blt(type, sf, coeff, order)
/*transform frequencies back to digital domain*/
/* Use Eqn 7.108*/

int type;	    /*frequency response type - i.e LP, HP, etc.*/
int order;	    /*desired filter order*/
float sf;	    /*sampling frequency*/
double_cplx *coeff; /*real coefficients*/
{
  int i,j;
  double temp;
  for(i=0; i<order; i++){
    if(fabs(coeff[i].imag) > 1e15 || fabs(coeff[i].real) > 1e15){
      coeff[i].real = -1.;
      coeff[i].imag = 0.;
    }
    else{
	temp = SQRD(2*sf - coeff[i].real) + SQRD(coeff[i].imag);
	coeff[i].real = (SQRD(2*sf) - SQRD(coeff[i].real) - 
			 SQRD(coeff[i].imag))/temp;
	coeff[i].imag = 4.*sf*coeff[i].imag/temp;
      }
  }
}

void
  pz_to_numden(npz, pz, nco, co)
int npz;
double_cplx *pz;
short *nco;
double *co; 
{
  double *real, *imag;
  int i;
  real = (double *) malloc (npz*sizeof(double));
  spsassert(real,"pz_to_numden: real malloc failed\n");
  imag = (double *) malloc (npz*sizeof(double));
  spsassert(imag,"pz_to_numden: imag malloc failed\n");
  for(i=0; i<npz; i++){
    real[i] = pz[i].real;
    imag[i] = pz[i].imag;
  }
  pz_to_co_d(npz, real, imag, nco, co);
  free(real);
  free(imag);
}
  


void
set_gain(response_type, frec, gain, gain_factor, band_edges, sf, poly)

/* This module works correctly for BUTTERWORTH filters only. For
    CHEBYSHEV1 filters, the specified gain is somewhere between the
    minimum and the maximum of the ripples */

int response_type;
struct feafilt *frec;   /* data */
double gain;		/* desired pass band gain */
double *gain_factor;	/* computed gain factor*/
float *band_edges;		/*BW, CH1, and CH2 critical frequencies*/
float sf;		/* sampling frequency*/
int poly;		/* filter polynomial type - BW, CH!, CH2, etc*/
{

  int j, i;
  double_cplx uc;
  double up=1, down=1;
  double_cplx *poles;
  double_cplx *zeros;
  double del;
  long psiz, zsiz;
  float phase;
  float magrry[128], maxr=0;
  int N = 128;

  psiz = *frec->pole_dim;
  zsiz = *frec->zero_dim;
  poles = frec->poles;
  zeros = frec->zeros;
  
  switch(response_type){
    
  case FILT_LP:
  case FILT_BS:
    /* evaluate at 0 freq on circle*/
    phase = 0;
    del = ( 2 * PI * band_edges[0] / sf) /N;      /* pass band */
    break;
  case FILT_HP:
    phase = 2 * PI * band_edges[1] /sf;
    del = (PI - phase) / N;
    break;
  case FILT_BP:
    {
      phase = 2 * PI * (band_edges[1]/sf);
      del = ((2 * PI * band_edges[2]/sf) - phase ) / N;
      break;
    }
  default:		
    Fprintf(stderr,
	    "set_gain: Invalid filter response type - exiting\n");
    exit(1);
  }
  phase -= del;
  for(i=0; i< N; i++){
    up = 1;
    down = 1;
    phase += del;
    uc.real = cos(phase);
    uc.imag = sin(phase);
    for( j=0; j<psiz; j++ ){
      down *= (uc.real - poles[j].real) * (uc.real - poles[j].real) 
	+ (uc.imag - poles[j].imag) * (uc.imag - poles[j].imag);
      if( poles[j].imag != 0)
	down *= (uc.real - poles[j].real)* (uc.real -poles[j].real)
	  + (uc.imag + poles[j].imag) * (uc.imag + poles[j].imag);
    }
    for( j=0; j<zsiz; j++ ){
      up *= (uc.real - zeros[j].real) * (uc.real - zeros[j].real) 
	+ (uc.imag - zeros[j].imag) * (uc.imag - zeros[j].imag);
      if( zeros[j].imag != 0 )
	up *= (uc.real - zeros[j].real) * (uc.real - zeros[j].real) 
	  + (uc.imag + zeros[j].imag) * (uc.imag + zeros[j].imag);
    }
    magrry[i] = up/down;
  }

  for(i=0;i<N;i++)
    if(magrry[i] > maxr) maxr = magrry[i];
  *gain_factor =  gain/sqrt(maxr);
  
  for(j = 0; j < *frec->num_size; j++)
    frec->re_num_coeff[j] = frec->re_num_coeff[j]*(*gain_factor);
}







