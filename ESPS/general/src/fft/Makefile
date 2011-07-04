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
# @(#)Makefile	3.15	9/10/93	ESI/ERL
# 
# Written by:  Shore
# Checked by:
# Revised by:
# 
# Brief description: makefile for fft
# 
 	
CFLAGS = -g $(PROGCFLAGS) 

OBJS = fft.o 
SRCS = fft.c 

PROGNAME = fft
XPROGNAME = xfft

MANNAME = fft.1
XMANNAME = xfft.1

DEFPARAM = Pfft
DEFPARAMW = PWfft
DEFPARAMWC = PWfftc

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h $(SINC)/scbk.h

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
# install the X cover
	-mv -f $(BINDIR)/$(XPROGNAME) $(OLDBIN)/$(XPROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(XPROGNAME)
	cp $(XPROGNAME) $(BINDIR)/$(XPROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(XPROGNAME)
	-rm $(MANDIR)/man1/$(XMANNAME)
	ln $(MANDIR)/man1/$(MANNAME) $(MANDIR)/man1/$(XMANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(XMANNAME)
# install the default parameter files
	-rm -f $(PARAMDIR)/$(DEFPARAM)
	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)
	-rm -f $(PARAMDIR)/$(DEFPARAMW)
	cp $(DEFPARAMW) $(PARAMDIR)/$(DEFPARAMW)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAMW)
	-rm -f $(PARAMDIR)/$(DEFPARAMWC)
	cp $(DEFPARAMWC) $(PARAMDIR)/$(DEFPARAMWC)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAMWC)

clean:	
	-rm -f $(OBJS) $(PROGNAME) *.sd core
	
lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > lint

test:	$(PROGNAME)
	sh $(PROGNAME).test
