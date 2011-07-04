/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/
static char *sccs_id = "@(#)dplympy.c	1.2	9/26/95	ATT/ERL";

/* ply mpy of c = a 8 b
	ln is dim of pol including the 0 order	*/
static double *pa,*pb,*pc,*pcl;
static int i,j,k;
dplympy(a,la,b,lb,c,lc) double *a,*b,*c; int *la,*lb,*lc;{
	for(pc=c,pcl=c+ *lc;pc<pcl;*pc++ = 0.);
	for(pa=a,i=0;i<*la;i++,pa++){
		for(pb=b,j=0;j<*lb;j++,pb++){
			k= i + j;
			if(k<*lc)
				c[k] += *pa * *pb;
		}
	}
}
