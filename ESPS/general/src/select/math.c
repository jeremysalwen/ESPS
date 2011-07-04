/* this module is taken from "The Unix Programming Environment" by
   Kerningham and Pike.  I'm using it because its exactly what I need
   and it is published.  I have no evil intent and don't wish to steal
   their software if they object.   I'll let some lawyer decide if this is
   proper.  I can rewire this if required; but for now it will save our
   time to use this module as is.  

   Alan Parker, Entropic Speech, Inc.



*/

#ifndef lint
	char *math_sccs = "@(#)math.c	3.2	1/6/93 EPI";
#endif

#include "select.h"

extern int errno;
double errcheck();
extern int run_error;

double Log(x)
double x;
{
	return errcheck(log(x), "log");
}

double Log10(x)
double x;
{
	return errcheck(log10(x), "log10");
}

double Sqrt(x)
double x;
{
	return errcheck(sqrt(x), "sqrt");
}

double Exp(x)
double x;
{
	return errcheck(exp(x), "exp");
}

double Pow(x, y)
double x, y;
{
	return errcheck(pow(x,y), "exponentiation");
}

double integer(x)
double x;
{
	return (double)(long)x;
}

double errcheck(d, s)
double d;
char *s;
{
	if (errno == EDOM) {
		errno = 0;
		errmsg1("%s: argument out of domain; ",s);
		run_error++;
	}
	else if (errno == ERANGE) {
		errno = 0;
		errmsg1("%s: result out of range; ",s);
		run_error++;
	}
	return d;
}
