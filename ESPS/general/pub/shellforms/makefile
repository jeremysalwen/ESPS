#
# @(#)makefile	1.13	9/9/91	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for:  shellforms
# 


CFLAGS = $(OPT_FLAG)  $(PROGCFLAGS)
LFLAGS =  $(LINTFLAGS)


OBJS	= sf.o form.o load.o field.o selection.o keyword.o	\
	  option.o msg.o term.o io.o summary.o
SRCS	= sf.c form.c load.c field.c selection.c keyword.c	\
	  option.c msg.c term.c io.c summary.c

PROGNAME = sf 
MANNAME = sf.1



$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB)  -lm -ltermcap -o $(PROGNAME)


install: $(PROGNAME)
#program
	$(STRIP) $(PROGNAME)
	-mv $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	mv $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
#man pages
	cp $(MANNAME) $(MANDIR)/man1/$(MANNAME)
	chmod $(MANMOD) $(MANDIR)/man1/$(MANNAME)

clean:	
	-rm -f $(PROGNAME) $(OBJS)
	
lint:	$(SRCS)
	$(LINT) $(LFLAGS) $(SRCS) $(LINTLIB) $(WINLIBS) -lm > lint

