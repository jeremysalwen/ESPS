#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice
#
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
#
# makefile for distort(1-ESPS)  @(#)makefile	3.1	11/3/87	ESI
 	

CFLAGS = -O $(PROGCFLAGS)


DISTORT_OBJS = distort.o check_dist.o  gendistort.o prnt_dist.o \
	       specdistort.o anafeadist.o anamodules.o

DISTORT_SRCS = distort.c check_dist.c gendistort.c prnt_dist.c \
	       specdistort.c anafeadist.c anamodules.c

distort:	$(DISTORT_OBJS) $(SPSLIB)
		cc $(CFLAGS) $(DISTORT_OBJS) $(SPSLIB) -lm -o distort

$(DISTORT_OBJS): $(SINC)/esps.h distort.h ana.h spec.h

clean:
	rm -f $(DISTORT_OBJS) distort

lint:	$(DISTORT_SRCS)
	$(LINT) $(LINTFLAGS) $(DISTORT_SRCS) $(LINTLIB) -lm > lint

install: distort
	-mv -f $(BINDIR)/distort $(OLDBIN)/distort
	-chmod $(PROGMOD) $(OLDBIN)/distort
	mv distort $(BINDIR)/distort
	-$(STRIP) $(BINDIR)/distort
	chmod $(PROGMOD) $(BINDIR)/distort
	-diffmk  $(MANDIR)/man1/distort.1t distort.1t man.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/distort.1t
	cp distort.1t $(MANDIR)/man1/distort.1t
	chmod $(MANMOD) $(MANDIR)/man1/distort.1t

test: distort
	-sh testfile
