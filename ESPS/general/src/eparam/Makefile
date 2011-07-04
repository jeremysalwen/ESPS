# SPS makefile for eparam (1-ESPS)


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
# @(#)Makefile	1.9	11/18/96	ESI/ERL
# 
# Written by:  Alan Parker
# Checked by:
# Revised by:
# 
# Makefile for:  eparam
# 

mkeparam :  
	chmod $(PROGMOD) eparam
	-ln -s eparam xeparam

install: mkeparam
	-rm -f $(BINDIR)/eparam
	cp eparam $(BINDIR)/eparam
	-(cd $(BINDIR); rm xeparam; ln -s eparam xeparam )
	chmod $(PROGMOD) $(BINDIR)/eparam
	-diffmk $(MANDIR)/man1/eparam.1 eparam.1 eparam.diff
	@echo diffmked man page left in eparam.diff
	-rm $(MANDIR)/man1/eparam.1
	cp eparam.1 $(MANDIR)/man1/eparam.1
	chmod $(MANMOD) $(MANDIR)/man1/eparam.1
	-rm $(MANDIR)/man1/xeparam.1
	cp xeparam.1 $(MANDIR)/man1/xeparam.1
	chmod $(MANMOD) $(MANDIR)/man1/xeparam.1

test: 
	@echo Now running test script
	-sh eparam.t

clean:
	-rm eparam.lo test.sd test.spec

lint:
	@echo Nothing here to lint
