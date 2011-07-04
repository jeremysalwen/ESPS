#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
#
#  makefile for tpen, plotas
#  @(#)makefile	3.3  12/4/95  ESI 				

CFLAGS = -O $(PROGCFLAGS)


install:	$(LIBDIR)/tpen $(LIBDIR)/plotas

lint:		tpen.lint plotas.lint

TPEN_SRCS = tpen.c geth.c
TPEN_OBJS = tpen.o geth.o

tpen:		$(TPEN_OBJS)
	cc $(CFLAGS) $(TPEN_OBJS) -lm -o tpen

tpen.o:		chars.h $(SINC)/unix.h

tpen.lint:	$(TPEN_SRCS)
	$(LINT) $(LINTFLAGS) $(TPEN_SRCS) $(LINTLIB) > tpen.lint

$(LIBDIR)/tpen:	tpen
	$(STRIP) tpen
	-rm -f $(LIBDIR)/tpen
	mv -f tpen $(LIBDIR)/tpen
	chmod $(PROGMOD) $(LIBDIR)/tpen

PLOTAS_SRCS = plotas.c puth.c
PLOTAS_OBJS = plotas.o puth.o

plotas:		$(PLOTAS_OBJS)
	cc $(CFLAGS) $(PLOTAS_OBJS) -o plotas

plotas.lint:	$(PLOTAS_SRCS)
	$(LINT) $(LINTFLAGS) $(PLOTAS_SRCS) $(LINTLIB) > plotas.lint

$(LIBDIR)/plotas: plotas
	$(STRIP) plotas
	-rm -f $(LIBDIR)/plotas
	mv -f plotas $(LIBDIR)/plotas
	chmod $(PROGMOD) $(LIBDIR)/plotas

clean:
	-rm -f *.o tpen plotas

test:
	@echo "Tpen and plotas tested by test of cover scripts for plot programs."
