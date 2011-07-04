/* lpc_poles.c */

/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin, ATT
 * Checked by:
 * Revised by:  John Shore ERL
 */

static char *sccs_id = "@(#)lpc_poles.c	1.9 10/7/96 ERL/ATT";

/* computation and I/O routines for dealing with LPC poles */

#include <Objects.h>
#ifndef LINUX_OR_MAC
#include <sgtty.h>
#endif
#include "tracks.h"

#define MAXORDER 30

extern int debug;

int read_poles(), write_poles();

Signal *dpfund(), *downsample(), *highpass(), *get_signal(),
       *dpform(), *new_signal();


char *localloc();

/*************************************************************************/
Signal *lpc_poles(sp,wdur,frame_int,lpc_ord,preemp,lpc_type,w_type)
     Signal *sp;
     int lpc_ord, lpc_type, w_type;
     double wdur, frame_int, preemp;
{
  int i, j, k, size, step, nform, init, nfrm;
  POLE **pole;
  double lpc_stabl = 70.0, energy, lpca[MAXORDER], normerr,
         *bap=NULL, *frp=NULL, *rhp=NULL, integerize();
  short *datap;
  char *np, temp[200];
  Signal *lp;

  if(lpc_type == 1) { /* force "standard" stabilized covariance (ala bsa) */
    wdur = 0.025;
    preemp = exp(-62.831853 * 90. / sp->freq); /* exp(-1800*pi*T) */
  }
  if((lpc_ord > MAXORDER) || (lpc_ord < 2) || (! ((short**)sp->data)[0]))
    return(NULL);
  np = (char*)new_ext(sp->name,"pole");
  wdur = integerize(wdur,sp->freq);
  frame_int = integerize(frame_int,sp->freq);
  nfrm= 1 + (((((double)sp->buff_size)/sp->freq) - wdur)/(frame_int));
  if((lp = new_signal(np,SIG_UNKNOWN,dup_header(sp->header),NULL,nfrm,1.0/frame_int,1))) {
/*    lp->type =  SIG_LPC_POLES | P_SHORTS | VAR_REC_SIGNALS; (for binary)*/
 lp->type =  SIG_LPC_POLES | P_DOUBLES | SIG_ASCII | VAR_REC_SIGNALS;
    lp->utils->read_data = read_poles;
    lp->utils->write_data = write_poles;
    lp->start_time = sp->start_time + wdur/2.0;	/* compensate for window width */
    sprintf(temp,"lpc_poles: lpc_ord %d preemp %6.3f lpc_stabl %4.1f wdur %f w_type %d lpc_type %d signal %s",
      lpc_ord,preemp,lpc_stabl,wdur,w_type,lpc_type,sp->name);
    i = lp->version + 1;
    head_printf(lp->header, "version", &i);
    head_printf(lp->header, "type_code", &(lp->type));
    head_printf(lp->header, "dimensions", &(lp->dim));
    head_printf(lp->header,"operation",temp);
    head_ident(lp->header,"POLE_structure");
    head_printf(lp->header,"samples",&nfrm);
    head_printf(lp->header,"frequency",&(lp->freq));
    head_printf(lp->header,"start_time",&(lp->start_time));
    head_printf(lp->header,"time", get_date());
    if(debug & 4)
      printf("In lpc_poles(): lp->buff_size:%d  wdur:%f  frame_int:%f\n",
	 lp->buff_size,wdur,frame_int);
  if(lp->buff_size >= 1) {
    size = .5 + (wdur * sp->freq);
    step = .5 + (frame_int * sp->freq);
    pole = (POLE**)localloc(lp->buff_size * sizeof(POLE*));
    for(j=0, init=TRUE, datap=((short**)sp->data)[0]; j < lp->buff_size;
	j++, datap += step){
      pole[j] = (POLE*)localloc(sizeof(POLE));
      pole[j]->freq = frp = (double*)localloc(sizeof(double)*lpc_ord);
      pole[j]->band = bap = (double*)localloc(sizeof(double)*lpc_ord);
      switch(lpc_type) {
      case 0:
	if(! lpc(lpc_ord,lpc_stabl,size,datap,lpca,rhp,NULL,&normerr,
		 &energy, preemp, w_type)){
	  notify("Problems with lpc in lpc_poles()",0,0);
	  break;
	}
	break;
      case 1:
	if(! lpcbsa(lpc_ord,lpc_stabl,size,datap,lpca,rhp,NULL,&normerr,
		    &energy, preemp)){
          notify("Problems with lpc in lpc_poles()",0,0);
	  break;
	}
	break;
      case 2:
	{
	  int Ord = lpc_ord;
	  double alpha, r0;

	  w_covar(datap, &Ord, size, 0, lpca, &alpha, &r0, preemp, 0);
	  if((Ord != lpc_ord) || (alpha <= 0.0))
	    printf("Problems with covar(); alpha:%f  Ord:%d\n",alpha,Ord);
	  energy = sqrt(r0/(size-Ord));
	}
	break;
      }
      pole[j]->change = 0.0;
       /* don't waste time on low energy frames */
       if((pole[j]->rms = energy) > 1.0){
	 formant(lpc_ord, sp->freq, lpca, &nform, frp, bap, init);
	 pole[j]->npoles = nform;
	 init=FALSE;		/* use old poles to start next search */
       } else {			/* write out no pole frequencies */
	 pole[j]->npoles = 0;
	 init = TRUE;		/* restart root search in a neutral zone */
       }
       if(debug & 4) {
	 printf("\nfr:%4d np:%4d rms:%7.0f  ",j,pole[j]->npoles,pole[j]->rms);
	 for(k=0; k<pole[j]->npoles; k++)
	   printf(" %7.1f",pole[j]->freq[k]);
	 printf("\n                   ");
	 for(k=0; k<pole[j]->npoles; k++)
	   printf(" %7.1f",pole[j]->band[k]);
	 printf("\n");
       }
     } /* end LPC pole computation for all lp->buff_size frames */
     lp->data = (caddr_t)pole;
     return(lp);
   } else
     printf("Bad buffer size in lpc_poles()\n");
   } else
     printf("Can't create a new Signal in lpc_poles()\n");
   return(NULL);
}

/**********************************************************************/
double frand()
{
  return (((double)rand())/2147483648.0);
}
    
/**********************************************************************/
/* a quick and dirty interface to bsa's stabilized covariance LPC */
#define NPM	30	/* max lpc order		*/

lpcbsa(np, lpc_stabl, wind, data, lpc, rho, nul1, nul2, energy, preemp)
     int np, wind;
     short *data;
     double *lpc, *rho, *nul1, *nul2, *energy, lpc_stabl, preemp;
{
  extern int optind;
  extern char *optarg;
  static int nsam, i, skip, rw2, mm, owind=0, wind1;
  static double w[1000];
  double rc[NPM],phi[NPM*NPM],shi[NPM],sig[1000];
  double xl = .09, fham, amax;
  register double *psp1, *psp2, *psp3, *pspl;
  short *pref1, *prefl;


  if(owind != wind) {		/* need to compute a new window? */
    fham = 6.28318506 / wind;
    for(psp1=w,i=0;i<wind;i++,psp1++)
      *psp1 = .54 - .46 * cos(i * fham);
    owind = wind;
  }
  wind += np + 1;
  wind1 = wind-1;

  for(psp3=sig,pspl=sig+wind; psp3 < pspl; )
    *psp3++ = (double)(*data++) + .016 * frand() - .008;
  for(psp3=sig+1,pspl=sig+wind;psp3<pspl;psp3++)
    *(psp3-1) = *psp3 - preemp * *(psp3-1);
  for(amax = 0.,psp3=sig+np,pspl=sig+wind1;psp3<pspl;psp3++)
    amax += *psp3 * *psp3;
  *energy = sqrt(amax / (double)owind);
  amax = 1.0/(*energy);
	
  for(psp3=sig,pspl=sig+wind1;psp3<pspl;psp3++)
    *psp3 *= amax;
  if((mm=dlpcwtd(sig,&wind1,lpc,&np,rc,phi,shi,&xl,w))!=np) {
    printf("LPCWTD error mm<np %d %d\n",mm,np);
    return(FALSE);
  }
  return(TRUE);
}

