/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Joseph T. Buck
 * Checked by:
 * Revised by:  Alan Parker
 *
 * Return the size of a record in an SPS file, based on the header.
 *
 */

static char *sccs_id = "@(#)sizerec.c	1.10	5/1/98	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/esignal_fea.h>
#include <esps/pc_wav.h>

size_rec (h)
struct header *h;
{
    int 	size, mach_code, edr, from_alpha;
    esignal_hd	*esig_hd;
    pc_wav_hd	*wav_hd;

    spsassert(h != NULL, "size_rec: h NULL");

    if ((esig_hd = get_esignal_hdr(h)) != NULL)
	return esignal_rec_size(esig_hd);

    if ((wav_hd = get_pc_wav_hdr(h)) != NULL)
	return pc_wav_rec_size(wav_hd);

    mach_code = h->common.machine_code;
    edr = h->common.edr;
    from_alpha = (mach_code == DEC_ALPHA_CODE) && !edr;

    size = h->common.ndouble * sizeof (double) +
	   h->common.nfloat * sizeof (float) +
	   h->common.nlong * (from_alpha ? 8 : 4) +
	   h->common.nshort * sizeof (short) +
	   h->common.nchar * sizeof (char) +
	   BOOL (h->common.tag) * (from_alpha ? 8 : 4);

    if ((int)get_genhd_val("encoding", h, (double)0.0)) {
/* then this file has mu-law encoded fields */
	size -= get_fea_siz("samples",h,NULL,NULL)*sizeof(short);
	size += get_fea_siz("samples",h,NULL,NULL)*sizeof(char);
    }
    return size;
}


size_rec2 (h)
struct header *h;
{
    int 	size, mach_code, edr, from_alpha;
    esignal_hd	*esig_hd;
    pc_wav_hd	*wav_hd;

    spsassert(h != NULL, "size_rec2: h NULL");

    if ((esig_hd = get_esignal_hdr(h)) != NULL)
	return esignal_rec_size(esig_hd);

    if ((wav_hd = get_pc_wav_hdr(h)) != NULL)
	return pc_wav_rec_size(wav_hd);

    mach_code = h->common.machine_code;
    edr = h->common.edr;
    from_alpha = (mach_code == DEC_ALPHA_CODE) && !edr;

    size = h->common.ndouble * sizeof (double) +
	   h->common.nfloat * sizeof (float) +
	   h->common.nlong * (from_alpha ? 8 : 4) +
	   h->common.nshort * sizeof (short) +
	   h->common.nchar * sizeof (char) +
	   BOOL (h->common.tag) * (from_alpha ? 8 : 4);
    return size;
}


int
intern_size_rec(h)
    struct header   *h;
{
    int		    size;

    spsassert(h != NULL, "intern_size_rec: h NULL");

    size = h->common.ndouble * sizeof(double)
	   + h->common.nfloat * sizeof(float)
	   + h->common.nlong * sizeof(long)
	   + h->common.nshort * sizeof(short)
	   + h->common.nchar * sizeof(char)
	   + BOOL(h->common.tag) * sizeof(long);

    return size;
}
