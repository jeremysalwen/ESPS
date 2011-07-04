#
#  This material contains proprietary software of Entropic Research, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Research, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Research, Inc. must bear the notice			
# 								
#      "Copyright (c) 1990 Entropic Research, Inc.; All rights reserved"
# 				
#
# 	
# @(#)makefile	1.3 3/15/94 makefile for ESPS XView library

#CFLAGS =  $(LIBCFLAGS) -I$(XVIEW_INC) -I$(XPRINTER_INC)
CFLAGS =  $(LIBCFLAGS) -I$(XVIEW_INC) 

OBJS =	\
exv_misc.o xecolors.o xerlicons.o xhelp.o xpromptpar.o xbbox.o \
exv_olwm.o

SRCS =	\
exv_misc.c xecolors.c xerlicons.c xhelp.c xpromptpar.c xbbox.c \
exv_olwm.c

$(LIBDIR)/$(EXVLIB):	$(OBJS)
		ar rv $(LIBDIR)/$(EXVLIB) $?; $(RANLIB) $(LIBDIR)/$(EXVLIB)

install:	$(LIBDIR)/$(EXVLIB) 


lint:		$(SRCS)	
		$(LINT) $(LINTFLAGS) $(SRCS) -lm > lint

clean:
		rm -f $(OBJS) $(EXVLIB) 


