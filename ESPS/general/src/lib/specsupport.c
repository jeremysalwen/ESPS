/* 
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				

 	
 * Written by:  Alan Parker
 * 			
 * Module:	specsupport.c
 *	
 * Alan Parker, Entropic Speech, Inc.
 * 
 * This module provides the record support for the ESPS spec file.
 */

#ifndef lint
static char	*sccs_id = "@(#)specsupport.c	1.1	10/19/93 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/spec.h>

#ifndef DEC_ALPHA
char *calloc();
char	*malloc();
#endif

void
spec_ue_check(func, hd)
char	*func;
struct header *hd;

{
	spsassert(hd != NULL, func);
	spsassert(hd->common.type == FT_SPEC, func);
	spsassert(hd->hd.spec->num_freqs >= 0, func);
	return;
}


/* allocate memory for spec_data for use with file described by header
 * hd
 */

struct spec_data *
allo_spec_rec(hd)
struct header *hd;

{
	struct spec_data *p;
	spsassert(hd != NULL, "allo_spec_rec: hd is NULL");
	spsassert(hd->common.type == FT_SPEC, "allo_spec_rec: file not SPEC");
	spsassert(hd->hd.spec->num_freqs >= 0, "allo_spec_rec: num_freq < 0");

	p = (struct spec_data *)calloc((unsigned)1, sizeof *p);
	p->re_spec_val=(float *)calloc((unsigned)hd->hd.spec->num_freqs, 
		sizeof (float));
	p->im_spec_val=(float *)calloc((unsigned)hd->hd.spec->num_freqs, 
		sizeof (float));
	p->frqs = (float *)calloc((unsigned)hd->hd.spec->num_freqs, 
		sizeof (float));
	return(p);
}


int
get_spec_rec(p, hd, file)
struct spec_data *p;
struct header *hd;
FILE *file;

{
	int	num_freqs, edr, machine;

	spsassert(hd != NULL, "get_spec_rec: hd is NULL");
	spsassert(p != NULL, "get_spec_rec: p is NULL");
	spsassert(file != NULL, "get_spec_rec: file is NULL");
	spsassert(hd->common.type == FT_SPEC, "get_spec_rec: file not SPEC");
	spsassert(hd->hd.spec->num_freqs >= 0, "get_spec_rec: num_freq < 0");

	num_freqs = hd->hd.spec->num_freqs;
	edr = hd->common.edr;
	machine = hd->common.machine_code;

	if (hd->common.tag == YES)
		if (!miio_get_long(&p->tag, 1, edr, machine, file))
			return(EOF);
	if (!miio_get_float(&p->tot_power, 1, edr, machine, file))
		if (hd->common.tag == YES) 
			goto trouble;
		else 
			return(EOF);
	if (!miio_get_float(p->re_spec_val, num_freqs, edr, machine, file))
		goto trouble;
	if (hd->hd.spec->spec_type == ST_CPLX)
		if (!miio_get_float(p->im_spec_val, num_freqs, edr, machine, file))
			goto trouble;
	if (hd->hd.spec->freq_format == ARB_VAR) {
		if (!miio_get_float(p->frqs, num_freqs, edr, machine, file))
			goto trouble;
		if (!miio_get_long(&p->n_frqs, 1, edr, machine, file))
			goto trouble;
	}
	if (hd->hd.spec->frame_meth != FM_NONE && 
	    hd->hd.spec->frame_meth != FM_FIXED)
		if (!miio_get_short(&p->frame_len, 1, edr, machine, file))
			goto trouble;
	if (hd->hd.spec->voicing == YES)
		if (!miio_get_short(&p->voiced, 1, edr, machine, file))
			goto trouble;
	return(1);
trouble: 
	fprintf(stderr, "get_spec_rec: short record read.\n");
	return(1);
}


void
put_spec_rec(p, hd, file)
struct spec_data *p;
struct header *hd;
FILE *file;

{
	int	num_freqs;

	spsassert(hd != NULL, "put_spec_rec: hd is NULL");
	spsassert(p != NULL, "put_spec_rec: p is NULL");
	spsassert(file != NULL, "put_spec_rec: file is NULL");
	spsassert(hd->common.type == FT_SPEC, "put_spec_rec: file not SPEC");
	spsassert(hd->hd.spec->num_freqs >= 0, "put_spec_rec: num_freq < 0");

	num_freqs = hd->hd.spec->num_freqs;
	if (hd->common.tag == YES)
		if (!miio_put_long(&p->tag, 1, hd->common.edr, file)) 
			goto trouble;
	if (!miio_put_float(&p->tot_power, 1, hd->common.edr, file)) 
		goto trouble;
	if (!miio_put_float(p->re_spec_val, num_freqs, hd->common.edr, file))
		goto trouble;
	if (hd->hd.spec->spec_type == ST_CPLX)
		if (!miio_put_float(p->im_spec_val, num_freqs, 
		  hd->common.edr, file))
			goto trouble;
	if (hd->hd.spec->freq_format == ARB_VAR) {
		if (!miio_put_float(p->frqs, num_freqs, hd->common.edr, file))
			goto trouble;
		if (!miio_put_long(&p->n_frqs, 1, hd->common.edr, file)) 
			goto trouble;
	}
	if (hd->hd.spec->frame_meth != FM_NONE && 
	    hd->hd.spec->frame_meth != FM_FIXED)
		if (!miio_put_short(&p->frame_len, 1, hd->common.edr, file))
			goto trouble;
	if (hd->hd.spec->voicing == YES)
		if (!miio_put_short(&p->voiced, 1, hd->common.edr, file)) 
			goto trouble;
	return;
trouble:
	fprintf(stderr, "put_spec_rec: write error.\n");
	exit(1);
}


/* print a spec record; useful for debugging and used by psps.
 */


void
print_spec_rec(p, hd, file)
struct spec_data *p;
struct header *hd;
FILE *file;

{
	int	i;
	int	j;
	long	n;
	float	sf;
	float	*freqs;
	double	freqmin;

	spsassert(hd != NULL, "print_spec_rec: hd is NULL");
	spsassert(p != NULL, "print_spec_rec: p is NULL");
	spsassert(file != NULL, "print_spec_rec: file is NULL");
	spsassert(hd->common.type == FT_SPEC, "print_spec_rec: file not SPEC");
	spsassert(hd->hd.spec->num_freqs >= 0, "print_spec_rec: num_freq < 0");

	if (hd->common.tag == YES) 
		fprintf(file, "tag: %ld, ", p->tag);

	fprintf(file, "frame_len: %d, voiced: %s tot_power: %e\n", 
		p->frame_len, (p->voiced ? "YES" : "NO" ), p->tot_power);

	if (hd->hd.spec->freq_format == ARB_VAR)
		n = p->n_frqs;
	else
		n = hd->hd.spec->num_freqs;

	freqs = (float *) malloc((unsigned) n * sizeof(float));
	(sf = hd->hd.spec->sf) || (sf = 1.0);

	switch (hd->hd.spec->freq_format) {
	case SYM_CTR:
		freqmin = sf / (4 * n);
		for (j = 0; j < n; j++)
			freqs[j] = freqmin + j * sf / (2 * n);
		break;
	case SYM_EDGE:
		freqmin = 0;
		for (j = 0; j < n; j++)
			freqs[j] = j * sf / (2 * (n - 1));
		break;
	case ASYM_CEN:
		freqmin = -sf / 2 + sf / (2 * n);
		for (j = 0; j < n; j++)
			freqs[j] = freqmin + j * sf / n;
		break;
	case ASYM_EDGE:
		freqmin = -sf / 2;
		for (j = 0; j < n; j++)
			freqs[j] = freqmin + j * sf / (n - 1);
		break;
	case ARB_VAR:
		for (j = 0; j < n; j++)
			freqs[j] = p->frqs[j];
		break;
	case ARB_FIXED:
		for (j = 0; j < n; j++)
			freqs[j] = hd->hd.spec->freqs[j];
		break;
	default:
		for (j = 0; j < n; j++)
			freqs[j] = p->frqs[j];
		break;
	}

	fprintf(file, "freq\t\tre_spec_val\tim_spec_val\n");

	for (i = 0; i < n; i++) {
		fprintf(file, "%e", freqs[i]);
		fprintf(file, "\t%e", p->re_spec_val[i]);
		if (hd->hd.spec->spec_type == ST_CPLX)
			fprintf(file, "\t%e", p->im_spec_val[i]);
		fprintf(file, "\n");
	}
	return;
}
