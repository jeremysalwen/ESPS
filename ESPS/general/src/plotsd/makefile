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
#  makefile for plotsd
#  %W%  %G%  ESI
 	
OFLAG = -g
CFLAGS = $(OFLAG) $(PROGCFLAGS)

PROGNAME = plotsd
OBJS = plotsd.o drawbox.o plotdataf.o plotscale.o printtime.o plotexscal.o
SRCS = plotsd.c drawbox.c plotdataf.c plotscale.c printtime.c plotexscal.c 


$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

plotsd.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/sd.h
drawbox.o:	$(SINC)/esps.h $(SINC)/unix.h

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
	@echo "Plotsd tested by test of cover script."
