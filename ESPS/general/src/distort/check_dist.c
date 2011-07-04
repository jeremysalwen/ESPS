/* check_dist - check various ESPS files for compatability
 *
 *  This material contains proprietary software of Entropic Speech, Inc.
 *  Any reproduction, distribution, or publication without the prior
 *  written permission of Entropic Speech, Inc. is strictly prohibited.
 *  Any public distribution of copies of this work authorized in writing by
 *  Entropic Speech, Inc. must bear the notice
 *
 *      "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
 *
 * Module Name:  check_dist
 * 
 * Written By:  Ajaipal S. Virdy
 *
 *
 * Purpose:  Check ESPS files for compatability
 *
 *
 */

#ifdef SCCS
	static char *sccs_id = "%W%	%G%	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/anafea.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/feaspec.h>

#ifdef ESI
#include <esps/ana.h>
#include <esps/pitch.h>
#endif

#include "distort.h"

#ifdef ESI
void
check_ana (f1h, f2h)
struct header	*f1h;
struct header	*f2h;
{

   if (debug_level > 0)
      (void) fprintf (stderr,
      "\n");

    if ( f1h->hd.ana->rms_fullscale != f2h->hd.ana->rms_fullscale ) 
	(void) fprintf (stderr,
	"check_ana: warning, rms_fullscale values are different.\n");

    if ( f1h->hd.ana->sinc_flg != f2h->hd.ana->sinc_flg ) 
	(void) fprintf (stderr,
	"check_ana: warning, sinc_flg values are different.\n");

    if ( f1h->hd.ana->dcrem != f2h->hd.ana->dcrem ) 
	(void) fprintf (stderr,
	"check_ana: warning, dcrem values are different.\n");

   if (debug_level > 0)
      (void) fprintf (stderr, "\n");

}

void
check_pit (f1h, f2h)
struct header	*f1h;
struct header	*f2h;
{

   if (debug_level > 0)
      (void) fprintf (stderr,
      "\n");

    if ( f1h->hd.pit->src_sf != f2h->hd.pit->src_sf ) {
	(void) fprintf (stderr,
	"check_pit: fatal error: src_sf values are different.\n");
	exit (1);
    }

    if ( f1h->hd.pit->dcrem != f2h->hd.pit->dcrem ) 
	(void) fprintf (stderr,
	"check_pit: warning, dcrem values are different.\n");

    if ( f1h->hd.pit->p_method != f2h->hd.pit->p_method ) 
	(void) fprintf (stderr,
	"check_pit: warning, p_method values are different.\n");

   if (debug_level > 0)
      (void) fprintf (stderr, "\n");

}
#endif

void
check_anafea (f1h, f2h)
struct header	*f1h;
struct header	*f2h;
{

   char    *get_genhd();

   if (debug_level > 0)
      (void) fprintf (stderr,
      "\n");

    if (*(float *)get_genhd("src_sf",f1h) != *(float *)get_genhd("src_sf",f2h))
    {
	(void) fprintf (stderr,
	"distort: check_anafea: source sampling frequencies (src_sf) differ in input files.\n");
	exit (1);
    }

   if (debug_level > 0)
      (void) fprintf (stderr, "\n");
}

void
check_sd (f1h, f2h)
struct header	*f1h;
struct header	*f2h;
{

   if (debug_level > 0)
      (void) fprintf (stderr,
      "\n");

    if ( get_genhd_val("record_freq", f1h, (double) -1.) != get_genhd_val("record_freq", f2h, (double) -2) ) {
	(void) fprintf (stderr,
	"check_sd: fatal error: sf values are different.\n");
	exit (1);
    }

    if ( get_genhd_val("src_sf", f1h, (double)-1) != get_genhd_val("src_sf", f2h, (double)-1) ) {
	(void) fprintf (stderr,
	"check_sd: warning: src_sf values are different.\n");
    }

    if ( get_genhd_val("dcrem", f1h, (double)0) != get_genhd_val("dcrem", f2h, (double)0) ) 
	(void) fprintf (stderr,
	"check_sd: warning: dcrem values are different.\n");

    if ( get_genhd_val("q_method", f1h, (double)0) != get_genhd_val("q_method", f2h, (double)0) ) 
	(void) fprintf (stderr,
	"check_sd: warning: q_method values are different.\n");

   if (debug_level > 0)
      (void) fprintf (stderr, "\n");

}

void
check_spec (f1h, f2h)
struct header	*f1h;
struct header	*f2h;
{

   if (debug_level > 0)
      (void) fprintf (stderr,
      "\n");

    if ( get_genhd_val("num_freqs",f1h,(double)0.) != 
	           get_genhd_val("num_freqs", f2h, (double)0.) ) {
	(void) fprintf (stderr,
	"check_spec: warning, num_freqs values are different.\n");
    }

    if ( get_genhd_val("sf", f1h, (double)0.) != 
	           get_genhd_val("sf",f2h, (double)0.) ) {
	(void) fprintf (stderr,
	"check_spec: sf values are different.\n");
	exit (1);
    }

    if ( *get_genhd_s("spec_type", f1h) != 
	          *get_genhd_s("spec_type",f2h) ) {
	(void) fprintf (stderr,
	"check_spec: spec_type values are different.\n");
	exit (1);
    }

    if ( *get_genhd_s("contin", f1h) != 
	          *get_genhd_s("contin", f2h) ) {
	(void) fprintf (stderr,
	"check_spec: contin values are different.\n");
	exit (1);
    }

    if ( *get_genhd_s("freq_format", f1h) != 
	          *get_genhd_s("freq_format", f2h )) {
	(void) fprintf (stderr,
	"check_spec: freq_format values are different.\n");
	exit (1);
    }

   if (debug_level > 0)
      (void) fprintf (stderr, "\n");

}
