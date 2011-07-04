/* SCBK file type support routines

  Alan Parker, Entropic Processing, Inc.
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
 	
*/
#ifdef SCCS
	static char *sccs_id = "@(#)scbksupport.c	1.1 10/19/93 ESI";
#endif
#include <stdio.h>
#include <esps/esps.h>
#include <esps/scbk.h>

#ifndef DEC_ALPHA
char *calloc();
#endif

void
scbk_ue_check(func,hd)
char *func;
struct header *hd;

{
#ifdef UE
  if(hd->common.type != FT_SCBK) {
	Fprintf(stderr,"%s: called with non-SCBK header.\n",func);
	exit(1);
  }
  if(hd->hd.scbk->num_cdwds < 1) {
	Fprintf(stderr,"%s: num_cdwds (in header) is less than one.\n",func);
	exit(1);
  }
  return;
#endif
}

/* allocate memory for scbk_data for use with file described by header
 * hd
 */

struct scbk_data *
allo_scbk_rec(hd)
struct header *hd;

{
  struct scbk_data *p;
  (void) scbk_ue_check("allo_scbk_rec",hd);
  p = (struct scbk_data *)calloc(1,sizeof *p);
  p->cdwd_dist = (float *)calloc(hd->hd.scbk->num_cdwds,sizeof (float));
  p->final_pop = (long *)calloc(hd->hd.scbk->num_cdwds,sizeof (long));
  p->qtable = (struct qtable_entry *)calloc(hd->hd.scbk->num_cdwds,
	sizeof (*p->qtable));

  return(p);
}

int
get_scbk_rec(p,hd,file)
struct scbk_data *p;
struct header *hd;
FILE *file;

{
  int num_cdwds,i, edr, machine;
  spsassert(p, "get_scbk_rec: data pointer is NULL");
  spsassert(hd,"get_scbk_rec: header pointer is NULL");
  spsassert(file, "get_scbk_rec: file pointer is NULL");

  edr = hd->common.edr;
  machine = hd->common.machine_code;

  num_cdwds = hd->hd.scbk->num_cdwds;
  (void) scbk_ue_check("get_scbk_rec",hd);
  if(!miio_get_float(&p->final_dist, 1, edr, machine, file) ||
     !miio_get_float(p->cdwd_dist, num_cdwds, edr, machine, file)) return (EOF);


  for (i = 0; i < num_cdwds; i++)
      if (!miio_get_float(&p->qtable[i].enc, 1, edr, machine, file))
	 return (EOF);

  for (i = 0; i < num_cdwds; i++)
      if (!miio_get_float(&p->qtable[i].dec, 1, edr, machine, file))
	 return (EOF);

  if(!miio_get_long(p->final_pop, num_cdwds, edr, machine, file)) return(EOF);
  for(i=0;i<num_cdwds;i++) 
     if(!miio_get_short(&p->qtable[i].code, 1, edr, machine, file)) return(EOF);
  return(1);
}

void
put_scbk_rec(p,hd,file)
struct scbk_data *p;
struct header *hd;
FILE *file;

{
  int num_cdwds,i;

  num_cdwds = hd->hd.scbk->num_cdwds;
  (void) scbk_ue_check("put_scbk_rec",hd);
  if(!miio_put_float(&p->final_dist,1,hd->common.edr,file) ||
     !miio_put_float(p->cdwd_dist,num_cdwds,hd->common.edr,file)) goto error;

  for (i = 0; i < num_cdwds; i++)
      if (!miio_put_float(&p->qtable[i].enc, 1, hd->common.edr, file))
	 goto error;

  for (i = 0; i < num_cdwds; i++)
      if (!miio_put_float(&p->qtable[i].dec, 1, hd->common.edr, file))
	 goto error;

  if(!miio_put_long(p->final_pop,num_cdwds,hd->common.edr,file)) goto error;
  for(i=0;i<num_cdwds;i++) 
     if(!miio_put_short(&p->qtable[i].code,1,hd->common.edr,file)) goto error;
  return;
error:
  Fprintf(stderr,"put_scbk_rec: i/o error.\n");
  exit(1);
}

/* print a scbk record; useful for debugging and used by psps.
 */
void
print_scbk_rec(p,hd,file)
struct scbk_data *p;
struct header *hd;
FILE *file;

{
  float last;
  int i=0;
  (void) scbk_ue_check("print_scbk_rec",hd);
  Fprintf(file,"final_dist: %g\n",p->final_dist);
  Fprintf(file,"index     cdwd_dist final_pop       qtable\n");
  Fprintf(file,"                                    [from]            to      decode code\n");
  Fprintf(file,"%5d %13.5f %9ld                %11.5f %11.5f 0x%x\n",
   i,p->cdwd_dist[0],p->final_pop[0],p->qtable[0].enc,
   p->qtable[0].dec,p->qtable[0].code);
  for(i=1; i < hd->hd.scbk->num_cdwds; i++) {
	last = p->qtable[i-1].enc;
	Fprintf(file,"%5d %13.5f %9ld  %11.5f   %11.5f %11.5f 0x%x\n",
	 i,p->cdwd_dist[i],p->final_pop[i],last,p->qtable[i].enc,
	 p->qtable[i].dec,p->qtable[i].code);
  }
}


