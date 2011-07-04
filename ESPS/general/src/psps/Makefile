#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for psps @(#)Makefile	3.16	2/14/96	 ESI
 	
CFLAGS = $(OPT_FLAG) $(PROGCFLAGS)


psps:	psps.o prnt_hdr.o $(WSPSLIB)
	$(CC) $(CFLAGS) psps.o prnt_hdr.o $(WSPSLIB) -lm  -o psps

pure:	psps.o prnt_hdr.o $(WSPSLIB)
	purify $(CC) $(CFLAGS) psps.o prnt_hdr.o $(WSPSLIB) -lm  -o psps


psps.o: $(SINC)/header.h $(SINC)/esps.h 
prnt_hdr.o: $(SINC)/header.h $(SINC)/esps.h 

clean:
	-rm -f *.o psps *.sd signal.rc signal.lsf

install:	$(BINDIR)/psps $(MANDIR)/man1/psps.1

lint:   psps.c prnt_hdr.c 
	$(LINT) $(LINTFLAGS) psps.c prnt_hdr.c $(LINTLIB) > lint

$(BINDIR)/psps: psps
	-mv -f $(BINDIR)/psps $(OLDBIN)/psps
	-chmod $(PROGMOD) $(OLDBIN)/psps
	-rm -f $(BINDIR)/psps
	mv psps $(BINDIR)/psps
	-$(STRIP) $(BINDIR)/psps
	-(cd $(BINDIR); rm epsps; ln -s psps epsps)
	chmod $(PROGMOD) $(BINDIR)/psps

$(MANDIR)/man1/psps.1: psps.1
	-diffmk  $(MANDIR)/man1/psps.1 psps.1 man.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/psps.1
	cp psps.1 $(MANDIR)/man1/psps.1
	chmod $(MANMOD) $(MANDIR)/man1/psps.1
	-(cd $(MANDIR)/man1 ; rm man1/epsps.1; ln -s psps.1 epsps.1 )

test:	psps
	-sh psps.test
