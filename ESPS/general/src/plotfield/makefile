
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for plotfield @(#)makefile	1.1 12/8/87 ESI



install: $(BINDIR)/plotfield

$(BINDIR)/plotfield: plotfield
	rm -f $(BINDIR)/plotfield
	cp plotfield $(BINDIR)/plotfield
	chmod $(PROGMOD) $(BINDIR)/plotfield
	-diffmk $(MANDIR)/man1/plotfield.1 plotfield.1 plotfield.diff
	@echo diffmked man page left in plotfield.diff
	-rm $(MANDIR)/man1/plotfield.1
	cp plotfield.1 $(MANDIR)/man1/plotfield.1
	chmod $(MANMOD) $(MANDIR)/man1/plotfield.1

plotfield: plotfield.sh
	rm -f plotfield
	cp plotfield.sh plotfield
	chmod $(PROGMOD) plotfield

clean:
	-rm -f plotfield rc.fana

lint:
	@echo Nothing here to lint

test:	plotfield
	-sh testfile
