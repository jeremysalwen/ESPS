# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# %W%	%G%	ERL
# 
# Brief description: makefile for misc
 	
CFLAGS = -g $(PROGCFLAGS)
LIB = $(SPSLIB)
MAN = $(MANDIR)/man1

PROGS =  eman pspsdiff echeck espsenv  elpr featype

all:		$(PROGS)

install: $(BINDIR)/eman $(BINDIR)/pspsdiff $(BINDIR)/echeck $(BINDIR)/espsenv  $(LIBDIR)/Eenv.csh $(LIBDIR)/Eenv.sh $(BINDIR)/elpr $(BINDIR)/featype

$(LIBDIR)/Eenv.csh:	Eenv.csh
		-rm -f $(LIBDIR)/Eenv.csh
		cp Eenv.csh $(LIBDIR)
		chmod $(LIBMOD) $(LIBDIR)/Eenv.csh

$(LIBDIR)/Eenv.sh:	Eenv.sh
		-rm -f $(LIBDIR)/Eenv.sh
		cp Eenv.sh $(LIBDIR)
		chmod $(LIBMOD) $(LIBDIR)/Eenv.sh

eman:		eman.o $(LIB)
		$(CC)  $(CFLAGS) eman.o $(LIB) -o eman

eman.o:		$(SINC)/esps.h
		$(CC) -c $(CFLAGS) \
		-DDEFAULT_PAGER=\"$(DEFAULT_PAGER)\" \
		-DVTROFF=\"$(VTROFF)\" \
		-DMAN_MACRO=build_esps_path\(\"/lib/tmac.an\"\) \
		eman.c

$(BINDIR)/eman:	eman
		-mv -f $(BINDIR)/eman $(OLDBIN)
		-chmod $(PROGMOD) $(OLDBIN)/eman
		cp eman $(BINDIR)/eman
		chmod $(PROGMOD) $(BINDIR)/eman
		-$(STRIP) $(BINDIR)/eman
		-diffmk $(MAN)/eman.1 eman.1 eman.diff
		@echo diffmked man left in eman.diff
		-rm -f $(MAN)/eman.1
		cp eman.1 $(MAN)/eman.1 
		chmod $(MANMOD) $(MAN)/eman.1

elpr:	elpr.sh
		-rm -f elpr
		cp elpr.sh elpr
		chmod $(PROGMOD) elpr

$(BINDIR)/elpr:	elpr
		-mv -f $(BINDIR)/elpr $(OLDBIN)
		-chmod $(PROGMOD) $(OLDBIN)/elpr
		cp elpr $(BINDIR)/elpr
		chmod $(PROGMOD) $(BINDIR)/elpr
		-diffmk $(MAN)/elpr.1 elpr.1 elpr.diff
		@echo diffmked man left in elpr.diff
		-rm -f $(MAN)/elpr.1
		cp elpr.1 $(MAN)/elpr.1 
		chmod $(MANMOD) $(MAN)/elpr.1

pspsdiff:	pspsdiff.sh
		-rm -f pspsdiff
		cp pspsdiff.sh pspsdiff

$(BINDIR)/pspsdiff:	pspsdiff
		-mv -f $(BINDIR)/pspsdiff $(OLDBIN)
		-chmod $(PROGMOD) $(OLDBIN)/pspsdiff
		cp pspsdiff $(BINDIR)/pspsdiff
		chmod $(PROGMOD) $(BINDIR)/pspsdiff
		-diffmk $(MAN)/pspsdiff.1 pspsdiff.1 pspsdiff.diff
		@echo diffmked man left in pspsdiff.diff
		-rm -f $(MAN)/pspsdiff.1
		cp pspsdiff.1 $(MAN)/pspsdiff.1 
		chmod $(MANMOD) $(MAN)/pspsdiff.1

LFILE=build_esps_path\(\"lib/llib-lespsg.c\"\)
LINTLIB=build_esps_path("lib/llib-lespsg.ln")

echeck:		echeck.o
		$(CC) $(CFLAGS) -DLINTFILE=$(LFILE)  echeck.o $(LIB) -o echeck

echeck.o:	echeck.c
	$(CC) -c $(CFLAGS) -DLINTFILE=$(LFILE) echeck.c 

$(BINDIR)/echeck:	echeck
		-mv -f $(BINDIR)/echeck $(OLDBIN)
		-chmod $(PROGMOD) $(OLDBIN)/echeck
		cp echeck $(BINDIR)/echeck
		chmod $(PROGMOD) $(BINDIR)/echeck
		-$(STRIP) $(BINDIR)/echeck
		-diffmk $(MAN)/echeck.1 echeck.1 echeck.diff
		@echo diffmked man left in echeck.diff
		-rm -f $(MAN)/echeck.1
		cp echeck.1 $(MAN)/echeck.1 
		chmod $(MANMOD) $(MAN)/echeck.1

espsenv:	espsenv.sh
		-rm -f espsenv
		cp espsenv.sh espsenv

$(BINDIR)/espsenv:	espsenv
		-mv -f $(BINDIR)/espsenv $(OLDBIN)
		-chmod $(PROGMOD) $(OLDBIN)/espsenv
		cp espsenv $(BINDIR)/espsenv
		chmod $(PROGMOD) $(BINDIR)/espsenv
		-diffmk $(MAN)/espsenv.1 espsenv.1 espsenv.diff
		@echo diffmked man left in espsenv.diff
		-rm -f $(MAN)/espsenv.1
		cp espsenv.1 $(MAN)/espsenv.1 
		chmod $(MANMOD) $(MAN)/espsenv.1

featype:        featype.o
		$(CC) -o featype $(CFLAGS) featype.o $(SPSLIB) -lm

$(BINDIR)/featype:	featype
		-mv -f $(BINDIR)/featype $(OLDBIN)
		-chmod $(PROGMOD) $(OLDBIN)/featype
		-strip featype
		cp featype $(BINDIR)/featype
		chmod $(PROGMOD) $(BINDIR)/featype


clean:
		rm -f *.o $(PROGS)
