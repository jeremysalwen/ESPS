# **********************************************************
# Project:		ARG General libraries
# SubTree:		/home/projects/devel_source/general/src/lib_header/sphere2.6
# Filename:		Makefile
# Programmer:		J. Fiscus
# Organization:		NIST/NCSL/DIV 670/Auto Rec. Group
# Host System:		SUN 4 OS/4.1.1 (UNIX)
# Date Created:		05/28/92
#
# **********************************************************
# Makefile for Root Project Directory
# **********************************************************
#Line below doesn't work for gnu make on Fedora linux
#PROJDIR = `pwd`
# Line below assumes that the source and the ESPS installation are in the same
# directory tree.  If the assumption is wrong, pass the value of PROJDIR to
# make on the command line and it will take precedence over this setting.
PROJDIR = $(ESPS_BASE)/../src/ESPS/general/src/lib_header/sphere2.6
SUBS = src
SHELL = /bin/sh

it install depend clean:
	@-X=`pwd`; \
	for i in $(SUBS); \
	do echo '<<<' $$i '>>>'; cd $$X/$$i; \
	make $@ PROJDIR=$(PROJDIR) ; done

bare: clean
	rm -f bin/* lib/packages/*.pas lib/lib*.a 
