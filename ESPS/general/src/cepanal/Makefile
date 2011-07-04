# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1997 Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.1 29 Apr 1997 ERL
# 
# Makefile for: cepanal
#
# Written by:  Rod Johnson
# Checked by:
# Revised by:

CFLAGS = -g $(PROGCFLAGS)
#FOLLOWING FOR TEST ONLY
#CFLAGS = -g -I../../window -Dsparc $(PROGCFLAGS)

OBJS = cpeanal.o
SRCS = cepanal.c
#FOLLOWING FOR TEST ONLY
#OBJS = cpeanal.o ../../window/window.o ../../window/lin_search.o
#SRCS = cepanal.c ../../window/window.c ../../window/lin_search.c

PROGNAME = cepanal
MANNAME = cepanal.1
DEFPARAM = Pcepanal


$(PROGNAME): $(SPSLIB) $(SRCS) \
	$(SINC)/esps.h $(SINC)/fea.h $(SINC)/feasd.h
	$(ANSI_CC) $(CFLAGS) $(SRCS) $(SPSLIB) -lm -o $(PROGNAME)

pure:	$(SPSLIB) $(SRCS) \
	$(SINC)/esps.h $(SINC)/fea.h $(SINC)/feasd.h
	purify $(ANSI_CC) $(CFLAGS) $(SRCS) $(SPSLIB) -lm -o $(PROGNAME)

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
# install the default parameter file
#	-rm -f $(PARAMDIR)/$(DEFPARAM)
#	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
#	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)

clean:	
	-rm -f $(OBJS) $(PROGNAME) lint core 

lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > lint

test:	$(PROGNAME)
	sh $(PROGNAME).tst

