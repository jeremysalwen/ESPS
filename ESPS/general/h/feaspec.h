/*
 *  This material contains proprietary software of Entropic Speech, Inc.
 *  Any reproduction, distribution, or publication without the the prior
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing
 *  by Entropic Speech, Inc. must bear the notice
 * 								
 *    "Copyright (c) 1988-1990 Entropic Speech, Inc.
 *     Copyright (c) 1990-1995 Entropic Research Laboratory, Inc.
 *                   All rights reserved."
 *
 *  		@(#)feaspec.h	1.6	2/20/96	ESI
 * include file for feature file subtype FEA_SPEC (spectral records)
 */

#ifndef feaspec_H
#define feaspec_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esps/esps.h>


/*
 * Defines for freq_format.
 * Keep consistent with string array in feaspecsupp.c.
 * While SPEC supported, keep consistent with #defines for freq_format
 * in esps/header.h.  (But note spelling: ASYM_CTR here, ASYM_CEN there.)
 */
/* NONE is also a valid value (= 0) */
#define SPFMT_SYM_CTR	1
#define SPFMT_SYM_EDGE	2
#define SPFMT_ASYM_CTR	3
#define SPFMT_ASYM_EDGE	4	
#define SPFMT_ARB_VAR	5
#define SPFMT_ARB_FIXED	6

extern char *spfmt_names[];

/*
 * Defines for spec_type.
 * Keep consistent with string array in feaspecsupp.c.
 * While SPEC supported, keep consistent with #defines for spec_type
 * in esps/header.h.
 */
/* NONE is also a valid value (= 0) */
#define SPTYP_PWR	1
#define SPTYP_DB	2
#define SPTYP_REAL	3
#define SPTYP_CPLX	4

extern char *sptyp_names[];

/*
 * Defines for frame_meth.
 * Keep consistent with string array in feaspecsupp.c.
 * While SPEC supported, keep consistent with #defines for frame_meth
 * in esps/header.h.
 */
#define SPFRM_NONE	0
#define SPFRM_FIXED 	1
#define SPFRM_VARIABLE 	2

extern char *spfrm_names[];

/*
 * structure definition for FEA_SPEC records 
 */

struct feaspec
{
    long	    *tag;
    float	    *tot_power;
    float	    *re_spec_val;
    char	    *re_spec_val_b;
    float	    *im_spec_val;
    float	    *frqs;
    long	    *n_frqs;
    long	    *frame_len;
    struct fea_data *fea_rec;	/*pointer to corresponding FEA record*/
};

/*
 * The items in the feaspec structures have the following shapes
 * if the corresponding header fields are defined;
 * the pointers are NULL otherwise.
 * Dimension information refers to generic header items.
 *
 *	Item		Shape	Dimension
 *
 *	tag		scalar
 *	tot_power	scalar
 *	re_spec_val	vector	num_freqs
 *	re_spec_val_b	vector	num_freqs
 *	im_spec_val	vector	num_freqs
 *	n_frqs		scalar
 *	frqs		vector	num_freqs
 *	frame_len	scalar
 * 
 * The pointer fea_rec does not correspond to actual data that is
 * part of the feaspec record.  
 */

/*
 * Declarations for support functions.
 */

struct feaspec *
allo_feaspec_rec ARGS((struct header *hd, int re_spec_format));

int
init_feaspec_hd ARGS((struct header *hd, int def_tot_power,
		      int freq_format, int spec_type, int contin,
		      long int num_freqs, int frame_meth, float *freqs,
		      double sf, long int frmlen, int re_spec_format));

long
get_feaspec_rec ARGS((struct feaspec *spec_rec,
		      struct header *hd, FILE *file));

long
put_feaspec_rec ARGS((struct feaspec *spec_rec,
		      struct header *hd, FILE *file));

void
print_feaspec_rec ARGS((struct feaspec *p, struct header *hd, FILE *file));

char **
get_feaspec_xfields ARGS((struct header *hd));


#ifdef __cplusplus
}
#endif

#endif /* feaspec_H */
