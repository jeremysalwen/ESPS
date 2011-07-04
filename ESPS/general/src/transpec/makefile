
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
# @(#)makefile	1.4	6/13/91	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for: transpec
# 

CFLAGS =  -O $(PROGCFLAGS)

OBJS = transpec.o
SRCS = transpec.c
PROGNAME = transpec
MANNAME = transpec.1
DEFPARAM = Ptranspec


$(PROGNAME): $(SPSLIB) $(OBJS)
	$(CC)  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h $(SINC)/unix.h $(SINC)/fea.h $(SINC)/anafea.h

install: $(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm -f $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

	-rm -f $(PARAMDIR)/$(DEFPARAM)
	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)

clean:	
	-rm -f $(OBJS) $(PROGNAME) lint
	
lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > lint

test:	$(PROGNAME)
	-sh $(PROGNAME).test
