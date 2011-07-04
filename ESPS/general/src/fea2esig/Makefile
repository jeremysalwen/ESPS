# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1995-1997 Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.1	10/4/97	ERL
# 
# Written by:   Rod Johnson, Alan Parker
# Checked by:
# Revised by:
# 
# Makefile for:  fea2esig
# 

CFLAGS = -g $(PROGCFLAGS)

PROG = fea2esig
SRC = fea2esig.c
MAN = fea2esig.1

ESIGBASE = ../lib_header/esignal

# LIBS doesn't need an explicit "libesignal.a"
# because it's contained in libheader.a.
LIBS = $(SPSLIB)

prog: $(PROG)

$(PROG): $(SRC) $(LIBS) $(SINC)/esps.h $(SINC)/fea.h $(ESIGBASE)/src/esignal.h
	$(ANSI_CC) $(CFLAGS) -I$(ESIGBASE)/src $(SRC) $(LIBS) -lm -o $(PROG)

install: prog
	-rm -f $(BINDIR)/$(PROG)
	$(STRIP) $(PROG)
	mv $(PROG) $(BINDIR)
	chmod $(PROGMOD) $(BINDIR)/$(PROG)
	-rm -f $(MANDIR)/man1/$(MAN)
	cp $(MAN) $(MANDIR)/man1
	chmod $(MANMOD) $(MANDIR)/man1/$(MAN)

clean:
	-rm -f $(PROG) core
