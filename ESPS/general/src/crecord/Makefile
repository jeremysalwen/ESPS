#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#  makefile for crecord %W% %G% ESI
 	

CFLAGS = $(PROGCFLAGS) -DCLKDEV=\"$(CLKDEV)\" -DADDEV=\"$(ADDEV)\" -DADTYPE=\"$(ADTYPE)\" -DMAXAD=$(MAXAD) -DMINAD=$(MINAD)

OBJS = crecord.o
SRCS = crecord.c
PROGNAME = crecord
MANNAME = crecord.1t


$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB)  -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h 

install: $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	-rm -f $(BINDIR)/record
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	-strip $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	ln $(BINDIR)/$(PROGNAME) $(BINDIR)/record
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm -f $(MANDIR)/man1/$(MANNAME)
	-rm -f $(MANDIR)/man1/record.1t
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	ln $(MANDIR)/man1/$(MANNAME) $(MANDIR)/man1/record.1t
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

clean:	
	-rm -f $(OBJS) $(PROGNAME) lint *.diff
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) ../mcplay/mrlintcheck.c > lint

test:	$(PROGNAME)
	sh testfile
	
