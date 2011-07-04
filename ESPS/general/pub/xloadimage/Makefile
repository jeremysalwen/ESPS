# @(#)Makefile.std	1.2  10/14/90
# Makefile for xloadimage using standard C compiler
#
# Copyright 1989, 1990 Jim Frost
#
# See file "copyright.h" for complete copyright information.

# Modifications and additional program next_slide to allow advancing to
# next image under external program control.
# -- Rod Johnson, Entropic Research Laboratory, Inc.  Sept. 1990.

SYSPATHFILE=$(LIBDIR)/xloadimage
INSTALLDIR=$(BINDIR)
CC= cc
CFLAGS= -O -DSYSPATHFILE=\"$(SYSPATHFILE)\" $(PROGCFLAGS)
CP= cp
LN= ln -s
RM= rm -f
LIBS= $(WINLIBS)

SRCS= bright.c clip.c compress.c dither.c faces.c fill.c g3.c gif.c \
      halftone.c imagetypes.c merge.c misc.c new.c options.c path.c \
      pbm.c \
      ready.c \
      reduce.c root.c send.c sunraster.c value.c window.c \
      xbitmap.c xloadimage.c xpixmap.c zio.c zoom.c 

OBJS= bright.o clip.o compress.o dither.o faces.o fill.o g3.o gif.o \
      halftone.o imagetypes.o merge.o misc.o new.o options.o path.o \
      pbm.o \
      ready.o \
      reduce.o root.o send.o sunraster.o value.o window.o \
      xbitmap.o xloadimage.o xpixmap.o zio.o zoom.o

NXSRCS= next_slide.c ready.c

NXOBJS= next_slide.o ready.o

xloadimage: $(OBJS)
	$(CC) $(CFLAGS) -o xloadimage $(OBJS) $(LIBS)

next_slide: $(NXOBJS)
	$(CC) $(CFLAGS) -o next_slide $(NXOBJS) $(LIBS)

install: xloadimage next_slide $(SYSPATHFILE)
	$(RM) $(INSTALLDIR)/xloadimage
	$(RM) $(INSTALLDIR)/xsetbg
	$(RM) $(INSTALLDIR)/xview
	$(CP) xloadimage $(INSTALLDIR)/xloadimage
	$(LN) $(INSTALLDIR)/xloadimage $(INSTALLDIR)/xsetbg
	$(LN) $(INSTALLDIR)/xloadimage $(INSTALLDIR)/xview
	$(RM) $(INSTALLDIR)/next_slide
	$(CP) next_slide $(INSTALLDIR)/next_slide

clean:
	rm -f *.o *~ xloadimage next_slide

.c.o: xloadimage.h
	$(CC) -c $(CFLAGS) $*.c

$(SYSPATHFILE):
	@echo "*** Creating default $(SYSPATHFILE) since you"
	@echo "*** don't have one. This file is used to set up default places"
	@echo "*** and names to look for images.  You probably want to edit"
	@echo "*** it for your site.  See the xloadimage manual page for"
	@echo "*** details on the contents of this file."
	cp /dev/null $(SYSPATHFILE)
	echo "path= /usr/local/images" \
		>> $(SYSPATHFILE)
	echo "extension=.csun .msun .sun .face .xbm .bm .gif" \
		>>$(SYSPATHFILE)

# these targets are for those of us who have Saber-C

# load all objects in saber.  useful if you then swap the portions to debug
# for source.

objinsaber:
	#setopt ccargs -g -DSYSPATHFILE=\\\"$(SYSPATHFILE)\\\"
	#load $(OBJS) $(LIBS)
	#link

# load all sources in saber.  not as useful as the last one.

srcinsaber:
	#setopt load_flags -DSYSPATHFILE=\\\"$(SYSPATHFILE)\\\"
	#load $(SRCS) $(LIBS)
	#link
