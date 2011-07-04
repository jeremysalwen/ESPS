#
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1987-1990  AT&T, Inc. 
#    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.4	1/15/93	ATT/ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by: David Talkin
# 
#  Makefile for new (and hopefully final!) v32play and v32record
#

SUNLIB = $(XVIEW_B_OPT) $(XVIEW_LIB) -lm

CFLAGS = -g -I../h -I$(XVIEW_INC) -I../../../dsp32/lib  -I../../../dsp32/inc -DFOR_XVIEW $(PROGCFLAGS) -DUSE_DSP32C_VME

LPATH = $(WAVESDIR)
LBIN = $(BINDIR)
LIBSIG = $(WAVESDIR)/xlibsig.a
# LINE BELOW FOR TESTING ONLY.
# LIBSIG = ../../../libsig/xlibsig.a
WSOURCES = nplay.c  ddsetrate.c ddutils.c playrec.c setrate.c

progs: v32play v32record

v32play: newplay.o   nplay.o  surf_stubs.o fab2_stubs.o playrec.o \

	setrate.o ddsetrate.o   ddutils.o
	cc -o v32play  newplay.o nplay.o  surf_stubs.o \
	       fab2_stubs.o playrec.o \
	setrate.o ddsetrate.o  ddutils.o \
        $(LIBSIG)   $(WAVESDIR)/libds.a $(WSPSLIB) -lm

newplay.c:
	esccs indigo_playrec get newplay.c

v32record:  ddsetrate.o nrec.o record.o  $(LIBSIG)  ddutils.o version.o
	cc  -o v32record ${CFLAGS} record.o  nrec.o ddsetrate.o  ddutils.o \
	$(LIBSIG) $(WAVESDIR)/libds.a  version.o $(WSPSLIB) -lm

nrec.c: 
	esccs wrecord get nrec.c

install: progs
	$(STRIP) v32play
	-rm -f $(BINDIR)/v32play
	mv v32play $(BINDIR)
	chmod $(PROGMOD) $(BINDIR)/v32play
	-rm -f $(MANDIR)/man1/v32play.1
	cp v32play.1 $(MANDIR)/man1/v32play.1
	chmod $(MANMOD) $(MANDIR)/man1/v32play.1

	$(STRIP) v32record
	-rm -f $(BINDIR)/v32record
	mv v32record $(BINDIR)
	chmod $(PROGMOD) $(BINDIR)/v32record
	-rm -f $(MANDIR)/man1/v32record.1
	cp v32record.1 $(MANDIR)/man1/v32record.1
	chmod $(MANMOD) $(MANDIR)/man1/v32record.1


clean:
	-rm -f *.o *~ core v32play v32record

$(WSOURCES): ../c/$@
	rm -f $@
	ln -s ../c/$@ .
