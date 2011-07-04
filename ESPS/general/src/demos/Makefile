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
# %W% %G% ERL
# 
# Makefile for: demo button panel
#
# Written by:  David Burton
# Checked by:
# Revised by:  John Shore

# Assumes the existence of newdemos
MAIN_DEMO_DIR = $(SPSDIR)/newdemos

#Remove after testing:
#MAIN_DEMO_DIR = /h5/shore/demo

PROGNAME=edemos
PROGNAME2=do_demo
PROGNAME3=edemos.sh
PROGNAME4=randport
PROGNAME5=guess_audio

PROGNAME4_OBJ=randport.o

MENUNAME=demo_menu
WAVES_MENUNAME=demo_menu.waves
MANNAME=edemos.1
MANMOD=0444
DIRMODE=0755
EXMODE=0555

CFLAGS = -O $(PROGCFLAGS)

randport: randport.o 
	$(CC)  $(CFLAGS) randport.o $(SPSLIB) -lm -o $(PROGNAME4)

install:	$(PROGNAME) $(PROGNAME2) $(PROGNAME3) $(PROGNAME4) $(MANNAME) $(MENUNAME) $(WAVES_MENUNAME)
	-rm -f $(MAIN_DEMO_DIR)/$(MENUNAME)
	-cp $(MENUNAME) $(MAIN_DEMO_DIR)/$(MENUNAME)
	-chmod $(MANMOD) $(MAIN_DEMO_DIR)/$(MENUNAME)

	-rm -f $(MAIN_DEMO_DIR)/$(WAVES_MENUNAME)
	-cp $(WAVES_MENUNAME) $(MAIN_DEMO_DIR)/$(WAVES_MENUNAME)
	-chmod $(MANMOD) $(MAIN_DEMO_DIR)/$(WAVES_MENUNAME)

	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	-cp $(PROGNAME) $(BINDIR)/$(PROGNAME)
	-chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)

	-mv -f $(BINDIR)/$(PROGNAME2) $(OLDBIN)/$(PROGNAME2)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME2)
	-cp $(PROGNAME2) $(BINDIR)/$(PROGNAME2)
	-chmod $(PROGMOD) $(BINDIR)/$(PROGNAME2)

	-rm -f $(LIBDIR)/$(PROGNAME3)
	-cp $(PROGNAME3) $(LIBDIR)/$(PROGNAME3)
	-chmod $(PROGMOD) $(LIBDIR)/$(PROGNAME3)

	-mv -f $(BINDIR)/$(PROGNAME4) $(OLDBIN)/$(PROGNAME4)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME4)
	-strip $(PROGNAME4)
	-mv $(PROGNAME4) $(BINDIR)/$(PROGNAME4)
	-chmod $(PROGMOD) $(BINDIR)/$(PROGNAME4)

	-mv -f $(BINDIR)/$(PROGNAME5) $(OLDBIN)/$(PROGNAME5)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME5)
	-cp $(PROGNAME5) $(BINDIR)/$(PROGNAME5)
	-chmod $(PROGMOD) $(BINDIR)/$(PROGNAME5)

	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm $(MANDIR)/man1/$(MANNAME)
	-cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	-chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

	(cd demo3d; emake install)
	(cd filterdemo; emake install)
	(cd sounds; emake install)
	(cd testsignal; emake install)
	(cd timitdemo; emake install)
	(cd wintro; emake install)


test: $(PROGNAME) $(PROGNAME2) $(PROGNAME3) $(MANNAME) $(MENUNAME)
	-rm -f $(MAIN_DEMO_DIR)/$(MENUNAME)
	cp $(MENUNAME) $(MAIN_DEMO_DIR)/$(MENUNAME)
	chmod $(MANMOD) $(MAIN_DEMO_DIR)/$(MENUNAME)
	-cp $(PROGNAME) $(MAIN_DEMO_DIR)/$(PROGNAME)
	chmod $(PROGMOD) $(MAIN_DEMO_DIR)/$(PROGNAME)
	-cp $(PROGNAME2) $(MAIN_DEMO_DIR)/$(PROGNAME2)
	chmod $(PROGMOD) $(MAIN_DEMO_DIR)/$(PROGNAME2)
	-rm -f $(MAIN_DEMO_DIR)/$(PROGNAME3)
	-cp $(PROGNAME3) $(MAIN_DEMO_DIR)/$(PROGNAME3)
	chmod $(PROGMOD) $(MAIN_DEMO_DIR)/$(PROGNAME3)
	-cp $(PROGNAME5) $(MAIN_DEMO_DIR)/$(PROGNAME5)
	chmod $(PROGMOD) $(MAIN_DEMO_DIR)/$(PROGNAME5)

clean:
	rm -f randport randport.o

