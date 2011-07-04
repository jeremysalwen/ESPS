#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#  makefile for addsd @(#)Makefile	3.5 2/22/90 
 	

CFLAGS = $(PROGCFLAGS) -g

OBJS = addsd.o
SRCS = addsd.c
PROGNAME = addsd
PROGNAME2 = multsd
MANNAME = addsd.1
MANNAME2 = multsd.1

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h 

install: $(PROGNAME)
	-strip $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	rm -f $(BINDIR)/$(PROGNAME2)
	ln $(BINDIR)/$(PROGNAME) $(BINDIR)/$(PROGNAME2)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME2)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	rm -f $(MANDIR)/man1/$(MANNAME2)
	ln $(MANDIR)/man1/$(MANNAME) $(MANDIR)/man1/$(MANNAME2)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME2)

clean:	
	-rm -f $(OBJS) $(PROGNAME) $(PROGNAME2) *.sd
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	sh $(PROGNAME).test
