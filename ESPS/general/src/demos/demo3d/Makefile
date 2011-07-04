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
# @(#)Makefile	1.10 1/18/93 ERL
# 
# Makefile for: plot3d demo
#
# Written by:  John Shore
# Checked by:
# Revised by:

SHELL=/bin/sh
DEMONAME = demo3d

MAIN_DEMODIR = $(SPSDIR)/newdemos

#line below is for testing (replaces line above
#MAIN_DEMODIR = /h5/shore/demo

D_DIRS = files wcommands menus 

D_TOP_FILES = README rundemo rundemo.sh

D_COLORMAPS = 

D_WCOMMANDS = demoinit.WC demoquit.WC builtinspec.WC builtinwave.WC

D_MENUS = dcolors.WM demofiles.BM plot3d.BM

D_PARAMS =

D_SCRIPTS = 

D_FILES = wave_pro

D_BINARY_FILES = sat.fspec

SCCS_FILES = $(D_TOP_FILES) $(D_COLORMAPS) $(D_WCOMMANDS) $(D_MENUS) \
	$(D_PARAMS) $(D_SCRIPTS) $(D_FILES) 

DEMODIR=$(MAIN_DEMODIR)/$(DEMONAME)
READMOD=0444
DIRMODE=0755
EXMODE=0555

install: 	top $(D_DIRS)

top:
		rm -rf $(DEMODIR)
		mkdir $(DEMODIR)
		chmod $(DIRMODE) $(DEMODIR)
		cp $(D_TOP_FILES) $(DEMODIR)
		chmod $(READMOD) $(DEMODIR)/*
		chmod $(EXMODE) $(DEMODIR)/rundemo
		chmod $(EXMODE) $(DEMODIR)/rundemo.sh

wcommands:	top
		mkdir $(DEMODIR)/wcommands
		chmod $(DIRMODE) $(DEMODIR)/wcommands
		cp $(D_WCOMMANDS) $(DEMODIR)/wcommands
		chmod $(READMOD) $(DEMODIR)/wcommands/*

files:		top
		mkdir $(DEMODIR)/files
		chmod $(DIRMODE) $(DEMODIR)/files
		cp $(D_FILES) $(DEMODIR)/files
		(cd binary_files; cp $(D_BINARY_FILES) $(DEMODIR)/files)
		chmod $(READMOD) $(DEMODIR)/files/*

colormaps:	top
		mkdir $(DEMODIR)/colormaps
		chmod $(DIRMODE) $(DEMODIR)/colormaps
		cp $(D_COLORMAPS) $(DEMODIR)/colormaps
		chmod $(READMOD) $(DEMODIR)/colormaps/*

	
menus: 		top
		mkdir $(DEMODIR)/menus
		chmod $(DIRMODE) $(DEMODIR)/menus
		cp $(D_MENUS) $(DEMODIR)/menus
		chmod $(READMOD) $(DEMODIR)/menus/*

params:		
		mkdir $(DEMODIR)/params
		chmod $(DIRMODE) $(DEMODIR)/params
		cp $(D_PARAMS) $(DEMODIR)/params
		chmod $(READMOD) $(DEMODIR)/params/*

bin:		
		mkdir $(DEMODIR)/bin
		chmod $(DIRMODE) $(DEMODIR)/bin
		cp $(D_SCRIPTS) $(DEMODIR)/bin
		chmod $(EXMODE) $(DEMODIR)/bin/*

esccs:		
		esccs $(DEMONAME) get $(SCCS_FILES)
