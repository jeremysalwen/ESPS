#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#   "Copyright (c) 1990 Entropic Speech, Inc. All rights reserved."
# 				
#
#  makefile for mu2esps
#  @(#)Makefile	1.7 10/5/95 ESI
#
CFLAGS =  -O $(PROGCFLAGS) $(SUNAUDIOINC)

OBJS = mu2esps.o
SRCS = mu2esps.c
PROGNAME =  mu2esps
MANNAME = mu2esps.1


$(PROGNAME): $(WSPSLIB) $(OBJS)
	cc  $(OBJS) $(CFLAGS)  $(WSPSLIB) $(SUNAUDIOLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h $(SINC)/unix.h $(SINC)/feasd.h $(SINC)/fea.h

install: $(PROGNAME)
	$(STRIP) $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)


clean:	
	-rm -f $(PROGNAME) $(OBJS)
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > lint

test:	$(PROGNAME)
	sh testfile
