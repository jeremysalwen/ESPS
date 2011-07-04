# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1998 Entropic, Inc.  All rights reserved."
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.1 9/10/98 ERL
# 
# Makefile for: mbs_dist
#
# Written by:  Rod Johnson
# Checked by:
# Revised by:
 	
CFLAGS = -g $(PROGCFLAGS) 
CC = $(ANSI_CC)

OBJS = mbs_dist.o
SRCS = mbs_dist.c
PROGNAME = mbs_dist
MANNAME = mbsdist.1
DEFPARAM = Pmbs_dist


$(PROGNAME): $(SPSLIB) $(OBJS)
	$(ANSI_CC) $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h $(SINC)/fea.h $(SINC)/feaspec.h

mbs_dist.o:
	$(ANSI_CC) -c $(CFLAGS) mbs_dist.c

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
	-rm -f $(PARAMDIR)/$(DEFPARAM)
	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)

codecenter_src: $(SRCS)
	#load $(CFLAGS) $(SPSLIB) $(XVLIB) -lm $(SRCS)

clean:	testclean
	-rm -f $(OBJS) $(PROGNAME) core 
	
lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	sh $(PROGNAME).tst
	sh $(PROGNAME).tst clean
# Comment out the last line above to keep "emake test" from removing
# the test files after running the tests.
testclean:
	sh $(PROGNAME).tst clean

