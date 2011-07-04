#  This material contains proprietary software of Entropic Research Lab, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Research Lab, Inc.; All rights reserved"
# 				
#  makefile for formant %W% %G% ERL
 	

CFLAGS = -O -I../../waves/src/h -I$(XVIEW_INC) -I../../dsp32/lib $(PROGCFLAGS) -DFOR_XVIEW
WAVES = ../../waves/src/c


SUNLIB = $(XVIEW_B_OPT) $(XVIEW_LIB) -lm
DOCDIR = $(SPSDIR)/doc/waves

FORMANT_OBJS = dpform.o formant.o lpc_poles.o \
	dpfund.o cross.o downsample.o pole_methods.o xversion.o 
PROGS =  formant 
PROGS_I = $(BINDIR)/formant
OTHER_STUFF = addnoise downsamp gate formsy debias

formant: $(FORMANT_OBJS) $(WAVESDIR)/libdsigproc.a $(WSPSLIB) \
		$(LIBDIR)/$(WHDRLIB)
	cc -o formant ${CFLAGS} \
	$(FORMANT_OBJS) $(WAVESDIR)/libdsigproc.a \
	$(WAVESDIR)/xlibsig.a  $(SUNLIB) $(LIBDIR)/$(WHDRLIB) $(WSPSLIB) $(SUNLIB)

pole_methods.o: $(WAVES)/pole_methods.c
	-rm -f pole_methods.c
	-cp $(WAVES)/pole_methods.c .
	cc -c $(CFLAGS) pole_methods.c

xversion.o: $(WAVES)/xversion.c
	-rm -f xversion.c
	-cp $? .
	cc -c $(CFLAGS) xversion.c

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

addnoise:  addnoise.o  /usr/local/lib/xlibsig.a
	cc -o addnoise ${CFLAGS} addnoise.o $(WAVESDIR)/xlibsig.a -lm

downsamp:  downsamp.o  downsample.o
	cc -o downsamp ${CFLAGS} downsamp.o downsample.o $(WAVESDIR)/xlibsig.a \
	-lm

fft: fft.o
	cc -o fft ${CFLAGS} fft.o -lm

sine: sine.o
	cc -o sine ${CFLAGS} sine.o -lm

gate: gate.o
	cc -o gate ${CFLAGS} gate.o $(WAVESDIR)/xlibsig.a -lm

debias:  debias.o
	cc -o debias ${CFLAGS} debias.o $(WAVESDIR)/xlibsig.a -lm

formsy: formsy.o
	cc -o formsy ${CFLAGS} formsy.o $(WAVESDIR)/xlibsig.a -lm

formant.o: formant.c
	cc -c $(CFLAGS) -DHELPFILE=\"$(DOCDIR)\" formant.c

clean:
	rm -f *.o   $(PROGS) $(OTHER_STUFF) xversion.c pole_methods.c
