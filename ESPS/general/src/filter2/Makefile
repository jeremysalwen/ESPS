# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# %W% %G% ERL
# 
# Makefile for: 
#
# Written by:  Derek Lin
# Checked by:
# Revised by:
 	
CFLAGS = -O $(PROGCFLAGS) 

OBJS = filter2.o
SRCS = filter2.c
PROGNAME = filter2
MANNAME = filter2.1
DEFPARAM = Pfilter2

$(PROGNAME): $(SPSLIB) $(OBJS)
	$(CC)  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h $(SINC)/filt.h $(SINC)/sd.h

install: $(PROGNAME)
	-$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
	-rm -f $(BINDIR)/filter
	ln $(BINDIR)/filter2 $(BINDIR)/filter
	-rm -f $(MANDIR)/man1/filter.1
	ln $(MANDIR)/man1/filter2.1 $(MANDIR)/man1/filter.1
# install the default parameter file
	-rm -f $(PARAMDIR)/$(DEFPARAM)
	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)

clean:	
	-rm -f $(OBJS) test.sd test ref
	-rm -f *o *~
	
lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm  > lint

test:	$(PROGNAME)
	-sh $(PROGNAME).test

sort: sort.o
	cc -o sort sort.o -lm

