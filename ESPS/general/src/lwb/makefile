#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for esps2lwb and lwb2esps makefile @(#)makefile	1.6 3/8/91 ESI
 	


CFLAGS =   $(PROGCFLAGS)


PROGS = esps2lwb lwb2esps

all:	$(PROGS)

esps2lwb:	esps2lwb.o  $(SPSLIB)
	cc $(CFLAGS) esps2lwb.o  $(SPSLIB) -lm -o esps2lwb

lwb2esps:	lwb2esps.o $(WSPSLIB) $(SPSLIB)
	cc $(CFLAGS) lwb2esps.o $(WSPSLIB) $(SPSLIB) -lm -o lwb2esps

lwb2esps.o: $(SINC)/esps.h $(SINC)/sd.h header.h
esps2lwb.o: $(SINC)/esps.h $(SINC)/sd.h header.h

install: $(PROGS)
	@echo esps2lwb:
	-mv -f $(BINDIR)/esps2lwb $(OLDBIN)/esps2lwb
	-chmod $(PROGMOD) $(OLDBIN)/esps2lwb
	mv esps2lwb $(BINDIR)/esps2lwb
	$(STRIP) $(BINDIR)/esps2lwb
	chmod $(PROGMOD) $(BINDIR)/esps2lwb
	-diffmk  $(MANDIR)/man1/esps2lwb.1 esps2lwb.1 e2l.diff
	@echo diffmked man in e2l.diff
	-rm -f $(MANDIR)/man1/esps2lwb.1
	cp esps2lwb.1 $(MANDIR)/man1/esps2lwb.1
	chmod $(MANMOD) $(MANDIR)/man1/esps2lwb.1
#
	@echo lwb2esps:
	-mv -f $(BINDIR)/lwb2esps $(OLDBIN)/lwb2esps
	-chmod $(PROGMOD) $(OLDBIN)/lwb2esps
	mv lwb2esps $(BINDIR)/lwb2esps
	$(STRIP) $(BINDIR)/lwb2esps
	chmod $(PROGMOD) $(BINDIR)/lwb2esps
	-diffmk  $(MANDIR)/man1/lwb2esps.1 lwb2esps.1 l2e.diff
	@echo diffmked man in l2e.diff
	-rm -f $(MANDIR)/man1/lwb2esps.1
	cp lwb2esps.1 $(MANDIR)/man1/lwb2esps.1
	chmod $(MANMOD) $(MANDIR)/man1/lwb2esps.1

lint:	lwb2esps.lt esps2lwb.lt

lwb2esps.lt:	lwb2esps.c 
	$(LINT) $(LINTFLAGS)  lwb2esps.c $(LINTLIB)  > lwb2esps.lt

esps2lwb.lt:	esps2lwb.c
	$(LINT) $(LINTFLAGS)  esps2lwb.c $(LINTLIB)  > esps2lwb.lt

clean:
	-rm -f *.o lwb2esps esps2lwb *.diff *.sd

test: $(PROGS)
	-sh lwb.test
