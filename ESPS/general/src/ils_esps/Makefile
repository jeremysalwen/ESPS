#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for ils_esps @(#)Makefile	3.4 3/8/91 ESI
 	
#
CFLAGS =  -O $(PROGCFLAGS)

#
OBJS = ils_esps.o

SRCS = ils_esps.c

#
PROGNAME =  ils_esps

$(PROGNAME): $(WSPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(WSPSLIB) -lm -o $(PROGNAME)

#
# list .o file dependecies here, $(SINC) is the parent of the
# esps include directories  (remove these two lines)
# eg.
ils_esps.o:    $(SINC)/esps.h $(SINC)/sd.h

install: $(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	cp  $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(PROGNAME).1 $(PROGNAME).1 man.diff
	@echo diffmked man page left in man.diff
	cp ils_esps.1 $(MANDIR)/man1/$(PROGNAME).1
	chmod $(MANMOD) $(MANDIR)/man1/$(PROGNAME).1

clean:	
	-rm -f $(PROGNAME) $(OBJS)
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > lint

test:	$(PROGNAME)
	-sh ils_esps.test
