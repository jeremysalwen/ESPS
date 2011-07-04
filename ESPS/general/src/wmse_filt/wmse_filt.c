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
 *  Written By:   Brian Sublett   8/13/86
 *  Modified for ESPS 3.0  by David Burton
 *  Modified for FEAFILT files by Bill Byrne, 5/24/91
 *  Checked by: 
 *
 *  Description: This program designs an FIR filter using
 *		the weighted mean square error criterion.
 *
 *  Secrets:  See John Burg's notes on weighted mean square
 *	     error filter design.
 *
 *********************************************************/

static char *sccs_id = "@(#)wmse_filt.c	3.11  6/23/98  ERL";
char *Version="3.11", *Date="6/23/98";

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
# define MX_S 2047  /* Max Number of characters in user entered comments. */
# define COM_S 50  /* Extra characters in default comments. */
# define SYNTAX USAGE ("wmse_filt [-P param_file] [-x debug_level][-n filt_length] [-d d_file][-w w_file] feafilt_file");
# define SIXTY 60 /* maximum number of characters in input file name*/
#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
					 argv[0], text); exit(1);}

/*
 * ESPS Functions
 */
    char *get_cmd_line ();
    void get_fftd();


int debug_level = 0;

main (argc, argv)
int argc;
char *argv[];
{
    
    FILE *fpin, *fpout;        	/*input and output stream pointers*/
    struct header *oh;	/*ptr to output file header*/
    struct feafilt *frec;		/*output data structure*/
    filtdesparams fdp;
    int i, j, c;
    int cflag = NO, w_flag = 0;		/*option flags*/
    int nco = 0;			/* number of filter coefficients*/
    int uniform_weight = 0, point_mode = 0; /*more flags*/
    char *d_file=NULL, *w_file=NULL, *filt_file, *param_file = NULL;

    double *h, *w, *hw, *filt;		/*hold filter related coefficients*/
    double fs = 0.0, fs2;			/*sampling frequency of data*/
    double offset, x, temp;
    float *be, *wt, *g;			/*hold band edges, wts, and gains*/
    float *points;
    float *H, *W;
    double *R, *f, *b, sin (), cos ();
    double *data_real, *data_imag; /* time domain rep. of desired resp.*/
    int nh, nw, nW, nH, nhw, size;	/* number of items*/
    int Log2 ();		    /*compute closest power of 2*/
    int nb = 0, nflag=0;
    char *str, stri[3], *stu="band_edge", *stv="_des", *stb="band", *stw="_wt";

    extern char *optarg;
    extern optind;


/* Check the command line options. */

    while ((c = getopt (argc, argv, "P:x:n:d:w:n:")) != EOF){
      switch (c){
      case 'P':
	param_file = optarg;
	break;
      case 'x':
	debug_level = atoi(optarg);
	break;
      case 'n':
	nco = atoi(optarg);
	nflag++;
	if (((int)(nco/2))*2 == nco) 
	  ERROR_EXIT("wmse: Even number of coefficients not supported.")
        break;
      case 'd':
	point_mode = 1;
	d_file = optarg;
	break;
      case 'w':
	w_file = optarg;
	w_flag = 1;
	if (strcmp (w_file, "-") == 0)
	  uniform_weight = 1;
	break;
      default:
	SYNTAX
	}
    }

    /* Get the filenames. */
    
    if (optind < argc){
      filt_file = eopen("wmse_filt", argv [optind], "w", FT_FEA, FEA_FILT,
			&oh, &fpout);
      if (debug_level) Fprintf (stderr,"wmse: Output file is %s\n", filt_file);
    }
    else{
      Fprintf (stderr,"wmse: No output file specified.\n");
      SYNTAX
      exit (1);
    }

    if(!param_file && !point_mode){
      fprintf(stderr," no parameter file specified - exiting\n");
      SYNTAX
    }
    
    /* read parameter file */
    if( !point_mode){
      if (read_params (param_file, SC_NOCOMMON, (char *)NULL) != 0)
	ERROR_EXIT("read_params could not read the params file.");
      if(!nflag && symtype("filt_length") != ST_UNDEF) 
	nco = getsym_i("filt_length");
      else ERROR_EXIT("filt_length parameter not specified");
       if (((int)(nco/2))*2 == nco) 
	 ERROR_EXIT("wmse: Even number of coefficients not supported.")
       else
	 offset = 0.0;
      if( symtype("samp_freq") != ST_UNDEF) fs = getsym_d("samp_freq");
      else ERROR_EXIT("samp_freq not found");	
      fs2 = fs/2;
      if( symtype("nbands") != ST_UNDEF) nb = getsym_i("nbands");
      else ERROR_EXIT("nbands not found");	
      
      be = (float *) malloc( sizeof(float) * (nb+1));
      spsassert(be," Couldn't allocate space for bandedges values");
      g = (float *) malloc( sizeof(float) * nb);
      spsassert(g," Couldn't allocate space for desired gain values");
      wt = (float *) malloc( sizeof(float) * nb);
      spsassert(wt," Couldn't allocate space for weight values");
      str = (char *) malloc(sizeof(char)*20);
      for( i =1, j=0 ;i<=nb+1; i++, j++){
	str = (char *) strcpy( str, stu);
	sprintf(stri, "%d", i);
	str = (char *) strcat(str, stri);
	/* find edges */
	if( symtype(str) != ST_UNDEF){
	  if( (be[j] = getsym_d(str)) > fs2) 
	    ERROR_EXIT("band edges greater than Nyquist rate");
	}
	else{
	  fprintf(stderr,"%s: missing %d-th band edge\n", argv[0], i);
	  exit(1);
	}
      }
      for(j=1; j<=nb; j++)
	if( be[j] < be[j-1] )
	  ERROR_EXIT("band edges must be specified in an increasing order");
      if( be[0] !=0 || be[nb] != fs2)
	ERROR_EXIT("first and last bandedges must start at 0 and end at Nyquist rate");
      /* normalize */
      for(i=0; i<=nb; i++) be[i] = be[i] / fs;
      for(i=1,j=0 ; i<=nb; i++, j++){
	str = (char *) strcpy( str, stb);
	sprintf(stri, "%d", i);
	str = (char *) strcat(str, stri);
	str = (char *) strcat(str, stv);
	/* find desired value */
	if( symtype(str) != ST_UNDEF)
	  g[j] = getsym_d(str);
	else{
	  fprintf(stderr,"%s: missing desired value for band %d\n", argv[0], i);
	  exit(1);
	}
      }
      for(i=1,j=0 ; i<=nb; i++, j++){
	str = (char *) strcpy( str, stb);
	sprintf(stri, "%d", i);
	str = (char *) strcat(str, stri);
	str = (char *) strcat(str, stw);
	/* find desired value */
	if( symtype(str) != ST_UNDEF)
	  wt[j] = getsym_d(str);
	else{
	  fprintf(stderr,"%s: missing weighting value for band %d\n", argv[0], i);
	  exit(1);
	}
      }
      /* Allocate the array space.  */
      /* This is actually twice as many hw's as I need. */
      nhw = nw = 2*nco - 1;
      w = (double*) calloc ((unsigned)nw, sizeof(double));
      spsassert(w != NULL, 
		"Couldn't allocate space for weights");
      hw = (double*) calloc ((unsigned)nhw, sizeof(double));
      spsassert(hw != NULL, 
		"Couldn't allocate space for combined weight and freq. resp");
      for (i=0; i<nhw; i++) {
	w[i] = 0.0;
	hw[i] = 0.0;
      }
      /* Calculate points on a sum of sinc functions.  */
      for (i=0; i<nhw; i++)
      {
	  x = i - nhw/2 + offset;
	  /* sum over components from each band. */
	  for (j=0; j<nb; j++){
	    if (x == 0.0)
	      temp = be[j+1] - be[j];
	    else{
	      if (be[j+1] == 0.0)
		temp = 0.0;
	      else
		temp = be[j+1]*sin (2.0*PI*x*be[j+1])/(2*PI*x*be[j+1]);
	      if (be[j] == 0)
		temp -= 0.0;
	      else
		temp -= be[j]*sin (2.0*PI*x*be[j])/(2*PI*x*be[j]);
	    }
	    temp *= 2.0;
	    w[i] += wt[j]*temp;
	    hw[i] += g[j]*wt[j]*temp;
	  }
      }
      if (debug_level)
	(void)printarr ((char *)w, "lf", nw, "warray");
    }
    /* pointwise mode  */
    else {               
      if( !nflag ) ERROR_EXIT("must specify the -n filter length option for point mode")
      if ((fpin = fopen (d_file, "r")) == NULL){
	Fprintf (stderr,"Can't open %s\n", d_file);
	exit (1);
      }
      
      /* Read the desired function. */
      (void)fscanf(fpin, "%d", &nH);
      H = (float*) calloc ((unsigned)2*nH, sizeof(float));
      spsassert(H != NULL, 
		"Couldn't allocate space for desired freq. response");
      
      for (i=0; i<nH; i++)
	(void)fscanf(fpin, "%f", &H[i]);
      
      if (uniform_weight){
	nW = nH;
	W = (float*) calloc ((unsigned)2*nW, sizeof(float));
	spsassert(W != NULL, 
		  "Couldn't allocate space for weighting function");
	
	for (i=0; i<nW; i++)
	  W[i] = 1.0;
      }
      else{
	if(w_file){
	  if (strcmp (d_file, w_file) != 0){
	    (void)fclose (fpin);
	    if ((fpin = fopen (w_file, "r")) == NULL){
	      Fprintf (stderr,"Can't open %s\n", w_file);
	      exit (1);
	    }
	  }
	}
	
	/* Read the weighting function. */
	if(EOF == fscanf(fpin, "%d", &nW))
	  ERROR_EXIT("No weight values in file in pointwise mode");
	if (nW != nH){
	  Fprintf (stderr, "wmse:Weighting function and Desired response\n");
	  Fprintf (stderr, "     must have same number of points.\n");
	  Fprintf(stderr," points of desired values: %d\n", nH);
	  Fprintf(stderr," points of weighting values: %d\n", nW);
	  exit (1);
	}
	W = (float*) calloc ((unsigned)2*nW, sizeof(float));
	spsassert(W != NULL, 
		  "Couldn't allocate space for weighting function");
	for (i=0; i<nW; i++){
	  (void)fscanf(fpin, "%f", &W[i]);
	  if (W[i] < 0.0){
	    Fprintf (stderr,"wmse: Band weights can't be less than zero.\n");
	    exit (1);
	  }
	}
      }
      /* Allocate space for the other arrays.  */
      h = (double*) calloc ((unsigned)2*nH, sizeof (double));
      spsassert(h != NULL, 
		"Couldn't allocate space for desired freq. response");
      w = (double*) calloc ((unsigned)2*nW, sizeof (double));
      spsassert(w != NULL, 
		"Couldn't allocate space for weights");
      points   = (float*) calloc ((unsigned)nH, sizeof (float));
      spsassert(points != NULL, 
		"Couldn't allocate space for frequency points");
      /* Fill in a points array for the output header.  */
      for (i=0; i<nH; i++){
	points[i] = (0.5*i)/nH;
      }
      /* Compute desired autocorrelation function.  */
      /* Make it symmetrical. */
      for (i=0; i < nH - 1; i++) {
	H[2*nH-2-i] = H[i];
      }
      data_real = (double*) calloc ((unsigned)2*nH-2, sizeof(double));
      spsassert(data_real != NULL, 
		"Couldn't allocate space for real part of fft");
      data_imag = (double*) calloc ((unsigned)2*nH-2, sizeof(double));
      spsassert(data_imag != NULL, 
		"Couldn't allocate space for imagunary part of fft");
      for (i=0; i<2*nH-2; i++){
	data_real[i] = H[i];
	data_imag[i] = 0.0;
      }
      if (debug_level)
	Fprintf (stderr,"wmse: Log2(%d)=%d\n", 2*nH-2, Log2(2*nH-2));
      (void)get_fftd (data_real, data_imag, Log2 (2*nH-2));
      
      /* Get the symmetric function in h. */
      nh = 2*nH - 2;
      h[nH-1] = data_real[0]/nh;
      for (i=1; i<nH; i++){
	h[nH - 1 + i] = h[nH - 1 - i] = data_real[i]/nh;
      }
      nh = 2*nH - 1;
      if (debug_level)
	(void)printarr ((char *)h, "lf", nh, "harray");

      /* Make it symmetrical. */
      for (i=0; i < nW - 1; i++) {
	W[2*nW-2-i] = W[i];
      }
      for (i=0; i<2*nW-2; i++){
	data_real[i] = W[i];
	data_imag[i] = 0.0;
      }
      if (debug_level)
	Fprintf (stderr,"wmse: Log2(%d)=%d\n", 2*nW-2, Log2(2*nW-2));
      (void)get_fftd (data_real, data_imag, Log2 (2*nW-2));
      
      nw = 2*nW - 2;
      w[nH-1] = data_real[0]/nw;
      for (i=1; i<nW; i++){
	w[nW - 1 + i] = w[nW - 1 - i] = data_real[i]/nw;
      }
      nw = 2*nW - 1;
      if (debug_level)
	(void)printarr ((char *)w, "lf", nw, "warray");
      
      /* Allocate and file convolution array.   */
      nhw = nw + nh - 1;
      hw = (double*) calloc ((unsigned)nhw, sizeof (double));
      spsassert(hw != NULL, 
	    "Couldn't allocate space for convolution array");
      
      /* Convolve h and w to get the b's.   */
      (void)convolv (h, nh, w, nw, hw, &nhw);
    }
    
    /*** Back to operations for both modes.  ***/
    filt = (double*) calloc ((unsigned)nco, sizeof (double));
	spsassert(filt != NULL, 
	    "Couldn't allocate space for filt array");

    if (debug_level)
        (void)printarr ((char *)hw, "lf", nhw, "hwarray");

/* Point the dummy pointers to the centers of the arrays. */
    b = hw + (int)(nhw/2);
    R = w + (int)(nw/2);
    f = filt + (int)(nco/2);

    size = nco/2 + 1;

/* Get the filter solution.   */
    (void)itopod (R, b, f, size);

/* Copy over the lower half of filt.  */
    for (i=0; i<size; i++)
	filt[i] = filt[nco-1-i];
    
/* Set key values in output header. */

    oh = new_header (FT_FEA);
    (void)add_comment (oh, get_cmd_line (argc, argv));

    oh->common.tag = NO;
    fdp.method = WMSE;
    fdp.type = FILT_ARB;
    fdp.filter_complex = NO;
    fdp.define_pz = NO;
    fdp.g_size = 0;
    fdp.nbits = 0;
    if (point_mode)
    {
	fdp.func_spec = POINT;
	fdp.nbands = 0;
	fdp.bandedges = NULL;
	fdp.npoints = nH;
	fdp.gains = H;
	fdp.wts = W;
	fdp.points = points;
    }
    else
    {
	fdp.func_spec = BAND;
	fdp.nbands = nb;
	fdp.bandedges = be;
	fdp.npoints = 0;
	fdp.gains = g;
	fdp.wts = wt;
	fdp.points = NULL;
    }

    Strcpy (oh->common.prog, "wmse_filt");
    Strcpy (oh->common.vers, Version);
    Strcpy (oh->common.progdate, Date);

/* add samp_freq as generic header item */
    (void)add_genhd_d("samp_freq", &fs, 1, oh);

/* add filter delay in samples */
    {
      int delay_samples = nco/2;
      double delay = delay_samples;
      (void)add_genhd_d("delay_samples", &delay, 1, oh);
    }

    init_feafilt_hd( oh, (long) nco, (long) 0, &fdp);

    write_header (oh, fpout);

/* Allocate the data record and fill in the values. */

    frec = allo_feafilt_rec (oh);
    *frec->num_size = nco;
    *frec->denom_size = 0;
/* Copy coefficients to a float array.  */
    for (i=0; i<nco; i++)
      frec->re_num_coeff[i] = filt[i];

/* Write the data to the output file. */

    put_feafilt_rec (frec, oh, fpout);

    
    (void) fclose (fpout);
    exit(0);
    return(0); /*lint pleasing*/
}

/********************************************************/
/* This returns the greatest power of two <= num.  */

int Log2 (num)
int num;
    {
    int count = 0;

    while ((num /= 2) != 0)
	count ++;

    return (count);
    }
