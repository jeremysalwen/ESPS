#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice
#
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
#
#
# makefile for mlplot @(#)makefile	3.5	11/2/87	ESI

CFLAGS = -O $(PROGCFLAGS)

OBJS =	mlplot.o init.o chrs.o draw.o gpsplot.o tekplot.o
SRCS =	mlplot.c init.c chrs.c draw.c gpsplot.c tekplot.c

mlplot:		$(OBJS) $(SPSLIB)
	cc $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o mlplot

mlplot.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/sd.h mlplot.h
init.o:		$(SINC)/esps.h $(SINC)/unix.h
draw.o:		$(SINC)/esps.h $(SINC)/unix.h mlplot.h
gpsplot.o:	$(SINC)/esps.h $(SINC)/unix.h

install:	mlplot
	$(STRIP) mlplot
	mv -f mlplot $(PLOTDIR)/mlplot
	chmod $(PROGMOD) $(PLOTDIR)/mlplot

clean:	
	rm -f $(OBJS) mlplot

lint:		$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > mlplot.lint

test:
	@echo "Mlplot tested by test of imlplot."

shar:
	shar -a mlplot.h $(SRCS) makefile >mlplot.shar

tar:
	tar cvf mlplot.tar mlplot.h $(SRCS) makefile
