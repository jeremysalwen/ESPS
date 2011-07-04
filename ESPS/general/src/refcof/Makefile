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
# Written by:  John Shore
# Checked by:
# Revised by:
# 
# Brief description: makefile for refcof
# 
 	
#CFLAGS = -O $(PROGCFLAGS) 
CFLAGS = -g $(PROGCFLAGS) 

#OBJS = refcof.o compute_rc.o struct_cov.o bestauto.o
#SRCS = refcof.c compute_rc.c struct_cov.o bestauto.o

OBJS = refcof.o 
SRCS = refcof.c 

PROGNAME = refcof
XPROGNAME = xrefcof

MANNAME = refcof.1
XMANNAME = xrefcof.1

DEFPARAM = Prefcof
DEFPARAMW = PWrefcof

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h $(SINC)/scbk.h

saber_src: $(SRCS)
	#load $(CFLAGS) $(SPSLIB) $(XVLIB) -lm $(SRCS)

install: $(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
# install the X cover
	-mv -f $(BINDIR)/$(XPROGNAME) $(OLDBIN)/$(XPROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(XPROGNAME)
	cp $(XPROGNAME) $(BINDIR)/$(XPROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(XPROGNAME)
	-rm $(MANDIR)/man1/$(XMANNAME)
	ln $(MANDIR)/man1/$(MANNAME) $(MANDIR)/man1/$(XMANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(XMANNAME)

# install the default parameter files
	-rm -f $(PARAMDIR)/$(DEFPARAM)
	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)
	-rm -f $(PARAMDIR)/$(DEFPARAMW)
	cp $(DEFPARAMW) $(PARAMDIR)/$(DEFPARAMW)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAMW)

clean:	
	-rm -f $(OBJS) $(PROGNAME) core *.auto *.rc *.sd *.fspec
	
lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	sh $(PROGNAME).test

