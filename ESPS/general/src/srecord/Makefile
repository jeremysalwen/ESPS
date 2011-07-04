# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.2 11/13/92 ERL
# 
# Makefile for: 
#
# Written by:  Derek Lin
# Checked by:
# Revised by:
 	

CFLAGS = -O -misalign -I../include -I -I$(XVIEW_INC) -DFOR_XVIEW $(PROGCFLAGS) 
SOURCES = *.c

srecord: srecord.o adc_codec.o 
	cc -o srecord  srecord.o adc_codec.o  $(WSPSLIB) -lm

install:	$(BINDIR)/srecord 

clean:
	-rm -f *.o *~ core

$(BINDIR)/srecord: srecord
	-mv -f $(BINDIR)/srecord $(OLDBIN)/srecord
	-chmod $(PROGMOD) $(OLDBIN)/srecord
	-rm -f $(BINDIR)/srecord
	mv srecord $(BINDIR)/srecord
	-$(STRIP) $(BINDIR)/srecord
	chmod $(PROGMOD) $(BINDIR)/srecord
	-rm -f $(MANDIR)/man1/srecord.1
	cp srecord.1 $(MANDIR)/man1/srecord.1
	chmod $(MANMOD) $(MANDIR)/man1/srecord.1
