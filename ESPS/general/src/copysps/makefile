#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing
#  by Entropic Speech, Inc. must bear the notice
#
# "Copyright (c) 1987, 1990 Entropic Speech, Inc. All rights reserved."
#
#
# makefile for copysps @(#)makefile	3.6	11/11/94	 ESI

CFLAGS = $(PROGCFLAGS)

SRCS = copysps.c sps_compat.c 
OBJS = copysps.o sps_compat.o 

copysps:    $(OBJS) $(WSPSLIB)
	    cc $(CFLAGS) $(OBJS) $(WSPSLIB) -lm -o copysps

copysps.o: $(SINC)/header.h $(SINC)/esps.h 
sps_compat.o: $(SINC)/header.h $(SINC)/esps.h 

clean:
	rm -f *.o copysps

saber_src: $(SRCS)
	#load $(CFLAGS) $(SPSLIB) $(XVLIB) -lm $(SRCS)


install:	$(BINDIR)/copysps $(MANDIR)/man1/copysps.1

lint:   $(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > copysps.lint

$(BINDIR)/copysps: copysps
	-mv -f $(BINDIR)/copysps $(OLDBIN)/copysps
	-chmod $(PROGMOD) $(OLDBIN)/copysps
	mv copysps $(BINDIR)/copysps
	-strip $(BINDIR)/copysps
	chmod $(PROGMOD) $(BINDIR)/copysps

$(MANDIR)/man1/copysps.1: copysps.1
	-diffmk  $(MANDIR)/man1/copysps.1 copysps.1 man.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/copysps.1
	cp copysps.1 $(MANDIR)/man1/copysps.1
	chmod $(MANMOD) $(MANDIR)/man1/copysps.1

