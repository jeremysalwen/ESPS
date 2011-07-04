/*

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
   "Copyright 1988 Entropic Speech, Inc."; All Rights Reserved
 				
  update_waves_gen - is used to update waves related generic header
  items in FEA and SD files.


  Alan Parker, ESI, Washington, DC.

*/

#ifndef lint
static char *sccsid = "@(#)updatewave.c	1.4 2/19/90	ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif

#define dNULL (double *)NULL

extern int  debug_level;

void
update_waves_gen (ih, oh, start, step)
struct header  *ih,		/* input file header */
               *oh;		/* output file header */
float   start,			/* starting record */
        step;			/* input records used up per output record */
{
  short   sd_in = 0;		/* flag for input is SD */
  short   fea_in = 0;		/* flag for input is FEA */
  short   sd_out = 0;		/* flag for output is SD */
  short   fea_out = 0;		/* flag for output is FEA */
  int     stime_size;		/* size of start_time generic */
  int     dsize;		/* dummy for genhd_type() call */
  double *start_time;		/* generic start_time */
  double *istart_time;		/* input generic start_time */
  double rec_time_offset;	/* relative time of first record */
  double rec_freq;		/* generic record_freq (sf for sd files) */
  int    i;			/* loop variable */


  sd_in = ih->common.type == FT_SD;
  fea_in = ih->common.type == FT_FEA;
  sd_out = oh->common.type == FT_SD;
  fea_out = oh->common.type == FT_FEA;

  spsassert (ih, "update_waves_gen: ih is NULL");
  spsassert (oh, "update_waves_gen: oh is NULL");
  spsassert (fea_in || sd_in, 
	     "update_waves_gen: input header is not FEA or SD");
  spsassert (fea_out || sd_out, 
	     "update_waves_gen: output header is not FEA or SD");

  if (debug_level && fea_in) {
    if (genhd_type("record_freq", &dsize, ih) == HD_UNDEF)
      Fprintf (stderr,
	       "update_waves_gen: record_freq missing from input file.\n");
    if (genhd_type("start_time", &stime_size, ih) == HD_UNDEF)
      Fprintf (stderr,
	       "update_waves_gen: start_time missing from input file.\n");
  }

  rec_freq = ( fea_in ? 
	      get_genhd_val("record_freq", ih, 1.0) 
	      : ih->hd.sd->sf);

  spsassert (rec_freq >= 0,
	     "update_waves_gen: record frequency <= 0");

  rec_time_offset = (double)(start - 1) / rec_freq;

  if (genhd_type("start_time", &stime_size, ih) == HD_UNDEF) 

      *add_genhd_d ("start_time", dNULL, 1, oh) = rec_time_offset;

  else {
    istart_time = get_genhd_d("start_time", ih); 
    start_time = add_genhd_d("start_time", dNULL, stime_size, oh);
    for (i = 0; i < stime_size; i++)
      start_time[i] = istart_time[i] + rec_time_offset;
  }

  if (fea_out && (step != 0)) 
    *add_genhd_d ("record_freq", dNULL, 1, oh) = rec_freq / (double)step;

}



