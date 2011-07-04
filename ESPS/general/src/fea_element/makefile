
# 
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for gen_element and fea_element @(#)makefile	3.4 11/11/94 ESI
 	
CFLAGS = -g $(PROGCFLAGS)

OBJS = fea_element.o gen_element.o

PROGS = fea_element gen_element

all:		$(PROGS)

install:	$(BINDIR)/fea_element $(BINDIR)/gen_element

fea_element:	fea_element.o $(WSPSLIB)
		cc $(CFLAGS) fea_element.o $(WSPSLIB) -lm -o fea_element

$(BINDIR)/fea_element: 	fea_element
	-$(STRIP) fea_element
	-mv -f $(BINDIR)/fea_element $(OLDBIN)/fea_element
	-chmod $(PROGMOD) $(OLDBIN)/fea_element
	mv  fea_element $(BINDIR)
	chmod $(PROGMOD) $(BINDIR)/fea_element
	-rm -f $(MANDIR)/man1/feaelemen.1t
	cp feaelemen.1t $(MANDIR)/man1
	chmod $(MANMOD) $(MANDIR)/man1/feaelemen.1t

gen_element:	gen_element.o $(WSPSLIB)
		cc $(CFLAGS) gen_element.o $(WSPSLIB) -lm -o gen_element

$(BINDIR)/gen_element: 	gen_element
	-$(STRIP) gen_element
	-mv -f  $(BINDIR)/gen_element $(OLDBIN)/gen_element
	-chmod $(PROGMOD) $(OLDBIN)/gen_element
	mv -f  gen_element $(BINDIR)
	chmod $(PROGMOD) $(BINDIR)/gen_element
	-rm -f $(MANDIR)/man1/genelemen.1t
	cp genelemen.1t $(MANDIR)/man1
	chmod $(MANMOD) $(MANDIR)/man1/genelemen.1t

clean:
		rm -f $(OBJS) $(PROGS) f.lint g.lint

test:	$(PROGS)
	-sh testfile
	
lint:	f.lint g.lint

f.lint: fea_element.c
	$(LINT) $(LINTFLAGS) fea_element.c $(LINTLIB) > f.lint

g.lint: gen_element.c
	$(LINT) $(LINTFLAGS) gen_element.c $(LINTLIB) > g.lint
