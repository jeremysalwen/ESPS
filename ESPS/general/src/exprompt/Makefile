# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1989  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.22	06 Apr 1993	ESI/ERL
# 
# Written by:  John Shore
# Checked by:
# Revised by:
# 
# Brief description: makefile for exprompt
# 

XVINC  = -I$(XVIEW_INC)
CFLAGS = $(XVINC) $(PROGCFLAGS) -g 

#XVLIB = $(XVIEW_LIB)
#for static libraries: 
XVLIB =  $(XVIEW_B_OPT) $(XVIEW_LIB) 

LINTFLAGS = -I/home/openwin/include

#OBJS = exprompt.o xpromptpar.o
#SRCS = exprompt.c xpromptpar.c

OBJS = exprompt.o 
SRCS = exprompt.c

PROGNAME = exprompt
MANNAME = exprompt.1

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS)   $(XVLIB) $(SPSLIB) $(BSDLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h

saber_src: $(SRCS)
	#load $(CFLAGS) $(SPSLIB) $(XVLIB) -lm $(SRCS)

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
	-rm -f $(OBJS) $(PROGNAME) core lint

lint:   $(SRCS)
	$(LINT) $(LINTFLAGS) $(XVINC) $(SRCS) $(LINTLIB) > lint

test:   $(PROGNAME)
	-sh $(PROGNAME).test
