
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for imlplot @(#)makefile	3.2 6/14/91 ESI
 	
PROGNAME = imlplot
MANNAME = imlplot.1

MAN = $(MANDIR)/man1


install:
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	cp $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk  $(MAN)/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man in man.diff
	-rm $(MAN)/$(MANNAME)
	cp $(MANNAME) $(MAN)/$(MANNAME)
	chmod $(MANMOD) $(MAN)/$(MANNAME)

clean:
	@echo nothing here to clean

lint:
	@echo nothing here to lint

test:
	-sh testfile
