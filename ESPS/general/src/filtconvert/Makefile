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
# @(#)Makefile	1.4 6/13/91 ERL
# 
# Makefile for: filtconvert
#
# Written by: Ken Nelson 
# Checked by:
# Revised by:
 	
CFLAGS = -g $(PROGCFLAGS) 
MANNAME = filtconver.1
DEFPARAM = Pfiltconvert

all :
	cd fea2filt; emake -f mk.fea2filt
	cd filt2fea; emake -f mk.filt2fea

install: 
	cd fea2filt; emake -f mk.fea2filt install
	cd filt2fea; emake -f mk.filt2fea install
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
	-(cd $(MANDIR)/man1; rm filt2fea.1 ; ln -s $(MANNAME) filt2fea.1)
	-(cd $(MANDIR)/man1; rm fea2filt.1 ; ln -s $(MANNAME) fea2filt.1)

# install the default parameter file
	-rm -f $(PARAMDIR)/$(DEFPARAM)
	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)

clean:	
	cd fea2filt; emake -f mk.fea2filt clean;
	cd filt2fea; emake -f mk.filt2fea clean;
	
test:
	sh filtconv.test
