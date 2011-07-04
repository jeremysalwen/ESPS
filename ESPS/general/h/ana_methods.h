/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1995  Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * @(#)ana_methods.h	1.4 2/20/96 ERL
 *
 * Written by:  John Shore 
 * Checked by:
 * Revised by:
 *
 * Brief description: include file for programs that compute or
 *	transform analysis parameters
 *
 */

#ifndef ana_methods_H
#define ana_methods_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>

#define AM_NONE 0
#define AM_AUTOC 1
#define AM_BURG 2
#define AM_COV 3
#define AM_MBURG 4
#define AM_FBURG 5
#define AM_VBURG 6
#define AM_STRCOV 7
#define AM_STRCOV1 8

#ifndef LIB
extern char *ana_methods[];
#endif

/*
 * Function declarations.
 */

int 
compute_rc ARGS((float *sdata, int sdata_size, int method,
		 int dcrem, int win_type, int order, int sincn,
		 int sc_iter, double sc_conv,
		 float *rc, double *r, float *gain));

void
get_atal ARGS((double *r, int order, float *c, float *rc, float *pgain));

void
get_auto ARGS((float *x, int lnt, double *r, int order));

int
covar ARGS((float *data, int lnt, float *c, int order,
	    float *grc, float *pgain, int window_flag));

void
get_burg ARGS((float *x, int lnt, double *r, int order,
	       float *c, float *rc, float *pgain, int method));
/* method is specified as char in pre-ANSI style in getburg.c;
 * therefore it is passed as int.
 */

void
get_fburg ARGS((float *x, int lnt, float *a, int order,
		float *rc, float *pgain));

void
get_vburg ARGS((float *x, int lnt, double *r, int order,
		float *c, float *rc, float *pgain, int matsiz));

void
strcov_auto ARGS((float *data, int lnt, double *R, int order,
		  int matsiz, int window_flag, int alg,
		  double conv_test, int max_iter));
/*
 * alg is specified as char in pre-ANSI style in bestauto.c;
 * therefore it is pased as double.
 */

int
struct_cov ARGS((double **S, double *R, double *distance,
		 int size, double cvgtst, int max_iter));

void
genburg ARGS((double *sigma_in, double *isigma_in, int *qd, double *pdist,
	      double *sigma_out, double *isigma_out,
	      int c_flag, int monitor_flag, int *returnflag,
	      double *R_out, double *iR_out, int init_flg, int anderson));

void
estimate_covar ARGS((float *data, int lnt,
		     double *Sxx, int matsiz, int window_flag));

int
afc2cep ARGS((float *afc, float *cep, int order));

int
auto_pefrc ARGS((int order, double *r, double *pef, double *c));

int
cep2afc ARGS((float *cep, float *afc, int order));

float
lar_to_rc ARGS((double lar));

int
lsf_to_pc ARGS((float *lsf, float *pc, int order, double bandwidth));
/* bandwidth is specified as float in pre-ANSI style in lsf_to_pc.c;
 * therefore it is passed as double.
 */

int
pc_to_lsf ARGS((float *pc,
		float *lsf, int order, double bwidth, double frq_res));
/* bwidth and frq_res are specified as float in pre-ANSI style in pc_to_lsf.c;
 * therefore they are passed as double.
 */

int
pef_autorc ARGS((int order, double *pef, double *r, double *c));

int
rc_autopef ARGS((int order, double *c, double *r, double *pef));

int
rc_reps ARGS((float *rc, float *output,
	      int output_type, int size, double bwidth, double frq_res));
/* bwidth and frq_res are specified as float in pre-ANSI style in rc_reps.c;
 * therefore they are passed as double.
 */
int
rc_to_lar ARGS((double rc, float *lar));

void
refl_to_auto ARGS((float *ref_coeff, double pfe, float *auto_corr, int order));
/* pfe is specified as float in pre-ANSI style in refl_to_auto.c;
 * therefore it is passed as double.
 */

int
refl_to_filter ARGS((float *rfl_cof, float *filt, int size));

int
reps_rc ARGS((float *input, int input_type,
	      float *rc, int size, double bwidth));
/* bwidth is specified as float in pre-ANSI style in reps_rc.c;
 * therefore it is passed as double.
 */


#ifdef __cplusplus
}
#endif

#endif /* ana_methods_H */
