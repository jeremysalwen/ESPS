
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for make_sd @(#)makefile	1.1 1/26/88 ESI



install: $(BINDIR)/make_sd

$(BINDIR)/make_sd: make_sd
	rm -f $(BINDIR)/make_sd
	cp make_sd $(BINDIR)/make_sd
	chmod $(PROGMOD) $(BINDIR)/make_sd
	-diffmk $(MANDIR)/man1/make_sd.1 make_sd.1 make_sd.diff
	@echo diffmked man page left in make_sd.diff
	-rm $(MANDIR)/man1/make_sd.1
	cp make_sd.1 $(MANDIR)/man1/make_sd.1
	chmod $(MANMOD) $(MANDIR)/man1/make_sd.1

make_sd: make_sd.sh
	rm -f make_sd
	cp make_sd.sh make_sd
	chmod $(PROGMOD) make_sd

clean:
	-rm -f make_sd rc.fana test.sd

lint:
	@echo Nothing here to lint

test:	make_sd
	-sh testfile
