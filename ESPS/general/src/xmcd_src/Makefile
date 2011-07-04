#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice
#
#      "Copyright (c) 1989 Entropic Speech, Inc.; All rights reserved"
#
#  makefile for xmcd @(#)Makefile	1.3	8/8/89	ESI

CFLAGS = $(PROGCFLAGS) -g $(WINDOWS)
LFLAGS = $(LINTFLAGS) 

OBJS = mcd.o text.o chrs.o xwinplt.o NamePix.o draw_line.o make_icon.o
SRCS = mcd.c text.c chrs.c xwinplt.c NamePix.c draw_line.c make_icon.c
PROGNAME = xmcd
MANNAME = xmcd.1


$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) -lXw -lXt -lX11 $(SPSLIB) -lm -o $(PROGNAME)

$(OBJS):	$(SINC)/esps.h 
make_icon.o:	sine.icon spec.icon image.icon epi.icon hist.icon

install:	$(PROGNAME)
	-$(STRIP) $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm -f $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

clean:	
	-rm -f $(OBJS) $(PROGNAME) tstmcd.o tstmcd
	
lint:		mcd.lint tstmcd.lint

mcd.lint:	$(SRCS) 
	$(LINT) $(LFLAGS) $(SRCS) $(LINTLIB) $(WINLIBS) > $(PROGNAME).lint

tstmcd.lint:	tstmcd.c
	$(LINT) $(LFLAGS) tstmcd.c $(LINTLIB) > tstmcd.lint

test:		$(PROGNAME) tstmcd
	sh mcd.test

tstmcd: tstmcd.o
	cc -o tstmcd tstmcd.o

tstmcd.c: ../mcd/tstmcd.c
	-rm -f tstmcd.c
	ln ../mcd/tstmcd.c .
