/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988, 1989 AT&T	*/
/*	  and Entropic Speech, Inc.		*/
/*	  All Rights Reserved.			*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC SPEECH, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* scale.c */
/* scale and translate data. */

#ifndef lint
static char *sccs_id = "@(#)scale.c	1.7	9/26/95	ATT/ERL";
#endif

#include <Objects.h>

main(argc, argv)
     int argc;
     char **argv;
{
  Signal *s;
  int in;
  double range, offset, smax, smin;
  register double fact, trans, out, rnd=0.5;
  register int i, j, k;
  char mess[200];
  
  if(argc < 4)
    printf("Usage: %s [-rn] [-tm] <input> <output>,\n  where, n is the desired range of the output data; m is the offset.\n",argv[0]);
  range = 0.0;
  offset = 0.0;
  for(in=1; in < argc; in++) {
    if(argv[in][0] == '-') {
      switch(argv[in][1]) {
      case 'r':
	range = atof(&(argv[in][2]));
	break;
      case 't':
	offset = atof(&(argv[in][2]));
	break;
      default:
	printf("Switch %s is not recognized.\n",argv[in]);
	exit(-1);
      }
    } else {

      if((argc - in) < 2) {
	printf("Usage: %s -rn -tm <input> <output>\n", argv[0]);
	exit(-1);
      }
      if((s = (Signal*)get_signal(argv[in],0.0,-1.0, NULL))) {
	close_sig_file(s);
 	get_maxmin(s);
	for(i=1, smin = *s->smin, smax = *s->smax; i < s->dim; i++) {
	  if(s->smin[i] < smin) smin = s->smin[i];
	  if(s->smax[i] > smax) smax = s->smax[i];
	}
	if(range != 0.0)
	fact = range/(smax-smin);
	else
	  fact = 1.0;
	if(offset != 0.0)
	trans = offset - (fact * smin);
	else
	  trans = 0.0;

	switch((s->type & VECTOR_SIGNALS)) {
	case P_CHARS:
	case P_UCHARS:
	  {
	    register char  *p, t;
	    register char **spp;
	    
	    spp = (char**)(s->data);
	    for(i=0; i < s->dim; i++) {
	      for(j=0, p = spp[i], k = s->buff_size; j++ < k; ) {
	        out = ((*p * fact) + trans);
		*p++ = (out > 0.0)? out + rnd : out - rnd;
	      }
	    }
	  }
	  break;
	case P_SHORTS:
	  {
	    register short  *p, t;
	    register short **spp;
	    
	    spp = (short**)(s->data);
	    for(i=0; i < s->dim; i++) {
	      for(j=0, p = spp[i], k = s->buff_size; j++ < k; ) {
	        out = ((*p * fact) + trans);
		*p++ = (out > 0.0)? out + rnd : out - rnd;
	      }
	    }
	  }
	  break;
	case P_FLOATS:
	  {
	    register float *p, t;
	    register float **spp;
	    
	    spp = (float**)(s->data);
	    for(i=0; i < s->dim; i++) {
	      for(j=0, p = spp[i], k = s->buff_size; j++ < k; ) {
		*p++ = ((*p * fact) + trans);
	      }
	    }
	  }
	  break;
	case P_DOUBLES:
	  {
	    register double *p, t;
	    register double **spp;
	    
	    spp = (double**)(s->data);
	    for(i=0; i < s->dim; i++) {
	      for(j=0, p = spp[i], k = s->buff_size; j++ < k; ) {
		*p++ = ((*p * fact) + trans);
	      }
	    }
	  }
	  break;
	default:
	  printf("Signals of type 0x%x are not supported\n",s->type);
	  exit(-1);
	}
	sprintf(mess,"scale: range %f offset %f signal %s",range,offset,s->name);
	rename_signal(s,argv[in+1]);
	*s->smax = smax*fact + trans;
	*s->smin = smin*fact + trans;
	head_printf(s->header,"operation",mess);
	head_printf(s->header,"maximum",s->smax);
	head_printf(s->header,"minimum",s->smin);
	put_signal(s);
	exit(0);
      }
    }
  }
}

	    
  
