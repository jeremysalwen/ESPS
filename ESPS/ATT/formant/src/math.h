/*	math.h	4.6	9/11/85	*/
/*      @(#)math.h	1.1 4/10/91 ERL */
/*   

   NOTE:  THIS FILE IS ONLY USED WHEN COMPILING ON THE APOLLO
*/

/*
 *	Apollo Changes:
 *
 *	08/11/89 rps		matherr decls from system V
 *      01/07/89 mills      Add asinh,acosh,atanh,copysign,drem,logb,scalb, and cabs
 *                          note no prototype checking for cabs()
 *      03/23/88 lwa        Fix incorrect fabs() decl.
 *	12/04/87 james_w	Provide function prototypes.
 */

/*  Enable function prototypes for ANSI C and C++  */
#if defined(__STDC__) || defined(c_plusplus) || defined(__cplusplus)
#    define _PROTOTYPES
#endif

/*  Required for C++ V2.0  */
#ifdef  __cplusplus
    extern "C" {
#endif

#ifndef _PROTOTYPES
extern double asinh(), acosh(), atanh();
extern double erf(), erfc();
extern double exp(), expm1(), log(), log10(), log1p(), pow();
extern double fabs(), floor(), ceil(), rint();
extern double lgamma();
extern double hypot(), cabs();
extern int matherr();
extern double copysign(), drem(), logb(), scalb();
extern int finite();
#ifdef vax
extern double infnan();
#endif
extern double j0(), j1(), jn(), y0(), y1(), yn();
extern double sin(), cos(), tan(), asin(), acos(), atan(), atan2();
extern double sinh(), cosh(), tanh();
extern double cbrt(), sqrt();
extern double modf(), ldexp(), frexp(), atof();
#else
extern double asinh(double x), acosh(double x), atanh(double x);
extern double j0(double x), j1(double x), jn(int n, double x), 
              y0(double x), y1(double x), yn(int n, double x);
extern double erf(double x), erfc(double x);
extern double exp(double x), log(double x), log10(double x), 
              pow(double x, double y), sqrt(double x),
              expm1(double x), log1p(double x);
extern double floor(double x), ceil(double x), rint(double x),
              fabs(double x);
extern double lgamma(double x);
extern double hypot(double x, double y), cabs();
extern int    matherr(struct exception *x);
extern double sinh(double x), cosh(double x), tanh(double x);
extern double sin(double x), cos(double x), tan(double x), 
              asin(double x), acos(double x), atan(double x), 
              atan2(double y, double x);
extern double atof(char *nptr);
extern double ldexp(double value, int exp), 
              frexp(double value, int *eptr), 
              modf(double value, double *iptr);
extern double copysign(), drem(), logb(), scalb();
extern int    finite();
#endif

#undef _PROTOTYPES

#ifdef  __cplusplus
    }
#endif

#define HUGE		((float) 0.3402823466e+39)
#define MAXDOUBLE	1.79769313486231570e+308

#ifndef APOLLO_68K
struct exception {
	int type;
	char *name;
	double arg1;
	double arg2;
	double retval;
};
#endif

#define	DOMAIN		1
#define	SING		2
#define	OVERFLOW	3
#define	UNDERFLOW	4
#define	TLOSS		5
#define	PLOSS		6
