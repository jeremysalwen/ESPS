#
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1987-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)makefile	1.16 28 Oct 1999 ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for:  lib_header (ESPS header.o library with license manager)
#


GFLAG = -g -DNO_LIC
CFLAGS =  $(GFLAG) $(CCKR) $(LIBCFLAGS)  $(OFLAGS) 
FFLAGS =  $(LIBFFLAGS)
ELMDIR = $(LIBDIR)/elm
OBJS = genhd.o eopen.o ss.o
SPHERE_LIBS = sphere2.6/lib
SPHERE_INC = sphere2.6/include

ESIGNAL_LIB = $(LIBDIR)
#ESIGNAL_LIB = esignal/lib
ESIGNAL_INC = esignal/src

install:	$(LIBDIR)/$(EHDRLIB) $(LIBDIR)/$(WHDRLIB) $(LIBDIR)/$(SHDRLIB) \
	$(LIBDIR)/$(NHDRLIB)


$(LIBDIR)/$(EHDRLIB):	 $(OBJS) esps/headers.o 
		mv headers.o foo.o
		ar rv $@  $?; $(RANLIB) $@
		mv foo.o headers.o

$(LIBDIR)/$(WHDRLIB):	 $(OBJS) waves/headers.o 
		mv headers.o foo.o
		ar rv $@  $?; $(RANLIB) $@
		mv foo.o headers.o

$(LIBDIR)/$(SHDRLIB):	 $(OBJS) ers/headers.o 
		mv headers.o foo.o
		ar rv $@  $?; $(RANLIB) $@
		mv foo.o headers.o

$(LIBDIR)/$(NHDRLIB):	 $(OBJS) nolic/headers.o 
		ar rv $@  $?; $(RANLIB) $@

# 
#
esps/headers.o:  license.c headers.o $(SINC)/header.h  \
			$(SINC)/esps.h   sphere.o \
			$(ESIGNAL_LIB)/libesignal.a esignal_fea.o  pc_wav.o
		$(ELM_CC) -c  $(CFLAGS)   -DESPS_PROG $(NO_LIC) license.c
		mv license.o esps
		ld -o esps/headers.o -r headers.o  esps/license.o \
		  sphere.o \
		esignal_fea.o $(ESIGNAL_LIB)/libesignal.a  pc_wav.o \
		$(SPHERE_LIBS)/libsp.a  $(SPHERE_LIBS)/libutil.a 

waves/headers.o: license.c headers.o  $(SINC)/header.h   \
			$(SINC)/esps.h   sphere.o \
			$(ESIGNAL_LIB)/libesignal.a esignal_fea.o  pc_wav.o
		$(ELM_CC) -c $(OFLAGS) $(CFLAGS) -I$(XVIEW_INC) -DWAVES_PROG $(NO_LIC) license.c
		mv license.o waves
		ld -o waves/headers.o -r headers.o waves/license.o \
		sphere.o \
		esignal_fea.o $(ESIGNAL_LIB)/libesignal.a  pc_wav.o \
		$(SPHERE_LIBS)/libsp.a  $(SPHERE_LIBS)/libutil.a 

ers/headers.o: license.c headers.o  $(SINC)/header.h   \
			$(SINC)/esps.h sphere.o \
			$(ESIGNAL_LIB)/libesignal.a esignal_fea.o  pc_wav.o
		$(ELM_CC) -c $(CFLAGS) -I$(XVIEW_INC) -DWAVES_PROG -DERS_PROG $(NO_LIC) license.c
		mv license.o ers
		ld -o ers/headers.o -r headers.o ers/license.o \
		sphere.o \
		esignal_fea.o $(ESIGNAL_LIB)/libesignal.a pc_wav.o \
		$(SPHERE_LIBS)/libsp.a  $(SPHERE_LIBS)/libutil.a 

nolic/headers.o: nolic_headers.o  $(SINC)/header.h   \
			$(SINC)/esps.h  sphere.o \
			$(ESIGNAL_LIB)/libesignal.a esignal_fea.o  pc_wav.o
		ld -o nolic/headers.o -r nolic_headers.o sphere.o \
		esignal_fea.o $(ESIGNAL_LIB)/libesignal.a  pc_wav.o \
		$(SPHERE_LIBS)/libsp.a  $(SPHERE_LIBS)/libutil.a 

$(ESIGNAL_LIB)/libesignal.a:
		cd $(ESIGNAL_INC); emake clean ; emake lib

headers.o:	$(SINC)/header.h $(SINC)/esps.h 
nolic_headers.o:	headers.c $(SINC)/header.h $(SINC)/esps.h 
		$(CC) -DNO_LIC $(CFLAGS) -c -o nolic_headers.o headers.c
genhd.o:	$(SINC)/header.h  $(SINC)/esps.h
eopen.o:	$(SINC)/esps.h
ss.o:		$(SINC)/ss.h
sphere.o:	$(SINC)/esps.h 
		$(ANSI_CC) -c $(GFLAG) -DLIB -DSCCS -I$(SPSDIR)/include \
			-D$(MACH) -I$(SPHERE_INC)  sphere.c

esignal_fea.o:	$(SINC)/esps.h $(SINC)/fea.h $(ESIGNAL_INC)/esignal.h \
			$(SINC)/esignal_fea.h
		$(ANSI_CC)  -c $(CFLAGS) -D$(MACH) -I$(ESIGNAL_INC) \
			esignal_fea.c

pc_wav.o:	$(SINC)/esps.h $(SINC)/fea.h $(SINC)/pc_wav.h
		$(ANSI_CC) -c $(GFLAG) $(LIBCFLAGS) pc_wav.c

clean:
		-rm $(OBJS) foo.o nolic_headers.o headers.o esps/*.o \
			waves/*.o ers/*.o sphere.o esignal_fea.o pc_wav.o

