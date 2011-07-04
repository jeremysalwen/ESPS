# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1998  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.1 3/17/98 ERL
# 
# Makefile for: featohtk
#
# Written by:  Rod Johnson
# Checked by:
# Revised by:
 	
CFLAGS = -O $(PROGCFLAGS) 

OBJS = featohtk.o
SRCS = featohtk.c
PROGNAME = featohtk
MANNAME = featohtk.1
#DEFPARAM = 


$(PROGNAME): $(SPSLIB) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

featohtk.o: $(SINC)/esps.h $(SINC)/range_switch.h $(SINC)/strlist.h

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
