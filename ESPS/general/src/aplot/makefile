#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing
#  by Entropic Speech, Inc. must bear the notice			
# 								
#   "Copyright (c) 1987, 1988 Entropic Speech, Inc. All rights reserved."
# 				
#
# makefile for aplot @(#)makefile	3.2 6/8/88 ESI

CFLAGS = -O $(PROGCFLAGS) 

OBJS = aplot.o 
SRCS = aplot.c 
PROGNAME = aplot
MANNAME = aplot.1


$(PROGNAME):	$(SPSLIB) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

aplot.o:	$(SINC)/esps.h $(SINC)/unix.h

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
	@echo "Aplot tested by test of cover script."
