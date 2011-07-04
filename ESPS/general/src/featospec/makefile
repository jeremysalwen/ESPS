#
#  This material contains proprietary software of Entropic Speech,
#  Inc.  Any reproduction, distribution, or publication without the
#  prior written permission of Entropic Soeech, Inc. is strictly
#  prohibited.  Any public distribution of copies of this work
#  authorized in writing by Entropic Speech, Inc. must bear the notice
#
#    "Copyright (c) 1988 Entropic Speech, Inc. All rights reserved."
#
#
#  makefile for featospec
#  @(#)makefile	1.1	11/7/88	ESI
#
CFLAGS =  -g $(PROGCFLAGS)

OBJS = featospec.o 
SRCS = featospec.c
PROGNAME = featospec
MANNAME = featospec.1

$(PROGNAME): $(SPSLIB) $(OBJS)
	$(CC)  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h $(SINC)/unix.h $(SINC)/fea.h $(SINC)/feaspec.h

install: $(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm -f $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

clean:	
	-rm -f $(OBJS) $(PROGNAME)  lint
	
lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > lint

test:	$(PROGNAME)
	echo No test script yet.
#	-sh $(PROGNAME).test
