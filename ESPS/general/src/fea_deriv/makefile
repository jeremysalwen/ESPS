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
# makefile for fea_deriv @(#)makefile	3.4  2/14/96  ESI
 	
CFLAGS = $(PROGCFLAGS)

SRCS = fea_deriv.c gethighfie.c
OBJS = fea_deriv.o gethighfie.o


fea_deriv:	$(OBJS) $(SPSLIB)
	cc $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o fea_deriv

pure:	$(OBJS) $(SPSLIB)
	purify cc $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o fea_deriv

fea_deriv.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/fea.h
gethighfie.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/fea.h

install:	fea_deriv
	$(STRIP) fea_deriv
	-mv -f $(BINDIR)/fea_deriv $(OLDBIN)/fea_deriv
	-chmod $(PROGMOD) $(OLDBIN)/fea_deriv
	-mv -f fea_deriv $(BINDIR)/fea_deriv
	chmod $(PROGMOD) $(BINDIR)/fea_deriv
	-diffmk  $(MANDIR)/man1/fea_deriv.1 fea_deriv.1 man.diff
	@echo diffmked man left in man.diff
	-rm -f $(MANDIR)/man1/fea_deriv.1
	cp fea_deriv.1 $(MANDIR)/man1/fea_deriv.1
	chmod $(MANMOD) $(MANDIR)/man1/fea_deriv.1

clean:
	rm -f $(OBJS) fea_deriv

lint:		$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:		fea_deriv
	-sh testfile
