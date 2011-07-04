#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice
#
#      "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
#
#  makefile for xtext @(#)Makefile	1.6 11/22/91 ESI



XVINC  = -I$(XVIEW_INC)
CFLAGS = $(PROGCFLAGS) -g $(XVINC)

#XVLIB = $(XVIEW_LIB)
#for static libraries: 
XVLIB =  $(XVIEW_B_OPT)  $(XVIEW_LIB)

LINTFLAGS = -I/home/openwin/include

OBJS = xtext.o 
SRCS = xtext.c 


PROGNAME = xtext
MANNAME = xtext.1

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS)  $(XVLIB) $(SPSLIB) $(BSDLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h

install: $(PROGNAME)
	-strip $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

clean:
	-rm -f $(OBJS) $(PROGNAME) foo.sd

lint:   $(SRCS)
	$(LINT) $(LINTFLAGS) $(XVINC) $(SRCS) $(LINTLIB) > lint

test:   $(PROGNAME)
	-sh $(PROGNAME).test
