# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	3.6	6/4/92	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for: btops

CFLAGS = $(PROGCFLAGS) -g

OBJS = btosps.o
SRCS = btosps.c
PROGNAME = btosps
MANNAME = btosps.1
DEFPARM = Pbtosps


$(PROGNAME): $(SSPSLIB) $(OBJS)
	gcc  $(CFLAGS) $(OBJS)  $(SSPSLIB) -lm -o $(PROGNAME)

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
	-rm -f $(PARAMDIR)/$(DEFPARM)
	cp $(DEFPARM) $(PARAMDIR)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARM)

clean:	
	-rm -f $(OBJS) $(PROGNAME)
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	sh testfile
