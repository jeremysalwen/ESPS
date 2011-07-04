# This material contains proprietary software of Entropic Research 
# Laboratory, Inc. Any reproduction, distribution, or publication 
# of this work must be authorized in writing by Entropic Research 
# Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1991-1992  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# Brief description: Makefile for e2sphere
#
# Author:                      John Shore
# Currently Maintained by:     John Shore
# SCCS info:                   @(#)Makefile	1.1 1/4/93 ERL

#CFLAGS = -I./ $(PROGCFLAGS) -g
#OBJS = e2sphere.o sdtosph.o
#SRCS = e2sphere.c sdtosph.c

CFLAGS = $(PROGCFLAGS)
OBJS = e2sphere.o 
SRCS = e2sphere.c 

PROGNAME = e2sphere
MANNAME = e2sphere.1

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h 

ccenter_src: $(SRCS)
	#load $(CFLAGS) $(SPSLIB) $(XVLIB) -lm $(SRCS)

install: $(PROGNAME)
	-$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
	
clean:	
	-rm -f $(OBJS) $(PROGNAME) e2.sph e2.fsd lint
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	-sh $(PROGNAME).test
