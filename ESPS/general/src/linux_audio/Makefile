#
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1987-1990  AT&T, Inc. 
#    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# %W% %G% ERL
# 
# Written by:  
# Checked by:
# Revised by: David Talkin, Alan Parker
# 
#

SUNLIB = $(XVIEW_B_OPT) $(XVIEW_LIB) -lm

CFLAGS = -g -I$(SPSDIR)/include/waves -I$(XVIEW_INC) $(PROGCFLAGS) -DPLAY

LIBSIG = $(WAVESDIR)/xlibsig.a
LBIN = $(BINDIR)

install: all man
	$(STRIP) lrecord
	$(STRIP) lplay
	mv -f lrecord $(BINDIR)/lrecord
	mv -f lplay $(BINDIR)/lplay
	-rm -f $(BINDIR)/eplay
	ln -s $(BINDIR)/lplay $(BINDIR)/eplay

man : lplay.1 lrecord.1
	-diffmk $(MANDIR)/man1/lplay.1 lplay.1
	-diffmk $(MANDIR)/man1/lrecord.1 lrecord.1
	@echo diffmked man page left in man.diff
	-rm $(MANDIR)/man1/lplay.1
	-rm $(MANDIR)/man1/lrecord.1
	cp lplay.1 $(MANDIR)/man1/lplay.1
	cp lrecord.1 $(MANDIR)/man1/lrecord.1
	chmod $(MANMOD) $(MANDIR)/man1/lplay.1
	chmod $(MANMOD) $(MANDIR)/man1/lrecord.1

all: lrecord lplay

lrecord: newrec.o adc.o  version.o 
	cc -o lrecord  newrec.o adc.o  version.o  \
              $(LIBSIG) $(WSPSLIB) $(WINLIBS) $(AUDIO_LIB) -lm

lplay: newplay.o dac.o  version.o 
	cc -o lplay  newplay.o dac.o version.o  \
	 $(LIBSIG) $(NSPSLIB) $(WINLIBS) -lm $(BSDLIB)

newplay.c: ../s16audio/newplay.c
	rm -f newplay.c
	cp ../s16audio/newplay.c .

newrec.c: ../s16audio/newrec.c
	rm -f newrec.c
	cp ../s16audio/newrec.c .


clean:
	-rm -f *.o *~ core 

clobber: clean
	rm -f lplay lrecord newplay.c dac_linux.c newrec.c 

