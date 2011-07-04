#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing
#  by Entropic Speech, Inc. must bear the notice			
# 								
#  "Copyright (c) 1987, 1990 Entropic Speech, Inc. All rights reserved."
# 				
#
#  makefile for mcd @(#)makefile	3.4 5/16/90 ESI
 	

CFLAGS = $(PROGCFLAGS) -g

OBJS = mcd.o text.o chrs.o winplt.o
SRCS = mcd.c text.c chrs.c winplt.c
PROGNAME = mcd
MANNAME = mcd.1

SUNVIEWLIBS = -lsuntool -lsunwindow -lpixrect

$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) $(SUNVIEWLIBS) -lm -o $(PROGNAME)

$(OBJS):	$(SINC)/esps.h 

install:	$(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm -f $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
	-rm -f $(LIBDIR)/icon.spec
	cp icon.spec $(LIBDIR) 
	chmod $(MANMOD) $(LIBDIR)/icon.spec

clean:	
	-rm -f $(OBJS) $(PROGNAME) tstmcd.o tstmcd
	
lint:		mcd.lint tstmcd.lint

mcd.lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) $(SUNVIEWLIBS) > $(PROGNAME).lint

tstmcd.lint:	tstmcd.c
	$(LINT) $(LINTFLAGS) tstmcd.c $(LINTLIB) > tstmcd.lint

test:		$(PROGNAME) tstmcd
	sh $(PROGNAME).test
