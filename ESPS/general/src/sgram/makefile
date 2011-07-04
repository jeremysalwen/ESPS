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
# Brief description: makefile for sgram
# 

CFLAGS =   -O $(PROGCFLAGS)

PARAMDIR = $(LIBDIR)/params

OBJS = sgram.o 
SRCS = sgram.c 

PROGNAME = sgram
MANNAME = sgram.1

DEFPARAM = Psgram
DEFPARAMW = PWsgram

$(PROGNAME): $(WSPSLIB) $(OBJS)
	$(CC)  $(CFLAGS) $(OBJS) $(WSPSLIB) $(BSDLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h $(SINC)/unix.h $(SINC)/limits.h $(SINC)/sd.h \
	$(SINC)/fea.h $(SINC)/window.h $(SINC)/feaspec.h

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
	-rm -f $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

	-rm -f $(PARAMDIR)/$(DEFPARAM)
	cp $(DEFPARAM) $(PARAMDIR)/$(DEFPARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAM)
	-rm -f $(PARAMDIR)/$(DEFPARAMW)
	cp $(DEFPARAMW) $(PARAMDIR)/$(DEFPARAMW)
	chmod $(LIBMOD) $(PARAMDIR)/$(DEFPARAMW)

	-rm -rf $(SPSDIR)/src_examples/sgram
	mkdir $(SPSDIR)/src_examples/sgram
	-cp * $(SPSDIR)/src_examples/sgram
	(cd $(SPSDIR)/src_examples/sgram; emake clean)

clean:	
	-rm -f $(OBJS) $(PROGNAME)  lint
	
lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > lint

test:	$(PROGNAME)
	-sh $(PROGNAME).test
