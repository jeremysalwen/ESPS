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
# %W% %G% ERL
# 
# Makefile for:  splay
#
# Written by:  Derek Lin
# Checked by:
# Revised by:
 	
CFLAGS = -O -misalign -I../include -I$(ESPS_BASE)/include/waves -I$(XVIEW_INC) -DFOR_XVIEW $(PROGCFLAGS) 
LIBSIG = $(ESPS_BASE)/lib/waves/xlibsig.a  $(LIBDIR)/libhdrw.a libespsg.a

splay: splay.o dac_codec.o  codec_stubs.o ereader.o $(LIBSIG)
	cc -o splay splay.o dac_codec.o  codec_stubs.o ereader.o $(LIBSIG)  -lm

libespsg.a: $(LIBDIR)/libespsg.a
	cp $(LIBDIR)/libespsg.a .
	ar dv libespsg.a ereader.o
	ranlib libespsg.a

ereader.o: ./ereader.c

install:	$(BINDIR)/splay 

clean:
	-rm -f *.o *~ core splay *.a

$(BINDIR)/splay: splay
	-mv -f $(BINDIR)/splay $(OLDBIN)/splay
	-chmod $(PROGMOD) $(OLDBIN)/splay
	-rm -f $(BINDIR)/splay
	mv splay $(BINDIR)/splay
	-$(STRIP) $(BINDIR)/splay
	chmod $(PROGMOD) $(BINDIR)/splay
	-rm -f $(MANDIR)/man1/splay.1
	cp splay.1 $(MANDIR)/man1/splay.1
	chmod $(MANMOD) $(MANDIR)/man1/splay.1


clobber: 
	rm -f cplay

