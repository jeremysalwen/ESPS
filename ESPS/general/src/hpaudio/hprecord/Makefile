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
# Revised by: 
# 
#


CFLAGS = -g -I$(ESPS_BASE)/include/waves $(PROGCFLAGS)

LIBSIG = $(ESPS_BASE)/lib/waves/xlibsig.a $(SPSLIB)
LBIN = $(BINDIR)

hprecord: newrec.o adc_hpaudiolib.o
	cc -o $@  newrec.o adc_hpaudiolib.o $(WSPSLIB) $(AUDIO_LIB) $(X_LIB) -lm

newrec.c: ../../s16audio/newrec.c
	rm -f newrec.c
	cp ../../s16audio/newrec.c .

install: hprecord
	$(STRIP) hprecord
	-rm -f $(BINDIR)/hprecord
	mv -f hprecord $(BINDIR)/hprecord

	-rm $(MANDIR)/man1/hprecord.1
	cp hprecord.1 $(MANDIR)/man1/hprecord.1
	chmod $(MANMOD) $(MANDIR)/man1/hprecord.1

clean:
	-rm -f *.o *~ core

