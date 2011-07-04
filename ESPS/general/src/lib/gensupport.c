/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
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
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description:
 * general record support routines.
 */

static char *sccs_id = "@(#)gensupport.c	1.15	5/1/98	ESI/ERL";


#include <stdio.h>
#include <esps/esps.h>
#include <esps/esignal_fea.h>
#include <esps/pc_wav.h>

int get_gen_recd (dbuf, tagp, hd, file)
double *dbuf;
long *tagp;
struct header *hd;
FILE *file;

{
int j,k,n,count=0, edr, machine;

  spsassert(dbuf != NULL, "get_gen_recd: dbuf is NULL");
  spsassert(tagp != NULL, "get_gen_recd: tagp is NULL");
  spsassert(hd != NULL, "get_gen_recd: hd is NULL");
  spsassert(file != NULL, "get_gen_recd: file is NULL");

  edr = hd->common.edr;
  machine = hd->common.machine_code;

/*
 * This function doesn't currently handle Esignal files or field_order
 * FEA files.
 */

  if (get_esignal_hdr(hd))
  {
	fprintf(stderr, "get_gen_recd: cannot handle Esignal formats.\n");
	exit(1);
  }

  if (get_pc_wav_hdr(hd))
  {
	fprintf(stderr, "get_gen_recd: cannot handle PC WAVE format.\n");
	exit(1);
  }
  
#ifndef NOPAD
  if (hd->common.type == FT_FEA && hd->hd.fea->field_order == YES) {
	fprintf(stderr,
	 "get_gen_recd: cannot handle field_order feature files.\n");
	exit(1);
  }
#endif
	
  if (hd->common.tag == YES) {
	if(!miio_get_long(tagp,1,edr,machine,file)) return(EOF);
  }
  else
	*tagp = 0;
  if (hd->common.ndouble > 0) {
	if(!miio_get_double(dbuf,(int)hd->common.ndouble,edr,machine,file)) 
	  return(EOF);
	dbuf += hd->common.ndouble;
	count += hd->common.ndouble;
  }
  if (hd->common.nfloat > 0) {
	k = hd->common.nfloat;
	n = BUFSIZ/sizeof(float);
	while (k > 0) {
	  float data[BUFSIZ/sizeof (float)], *p = data;
	  if (k < n) n = k;
	  if(!miio_get_float(data,n,edr,machine,file)) return(EOF);
	  for (j = 0; j < n; j++) *dbuf++ = *p++;
	  k -= n;
	}
  	count += hd->common.nfloat;
  }
  if (hd->common.nlong > 0) {
	k = hd->common.nlong;
	n = BUFSIZ/sizeof(long);
	while (k > 0) {
	  long data[BUFSIZ/sizeof (long)], *p = data;
	  if (k < n) n = k;
	  if(!miio_get_long(data,n,edr,machine,file)) return(EOF);
	  for (j = 0; j < n; j++) *dbuf++ = *p++;
	  k -= n;
	}
	count += hd->common.nlong;
  }
  if (hd->common.nshort > 0) {
	k = hd->common.nshort;
	n = BUFSIZ/sizeof(short);
	while (k > 0) {
	  short data[BUFSIZ/sizeof (short)], *p = data;
	  if (k < n) n = k;
	  if(!miio_get_short(data,n,edr,machine,file)) return(EOF);
	  for (j = 0; j < n; j++) *dbuf++ = *p++;
	  k -= n;
	}
	count += hd->common.nshort;
  }
  if (hd->common.nchar > 0) {
	k = hd->common.nchar;
	n = BUFSIZ/sizeof(char);
	while (k > 0) {
	  char data[BUFSIZ/sizeof (char)], *p = data;
	  if (k < n) n = k;
	  if(!miio_get_char(data,n,edr,machine,file)) return(EOF);
	  for (j = 0; j < n; j++) *dbuf++ = *p++;
	  k -= n;
	}
	count += hd->common.nchar;
  }
  return(count);
}


/* get_rec_len returns the number of 'elements' in a record.  This
 * is the number of items (elements) in a record, as opposed to the number of 
 * bytes that a record takes (that's called size).    
 */

long get_rec_len(hd)
struct header *hd;

{
   spsassert(hd != NULL, "get_rec_len: hd is NULL");
   return(hd->common.ndouble+hd->common.nfloat+hd->common.nlong+ 
          hd->common.nshort+hd->common.nchar);
}
