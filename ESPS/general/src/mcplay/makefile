#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for play @(#)makefile	3.5 1/10/89 ESI
 	

CFLAGS = $(PROGCFLAGS) -DDADEV=\"$(DADEV)\" -DCLKDEV=\"$(CLKDEV)\" -DMAXDA=$(MAXDA)
#
#

mcplay:	mcplay.o $(SPSLIB)
	cc mcplay.o $(SPSLIB) -lmr -lm -o mcplay

mcplay.o: $(SINC)/esps.h $(SINC)/sd.h

clean:
	rm -f mcplay.o mcplay lint man.diff

lint:	mcplay.c mrlintcheck.c
	$(LINT) $(LINTFLAGS) mcplay.c mrlintcheck.c $(LINTLIB) > lint

install: mcplay
	-mv -f $(BINDIR)/mcplay $(OLDBIN)/mcplay
	-chmod $(PROGMOD) $(OLDBIN)/mcplay
	mv mcplay $(BINDIR)/mcplay
	-strip $(BINDIR)/mcplay
	chmod $(PROGMOD) $(BINDIR)/mcplay
	-diffmk  $(MANDIR)/man1/mcplay.1 mcplay.1 man.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/mcplay.1
	cp mcplay.1 $(MANDIR)/man1/mcplay.1
	chmod $(MANMOD) $(MANDIR)/man1/mcplay.1
# on a given system, mcplay will be installed as play
	-rm -f $(BINDIR)/play
	ln $(BINDIR)/mcplay $(BINDIR)/play
	chmod $(PROGMOD) $(BINDIR)/play
	-rm -f $(MANDIR)/man1/play.1
	ln $(MANDIR)/man1/mcplay.1 $(MANDIR)/man1/play.1
	chmod $(MANMOD) $(MANDIR)/man1/play.1

test:	mcplay
	sh testfile
