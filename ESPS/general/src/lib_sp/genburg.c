/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 */

static char *sccs_id = "@(#)genburg.c	1.10	2/21/96	ESI/ERL";

/*
The generalized Burg technique

Programmed by Daniel L. Wenger
408-425-5790 Santa Cruz, California

See:
"Estimation of Structured Covariance Matrices", J.P.Burg, D.G.Luenberger,
D.L.Wenger; Proceedings of the IEEE, Vol. 70, No. 9 September 1982

"Estimation of Covariance Matrices Which Are Linear Combinations or
Whose Inverses Are Linear Combinations of Given Matrices", T.W.Anderson

This program uses the algorithms discussed in the above papers to
find the best estimate for the covariance matrix of single channel real or
complex, multidimensional real or two channel real problems.

Input is an Hermitian sample covariance matrix 
Output is an Hermitian Toeplitz (block Toeplitz) matrix

sigma_in is the address of the real part of the sample matrix
isigma_in is the address of the imaginary part of the sample matrix

sigma_out is the address of the real part of the Toeplitze matrix
isigma_out is the address of the imaginary part of the Hermitian matrix

sigma_out and isigma_out are also used to pass the initial guess
              to the routine if initflag=1

*qd=qdim and *pd=pdim are integers

in the single channel case, qdim is the filter order plus one and pdim is one.

in the multi-dimensional case, qdim is the dimension of the block Toeplitz 
	matrix and pdim is the dimension of the individual element matrices.

in the multi-channel case, qdim is the filter order plus one and pdim
	is the number of channels ( restricted to 2 )

in all cases, the dimension of the input matrix is qdim times pdim

*c is a flag   =0  for a real matrix
	       =1  for a complex matrix

*sflag is a flag   =1  for the single channel case
	       =2  for the multi-dimensional case
               =3  for the multi-channel case

*mo=moflag is a flag  =0  for no monitoring
                      =1  for monitoring

if *inflag=initflag=0, initial guess is the identity matrix
if *inflag=initflag=1, initial guess is in sigma_out and possibly isigma_out
if *inflag=initflag=2, initial guess is the first projection of sigma_in
if *inflag=initflag=3, initial guess is John's average + first projection

the final distance or measure is returned in pdist, *pdist is a double

*small_in=small is a double used as a limit in the interpolation search
	        a reasonable value is 1e-8 to 1e-12

*Maxit_in=Maxit is a cutoff for maximum number of iterations with no decrease of
            	measure, a reasonable value is 4

a return flag is returned in returnflag
a value of zero is returned upon no decrease in measure after Maxit attempts
a value of one is returned upon Rinit or Rnew singular or Rinit neg definite
a value of two is returned upon sigma_in or Rinit non-pos-definite
a value of three is returned upon Aij singular
a value of four is returned upon unsuccessful interpolation, no pos-def Rnew
a value of five is returned upon measure ratio convergence test
a value of six is returned upon insufficient storage allocation

*convfact=cvfact is a convergence factor,
                 (R_measure-Rnew_measure)/Rnew_measure<convfact
*/

#include <stdio.h>
#include <math.h>
#if !defined(LINUX_OR_MAC)
#include <malloc/malloc.h>
#endif

static void	init();
static void	doidentity();
static void	doinit();
static int	ckS();
static int	Iterate();
static void	ckRinit();
static int	ckRnew();
static int	cvtest();
static int	inv_det();
static double	domeasure();
static int	do_algorithm();
static void	DooRnew();
static void	doAspec();
static void	doAij();
static void	docAij();
static void	finishunroll();
static void	setunroll();
static double	dooAij();
static double	doocAij();
static double	doooA();
static void	doBi();
static void	antisymmetrize();
static void	doRnew();
static void	makesc();
static void	make();
static void	doPtab();
static void	makesigma();
static void	gb_free();
static int	do_alloc();
static void	setdel();
static int	interp();
static void	update();
static double	dointerppoint();
static void	Rnew_R();
static void	Rnew_R0();
static void	R_R0();
static int	iinterp();
static void	spmult();
static void	exR();
static void	set_up();
static void	getdist();
static int	allocdp();
static int	allocsp();
static void	cdmmult();
static int	cholesky();
static int	cnormal();
static void	dmadd();
static void	dmclear();
static void	dmmove();
static void	dmmult();
static void	dmsub();
static double	dmtrace();
static void	dojohnavg();
static double 	dot();
static void	dvclear();
static void	dvmove();
static void	dvprintf();
static void	dvsub();
static void	freedp();
static void	freesp();
static void	get_min();
static double 	hdmtrace();
static int	lctoepinv();
static int	ldmsyminv();
static int	lorthogin();
static int	lsyminv();
static int	ltoepinv();
static int	normal();
static int	orthog();
static void	sp1();
static int	posdefsol();
static void	solchol();
static double	spdmtrace();
static double	spdot();
static void	toepmult();
static double 	trace();
static int	triinv();

static int qdim=0;
static int pdim=1;
static int rflag=1;	/* real flag */
static int scflag=1;	/* single channel flag */
static int mdflag=0;	/* multi-dimensional flag */
static int mcflag=0;	/* multi-channel flag */
static int cflag = 0;	/* complex */
static int moflag = 0;	/* monitor flag */
static double small=1e-8;	/* convergence criterian */
static int cnt=1;		/* iterations for which measure does not decrease */
static int Maxit=4;	/* maximum iteration limit */
static int Ni=0;
static int tNi=0;
static int qp=0;
static int qpxqp=0;
static int it_cnt=0;/* total iteration count */

static short *Pii=0,*Pi=0,*Pj=0,*Ptaba=0,*Ptabb=0,*Ptabc=0;

static double *R=0,*iR=0;	/* solution with the lowest measure todate */
static double R_det=0;		/* natural log of determinant of R */
static double R_measure=0;	/* measure of R */

static double *R0=0,*iR0=0;	/* last solution of algorithm or searches */
static double R0_det=0;		/* natural log of determinant of R0 */
static double R0_measure=0;	/* measure of R0 */

static double *Rnew=0,*iRnew=0;	/* current solution of algorithm or searches */
static double Rnew_det=0;		/* natural log of determinant of Rnew */
static double Rnew_measure=0;	/* measure of Rnew */

static double *del=0,*idel=0;	/* difference between Rnew and R0 */

static double *siginv=0;
static double *isiginv=0;
static double *sigtemp=0,*isigtemp=0;
static double *sig1temp=0,*isig1temp=0;
static double *sig2temp=0,*isig2temp=0;
static double *Aij=0;
static double *Ainv=0,*M=0;
static double *Bi=0,*tBi=0,*ttBi=0,*tttBi=0;

static double *S=0;	/* pointer to sigma */
static double *iS=0;	/* pointer to isigma */
static double logdetS=0;

/* relating to unroll of doAij */
static int ii=0;/* count for table length */
static int iiflag=0;/* status of table making */
static short *Aijtab=0;/* unroll table */
static short *pAijtab=0;/* address in unroll table */
static int qdimsave=0;/* previous qdim */
static int pdimsave=0;/* previous pdim */

static long t0=0,tf=0;

static double cvfact=1e-6;
	/* convergence factor, |(d(new)-d(previous))/d(new)|<cvfact */

static int initflag;
	/* flag to select initial guess, 0=identity,1=input matrix, 2=first
	projection of sigma_in, 3= John's average + first projecton */

static int andersn=0;/* 0=burg,1=anderson, no anderson for complex case */

static int cntAij=0;/* count of Aij construction */

void
genburg(sigma_in,isigma_in,qd,pdist,sigma_out,isigma_out,c_flag, monitor_flag,returnflag,R_out,iR_out,init_flg, anderson)
double *sigma_in,*isigma_in,*pdist,*sigma_out,*isigma_out;
double *R_out,*iR_out;
int *qd,*returnflag,c_flag,monitor_flag,init_flg,anderson;
{
	/* assign input values to internal variables */

        initflag = init_flg;
	andersn = anderson;
	cntAij=0;
	S=sigma_in;
	iS=isigma_in;
	qdim= *qd;
	if (c_flag) {
	    cflag = 1;
	    rflag = 0;
	  }
	moflag = monitor_flag;
	set_up();/* calculate constants */

	if(do_alloc())/* allocate storage */
		{
		fprintf(stderr,"Storage allocation not possible for genburg().\n");
		gb_free();
		*returnflag=6;
		return;
		}

	doPtab();/* make tables for making the bases matrices */

	ckS();/* get ln|S| */

	init(initflag,sigma_out,isigma_out,R,iR);/* select initial R vector and put in R[] and R0[] */

	if(moflag)
		{
		fprintf(stderr,"Rinit=\n");
		dvprintf(R,Ni); if(cflag)dvprintf(iR,Ni);
		fprintf(stderr,"ln|Rinit|=%g\n",R_det);
		fprintf(stderr,"d(S,Rinit)=%.15g\n",R_measure);
		}

	*returnflag=Iterate();/* main iteration loop */

	makesigma(R,iR,sigma_out,isigma_out,Pi);/* construct output matrix
	from final R vector */

	dvmove(R,R_out,Ni);
	if(cflag)
		dvmove(iR,iR_out,Ni);

	*pdist=R_measure;/* output final measure */

	gb_free();/* free storage */
	return;
}
/* genburg()
		Main program.
		Sets flags, allocates storage for arrays via do_alloc(), 
		sets up tables for generating the basis matrices Pi via
		doPtab(), sets up primary vectors R[], R0[] and Rnew[]
		via init(), R[] should contain a positive definite initial
		quess for the solution, checks Rinit via ckRinit(), checks
		S via ckS() and gets ln|S| for the distance measure,
		calls Iterate(), the central loop, upon completion
		of iteration, constructs an output matrix from the
		vector R[], sets return flags, frees storage via
		gb_free() and exits.

		The three vecotors R[],R0[] and Rnew[] have the following
		roles:

		R[] contains the initial vector which is positive definite, and
			the most recent vector that is positive definite
			and of smaller measure than the previous R[]
		R0[] contains the most recent positive definite vector,
			not necessarily of lower measure than the previous,
			used to get the next vector in the algorithm
		Rnew[] contains the newest vector, yet to be determined to
			positive definite

		Throughout the program, 'moflag' is a flag set for monitoring.
		Ni is the length of the R[] vector, tNi is the combined
		length of the vectors R[] and iR[], where iR[] is the
		imaginary part in the complex case.

		The coding
			temp=funct();
			R[i]=temp;
		which occurs in program is due to an error in the
		Unisoft C compiler, the above intermediate assigment
		circumvents the bug, namely, one cannot assign directly
		the result of an expression to an array.
*/

static void
init(iflag,sigma,isigma,R,iR)
/* select initial R vector */
/* check for positive definiteness, revert to a positive definite initial R */
/* setup R and R0 */
register int iflag;
register double *sigma,*isigma,R[],iR[];
{



	if(iflag==1)
		{
		exR(sigma,isigma,R,iR,qp,Ni); /* extract R from sigma */
		ckRinit();
		}

	else if(iflag==2)
		{
		doinit(S,iS,R,iR);/* use S projection as Rinit */
		ckRinit();
		}

	else if(iflag==3 && rflag)
		{
		dojohnavg(S,siginv,qp);/* average matrix according to John */
		doinit(siginv,isiginv,R,iR);/* use S projection as Rinit */
		ckRinit();
		}

	else /* set initial R to the identity matrix */
		{
		initflag=0;
		dvclear(R,Ni);
		if(cflag)
			dvclear(iR,Ni);
		R[0]=1;
		doidentity(siginv,isiginv,qp);/* set inverse and measure */
		}

	dvmove(R,R0,Ni);
	dvclear(Rnew,Ni);
	if(cflag)
		{
		dvmove(iR,iR0,Ni);
		dvclear(iRnew,Ni);
		}
	return;
}
/* init()
	Based upon the input flag 'initflag', sets up R[] with the initial
	guess, if initflag==0, the initial guess is the identity matrix,
	if initflag==1, the initial guess is extracted via exR() from
	the sigma given in the input, if initflag==2, the initial guess
	is the projection R[i]=trace[S,Pi]/Pii[i], obtained via doinit()
	where S is the input sample covariance matrix, Pi are the basis
	matrices, and Pii[i] is the metric of the basis matrices, namely,
	Pii[i]=trace[Pi,Pi], the off diagonal elements of the metric
	are zero since the Pi are chosen to be orthogonal.
	The initial R vector is checked for positive definitness and
	reversion to a positive definite initial R is done if necessary.
*/

static void
doidentity(siginv,isiginv,qp)/* set inverse and measure for identity start */
register double *siginv,*isiginv;
register int qp;
{
	register int i;
	double trace();

	dmclear(siginv,qp);/* set inverse to identity matrix */
	for(i=0;i<qp;i++,siginv+=qp+1)
		*siginv=1;
	if(cflag)
		dmclear(isiginv,qp);
	R_measure=trace(S,qp)-qp;
	R_measure-=logdetS;/* measure for identity start */
	return;
}

static void
doinit(S,iS,R,iR)/* make an R vector from the input sample covariance matrix */
register double S[],iS[],R[],iR[];
{
	register int fNi,i;
	register double Rt,temp;
	double spdmtrace();

	fNi=Ni;
	for(i=0;i<fNi;i++)
		{
		make(Pi,i);
		Rt=spdmtrace(S,Pi,qp);/* trace of M1*M2, M1 double, M2 short */
		temp=Rt/Pii[i];
		R[i]=temp;
		}
	if(cflag)
		{
		iR[0]=0;
		for(i=1;i<fNi;i++)
			{
			make(Pi,i);
			/* make the symmetric matrix Pi antisymmetric */
			antisymmetrize(Pi,qp);
			Rt=spdmtrace(iS,Pi,qp);
			temp= -Rt/Pii[i];
			iR[i]=temp;
			}
		}
	return;
}
/* doinit()
		Forms the R[] vector from the input sample covariance
		matrix, R[i]=trace[S,Pi]/Pii[i], where S is the sample
		covariance matrix, Pi are the basis matrices, and Pii[i]
		is trace[Pi,Pi]. This corresponds to the initial guess
		(almost) of Anderson.
*/

static int
ckS()/* check S for positive definitness and get ln|S| */
{
	register int rtflag;

	if(rflag)
		{
		dmmove(S,sigtemp,qp);

		/* test for positive definitness with cholesky and get ln|S| */
		rtflag=lsyminv(sigtemp,siginv,sig1temp,qp,&logdetS);

		if(rtflag==1)
			{
			fprintf(stderr,"sigma sample singular (or possibly negative definite)\n");
			logdetS=0;
			}
		if(rtflag==2) 
			fprintf(stderr,"sigma sample non-positive-definite\n");
		}

	if(cflag)
		{
		fprintf(stderr,"no ln|S| available, set to zero\n");
		logdetS=0;
		/*
		rtflag=lhdmsym(siginv,isiginv,siginv,isiginv,&logdetS,qp);
		*/
		}

	if(moflag)
		fprintf(stderr,"ln|S|=%g\n",logdetS);

	return(rtflag);
}
/* ckS()
		Check to see if the sample covariance matrix S is
		positive definite, singular or negative definite,
		by computing its inverse via a routine that also
		returns the determinant.
		The determination of the properties of S is not
		definitive.
*/

static int
Iterate()/* main interation loop */
{
	register int rtflag;

	cnt=0;	/* count of number of times no increase in measure */
	it_cnt=0;	/* iteration count */
	while(cnt<Maxit)
		{
		it_cnt++;

		/* with initial unit matrix, compute Rnew quickly */
		if(it_cnt==1 && initflag==0)
			doinit(S,iS,Rnew,iRnew);/* use S projection as Rnew */

		else
			{
			rtflag=do_algorithm();/* compute a new Rnew using R0 */
			if(rtflag==3)
				{
				fprintf(stderr,"Rnew(1) not possible, Aij singular\n");
				fprintf(stderr,"Quiting.\n");
				break;
				}

			}

		rtflag=ckRnew();/* check Rnew for positive definitness */

		if(rtflag==0)
			continue;
		if(rtflag==1)
			break;/* Rnew singular */
		if(rtflag==4)
			break;/* 1/10 interpolation failed */
		if(rtflag==5)
			break;/* converged by cvtest */
		}
	if(moflag)
		{
		if(cnt==Maxit) fprintf(stderr,"\nMaxit reached, stop\n");
		fprintf(stderr,"after %d iterations\n\n",it_cnt);
		fprintf(stderr,"R final =\n");
		dvprintf(R,Ni); if(cflag) dvprintf(iR,Ni);
		fprintf(stderr,"ln|Rfinal|=%g\n",R_det);
		fprintf(stderr,"d(S,Rfinal)=%.15g\n",R_measure);
		if (mcflag)fprintf(stderr,"coherence=%g\n",R[2]*R[2]/(R[0]*R[1]));
		}
	return(rtflag);
}
/* Iterate()
		Main loop of the program. Checks to see if 'Maxit' iterations
		have occured during which no new solution has been found
		with a reduced measure.
		Via do_algorithm(), computes a new Rnew[] vector and
		checks to see if it is positive definite via ckRnew().
		ckRnew() also calls an interpolation routine interp()
		if the new solution is not positive definite.
*/

static void
ckRinit()/* check Rinit or first projection of S for positive definitness */
{
	register int rtflag;
	double domeasure();

	/* get inverse and ln|R| to do initial measure */

	rtflag=inv_det(R,iR,siginv,isiginv,&R_det);

	if(rtflag)
		{
		if(rtflag==1) fprintf(stderr,"initial R singular or negative definite\n");
		if(rtflag==2) fprintf(stderr,"initial R non-positive-definite\n");
		if(initflag==1)/* sigma_out start */
			{
			if(moflag) fprintf(stderr,"Reverting to first projection for initial quess\n");
			initflag=2;/* first projection */
			init(initflag,siginv,isiginv,R,iR);
			}
		else if(initflag==2 || initflag==3)/* first projection start */
			{
			if(moflag) fprintf(stderr,"Reverting to identity matrix for initial quess\n");
			initflag=0;
			init(initflag,siginv,isiginv,R,iR);
			rtflag=inv_det(R,iR,siginv,isiginv,&R_det);
			}

		}
	R_measure=domeasure(siginv,isiginv,R_det);
	return;
}
/* ckRinit()
		Check the initial guess for R[] to see if it is positive
		definit and in the process, get the measure for the
		initial guess. If the initial guess is not positive
		definite and is given as input, revert to the 
		first projection of S as the initial guess. If that
		is not positive definite, revert to the identity matrix
		as the initial guess.
*/

static int
ckRnew()	/* check Rnew for positive definitness */
{
	register int rtflag,rttflag;
	double domeasure();
	double Rtemp_measure;/* R_measure saved for interpolation then cvtest */

	rtflag=inv_det(Rnew,iRnew,siginv,isiginv,&Rnew_det);
		 /* 0 Rnew pos-def
		    1 Rnew singular, Rnew_det=0
		    2 Rnew non-pos-def, Rnew_det=0 */
	Rnew_measure=domeasure(siginv,isiginv,Rnew_det);

	if(rtflag==1)
		{
		if(moflag)
			{
			fprintf(stderr,"\nRnew singular\n");
			fprintf(stderr,"Rnew=\n");
			dvprintf(Rnew,Ni);if(cflag)dvprintf(iRnew,Ni);
			fprintf(stderr,"interpolation not possible\n");
			}
		return(rtflag);/* break */
		}
	if(rtflag==2)
		{
		if(moflag) fprintf(stderr,"\nRnew npd\n");
		Rtemp_measure=R_measure;/* save measure for cvtest */
		rttflag=interp();/* do interpolation by 1/10 */
		if(rttflag==1)
			{
			if(moflag) fprintf(stderr,"stop, complete interpolation\n");
			return(4);
			}
		/* interp found a positive definite Rnew */
		if(cvtest(R_measure,Rtemp_measure))return(5);/* conclude iteration */
		return(0);
		}
	if(rtflag==0)
		{
		if(moflag)
			{
			fprintf(stderr,"\nRnew(1)=\n");
			dvprintf(Rnew,Ni); if(cflag) dvprintf(iRnew,Ni);
			fprintf(stderr,"ln|Rnew|=%g\n",Rnew_det);
			fprintf(stderr,"d(S,Rnew)=%.15g\n",Rnew_measure);
			}
		if(Rnew_measure>=R_measure)
			{
			Rtemp_measure=R_measure;/* save measure for cvtest */
			if(moflag)fprintf(stderr,"measure did not decrease\n");
			/* do interpolation by 1/2 */
			/* try to find a decreased measure */
			rttflag=iinterp();
			if(rttflag==1)
				{
				cnt++;
				if(moflag)
					fprintf(stderr,"no decrease in measure during interpolation\n");
				}
			/* iinterp() found a decreased measure */
			if(cvtest(R_measure,Rtemp_measure))return(5);/* conclude iteration */
			return(0);
			}
		if(Rnew_measure<R_measure)
			{
			if(cvtest(R_measure,Rnew_measure))
				{
				Rnew_R();
				return(5);/* conclude iteration */
				}
			Rnew_R();
			}
		Rnew_R0();/* use Rnew for next iteration */
		return(rtflag);
		}
	fprintf(stderr,"trouble ckRnew\n");
	exit(1);
	/*NOTREACHED*/
}

static int
cvtest(R_measure,Rnew_measure)
/* convergence test on measure change */
register double R_measure,Rnew_measure;
{
	register double temp;

	temp=(R_measure-Rnew_measure)/Rnew_measure;
	temp=(temp>=0)?temp:-temp;
	if(temp<cvfact)
		{
		if(moflag)
			fprintf(stderr,"Stopping due to convergence, rel_ratio=%g cvfact=%g\n",temp,cvfact);
		return(1);
		}
	return(0);
}

static int
inv_det(tR,itR,siginv,isiginv,pdet)
	/* return inverse and natural log of determinant */
double *tR,*itR,*siginv,*isiginv,*pdet;
{
		int rtflag;

		if(scflag)
			{
			if(rflag)
				{
				rtflag=ltoepinv(tR,siginv,pdet,qp);
				return(rtflag);
				}
			if(cflag)
				{
				rtflag=lctoepinv(tR,itR,siginv,isiginv,pdet,qp);
				return(rtflag);
				}
			}
		if(mdflag || mcflag)
			{
			makesigma(tR,itR,sigtemp,isigtemp,Pi);
			if(rflag)
				{
				/* test for positive definitness with cholesky
				but use more accurate orthogonalizaton technique
				to get siginv if positive def */


			/*
			dmmove(sigtemp,sig2temp,qp);
			rtflag=lsyminv(sig2temp,siginv,sig1temp,qp,pdet);
			*/

			rtflag=lsyminv(sigtemp,siginv,sig1temp,qp,pdet);
				if(rtflag)return(rtflag);

				/*
				rtflag=ldmsyminv(sigtemp,siginv,pdet,qp,0);
				*/
				return(rtflag);
				}
			if(cflag)
				{
				fprintf(stderr,"no ln|R| available\n");
				exit(1);
				/*
				rtflag=lhdmsym(siginv,isiginv,siginv,isiginv,pdet,qp);
				return(rtflag);
				*/
				}
			}
	fprintf(stderr,"Error in inv_det\n");
	exit(1);
	/*NOTREACHED*/
}

static double
domeasure(siginv,isiginv,det)
	/* det is the natural logrithm of the determinant of R */
register double det,*siginv,*isiginv;
{
	register double measure;
	double hdmtrace(),dmtrace();

	if(rflag)
		{
		measure=dmtrace(siginv,S,qp)-qp;
		measure=measure+det-logdetS;
		}
	if(cflag)
		{
		measure=hdmtrace(siginv,isiginv,S,iS,qp)-qp;
		measure=measure+det-logdetS;
		}

	return(measure);
}

static int
do_algorithm()
/* make an Rnew[] from the R0[] using the algorithm */
/* R0 is known to be positive definite */
{
	register int rtflag;
	double dum;

	doBi(siginv,Pi,Bi,qp);/* make Bi */

	/* sig1temp=siginv*S*siginv */
	/* sigtemp=siginv*S*siginv - siginv */

	if(andersn && (initflag==2 || initflag==3 || it_cnt>1) && rflag)
	/* do not allow anderson on first iteration unless initflag=2 or 3, do not allow anderson in complex case */
		{
		dmadd(sigtemp,sig1temp,sig2temp,qp);/* make 2*siginv*S*siginv-siginv */
		doAij(sig2temp,siginv,Aij);/* make Aij */
		}
	else
		{
		if(rflag)
			doAij(siginv,siginv,Aij);/* make Aij */
		if(cflag)
			docAij(siginv,isiginv,siginv,isiginv,Aij);/* make Aij */
		}

	/*
	rtflag=ldmsyminv(Aij,Ainv,&dum,tNi,0);
	if(rtflag==1)
		{ fprintf(stderr,"Aij singular\n"); return(3); }
	doRnew(Ainv,Bi);
	*/

	/* solves Aij*x=y, Aij pos definite */
	/* tBi=x, Bi=y, ttBi=tttBi=temp vector, M and Ainv are temp matrices */

	if(posdefsol(Aij,M,Ainv,tBi,Bi,ttBi,tttBi,tNi))
		{
		if(moflag)
			fprintf(stderr,"Aij npd from posdefsol\n");
		if(andersn)/* try burg algorithm */
			{
			fprintf(stderr,"trying burg algorithm\n");
			andersn=0;
			rtflag=do_algorithm();
			return(rtflag);
			}
		else
			{
			rtflag=ldmsyminv(Aij,Ainv,&dum,tNi,0);
			if(rtflag==1)
				{ fprintf(stderr,"Aij singular from ldmsyminv\n"); return(3); }
			doRnew(Ainv,Bi);
			return(0);
			}
		}

	else
		DooRnew(tBi,Rnew,R0);/* breakup tBi[] into Rnew[] and iRnew[] */
	return(0);
}

static void
DooRnew(tBi,Rnew,R0)/* breakup tBi[] into Rnew[] and iRnew[] */
register double tBi[],Rnew[],R0[];
{
	register int i,fNi;
	register double dum;

	fNi=Ni;

	if(andersn && (initflag==2 || initflag==3 || it_cnt>1) && rflag)
		{
		for(i=0;i<fNi;i++)
			{
			dum=tBi[i]+R0[i];
			Rnew[i]=dum;
			}
		}
	else
		{
		dvmove(tBi,Rnew,fNi);
		if(cflag)
			dvmove(tBi+fNi,iRnew+1,tNi-fNi);
		}
	return;
}

static void
doAspec(siginv,matrix,Aij)
register double matrix[],siginv[],Aij[];
{
	register double At;
	register int tab;
	register short *p;

	p=Aijtab;
	while(*p!= -2)
		{
		At=0;
		while((tab=(*p))!= -1)
			{
			p++;
			At+=siginv[tab]*matrix[*p];
			p++;
			}
		p++;
		*(Aij+ *p)=At;
		p++;
		*(Aij+ *p)=At;
		p++;
		}
	return;
}

static void
doAij(matrix,siginv,Aij)/* make Aij real case */
register double matrix[],siginv[],Aij[];
/* Aij=trace Pi*siginv*Pj*matrix */
/* Aij= (Pi)mn*(siginv)np*(Pj)pq*(matrix)qm with Einstein summation convent. */
{
	register int i,j,iM,jM,fNi,ftNi;
	register double At;
	double dooAij();

	/*
	cntAij++;
	if(cntAij>3)return;
	*/
	if(iiflag==2)/* unroll table has been made */
		{
		if(qdim==qdimsave && pdim==pdimsave)
			/* unroll table has been made and it is for the current
			dimension */
			{ doAspec(siginv,matrix,Aij); return; }/* do fast Aij */
		else
			{
			/* unroll table is not for current dimension */
			freedp(&Aijtab);/* free storage */
			iiflag=0;/*set to count for new table size */
			}
		}

	/* setup for unroll */
	/* first time (iiflag==0) just count for size of unroll table,
	second time (iiflag==1) assign indices to table */
	ii=0;
	pAijtab=Aijtab;

	fNi=Ni;
	ftNi=tNi;

	/* do Pi*siginv*Pj*matrix */
	/* see comments on Aij in Aijxxx */

	for(i=iM=0;i<fNi;i++,iM+=ftNi)
		{
		make(Pi,i);
		for(j=i,jM=iM+i;j<fNi;j++,jM+=ftNi)
			{
			if(i==j)
				{
				At=dooAij(siginv,Pi,matrix,Pi);
				Aij[iM+j]=At;
				}
			else
				{
				make(Pj,j);
				At=dooAij(siginv,Pi,matrix,Pj);
				Aij[jM]=Aij[iM+j]=At;
				}
			setunroll(jM,iM+j);
			}
		}
	finishunroll();
	return;
}

static void
docAij(matrix,imatrix,siginv,isiginv,Aij)/* do complex case Aij */
register double matrix[],imatrix[],siginv[],isiginv[],Aij[];
/* Aij=trace Pi*siginv*Pj*matrix */
/* Aij= (Pi)mn*(siginv)np*(Pj)pq*(matrix)qm with Einstein summation convent. */
{
	register int i,j,iM,jM,fNi,ftNi;
	int NixtNi;
	register double At;
	double doocAij();

	fNi=Ni;
	ftNi=tNi;

	/* do Pi*siginv*Pj*matrix - Pi*isiginv*Pj*imatrix */
	/* see comments on Aij */

	for(i=0,iM=0;i<fNi;i++,iM+=ftNi)
		{
		make(Pi,i);
		for(j=i,jM=iM+i;j<fNi;j++,jM+=ftNi)
			{
			if(i==j)
				{
				At=doocAij(siginv,isiginv,Pi,matrix,imatrix,Pi,0);
				Aij[iM+j]=At;
				}
			else
				{
				make(Pj,j);
				At=doocAij(siginv,isiginv,Pi,matrix,imatrix,Pj,0);
				Aij[jM]=Aij[iM+j]=At;
				}
			}
		}

	NixtNi=fNi*ftNi;

	/* do Pi*siginv*iPj*imatrix + Pi*isiginv*iPj*matrix */

	for(i=0,iM=0;i<fNi;i++,iM+=ftNi)
		{
		make(Pi,i);
		for(j=fNi,jM=NixtNi;j<ftNi;j++,jM+=ftNi)
			{
			make(Pj,j-fNi+1);
			At=doocAij(siginv,isiginv,Pi,matrix,imatrix,Pj,1);
			Aij[jM+i]=Aij[iM+j]=At;
			}
		}

	/* do iPi*siginv*iPj*matrix - iPi*isiginv*iPj*imatrix */

	for(i=fNi,iM=NixtNi;i<ftNi;i++,iM+=ftNi)
		{
		make(Pi,i-fNi+1);
		for(j=i,jM=iM;j<ftNi;j++,jM+=ftNi)
			{
			if(i==j)
				{
				At=doocAij(siginv,isiginv,Pi,matrix,imatrix,Pi,2);
				Aij[iM+j]=At;
				}
			else
				{
				make(Pj,j-fNi+1);
				At=doocAij(siginv,isiginv,Pi,matrix,imatrix,Pj,2);
				Aij[jM+i]=Aij[iM+j]=At;
				}
			}
		}
	return;
}

static void
finishunroll()
{
	if(iiflag==1)/* making table */
		{
		/* signal end of table */
		*pAijtab= -2;
		pAijtab++;
		}
	ii++;
	if(iiflag==0)/* not yet made table */
		{
		if(allocsp(&Aijtab,ii))
			fprintf(stderr,"cannot allocate for Aijtab\n");
		else
			{
			iiflag=1;/* set to make table */
			qdimsave=qdim;/* save dimensions of table */
			pdimsave=pdim;
			}
		}
	else if(iiflag==1)/* if makeing table, set table finished */
		iiflag=2;/* table finished */
	return;
}

static void
setunroll(i1,i2)/* set up addresses for unroll or just count for allocation */
register int i1,i2;
{
	if(iiflag==1)/* making table */
		{
		*pAijtab= -1;/* signal new element */
		pAijtab++;
		*pAijtab=i1;
		pAijtab++;
		*pAijtab=i2;
		pAijtab++;
		}
	else
		{
		/* doing count only */
		ii++;
		ii++;
		ii++;
		}
	return;
}

static double
dooAij(siginv,Pi,matrix,Pj)
register double *matrix,*siginv;
register short *Pj,*Pi;

/* Aij=trace Pi*siginv*Pj*matrix */
/* matrix is siginv or 2*siginv*S*siginv-siginv */
/* Aij= (Pi)mn*(siginv)np*(Pj)pq*(matrix)qm with Einstein summation convent. */
{
	register int m,n,nM,p,q,qM,qpr;
	register short *pPj;
	register double At;

	qpr=qp;
	At=0;
	for(m=0;m<qpr;m++)
		{
		for(n=0,nM=0;n<qpr;n++,Pi++,nM+=qpr)
			{
			if(!*Pi)
				continue;
			pPj=Pj;
			for(p=0;p<qpr;p++)
				{
				for(q=0,qM=m;q<qpr;q++,qM+=qpr,pPj++)
					{
					if(!*pPj)
						continue;
					At+= *(siginv+nM+p)* *(matrix+qM);
					if(iiflag)/* making table */
						{ *pAijtab=nM+p; pAijtab++; *pAijtab=qM; pAijtab++; }
					else/* just counting */
						{ ii++; ii++; }
					}
				}
			}
		}
	return(At);
}

static double
doocAij(siginv,isiginv,Pi,matrix,imatrix,Pj,flag)/* complex case */
register double *matrix,*imatrix,*siginv,*isiginv;
register short *Pj,*Pi;
register int flag;/* indicates the terms in the complex expansion */

/* Aij=trace Pi*siginv*Pj*matrix */
/* matrix is siginv or 2*siginv*S*siginv-siginv */
/* Aij= (Pi)mn*(siginv)np*(Pj)pq*(matrix)qm with Einstein summation convent. */
{
	register int m,n,nM,p,q,qM,qpr;
	register short *pPj;
	register double At;
	double doooA();

	qpr=qp;
	At=0;
	for(m=0;m<qpr;m++)
		{
		for(n=0,nM=0;n<qpr;n++,Pi++,nM+=qpr)
			{
			if(!*Pi)
				continue;
			pPj=Pj;
			for(p=0;p<qpr;p++)
				{
				for(q=0,qM=m;q<qpr;q++,qM+=qpr,pPj++)
					{
					if(!*pPj)
						continue;
			At+=doooA(siginv,isiginv,matrix,imatrix,nM+p,qM,flag,n,m,p,q);
					}
				}
			}
		}
	return(At);
}

static double /* do multiplication of matrices, see comments in Aijxxx */
doooA(siginv,isiginv,matrix,imatrix,i1,i2,flag,n,m,p,q)
register double siginv[],isiginv[],matrix[],imatrix[];
register int i1,i2,flag,n,m,p,q;
{
    register double term;

    if (!flag) {
	term = *(siginv + i1) * *(matrix + i2) - *(isiginv + i1) * *(imatrix + i2);
	return (term);
    }
    else if (flag == 1) {
	term = (siginv[i1] * imatrix[i2] + isiginv[i1] * matrix[i2]);
	if (q > p)
	    return (-term);
	else
	    return (term);
    }
    else /* if (flag == 2) */ {
    /* fact1=(n>m)?1:-1; fact2=(q>p)?1:-1; */
    /* here complex Pi are positive one in upper triangle */

	term = siginv[i1] * matrix[i2] - isiginv[i1] * imatrix[i2];
	if (((n > m) && (q > p)) || ((m > n) && (p > q)))
	    return (-term);
	else
	    return (term);
    }
}
/*
Aijxxx see also the discussion in doBi

Aij=trace[Pi*siginv*M*Pj] where M=2*siginv*S*siginv-siginv or =siginv

siginv*S*siginv is hermitian
M is hermitian since it is composed of hermitian matrices

Aij is hermitian,

Aij=trace[Pi*siginv*Pj*M]=trace[Pj*M*Pi*siginv]=trace[(M*Pj)'*(siginv*Pi)']=
trace[siginv*Pi*M*Pj]complex conjugate=trace[Pj*siginv*Pi*M]complex conjugate=
Aji complex conjugate, Q.E.D.

For M=siginv, Aij is symmetric and real
For the real case, Aij is symmtric and real

The Pi and Pj are symmetric, the iPi and iPj are anti-symmetric

trace [ Pi*siginv*Pj*matrix ]=

trace [ Pi*siginv*Pj*matrix ]+		flag=0
trace [ iPi*siginv*Pj*matrix ]+		anti-symmetric, trace zero
trace [ Pi*siginv*iPj*matrix ]+		anti-symmetric, trace zero
trace [ iPi*siginv*iPj*matrix ]+	flag=2
trace [ Pi*siginv*Pj*imatrix ]+		anti-symmetric, trace zero
trace [ iPi*siginv*Pj*imatrix ]+	term not computed directly, use Aji 
trace [ Pi*siginv*iPj*imatrix ]+	flag=1
trace [ iPi*siginv*iPj*matrix ]+	imaginary, does not contribute
trace [ Pi*isiginv*Pj*matrix ]+		anti-symmetric, trace zero
trace [ iPi*isiginv*Pj*matrix ]+	term not computed directly, use Aji 
trace [ Pi*isiginv*iPj*matrix ]+	flag=1
trace [ iPi*isiginv*iPj*matrix ]+	imaginary, does not contribute
trace [ Pi*isiginv*Pj*imatrix ]+	flag=0
trace [ iPi*isiginv*Pj*imatrix ]+	imaginary, does not contribute
trace [ Pi*isiginv*iPj*imatrix ]+	imaginary, does not contribute
trace [ iPi*isiginv*iPj*imatrix ]	flag=2

The Aij matrix is tNi by tNi, the Ni by Ni portion is due to the real
Pi, the others parts involve the imaginary iPi.

*/

static void
doBi(siginv,Pi,Bi,qp)	/* make Bi */
register double Bi[],siginv[];
register short *Pi;
register int qp;
/* Bi[i]=trace siginv*S*siginv*Pi */
{
	register int i,fNi,ftNi;
	register double Bt;
	double spdmtrace();

	fNi=Ni;
	ftNi=tNi;
	if(rflag)
	{
	dmmult(S,siginv,sigtemp,qp);/* make S*siginv */
	dmmult(siginv,sigtemp,sig1temp,qp);/* make siginv*S*siginv */
	if(andersn && (initflag==2 || initflag==3 || it_cnt>1) && rflag)
		{
		dmsub(sig1temp,siginv,sigtemp,qp);/* make siginv*S*siginv-siginv */
		/* sig1temp=siginv*S*siginv */
		/* sigtemp=siginv*S*siginv - siginv */
		}
	for(i=0;i<fNi;i++)
		{
		make(Pi,i);
		if(andersn && (initflag==2 || initflag==3 || it_cnt>1) && rflag)
			Bt=spdmtrace(sigtemp,Pi,qp);/*trace(siginv*S*siginv-siginv)Pi */
		else
			Bt=spdmtrace(sig1temp,Pi,qp);/*trace(siginv*S*siginv*Pi) */
		Bi[i]=Bt;
		}
	return;
	}

	/* complex case
	Bi is real

	Let M' be the complex transpose of M,
	if M=M', then M is hermitian, siginv and S and Pi are hermitian,
	and siginv*S*siginv=M is hermitian, so
	trace[M*Pi]=trace[Pi*M]=trace[(M*Pi)']=trace[M*Pi]complex cojugate,Q.E.D

	Bi=trace[M*Pi], M=siginv*S*siginv=Ms+iMa
	Ms is real and symmetric, iMa is imaginary and anti-symmetric
	Bi=trace[Ms*Pi] i=0 to Ni-1
	Bi=trace[iMa*iPi] i=Ni to tNi-1
	iPi is the anti-symmetrized Pi, with an i added
	*/

	/* make S*siginv in sigtemp */
	cdmmult(S,iS,siginv,isiginv,sigtemp,isigtemp,sig1temp,isig1temp,qp);

	/* make siginv*S*siginv in sig1temp */
	cdmmult(siginv,isiginv,sigtemp,isigtemp,sig1temp,isig1temp,sig2temp,isig2temp,qp);

	for(i=0;i<fNi;i++)
		{
		make(Pi,i);
		Bt=spdmtrace(sig1temp,Pi,qp);
		Bi[i]=Bt;
		}
	for(i=fNi;i<ftNi;i++)
		{
		make(Pi,i-fNi+1);
		antisymmetrize(Pi,qp);
		Bt=spdmtrace(isig1temp,Pi,qp);
		Bi[i]= -Bt;/* -1 because of product of two i's */
		}
	return;
}

static void
antisymmetrize(Pi,n)/* make the symmetric matric Pi antisymmetric, ones in upper triangle */
register short *Pi;
register int n;
{
	register int i,j,iM,jM;

	for(i=iM=0;i<n;i++,iM+=n)
		{
		for(j=i+1,jM=iM+n;j<n;j++,jM+=n)
			{
			if(*(Pi+iM+j))
				*(Pi+jM+i)= -1;
			}
		}
	return;
}

static void
doRnew(Ainv,Bi)/* make Ainv*Bi */
register double Ainv[],Bi[];
{
	register int i,j,iM,fNi,ftNi;
	register double Rt,*s,iRt;
	double dot();

	fNi=Ni;
	ftNi=tNi;
	for(i=0,s=Ainv;i<fNi;i++,s+=ftNi)
		{
		Rt=dot(s,1,Bi,1,ftNi);
		if(andersn && (initflag==2 || initflag==3 || it_cnt>1) && rflag)
			Rt+=R0[i];
		Rnew[i]=Rt;
		}
	if(cflag)
		{
		iRnew[0]=0;
		for(i=fNi,iM=fNi*ftNi;i<ftNi;i++,iM+=ftNi)
			{
			iRt=0;
			for(j=0;j<tNi;j++)
				iRt+=Ainv[iM+j]*Bi[j];
			iRnew[i-Ni+1]=iRt;
			}
		}
	return;
}

static void
makesc(Pi,s,qp)/* make single channel basis matrix */
register short Pi[];
register int s,qp;
{
	register int i,del,lim;
	register short *p,*pm,value;

	lim=qpxqp;
	value=0;
	for(i=0,p=Pi;i<lim;i++,p++)
		*p=value;
	lim=qp-s;
	value=1;
	del=qp+1;
	for(i=0,p=Pi+s,pm=Pi+s*qp;i<lim;i++,p+=del,pm+=del)
		{
		*p=value;
		*pm=value;
		}
	/*
	fprintf(stderr,"\n");
	for(i=0;i<qp;i++)
		{
		for(j=0;j<qp;j++)
			fprintf(stderr,"%d ",*(Pi+i*qp+j));
		fprintf(stderr,"\n");
		}
	*/
	return;
}

static void
make(Pi,s)
register short Pi[];
register int s;
{
	int i,j,k,l,P,a,b,c,kp,lp;
	register int m,n;

	if(scflag)
		{
		makesc(Pi,s,qp);
		return;
		}
	if(mdflag)
		{
		a=Ptaba[s]; b=Ptabb[s];
		for(k=0,kp=0;k<qdim;k++,kp+=pdim)
			{
			for(l=k,lp=kp;l<qdim;l++,lp+=pdim)
				{
				for(i=0;i<pdim;i++)
					{
					j=0;
					if(l==k)j=i;
					for(;j<pdim;j++)
						{
						m=kp+i;
						n=lp+j;
						P=0;
						if( (a == (l-k)) && (b == (j-i))) P=1;
						Pi[m*qp+n]=Pi[n*qp+m]=P;
						}
					}
				}
			}
		return;
		}
	if(mcflag)
		{
		a=Ptaba[s];
		b=Ptabb[s];
		c=Ptabc[s];
		for(k=0,kp=0;k<qdim;k++,kp+=pdim)
			{
			for(l=k,lp=kp;l<qdim;l++,lp+=pdim)
				{
				for(i=0;i<pdim;i++)
					{
					j=0;
					if(l==k)j=i;
					for(;j<pdim;j++)
						{
						m=kp+i;
						n=lp+j;
						P=0;
						if((a == (l-k)) && (b == (j-i)))
							{
							if(b==0 && i==c)P=1;
							if(b!=0) P=1;
							}
						Pi[m*qp+n]=Pi[n*qp+m]=P;
						}
					}
				}
			}
		return;
		}
	fprintf(stderr,"trouble in make().\n");
	exit(1);
}

static void
doPtab()
{
	int i,a,b,j,isum,k,l;

	if(scflag || mdflag)
		{
		i=0;
		a=0;
		for(b=0;b<pdim;b++)
			{
			Ptaba[i]=a;
			Ptabb[i]=b;
			i++;
			}
		for(a=1;a<qdim;a++)
			{
			for(b= -(pdim-1);b<pdim;b++)
				{
				Ptaba[i]=a;
				Ptabb[i]=b;
				i++;
				}
			}
		}
	if(mcflag)
		{
		i=0;
		a=0;
		b=0;
		for(j=0;j<pdim;j++)
			{
			Ptaba[i]=a;
			Ptabb[i]=b;
			Ptabc[i]=j;
			i++;
			}
		for(b=1;b<pdim;b++)
			{
			Ptaba[i]=a;
			Ptabb[i]=b;
			i++;
			}
		for(a=1;a<qdim;a++)
			{
			for(b= -(pdim-1);b<pdim;b++)
				{
				if(b==0)
					{
					for(j=0;j<pdim;j++)
						{
						Ptaba[i]=a;
						Ptabb[i]=b;
						Ptabc[i]=j;
						i++;
						}
					}
				else
					{
					Ptaba[i]=a;
					Ptabb[i]=b;
					i++;
					}
				}
			}
		}
	if(i!=Ni) { fprintf(stderr,"Ptab not ok\n");exit(1); }

	for(i=0;i<Ni;i++)
		{
		make(Pi,i);
		for(j=i;j<Ni;j++)
			{
			make(Pj,j);
			isum=0;
			for(k=0;k<qp;k++)
				{
				for(l=0;l<qp;l++)
					{
					if(!Pi[k*qp+l])continue;
					if(!Pj[l*qp+k])continue;
					isum++;
					}
				}
			if(isum!=0 && i!=j) { fprintf(stderr,"pij!=0\n");exit(1); }
			if(i!=j)continue;
			if(isum==0) { fprintf(stderr,"Pii=0\n"); }
			Pii[i]=isum;
			}
		}
	return;
}

static void
makesigma(R,iR,sigma,isigma,Pi)
register double R[],iR[],sigma[],isigma[];
register short Pi[];
{
	register int i,m,n,nM,mM;
	register double Rt,iRt;

	dmclear(sigma,qp);
	if(cflag)
		dmclear(isigma,qp);

	for(i=0;i<Ni;i++)
		{
		make(Pi,i);
		Rt=R[i];
		if(cflag)iRt=iR[i];
		for(m=0,mM=0;m<qp;m++,mM+=qp)
			{
			for(n=m;n<qp;n++)
				{
				if(!Pi[mM+n])continue;
				sigma[mM+n]+=Rt;
				if(cflag)
					isigma[mM+n]+=iRt;/* iPi one's in upper triangle */
				}
			}
		}
	for(m=0,mM=0;m<qp;m++,mM+=qp)
		{
		for(n=m+1,nM=mM+qp;n<qp;n++,nM+=qp)
			{
			sigma[nM+m]=sigma[mM+n];
			if(cflag)
				isigma[nM+m]= -isigma[mM+n];
			}
		}
	return;
}

static void
gb_free()
{
	freedp(&R);
	freedp(&R0);
	freedp(&Rnew);
	freedp(&del);
	freedp(&siginv);
	freedp(&sigtemp);
	freedp(&sig1temp);
	freedp(&sig2temp);
	freesp(&Pii);
	freesp(&Pi);
	freesp(&Pj);
	freesp(&Ptaba);
	freesp(&Ptabb);
	freesp(&Ptabc);
	freedp(&Aij);
	freedp(&Ainv);
	freedp(&M);
	freedp(&Bi);
	freedp(&tBi);
	freedp(&ttBi);
	freedp(&tttBi);
	if(cflag)
		{
		freedp(&iR);
		freedp(&iR0);
		freedp(&iRnew);
		freedp(&idel);
		freedp(&isiginv);
		freedp(&isigtemp);
		freedp(&isig1temp);
		freedp(&isig2temp);
		}
	return;
}

static int
do_alloc()
{
	if(allocsp(&Pii,Ni))return(1);
	if(allocsp(&Pi,qpxqp))return(1);
	if(allocsp(&Pj,qpxqp))return(1);
	if(allocsp(&Ptaba,Ni))return(1);
	if(allocsp(&Ptabb,Ni))return(1);
	if(mcflag)
		{ if(allocsp(&Ptabc,Ni))return(1); }
	if(allocdp(&R,Ni))return(1);
	if(allocdp(&R0,Ni))return(1);
	if(allocdp(&Rnew,Ni))return(1);
	if(allocdp(&del,Ni))return(1);
	if(allocdp(&siginv,qpxqp))return(1);
	if(allocdp(&Aij,tNi*tNi))return(1);
	if(allocdp(&Ainv,tNi*tNi))return(1);
	if(allocdp(&M,tNi*tNi))return(1);
	if(allocdp(&Bi,tNi))return(1);
	if(allocdp(&tBi,tNi))return(1);
	if(allocdp(&ttBi,tNi))return(1);
	if(allocdp(&tttBi,tNi))return(1);
	if(allocdp(&sigtemp,qpxqp))return(1);
	if(allocdp(&sig1temp,qpxqp))return(1);
	if(allocdp(&sig2temp,qpxqp))return(1);

	if(cflag)
		{
		if(allocdp(&iR,Ni))return(1);
		if(allocdp(&iR0,Ni))return(1);
		if(allocdp(&iRnew,Ni))return(1);
		if(allocdp(&idel,Ni))return(1);
		if(allocdp(&isiginv,qpxqp))return(1);
		if(allocdp(&isigtemp,qpxqp))return(1);
		if(allocdp(&isig1temp,qpxqp))return(1);
		if(allocdp(&isig2temp,qpxqp))return(1);
		}
	return(0);
}

static void
setdel()/* set del vector for interpolation */
{
	if(andersn && (initflag==2 || initflag==3 || it_cnt>1) && rflag)
		dvmove(Rnew,del,Ni);
	else
		{
		dvsub(Rnew,R0,del,Ni);
		if(cflag)
			dvsub(iRnew,iR0,idel,Ni);
		}
	return;
}

static int
interp()	/* long interpolation, by tenths of interval  */
{
	double Fact,lim;
	register double fact,ssfact,sfact;
	double dointerppoint();

	setdel();/* setup del vector */
	sfact=0;
	for(Fact=.1;sfact==0 && Fact>small;Fact/=10)
		{
		lim=10.1*Fact;
		for(fact=0;fact<=lim && !((fact+Fact)==fact);fact+=Fact)
			{
			/* check interpolated point for positive
			definitness and save in R0, and save in
			R if it has a lower measure */

			if((ssfact=dointerppoint(fact)))
				sfact=ssfact;
			}
		}
	if(sfact)update(sfact);/* update R0 and monitor if moflag */
	return(sfact?0:1);/* return indication of sucess or not */
}

static void
update(sfact)/* maybe update R0 and monitor */
double sfact;
{
	R_R0();/* save best R in R0 */
	inv_det(R0,iR0,siginv,isiginv,&R0_det);/* get inverse and determinant */

	if(moflag)
		{
		fprintf(stderr,"maxfact=%g\n",sfact);
		fprintf(stderr,"R(%g)=\n",sfact);
		dvprintf(R,Ni);
		if(cflag)dvprintf(iR,Ni);
		fprintf(stderr,"ln|R(%g)|=%g\n",sfact,R_det);
		fprintf(stderr,"d(S,R(%g))=%.15g\n\n",sfact,R_measure);
		}
	return;
}

static double
dointerppoint(fact)/* do interpolation point for fact */
register double fact;
{
	register int rtflag;

	spmult(R0,fact,del,Rnew,Ni);/* form Rnew[]=R0[]+fact*del[] */
	if(cflag)
		spmult(iR0,fact,idel,iRnew,Ni);

	rtflag=inv_det(Rnew,iRnew,siginv,isiginv,&Rnew_det);
		 /* 0 Rnew pos-def
		    1 Rnew singular, Rnew_det=0
		    2 Rnew non-pos-def, Rnew_det=0 */
	Rnew_measure=domeasure(siginv,isiginv,Rnew_det);

	if(moflag)
		{
		fprintf(stderr,"%g %.10g ",fact,Rnew_measure);
		if(rtflag==0)fprintf(stderr,"\n");
		if(rtflag==1)fprintf(stderr,"singular or negative def\n");
		if(rtflag==2)fprintf(stderr,"npd\n");
		}
	if((Rnew_measure<R_measure) && rtflag==0)
		{
		Rnew_R();/* update R[] with Rnew[] */
		return(fact);/* sucessful point */
		}
	else
		return((double)0);/* not a sucessful point */
}

static void
Rnew_R()/* update R[] with Rnew[] */
{
	R_measure=Rnew_measure;
	R_det=Rnew_det;
	dvmove(Rnew,R,Ni);
	if(cflag)
		dvmove(iRnew,iR,Ni);
	return;
}

static void
Rnew_R0()/* move Rnew[] to R0[] */
{
	R0_det=Rnew_det;
	R0_measure=Rnew_measure;
	dvmove(Rnew,R0,Ni);
	if(cflag)
		dvmove(iRnew,iR0,Ni);
	return;
}

static void
R_R0()/* move R[] vector to R0[] vector */
{
	R0_det=R_det;
	R0_measure=R_measure;
	dvmove(R,R0,Ni);
	if(cflag)
		dvmove(iR,iR0,Ni);
	return;
}

static int
iinterp()	/* interpolation by 1/2 */
{
	register double ssfact,sfact,Fact;
	double dointerppoint();

	setdel();/* setup del vector */
	sfact=0;
	for(Fact=.5;sfact==0 && Fact>small;Fact/=2)
		{
		if((ssfact=dointerppoint(Fact)))
			sfact=ssfact;
		}
	if(sfact)update(sfact);/* update R0 and maybe monitor */
	return(sfact?0:1);
}

static void
spmult(p1,f,p2,p3,n)/* form the vector p3=p1+f*p2 */
register double *p1,f,*p2,*p3;
register int n;
{
	register int i;

	for(i=0;i<n;i++,p1++,p2++,p3++)
		*p3= *p1 + f*(*p2);
	return;
}

static void
exR(sigma,isigma,R,iR,qp,Ni)
/* extract R from sigma */
register int qp,Ni;
register double sigma[],isigma[],R[],iR[];
{
	register int i;
	int n,m,nM,mM;
	register double temp,trace;
	double spdmtrace();

	for(i=0;i<Ni;i++)
		{
		make(Pi,i);
		trace=spdmtrace(sigma,Pi,qp);
		temp=trace/Pii[i];
		R[i]=temp;
		if(cflag)
			{
			for(m=0,mM=0;m<qp;m++,mM+=qp)
				{
				for(n=m+1,nM=mM+qp;n<qp;n++,nM+=qp)
					Pi[nM+m]= -Pi[mM+n];
				}
			trace=spdmtrace(isigma,Pi,qp);
			temp= -trace/Pii[i];
			iR[i]=temp;
			}
		}
	return;
}

static void
set_up()
{
	if(scflag)
		{
		pdim=1;
		Ni=qdim;
		qp=qdim;
		tNi=Ni;
		if(cflag)
			tNi= 2*Ni-1;
		}
	if(mdflag)
		{
		qp=qdim*pdim;
		Ni= 2*qp-qdim-pdim+1;
		tNi=Ni;
		if(cflag)
			tNi= 2*Ni-1;
		}
	if(mcflag)
		{
		pdim=2;
		qp=qdim*pdim;
		Ni= 2*qp-qdim-pdim+1;
		Ni+=qdim;	/* not general */
		tNi=Ni;
		if(cflag)
			tNi= 2*Ni-2;
		}
	qpxqp=qp*qp;
	return;
}

static void
getdist(sigma_in, qd, pdist, R)
double *sigma_in, *pdist;
double  R[];
int     qd;
{
    double  isigma_in[10];
    double  iR[10];
    double gain;
    double log(), dmtrace();

    /* assign input values to internal variables */

    cntAij = 0;
    S = sigma_in;
    iS = isigma_in;
    qdim = qd;

    set_up ();			/* calculate constants */

    if (do_alloc ())		/* allocate storage */
    {
	fprintf (stderr, "Storage allocation not possible for genburg().\n");
	gb_free ();
	return;
    }

/* test S for positive definitness with cholesky and get ln|S| */

    dmmove (S, sigtemp, qp);
    (void) lsyminv (sigtemp, siginv, sig1temp, qp, &logdetS);

/* get inverse and ln|R| to do initial measure */

    (void) inv_det (R, iR, siginv, isiginv, &R_det);
    gain = dmtrace ( siginv, S, qp);
    gain = gain / qp;
    *pdist = log (gain) * qp + R_det -logdetS;
    gb_free ();			/* free storage */
    return;
}


/* THE STUFF BELOW USED TO BE IN toeplib.c */


#define pi 3.141592653589792
#define two_pi 6.283185307179584
#define dcomplex struct {double R;double I;}
#define SMALL .00000000000000004
#define LARGE 1.705099e38


static int
allocdp(p_p_block,n)	/* allocate a double pointer */
int n;
double **p_p_block;
{
	/*
	assume an 8 byte 64 bit double  
	program calls malloc to establish storage for a block of n doubles
	p_block is a pointer to the block
	p_p_block is a pointer to the pointer p_block

	return(0) for sucessful allocation
	return(1) for unsucessful allocation
	*/

	char *p_char;
	unsigned size;

	if(*p_p_block!=0)
		{
		fprintf(stderr, "allocdp expects a null p_block\n");
		exit(1);
		}
	size= 8*n;
	p_char=malloc(size);
	if(p_char==0)return(1);
	*p_p_block = (double *)p_char;
	return(0);
}

static int
allocsp(p_p_block,n)	/* allocate a short pointer */
int n;
short **p_p_block;
{
	/*
	assume a 2 byte 16 bit short
	program calls malloc to establish storage for a block of n shorts
	p_block is a pointer to the block
	p_p_block is a pointer to the pointer p_block

	return(0) for sucessful allocation
	return(1) for unsucessful allocation
	*/

	char *p_char;
	unsigned size;

	if(*p_p_block!=0)
		{
		fprintf(stderr, "allocsp expects a null p_block\n");
		exit(1);
		}
	size= 2*n;
	p_char=malloc(size);
	if(p_char==0)return(1);
	*p_p_block =(short *) p_char;
	return(0);
}

static void
cdmmult(A,iA,B,iB,C,iC,M,iM,n)/* multiply complex matrices */
/* M and iM are temporary matrices */
/* C=A*B */
/* A+iA' etc */
/* C=A*B - A'*B' */
/* iC=A*B' + A'*B */
register double *A,*iA,*B,*iB,*C,*iC,*M,*iM;
register int n;
{
	dmmult(A,B,M,n);
	dmmult(iA,iB,iM,n);
	dmsub(M,iM,C,n);
	dmmult(A,iB,M,n);
	dmmult(iA,B,iM,n);
	dmadd(M,iM,iC,n);
	return;
}

static int
cholesky(M,A,n)	/* routine destroys M but finds A such that M=At*A */
		/* M symmetric (toeplitz?) A upper triangular */
		/* 0 normal return, 1 M singular, 2 M negative definite (???) */
register int n;
register double *M,*A;
{
	register int i,j,k,iM,jM;
	double Mii,Aii;
	register double dtemp,fact;
	double sqrt();

	for(i=iM=0;i<n;i++,iM+=n)
		{
		if((Mii=M[iM+i]) == 0 )return(1);
		if(Mii<0)return(2);
		Aii=sqrt(Mii);
		A[iM+i]=Aii;
		fact=1/Aii;
		for(j=i+1;j<n;j++)
			{
			dtemp=M[iM+j]*fact;
			A[iM+j]=dtemp;
			}
		for(j=i+1,jM=iM+n;j<n;j++,jM+=n)
			{
			fact=A[iM+j];
			for(k=j;k<n;k++)
				{
				dtemp=fact*A[iM+k];
				M[jM+k]-=dtemp;
				}
			}
		}

/* clear lower triangle */

	fact=0;
	for(i=0;i<n;i++,A+=n)
		{
		for(j=0;j<i;j++)
			*(A+j)=fact;
		}
	return(0);
}

static int
cnormal(R,iR,a,ia,p,n)
	/* rtflag=1 singular, rtflag=2 non-pos-def */
double R[],iR[],a[],ia[],p[];
register int n;
{
	register int i,j,ss,s,iM;
	int rtflag;
	register double ci,ici,po,pf,Rt,iRt,at,iat,temp;

	rtflag=0;

	for(i=0,iM=0;i<n;i++,iM+=n)
		{
		a[iM]=1;
		ia[iM]=0;
		for(j=i+1;j<n;j++)
			a[iM+j]=ia[iM+j]=0;
		}
	p[0]=R[0];
	for(i=1,s=n;i<n;i++,s+=n)
		{
		ss=s-n;
		ci=R[i];
		ici=iR[i];
		for(j=1;j<i;j++)
			{
			Rt=R[i-j];
			iRt=iR[i-j];
			at=a[ss+j];
			iat=ia[ss+j];
			ci+= at*Rt-iat*iRt;
			ici+= at*iRt+iat*Rt;
			}
		if((po=p[i-1])==0)return(1);
		ci= -ci/po;
		ici= -ici/po;
		a[s+i]=ci;
		ia[s+i]=ici;
		for(j=1;j<i;j++)
			{
			temp=a[ss+j]+ci*a[ss+i-j]+ici*ia[ss+i-j];
			a[s+j]=temp;
			temp=ia[ss+j]-ci*ia[ss+i-j]+ici*a[ss+i-j];
			ia[s+j]=temp;
			}
		pf=(1-(ci*ci+ici*ici))*po;
		p[i]=pf;

		if(pf<0)rtflag++;

		if(pf==0)return(1);

		}
	if(rtflag>0)return(2);
	return(0);
}

/*
acos(arg)

0<= acos <=pi
Arctan is called after appropriate range reduction.
*/


static void
dmadd(A,B,C,n)/* matrices C=A+B */
register double *A,*B,*C;
register int n;
{
	register int i,lim;

	lim=n*n;
	for(i=0;i<lim;i++,C++,A++,B++)
		*C= *A + *B;
	return;
}
static void
dmclear(M,n)/* clear matrix */
register double *M;
register int n;
{
	register int i,lim;
	register double zero;

	lim=n*n;
	zero=0;
	for(i=0;i<lim;i++,M++)
		*M=zero;
	return;
}



static void
dmmove(A,B,n)/* move matrix A to matrix B */
register double *A,*B;
register int n;
{
	register int i,lim;

	lim=n*n;
	for(i=0;i<lim;i++,A++,B++)
		*B= *A;
	return;
}

static void
dmmult(a,b,c,n)/* multiply of matrices */
/* c=a*b */
register double *a,*b,*c;
register int n;
{
	register int i,j;
	register double *s;
	double dot();

	for(i=0;i<n;i++,a+=n)
		{
		for(j=0,s=b;j<n;j++,s++,c++)
			*c=dot(a,1,s,n,n);
		}
	return;
}

static void
dmsub(A,B,C,n)/* matrices C=A-B */
register double *A,*B,*C;
register int n;
{
	register int i,lim;

	lim=n*n;
	for(i=0;i<lim;i++,C++,A++,B++)
		*C= *A - *B;
	return;
}

static double
dmtrace(M1,M2,n)/* returns trace of M1*M2, M1 and M2 matrices  */
register double *M1,*M2;
register int n;
{
	register int i;
	register double trace;
	double dot();

	trace=0;
	for(i=0;i<n;i++,M1+=n,M2++)
		trace+=dot(M1,1,M2,n,n);
	return(trace);
}

static void
dojohnavg(M,Mout,n)/* John's averaging process for matrices */
register double *M,*Mout;
register int n;
{
	register int i,j,iM,jM;
	register double term,diag;
	double tr;
	double trace();

	tr=trace(M,n)*2/n;

	for(i=iM=0;i<n;i++,iM+=n)
		{
		diag=M[iM+i];
		for(j=jM=0;j<n;j++,jM+=n)
			{
			if(i==j)continue;
			term=M[iM+j]*tr/(diag+M[jM+j]);
			Mout[iM+j]=term;
			}
		}
	term=tr/2;
	for(i=0;i<n;i++,Mout+=n+1)
		*Mout=term;
	return;
}

static double 
dot(a,na,b,nb,n)/* dot product a and b, na is a increment, mb is b increment */
register double *a,*b;
register int na,nb,n;
{
	register int i;
	register double sum;

	sum=0;
	for(i=0;i<n;i++,a+=na,b+=nb)
		sum+=(*a)*(*b);
	return(sum);
}

static void
dvclear(M,n)/* clear vector */
register double *M;
register int n;
{
	register int i;
	register double zero;

	zero=0;
	for(i=0;i<n;i++,M++)
		*M=zero;
	return;
}

static void
dvmove(x,y,n)/* move vector x to y */
register double *x,*y;
register int n;
{
	register int i;
	for(i=0;i<n;i++,y++,x++)
		*y= *x;
	return;
}

static void
dvprintf(x,n)
double x[];
int n;
{
	int i;
	for(i=0;i<n;i++)
		fprintf(stderr, "%f ",x[i]);
	fprintf(stderr, "\n");
	return;
}

static void
dvsub(A,B,C,n)/* vectors C=A-B */
register double *A,*B,*C;
register int n;
{
	register int i;

	for(i=0;i<n;i++,C++,A++,B++)
		*C= *A - *B;
	return;
}

static void
freedp(p_p_block)
double **p_p_block;
{
	char *p_char;

	p_char =(char *) *p_p_block;
	if(p_char==0)return;
	free(p_char);
	*p_p_block=0;
	return;
}


static void
freesp(p_p_block)
short **p_p_block;
{
	char *p_char;

	p_char =(char *) *p_p_block;
	if(p_char==0)return;
	free(p_char);
	*p_p_block=0;
	return;
}




/*
Given the range xmin to xmax and the number of points N on the range
( including the end points ) and the function name for computing the
value of the function, find the minimum value of the function and the
x that produces that minimum.
Assume only one minimum in range and no maxima except at the ends of the range.
*/

static void
get_min(xmin,xmax,N,function,min_x,min_y,level,flag)
int N,level,flag;
double xmin,xmax,*min_x,*min_y;
double (*function)();
{
	double x,del_x,y;
	double new_xmin,new_xmax;
	double n_min_x,n_min_y;
	int i,s_i;

	del_x=(xmax-xmin)/(N-1);
	*min_y= 1e30;
	for(i=0;i<N;i++)
		{
		x=xmin+i*del_x;
		y=(*function)(x);
		fprintf(stderr, "x=%g y=%g\n",x,y);
		if(*min_y>y)
			{
			/* function decreasing */
			*min_y=y;
			*min_x=x;
			s_i=i;/* save position in table of minimum */
			}
		else
			{
			if(i && !flag)
				/* function increasing */
				break;
			}
		}
	if(level>1)
		{
		if(s_i==0)
			{
			new_xmin=xmin;
			new_xmax=xmin+del_x;
			}
		else if(s_i==(N-1))
			{
			new_xmin=xmax-del_x;
			new_xmax=xmax;
			}
		else
			{
			new_xmin=xmin+(s_i-1)*del_x;
			new_xmax=xmin+(s_i+1)*del_x;
			}
	get_min(new_xmin,new_xmax,N,function,&n_min_x,&n_min_y,level-1,flag);
		if(n_min_y<*min_y)
			{
			*min_y=n_min_y;
			*min_x=n_min_x;
			}
		}
	return;
}


static double 
hdmtrace(M1,iM1,M2,iM2,n)
/* M1 and M2 are hermitian matrices, hdmtrace produces the trace of the
product of the two matrices, which is real */
double M1[],M2[],iM1[],iM2[];
int n;
{
	int j,k,jM,kM;
	double sum;
	sum=0;
	for(j=0,jM=0;j<n;j++,jM+=n)
		{
		for(k=0,kM=0;k<n;k++,kM+=n)
			{
			sum+=(M1[jM+k]*M2[kM+j]-iM1[jM+k]*iM2[kM+j]);
			}
		}
	return(sum);
}

static int
lctoepinv(R,iR,Rinv,iRinv,pdet,n)
	/* natural log of determinant is returned */
	/* R a vector of length n, Rinv a matrix of rank n */
register double R[],iR[],Rinv[],iRinv[],*pdet;
register int n;
{
	register int l,i,iM,j,jM,ss;

	int N,rtflag,toggle;
	static double *a,*ia,*p,Rt,iRt;
	register double det,pt,dtemp;
	double log();

	N=n-1;

	if(allocdp(&a,n*n))
		{
		fprintf(stderr, "ctoepinv not possible due to storage limitations\n");
		exit(1);
		}
	if(allocdp(&ia,n*n))
		{
		freedp(&a);
		fprintf(stderr, "ctoepinv not pollsible due to storage limitations\n");
		exit(1);
		}
	if(allocdp(&p,n))
		{
		freedp(&a);
		freedp(&ia);
		fprintf(stderr, "ctoepinv not possible due to storage limitations\n");
		exit(1);
		}
	
	rtflag=cnormal(R,iR,a,ia,p,n);
	if(rtflag==1)
		{
		freedp(&a);
		freedp(ia);
		freedp(&p);
		*pdet=0;
		return(1);
		}
	
	/* if(rtflag==2) fprintf(stderr, "ctoep non-pos-definite\n"); */

	det=0;
	if(rtflag==0)
		{
		for(i=0;i<n;i++)
			det+=log(p[i]);
		}
	if(rtflag==2)
		{
		toggle=1;
		for(i=0;i<n;i++)
			{
			if((pt=p[i])<0)
				{
				toggle= -toggle;
				pt= -pt;
				}
			det+=log(pt);
			}
		if(toggle!=1)
			det=0;
		}
	*pdet=det;

	for(i=0,iM=0;i<n;i++,iM+=n)
		{
		for(j=i,jM=i*n;j<n;j++,jM+=n)
			{
			Rt=0;
			iRt=0;
			for(l=0,ss=N*n;l<=i;l++,ss-=n)
				toepmult(a[ss+i-l],-ia[ss+i-l],a[ss+j-l],ia[ss+j-l],&Rt,&iRt,p[N-l]);
			Rinv[jM+i]=Rinv[iM+j]=Rt;
			iRinv[iM+j]=iRt;
			if(i!=j)
				{
				dtemp= -iRt;
				iRinv[jM+i]=dtemp;
				}
			}
		}
	
	freedp(&a);
	freedp(&ia);
	freedp(&p);
	return(rtflag);
}

static int
ldmsyminv(M1,M2,pdet,n,flag)
/* this routine appears to give the inverse for non symmetric matrices */
/* produces the inverse of the symmetric matrix M1 in M2 */
/* M2 can =M1, M1 is destroyed unless flag=1 */
/* returns 0 for non-singular matrix, 1 for singular matrix */
/* return 2 for storage allocation problems */
/* the determinant is returned as the natural log */
/* det is zero for singular case and npd case */
register double M1[],M2[],*pdet;
register int n,flag;
{
	register int rtflag,nxn;
	static double *M3,*M4;
	double det,t0,t2,t3,dtemp;
	double log();

	if(n==1)
		{
		if(M1[0]<=0)
			*pdet=0;
		else *pdet=log(M1[0]);
		if(M1[0]==0)
			return(1);
		dtemp=1/M1[0];
		M2[0]=dtemp;
		return(0);
		}
	if(n==2)
		{
		det=M1[0]*M1[3]-M1[2]*M1[1];
		if(det<=0)
			*pdet=0;
		else *pdet=log(det);
		if(det==0)
			return(1);
		t0=M1[3]/det;
		t3=M1[0]/det;
		t2= -M1[1]/det;
		M2[3]=t3;
		M2[0]=t0;
		M2[2]=M2[1]=t2;
		return(0);
		}
	nxn=n*n;
	if( (flag==1) || M1==M2 )
		{
		if(allocdp(&M3,nxn) || allocdp(&M4,nxn))
			{
			fprintf(stderr, "ldmsyminv not possible due to storage allocation problems\n");
			return(2);
			}
		dmmove(M1,M4,n);/* move M1 to M4 */
		rtflag=lorthogin(M4,M2,M3,pdet,n);
		freedp(&M3);
		freedp(&M4);
		if(rtflag)
			return(1);
		return(0);
		}
	if(allocdp(&M3,nxn))
		{
		fprintf(stderr, "ldmsyminv not possible due to storage allocation problems\n");
		return(2);
		}
	rtflag=lorthogin(M1,M2,M3,pdet,n);
		freedp(&M3);
	if(rtflag)
		return(1);
	return(0);
}

static int
lorthogin(M,Minv,Tinv,pdet,n)
register double *M,*Minv,*Tinv,*pdet;
register int n;
/* returns log(det) , 0 if negative definite or singular */
{
	register int i,j,toggle;
	register double Mt,*s,*s1;
	double log(),dot();

	if(orthog(M,Minv,n))
		{
		*pdet=0;
		return(1);
		}
	/* orthog returns an orthogonal matrix O in M and an upper triagonal
	matrix T in Minv such that M=O*T */

	*pdet=0;
	toggle=1;
	j=n+1;
	for(i=0,s=Minv;i<n;i++,s+=j)
		{
		if((Mt= *s)<0)
			{
			toggle= -toggle;
			Mt= -Mt;
			}
		if(!Mt)
			{
			toggle= -1;
			break;
			}
		*pdet+=log(Mt);
		}
	if(toggle!=1)
		*pdet=0;

	if(triinv(Minv,Tinv,n))
		return(1);
	for(i=0,s=Minv;i<n;i++,Tinv+=n)
		{
		for(j=0,s1=M;j<n;j++,s1+=n,s++)
			*s=dot(Tinv,1,s1,1,n);
		}
	return(0);
}

static int
lsyminv(M,Minv,Temp,n,pdet)	/* M symmetric (toeplitz?) positive definite */
				/* Minv=Ainv*Ainvt */
		/* cholesky finds A, such that M=At*A, A upper triangular */
		/* triinv finds Ainv */
		/* return 0 normal, 1 M singular, 2 M negative definite (???) */
		/* returns ln det of M in pdet */
register int n;
double M[],Minv[],Temp[],*pdet;
{
	register double sum;
	register int i,j,k,iM,jM;
	int rtflag;
	double log();

	for(i=iM=0;i<n;i++,iM+=n)
		{
		for(j=0;j<n;j++)
			Temp[iM+j]=M[iM+j];
		}
	if((rtflag=cholesky(Temp,Minv,n)))
		{
		*pdet=0;
		return(rtflag);
		}
	/* Minv contains A, Temp scribbled */
	if(triinv(Minv,Temp,n))
		return(1);
		/* Temp==Ainv, Minv==A not scribbled, do det */
	sum=0;
	for(i=iM=0;i<n;i++,iM+=n)
		sum+=log(Minv[iM+i]);
	*pdet=2*sum;/* return ln det M */
	for(i=iM=0;i<n;i++,iM+=n)
		{
		for(j=i,jM=i*n;j<n;j++,jM+=n)
			{
			sum=0;
			for(k=0;k<n;k++)
				sum+=Temp[iM+k]*Temp[jM+k];
			Minv[jM+i]=Minv[iM+j]=sum;
			}
		}
	return(0);
}

static int
ltoepinv(R,Rinv,pdet,n)
	/* natural log of determinant is returned */
double R[],Rinv[],*pdet;
register int n;
{
	register int l,i,iM,j,jM,ss;
	int N;
	int rtflag,toggle;
	double det,Rt,pt,log();
	static double *a,*p;

	N=n-1;

	if(allocdp(&a,n*n))
		{
		fprintf(stderr, "ltoepinv not possible due to storage limitations\n");
		exit(1);
		}
	if(allocdp(&p,n))
		{
		freedp(&a);
		fprintf(stderr, "ltoepinv not possible due to storage limitations\n");
		exit(1);
		}

	rtflag=normal(R,a,p,n);
	/* rtflag=1 singular, rtflag=2 non-pos-def */

	if(rtflag==1)
		{
		freedp(&a);
		freedp(&p);
		*pdet=0;
		return(1);
		}

	/* if(rtflag==2) fprintf(stderr, "non-minimum phase\n"); */

	det=0;
	if(rtflag==0)
		{
		for(i=0;i<n;i++)
			det+=log(p[i]);
		}
	if(rtflag==2)
		{
		toggle=1;
		for(i=0;i<n;i++)
			{
			if((pt=p[i])<0)
				{
				toggle= -toggle;
				pt= -pt;
				}
			det+=log(pt);
			}
		if(toggle!=1)
			det=0;
		}
	*pdet=det;

	for(i=0,iM=0;i<n;i++,iM+=n)
		{
		for(j=i,jM=i*n;j<n;j++,jM+=n)
			{
			Rt=0;
			for(l=0,ss=N*n;l<=i;l++,ss-=n)
				Rt+= a[ss+i-l]*a[ss+j-l]/p[N-l];
			Rinv[jM+i]=Rinv[iM+j]=Rt;
			}
		}

	freedp(&a);
	freedp(&p);
	return(rtflag);
}

static int
normal(R,a,p,n)
	/* rtflag=1 if singular, rtflag=2 if non-pos-definite */
double R[],a[],p[];
register int n;
{
	register int i,j,ss,s,iM;
	int rtflag;
	register double ci,po,pf,temp;

	rtflag=0;

	for(i=0,iM=0;i<n;i++,iM+=n)
		{
		a[iM]=1;
		for(j=i+1;j<n;j++)
			a[iM+j]=0;
		}
	p[0]=R[0];
	for(i=1,s=n;i<n;i++,s+=n)
		{
		ss=s-n;
		ci=R[i];
		for(j=1;j<i;j++)
			ci+= a[ss+j]*R[i-j];
		if((po=p[i-1])==0)return(1);
		ci= -ci/po;
		if(ci<-1 || ci>1)
			rtflag++;
		a[s+i]=ci;
		for(j=1;j<i;j++)
			{
			temp=a[ss+j]+ci*a[ss+i-j];
			a[s+j]=temp;
			}
		pf=(1-ci*ci)*po;
		p[i]=pf;
		if(pf==0)return(1);
		}
	if(rtflag>0)return(2);
	return(0);
}

static int
orthog(M,T,n)
/* routine returns in M an orthogonal matrix O
and in T an upper triangular matrix T such that M=O*T */
/* if M non-singular, T non-singular */
register double *M,*T;
register int n;
{
	register int i,j,k;
	register double *s,*c,*sT,*s1T,*sM;
	double sqrt(),dot();

	for(k=0,s1T=T,c=M;k<n;k++,s1T+=n+1,c++)
		{
		if(k>0)
			{
			for(j=0,sT=T+k,sM=M;j<k;j++,sT+=n,sM++)
				{
				*sT=dot(sM,n,c,n,n);
				sp1(*sT,sM,c,n);
				}
			}
		(*s1T)=dot(c,n,c,n,n);
		if((*s1T)<=0)
			return(1);

		*s1T=sqrt(*s1T);
		for(i=0,s=c;i<n;i++,s+=n)
			*s/= *s1T;
		}
	return(0);
}

static void
sp1(p1,p2,p3,n)
register double p1,*p2,*p3;
register int n;
{
	register int i;

	for(i=0;i<n;i++,p2+=n,p3+=n)
		*p3 -= p1 * *p2;
	return;
}

static int
posdefsol(M,A,B,x,y,t,t1,n)/* solves M*x=y, M pos definite,
A, B temp matrices, t and t1 temp vectors */
register double A[],B[],M[],x[],y[],t[],t1[];
register int n;
{
	register int rtflag,i;

	dmmove(M,B,n);/* save M, move M to B which is destroyed */
	rtflag=cholesky(B,A,n);	/* routine destroys B but finds A such that B=At*A */
	if(rtflag)
		return(rtflag);
	for(i=0;i<n;i++)
		t1[i]= -y[i];
	solchol(A,t1,x,t,n);
	/* A upper triangular from cholesky, t1=b constants, x unknowns */
	/* Mx+b=0   M=At*A */
	return(rtflag);
}

static void
solchol(R,b,x,t,n)
/* R upper triangular from cholesky, b constants, x unknowns */
/* t is a temporary vector */
/* Mx+b=0   M=Rt*R */
register double R[],b[],*x,t[];
register int n;
{
	register int k,np1,i,iM;
	register double *M,dtemp;
	register double S;
	double dot();

	np1=n+1;

/* forward substitution */

	for(k=0,M=R;k<n;k++,M+=np1)
		{
		dtemp= -(b[k]+dot(R+k,n,t,1,k))/(*M);
		t[k]=dtemp;
		}
	/*
	for(k=0,kM=0;k<n;k++,kM+=n)
		{
		S=b[k];
		for(i=0,iM=0;i<k;i++,iM+=n)
			S+=R[iM+k]*t[i];
		dtemp= -S/R[kM+k];
		t[k]=dtemp;
		}
	*/

/* backward substitution */

	/*
	for(k=0,x=x+n,M=R+n*n;k<n;k++,M-=np1,x--)
		{
		dtemp=(t[n-1-k]-dot(M,1,x,1,k))/(*(M-1));
		x[k]=dtemp;
		}
	*/
	/*
	for(i=n-1,iM=(n-1)*n;i>=0;i--,iM-=n)
		{
		S=t[i];
		for(k=i+1;k<n;k++)
			S-=R[iM+k]*x[k];
		dtemp=S/R[iM+i];
		x[i]=dtemp;
		}
	*/
	for(i=n-1,iM=(n-1)*n;i>=0;i--,iM-=n)
		{
		S=t[i]-dot(R+iM+i+1,1,x+i+1,1,n-i-1);
		dtemp=S/R[iM+i];
		x[i]=dtemp;
		}
	return;
}

static double
spdmtrace(M1,M2,n)/* trace of M1*M2, M1 double, M2 short */
register double *M1;
register short *M2;
register int n;
{
	register int i;
	register double trace;
	double spdot();

	trace=0;
	for(i=0;i<n;i++,M1+=n,M2++)
		trace+=spdot(M1,1,M2,n,n);
	return(trace);
}

static double
spdot(a,na,b,nb,n)/* dot product of double and short */
/* short is one or zero or minus one */
register double *a;
register short *b;
register int n,na,nb;
{
	register int i;
	register double dot;

	dot=0;
	for(i=0;i<n;i++,a+=na,b+=nb)
		{
		if(*b)
			{
			if(*b>0)/* *b == 1 */
				dot+=(*a);
			else/* *b == -1 */
				dot-=(*a);
			}
		}
	return(dot);
}

static void
toepmult(a,ia,b,ib,c,ic,p)
double a,ia,b,ib,*c,*ic,p;
{
	*c+= (a*b-ia*ib)/p;
	*ic+= (a*ib+ia*b)/p;
	return;
}

static double 
trace(M,n)/* return trace of double matrix */
register double *M;
register int n;
{
	register int i,con;
	register double sum;

	sum=0;
	con=n+1;
	for(i=0;i<n;i++,M+=con)
		sum+= *M;
	return(sum);
}

static int
triinv(T,Tinv,n)	/* T and Tinv upper triangular */
		/* 0 normal return, 1 T singular */
register int n;
register double *T,*Tinv;
{
	register int i,j,k,iT,kT;
	register double sum,dtemp,*sT,*sTinv;

	j=n+1;
	for(i=0,sT=T,sTinv=Tinv;i<n;i++,sT+=j,sTinv+=j)
		{
		if(!*sT)return(1);
		*sTinv=1/(*sT);
		}
	for(j=n-1;j>=0;j--)
		{
		for(i=j-1,iT=(j-1)*n;i>=0;i--,iT-=n)
			{
			sum=0;
			for(k=i+1,kT=(i+1)*n;k<j+1;k++,kT+=n)
				sum+=T[iT+k]*Tinv[kT+j];
			dtemp= -sum*Tinv[iT+i];
			Tinv[iT+j]=dtemp;
			}
		}
	for(i=0;i<n;i++,Tinv+=n)/* clear lower triangle */
		{
		for(j=0,sT=Tinv;j<i;j++,sT++)
			*sT=0;
		}
	return(0);
}
