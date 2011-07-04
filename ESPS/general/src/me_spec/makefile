# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)makefile	1.7	6/13/91	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Brief description: makefile for me_sgram
# 

CFLAGS = -O $(PROGCFLAGS) 

OBJS = me_spec.o getarspec.o version.o genanasupp.o
SRCS = me_spec.c getarspec.c version.c genanasupp.c

PROGNAME = me_spec
MANNAME = mespec.1

DEFPARAM = Pme_spec
DEFPARAMW = PWme_spec

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

me_spec.o: $(SINC)/unix.h $(SINC)/esps.h $(SINC)/fea.h $(SINC)/anafea.h $(SINC)/spec.h genana.h

getarspec.o: $(SINC)/unix.h $(SINC)/esps.h $(SINC)/constants.h

genanasupp.o: $(SINC)/esps.h $(SINC)/fea.h genana.h

install: $(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
# make a link to fanaspec
# this will probably come out in a later version
	rm -f $(BINDIR)/fanaspec
	ln $(BINDIR)/$(PROGNAME) $(BINDIR)/fanaspec
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm -f $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
# install the default parameter files
	-rm -f $(PARAMDIR)/$(DEFPARAM)
	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)
	-rm -f $(PARAMDIR)/$(DEFPARAMW)
	cp $(DEFPARAMW) $(PARAMDIR)/$(DEFPARAMW)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAMW)
	
clean:	
	-rm -f $(OBJS) $(PROGNAME)

lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS)  $(SRCS) $(LINTLIB) -lm > lint

test:	$(PROGNAME)
	-sh $(PROGNAME).tes $(LIBDIR)
