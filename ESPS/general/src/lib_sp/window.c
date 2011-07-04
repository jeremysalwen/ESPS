/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1997  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Burton
 * Checked by:
 * Revised by:  Derek Lin for Kaiser window
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)window.c	1.13	24 Mar 1997	ESI/ERL";

/* This function applies a data window to a set of points.
   Currently rectangular, triangular, cos**4, Hanning, Hamming, Kaiser
   windows are supported. */

#include <math.h>

#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/window.h>
#include <esps/strlist.h>
#include <esps/constants.h>

/*
 * Add new strings at the end of this list (just before the NULL),
 * and don't change the order of the existing strings.
 * Keep the list of constant symbols in <esps/window.h> consistent
 * with this list.
 */

char *window_types[] = {
    "WT_NONE",
    "WT_RECT",
    "WT_HAMMING",
    "WT_TRIANG",
    "WT_HANNING",
    "WT_COS4",
    "WT_KAISER",
    "WT_ARB",
    "WT_SINC",
    "WT_SINC_C4",
    NULL};

double pow();

static void
do_hamming(size, input, output)
long size;
float *input, *output;
{
    long i;

/*
 * Window data
*/
    for(i=0;i<size;i++){
	output[i] = input[i] * (.54 - .46*cos(2*PI*i/(size-1)));
    }
    
    return;
}

static void
do_hamming_d(size, input, output)
long size;
double *input, *output;
{
    long i;

/*
 * Window data
*/
    for(i=0;i<size;i++){
	output[i] = input[i] * (.54 - .46*cos(2*PI*i/(size-1)));
    }
    
    return;
}

static void
do_hamming_cf(size, input, output)
long size;
float_cplx *input, *output;
{
    long i;
    float wval;

/*
 * Window data
*/
    for(i=0;i<size;i++){
        wval = (.54 - .46*cos(2*PI*i/(size-1)));
	output[i].real = input[i].real * wval;
	output[i].imag = input[i].imag * wval;
    }
    
    return;
}

static void
do_hamming_cd(size, input, output)
long size;
double_cplx *input, *output;
{
    long i;
    double wval;
/*
 * Window data
*/
    for(i=0;i<size;i++){
        wval = (.54 - .46*cos(2*PI*i/(size-1)));
	output[i].real = input[i].real * wval;
        output[i].imag = input[i].imag * wval;
    }
    
    return;
}

static void
do_triangular(size, input, output)
long size;
float *input, *output;
{
    long i;
    float tmp;
/*
 * Window data
*/
    for(i=0; i <= (size-1)/2; i++){
	tmp = (float)(2 * i)/(float)(size - 1);
	output[i] = input[i]*tmp;
	output[size-1-i] = input[size-1-i]*tmp;
    }
	
    return;
}

static void
do_triangular_d(size, input, output)
long size;
double *input, *output;
{
    long i;
    double tmp;
/*
 * Window data
*/
    for(i=0; i <= (size-1)/2; i++){
	tmp = (double)(2 * i)/(double)(size - 1);
	output[i] = input[i]*tmp;
	output[size-1-i] = input[size-1-i]*tmp;
    }
	
    return;
}
static void
do_triangular_cf(size, input, output)
long size;
float_cplx *input, *output;
{
    long i;
    double tmp;
/*
 * Window data
*/
    for(i=0; i <= (size-1)/2; i++){
	tmp = (double)(2 * i)/(double)(size - 1);
	output[i].real = input[i].real*tmp;
	output[i].imag = input[i].imag*tmp;
	output[size-1-i].real = input[size-1-i].real*tmp;
	output[size-1-i].imag = input[size-1-i].imag*tmp;
    }
	
    return;
}
static void
do_triangular_cd(size, input, output)
long size;
double_cplx *input, *output;
{
    long i;
    double tmp;
/*
 * Window data
*/
    for(i=0; i <= (size-1)/2; i++){
	tmp = (double)(2 * i)/(double)(size - 1);
	output[i].real = input[i].real*tmp;
	output[i].imag = input[i].imag*tmp;
	output[size-1-i].real = input[size-1-i].real*tmp;
	output[size-1-i].imag = input[size-1-i].imag*tmp;
    }
	
    return;
}

static void
do_hanning(size, input, output)
     register long size;
     register float *input, *output;
{
  static float *wind = NULL;
  static long nwind = 0;
  if(size > 1) {
    register float *wp;
    register long i;

    if(size != nwind) {
      register double arg = (2.0*PI/(size-1)), half = 0.5;

      if(size > nwind) {
	if(wind)
	  free(wind);
	if(!(wind = (float*)malloc(sizeof(float)*size))) {
	  (void)fprintf(stderr,"Allocation problems in window(%d)\n",size);
	  return;
	}
      }
      nwind = size;
      for(i = 0, wp = wind; i < size; i++)
	*wp++ = (half - half*cos(arg*(double)(i)));
    }
    wp = wind;
    do { *output++ = *input++ * *wp++; } while( --size);
  }
  return;
}

static void
do_hanning_d(size, input, output)
long size;
double *input, *output;
{
  long i;
  double arg;
  for(i = 0, arg = (2*PI/(size-1)); i < size; i++){
      output[i] = input[i] * 
       (.5 - .5*cos(arg*(double)(i)));
  }
  
  return;
}

static void
do_hanning_cf(size, input, output)
long size;
float_cplx *input, *output;
{
  long i;
  double arg, wval;
  for(i = 0, arg = (2*PI/(size-1)); i < size; i++){
      wval = (.5 - .5*cos(arg*(double)(i)));
      output[i].real = input[i].real * wval;
      output[i].imag = input[i].imag * wval;
  }
  
  return;
}

static void
do_hanning_cd(size, input, output)
long size;
double_cplx *input, *output;
{
  long i;
  double arg, wval;
  for(i = 0, arg = (2*PI/(size-1)); i < size; i++){
      wval = (.5 - .5*cos(arg*(double)(i)));
      output[i].real = input[i].real * wval;
      output[i].imag = input[i].imag * wval;
  }
  
  return;
}

static void
do_cos4(size, input, output)
long size;
float *input, *output;
{
  long i;
  double arg, tmp;
  for(i = 0, arg = 2*PI/(size-1); i < size; i++){
    tmp = .5*(1.0 - cos(arg * (double)(i)));
    tmp = pow(tmp, (double)2.);
    output[i] = input[i] * tmp;
  }

  return;
}


static void
do_cos4_d(size, input, output)
long size;
double *input, *output;
{
  long i;
  double arg, tmp;
  for(i = 0, arg = (2*PI/(size-1)); i < size; i++){
    tmp = .5*(1.0 - cos(arg * (double)(i)));
    tmp = pow(tmp, (double)2.);
    output[i] = input[i] * tmp;
  }

  return;
}

static void
do_cos4_cf(size, input, output)
long size;
float_cplx *input, *output;
{
  long i;
  double arg, tmp;
  for(i = 0, arg = (2*PI/(size-1)); i < size; i++){
    tmp = .5*(1.0 - cos(arg * (double)(i)));
    tmp = pow(tmp, (double)2.);
    output[i].real = input[i].real * tmp;
    output[i].imag = input[i].imag * tmp;
  }

  return;
}

static void
do_cos4_cd(size, input, output)
long size;
double_cplx *input, *output;
{
  long i;
  double arg, tmp;
  for(i = 0, arg = (2*PI/(size-1)); i < size; i++){
    tmp = .5*(1.0 - cos(arg * (double)(i)));
    tmp = pow(tmp, (double)2.);
    output[i].real = input[i].real * tmp;
    output[i].imag = input[i].imag * tmp;
  }

  return;
}

/****************************/
static void
do_kaiser(size, input, output, parameters)
long size;
float *input, *output;
double *parameters;
{
  long i;
  double *generate_kaiser(), *kaiser;
  spsassert(parameters!=NULL, 
	    "window: Parameter vector for user-defined window is NULL\n");
  kaiser = generate_kaiser(size, parameters[0]);
  for(i = 0; i < size; i++){
    output[i] = input[i] * kaiser[i];
  }
  return;
}

static void
do_kaiser_d(size, input, output, parameters)
long size;
double *input, *output;
double *parameters;
{
  long i;
  double *generate_kaiser(), *kaiser;
  spsassert(parameters!=NULL, 
	    "window: Parameter vector for user-defined window is NULL\n");
  kaiser = generate_kaiser(size, parameters[0]);
  for(i = 0; i < size; i++){
    output[i] = input[i] * kaiser[i];
  }
  return;
}

static void
do_kaiser_cf(size, input, output, parameters)
long size;
float_cplx *input, *output;
float_cplx *parameters;
{
  long i;
  double *generate_kaiser(), *kaiser;
  spsassert(parameters!=NULL, 
	    "window: Parameter vector for user-defined window is NULL\n");
  kaiser = generate_kaiser(size, (double) parameters[0].real);
  for(i = 0; i < size; i++){
    output[i].real = input[i].real * kaiser[i];
    output[i].imag = input[i].imag * kaiser[i];
  }
  return;
}

static void
do_kaiser_cd(size, input, output, parameters)
long size;
double_cplx *input, *output;
double_cplx *parameters;
{
  long i;
  double *generate_kaiser(), *kaiser;
  spsassert(parameters!=NULL, 
	    "window: Parameter vector for user-defined window is NULL\n");
  kaiser = generate_kaiser(size, parameters[0].real);
  for(i = 0; i < size; i++){
    output[i].real = input[i].real * kaiser[i];
    output[i].imag = input[i].imag * kaiser[i];
  }
  return;
}

double *generate_kaiser(size, beta)
long size;
double beta;
{
  double *kaiser, y, alpha, i0();
  long i;
  kaiser = (double *) malloc( size * sizeof(double));
  spsassert(kaiser,"malloc failed in window.c for kaiser");
  alpha = ((double)(size-1)) / 2.0;
  for(i=0; i< size; i++){
    y = pow( 1 - ( (i-alpha)/alpha ) * ( (i-alpha)/alpha), 0.5);
    kaiser[i] = i0( beta * y ) / i0( beta );
  }
  return(kaiser);
}

  
/* Modified Bessel function, from Kaiser(1974) */

double i0( x )
     register double x;
{
  register double d, ds, s;

  ds = 1;
  d = 2;
  s = ds;
  ds = (x*x)/4.0;
  while ( ds >= .2e-8*s )
    {
      d += 2;
      s += ds;
      ds *= (x*x) / (d*d);
    }
  return s;
}

static void
do_arb(size, input, output, parameters)
long size;
float *input, *output;
double *parameters;
{
  long i;

  spsassert(parameters!=NULL, 
	    "window: Parameter vector for user-defined window is NULL\n");

  for(i = 0; i < size; i++){
    output[i] = input[i] * parameters[i];
  }

  return;
}

static void
do_arb_d(size, input, output, parameters)
long size;
double *input, *output;
double *parameters;
{
  long i;

  spsassert(parameters!=NULL, 
	    "windowd: Parameter vector for user-defined window is NULL\n");

  for(i = 0; i < size; i++){
    output[i] = input[i] * parameters[i];
  }

  return;
}

static void
do_arb_cf(size, input, output, parameters)
long size;
float_cplx *input, *output;
float_cplx *parameters;
{
  long i;

  spsassert(parameters!=NULL, 
	    "windowd: Parameter vector for user-defined window is NULL\n");

  for(i = 0; i < size; i++){
    output[i].real = input[i].real * parameters[i].real;
    output[i].imag = input[i].imag * parameters[i].imag;
  }

  return;
}

static void
do_arb_cd(size, input, output, parameters)
long size;
double_cplx *input, *output;
double_cplx *parameters;
{
  long i;

  spsassert(parameters!=NULL, 
	    "windowd: Parameter vector for user-defined window is NULL\n");

  for(i = 0; i < size; i++){
    output[i].real = input[i].real * parameters[i].real;
    output[i].imag = input[i].imag * parameters[i].imag;
  }

  return;
}

/*      ----------------------------------------------------------      */
sync_fir(n,nf,coef,type)
/* Create the coefficients for a symmetric FIR lowpass filter using the
	window technique with a Hanning window. */
     float	*coef;		/* preallocated array of length nf */
     register long n,		/* time extent of ideal filter */
       nf;			/* time extent of truncated impulse response */
     int type;			/* type of truncation */
{
    register long	i, ieo;
    register double	twopi, fn, c, t, fc;

    fc = 1.0/(double)n;

    /*  Compute part of the ideal impulse response (the sin(x)/x kernel). */
    twopi = M_PI * 2.0;
    c = M_PI * 2.0 * fc;
    fn = twopi * fc;
    for(i=0; i<nf; i++) coef[i+nf] = sin((0.5 + i) * fn)/(c * (0.5 + i));

    /* Now apply a cos^4 window to the (infinite) impulse response. */
    fn = M_PI/(double)(nf);
    
    ieo = (nf << 1) - 1;
    if(type == WT_SINC_C4)
      for(i=0; i<nf; i++) {
	t = (.5 - (.5 * cos(fn * ((double)i + 0.5))));
	coef[i] = (coef[ieo - i] *= t*t*t*t);
      }
    else
      for(i=0; i<nf; i++) {
	t = (.5 - (.5 * cos(fn * ((double)i + 0.5))));
	coef[i] = (coef[ieo - i] *= t);
      }
    return(TRUE);
}


float  *sncwindow(n,m,type)
     long n;			/* effective window length */
     int type;			/* simple sinc vs cos^4 weight */
     unsigned long m;		/* array size */
{
  static float *sc = NULL;
  static unsigned long sc_size = 0;
  static long win_size = 0;
  static int old_type = -1;
 
  if(m > sc_size) {
    if(sc)
      free(sc);
    if(! (sc = (float*)malloc(sizeof(float)*m))) {
      fprintf(stderr, "Allocation failure in sncwindow()\n");
      sc_size = 0;
      return(NULL);
    }
  }
    
  if((m != sc_size) || (n != win_size) || (type != old_type))
    sync_fir(n, (long) (m>>1), sc, type);
  sc_size = m;
  win_size = n;

  return(sc);
}

int gp_window(nsamp, in, out, type, p, argp)
     int type;
     long nsamp;
     float *in, *out;
     double *p;
     void *argp;
{
  switch(type) {
  case WT_SINC:
  case WT_SINC_C4:
    {
      unsigned long arr_size = *(unsigned long*)argp;
      float *wind;

      if((wind = sncwindow(nsamp, arr_size, type))) {
	for( ; arr_size > 0; arr_size--)
	  *out++ = *in++ * *wind++;
	return(0);
      } else
	return(1);
    }
    default:
      return(window(nsamp, in, out, type, p));
  }
}

int
window(nsamp, in, out, type, p)
int type;
long nsamp;
float *in, *out;
double *p;

{

    void do_hamming();
    void do_triangular();
    void do_hanning();
    void do_cos4();
    void do_arb();
    void do_kaiser();
    long i;

    spsassert(in, "window: Input array NULL");
    spsassert(out,"window: Output array NULL");
    spsassert(nsamp > 0,"window: nsamp is less than one");


    switch (type){
    case WT_HAMMING:
	do_hamming(nsamp, in, out);
	return(0);
	break;
    case WT_RECT: /*rectangular window*/
	for(i=0; i<nsamp; i++)
	    out[i]=in[i];
	return(0);
	break;
    case WT_TRIANG:/*triangular window*/
	do_triangular(nsamp, in, out);
	return(0);
	break;
    case WT_HANNING: 
	do_hanning(nsamp, in, out);
	return(0);
	break;
    case WT_COS4:/*cos**4 window*/
	do_cos4(nsamp, in, out);
	return(0);
	break;
   case WT_KAISER:
	do_kaiser(nsamp, in , out, p);
	return(0);
	break;
    case WT_ARB: /*user defined*/
	do_arb(nsamp, in, out, p);
	break;
    default:
	(void)fprintf(stderr, "window: Invalid window type specified\n");
	return(1);
    }
/* NOTREACHED */
}

int
windowd(nsamp, in, out, type, p)
int type;
long nsamp;
double *in, *out;
double *p;

{

    void do_hamming_d();
    void do_triangular_d();
    void do_hanning_d();
    void do_cos4_d();
    void do_arb_d();
    void do_kaiser_d();
    long i;

    spsassert(in, "windowd: Input array NULL");
    spsassert(out,"windowd: Output array NULL");
    spsassert(nsamp > 0, "windowd: nsamp is less than  1");

    switch (type)
      {
      case WT_HAMMING: /*Hamming window*/
	do_hamming_d(nsamp, in, out);
	return(0);
	break;
      case WT_RECT: /*rectangular window*/
	for(i=0; i<nsamp; i++)
	  out[i]=in[i];
	return(0);
	break;
      case WT_TRIANG: /*triangular window*/
	do_triangular_d(nsamp, in, out);
	return(0);
	break;
      case WT_HANNING: 
	do_hanning_d(nsamp, in, out);
	return(0);
	break;
      case WT_COS4:/*cos**4 window*/
	do_cos4_d(nsamp, in, out);
	return(0);
	break;
      case WT_KAISER:
	do_kaiser_d(nsamp, in, out, p);
	return(0);
	break;
      case WT_ARB:/*user defined*/
	do_arb_d(nsamp, in, out, p);
	return(0);
	break;
      default:
	(void)fprintf(stderr, "windowd: Invalid window type specified\n");
	return(1);
      }
    /* NOTREACHED */
}

int
windowcf(nsamp, in, out, type, p)
int type;
long nsamp;
float_cplx *in, *out;
float_cplx *p;

{

    void do_hamming_cf();
    void do_triangular_cf();
    void do_hanning_cf();
    void do_cos4_cf();
    void do_arb_cf();
    void do_kaiser_cf();
    long i;

    spsassert(in, "windowcf: Input array NULL");
    spsassert(out,"windowcf: Output array NULL");
    spsassert(nsamp > 0, "windowcf: nsamp is less than  1");

switch (type){
    case WT_HAMMING: /*Hamming window*/
	do_hamming_cf(nsamp, in, out);
	return(0);
	break;
    case WT_RECT: /*rectangular window*/
	for(i=0; i<nsamp; i++)
	    out[i]=in[i];
	return(0);
	break;
    case WT_TRIANG: /*triangular window*/
	do_triangular_cf(nsamp, in, out);
	return(0);
	break;
    case WT_HANNING: 
	do_hanning_cf(nsamp, in, out);
	return(0);
	break;
    case WT_COS4:/*cos**4 window*/
	do_cos4_cf(nsamp, in, out);
	return(0);
	break;
    case WT_KAISER:/*cos**4 window*/
	do_kaiser_cf(nsamp, in, out, p);
	return(0);
	break;
    case WT_ARB:/*user defined*/
	do_arb_cf(nsamp, in, out, p);
	return(0);
	break;
    default:
	(void)fprintf(stderr, "windowcf: Invalid window type specified\n");
	return(1);
    }
/* NOTREACHED */
}

int
windowcd(nsamp, in, out, type, p)
int type;
long nsamp;
double_cplx *in, *out;
double_cplx *p;

{

    void do_hamming_cd();
    void do_triangular_cd();
    void do_hanning_cd();
    void do_cos4_cd();
    void do_kaiser_cd();
    void do_arb_cd();
    long i;

    spsassert(in, "windowcd: Input array NULL");
    spsassert(out,"windowcd: Output array NULL");
    spsassert(nsamp > 0, "windowcd: nsamp is less than  1");

switch (type){
    case WT_HAMMING: /*Hamming window*/
	do_hamming_cd(nsamp, in, out);
	return(0);
	break;
    case WT_RECT: /*rectangular window*/
	for(i=0; i<nsamp; i++)
	    out[i]=in[i];
	return(0);
	break;
    case WT_TRIANG: /*triangular window*/
	do_triangular_cd(nsamp, in, out);
	return(0);
	break;
    case WT_HANNING: 
	do_hanning_cd(nsamp, in, out);
	return(0);
	break;
    case WT_COS4:/*cos**4 window*/
	do_cos4_cd(nsamp, in, out);
	return(0);
	break;
    case WT_KAISER:/*cos**4 window*/
	do_kaiser_cd(nsamp, in, out, p);
	return(0);
	break;
   case WT_ARB:/*user defined*/
	do_arb_cd(nsamp, in, out, p);
	return(0);
	break;
    default:
	(void)fprintf(stderr, "windowcd: Invalid window type specified\n");
	return(1);
    }
/* NOTREACHED */
}


/*
 * Get numeric window type code from name with no "WT_" prefix.
 * Returns WT_NONE for undefined (or for explicit argument "NONE").
 */

#define WT_PREFIX   "WT_"

int
win_type_from_name(name)
    char    *name;
{
    char    *pref_w_name;
    int	    win;

    spsassert(name != NULL, "win_type_from_name: name is NULL");

    pref_w_name = (char *) malloc(sizeof(WT_PREFIX) + strlen(name));
    spsassert(pref_w_name != NULL,
	      "can't allocate storage for window type name");
    sprintf(pref_w_name, "%s%s", WT_PREFIX, name);
    win = lin_search(window_types, pref_w_name);
    free(pref_w_name);

    return (win == -1) ? WT_NONE : win;
}
