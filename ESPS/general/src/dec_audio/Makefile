#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1996 Entropic Research Lab., Inc.; All rights reserved"
# 				
#  makefile for decrecord and decplay %W% %G%
 	
install: decrecord decplay
	mv -f decrecord $(BINDIR)/decrecord
	chmod $(PROGMOD) $(BINDIR)/decrecord
	-rm -f $(MANDIR)/man1/decrecord.1
	cp decrecord.1 $(MANDIR)/man1/decrecord.1
	chmod $(MANMOD) $(MANDIR)/man1/decrecord.1
	mv -f decplay $(BINDIR)/decplay
	chmod $(PROGMOD) $(BINDIR)/decplay
	-rm -f $(MANDIR)/man1/decplay.1
	cp decplay.1 $(MANDIR)/man1/decplay.1
	chmod $(MANMOD) $(MANDIR)/man1/decplay.1
	-rm -f $(BINDIR)/eplay
	ln -s $(BINDIR)/decplay $(BINDIR)/eplay

decrecord:	decrecord.sh
	rm -f decrecord
	cp decrecord.sh decrecord
	chmod $(PROGMOD) decrecord

decplay:	decplay.sh
	rm -f decplay
	cp decplay.sh decplay
	chmod $(PROGMOD) decplay

clean:	
	-rm -f decplay decrecord
	



