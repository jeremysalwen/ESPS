/*	Copyright (c) 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* maketable.c */
/* Makes sine and cosine tables for fft.c (These should be put in a source
   file called "theftable.c"). */

#ifndef lint
static char *sccs_id = "@(#)mkftable.c	1.2 12/8/89 ATT, ESI";
#endif

#include	<stdio.h>
#include	<math.h>
#define PI 3.1415927

/*-----------------------------------------------------------------------*/
main(argc,argv)
int	argc;
char	**argv;
{
int l = 5, atoi();
register int	j1, j2, count, lmm;
double	c, s,  t1, t2;
int	np, lmx, lo, lix, lm, li, j, nv2, i, k, im, jm;
double	scl, arg;

if(argc > 1) l = atoi(argv[1])-1;

  for ( np=1, i=1; i <= l; i++) np *= 2;
printf("\n int fft_ftablesize = %d;\n\n",np/2);
printf("\n/* sine table for arguments from 0 to PI; increments of PI/%d  */\n",np/2);
printf("\nstatic float	fsine[%d] = {\n    ",np/2);
  scl = ((double)(PI)*2.0);
    for (lmx=np, lo=0; lo < 1; lo++) {
	lix = lmx;
	lmx /= 2;
	lmm = lmx -1;
	for (count = 0,arg=0.0, lm=0; lm<lmx; lm++ ) {
	    arg = (scl * ((double)lm))/np;
	    s = sin(arg);
	    printf("%.16f",s);
	    if(	lm < lmm) printf(", ");
	    if(++count >= 4) {
		count = 0;
		printf("\n    ");
	    }
	    
	}
    }
printf("\n};\n");
printf("\n/* cosine table for arguments from 0 to PI; increments of PI/%d  */\n",np/2);
printf("\nstatic float	fcosine[%d] = {\n    ",np/2);
    for (lmx=np, lo=0; lo < 1; lo++) {
	lix = lmx;
	lmx /= 2;
	lmm = lmx -1;
	for (count = 0,arg=0.0, lm=0; lm<lmx; lm++ ) {
	    arg = (scl * ((double)lm))/np;
	    c = cos(arg);
	    printf("%.16f",c);
	    if(	lm < lmm) printf(", ");
	    if(++count >= 4) {
		count = 0;
		printf("\n    ");
	    }
	}
    }
printf("\n};\n");
}


