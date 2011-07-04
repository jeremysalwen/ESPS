# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.4 7/24/91 ERL
# 
# Makefile for:  nodiff
#
# Written by:  Bill Byrne
# Checked by:
# Revised by:
 	
CFLAGS = -g $(PROGCFLAGS) 

PROGNAME = nodiff
MANNAME = nodiff.1
FILTDIR = $(SPSDIR)/lib/filters

install: $(PROGNAME)
	cp  $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
	-cp *.filt $(FILTDIR)
	chmod $(LIBMOD) $(FILTDIR)/*.filt

clean:	
	@echo "Nothing to clean here."

lint:	
	@echo "Nothing to lint here"

test:	$(PROGNAME)
	sh $(PROGNAME).t

