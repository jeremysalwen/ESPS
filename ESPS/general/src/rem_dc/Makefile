#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for rem_dc %W% %G% ESI
#
#
CFLAGS =  -O $(PROGCFLAGS)

#
OBJS = rem_dc.o

SRCS = rem_dc.c

PROGNAME =  rem_dc

$(PROGNAME): $(WSPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(WSPSLIB) -lm -o $(PROGNAME)

#

rem_dc.o:    $(SINC)/esps.h $(SINC)/sd.h

install: $(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f  $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(PROGNAME).1 $(PROGNAME).1 man.diff
	@echo diffmked man page left in man.diff
	cp rem_dc.1 $(MANDIR)/man1/$(PROGNAME).1
	chmod $(MANMOD) $(MANDIR)/man1/$(PROGNAME).1

clean:	
	-rm -f $(PROGNAME) $(OBJS)
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm  > lint

test:	$(PROGNAME)
	-sh testfile
