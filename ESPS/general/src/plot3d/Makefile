# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# %W%	%G%	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for: plot3d
# 

CFLAGS =  -g -I$(XVIEW_INC) $(PROGCFLAGS) 

SRCS = plot3d.c p3dcoords.c p3dfiles.c p3dinter.c p3dplot.c \
	p3dtext.c chrs.c p3drobar.c p3ddata.c p3dwaves.c p3dprint.c
OBJS = plot3d.o p3dcoords.o p3dfiles.o p3dinter.o p3dplot.o \
	p3dtext.o chrs.o p3drobar.o p3ddata.o p3dwaves.o p3dprint.o
PROGNAME = plot3d
MANNAME = plot3d.1
# DEFPARAM = Pplot3d
LIBMAN = $(LIBDIR)/plot3d.man

$(PROGNAME): $(WSPSLIB) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(XVIEW_B_OPT) \
		-lXp  $(XPRINTER_GNULIB) \
		$(WSPSLIB) -lm $(LIBMV)  $(XVIEW_LIB) -o $(PROGNAME)
pure: $(WSPSLIB) $(OBJS)
	purify $(CC) $(CFLAGS) $(OBJS) $(XVIEW_B_OPT) \
		-L$(XPRINTER_LIB) -lXp  $(XPRINTER_GNULIB) \
		$(WSPSLIB) -lm $(LIBMV)  $(XVIEW_LIB) -o $(PROGNAME)


plot3d.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/fea.h \
		$(SINC)/constants.h plot3d.h
p3dcoords.o:	$(SINC)/esps.h plot3d.h
p3dfiles.o:	$(SINC)/esps.h plot3d.h
p3dinter.o:	$(SINC)/esps.h $(SINC)/constants.h plot3d.h
p3dplot.o:	$(SINC)/esps.h plot3d.h

p3drobar.o:	$(SINC)/esps.h plot3d.h
p3dtext.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/constants.h
p3ddata.o:	$(SINC)/esps.h plot3d.h $(SINC)/fea.h $(SINC)/feaspec.h
p3dprint.o:	$(SINC)/esps.h 

install: $(PROGNAME) $(LIBMAN)
	-$(STRIP) $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv -f $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm -f $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

#	-rm -f $(LIBDIR)/$(DEFPARAM)
#	cp $(DEFPARAM) $(LIBDIR)/$(DEFPARAM)
#	chmod $(LIBMOD) $(LIBDIR)/$(DEFPARAM)

$(LIBMAN): $(MANNAME)
	nroff -u0 $(LIBDIR)/tmac.an $(MANNAME) | col | ul -tadm3a | sed -e "s/_//g" -e "/Copyright c/d" >$(LIBMAN)
	
clean:	
	-rm -f $(OBJS) $(PROGNAME) lint
	
lint:	$(SRCS) 
	$(LINT) $(LINTFLAGS) -I$(XVIEW_INC) $(SRCS) $(LINTLIB) -lm > lint

# test:	$(PROGNAME)
# 	-sh $(PROGNAME).test
