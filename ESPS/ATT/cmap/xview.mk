#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1991 Entropic Research Lab.
# 				
#  makefile for xcmap (XView version) @(#)xview.mk	1.5 1/7/93 ERL
 	

CFLAGS = -I$(XVIEW_INC) $(PROGCFLAGS)
SUNLIB = $(XVIEW_B_OPT) $(XVIEW_LIB) 

xcmap: xcmap.o
	rm -f $@
	cc -o xcmap xcmap.o $(FLOAT) $(SUNLIB) $(SPSLIB) -lm

install: $(BINDIR)/xcmap $(MANDIR)/man1/cmap.1

$(BINDIR)/xcmap: xcmap
	-strip $?
	-rm -f $(BINDIR)/$?
	cp $? $(BINDIR)/$?
	chmod $(PROGMOD) $(BINDIR)/$?

$(MANDIR)/man1/cmap.1: cmap.1
	-rm -f $(MANDIR)/man1/cmap.1
	-rm -f $(MANDIR)/man1/xcmap.1
	cp cmap.1 $(MANDIR)/man1
	chmod $(MANMOD) $(MANDIR)/man1/cmap.1
	(cd $(MANDIR)/man1 ; ln -s cmap.1 xcmap.1)


clean:
	rm -f *.o xcmap
