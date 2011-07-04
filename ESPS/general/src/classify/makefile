#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved."
# 				
#
# makefile for classify @(#)makefile	3.3  6/1/88  ESI
 	
CFLAGS = -O $(PROGCFLAGS)
PROGNAME = classify


classify:	classify.o $(SPSLIB)
	cc $(CFLAGS) classify.o $(SPSLIB) -lm -o classify

classify.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/fea.h $(SINC)/feastat.h

install:	classify
	$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/classify $(OLDBIN)/classify
	-chmod $(PROGMOD) $(OLDBIN)/classify
	mv -f classify $(BINDIR)/classify
	chmod $(PROGMOD) $(BINDIR)/classify
	-diffmk  $(MANDIR)/man1/classify.1 classify.1 man.diff
	@echo diffmked man page in man.diff
	-rm $(MANDIR)/man1/classify.1
	cp classify.1 $(MANDIR)/man1/classify.1
	chmod $(MANMOD) $(MANDIR)/man1/classify.1

clean:
	-rm -f classify.o classify

lint:	classify.c 
	$(LINT) $(LINTFLAGS) classify.c $(LINTLIB) -lm > lint

test:	$(PROGNAME)
	sh testfile
