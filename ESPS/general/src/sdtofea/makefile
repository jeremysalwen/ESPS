#
#  This material contains proprietary software of Entropic Speech,
#  Inc.  Any reproduction, distribution, or publication without the
#  prior written permission of Entropic Soeech, Inc. is strictly
#  prohibited.  Any public distribution of copies of this work
#  authorized in writing by Entropic Speech, Inc. must bear the notice
#
#    "Copyright (c) 1989 Entropic Speech, Inc. All rights reserved."
#
#
#  makefile for sdtofea
#  @(#)makefile	1.2	3/23/90	ESI
#
CFLAGS =  -O $(PROGCFLAGS)

OBJS = sdtofea.o
SRCS = sdtofea.c
PROGNAME = sdtofea
MANNAME = sdtofea.1

$(PROGNAME): $(WSPSLIB) $(OBJS)
	$(CC)  $(CFLAGS) $(OBJS) $(WSPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h $(SINC)/unix.h $(SINC)/fea.h $(SINC)/feasd.h

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
