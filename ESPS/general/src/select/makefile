# This material contains unpublished, proprietary software of
# Entropic Research Laboratory, Inc. Any reproduction, distribution,
# or publication of this work must be authorized in writing by Entropic
# Research Laboratory, Inc., and must bear the notice:
#
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc.
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended
# publication of this source code.
#
# @(#)makefile	3.10 07 Apr 1993 ERL
#
# Makefile for: select
#
# Written by: Alan Parker
# Checked by:
# Revised by: Ken Nelson

CFLAGS = $(PROGCFLAGS) -g

YFLAGS = -d

OBJS = parser.o lexan.o comm.o select.o io.o version.o interp.o math.o
CSRCS =  comm.c select.c io.c version.c interp.c math.c

select: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS)  $(SPSLIB)  -lm $(SOLARIS_LIBW) -o select

comm.o:   select.h message.h unix.h
parser.o: interp.h unix.h
interp.o: interp.h $(SINC)/esps.h $(SINC)/fea.h message.h unix.h
io.o:     select.h message.h unix.h
math.o:   message.h unix.h
select.o: select.h message.h unix.h

vgrind:
	vgrind $(CSRCS) parser.y lexan.l


lint:  $(CSRCS) parser.c lexan.c select.h interp.h message.h unix.h
	$(LINT) $(LINTFLAGS) $(CSRCS) parser.c lexan.c $(LINTLIB) -lm > lint

install: select
	-mv -f $(BINDIR)/select $(OLDBIN)/select
	-chmod $(PROGMOD) $(OLDBIN)/select
	-rm -f $(BINDIR)/select
	-rm -f $(BINDIR)/eselect
	mv select $(BINDIR)/select
	-strip $(BINDIR)/select
	chmod $(PROGMOD) $(BINDIR)/select
	ln $(BINDIR)/select $(BINDIR)/eselect
	-rm -f $(MANDIR)/man1/select.1t
	-rm -f $(MANDIR)/man1/eselect.1t
	cp select.1t $(MANDIR)/man1/select.1t
	chmod $(MANMOD) $(MANDIR)/man1/select.1t
	ln $(MANDIR)/man1/select.1t $(MANDIR)/man1/eselect.1t
	rm -f $(LIBDIR)/select.hlp
	cp select.hlp $(LIBDIR)/select.hlp
	chmod $(MANMOD) $(LIBDIR)/select.hlp

clean:
	rm -f parser.c lexan.c *.o select lint y.tab.h y.output

test: select
	-select < testfile
