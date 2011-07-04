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
# @(#)Makefile	1.4 11/11/96 ERL
# 
# Makefile for: 
#
# Written by:  Derek Lin
# Checked by:
# Revised by:
 	
CFLAGS = -O -I$(ESPS_BASE)/include/waves $(PROGCFLAGS) 
SIGPROC = $(ESPS_BASE)/lib/waves/libdsigproc.a

OBJS = ps_ana.o 
SRCS = ps_ana.c 
PROGNAME = ps_ana
MANNAME = ps_ana.1
DEFPARAM = Pps_ana


$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) $(SIGPROC) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h 

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

clean:	
	-rm -f $(OBJS) $(PROGNAME) core 
	
lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	csh $(PROGNAME).test

