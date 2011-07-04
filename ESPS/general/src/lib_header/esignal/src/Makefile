# Copyright (c) 1995 Entropic Research Laboratory, Inc.
# All rights reserved.
#
# Permission to use, copy, modify, and distribute this Esignal (TM)
# I/O software and its documentation for any purpose and without fee
# is hereby granted, provided that the above copyright notice and this
# statement of permission appears in all copies of the software as
# well as in supporting documentation.  Entropic Research Laboratory,
# Inc. makes no representations about the suitability of this
# software for any purpose.  It is provided "as is" without express
# or implied warranty.
#
# (TM) Esignal is a Trademark of Entropic Research Laboratory, Inc.
# (R) elib is a Registered Trademark of Entropic Research Laboratory,
# Inc.
########################################################################
# %W% %G% ERL
#
# Makefile for illustrative functions for Esignal file format file I/O.
#
# Author: Rod Johnson
#

INCDIR = .
#LIBDIR = ../lib
TSTDIR = ../testprog

CFLAGS = -g -I$(INCDIR)

#CC = cc -Xa -D'ARCH="Sun4"' 
#CC = acc -Xa -D'ARCH="Sun4"' 
#CC = gcc -ansi -pedantic -Wall -D'ARCH="Sun4"' 
CC = gcc -ansi -pedantic -Wall -D'ARCH="Linux"' 
#CC = cc -std1 -D'ARCH="Alpha"'
#CC = cc -ansi -fullwarn -D'ARCH="Iris"'
#CC = cc -Aa -D'ARCH="HP700"' 


LIB = $(LIBDIR)/libesignal.a
LIBSRC = esignal.c esig_asc.c esig_edr.c esig_nat.c
LIBOBJ = esignal.o esig_asc.o esig_edr.o esig_nat.o

TSTPROG = $(TSTDIR)/t_esignal
TSTSRC = t_esignal.c
#TSTSRC = t_esignal.c getopt.c
TSTOBJ = t_esignal.o
#TSTOBJ = t_esignal.o getopt.o

INCLUDE = esignal.h


lib: $(LIB)

$(LIB):	$(LIBOBJ)
	ar r $(LIB) $?
	-ranlib $(LIB)

$(LIBOBJ): $(INCLUDE)


testprog: $(TSTPROG)

$(TSTPROG): $(TSTOBJ) $(LIB) 
	$(CC) $(CFLAGS) $(TSTOBJ) $(LIB) -lm -o $(TSTPROG)
$(TSTOBJ):  $(INCLUDE)


clean:
	-rm -f $(LIBOBJ) $(TSTOBJ) $(LIB) core
