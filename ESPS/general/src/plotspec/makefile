#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#   "Copyright (c) 1987, 1988 Entropic Speech, Inc. All rights reserved."
#
#
#  makefile for plotspec
#  @(#)makefile	3.3  12/23/88  ESI
 	
CFLAGS = -O $(PROGCFLAGS)

PROGNAME = plotspec
OBJS = plotspec.o drawbox.o plot2data.o plotphdata.o \
	printtime.o plotscale.o plotexscal.o
SRCS = plotspec.c drawbox.c plot2data.c plotphdata.c \
	printtime.c plotscale.c plotexscal.c


$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

plotspec.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/spec.h $(SINC)/feaspec.h
drawbox.o:	$(SINC)/esps.h $(SINC)/unix.h

printtime.c:	../plotsd/printtime.c
	rm -f printtime.c
	ln ../plotsd/printtime.c .

plotscale.c:	../plotsd/plotscale.c
	rm -f plotscale.c
	ln ../plotsd/plotscale.c .

plotexscal.c:	../plotsd/plotexscal.c
	rm -f plotexscal.c
	ln ../plotsd/plotexscal.c .

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
	@echo "Plotspec tested by test of cover script."
