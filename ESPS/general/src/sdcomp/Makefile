# SPS makefile for sdcomp (1-ESPS)

#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for sdcomp 10/30/87 @(#)Makefile	3.3 ESI

install: $(BINDIR)/sdcomp

$(BINDIR)/sdcomp: sdcomp
	-rm -f $(BINDIR)/sdcomp
	mv sdcomp $(BINDIR)/sdcomp
	chmod $(PROGMOD) $(BINDIR)/sdcomp
	-diffmk $(MANDIR)/man1/sdcomp.1 sdcomp.1 sdcomp.diff
	@echo diffmked man page left in sdcomp.diff
	-rm $(MANDIR)/man1/sdcomp.1
	cp sdcomp.1 $(MANDIR)/man1/sdcomp.1
	chmod $(MANMOD) $(MANDIR)/man1/sdcomp.1

sdcomp: sdcomp.sh
	cp sdcomp.sh sdcomp
	chmod $(PROGMOD) sdcomp

test: sdcomp
	-sh testfile

clean:
	-rm -f sdcomp

lint:
	@echo Nothing here to lint
