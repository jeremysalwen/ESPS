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
# @(#)Makefile	1.10 9/28/98 ERL
# 
# Makefile for: 
#
# Written by:  Ken Nelson
# Checked by:
# Revised by:
 	
CFLAGS = -g $(PROGCFLAGS) 

PROGNAME = einfo


install: $(PROGNAME)
	-rm -f $(BINDIR)/$(PROGNAME)
	cp $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)

clean:
	@echo nothing here to clean
