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
# Makefile for:  doc
#
# Written by:  Ken Nelson
# Checked by:
# Revised by:
 	
DOCDIR=$(SPSDIR)/doc

default: 
	echo "install is only target of this makefile."
	echo ""

STANDARDDOCS = FandC convert fea feasd files guide intro param \
	       rel4.1 rel40 wrel2.1 wrel20 keys READFIRST miio \
	       history guide xig  machine_specific

install: installdocs installdocsrc install_get_help

installdocsrc:
	cp rel4.1.me $(DOCDIR)
	cp wrel2.1.me $(DOCDIR)
	chmod $(MANMOD) $(DOCDIR)/*.me
	-rm $(SPSDIR)/doc/esps.rel
	-(cd $(SPSDIR)/doc;\
	  ln -s rel4.1.doc esps.rel )
	-rm $(SPSDIR)/doc/waves.rel
	-(cd $(SPSDIR)/doc;\
	  ln -s wrel2.1.doc waves.rel)

mach_spec: 
	sh ./install.sh machine_specific

installdocs: 
	sh ./install.sh $(STANDARDDOCS)
	cp filter.doc $(DOCDIR)/filter.doc
	chmod $(MANMOD) $(DOCDIR)/filter.doc

install_get_help:
	cp gettinghelp.doc $(DOCDIR)/gettinghelp.doc
	chmod $(MANMOD) $(DOCDIR)/gettinghelp.doc

print:
	sh ./printdocs $(STANDARDDOCS)

