#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1989 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for muplay @(#)Makefile	1.1 4/2/90	 ESI
 	


CFLAGS =   $(PROGCFLAGS)

PROGS = muplay

muplay:	muplay.o
	cc $(CFLAGS) muplay.o -lm -o muplay

install: $(PROGS)
	@echo muplay:
	-mv -f $(BINDIR)/muplay $(OLDBIN)/muplay
	-chmod $(PROGMOD) $(OLDBIN)/muplay
	mv muplay $(BINDIR)/muplay
	$(STRIP) $(BINDIR)/muplay
	chmod $(PROGMOD) $(BINDIR)/muplay
	-diffmk  $(MANDIR)/man1/muplay.1 muplay.1 muplay.diff
	@echo diffmked man in muplay.diff
	-rm -f $(MANDIR)/man1/muplay.1
	cp muplay.1 $(MANDIR)/man1/muplay.1
	chmod $(MANMOD) $(MANDIR)/man1/muplay.1

lint:	muplay.c
	$(LINT) $(LINTFLAGS)  muplay.c $(LINTLIB)  > lint

clean:
	-rm -f *.o muplay *.diff 

#test: $(PROGS)
#	-sh muplay.test
