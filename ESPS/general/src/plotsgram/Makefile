
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
# @(#)Makefile	1.12 6/12/91 ERL
#
# Makefile for: spgm
#
# Written by: Alan Parker
# Checked by:
# Revised by: Ken Nelson

# remove comments for testing only.  Put comments back before sccs'ing.
# SPOOL_CMD = IPR
# PROGCFLAGS = -I/usr/esps3/include
# BINDIR = /tmp
# LIBDIR = /tmp
# MANDIR = /tmp
# PROGMOD = 0755
# MANMOD = 0755
# SPSLIB = /u2/esps3/lib/libespsg.a

plotsgram : plotsgram.sh
	sed 's;\$$SPOOL_CMD\$$;$(SPOOL_CMD);' plotsgram.sh > plotsgram
	chmod $(PROGMOD) plotsgram

install: $(BINDIR)/plotsgram 


$(BINDIR)/sgram: sgram.sh
	rm -f $(BINDIR)/sgram
	chmod $(PROGMOD) $(BINDIR)/sgram
	-diffmk $(MANDIR)/man1/sgram.1 sgram.1 sgram.diff
	@echo diffmked man page left in sgram.diff
	-rm $(MANDIR)/man1/sgram.1
	cp sgram.1 $(MANDIR)/man1/sgram.1
	chmod $(MANMOD) $(MANDIR)/man1/sgram.1

$(BINDIR)/plotsgram: plotsgram
	rm -f $(BINDIR)/plotsgram
	cp plotsgram $(BINDIR)/plotsgram
	chmod $(PROGMOD) $(BINDIR)/plotsgram
	-diffmk $(MANDIR)/man1/plotsgram.1 plotsgram.1 plotsgram.diff
	@echo diffmked man page left in plotsgram.diff
	-rm $(MANDIR)/man1/plotsgram.1
	cp plotsgram.1 $(MANDIR)/man1/plotsgram.1
	chmod $(MANMOD) $(MANDIR)/man1/plotsgram.1


$(LIBDIR)/get_sf: $(SPSLIB) get_sf.o
	$(CC)  $(PROGCFLAGS)  get_sf.o $(SPSLIB) -lm -o get_sf
	-strip get_sf
	mv -f get_sf $(LIBDIR)/get_sf
	chmod $(PROGMOD) $(LIBDIR)/get_sf

get_sf.o: get_sf.c
	$(CC) -c $(PROGCFLAGS) get_sf.c
	

clean:	
	-rm -f get_sf.o get_sf plotsgram
	
test:	plotsgram
	-sh sgram.test  $(BINDIR)

