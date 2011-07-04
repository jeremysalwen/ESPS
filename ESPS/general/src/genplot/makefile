#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice
#
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
#
#
# makefile for genplot @(#)makefile	3.4	11/2/87	ESI

CFLAGS = $(PROGCFLAGS)

OBJS =	genplot.o init.o chrs.o draw.o gpsplot.o tekplot.o fearangesw.o
SRCS =	genplot.c init.c chrs.c draw.c gpsplot.c tekplot.c fearangesw.c

genplot:	$(OBJS) $(SPSLIB)
	cc $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o genplot

genplot.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/sd.h $(SINC)/fea.h genplot.h
init.o:		$(SINC)/esps.h $(SINC)/unix.h
draw.o:		$(SINC)/esps.h $(SINC)/unix.h genplot.h
gpsplot.o:	$(SINC)/esps.h $(SINC)/unix.h
fearangesw.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/fea.h

chrs.c:		../mlplot/chrs.c
	rm -f chrs.c
	ln ../mlplot/chrs.c .

gpsplot.c:	../mlplot/gpsplot.c
	rm -f gpsplot.c
	ln ../mlplot/gpsplot.c .

tekplot.c:	../mlplot/tekplot.c
	rm -f tekplot.c
	ln ../mlplot/tekplot.c .

init.c:		../mlplot/init.c
	rm -f init.c
	ln ../mlplot/init.c .

install:	genplot
	$(STRIP) genplot
	mv -f genplot $(PLOTDIR)/genplot
	chmod $(PROGMOD) $(PLOTDIR)/genplot

clean:	
	rm -f $(OBJS) genplot

lint:		$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > genplot.lint

test:
	@echo "Genplot tested by test of igenplot."

shar:
	shar -a $(SRCS) makefile >genplot.shar

tar:
	tar cvf genplot.tar $(SRCS) makefile
