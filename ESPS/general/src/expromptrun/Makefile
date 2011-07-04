# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.6	2/4/93	ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Brief description: Makefile for expromptrun

XVINC  = -I$(XVIEW_INC)
CFLAGS = $(PROGCFLAGS) -g $(XVINC)

XVLIB =  $(XVIEW_B_OPT) $(XVIEW_LIB) 

LINTFLAGS = -I/home/openwin/include

#OBJS = expromptrun.o xpromptpar.o
#SRCS = expromptrun.c xpromptpar.c

OBJS = expromptrun.o 
SRCS = expromptrun.c 

PROGNAME = expromptrun
MANNAME = expromptru.1

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
	-rm -f $(OBJS) $(PROGNAME) test.sd core lint

lint:   $(SRCS)
	$(LINT) $(LINTFLAGS) $(XVINC) $(SRCS) $(LINTLIB) > lint

test:   $(PROGNAME)
	-sh exprun.test
