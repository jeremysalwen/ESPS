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
# %W%	%G%	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for: gpstohp
# 

CFLAGS = $(PROGCFLAGS) -g

OBJS = mcd.o text.o chrs.o imdevhp.o
SRCS = mcd.c text.c chrs.c imdevhp.c
PROGNAME = gpstohp
MANNAME = 


$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB)  -lm -o $(PROGNAME)

$(OBJS):	$(SINC)/esps.h 

install:	$(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)

clean:	
	-rm -f $(OBJS) $(PROGNAME) tstmcd.o tstmcd
	
lint:		mcd.lint tstmcd.lint

mcd.lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB)  > $(PROGNAME).lint

tstmcd.lint:	tstmcd.c
	$(LINT) $(LINTFLAGS) tstmcd.c $(LINTLIB) > tstmcd.lint

test:		$(PROGNAME) tstmcd
	sh $(PROGNAME).test
