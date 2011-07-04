#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# 	
# %W% %G% makefile for SPS library

CFLAGS =  -O3 $(CCKR) $(LIBCFLAGS)
FFLAGS =  $(LIBFFLAGS)
ELMDIR = ../elm/lib

OBJS =	\
alaw.o auto_pefrc.o block_filter.o cepstrum.o classify.o complex.o \
convolv.o covar.o cover_type.o emphasis.o gauss.o getarspect.o \
getatal.o getauto.o getburg.o getcfft.o getdct.o getfft.o hamming.o \
interp_filt.o is_distort.o lar_to_rc.o lloyd.o lsf_to_pc.o lsq_solv.o \
matalloc.o matinv.o minrelent.o mulaw.o pc_to_lsf.o pef_autorc.o \
pz_to_co.o rand_int.o rand_intnr.o rc_autopef.o rc_reps.o rc_to_lar.o rctoc.o \
refl_to_auto.o refl_to_filt.o relent.o reps_rc.o reverse.o statsiz.o \
stsolv.o vqdesign.o vqencode.o  window.o zero_fill.o  \
blt.o fft_cepstrum.o dtw_tl.o dtw_l2.o \
genburg.o getvburg.o  getfburg.o bestauto.o  struct_cov.o compute_rc.o \
arr_func.o block_futil.o block_f2.o fil_spectrum.o arr_op.o lp_procs.o \
bark_hertz.o crit_band_filt_wt.o mel_hertz.o phon_to_sone.o get_snr.o


SRCS =	\
alaw.c auto_pefrc.c block_filter.c cepstrum.c classify.c complex.c \
convolv.c covar.c cover_type.c emphasis.c gauss.c getarspect.c \
getatal.c getauto.c getburg.c getcfft.c getdct.c getfft.c hamming.c \
interp_filt.c is_distort.c lar_to_rc.c lloyd.c lsf_to_pc.c lsq_solv.c \
matalloc.c matinv.c minrelent.c mulaw.c pc_to_lsf.c pef_autorc.c \
pz_to_co.c rand_int.c rand_intnr.c rc_autopef.c rc_reps.c rc_to_lar.c \
rctoc.c refl_to_auto.c refl_to_filt.c relent.c reps_rc.c reverse.c \
statsiz.c stsolv.c vqdesign.c vqencode.c  window.c zero_fill.c \
blt.c fft_cepstrum.c dtw_tl.c dtw_l2.c \
genburg.c getvburg.c  getfburg.c bestauto.c  struct_cov.c compute_rc.c \
arr_func.c block_futil.c block_f2.c fil_spectrum.c arr_op.c lp_procs.c \
bark_hertz.c crit_band_filt_wt.c mel_hertz.c phon_to_sone.c get_snr.c


$(LIBDIR)/$(LIBNAME):	$(OBJS)
		ar rv $(LIBDIR)/$(LIBNAME) $?; $(RANLIB) $(LIBDIR)/$(LIBNAME)

# do there so we don't use the implied -g of the other compiles
#
vqdesign.o:	vqdesign.c
		cc -c $(OLIBCFLAGS)  vqdesign.c
vqencode.o:	vqencode.c
		cc -c $(OLIBCFLAGS)  vqencode.c
block_filter.o:	$(SINC)/esps.h
interp_filt.o:	$(SINC)/esps.h
vqdesign.o:	$(SINC)/fea.h $(SINC)/vq.h $(SINC)/esps.h
anafeasupp.o:	$(SINC)/fea.h $(SINC)/anafea.h $(SINC)/header.h $(SINC)/esps.h
uniq_name.o:	$(SINC)/esps.h
genzfunc.o:	$(SINC)/esps.h
getderiv_vec.o:	$(SINC)/esps.h $(SINC)/fea.h
reps_rc.o:	$(SINC)/esps.h $(SINC)/anafea.h $(SINC)/fea.h
rc_reps.o:	$(SINC)/esps.h $(SINC)/anafea.h $(SINC)/fea.h
getdct.o:	$(SINC)/esps.h $(SINC)/constants.h
classify.o:	$(SINC)/constants.h $(SINC)/esps.h
get_fea_rec.o:	$(SINC)/esps.h $(SINC)/fea.h
marg_index.o:	$(SINC)/esps.h $(SINC)/unix.h
matinv.o:	$(SINC)/esps.h
addstr.o:	$(SINC)/esps.h $(SINC)/unix.h
fldrangesw.o:	$(SINC)/esps.h $(SINC)/unix.h
stsolv.o:	$(SINC)/esps.h $(SINC)/unix.h
lsq_solv.o:	$(SINC)/esps.h $(SINC)/unix.h
feaspecsupp.o:	$(SINC)/esps.h $(SINC)/unix.h
updatewave.o:	$(SINC)/esps.h $(SINC)/unix.h
miio.o:		$(SINC)/esps.h $(SINC)/header.h
is_complex.o:	$(SINC)/esps.h $(SINC)/fea.h
fea_compat.o:	$(SINC)/esps.h $(SINC)/fea.h
arr_func.o:	$(SINC)/esps.h $(SINC)/func.h

install:	$(LIBDIR)/$(LIBNAME) 


lint:		$(SRCS)	
		$(LINT) $(LINTFLAGS) $(SRCS) -lm > lint

clean:
		rm -f $(OBJS) pfparser.c pflexan.c vqdesign.o vqencode.o \
		$(LIBNAME) $(LLIBNAME)


