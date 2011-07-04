/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  John Shore (from refcof code)
 * Checked by:
 * Revised by:
 *
 * Brief description: compute RCs by various methods
 *
 */

static char *sccs_id = "@(#)compute_rc.c	1.10	8/12/91	ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/window.h>
#include <esps/ana_methods.h>

extern int debug_level;

static void rsinc();
static void rc2auto();
void	pr_farray();
double sin();

char  *ana_methods[] = {"NONE", "AUTOC", "BURG", "COV", "MBURG", "FBURG", "VBURG", "STRCOV", "STRCOV1", NULL};

#define Fprintf (void)fprintf
#define DEBUG(n) if (debug_level >= n) Fprintf
#define PI 3.141592654

int 
compute_rc(sdata, sdata_size, method, dcrem, win_type, order, sincn, sc_iter, sc_conv, rc, r, gain)
float  *sdata;
int    sdata_size;
int    method;
int    dcrem;
int    win_type;
int    order;
int    sincn;
int    sc_iter;
double sc_conv;
float  *rc;
double  *r;
float  *gain;
{
    static float *dcopy = NULL;
    static float *wx = NULL;
    float *lpc = NULL;
    static int last_size = 0;

    float energy = 0;  /*energy of residual */
    char *nomemory = "compute_rc: couldn't allocate enough memory";
    float dcval;
    int i;

    spsassert(sdata != NULL, "argument sdata is NULL");
    spsassert(rc != NULL, "argument rc is NULL");
    spsassert(r != NULL, "argument r is NULL");
    spsassert(gain != NULL, "argument gain is NULL");

     if (debug_level > 2) {
 	Fprintf(stderr, "Entered compute_rc with arguments:\n");
 	Fprintf(stderr, "method = %d, dcrem = %d, win_type = %d\n",
 		method, dcrem, win_type);
 	Fprintf(stderr, "order = %d, sincn = %d\n",
 		order, sincn);
 	Fprintf(stderr, "sc_iter = %d, sc_conv = %g\n",
 		sc_iter, sc_conv);
	Fprintf(stderr, "sdata has %d points\n", sdata_size); 

	if (debug_level > 3) 
	  pr_farray(sdata, sdata_size, "sdata");
       }

    if (sdata_size > last_size) {
	
	last_size = sdata_size; 

	if (dcopy) {
	    free(dcopy); 
	    dcopy = NULL;
	  }

	dcopy = (float *) calloc((unsigned) sdata_size, sizeof(float));

	if (wx) {
	  free(wx);
	  wx = NULL;
	}

	wx =(float *) calloc((unsigned) sdata_size, sizeof(double));

	spsassert(dcopy != NULL, nomemory);    
	spsassert(wx != NULL, nomemory);
      }

    lpc = (float *) calloc((unsigned) order + 1, sizeof(float));

    spsassert(lpc != NULL, nomemory);

    /*
     * initialize arrays 
     */
    *gain = 0;
    for (i = 0; i <= order; i++) {
	r[i] = rc[i] = lpc[i] = 0.0;
      }
    /*
     * remove DC if called for 
     */
    if (dcrem) {

	dcval = 0.0;
	for (i = 0; i < sdata_size; i++) 
	  dcval += sdata[i];

	dcval = dcval / sdata_size;

	for (i = 0; i < sdata_size; i++)
	  dcopy[i] = sdata[i] - dcval;

	if (debug_level > 2) 
	  Fprintf(stderr, "compute_rc: removed DC value %f\n", dcval);
      }
    else {
	for (i = 0; i < sdata_size; i++)
	  dcopy[i] = sdata[i];
      }

    /* Window data.  */

    (void) window((long)sdata_size, dcopy, wx, win_type, (double *) NULL);

    if (debug_level > 3) pr_farray(wx, sdata_size, "windowed frame");

    /*
     * compute reflection coefficients
     */

    if (debug_level > 1)
      Fprintf(stderr, "compute_rc: computing spectrum\n");
    switch (method) {
	case AM_AUTOC:

	DEBUG(2)(stderr, "calling get_auto\n"); 

	(void)get_auto(wx, sdata_size, r, order);
	/*r(0) is energy, so convert to power*/
	r[0] = r[0] / sdata_size;

	if (sincn != 0) 
	  rsinc(r, order, sincn);

	DEBUG(2)(stderr, "calling get_atal\n"); 
	
	(void)get_atal(r, order, lpc,  rc, gain);
	break;

	case AM_BURG:

	DEBUG(2)(stderr, "calling get_burg('b')\n"); 

	(void)get_burg(wx, sdata_size, r, order, lpc, rc, gain, 'b');
	/* r[0] is energy, so convert to power*/
	r[0] = r[0] / sdata_size;
	break;

	case AM_COV:
	/*
	 * Get energy
	 */
	DEBUG(2)(stderr, "calling get_auto\n"); 

	(void)get_auto(&wx[order], (sdata_size-order), r, 0);
	energy = r[0];

	/* find lpc coefficients*/
	DEBUG(2)(stderr, "calling covar\n"); 

	(void)covar(wx, sdata_size, lpc,  order, rc, gain, (int) 0);

	/* 
	 *find input power = energy / length
	 */
	r[0] = (energy / (sdata_size - order));
	break;

	case AM_MBURG:

	DEBUG(2)(stderr, "calling get_burg('m')\n"); 

	(void)get_burg(wx, sdata_size, r, order, lpc, rc, gain, 'm');
	/* r[0] is sum of squares, so convert to power*/
	r[0] = r[0] / (sdata_size - order);
	break;

	case AM_FBURG:
	
	DEBUG(2)(stderr, "calling get_fburg\n"); 
	
	(void)get_fburg(wx, sdata_size, lpc,  order, rc, gain);

	(void) rc2auto(rc, r, 1); 

	break;

	case AM_VBURG:

	DEBUG(2)(stderr, "calling get_vburg\n"); 

	(void)get_vburg(wx, sdata_size, r, order, lpc, rc, gain, order + 1);

	/* r[0] is sum of squares, so convert to power*/
	r[0] = r[0] / (sdata_size - order);

	break;

	case AM_STRCOV:

	/* structured covariance via Fraenkel's  function */

	DEBUG(2)(stderr, "calling strcov_auto\n"); 

	(void)strcov_auto(wx, sdata_size, r, order, order + 1, (int) 0, 
		       'f', sc_conv, sc_iter);

	if (sincn != 0) 
	  rsinc(r, order, sincn);
	    
	DEBUG(2)(stderr, "calling get_atal\n"); 

	(void)get_atal(r, order, lpc, rc, gain);
	break;

	case AM_STRCOV1:

	/* structured covariance via Wenger's function */

	DEBUG(2)(stderr, "calling strcov_auto\n"); 

	(void)strcov_auto(wx, sdata_size, r, order, order + 1, (int) 0, 
		       'w', sc_conv, sc_iter);

	if (sincn != 0) 
	  rsinc(r, order, sincn);
	    
	DEBUG(2)(stderr, "calling get_atal\n"); 

	(void)get_atal(r, order, lpc, rc, gain);
	break;

	default:		/* should never get here */
	(void) fprintf(stderr, "compute_rc: unsupported analysis method");
	free(lpc);
	return(-1); 
	}
    if (debug_level > 2) {
	for (i = 0; i <= order; i++)
	  Fprintf(stderr,
		  "r[%d] = %f rc[%d] = %f lpc[%d] = %f\n", 
		  i, r[i], i, rc[i], i, lpc[i]);
	Fprintf(stderr, "\n\n");
      }
    free(lpc);
    return(0); 
  }

static void
rsinc(aut, order,n)
double *aut;
int n, order;
{
    int i;
    double arg;
    for (i=1; i<=order; i++) {
	arg = (double)(PI*i)/n;
	aut[i] = aut[i]*(sin (arg)/(arg));
    }
}

static void
rc2auto(input, output,  size)
float *input;
double *output; 
int size;
{
    float *autoc;
    int i;
/*
 * allocate memory
*/
    autoc = (float *)calloc ((unsigned)(size+2),  sizeof(float));

/*
 * Process spectral parameters
*/

	(void)refl_to_auto(input - 1, 1., autoc, (int)size);
	for(i=0; i<size; i++)
	    output[i] = (double) autoc[i+2]/autoc[1];

/*
 * free memory
*/
    free((char *)autoc);
}





