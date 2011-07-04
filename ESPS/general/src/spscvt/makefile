#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice
#
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
#
#
# makefile for spscvt(3-ESPS) @(#)makefile	3.4 12/4/96 ESI
 	

CFLAGS = $(PROGCFLAGS)

all:	atosps spstoa

atosps:	atospsg.o fea_sup.o $(SPSLIB)
	cc $(CFLAGS) atospsg.o fea_sup.o $(SPSLIB) -lm -o atosps

spstoa:	spstoag.o fea_sup.o $(SPSLIB)
	cc $(CFLAGS) spstoag.o fea_sup.o $(SPSLIB) -lm -o spstoa

atospsg.o:   tagdef.h $(SINC)/esps.h atospsg.c

spstoag.o:   tagdef.h $(SINC)/esps.h spstoag.c

fea_sup.o:   fea_sup.c

lint:	fea_sup.c atospsg.c spstoag.c
	$(LINT) $(LINTFLAGS) atospsg.c fea_sup.c $(LINTLIB) >atospsg.lint
	$(LINT) $(LINTFLAGS) spstoag.c fea_sup.c $(LINTLIB) >spstoag.lint

install: spstoa atosps 
	@echo spstoa...
	-mv -f $(BINDIR)/spstoa $(OLDBIN)/spstoa
	-chmod $(PROGMOD) $(OLDBIN)/spstoa
	mv spstoa $(BINDIR)/spstoa
	-strip $(BINDIR)/spstoa
	chmod $(PROGMOD) $(BINDIR)/spstoa
	-diffmk  $(MANDIR)/man1/spstoa.1 spstoa.1 spstoa.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/spstoa.1
	cp spstoa.1 $(MANDIR)/man1/spstoa.1
	chmod $(MANMOD) $(MANDIR)/man1/spstoa.1

	@echo atosps...
	-mv -f $(BINDIR)/atosps $(OLDBIN)/atosps
	-chmod $(PROGMOD) $(OLDBIN)/atosps
	mv atosps $(BINDIR)/atosps
	-strip $(BINDIR)/atosps
	chmod $(PROGMOD) $(BINDIR)/atosps
	-diffmk  $(MANDIR)/man1/atosps.1 atosps.1 atosps.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/atosps.1
	cp atosps.1 $(MANDIR)/man1/atosps.1
	chmod $(MANMOD) $(MANDIR)/man1/atosps.1


clean:
	rm -f spstoag.o atospsg.o fea_sup.o atosps spstoa

test: spstoa atosps
	sh testfile
