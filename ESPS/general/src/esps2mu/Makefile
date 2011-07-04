#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1989 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for esps2mu %W% %G%	 ESI

CFLAGS =   $(PROGCFLAGS)

PROGS = esps2mu

esps2mu:	esps2mu.o $(WSPSLIB)
	cc $(CFLAGS) esps2mu.o $(WSPSLIB) -lm -o esps2mu

esps2mu.o: $(SINC)/esps.h $(SINC)/sd.h $(SINC)/feasd.h

install: $(PROGS)
	@echo esps2mu:
	-mv -f $(BINDIR)/esps2mu $(OLDBIN)/esps2mu
	-chmod $(PROGMOD) $(OLDBIN)/esps2mu
	mv esps2mu $(BINDIR)/esps2mu
	$(STRIP) $(BINDIR)/esps2mu
	chmod $(PROGMOD) $(BINDIR)/esps2mu
	-diffmk  $(MANDIR)/man1/esps2mu.1 esps2mu.1 e2l.diff
	@echo diffmked man in e2l.diff
	-rm -f $(MANDIR)/man1/esps2mu.1
	cp esps2mu.1 $(MANDIR)/man1/esps2mu.1
	chmod $(MANMOD) $(MANDIR)/man1/esps2mu.1

lint:	esps2mu.lt

esps2mu.lt:	esps2mu.c
	$(LINT)  $(LINTFLAGS) esps2mu.c $(LINTLIB) -lm  > esps2mu.lt

clean:
	-rm -f *.o esps2mu *.diff *.sd *.mu

test: $(PROGS)
	-sh mu.test
