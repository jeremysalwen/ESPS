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
# @(#)Makefile	1.8	6/13/91	ESI/ERL
# 
# Written by:  John Shore
# Checked by:
# Revised by:
# 
# Brief description: makefile for me_sgram
# 

# remove comments for testing only.  Put comments back before sccs'ing.
# SPOOL_CMD = IPR
# PROGCFLAGS = -I/usr/esps3/include
# BINDIR = /tmp
# LIBDIR = /tmp
# MANDIR = /tmp
# PROGMOD = 0755
# MANMOD = 0755


DEFPARAM = Pme_sgram
DEFPARAMW = PWme_sgram

me_sgram: me_sgram.sh
	cp me_sgram.sh me_sgram
	chmod $(PROGMOD) me_sgram

install: $(BINDIR)/me_sgram 

$(BINDIR)/me_sgram: me_sgram
	rm -f $(BINDIR)/me_sgram
	cp me_sgram $(BINDIR)/me_sgram
	chmod $(PROGMOD) $(BINDIR)/me_sgram
	-diffmk $(MANDIR)/man1/me_sgram.1 me_sgram.1 me_sgram.diff
	@echo diffmked man page left in me_sgram.diff
	-rm $(MANDIR)/man1/me_sgram.1
	-cp me_sgram.1 $(MANDIR)/man1/me_sgram.1
	-chmod $(MANMOD) $(MANDIR)/man1/me_sgram.1
# install the default parameter files
	-rm -f $(PARAMDIR)/$(DEFPARAM)
	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)
	-rm -f $(PARAMDIR)/$(DEFPARAMW)
	cp $(DEFPARAMW) $(PARAMDIR)/$(DEFPARAMW)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAMW)
clean:
	@echo nothing here to clean
	
test:	me_sgram
	-sh me_sgram.tes

