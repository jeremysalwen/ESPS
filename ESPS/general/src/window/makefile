
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
# @(#)makefile	1.2	6/13/91	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for:  window
# 

CFLAGS =  -O $(PROGCFLAGS)

OBJS = window.o
SRCS = window.c
PROGNAME =  window
MANNAME = window.1
DEFPARAM = Pwindow

$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

window.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/fea.h 


install: $(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
# install the default parameter file
	-rm -f $(PARAMDIR)/$(DEFPARAM)
	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)

clean:	
	-rm -f $(PROGNAME) $(OBJS) test.sd lint
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	sh $(PROGNAME).test
