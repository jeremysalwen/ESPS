#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 
#   "Copyright (c) 1989 Entropic Speech, Inc. All rights reserved."
# 
#
#  makefile for showras
#  @(#)makefile	1.1	9/27/89	ESI

CFLAGS =  -g

OBJS = showras.o
SRCS = showras.c
PROGNAME = showras
MANNAME = showras.1

$(PROGNAME): $(OBJS)
	cc $(CFLAGS) $(OBJS) -lsuntool -lsunwindow -lpixrect -lm -o $(PROGNAME)

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

clean:	
	-rm -f $(OBJS) $(PROGNAME)  lint
	
lint:	$(SRCS) 
	$(LINT) $(SRCS) -lm > lint

#test:	$(PROGNAME)
#	-sh $(PROGNAME).test
