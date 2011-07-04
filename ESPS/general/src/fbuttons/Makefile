# This material contains proprietary software of Entropic Research
# Laboratory, Inc. Any reproduction, distribution, or publication
# without the prior written permission of Entropic Research Laboratory,
# Inc. (ERL) is strictly prohibited.  Any public distribution of copies
# of this work authorized in writing by ERL must bear the notice:

# "Copyright (c) 1991 Entropic Research Laboratory, Inc.; All rights reserved"

#  makefile for xtext @(#)Makefile	1.9 6/18/93 ERL

XVINC  = -I$(XVIEW_INC)
CFLAGS =  $(XVINC) $(PROGCFLAGS) -g

XVLIB =  $(XVIEW_B_OPT)  $(XVIEW_LIB)

LINTFLAGS = -I/usr/openwin/include

#OBJS = fbuttons.o xbbox.o
#SRCS = fbuttons.c xbbox.c

OBJS = fbuttons.o 
SRCS = fbuttons.c 

PROGNAME = fbuttons
MANNAME = fbuttons.1

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS)  $(XVLIB) $(SPSLIB) $(BSDLIB) $(REGEXPLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h

saber_src: $(SRCS)
	#load $(CFLAGS) $(SPSLIB) $(XVLIB) -lm $(SRCS)

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
	-rm -f $(OBJS) $(PROGNAME) fbfoo? Ffile Efile Lfile Sfile

lint:   $(SRCS)
	$(LINT) $(LINTFLAGS) $(XVINC) $(SRCS) $(LINTLIB) > lint

test:   $(PROGNAME)
	-sh $(PROGNAME).test


