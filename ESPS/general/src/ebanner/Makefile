# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.2 6/18/93 ERL
# 
# Makefile for: ebanner
#
# Written by:  program by D. Talkin; cleanup by J. Shore

#CFLAGS = -g $(PROGCFLAGS) 

CFLAGS = -O $(PROGCFLAGS) 

OBJS = ebanner.o
SRCS = ebanner.c
PROGNAME = ebanner
MANNAME = ebanner.1
FONTS = cour.r.24

DEFPARAM = 

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h 

install: $(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
# install the fonts
	(cd fixedwidthfonts; cp $(FONTS) $(LIBDIR)/fixedwidthfonts)
	(cd $(LIBDIR)/fixedwidthfonts ; chmod $(LIBMOD) $(FONTS))

clean:	
	-rm -f $(OBJS) $(PROGNAME) core ebanner.hw ebanner.eb ebanner.sd \
	ebanner.fspec
	
lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	sh $(PROGNAME).test

codecenter_src: $(SRCS)
        #load $(CFLAGS) $(SPSLIB) $(XVLIB) -lm $(SRCS)

cc_ebanner: ebanner.c
	#load $(CFLAGS) $(SPSLIB) -lm ebanner.c 

