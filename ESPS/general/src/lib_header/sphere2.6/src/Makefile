# **********************************************************
# Project:		ARG General libraries
# SubTree:		/home/projects/devel_source/general/src/lib_header/sphere2.6/src
# Filename:		Makefile
# Programmer:		J. Fiscus
# Organization:		NIST/NCSL/DIV 670/Auto Rec. Group
# Host System:		SUN 4 OS/4.1.1 (UNIX)
# Date Created:		05/28/92
#
# **********************************************************
# Makefile for SRC Directory
# **********************************************************
SUBS = lib bin scripts
SHELL = /bin/sh

it install depend clean bare:
	@-X=`pwd`; \
	for i in $(SUBS); \
	do echo '<<' $$i '>>'; cd $$X/$$i; make $@ PROJDIR=$(PROJDIR); done
