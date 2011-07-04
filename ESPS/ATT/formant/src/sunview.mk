#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#  makefile for formant @(#)sunview.mk	1.4 5/25/90 ESI
 	

CFLAGS = -I../../waves/src/h -I../../dsp32/lib $(PROGCFLAGS)
WAVES = ../../waves/src/c


SUNLIB = -lsuntool -lsunwindow -lpixrect -lm
DOCDIR = $(SPSDIR)/doc/waves

FORMANT_OBJS = dpform.o formant.o lpc_poles.o \
	dpfund.o cross.o downsample.o pole_methods.o version.o
PROGS =  formant 
PROGS_I = $(BINDIR)/formant
OTHER_STUFF = addnoise downsamp gate formsy debias

formant: $(FORMANT_OBJS) $(WAVESDIR)/libdsigproc.a $(LIBDIR)/$(WHDRLIB) $(WSPSLIB)
	cc -o formant ${CFLAGS} \
	$(FORMANT_OBJS) $(WAVESDIR)/libdsigproc.a \
	$(WAVESDIR)/libsig.a  $(LIBDIR)/$(WHDRLIB) $(WSPSLIB) $(SUNLIB)

pole_methods.o: $(WAVES)/pole_methods.c
	-rm -f pole_methods.c
	-cp $? .
	cc -c $(CFLAGS) pole_methods.c

version.o: $(WAVES)/version.c
	-rm -f version.c
	-cp $? .
	cc -c $(CFLAGS) version.c

install: $(PROGS_I) doc

other_stuff: $(OTHER_STUFF)

$(BINDIR)/formant: formant
	-strip $?
	-rm -f $(BINDIR)/$?
	cp $? $(BINDIR)/$?
	chmod $(PROGMOD) $(BINDIR)/$?
	-rm -f $(MANDIR)/man1/formant.1
	cp ../man/formant.1 $(MANDIR)/man1
	chmod $(MANMOD) $(MANDIR)/man1/formant.1

doc:	$(DOCDIR)/formant.help

$(DOCDIR)/formant.help:	../text/formant.help
	-rm -f $(DOCDIR)/formant.help
	cp ../text/formant.help $(DOCDIR)
	chmod $(MANMOD) $(DOCDIR)/formant.help

addnoise:  addnoise.o  /usr/local/lib/libsig.a
	cc -o addnoise ${CFLAGS} addnoise.o $(WAVESDIR)/libsig.a -lm

downsamp:  downsamp.o  downsample.o
	cc -o downsamp ${CFLAGS} downsamp.o downsample.o $(WAVESDIR)/libsig.a \
	-lm

fft: fft.o
	cc -o fft ${CFLAGS} fft.o -lm

sine: sine.o
	cc -o sine ${CFLAGS} sine.o -lm

gate: gate.o
	cc -o gate ${CFLAGS} gate.o $(WAVESDIR)/libsig.a -lm

debias:  debias.o
	cc -o debias ${CFLAGS} debias.o $(WAVESDIR)/libsig.a -lm

formsy: formsy.o
	cc -o formsy ${CFLAGS} formsy.o $(WAVESDIR)/libsig.a -lm

formant.o: formant.c
	cc -c $(CFLAGS) -DHELPFILE=\"$(DOCDIR)\" formant.c

clean:
	rm -f *.o   $(PROGS) $(OTHER_STUFF) version.c pole_methods.c
