
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
# @(#)Makefile	1.6	07 Apr 1993	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for:  xacf
# 

XVINC  = -I$(XVIEW_INC) 
CFLAGS = -DXACFPARAMFILE=FIND_ESPS_LIB\(NULL,\"Pxacf\"\)  $(PROGCFLAGS) -g $(XVINC)

#XVLIB = $(XVIEW_LIB)
#for static libraries: 
LIBS =  $(XVIEW_B_OPT) $(XVIEW_LIB) 

LINTFLAGS = -I/home/openwin/include

OBJS = xacf.o 
SRCS = xacf.c 

PROGNAME = xacf
MANNAME = xacf.1
DEFPARAM = Pxacf

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc $(CFLAGS) $(OBJS)   $(LIBS) $(SPSLIB) $(BSDLIB) -lm -o $(PROGNAME)

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

	-rm -f $(PARAMDIR)/$(DEFPARAM)
	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)

clean:
	-rm -f $(OBJS) $(PROGNAME) sgram.params

lint:   $(SRCS)
	$(LINT) $(LINTFLAGS) $(XVINC) $(SRCS) $(LINTLIB) > lint

test:   $(PROGNAME)
	-csh $(PROGNAME).t






