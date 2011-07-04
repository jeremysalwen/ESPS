/*  sps_compat - check various ESPS files for compatability
 *
 *  This material contains proprietary software of Entropic Speech, Inc.
 *  Any reproduction, distribution, or publication without the prior
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing by
 *  Entropic Speech, Inc. must bear the notice
 *
 *      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * Module Name:  sps_compat
 * Written By:  Ajaipal S. Virdy
 * Checked By:  Alan Parker
 * Purpose:  Check ESPS files for compatability
 *
 */

#ifdef SCCS
	static char *sccs_id = "@(#)sps_compat.c	3.5	8/31/95	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/anafea.h>
#include <esps/fea.h>
#include <esps/filt.h>
#include <esps/scbk.h>
#include <esps/sd.h>
#include <esps/spec.h>
#include <esps/unix.h>

void
check_sd (f1h, f2h)
struct header	*f1h;
struct header	*f2h;
{
    if ( f1h->hd.sd->sf != f2h->hd.sd->sf ) {
	Fprintf (stderr,
	"check_sd: fatal error: sf values are different.\n");
	exit (1);
    }
    if ( f1h->hd.sd->src_sf != f2h->hd.sd->src_sf ) {
	Fprintf (stderr,
	"check_sd: fatal error: src_sf values are different.\n");
	exit (1);
    }
}

void
add_sd_comment (f1h, f2h)
struct header	*f1h;
struct header	*f2h;
{
    if ( f1h->hd.sd->dcrem != f2h->hd.sd->dcrem ) {
	Fprintf (stderr,
	"check_sd: warning, dcrem values are different.\n");
	add_comment (f2h, "\tcopysps: warning, dcrem values different.\n");
    }
    if ( f1h->hd.sd->q_method != f2h->hd.sd->q_method ) {
	Fprintf (stderr,
	"check_sd: warning, q_method values are different.\n");
	add_comment (f2h, "\tcopysps: warning, q_method values different.\n");
    }
}

void
check_spec (f1h, f2h)
struct header	*f1h;
struct header	*f2h;
{
    if ( f1h->hd.spec->spec_type != f2h->hd.spec->spec_type ) {
	Fprintf (stderr,
	"check_spec: spec_type values are different.\n");
	exit (1);
    }
    if ( f1h->hd.spec->freq_format != f2h->hd.spec->freq_format ) {
	Fprintf (stderr,
	"check_spec: freq_format values are different.\n");
	exit (1);
    }
}

void
add_spec_comment (f1h, f2h)
struct header	*f1h;
struct header	*f2h;
{
    if ( f1h->hd.spec->num_freqs != f2h->hd.spec->num_freqs ) {
	Fprintf (stderr,
	"check_spec: warning, num_freqs values are different.\n");
	add_comment (f2h, "\tcopysps: warning, num_freqs values different.\n");
    }
    if ( f1h->hd.spec->sf != f2h->hd.spec->sf ) {
	Fprintf (stderr,
	"check_spec: warning, sf values in headers are different.\n");
	add_comment (f2h, "\tcopysps: warning, sf values different.\n");
    }
    if ( f1h->hd.spec->win_type != f2h->hd.spec->win_type ) {
	Fprintf (stderr,
	"check_spec: warning, win_type values in headers are different.\n");
	add_comment (f2h, "\tcopysps: warning, win_type values different.\n");
    }
    if ( f1h->hd.spec->contin != f2h->hd.spec->contin ) {
	Fprintf (stderr,
	"check_spec: warning, contin values in headers are different.\n");
	add_comment (f2h, "\tcopysps: warning, contin values different.\n");
    }
}

void
check_filt (f1h, f2h)
struct header	*f1h;
struct header	*f2h;
{
    if ( f1h->hd.filt->max_num != f2h->hd.filt->max_num ) {
	Fprintf (stderr,
	"check_filt: max_num in headers are different.\n");
	exit (1);
    }
    if ( f1h->hd.filt->max_den != f2h->hd.filt->max_den ) {
	Fprintf (stderr,
	"check_filt: max_den in headers are different.\n");
	exit (1);
    }
}

void
add_filt_comment (f1h, f2h)
struct header	*f1h;
struct header	*f2h;
{
    if ( f1h->hd.filt->max_num != f2h->hd.filt->max_num ) {
	Fprintf (stderr,
	"check_filt: max_num in headers are different.\n");
	exit (1);
    }
    if ( f1h->hd.filt->max_den != f2h->hd.filt->max_den ) {
	Fprintf (stderr,
	"check_filt: max_den in headers are different.\n");
	exit (1);
    }
}
