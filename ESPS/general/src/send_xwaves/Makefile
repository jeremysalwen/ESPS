# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1995  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)Makefile	1.5	10/2/95	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Brief description:
# 

CFLAGS =   $(PROGCFLAGS) -g 	 -I$(X_INC)

SRCS = send_xwaves.c
OBJS = send_xwaves.o
PROGNAME = send_xwaves
MANNAME = sendxwaves.1

$(PROGNAME): $(SPSLIB) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS)  $(SPSLIB) $(X_LIB)   -o $(PROGNAME)


install: $(PROGNAME) 
	$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-rm -f $(MANDIR)/man1/$(MANNAME)
	cp send_xwaves.1 $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

clean:	
	-rm -f $(OBJS) $(PROGNAME) lint 
	
lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS)  $(SRCS) $(LINTLIB) -lm > lint

# test:	$(PROGNAME)
# 	-sh $(PROGNAME).test
