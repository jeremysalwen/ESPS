
/*********************************************************
*
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 				
*
*  Module Name: itopod.c
*
*  Written By:   Berny Fraenkel
*
*  Modified for ESPS 3.0 by David Burton
*  Checked by: 
*

				ITOPOD

SOLUTION OF THE EQUATION   toep.lft_hd = rgt_hd ,FOR lft_hd ,
	WHERE toep IS TOEPLITZ OF ODD DIMENSION, AND rgt_hd IS
	SYMMETRIC
toep IS AN ODD DIMENSION TOEPLITZ MATRICE t_sizxt_siz : toep(0)...toep(t_siz-1)
where t_siz = 2*size-1
rgt_hd IS A SYMMETRIC VECTOR OF DIMENSION 2*size-1 
		rgt_hd(size-1)... rgt_hd(1) rgt_hd(0) rgt_hd(1)...rgt_hd(size-1)
SO ARE lft_hd AND W 	W=rgt_hd - toep.lft_hd   W ~= 0
G IS A sizex (2size + 1) MATRIX SYMMETRIC AND WITH ZEROS IN THE UPPER AND
LOWER LEFT HAND CORNERS AND 1s ON THE CORRESPONDING DIAGONAL
P IS A DIAGONAL MATRIX sizexsize
IN FACT WE COMPUTE G COLUMN BY COLUMN AND P ELEMENT BY ELEMENT
SO G REPRESENTS THE COLUMN OF G THAT IS BEING COMPUTED.
IT IS COMPUTED AS A LINEAR COMBINATION OF THE PREVIOUS COLUMN G1
AND THE ONE BEFORE G2.THE RESPECTIVE COEFFICIENTS ARE u1 and u2.
EACH COEFFICIENT OF P IS THE SCALAR PRODUCT OF G AND THE
CORRESPONDING ROW OF toep i.e.THE ROW # i WHERE i IS THE ITERATION
INDICE.
TO COMPUTE u1 and u2 WE NEED v1 and v2;
v1 IS THE SCALAR
PRODUCT OF G AND THE ROW # (I + 1) OF R, v2 IS v1 OF THE PREVIOUS
STEP.

SUBROUTINES
INCR computes the increment of lft_hd each time we have a new
column of G
SCPROD performs the various scalar products (function)
CHECK computes W = rgt_hd - toep.lft_hd
SHIFT shift G1 into G2 and G into G1

rgt_hd is symmetric vector => need only read size coordinates
	note that rgt_hd is read starting from its center coordinate


----------------------------------------------------------------------------*/


static char *sccsid = "@(#)itopod.c	3.3  8/8/91 ESI ESI";

/*
 * System Includes
*/
#include <stdio.h>
#include <math.h>

/*
 * ESPS Includes
*/
# include <esps/unix.h>

# define Fprintf (void)fprintf

extern int debug_level;

int
itopod (toep, rgt_hd, lft_hd, size)
double *toep, *lft_hd, *rgt_hd;
int     size;

{
    static double  *G, *G1, *G2, *P, *W;
    static int  prev_alloc = 0;
    double  u2, u1, v1, v2, scprod (), stdvar, num, den;
    register int    i, j, l, im1, im2;


/* STORAGE ALLOCATION FOR LOCAL ARRAYS */

    /* if first time allocate storage for local arrays */
    if (prev_alloc == 0) {
        prev_alloc = size;
        if ((P = (double *) malloc ((unsigned)(size * sizeof (double)))) 
		== NULL)
	    faterr ("itopod", "cannot allocate P", 1);
        if ((W = (double *) malloc ((unsigned)(size * sizeof (double)))) 
		== NULL)
	    faterr ("itopod", "cannot allocate W", 1);
        if ((G = (double *) malloc ((unsigned)(size * sizeof (double)))) 
		== NULL)
	    faterr ("itopod", "cannot allocate G", 1);
        if ((G1 = (double *) malloc ((unsigned)(size * sizeof (double)))) 
		== NULL)
	    faterr ("itopod", "cannot allocate G1", 1);
        if ((G2 = (double *) malloc ((unsigned)(size * sizeof (double)))) 
		== NULL)
	    faterr ("itopod", "cannot allocate G2", 1);
    }
 /* check if need to reallocate storage for the local arrays*/
    else if (size > prev_alloc) { 
	prev_alloc = size;
	if ((P = (double *) realloc ((char *)P, (unsigned)(size * sizeof (double))))
		 == NULL)
	    faterr ("itopod", "cannot reallocate P", 1);
	if ((W = (double *) realloc ((char *)W, (unsigned)(size * sizeof (double))))
		 == NULL)
	    faterr ("itopod", "cannot reallocate W", 1);

	j = size + size + 1;/* G, G1, G2 are of size 2*size+1 */
	if ((G = (double *) realloc ((char *)G, (unsigned)(j * sizeof (double)))) 
		== NULL)
	    faterr ("itopod", "cannot reallocate G", 1);
	if ((G1 = (double *) realloc ((char *)G1, (unsigned)(j * sizeof (double)))) 
		== NULL)
	    faterr ("itopod", "cannot reallocate G1", 1);
	if ((G2 = (double *) realloc ((char *)G2, (unsigned)(j * sizeof (double)))) 
		== NULL)
	    faterr ("itopod", "cannot reallocate G2", 1);
    }

/* INITIALIZE */

    G2[0] = 1.0;
    P[0] = toep[0];
    G1[0] = -2.0 * toep[1] / toep[0];
    G1[1] = 1.0;
    v2 = toep[1];
    P[1] = toep[2] + toep[0] - 2.0 * toep[1] * toep[1] / toep[0];
    lft_hd[0] = rgt_hd[0] / P[0];
    for (l = 1; l < size; l++)
	lft_hd[l] = 0.0;

    if (P[1] == 0.0) {
	Fprintf (stderr, "itopod: P[1] = 0.0\ntoep is singular\n");
	return (1);
    }
    incr (lft_hd, rgt_hd, G1, P[1], 1);

/*
THE CASE i=2 IS SPECIAL FOR u1 & u2
*/

    G[2] = 1.0;
    v1 = scprod (toep, G1, 2, 1);
    u2 = -2.0 * P[1] / P[0];
    u1 = -v1 / P[1] + 2.0 * v2 / P[0];
    G[1] = G1[0] + u1;
    G[0] = G1[1] + G1[1] + u1 * G1[0] + u2 * G2[0];

    P[2] = scprod (toep, G, 2, 2);
    if (P[2] == 0.0) {
	Fprintf (stderr, "itopod: P[2] = 0.0\ntoep is singular\n");
	return (1);
    }
    incr (lft_hd, rgt_hd, G, P[2], 2);
    v2 = v1;
    shift (G2, G1, G, 2);

/* END OF INITIALIZATION */

    for (i = 3, im1 = 2, im2 = 1; i < size; i++, im1++, im2++) {
    /* im1=i-1 ; im2=i-2 */
	G[i] = 1.0;
	v1 = scprod (toep, G1, i, im1);
	u2 = -P[im1] / P[im2];
	u1 = v2 / P[im2] - v1 / P[im1];
	G[im1] = G1[im2] + u1;
	G[0] = G1[1] + G1[1] + u1 * G1[0] + u2 * G2[0];

	for (j = 1; j <= im2; j++)
	    G[j] = G1[j + 1] + G1[j - 1] + u1 * G1[j] + u2 * G2[j];

	P[i] = scprod (toep, G, i, i);

	if (P[i] == 0.0) {
	    Fprintf (stderr, "itopod: P[%d] = 0.0\ntoep is singular\n", i);
	    return (1);
	}
	incr (lft_hd, rgt_hd, G, P[i], i);
	v2 = v1;
	shift (G2, G1, G, i);
    }

    if (debug_level > 0) {
	itopod_chk (toep, lft_hd, rgt_hd, W, size);
	for (i = 0, num = den = 0.0; i < size; i++) {
	    num += W[i] * W[i];
	    den += rgt_hd[i] * rgt_hd[i];
	}
	stdvar = sqrt (num / den) / size;
	Fprintf (stderr, "  ITOPOD normalized error check = %lg \n", stdvar);
    }

    return (0);
}

/* ------------------------------------------------------------------ */

incr(x,y,g,p,i)
double *x,*y,*g,p;
int i;

/*
INCREMENT THE VALUE OF THE VECTOR x USING THE NEW VALUE IN THE
	MATRIX G GIVEN BY THE VECTOR g
	THE EQUATION IS x=G.Pinv.Gtranspose.y
	EACH TIME A NEW COLUMN ,g , OF G IS FOUND x IS INCREMENTED
	EACH ELEMENT OF P AS WE COMPUTE IT IS IN FACT HALF
	OF ITS TRUE VALUE. ON THE OTHER HAND, WHEN alfa	 IS COMPUTED
EACH ELEMENT OF G APPEARS TWICE[G (K) & G (-K)] EXCEPT G (0)
*/

{
    register int    k;
    register double alfa;

    alfa = g[0] * y[0] / 2.0;
    for (k = 1; k <= i; k++)
	alfa += g[k] * y[k];

    alfa /= p;

    for (k = 0; k <= i; k++)
	x[k] += g[k] * alfa;
}


/* -------------------------------------------------------------------*/

double scprod(a,b,i,i1)
double *a,*b;
int i,i1;

/*
a AND b REPRESENT SYMMETRIC ODD DIMENSIONAL VECTORS
	WHERE ONLY HALF OF THE COORDINATES HAVE BEEN WRITTEN 
(THE USEFUL 1.0S)
WE TAKE THE SCALAR PRODUCT AFTER HAVING SLIDED a BY i ELEMENTS
i1 IS THE NUMBER OF NON ZERO ELEMENTS OF VECTOR b
*/

{
    register int    l, j, k;
    static double   temp;	/* static => more efficient */

    if (i1 > i)
	faterr ("scprod", " i1 > i", 1);
/* Note that the size of array a is assumed to be at least 2*i */

    temp = b[0] * a[i];
    for (l = 1, j = i + 1, k = i - 1; l <= i1; l++, j++, k--)
				/*  j=i+l; k=i-l  */
	temp += b[l] * (a[j] + a[k]);

    return (temp);
}

/* -------------------------------------------------------------------*/

itopod_chk (r, x, y, w, n)
double *r, *x, *y, *w;
int     n;

/*
VERIFY THAT WE ACTUALLY HAVE r.x=y KNOWING THAT x AND y
 ARE SYMMETRIC
*/

{
    register int    k, l, i, j;
    register double temp;

    for (k = 0; k < n; k++) {
	temp = y[k] - x[0] * r[k];
	for (l = 1, j = k + 1; l < n; l++, j++) {/*  j=k+l  */
	    i = abs (k - l);
	    temp -= x[l] * (r[j] + r[i]);
	}
	w[k] = temp;
    }
}

/*------------------------------------------------------------------- */

shift (a, b, c, size)
double	*a, *b, *c;
int	size;

/*
SHIFT VECTOR b INTO VECTOR a AND VECTOR c INTO VECTOR b
	THE DIMENSION OF VECTORS a IS size and, IS  size+1 FOR b AND c
*/

{
register int l;
double *dptra, *dptrb, *dptrc;

    for (l=0, dptra = a, dptrb = b, dptrc = c ; l< size ; l++) {
	*dptra++ = *dptrb;
	*dptrb++ = *dptrc++;
    }
b[size]=c[size];

}

/*------------------------------------------------------------------- */

