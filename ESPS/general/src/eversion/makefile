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
# %W% %G% ERL
#
# Makefile for: eversion
#
# Written by: Alan Parker
# Checked by:
# Revised by: Ken Nelson
# 


install: $(BINDIR)/eversion $(SPSDIR)/version

$(BINDIR)/eversion: eversion
	rm -f $(BINDIR)/eversion
	cp eversion $(BINDIR)/eversion
	chmod $(PROGMOD) $(BINDIR)/eversion
	-rm $(MANDIR)/man1/eversion.1
	cp eversion.1 $(MANDIR)/man1/eversion.1
	chmod $(MANMOD) $(MANDIR)/man1/eversion.1

$(SPSDIR)/version: version
	rm -f $(SPSDIR)/version*
	cp version $(SPSDIR)/version
	ln -s $(SPSDIR)/version $(SPSDIR)/version5.3

clean:
	@echo nothing here to clean

test:
	@echo no test here
