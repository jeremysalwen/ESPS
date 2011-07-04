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
#  makefile for scatplot
#  @(#)makefile	3.2  9/29/93  ESI
 	
CFLAGS = -O $(PROGCFLAGS)

PROGNAME = scatplot
OBJS = scatplot.o plotletter.o draw_box2.o plotscale.o printtime.o plotexscal.o
SRCS = scatplot.c plotletter.c draw_box2.c plotscale.c printtime.c plotexscal.c


$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

scatplot.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/sd.h
draw_box2.o:	$(SINC)/esps.h $(SINC)/unix.h

printtime.c:	../plotsd/printtime.c
	rm -f printtime.c
	cp ../plotsd/printtime.c .

plotscale.c:	../plotsd/plotscale.c
	rm -f plotscale.c
	cp ../plotsd/plotscale.c .

plotexscal.c:	../plotsd/plotexscal.c
	rm -f plotexscal.c
	cp ../plotsd/plotexscal.c .

install:	$(PROGNAME)
	$(STRIP) $(PROGNAME)
	-rm -f $(PLOTDIR)/$(PROGNAME)
	mv $(PROGNAME) $(PLOTDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(PLOTDIR)/$(PROGNAME)

clean:
	rm -f $(OBJS) $(PROGNAME)

lint:		$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > lint

test:
	@echo "Scatplot tested by test of cover script."
