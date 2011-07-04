#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1988 Entropic Speech, Inc.; All rights reserved"
# 				
#  makefile for hditem @(#)Makefile	1.2 3/23/90 ESI
 	

CFLAGS = $(PROGCFLAGS) -g

OBJS = hditem.o
SRCS = hditem.c
PROGNAME = hditem
MANNAME = hditem.1


$(PROGNAME): $(WSPSLIB) $(OBJS)
	$(CC)  $(CFLAGS) $(OBJS) $(WSPSLIB) -lm -o $(PROGNAME)

$(OBJS): $(SINC)/esps.h $(SINC)/fea.h $(SINC)/sd.h $(SINC)/vq.h $(SINC)/spec.h $(SINC)/anafea.h $(SINC)/feastat.h

install: $(PROGNAME)
	-strip $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

clean:	
	-rm -f $(OBJS) $(PROGNAME)
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	-sh hditem.test
