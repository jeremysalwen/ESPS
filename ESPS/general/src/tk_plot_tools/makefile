#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#   "Copyright (c) 1997 Entropic Research Lab. All rights reserved."
#
#
#  makefile for tk plot programs
#  %W% %G% ERL
 	
CFLAGS = -O $(PROGCFLAGS)
PLOTTCL = plot.tcl
PLOTHELP = plot.help

all:	aplot plotspec plotsd scatplot

PROGNAME1 = plotspec
OBJS1 = plotspec.o drawbox.o plot2data.o plotphdata.o \
	printtime.o plotscale.o plotexscal.o tk.o
SRCS1 = plotspec.c drawbox.c plot2data.c plotphdata.c \
	printtime.c plotscale.c plotexscal.c tk.c
MANSRC1 = plotspec.1

PROGNAME2 = plotsd
OBJS2 = plotsd.o drawbox.o plotdataf.o plotscale.o printtime.o plotexscal.o \
        tk.o
SRCS2 = plotsd.c drawbox.c plotdataf.c plotscale.c printtime.c plotexscal.c \
        tk.c
MANSRC2 = plotsd.1

PROGNAME3 = aplot
OBJS3 = aplot.o  tk.o  printtime.o
SRCS3 = aplot.c  tk.c  printtime.c
MANSRC3 = aplot.1

PROGNAME4 = scatplot
OBJS4 = scatplot.o plotletter.o draw_box2.o plotscale.o printtime.o \
	plotexscal.o tk.o
SRCS4 = scatplot.c plotletter.c draw_box2.c plotscale.c printtime.c \
	plotexscal.c tk.c
MANSRC4 = scatplot.1


$(PROGNAME1):	$(SPSLIB) $(OBJS1)
	cc $(LINK_OPTS) $(CFLAGS) $(OBJS1) $(SPSLIB) $(TK_LIB) $(X_LIB) -lm -o $(PROGNAME1)

$(PROGNAME2):	$(SPSLIB) $(OBJS2)
	cc $(LINK_OPTS) $(CFLAGS) $(OBJS2) $(SPSLIB) $(TK_LIB) $(X_LIB) -lm -o $(PROGNAME2)

$(PROGNAME3):	$(SPSLIB) $(OBJS3)
	cc $(LINK_OPTS) $(CFLAGS) $(OBJS3) $(SPSLIB) $(TK_LIB) $(X_LIB) -lm -o $(PROGNAME3)

$(PROGNAME4):	$(SPSLIB) $(OBJS4)
	cc $(LINK_OPTS) $(CFLAGS) $(OBJS4) $(SPSLIB) $(TK_LIB) $(X_LIB) -lm -o $(PROGNAME4)

plotsd.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/feasd.h
plotspec.o:	$(SINC)/esps.h $(SINC)/unix.h $(SINC)/spec.h $(SINC)/feaspec.h
aplot.o:	$(SINC)/esps.h $(SINC)/unix.h
scatplot.o:	$(SINC)/esps.h $(SINC)/unix.h
drawbox.o:	$(SINC)/esps.h $(SINC)/unix.h

tk.o:		
		cc -c $(CFLAGS) -I$(X_INC) -I$(TK_INC) tk.c

install:	$(PROGNAME1) $(PROGNAME2) $(PROGNAME3) $(PROGNAME4)
	$(STRIP) $(PROGNAME1) $(PROGNAME2) $(PROGNAME3) $(PROGNAME4)
	-rm -f $(BINDIR)/$(PROGNAME1)
	-rm -f $(BINDIR)/$(PROGNAME2)
	-rm -f $(BINDIR)/$(PROGNAME3)
	-rm -f $(BINDIR)/$(PROGNAME4)
	mv $(PROGNAME1) $(BINDIR)/$(PROGNAME1)
	mv $(PROGNAME2) $(BINDIR)/$(PROGNAME2)
	mv $(PROGNAME3) $(BINDIR)/$(PROGNAME3)
	mv $(PROGNAME4) $(BINDIR)/$(PROGNAME4)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME1)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME2)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME3)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME4)
	rm -f $(MANDIR)/man1/$(MANSRC1)
	rm -f $(MANDIR)/man1/$(MANSRC2)
	rm -f $(MANDIR)/man1/$(MANSRC3)
	rm -f $(MANDIR)/man1/$(MANSRC4)
	cp $(MANSRC1) $(MANDIR)/man1/$(MANSRC1)
	cp $(MANSRC2) $(MANDIR)/man1/$(MANSRC2)
	cp $(MANSRC3) $(MANDIR)/man1/$(MANSRC3)
	cp $(MANSRC4) $(MANDIR)/man1/$(MANSRC4)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANSRC1)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANSRC2)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANSRC3)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANSRC4)
	rm -f $(PLOTDIR)/$(PLOTTCL)
	cp $(PLOTTCL) $(PLOTDIR)/$(PLOTTCL)
	chmod $(LIBMOD)  $(PLOTDIR)/$(PLOTTCL)
	rm -f $(LIBDIR)/$(PLOTHELP)
	cp $(PLOTHELP) $(LIBDIR)/$(PLOTHELP)
	chmod $(LIBMOD)   $(LIBDIR)/$(PLOTHELP)
clean:
	rm -f $(OBJS1) $(PROGNAME1) $(OBJS2) $(PROGNAME2) \
	$(OBJS3) $(PROGNAME3) $(OBJS4) $(PROGNAME4) 

lint:		$(SRCS1) $(SRCS2) $(SRCS3) $(SRCS4)
	$(LINT) $(LINTFLAGS) -I$(TK_INC) $(SRCS1) $(LINTLIB) -lm > lint.$(PROGNAME1)
	$(LINT) $(LINTFLAGS) -I$(TK_INC) $(SRCS2) $(LINTLIB) -lm > lint.$(PROGNAME2)
	$(LINT) $(LINTFLAGS) -I$(TK_INC) $(SRCS3) $(LINTLIB) -lm > lint.$(PROGNAME3)
	$(LINT) $(LINTFLAGS) -I$(TK_INC) $(SRCS4) $(LINTLIB) -lm > lint.$(PROGNAME4)

test:
	@echo "Plotspec, plotsd, scatplot, and aplot tested by test of cover script."
