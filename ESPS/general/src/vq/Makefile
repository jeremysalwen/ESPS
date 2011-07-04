
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
# @(#)Makefile	3.4	12/4/95	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for: vq
# 

CFLAGS = -O $(PROGCFLAGS) 

OBJS = vqdes.o vq.o vqdesasc.o vqasc.o
PROGS = vqdes vq vqdesasc vqasc

all:		$(PROGS)

install:	$(BINDIR)/vqdes $(BINDIR)/vq $(BINDIR)/vqdesasc \
		$(BINDIR)/vqasc 

vqdesasc:	vqdesasc.o $(SPSLIB)
		cc $(CFLAGS) vqdesasc.o $(SPSLIB) -lm -o vqdesasc

vqdesasc.o:	$(SINC)/esps.h

$(BINDIR)/vqdesasc:	vqdesasc
	-mv -f $(BINDIR)/vqdesasc $(OLDBIN)/vqdesasc
	-chmod $(PROGMOD) $(OLDBIN)/vqdesasc
	-rm -f $(BINDIR)/vqdesasc
	mv vqdesasc $(BINDIR)/vqdesasc
	$(STRIP) $(BINDIR)/vqdesasc
	chmod $(PROGMOD) $(BINDIR)/vqdesasc
	-diffmk  $(MANDIR)/man1/vqdesasc.1 vqdesasc.1 vqdesasc.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/vqdesasc.1
	cp vqdesasc.1 $(MANDIR)/man1/vqdesasc.1
	chmod $(MANMOD) $(MANDIR)/man1/vqdesasc.1
	-rm -f $(PARAMDIR)/Pvqdesasc
	cp Pvqdes $(PARAMDIR)/Pvqdesasc
	chmod $(LIBMOD) $(PARAMDIR)/Pvqdesasc


vqasc:		vqasc.o $(SPSLIB)
		cc $(CFLAGS) vqasc.o $(SPSLIB) -lm -o vqasc

vqasc.o:	$(SINC)/esps.h 

$(BINDIR)/vqasc: 	vqasc
	-mv -f $(BINDIR)/vqasc $(OLDBIN)/vqasc
	-chmod $(PROGMOD) $(OLDBIN)/vqasc
	-rm -f $(BINDIR)/vqasc
	mv vqasc $(BINDIR)/vqasc
	$(STRIP) $(BINDIR)/vqasc
	chmod $(PROGMOD) $(BINDIR)/vqasc
	-diffmk  $(MANDIR)/man1/vqasc.1 vqasc.1 vqasc.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/vqasc.1
	cp vqasc.1 $(MANDIR)/man1/vqasc.1
	chmod $(MANMOD) $(MANDIR)/man1/vqasc.1


vqdes:		vqdes.o $(SPSLIB)
		cc $(CFLAGS) vqdes.o $(SPSLIB) -lm -o vqdes

vqdes.o:	$(SINC)/esps.h

$(BINDIR)/vqdes: 	vqdes
	-mv -f $(BINDIR)/vqdes $(OLDBIN)/vqdes
	-chmod $(PROGMOD) $(OLDBIN)/vqdes
	-rm -f $(BINDIR)/vqdes
	mv vqdes $(BINDIR)/vqdes
	$(STRIP) $(BINDIR)/vqdes
	chmod $(PROGMOD) $(BINDIR)/vqdes
	-diffmk  $(MANDIR)/man1/vqdes.1 vqdes.1 vqdes.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/vqdes.1
	cp vqdes.1 $(MANDIR)/man1/vqdes.1
	chmod $(MANMOD) $(MANDIR)/man1/vqdes.1
	-rm -f $(PARAMDIR)/Pvqdes
	cp Pvqdes $(PARAMDIR)/Pvqdes
	chmod $(LIBMOD) $(PARAMDIR)/Pvqdes

vq:		vq.o $(SPSLIB)
		cc $(CFLAGS) vq.o $(SPSLIB) -lm -o vq

vq.o:		$(SINC)/esps.h

$(BINDIR)/vq: 	vq
	-mv -f $(BINDIR)/vq $(OLDBIN)/vq
	-chmod $(PROGMOD) $(OLDBIN)/vq
	-rm -f $(BINDIR)/vq
	mv vq $(BINDIR)/vq
	$(STRIP) $(BINDIR)/vq
	chmod $(PROGMOD) $(BINDIR)/vq
	-diffmk  $(MANDIR)/man1/vq.1 vq.1 vq.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/vq.1
	cp vq.1 $(MANDIR)/man1/vq.1
	chmod $(MANMOD) $(MANDIR)/man1/vq.1

clean:
		-rm -f $(OBJS) $(PROGS)
		-rm -f rc* asc* *.chk vqparams *hist test* *.file *.asc core


lint:		
		$(LINT) $(LINTFLAGS) vqdesasc.c $(LINTLIB) -lm > lint
		$(LINT) $(LINTFLAGS)  vqasc.c $(LINTLIB) -lm  >> lint
		$(LINT) $(LINTFLAGS)  vqdes.c $(LINTLIB) -lm  >> lint
		$(LINT) $(LINTFLAGS)  vq.c $(LINTLIB)  -lm >> lint

test: $(PROGS)
	rm -f rc* asc* *.chk vqparams *hist test* *.file *.asc core	
	sh vq.test
