/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1993 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description: displays FEA_SD file from an array of data
 *
 */

static char *sccs_id = "@(#)wavesshow.c	1.2	6/21/93	ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/feasd.h>

/* wavesshow():
*/

extern int debug_level;
char *e_temp_name();
char *ProgName = "waves_show_sd";
char *Version = "1.0";
char *Date = "6/17/93";

int waves_show_sd(samp_freq, start_time, data, length, type, num_channels, setting, name)
     double samp_freq, *start_time;
     char *data;
     long length;
     int type;
     int num_channels;
     char *setting;
     char *name;
{
  char *oname;
  FILE *osdfile;
  struct header *ohd;
  struct feasd *osd_rec;
  char buffer[100];
  int x, y, height, width;

  if(samp_freq <=0) samp_freq = 0;
  if(!start_time){
    int i;
    start_time = (double *) malloc(sizeof(double) * num_channels);
    for(i=0; i<num_channels; i++) start_time[i] = 0;
  }
  if(!data || length<=0){
    fprintf(stderr,"waves_show_sd(): NULL data argument or 0 length.\n");
    return(-1);
  }
  if(type != DOUBLE && type != FLOAT && type != LONG && type != SHORT
     && type != BYTE && type != DOUBLE_CPLX && type != FLOAT_CPLX &&
     type != LONG_CPLX && type != SHORT_CPLX && type != BYTE_CPLX ){
    fprintf(stderr,"waves_show_sd(): Unknown data type argument.\n");
    return(-1);
  }
  if(num_channels <= 0){
    fprintf(stderr,"waves_show_sd(): channel specification must be >= 1.\n");
    return(-1);
  }
  if(!name) oname = e_temp_name("wavesXXXXXX");
  if(name) oname = name;
  if( !setting )
    sprintf(buffer,"make file %s ", oname);
  else
    sprintf(buffer,"make file %s %s", oname, setting);

  oname = eopen("waves_show_sd", oname, "w", NONE, NONE, &ohd, &osdfile);
  ohd = new_header(FT_FEA);
  (void) strcpy (ohd->common.prog, ProgName);
  (void) strcpy (ohd->common.vers, Version);
  (void) strcpy (ohd->common.progdate, Date);
  ohd->common.tag = NO;

  (void) add_genhd_d("record_freq", &samp_freq, 1, ohd);
  (void) add_genhd_d("start_time", start_time, 1, ohd);

  init_feasd_hd( ohd, type, num_channels, start_time, NO, samp_freq);
  (void) write_header (ohd, osdfile);

  if(num_channels == 1)
    osd_rec = allo_feasd_recs(ohd, type, length, (char*) data, NO);
  else
    osd_rec =allo_feasd_recs(ohd, type, length, data, YES);
  
  put_feasd_recs(osd_rec, 0L, length, ohd, osdfile);

  (void) fclose(osdfile);

  return(send_xwaves2(NULL, 0, buffer, 0));
}
