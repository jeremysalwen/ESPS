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
#  makefile for range
#  @(#)makefile	3.2  3/18/88  ESI
 	
CFLAGS = -O $(PROGCFLAGS) 

MAN = $(MANDIR)/man1
OBJS = range.o
SRCS = range.c
PROGNAME = range
MANNAME = range.1


$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc $(CFLAGS) $(OBJS) $(SPSLIB) -lwindow -lgp -o $(PROGNAME)

$(OBJS):	$(SINC)/param.h $(SINC)/unix.h

install:	$(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk  $(MAN)/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm $(MAN)/$(MANNAME)
	cp $(MANNAME) $(MAN)/$(MANNAME)
	chmod $(MANMOD) $(MAN)/$(MANNAME)

clean:	
	-rm -f $(OBJS) $(PROGNAME)

lint:		$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:		$(PROGNAME)
	-sh testfile
