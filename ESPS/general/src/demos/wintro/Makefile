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
# @(#)Makefile	1.8 1/18/93 ERL
# 
# Makefile for: waves+ Intro demo
#
# Written by:  
# Checked by:
# Revised by:

DEMONAME = wintro

MAIN_DEMODIR = $(SPSDIR)/newdemos

#line below is for testing (replaces line above
#MAIN_DEMODIR = /h5/shore/demo

D_DIRS = files wcommands menus binary_files help colormaps bin params

D_TOP_FILES = README rundemo rundemo.sh

D_COLORMAPS = grey_neg

D_WCOMMANDS = defimgmen.WC do_files.WC mk_albert.WC mk_spgm2.WC wavmen1.WC defwavmen.WC do_menus.WC mk_image.WC mk_wform.WC wavmen2.WC do_color.fspec imagmen1.WC mk_spgm.WC mk_wform1.WC wtry.WC do_colors.WC imagmen2.WC mk_spgm1.WC mk_wform2.WC xw_init.WC builtinwave.WC builtinspec.WC

D_MENUS = colors.WM demofiles.BM files.WM controls.WM menus.WM

D_PARAMS = Pfft Prefcof

D_SCRIPTS = do_Xrefcof

D_HELP = README in_files.doc menus.doc brief.instrs inter_waves.doc colors.doc

D_FILES = wave_pro

D_BINARY_FILES = al.spec sat.fspec signal.sd signal2.sd speech.wb.spgm color.fspec signal.fft.spgm signal2.fft speech.sd

SCCS_FILES = $(D_TOP_FILES) $(D_COLORMAPS) $(D_WCOMMANDS) $(D_MENUS) \
	$(D_PARAMS) $(D_SCRIPTS) $(D_FILES) $(D_HELP)

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

help:		top
		mkdir $(DEMODIR)/help
		chmod $(DIRMODE) $(DEMODIR)/help
		cp $(D_HELP) $(DEMODIR)/help
		chmod $(READMOD) $(DEMODIR)/help/*

	
menus: 		top
		mkdir $(DEMODIR)/menus
		chmod $(DIRMODE) $(DEMODIR)/menus
		cp $(D_MENUS) $(DEMODIR)/menus
		chmod $(READMOD) $(DEMODIR)/menus/*

params:		top
		mkdir $(DEMODIR)/params
		chmod $(DIRMODE) $(DEMODIR)/params
		cp $(D_PARAMS) $(DEMODIR)/params
		chmod $(READMOD) $(DEMODIR)/params/*

bin:		top
		mkdir $(DEMODIR)/bin
		chmod $(DIRMODE) $(DEMODIR)/bin
		cp $(D_SCRIPTS) $(DEMODIR)/bin
		chmod $(EXMODE) $(DEMODIR)/bin/*

esccs:		
		esccs $(DEMONAME) get $(SCCS_FILES)


















