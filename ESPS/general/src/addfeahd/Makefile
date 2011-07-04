#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1988 Entropic Speech, Inc.; All rights reserved"
# 				
#  makefile for addfeahd @(#)Makefile	1.5 6/13/91 ESI
 	

CFLAGS = $(PROGCFLAGS) -O

OBJS = addfeahd.o
SRCS = addfeahd.c
PROGNAME = addfeahd
MANNAME = addfeahd.1t
PARAM = Paddfeahd


$(PROGNAME): $(WSPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(WSPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h 

install: $(PROGNAME)
	-$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
	-rm -f $(PARAMDIR)/$(PARAM)
	cp $(PARAM) $(PARAMDIR)/$(PARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(PARAM)

clean:	
	-rm -f $(OBJS) $(PROGNAME) 
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	-sh testfile
