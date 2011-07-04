# larcbk2rc @(#)Makefile	3.3 11/20/87 ESI
#
CFLAGS =  -O $(PROGCFLAGS)

#
OBJS = larcbk2rc.o

SRCS = larcbk2rc.c

#
PROGNAME =  larcbk2rc

$(PROGNAME): $(SPSLIB) $(OBJS)
	cc  $(CFLAGS) $(OBJS) $(SPSLIB) -lm -o $(PROGNAME)

#
# list .o file dependecies here, $(SINC) is the parent of the
# esps include directories  (remove these two lines)
# eg.
larcbk2rc.o:    $(SINC)/esps.h $(SINC)/sd.h

install: $(PROGNAME)
	-strip $(PROGNAME)
	-mv -f $(BINDIR)/$(PROGNAME) $(OLDBIN)/$(PROGNAME)
	-chmod $(PROGMOD) $(OLDBIN)/$(PROGNAME)
	cp  $(PROGNAME) $(BINDIR)/$(PROGNAME)
	chmod $(PROGMOD) $(BINDIR)/$(PROGNAME)
	-diffmk $(MANDIR)/man1/$(PROGNAME).1 $(PROGNAME).1 man.diff
	@echo diffmked man page left in man.diff
	cp larcbk2rc.1 $(MANDIR)/man1/$(PROGNAME).1
	chmod $(MANMOD) $(MANDIR)/man1/$(PROGNAME).1

clean:	
	-rm -f $(PROGNAME) $(OBJS)
	
lint:	$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) > lint

test:	$(PROGNAME)
	sh larcbk2rc.te
