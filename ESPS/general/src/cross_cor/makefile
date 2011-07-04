#
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
# @(#)makefile	1.7	11/7/91	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for:  cross_cor
# 

CFLAGS =  -O $(PROGCFLAGS)

OBJS = cross_cor.o
SRCS = cross_cor.c
PROGNAME =  cross_cor
MANNAME = crosscor.1
DEFPARAM = Pcross_cor

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

cross_cor.o: $(SINC)/esps.h $(SINC)/unix.h $(SINC)/sd.h $(SINC)/fea.h \
	$(SINC)/window.h


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
# update src_examples directory
	-rm -rf $(SPSDIR)/src_examples/cross_cor
	mkdir $(SPSDIR)/src_examples/cross_cor
	cp * $(SPSDIR)/src_examples/cross_cor
	(cd $(SPSDIR)/src_examples/cross_cor; emake clean)

clean:	
	-rm -f $(PROGNAME) $(OBJS)
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > lint

test:	$(PROGNAME)
	sh testfile
