#
#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice
#
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
#
#
# makefile for fea_stats @(#)makefile	1.5  6/1/88  ESI

CFLAGS = -O $(PROGCFLAGS)

OBJS =	fea_stats.o stat.o non_stat.o pr_non_sta.o
SRCS =	fea_stats.c stat.c non_stat.c pr_non_sta.c
LOC_INC = global.h stat.h non_stat.h

fea_stats:	$(OBJS) $(SPSLIB) 
	cc $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o fea_stats

fea_stats.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/spsassert.h \
			$(SINC)/fea.h $(SINC)/feastat.h $(LOC_INC)
stat.o:		$(SINC)/esps.h $(SINC)/unix.h $(SINC)/spsassert.h \
			$(SINC)/fea.h $(SINC)/feastat.h $(LOC_INC)
non_stat.o:	$(SINC)/esps.h $(SINC)/unix.h \
			$(SINC)/fea.h $(SINC)/feastat.h $(LOC_INC)
pr_non_sta.o:	$(SINC)/esps.h $(SINC)/fea.h $(SINC)/feastat.h $(LOC_INC)

install:	fea_stats
	$(STRIP) fea_stats
	-mv -f $(BINDIR)/fea_stats $(OLDBIN)/fea_stats
	-chmod $(PROGMOD) $(OLDBIN)/fea_stats
	mv -f fea_stats $(BINDIR)/fea_stats
	chmod $(PROGMOD) $(BINDIR)/fea_stats
	-diffmk $(MANDIR)/man1/fea_stats.1t fea_stats.1t man.diff
	@echo diffmked man page left in man.diff
	-rm $(MANDIR)/man1/fea_stats.1t
	cp fea_stats.1t $(MANDIR)/man1/fea_stats.1t
	chmod $(MANMOD) $(MANDIR)/man1/fea_stats.1t

lint:		$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > lint

clean:
	rm -f $(OBJS) fea_stats

shar:
	shar -a $(SRCS) $(LOC_INC) makefile fea_stats.1t check >fea_stats.shar

tar:
	tar cvf fea_stats.tar $(SRCS) $(LOC_INC) makefile fea_stats.1t check

test:		fea_stats
	sh check
