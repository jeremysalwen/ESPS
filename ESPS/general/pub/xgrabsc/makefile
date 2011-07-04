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
# @(#)makefile	1.13	9/9/91	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for:  xgrabsc
# 


# Don't use -O
CFLAGS = $(OPT_FLAG)  $(PROGCFLAGS)
LFLAGS = $(WINDOWS) $(LINTFLAGS)

OBJS = xgrabsc.o

SRCS = xgrabsc.c

PROGNAME = xgrabsc
MANNAME = xgrabsc



$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc $(XVIEW_B_OPT) $(CFLAGS) $(OBJS) $(WINLIBS) $(SPSLIB)  -lm -o $(PROGNAME)


install: $(PROGNAME)
#program
	$(STRIP) $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
#man pages
	cp $(MANNAME).man $(MANDIR)/man1/$(MANNAME).1
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME).1

clean:	
	-rm -f $(PROGNAME) $(OBJS)
	
lint:	$(SRCS)
	$(LINT) $(LFLAGS) $(SRCS) $(LINTLIB) $(WINLIBS) -lm > lint

