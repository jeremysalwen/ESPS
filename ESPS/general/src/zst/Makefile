# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# 
# @(#)Makefile	1.1 6/30/93 ERL
#
# Written by: 
# Checked by:
# Revised by:
 	

PROGNAME = zst


install: $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	cp $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-rm -f $(MANDIR)/man1/zst.1
	cp zst.1 $(MANDIR)/man1
	chmod $(MANMOD) $(MANDIR)/man1/zst.1

clean:	
	@echo nothing here to clean
	
