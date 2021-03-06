# This material contains unpublished, proprietary software of
# Entropic Research Laboratory, Inc. Any reproduction, distribution,
# or publication of this work must be authorized in writing by Entropic
# Research Laboratory, Inc., and must bear the notice:
#
#    "Copyright (c) 1995 Entropic Research Laboratory, Inc.
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended
# publication of this source code.
#
# %W% %G% ERL
#
# Makefile for:  waves src/c directory
#
# Written by:   David Talkin and Alan Parker

GFLAG =

SUNLIB =  $(XVIEW_LIB) -lm -lXt
#CFLAGS =  -Bstatic -I../h -I$(XVIEW_INC)  -DFOR_XVIEW  $(PROGCFLAGS)
CFLAGS =  $(GFLAG)  -I../h -DFOR_XVIEW  -DWAVES_BINS $(PROGCFLAGS)  -DNO_LIC
#CFLAGS =  $(GFLAG)  -I../h -I$(XVIEW_INC) -DFOR_XVIEW  -DWAVES_BINS $(PROGCFLAGS)  -DNO_LIC
LPATH = $(WAVESDIR)
LBIN = $(BINDIR)
DOCDIR = $(SPSDIR)/doc/waves
WAVES_MISC = $(WAVESDIR)
WAVES_COLOR = $(WAVESDIR)/colormaps
WAVES_FILES = $(WAVESDIR)/files
WAVES_MENUS = $(WAVESDIR)/menus
MISC = ../../misc
INIT = ../../waves_init
LIBSIG = $(WAVESDIR)/xlibsig.a
SIGPROC = $(WAVESDIR)
DSPDIR = $(WAVESDIR)
# LINES BELOW FOR TESTING ONLY.
# LIBSIG = ../../../libsig/xlibsig.a
# SIGPROC = ../../../sigproc
WAVES_MAN = ../../man
WAVES_TEXT = ../../text
INC_DIR = $(SPSDIR)/include/waves


WAVE_OBJS = paste.o xplot_data.o tag_methods.o xwaves.o view_utils.o browser.o \
        xcursors.o xesps.o xmenus.o xmethods.o xoverlays.o pole_methods.o \
        xedit_data.o xf0_methods.o xwaves_utils.o xaddop.o rpc.o \
        play_data.o playrec.o nplay.o xspect.o view_setter.o odd_objects.o \
        text_utils.o  xpanels.o \
	xserver.o ibm_stub.o dac.o keymapper.o

SPECTRUM_OBJS = xspectrum.o text_utils.o

LABEL_OBJS = xlabel.o labelprocs.o labelio.o xchartprocs.o text_utils.o

MARKS_OBJS = xmarks.o markerio.o markerutil.o markeralloc.o xpopup.o

LIB_OBJS = message.o  xreticle.o pole_methods.o xcolormap.o \
	xversion.o globals.o xnotice.o xp_stubs.o

PROGS = xwaves xspectrum xlabel xmarks
PROGS_I = $(BINDIR)/xwaves $(BINDIR)/xspectrum $(BINDIR)/xlabel \
	$(BINDIR)/xmarks
MAN = $(MANDIR)/man1/xwaves.1 $(MANDIR)/man1/xspectrum.1 \
	$(MANDIR)/man1/xlabel.1t $(MANDIR)/man1/xmarks.1 \
	$(MANDIR)/man1/xchart.1

it: xwaves

all:	xwaves xlabel xspectrum xmarks

install: $(PROGS_I) man misc waves_init waves_inc

install_stuff: man misc waves_init waves_inc

install_bins: $(PROGS_I)

install_nostrip: $(PROGS)
	mv $(PROGS) $(BINDIR)

xwaves:	 $(SIGPROC)/libdsigproc.a  $(LIBSIG) \
		xlibsigw.a $(WAVE_OBJS) $(WSPSLIB)
	$(CC)  $(XVIEW_B_OPT) $(GFLAG) $(FLOAT) -o xwaves $(WAVE_OBJS) \
	$(SIGPROC)/libdsigproc.a  $(LIBSIG) \
		xlibsigw.a $(WSPSLIB) ${SUNLIB}  $(BSDLIB) $(AUDIO_LIB)

pure_xwaves:	 $(SIGPROC)/libdsigproc.a  $(LIBSIG) \
		xlibsigw.a $(WAVE_OBJS) $(WSPSLIB)
	purify $(CC)  $(GFLAG) $(FLOAT) -o xwaves $(WAVE_OBJS) \
	$(SIGPROC)/libdsigproc.a  $(LIBSIG) \
		xlibsigw.a $(WSPSLIB) ${SUNLIB}  $(BSDLIB) $(AUDIO_LIB)

xspectrum: $(SPECTRUM_OBJS) xlibsigw.a $(SIGPROC)/libdsigproc.a $(WSPSLIB)
	$(CC) $(XVIEW_B_OPT) $(GFLAG) $(FLOAT) -o xspectrum $(SPECTRUM_OBJS) xlibsigw.a \
	$(SIGPROC)/libdsigproc.a  $(WSPSLIB) ${SUNLIB} $(BSDLIB)

pure_xlabel: $(LABEL_OBJS) xlibsigw.a $(WSPSLIB)
	purify $(CC) $(XVIEW_B_OPT) $(GFLAG) $(FLOAT) -o xlabel  $(LABEL_OBJS) xlibsigw.a \
	 $(WSPSLIB) ${SUNLIB}  $(BSDLIB) -lm

xlabel: $(LABEL_OBJS) xlibsigw.a $(WSPSLIB)
	$(CC) $(XVIEW_B_OPT) $(GFLAG) $(FLOAT) -o xlabel  $(LABEL_OBJS) xlibsigw.a \
	 $(WSPSLIB) ${SUNLIB}  $(BSDLIB) -lm

xmarks: $(MARKS_OBJS) xlibsigw.a
	 $(CC) $(XVIEW_B_OPT) $(GFLAG) $(FLOAT) -o xmarks $(MARKS_OBJS) xlibsigw.a \
		$(WSPSLIB) ${SUNLIB} $(BSDLIB)

$(BINDIR)/xwaves: xwaves
	-strip $?
	-rm -f $(BINDIR)/$?
	cp $? $(BINDIR)/$?
	chmod $(PROGMOD) $(BINDIR)/$?

$(BINDIR)/xspectrum: xspectrum
	-strip $?
	-rm -f $(BINDIR)/$?
	cp $? $(BINDIR)/$?
	chmod $(PROGMOD) $(BINDIR)/$?

$(BINDIR)/xchart: $(BINDIR)/xlabel

$(BINDIR)/xlabel: xlabel
	-strip $?
	-rm -f $(BINDIR)/$?
	cp $? $(BINDIR)/$?
	chmod $(PROGMOD) $(BINDIR)/$?
	-rm -f $(BINDIR)/xchart
	ln $(BINDIR)/xlabel $(BINDIR)/xchart

$(BINDIR)/xmarks: xmarks
	-strip $?
	-rm -f $(BINDIR)/$?
	cp $? $(BINDIR)/$?
	chmod $(PROGMOD) $(BINDIR)/$?

xlibsigw.a: $(LIBSIG)  $(LIB_OBJS)
	rm -f $@
	cp $(LIBSIG) xlibsigw.a
	ar r $@   $(LIB_OBJS)
	-ranlib $@

doc:	
	(cd $(WAVES_TEXT) ; make_doc $(SPSDIR) )
	-rm -f $(DOCDIR)/headers.help
	cp $(WAVES_TEXT)/headers.help $(DOCDIR)/SIGnal.help
	chmod $(MANMOD) $(DOCDIR)/SIGnal.help

man:	$(MAN) $(LIBDIR)/waves/xwaves.man $(LIBDIR)/waves/xspectrum.man \
	$(LIBDIR)/waves/xlabel.man $(LIBDIR)/waves/xchart.man

$(MANDIR)/man1/xwaves.1: $(WAVES_MAN)/xwaves.1
	-rm -f $(MANDIR)/man1/xwaves.1
	-rm -f $(MANDIR)/cat1/xwaves.1
	cp $(WAVES_MAN)/xwaves.1 $(MANDIR)/man1/xwaves.1
	chmod $(MANMOD) $(MANDIR)/man1/xwaves.1

$(LIBDIR)/waves/xwaves.man: $(WAVES_MAN)/ref.txt
	rm -f $(LIBDIR)/waves/xwaves.man
	cp $(WAVES_MAN)/ref.txt $(LIBDIR)/waves/xwaves.man

$(LIBDIR)/waves/xlabel.man: $(WAVES_MAN)/ref.txt
	rm -f $(LIBDIR)/waves/xlabel.man
	cp $(WAVES_MAN)/ref.txt $(LIBDIR)/waves/xlabel.man

$(LIBDIR)/waves/xchart.man: $(WAVES_MAN)/ref.txt
	rm -f $(LIBDIR)/waves/xchart.man
	cp $(WAVES_MAN)/ref.txt $(LIBDIR)/waves/xchart.man

$(LIBDIR)/waves/xspectrum.man: $(WAVES_MAN)/ref.txt
	rm -f $(LIBDIR)/waves/xspectrum.man
	cp $(WAVES_MAN)/ref.txt $(LIBDIR)/waves/xspectrum.man

$(MANDIR)/man1/xspectrum.1: $(WAVES_MAN)/xspectrum.1
	-rm -f $(MANDIR)/man1/xspectrum.1
	cp $(WAVES_MAN)/xspectrum.1 $(MANDIR)/man1/xspectrum.1
	chmod $(MANMOD) $(MANDIR)/man1/xspectrum.1

$(MANDIR)/man1/xlabel.1t: $(WAVES_MAN)/xlabel.1t
	-rm -f $(MANDIR)/man1/xlabel.1t
	cp $(WAVES_MAN)/xlabel.1t $(MANDIR)/man1/xlabel.1t
	chmod $(MANMOD) $(MANDIR)/man1/xlabel.1t

$(MANDIR)/man1/xmarks.1: $(WAVES_MAN)/xmarks.1
	-rm -f $(MANDIR)/man1/xmarks.1
	cp $(WAVES_MAN)/xmarks.1 $(MANDIR)/man1/xmarks.1
	chmod $(MANMOD) $(MANDIR)/man1/xmarks.1

$(MANDIR)/man1/xchart.1: $(WAVES_MAN)/xchart.1
	-rm -f $(MANDIR)/man1/xchart.1
	cp $(WAVES_MAN)/xchart.1 $(MANDIR)/man1/xchart.1
	chmod $(MANMOD) $(MANDIR)/man1/xchart.1

waves_init:
	( cd $(INIT) ; emake install )

waves_inc:
	-rm -f $(INC_DIR)/*
	-rm -f $(INC_DIR)/mp/*
	-mkdir $(INC_DIR)
	-mkdir $(INC_DIR)/mp
	cp ../h/*.h $(INC_DIR)
	cp ../h/mp/*.h $(INC_DIR)/mp
	chmod $(HMOD) $(INC_DIR)/*.h
	chmod 0755 $(INC_DIR)/mp
	chmod $(MANMOD) $(INC_DIR)/mp/*

misc:
	-rm -f $(WAVESDIR)/.wave_pro
	cp $(MISC)/.wave_pro $(WAVESDIR)
	chmod $(MANMOD) $(WAVESDIR)/.wave_pro
	cp $(MISC)/init_file.WC $(WAVESDIR)/commands
	chmod $(MANMOD) $(WAVESDIR)/.wave_pro
	-rm -f $(WAVES_COLOR)/neg_greymap
	cp $(MISC)/neg_greymap $(WAVES_COLOR)
	chmod $(MANMOD) $(WAVES_COLOR)/neg_greymap
	-rm -f $(WAVES_COLOR)/greymap
	cp $(MISC)/greymap $(WAVES_COLOR)
	chmod $(MANMOD) $(WAVES_COLOR)/greymap
	-rm -f $(WAVES_COLOR)/colormap
	cp $(MISC)/colormap $(WAVES_COLOR)
	chmod $(MANMOD) $(WAVES_COLOR)/colormap
	-rm -f $(WAVES_COLOR)/TImap
	cp $(MISC)/TImap $(WAVES_COLOR)
	chmod $(MANMOD) $(WAVES_COLOR)/TImap
	-rm -f $(WAVES_COLOR)/COLOR_MAP
	cp $(MISC)/COLOR_MAP $(WAVES_COLOR)
	chmod $(MANMOD) $(WAVES_COLOR)/COLOR_MAP
	-rm -f $(WAVES_MENUS)/labelmenu.timit
	cp $(MISC)/labelmenu.timit $(WAVES_MENUS)/labelmenu.timit
	chmod $(MANMOD) $(WAVES_MENUS)/labelmenu.timit
	-rm -f $(WAVES_MENUS)/labelmenu.darpa
	cp $(MISC)/labelmenu.darpa $(WAVES_MENUS)/labelmenu.darpa
	chmod $(MANMOD) $(WAVES_MENUS)/labelmenu.darpa
	-rm -f $(WAVES_MENUS)/labelmenu.def
	-rm -f $(WAVES_MENUS)/labelmenu
	(cd $(WAVES_MENUS); ln -s labelmenu.timit labelmenu.def; \
			    ln -s labelmenu.timit labelmenu )
	-rm -f $(WAVES_FILES)/nb_params
	cp $(MISC)/nb_params $(WAVES_FILES)
	chmod $(MANMOD) $(WAVES_FILES)/nb_params
	-rm -f $(WAVES_FILES)/wb_params
	cp $(MISC)/wb_params $(WAVES_FILES)
	chmod $(MANMOD) $(WAVES_FILES)/wb_params
	-rm -f $(WAVES_FILES)/dspinfo
	cp $(MISC)/dspinfo $(WAVES_FILES)
	chmod 0644 $(WAVES_FILES)/dspinfo
	-rm -f $(WAVES_FILES)/def_head.feasd
	cp $(MISC)/def_head.feasd $(WAVES_FILES)
	chmod 0644 $(WAVES_FILES)/def_head.feasd
	-rm -f $(WAVES_FILES)/def_head.dec
	cp $(MISC)/def_head.dec $(WAVES_FILES)
	chmod 0644 $(WAVES_FILES)/def_head.dec
	-rm -f $(WAVES_FILES)/def_head.sig
	cp $(MISC)/def_head.sig $(WAVES_FILES)
	chmod 0644 $(WAVES_FILES)/def_head.sig
	-rm -f $(WAVES_FILES)/color.fpsec
	cp $(MISC)/color.fspec $(WAVES_FILES)
	chmod 0644 $(WAVES_FILES)/color.fspec
	cp $(MISC)/marks.sh $(WAVES_FILES)
	chmod 0644 $(WAVES_FILES)/marks.sh
	-rm -f $(WAVESDIR)/xwaves.help
	cp $(MISC)/xwaves.help $(WAVESDIR)
	chmod 0644 $(WAVESDIR)/xwaves.help
	-rm -f $(WAVES_FILES)/xmarks.sh
	cp $(MISC)/xmarks.sh $(WAVES_FILES)
	chmod 0755 $(WAVES_FILES)/xmarks.sh

# These .h dependencies are hopelessly out of date.  If you change a
# .h file, do not assume the necessary recompilations will happen
# automatically! (dt 5/13/93)

xcolormap.o:	../h/Objects.h
xcursors.o:	../h/Objects.h
xedit_data.o:	../h/Objects.h ../h/spectrogram.h \
	    $(SINC)/esps.h $(SINC)/limits.h
xesps.o:	../h/Objects.h
xf0_methods.o: 	../h/Objects.h
globals.o:	../h/Objects.h
xlabel.o:	../h/Objects.h ../h/labels.h
labelio.o:	../h/Objects.h ../h/labels.h
labelprocs.o:	../h/Objects.h ../h/labels.h
xmenus.o:	../h/Objects.h
message.o:	../h/Methods.h
xmethods.o:	../h/Objects.h
nplay.o:	nplay.c ../h/Objects.h
	$(CC) -c ${CFLAGS}  nplay.c
xoverlays.o:	../h/Objects.h ../h/spectrogram.h ../h/tracks.h \
	    $(SINC)/esps.h
xpanels.o:

paste.o:	../h/Objects.h ../h/file_ext.h \
	    $(SINC)/esps.h

play_data.o:	play_data.c    ../h/Objects.h
	$(CC) -c ${CFLAGS} -DLBIN=\"${LBIN}\" play_data.c

playrec_ns.o:	playrec.c
	$(CC) -c ${CFLAGS}  playrec.c
	mv playrec.o playrec_ns.o

playrec.o:	playrec.c    ../h/dsplock.h
	$(CC) -c ${CFLAGS}  playrec.c

xplot_data.o:	../h/Objects.h
tag_methods.o:	../h/Objects.h ../h/Signals.h ../h/Utils.h
pole_methods.o:	../h/Objects.h ../h/tracks.h
record_data.o:	record_data.c
	$(CC) -c ${CFLAGS}  record_data.c

xreticle.o:	xreticle.c \
	    ../h/Objects.h ../h/reticle.h \
	    $(SINC)/esps.h
	$(CC) -c -DPW_PLOT $(CFLAGS) xreticle.c

xspect.o:	../h/Objects.h ../h/spectrogram.h ../h/dsplock.h \
	    $(SINC)/esps.h $(SINC)/feaspec.h

xspectrum.o:

text_utils.o:

xversion.o:	$(SINC)/esps.h
xwaves.o:	../h/Objects.h \
	    $(SINC)/esps.h
xwaves_utils.o:	../h/Objects.h ../h/spectrogram.h ../h/file_ext.h \
	    $(SINC)/esps.h $(SINC)/fea.h

clean:
	-rm -f *.o *~ core

clobber:
	-rm -f *.o *~ core *.a $(PROGS)

clean_bins:
	-rm -f $(PROGS)
