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
# @(#)Makefile	1.4 12/4/95 ERL
# 
# Makefile for: ps2frame
#
# Written by:  Alan Parker
# Checked by:
# Revised by:
 	
PROGNAME=ps2frame
MANNAME=ps2frame.1


install: ps2frame
	rm -f $(BINDIR)/$(PROGNAME)
	cp ${PROGNAME} $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-rm $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)


clean:	
	-rm -f ps2frame *.o
	
