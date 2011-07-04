
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for utils %W%  %G%  ESI
 	
CFLAGS = $(PROGCFLAGS)

install: $(BINDIR)/newer $(BINDIR)/makewhatis $(LIBDIR)/stou \
	$(BINDIR)/man_name $(BINDIR)/emakecats

lint:		stou.lint newer.lint man_name.lint

$(BINDIR)/makewhatis: makewhatis
	-rm -f $(BINDIR)/makewhatis
	cp makewhatis $(BINDIR)
	chmod $(PROGMOD) $(BINDIR)/makewhatis
	rm -f $(MANDIR)/man1/makewhatis.1
	cp makewhatis.1 $(MANDIR)/man1
	chmod $(MANMOD) $(MANDIR)/man1/makewhatis.1

$(BINDIR)/newer: newer
	$(STRIP) newer
	-rm -f $(BINDIR)/newer
	mv -f newer $(BINDIR)/newer
	chmod $(PROGMOD) $(BINDIR)/newer

stou:		$(SPSLIB) stou.o
	cc $(CFLAGS) stou.o $(SPSLIB) -o stou

man_name:	man_name.o
	cc $(CFLAGS) man_name.o -o man_name

newer:	newer.o
	cc $(CFLAGS) newer.o -o newer

$(LIBDIR)/stou: stou
	$(STRIP) stou
	-rm -f $(LIBDIR)/stou
	mv -f stou $(LIBDIR)/stou
	chmod $(PROGMOD) $(LIBDIR)/stou

$(BINDIR)/man_name: man_name
	$(STRIP) man_name
	mv -f man_name $(BINDIR)/man_name
	chmod $(PROGMOD) $(BINDIR)/man_name
	rm -f $(MANDIR)/man1/manname.1
	cp manname.1 $(MANDIR)/man1
	chmod $(MANMOD) $(MANDIR)/man1/manname.1

$(BINDIR)/emakecats: emakecats
	rm -f $(BINDIR)/emakecats
	cp emakecats $(BINDIR)/emakecats
	chmod $(PROGMOD) $(BINDIR)/emakecats
	rm -f $(MANDIR)/man1/emakecats.1
	cp emakecats.1 $(MANDIR)/man1
	chmod $(MANMOD) $(MANDIR)/man1/emakecats.1
	

clean:
	rm -f *.o newer stou man_name *.lint

stou.lint:	stou.c
	$(LINT) $(LINTFLAGS) stou.c $(LINTLIB) > stou.lint

newer.lint:	newer.c
	$(LINT) $(LINTFLAGS) newer.c > newer.lint

man_name.lint:	man_name.c
	$(LINT) $(LINTFLAGS) man_name.c > man_name.lint
	

test:
	@echo "Stou tested by test of cover scripts for plot programs."
