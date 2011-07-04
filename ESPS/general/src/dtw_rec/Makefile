# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.3 10/25/96 ERL
# 
# Makefile for:  dtw_rec
#
# Written by:  Bill Byrne
# Checked by:
# Revised by:

CFLAGS = -O $(PROGCFLAGS) 


OBJS = dtw_rec.o
SRCS = dtw_rec.c
PROGNAME = dtw_rec
MANNAME = dtw_rec.1

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc $(CFLAGS) $(OBJS)  $(SPSLIB) $(LIBS) -lm -o $(PROGNAME)

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
	-rm -f $(OBJS) $(PROGNAME) sgram.params

lint:   $(SRCS)
	$(LINT) $(LINTFLAGS) $(XVINC) $(SRCS) $(LINTLIB) > lint

test:   $(PROGNAME)
	-csh $(PROGNAME).t
