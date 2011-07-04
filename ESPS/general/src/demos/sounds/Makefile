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
# Makefile for: Sounds demo
#
# Written by:  
# Checked by:
# Revised by:


DEMONAME = sounds

MAIN_DEMODIR = $(SPSDIR)/newdemos

#line below is for testing (replaces line above
#MAIN_DEMODIR = /h5/shore/demo

D_DIRS = files wcommands menus bin params binary_files

D_TOP_FILES = README rundemo rundemo.sh

D_COLORMAPS = 

D_WCOMMANDS = demoinit.WC demoquit.WC builtinspec.WC builtinwave.WC

D_MENUS = dcolors.WM demofiles.BM files.BM sounds.BM dcontrols.WM dmbind.WM setup.BM

D_PARAMS = params.sgram

D_SCRIPTS = backplay dosound full_his null_play short_his excerpt play_sound

D_HELP =

D_FILES = wave_pro

D_BINARY_FILES = BigBen.sd CrashCymbal.sd MeepMeep.sd MetalAlarm.sd SteelyEchoC2.sd Whoosh.sd applause.sd areyouagod.sd art1.sd attack.sd bells.sd bong.sd breaking_glass.sd breathing.sd bubbles.sd burp.sd chirp.sd clink.sd clint_eastwood.sd computer.sd cowbell.sd crash.sd cuckoo.sd daffy_laugh.sd does_not_compute.sd dog.sd drip.sd elephant.sd flinstones.sd flush.sd gong.sd hallelujah.sd hey_rocky.sd i_am_a_wabbit.sd justwhat.sd kookaburra.sd laugh.sd laughter.sd
#split to satisfy sccs line length
D_BINARY_FILES2 =  max_headroom.sd mindgoing.sd mission.sd modanna_computer.sd monkey.sd moo.sd muchbetter.sd musicover.sd nomatter.sd noquestion.sd other_bird.sd perfectly.sd pigs.sd red_wattle_bird.sd rooster.sd sea_lion.sd something_for_dinner.sd something_skewey.sd sorrydave.sd spacemusic.sd splat.sd toccata.sd train.sd uh,_no,_no,...sd wheee.sd whip_bird.sd whistle.sd youvegotmail.sd zubetube.sd

SCCS_FILES = $(D_TOP_FILES) $(D_COLORMAPS) $(D_WCOMMANDS) $(D_MENUS) \
	$(D_PARAMS) $(D_SCRIPTS) $(D_FILES) $(D_HELP)

DEMODIR=$(MAIN_DEMODIR)/$(DEMONAME)
READMOD=0444
DIRMODE=0755
EXMODE=0555

install: 	top dirs

top:
		rm -rf $(DEMODIR)
		mkdir $(DEMODIR)
		chmod $(DIRMODE) $(DEMODIR)
		cp $(D_TOP_FILES) $(DEMODIR)
		chmod $(READMOD) $(DEMODIR)/*
		chmod $(EXMODE) $(DEMODIR)/rundemo
		chmod $(EXMODE) $(DEMODIR)/rundemo.sh

dirs: $(D_DIRS)

wcommands:	top
		mkdir $(DEMODIR)/wcommands
		chmod $(DIRMODE) $(DEMODIR)/wcommands
		cp $(D_WCOMMANDS) $(DEMODIR)/wcommands
		chmod $(READMOD) $(DEMODIR)/wcommands/*

files:		top
		mkdir $(DEMODIR)/files
		chmod $(DIRMODE) $(DEMODIR)/files
		cp $(D_FILES) $(DEMODIR)/files
		mkdir $(DEMODIR)/files/audio
		(cd binary_files; cp $(D_BINARY_FILES) $(DEMODIR)/files/audio; 					 cp $(D_BINARY_FILES2) $(DEMODIR)/files/audio)
		chmod $(READMOD) $(DEMODIR)/files/*
		chmod $(DIRMODE) $(DEMODIR)/files/audio
		chmod $(READMOD) $(DEMODIR)/files/audio/*

#colormaps:	top
#		mkdir $(DEMODIR)/colormaps
#		chmod $(DIRMODE) $(DEMODIR)/colormaps
#		cp $(D_COLORMAPS) $(DEMODIR)/colormaps
#		chmod $(READMOD) $(DEMODIR)/colormaps/*

#help:		top
#		mkdir $(DEMODIR)/help
#		chmod $(DIRMODE) $(DEMODIR)/help
#		cp $(D_HELP) $(DEMODIR)/help
#		chmod $(READMOD) $(DEMODIR)/help/*

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


















