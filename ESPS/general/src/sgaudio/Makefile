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
# %W%	%G%	ATT/ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by: David Talkin, Alan Parker
# 
#

SUNLIB = $(XVIEW_B_OPT) $(XVIEW_LIB) -lm

CFLAGS = -g -I$(SPSDIR)/include/waves -I$(XVIEW_INC) $(PROGCFLAGS) -DSGPLAY

LIBSIG = $(WAVESDIR)/xlibsig.a
LBIN = $(BINDIR)

install: all man
	$(STRIP) sgrecord
	$(STRIP) sgplay
	mv -f sgrecord $(BINDIR)/sgrecord
	mv -f sgplay $(BINDIR)/sgplay
	-rm -f $(BINDIR)/eplay
	ln -s $(BINDIR)/sgplay $(BINDIR)/eplay

man : sgplay.1 sgrecord.1
	-diffmk $(MANDIR)/man1/sgplay.1 sgplay.1
	-diffmk $(MANDIR)/man1/sgrecord.1 sgrecord.1
	@echo diffmked man page left in man.diff
	-rm $(MANDIR)/man1/sgplay.1
	-rm $(MANDIR)/man1/sgrecord.1
	cp sgplay.1 $(MANDIR)/man1/sgplay.1
	cp sgrecord.1 $(MANDIR)/man1/sgrecord.1
	chmod $(MANMOD) $(MANDIR)/man1/sgplay.1
	chmod $(MANMOD) $(MANDIR)/man1/sgrecord.1

all: sgrecord sgplay

sgrecord: newrec.o adc_indigo.o  version.o indigo_utils.o
	cc -o sgrecord  newrec.o adc_indigo.o indigo_utils.o version.o \
              $(LIBSIG) $(WSPSLIB) -lX11 -laudio -lm

sgplay: newplay.o dac_indigo.o  version.o indigo_stubs.o
	cc -o sgplay  newplay.o dac_indigo.o version.o indigo_stubs.o \
	 $(LIBSIG) $(WSPSLIB) -lX11 -laudio -lm

newplay.c: ../s16audio/newplay.c
	rm -f newplay.c
	cp ../s16audio/newplay.c .

dac_indigo.c: ../../../waves.esi/waves/src/c/dac_indigo.c
	rm -f dac_indigo.c
	cp ../../../waves.esi/waves/src/c/dac_indigo.c .

newrec.c: ../s16audio/newrec.c
	rm -f newrec.c
	cp ../s16audio/newrec.c .


clean:
	-rm -f *.o *~ core 

clobber: clean
	rm -f sgplay sgrecord newplay.c dac_indigo.c newrec.c 

