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
# @(#)Makefile	1.2 7/30/91 ERL
# 
# Makefile for: 
#
# Written by: Ken Nelson 
# Checked by:
# Revised by:
 	
CFLAGS = -g $(PROGCFLAGS) 

PROGNAME = erlsupport
MANNAME = erlsupport.1

chmod_erlsupport: 
	chmod $(PROGMOD) erlsupport

install: 
	cp $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
	cp erlsupport.defs.std $(LIBDIR)/erlsupport.defs
	chmod $(LIBMOD) $(LIBDIR)/erlsupport.defs

clean:
	echo "Ain't nothing to clean here."

lint:	 
	echo "Ain't nothing to lint here."

test:
	./erlsupport

