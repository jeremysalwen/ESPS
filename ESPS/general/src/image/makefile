# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)makefile	1.13	9/9/91	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for:  image
# 


# Don't use -O
CFLAGS = $(OPT_FLAG) $(WINDOWS) $(PROGCFLAGS)
LFLAGS = $(WINDOWS) $(LINTFLAGS)
IMAGE_COLOR = $(LIBDIR)/colormap

OBJS = image.o imagedot.o imagedev.o \
	chrs.o text.o imagedraw.o imagescale.o \
	imdevmcd.o imdevsun.o imdevnull.o imdevtyp.o imdevimp.o imdevras.o \
	imdevx11.o make_icon.o imdevps.o \
	imdevhp.o

SRCS = image.c imagedot.c imagedev.c \
	chrs.c text.c imagedraw.c imagescale.c \
	imdevmcd.c imdevsun.c imdevnull.c imdevtyp.c imdevimp.c imdevras.c \
	imdevx11.c make_icon.c imdevps.c \
	imdevhp.c

PROGNAME = image
MANNAME = image.1
PARAM = Pimage

CMAPS = CMap0 CMap1 CMap2 CMap3 TMap0 TMap1 TMap2 TMap3 YBMap


$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc $(XVIEW_B_OPT) $(CFLAGS) $(OBJS) $(WINLIBS) $(SPSLIB)  -lm -o $(PROGNAME)

image.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/fea.h $(SINC)/spec.h \
		image.h
imagedot.o:	$(SINC)/esps.h $(SINC)/unix.h image.h
imagedev.o:	$(SINC)/esps.h $(SINC)/unix.h image.h
text.o:		$(SINC)/esps.h $(SINC)/unix.h
imagedraw.o:	$(SINC)/esps.h $(SINC)/unix.h image.h
imagescale.o:	$(SINC)/esps.h $(SINC)/unix.h image.h
imdevimp.o:	$(SINC)/esps.h $(SINC)/unix.h image.h
imdevmcd.o:	$(SINC)/esps.h $(SINC)/unix.h image.h
imdevnull.o:	$(SINC)/esps.h
imdevras.o:	$(SINC)/esps.h $(SINC)/unix.h
imdevsun.o:	$(SINC)/esps.h $(SINC)/unix.h image.h
imdevtyp.o:	$(SINC)/esps.h $(SINC)/unix.h image.h
imdevx11.o:	$(SINC)/esps.h $(SINC)/unix.h image.h
make_icon.o:	sine.icon spec.icon image.icon epi.icon hist.icon
imdevps.o:	$(SINC)/esps.h $(SINC)/unix.h image.h
imdevhp.o:	$(SINC)/esps.h $(SINC)/unix.h image.h

install: $(PROGNAME)
#program
	$(STRIP) $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
#man pages
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)
#param file
	-rm -f $(PARAMDIR)/$(PARAM)
	cp $(PARAM) $(PARAMDIR)/$(PARAM)
	chmod $(LIBMOD) $(PARAMDIR)/$(PARAM)
#colormaps
	-(cd $(IMAGE_COLOR) ; rm -f $(CMAPS))
	cp $(CMAPS) $(IMAGE_COLOR)
	(cd $(IMAGE_COLOR) ; chmod $(LIBMOD) $(CMAPS))

clean:	
	-rm -f $(PROGNAME) $(OBJS)
	
lint:	$(SRCS)
	$(LINT) $(LFLAGS) $(SRCS) $(LINTLIB) $(WINLIBS) -lm > lint

test:	$(PROGNAME)
	sh image.tes
