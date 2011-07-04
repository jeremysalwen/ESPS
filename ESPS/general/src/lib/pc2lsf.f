C 
C	 This material contains proprietary software of Entropic Speech,
C Inc. Any reproduction, distribution, or publication without the the prior 
C written permission of Entropic Speech, Inc. is strictly prohibited.
C Any public distribution of copies of this work authorized in writing by
C	 Entropic Speech, Inc. must bear the notice			
C
C       		Copyright 1986, Entropic Speech, Inc
C
C
C
C
C   This module knows how to compute line spectral frequencies from 
C    prediction filter coefficients. The parameters have the following 
C    meanings:
C
C   "lsf" is the floating array that returns the line spectral frequencies
C   "pc" is a floating array that contains the linear prediction coefficients
C   "fres" is an double that is equal to the frequency spacing used in
C           the search for line spectral frequencies
C   "ierr" is error flag
C
C
C
C
C sccsid = @(#)pc2lsf.f	1.2	7/30/87 ESI
C
C
C
C	= PC_2_LSF CONVERSION SUBROUTINE (G. KANG & L. FRANSEN, NRL)=
C	modified by D. Burton - Nov. 19, 1986

	SUBROUTINE PC2LSF(PC, FREQ, fres, ierr)
C
	DIMENSION PC(10),FREQ(10),FREQF(10)
	DIMENSION FR1(5),FR2(5)
	DIMENSION AMP1(8000),AMP2(8000)
	DIMENSION CT(48000),ST(48000)
	integer ierr
	real*4  fqstep, sampfrq
	real*8 fres
C
C 	---- Do set ups and check inputs ---
C
	MAXFREQ = 100
	sampfrq = 8000.
	fqstep = sampfrq/fres
	ierr = 0 
	if(fres .lt. 1. .or. fres .gt. MAXFREQ) ierr = 1
	if(ierr .eq. 1) return

C
C	----- SINE AND COSINE TABLES -----
C
	IF(M.EQ.1)GO TO 90
	TPIT=2.*3.1415926/fqstep
	K=0
	DO 20 I=1, int(fqstep/2)
	WT=TPIT*(I-1)
	DO 30 J=1,5
	K=K+1
	CT(K)=COS(WT*(J-1))+COS(WT*(11-J))
	ST(K)=SIN(WT*(J-1))+SIN(WT*(11-J))
30	CONTINUE
	K=K+1
	CT(K)=COS(WT*5)
	ST(K)=SIN(WT*5)
20	CONTINUE
	M=1
C
C	----- REAL-ROOT FACTORED SUM AND DIFFERENCE FILTERS ----
C
90	P1=1.
	P2=-PC(1)-PC(10)-P1
	P3=-PC(2)-PC(9)-P2
	P4=-PC(3)-PC(8)-P3
	P5=-PC(4)-PC(7)-P4
	P6=-PC(5)-PC(6)-P5
	Q1=1.
	Q2=-PC(1)+PC(10)+Q1
	Q3=-PC(2)+PC(9)+Q2
	Q4=-PC(3)+PC(8)+Q3
	Q5=-PC(4)+PC(7)+Q4
	Q6=-PC(5)+PC(6)+Q5
C
C	----- APLITUDE SPECTRA OF P(Z) AND Q(Z) -----
C
	DO 120 I=1, int(fqstep/2)
	N=6*(I-1)
	N1=N+1
	N2=N+2
	N3=N+3
	N4=N+4
	N5=N+5
	N6=N+6
	RP=CT(N1)+P2*CT(N2)+P3*CT(N3)+P4*CT(N4)+P5*CT(N5)+P6*CT(N6)
	XP=ST(N1)+P2*ST(N2)+P3*ST(N3)+P4*ST(N4)+P5*ST(N5)+P6*ST(N6)
	RQ=CT(N1)+Q2*CT(N2)+Q3*CT(N3)+Q4*CT(N4)+Q5*CT(N5)+Q6*CT(N6)
	XQ=ST(N1)+Q2*ST(N2)+Q3*ST(N3)+Q4*ST(N4)+Q5*ST(N5)+Q6*ST(N6)
	AMP1(I)=RP*RP+XP*XP
	AMP2(I)=RQ*RQ+XQ*XQ
120	CONTINUE
C
C	---- NULL FREQUENCIES FROM P(Z) AND Q(Z) ----
C
	CALL NULL(AMP1,FR1, fqstep, fres)
	CALL NULL(AMP2,FR2, fqstep, fres)
	DO 170 I=1,5
	II=2*I-1
	FREQF(II)=FREQ(II)
	FREQ(II)=FR1(I)
	II=II+1
	FREQF(II)=FREQ(II)
	FREQ(II)=FR2(I)
170	CONTINUE
C
C	---- ILL-CONDITIONED CASES ----
C
	IFLAG=0
	DO 180 I=2,10
	IF(FREQ(I).EQ.0)IFLAG=1
	IF(FREQ(I).LE.FREQ(I-1))IFLAG=1
180	CONTINUE
	IF(IFLAG.EQ.0)GO TO 190
	DO 200 I=1,10
	FREQ(I)=FREQF(I)
200	CONTINUE
190	RETURN
	END
C
C	==== NULL-PICKING SUBROUTINE ====
C
	SUBROUTINE NULL(AMP,FR, fqstep, fres)
C
	DIMENSION AMP(8000),FR(5)
	real*8 fres
C
C	----- INITIALIZATION -----
C
	DO 100 I=1,5
	FR(I)=0
100	CONTINUE
C
C	----- NULL SEARCH -----
C
	K=0
	DO 110 I=1, int(fqstep/2)
	A3=A2
	A2=A1
	A1=AMP(I)
	IF(I.LT.3)GO TO 110
	IF(A3.LT.A2)GO TO 110
	IF(A1.LT.A2)GO TO 110
C
C	---- NULL PT. CORR. BY PARABOLIC APPROXIMATION OF (A1,A2,A3) ---
C
	CORR= (float(fres)/2)*(A3-A1)/(A1-2.*A2+A3)
C
C	---- LSP FREQUENCIES -----
C
	K=K+1
	FR(K)=fres*(I-2) + CORR
	IF(K.EQ.5)GO TO 120
110	CONTINUE
120	RETURN
	END
------
