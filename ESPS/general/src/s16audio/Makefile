#
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc.  Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# %W% %G% ERL 
# 
# Written by:  Alan Parker
# Checked by:
# Revised by: 
# 
#

CFLAGS = -g -DFOR_STANDALONE -I$(XVIEW_INC) $(PROGCFLAGS) -I$(SPSDIR)/include/waves $(OSFLAG)

LIBSIG = $(WAVESDIR)/xlibsig.a

both: s16play s16record


install: s16play s16record man
	rm -f $(BINDIR)/s16play
	cp s16play $(BINDIR)/s16play
	chmod $(PROGMOD) $(BINDIR)/s16play
	rm -f $(BINDIR)/s16record
	cp s16record $(BINDIR)/s16record
	chmod $(PROGMOD) $(BINDIR)/s16record
	-rm -f $(BINDIR)/eplay
	ln -s $(BINDIR)/s16play $(BINDIR)/eplay



man: 
	-rm $(MANDIR)/man1/s16play.1
	-rm $(MANDIR)/man1/s16record.1
	cp s16play.1 $(MANDIR)/man1/s16play.1
	cp s16record.1 $(MANDIR)/man1/s16record.1
	chmod $(MANMOD) $(MANDIR)/man1/s16play.1
	chmod $(MANMOD) $(MANDIR)/man1/s16record.1

pure_rec: newrec.o adc_dbri.o   dbri_utils.o version.o $(WSPSLIB)
	purify $(CC) -o s16record newrec.o adc_dbri.o dbri_utils.o version.o \
              $(LIBSIG) $(WSPSLIB) $(SOCKET) $(X_LIB) -lm

pure_play: newplay.o dac_dbri.o   dbri_stubs.o version.o $(WSPSLIB)
	purify $(CC) -o s16play  newplay.o dac_dbri.o  dbri_stubs.o version.o \
	 $(LIBSIG) $(WSPSLIB) $(SOCKET) $(X_LIB) -lm



s16record: newrec.o adc_dbri.o   dbri_utils.o version.o $(WSPSLIB)
	$(CC) -o s16record newrec.o adc_dbri.o dbri_utils.o version.o \
              $(LIBSIG) $(WSPSLIB) $(SOCKET) $(X_LIB) -lm

s16play: newplay.o dac_dbri.o   dbri_stubs.o version.o $(WSPSLIB)
	$(CC) -o s16play  newplay.o dac_dbri.o  dbri_stubs.o version.o \
	 $(LIBSIG) $(WSPSLIB) $(SOCKET) $(X_LIB) -lm


clean:
	-rm -f *.o  core  s16record s16play

