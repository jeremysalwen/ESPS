#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for cplay %W% %G% ESI
 	

CFLAGS = $(PROGCFLAGS) -D=DADEV=\"$(DADEV)\" -DCLKDEV=\"$(CLKDEV)\" -DMAXDA=$(MAXDA)
#
#

cplay:	cplay.o $(SPSLIB)
	cc cplay.o $(SPSLIB) -lm -o cplay

cplay.o: $(SINC)/esps.h $(SINC)/sd.h

clean:
	rm -f cplay.o cplay lint man.diff

lint:	cplay.c mrlintcheck.c
	$(LINT) $(LINTFLAGS) cplay.c mrlintcheck.c $(LINTLIB) > lint

install: cplay
	-mv -f $(BINDIR)/cplay $(OLDBIN)/cplay
	-chmod $(PROGMOD) $(OLDBIN)/cplay
	mv cplay $(BINDIR)/cplay
	-strip $(BINDIR)/cplay
	chmod $(PROGMOD) $(BINDIR)/cplay
	-diffmk  $(MANDIR)/man1/cplay.1 cplay.1 man.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/cplay.1
	cp cplay.1 $(MANDIR)/man1/cplay.1
	chmod $(MANMOD) $(MANDIR)/man1/cplay.1
# on a given system, cplay will be installed as play
	-rm -f $(BINDIR)/play
	ln $(BINDIR)/cplay $(BINDIR)/play
	chmod $(PROGMOD) $(BINDIR)/play
	-rm -f $(MANDIR)/play.1
	ln $(MANDIR)/man1/cplay.1 $(MANDIR)/man1/play.1
	chmod $(MANMOD) $(MANDIR)/man1/play.1

test:	cplay
	sh testfile
