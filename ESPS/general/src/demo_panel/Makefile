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
# @(#)Makefile	1.6 6/20/91 ERL
# 
# Makefile for: demo button panel
#
# Written by:  David Burton
# Checked by:
# Revised by:

README=README.panel
PROGNAME=DEMOS
MANNAME=demos.1
DIRMODE=0755
WDIRMODE=0777
# remove next line after testing
HMOD=0444

install: $(PROGNAME)
	-rm $(SPSDIR)/demo/$(README)
	cp $(README) $(SPSDIR)/demo
	chmod $(MANMOD) $(SPSDIR)/demo/$(README)

	-rm -rf $(SPSDIR)/demo/colormaps
	mkdir $(SPSDIR)/demo/colormaps
	chmod $(DIRMODE) $(SPSDIR)/demo/colormaps
	cp colormaps/* $(SPSDIR)/demo/colormaps
	chmod $(MANMOD) $(SPSDIR)/demo/colormaps/*

	-rm -rf $(SPSDIR)/demo/commands
	mkdir $(SPSDIR)/demo/commands
	chmod $(WDIRMODE) $(SPSDIR)/demo/commands
	cp commands/* $(SPSDIR)/demo/commands
	chmod $(MANMOD) $(SPSDIR)/demo/commands/*
	chmod $(WDIRMODE) $(SPSDIR)/demo/commands/reloc.w

	-rm -rf $(SPSDIR)/demo/files
	mkdir $(SPSDIR)/demo/files
	chmod $(WDIRMODE) $(SPSDIR)/demo/files
	cp files/* $(SPSDIR)/demo/files
	chmod $(MANMOD) $(SPSDIR)/demo/files/*

	-rm -rf $(SPSDIR)/demo/filters
	mkdir $(SPSDIR)/demo/filters
	chmod $(DIRMODE) $(SPSDIR)/demo/filters
	cp filters/* $(SPSDIR)/demo/filters
	chmod $(MANMOD) $(SPSDIR)/demo/filters/*

	-rm -rf $(SPSDIR)/demo/init_files
	mkdir $(SPSDIR)/demo/init_files
	chmod $(DIRMODE) $(SPSDIR)/demo/init_files
	cp init_files/* $(SPSDIR)/demo/init_files
	chmod $(MANMOD) $(SPSDIR)/demo/init_files/*

	-rm -rf $(SPSDIR)/demo/menus
	mkdir $(SPSDIR)/demo/menus
	chmod $(DIRMODE) $(SPSDIR)/demo/menus
	cp menus/* $(SPSDIR)/demo/menus
	chmod $(MANMOD) $(SPSDIR)/demo/menus/*

	-rm -rf $(SPSDIR)/demo/param_files
	mkdir $(SPSDIR)/demo/param_files
	chmod $(WDIRMODE) $(SPSDIR)/demo/param_files
	cp param_files/* $(SPSDIR)/demo/param_files
	chmod $(MANMOD) $(SPSDIR)/demo/param_files/*

	-rm -rf $(SPSDIR)/demo/scripts
	mkdir $(SPSDIR)/demo/scripts
	chmod $(DIRMODE) $(SPSDIR)/demo/scripts
	cp scripts/* $(SPSDIR)/demo/scripts
	chmod $(DIRMODE) $(SPSDIR)/demo/scripts/*
	
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	-cp $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(MANNAME) $(MANNAME) man.diff
	@echo diffmked man page left in man.diff
	-rm $(MANDIR)/man1/$(MANNAME)
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)



	

