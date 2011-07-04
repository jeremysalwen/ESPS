# This material contains proprietary software of Entropic Research
# Laboratory, Inc. Any reproduction, distribution, or publication
# without the prior written permission of Entropic Research Laboratory,
# Inc. (ERL) is strictly prohibited.  Any public distribution of copies
# of this work authorized in writing by ERL must bear the notice:

# "Copyright (c) 1991 Entropic Research Laboratory, Inc.; All rights reserved"

#  makefile for mbuttons @(#)Makefile	1.6 01 Mar 1996 ERL

XVINC  = -I$(XVIEW_INC)
CFLAGS = $(PROGCFLAGS) -g $(XVINC)

#XVLIB = $(XVIEW_LIB)
#for static libraries: 
XVLIB =  $(XVIEW_B_OPT)  $(XVIEW_LIB)

LINTFLAGS = -I/usr/openwin/include

OBJS = mbuttons.o 
SRCS = mbuttons.c 

PROGNAME = mbuttons
MANNAME = mbuttons.1

$(PROGNAME): $(SPSLIB) $(OBJS)
	$(CC)  $(CFLAGS) $(OBJS)  $(XVLIB) $(SPSLIB) -lm -o $(PROGNAME)

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
	-rm -f $(OBJS) $(PROGNAME) foo.sd

lint:   $(SRCS)
	$(LINT) $(LINTFLAGS) $(XVINC) $(SRCS) $(LINTLIB) > lint

test:   $(PROGNAME)
	-sh $(PROGNAME).test


