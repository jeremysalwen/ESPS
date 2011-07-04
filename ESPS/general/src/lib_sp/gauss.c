/*
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *              "Copyright 1986 Entropic Speech, Inc."; All Rights Reserved
 *
 * Program:	gauss
 *
 * Written by:  John Shore based on a routine by Shankar Narayan
 * Checked by:  Alan Parker
 *
 * This program generates zero-mean,  unit-variance Gaussian distributed
 * floating point values.  
 */

#ifndef lint
static char *sccs_id = "%W%	%G% ESI";
#endif
#define BIGRAND 2147483647.0	/*maximum value returned by random()*/
/*
 * system library routines
 */
long random();
double sqrt(),log();

float
gauss()
/*
 * gauss() returns zero-mean, unit-variance Gaussian distributed
 * floating point values.  Note that it actually computes two at a time 
 * but returns them one at a time.  Note also that no assumptions are made
 * about random seeds.  Calling programs should set the seed via srandom().
 */
{
	static int flag = 1;
	double s,v1,v2;
	static double x1,x2;
	if (flag == 2) { flag = 1; return(x1);}
	do 
	    {
#ifdef OS5
	     v1 = 2.0*((float)rand() / BIGRAND - .5);
     	     v2 = 2.0*((float)rand() / BIGRAND - .5);
#else
	     v1 = 2.0*((float)random() / BIGRAND - .5);
     	     v2 = 2.0*((float)random() / BIGRAND - .5);
#endif
	     s = v1*v1+v2*v2;
        }
	while (s > 1.0);
	x1 = sqrt(-2.0*log(s)/s);
	x2 = v2*x1;
	x1 = v1*x1;
	flag = 2;
	return (x2);
}
