#
# @(#)makefile	1.13	9/9/91	ESI/ERL
# 
# Written by:  
# Checked by:
# Revised by:
# 
# Makefile for:  tpipe
# 


CFLAGS = $(OPT_FLAG)  $(PROGCFLAGS)
LFLAGS =  $(LINTFLAGS)

OBJS = tpipe.o

SRCS = tpipe.c

PROGNAME = tpipe
MANNAME = tpipe.1



$(PROGNAME):	$(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS)  $(SPSLIB)  -lm -o $(PROGNAME)


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

