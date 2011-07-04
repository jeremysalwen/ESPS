
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
# makefile for fea_edit @(#)makefile	3.5 11/11/94 ESI
 	
CFLAGS =   -O $(PROGCFLAGS)

fea_edit:	fea_edit.o $(SPSLIB)
	$(CC) $(CFLAGS) fea_edit.o  $(SPSLIB) -lm -o fea_edit

fea_edit.o: $(SINC)/header.h $(SINC)/esps.h  $(SINC)/fea.h

clean:
	-rm -f *.o fea_edit test.fana

install:	$(BINDIR)/fea_edit $(MANDIR)/man1/fea_edit.1

lint:   fea_edit.c 
	$(LINT)  $(LINTFLAGS) fea_edit.c  $(LINTLIB) > lint

$(BINDIR)/fea_edit: fea_edit
	$(STRIP) fea_edit
	-mv -f  $(BINDIR)/fea_edit $(OLDBIN)/fea_edit
	-chmod $(PROGMOD) $(OLDBIN)/fea_edit
	mv  fea_edit $(BINDIR)
	chmod $(PROGMOD) $(BINDIR)/fea_edit

$(MANDIR)/man1/fea_edit.1: fea_edit.1
	-rm -f $(MANDIR)/man1/fea_edit.1
	cp fea_edit.1 $(MANDIR)/man1
	chmod $(MANMOD) $(MANDIR)/man1/fea_edit.1

man:
	itroff -man fea_edit.1

test:	fea_edit
	-sh testfile
