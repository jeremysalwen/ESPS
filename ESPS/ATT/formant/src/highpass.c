/*	Copyright (c) 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/*    highpass.c */

#ifndef lint
	static char *sccs_id = "@(#)highpass.c	1.2 1/26/93		ATT";
#endif

#include <Objects.h>


#define PI 3.1415927


main(ac,av)
int ac;
char **av;
{
  double sum, mean;
  register int size, step, i, j, k, l;
  int in, dimo, osamp, ord = 12, lnorm=1, *ds, *pi, *pi2,  nwind, nbk;
  register short *p, *p2;
  char mess[200], *cp;
  Signal *s, *so;

  if(ac < 3) {
    printf("Usage: %s  <input> <output>\n",av[0]);
    exit(-1);
  }
  for(in=1; in < ac; in ++ ) {

    if(av[in][0] == '-') {
      switch(av[in][1]) {
      default:
	printf("Unknown option %s\n",av[in]);
	exit(-1);
      }
    } else {
      if((s=get_signal(av[in],0.0,-1.0,NULL)) &&
	 ((s->type&VECTOR_SIGNALS)==P_SHORTS) ) {
	   highpass(s,av[in+1]);
	   if(!put_signal(s)) {
	     printf("Couldn't put_signal(%s); signal not saved\n",s->name);
	     exit(-1);
	   }
	   exit(0);
	 } else
	   printf("Problems getting signal %s\n",av[in]);
    }
  }
}

/*      ----------------------------------------------------------      */
highpass(s,newname)
     Signal *s;
     char *newname;
{

  short *datain, *dataout;
  static short *lcf;
  static int len = 0;
  short **dpp;
  double scale, fn;
  register int i;
  int it;
  Signal *so;
  char *cp,temp[200];
  Header *h, *dup_header();
  
#define LCSIZ 101
  /* This assumes the sampling frequency is 10kHz and that the FIR
     is a Hamming function of (LCSIZ/10)ms duration. */

  if(s) {
    if(!len) {			/* need to create a Hamming FIR? */
      lcf = (short*)malloc(sizeof(short) * LCSIZ);
      len = 1 + (LCSIZ/2);
      fn = PI * 2.0 / (LCSIZ - 1);
      scale = 32767.0/(.54 * LCSIZ);
      for(i=0; i < len; i++) 
	lcf[i] = scale * (.54 + (.46 * cos(fn * ((double)i))));
    }
    dataout = datain = ((short**)s->data)[0];
    do_fir(datain,s->buff_size,dataout,len,lcf,1); /* in downsample.c */
    it = s->version +1;
    head_printf(s->header,"version",&it);
    sprintf(temp,"highpass: filter Hamming nsamples %d signal %s",
	    LCSIZ,s->name);
    head_printf(s->header,"operation",temp);
    get_maxmin(s);
    head_printf(s->header,"maximum",s->smax);
    head_printf(s->header,"minimum",s->smin);
    head_printf(s->header,"time",get_date());
    rename_signal(s,newname);
    return(TRUE);
  } else
    printf("Null signal passed to highpass()\n");
  return(FALSE);
}

/*      ----------------------------------------------------------      */
char *new_ext(oldn,newex)
char *oldn, *newex;
{
  int	j;
  char *dot, *strcat(), *strrchr(), *strncpy(), *localloc();
  static char newn[256];
  static int len = 0;

  dot = strrchr(oldn,'.');
  if(dot != NULL){
    *dot = 0;
    j = strlen(oldn) + 1;
    *dot = '.';
    
  }
  else j = strlen(oldn);
  if((len = j + strlen(newex)+ 1) > 256) {
    printf("Name too long in new_ext()\n");
    return(NULL);
  }
  strncpy(newn,oldn,j);
  newn[j] = 0;
  return(strcat(newn,newex));
}
  

