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
# Makefile for: cepsyn
#
# Written by:  Rod Johnson
# Checked by:
# Revised by:

CFLAGS = -g $(PROGCFLAGS)

OBJS = cpesyn.o
SRCS = cepsyn.c
PROGNAME = cepsyn
MANNAME = cepsyn.1
DEFPARAM = Pcepsyn


$(PROGNAME): $(SPSLIB) $(SRCS) \
	$(SINC)/esps.h $(SINC)/fea.h $(SINC)/feasd.h
	$(ANSI_CC) $(CFLAGS) $(SRCS) $(SPSLIB) -lm -o $(PROGNAME)


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

