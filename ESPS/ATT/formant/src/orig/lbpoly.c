/*		lbpoly.c		*/
/*					*/
/* A polynomial root finder using the Lin-Bairstow method (outlined
	in R.W. Hamming, "Numerical Methods for Scientists and
	Engineers," McGraw-Hill, 1962, pp 356-359.)		*/

#ifndef lint
	static char *sccs_id = "@(#)lbpoly.c	1.1 12/14/88		ATT, ESI ";
#endif

#include <math.h>
#include <stdio.h>

#define MAX_ITS	100	/* Max iterations before trying new starts */
#define MAX_TRYS	100	/* Max number of times to try new starts */
#define MAX_ERR		1.e-6	/* Max acceptable error in quad factor */
#define MAXORD		30	/* Maximum allowable polynomial order */
#define TRUE		1
#define FALSE		0


quad(a,b,c,r1r,r1i,r2r,r2i) /* find x, where a*x**2 + b*x + c = 0 	*/
double	a, b, c;
double *r1r, *r2r, *r1i, *r2i; /* return real and imag. parts of roots */
{
double  sqrt(), numi;
double  den, y;

	if(a == 0.0){
		if(b == 0){
		   printf("Bad coefficients to _quad().\n");
		   return(FALSE);
		}
		*r1r = -c/b;
		*r1i = *r2r = *r2i = 0;
		return(TRUE);
	}
	den = 2.0 * a;
	numi = b*b -(4.0 * a * c);
	if(numi >= 0.0){
		*r1i = *r2i = 0.0;
		y = sqrt(numi);
		*r1r = (-b + y)/den;
		*r2r = (-b - y)/den;
		return(TRUE);
	}
	else {
		*r1i = sqrt( -numi )/den;
		*r2i = -*r1i;
		*r2r = *r1r = -b/den;
		return(TRUE);
	}
}

lbpoly(a,order,rootr,rooti) /* return FALSE on error */
double	*a;	/* coefficients of the polynomial (increasing order) */
int	order; /* the order of the polynomial */
double	*rootr, *rooti; /* the real and imaginary roots of the polynomial */
/* Rootr and rooti are assumed to contain starting points for the root search
	on entry to lbpoly(). */
{
int ord, ordp1, ordm1, itcnt, i, j, k, mmk, mmkp2, mmkp1, ntrys;
double fabs(), err, p, q, delp, delq, b[MAXORD], c[MAXORD], den;
double	new_try;

/* kluge kluge kluge kluge kluge kluge kluge kluge kluge kluge  */
/* For now, "fortranify" the indices: */
a--; rootr--; rooti--;
/* kluge kluge kluge kluge kluge kluge kluge kluge kluge kluge  */


for(ord = order; ord > 2; ord -= 2){
	ordp1 = ord+1;
	ordm1 = ord-1;
/* Here is a kluge to prevent UNDERFLOW! (Sometimes the near-zero roots left
	in rootr and/or rooti cause underflow here...	*/
	if(fabs(rootr[ord]) < 1.0e-10) rootr[ord] = 0.0;
	if(fabs(rooti[ord]) < 1.0e-10) rooti[ord] = 0.0;
	p = -2.0 * rootr[ord]; /* set initial guesses for quad factor */
	q = (rootr[ord] * rootr[ord]) + (rooti[ord] * rooti[ord]);
   for(ntrys = 0; ntrys < MAX_TRYS; ntrys++){
	for(itcnt = 0;itcnt < MAX_ITS; itcnt++){
		b[ordp1] = a[ordp1];
		b[ord] = a[ord] - (p * b[ordp1]);
		c[ordp1] = b[ordp1];
		c[ord] = b[ord] - (p * c[ordp1]);
		for(k = 2;k <= ordm1; k++){
			mmk = ordp1 - k;
			mmkp2 = mmk+2;
			mmkp1 = mmk+1;
			b[mmk] = a[mmk] - (p* b[mmkp1]) - (q* b[mmkp2]);
			c[mmk] = b[mmk] - (p* c[mmkp1]) - (q* c[mmkp2]);
		}
/* ????		b[1] = a[1] - q * b[3];			*/
		b[1] = a[1] - p * b[2] - q * b[3];	

		err = fabs(b[1]) + fabs(b[2]);

		if(err <= MAX_ERR) break;

		den = (c[3] * c[3]) - (c[4] * (c[2] - b[2]));  
		if(den == 0.0){
			printf("Zero den in _lbpoly.\n");
			return(FALSE);
		}
		delp = ((c[3] * b[2]) - (c[4] * b[1]))/den;
		delq = ((c[3] * b[1]) - (b[2] * (c[2] - b[2])))/den;  

/*
printf("\nerr=%f  delp=%f  delq=%f  p=%f  q=%f",err,delp,delq,p,q);
*/
		p += delp;
		q += delq;
	}
	if(itcnt >= MAX_ITS){ /* try some new starting values */
		p = ((double)rand() - (1<<30))/(1<<31);
		q = ((double)rand() - (1<<30))/(1<<31);
/*		printf("\nTried new values: p=%f  q=%f\n",p,q); */
	}
	else	/* we finally found the root! */
		break;

  } /* for(ntrys... */
	if((itcnt >= MAX_ITS) && (ntrys >= MAX_TRYS)){
		printf("Exceeded maximum trial count in _lbpoly.\n");
		return(FALSE);
	}

	if(!quad(1.0,p,q,&rootr[ord],&rooti[ord],&rootr[ordm1],&rooti[ordm1]))
			return(FALSE);

/* Update the coefficient array with the coeffs. of the reduced polynomial. */
	for( i = 1; i <= ordm1; i++) a[i] = b[i+2];
}

if(ord == 2){ /* Is the last factor a quadratic? */
	if(!quad(a[3],a[2],a[1],&rootr[2],&rooti[2],&rootr[1],&rooti[1]))
			return(FALSE);
	return(TRUE);
}
if(ord < 1) { 
	printf("Bad ORDER parameter in _lbpoly()\n");
	return(FALSE);
}

if( a[2] != 0.0) rootr[1] = -a[1]/a[2];
else {
	rootr[1] = 100.0; /* arbitrary recovery value */
	printf("Numerical problems in lbpoly()\n");
}
rooti[1] = 0.0;

return(TRUE);
}

/*
main()
{
int i,n;
int j;
double a[MAXORD],rootr[MAXORD],rooti[MAXORD];

while(1){
printf("\nOrder of the polynomial to be solved: ");
scanf("%d",&j);
n = j;
printf("\nEnter the coefficients of the polynomial in increasing order.\n");
for(i=0;i <= n;i++){
	printf("a\(%2d\) : ",i);
	scanf("%lf",&a[i]);
	rootr[i] = 22.0; 
	rooti[i] = -10.0;
}
if(! lbpoly(a,n,rootr,rooti)){
	printf("\nProblem in root solver.\n");
} else {
	for(i=0;i<n;i++)printf("\n%d  re %f    im%f",i,rootr[i],rooti[i]);
}
}
}
*/

