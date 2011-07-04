#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1989 Entropic Speech, Inc.; All rights reserved"
# 				
#  makefile for sigtosd @(#)sunview.mk	1.3 3/23/90 ESI
 	

CFLAGS = -I../waves/src/h  $(PROGCFLAGS) 
SUNLIB = -lm

SIGTOSD_OBJS = sigtosd.o
PROGS =  sigtosd 
PROGS_I = $(BINDIR)/sigtosd

sigtosd: $(SIGTOSD_OBJS) $(WAVESDIR)/libdsigproc.a $(LIBDIR)/$(WHDRLIB) \
		$(WSPSLIB)
	cc -o sigtosd ${CFLAGS} \
	$(SIGTOSD_OBJS) \
	$(WAVESDIR)/libsig.a  $(LIBDIR)/$(WHDRLIB) $(WSPSLIB) $(SUNLIB)

install: $(PROGS_I)

$(BINDIR)/sigtosd: sigtosd
	-strip $?
	-rm -f $(BINDIR)/$?
	cp $? $(BINDIR)/$?
	chmod $(PROGMOD) $(BINDIR)/$?
	-rm -f $(MANDIR)/man1/sigtosd.1
	cp sigtosd.1 $(MANDIR)/man1
	chmod $(MANMOD) $(MANDIR)/man1/sigtosd.1

sigtosd.o: sigtosd.c
	cc -c $(CFLAGS) -DHELPFILE=\"$(DOCDIR)\" sigtosd.c

clean:
	rm -f *.o   $(PROGS) 
