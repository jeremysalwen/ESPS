#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1989 Entropic Speech, Inc.; All rights reserved"
# 				
#  makefile for getparam @(#)Makefile	1.1 3/9/90 ESI
 	

CFLAGS = $(PROGCFLAGS) -g

OBJS = getparam.o
SRCS = getparam.c
PROGNAME = getparam
MANNAME = getparam.1


$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h 

install: $(PROGNAME)
	-strip $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

clean:	
	-rm -f $(OBJS) $(PROGNAME) 
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	-sh $(PROGNAME).test

