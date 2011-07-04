# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# %W% %G% ERL
# 
# Makefile for: echeckout efree freewaves
#
# Written by:  Alan Parker
# Checked by:
# Revised by:
 	
ELMDIR = $(LIBDIR)/elm
 	
CFLAGS = -I$(ELMDIR) -g $(PROGCFLAGS)
CC = $(ANSI_CC)


all:	echeckout efree hcheckout hfree holdalign relalign testalign \
	check_daemon

aligner: holdalign relalign  check_daemon

aligner_install: aligner
	mv holdalign relalign  check_daemon $(ALIGNER_DIR)/bin

echeckout:	echeckout.o   getespsbas.o getstatfile.o $(ELMDIR)/libelm.a ss.o
	$(CC) $(CFLAGS) echeckout.o  getespsbas.o getstatfile.o $(ELMDIR)/libelm.a ss.o \
		 $(BSDLIB) -o echeckout
	$(STRIP) echeckout

pure_echeckout:	echeckout.o   getespsbas.o getstatfile.o $(ELMDIR)/libelm.a ss.o
	purify $(CC) $(CFLAGS) echeckout.o  getespsbas.o getstatfile.o $(ELMDIR)/libelm.a ss.o \
		 $(BSDLIB) -o echeckout


holdalign:	holdalign.o    $(ELMDIR)/libelm.a 
	$(CC) $(CFLAGS) holdalign.o   $(ELMDIR)/libelm.a \
		 $(BSDLIB) -o holdalign
	$(STRIP) holdalign

check_daemon: check_daemon.o $(ELMDIR)/libelm.a
	$(CC) $(CFLAGS) check_daemon.o $(ELMDIR)/libelm.a \
		 $(BSDLIB) -o check_daemon
	$(STRIP) check_daemon

testalign:	testalign.o    $(SPSLIB)
	$(CC) $(CFLAGS) testalign.o   $(SPSLIB) $(BSDLIB) -o testalign
	$(STRIP) testalign


relalign:	relalign.o    $(ELMDIR)/libelm.a 
	$(CC) $(CFLAGS) relalign.o   $(ELMDIR)/libelm.a \
		 $(BSDLIB) -o relalign
	$(STRIP) relalign

hcheckout:	hcheckout.o   getespsbas.o getstatfile.o $(ELMDIR)/libelm.a ss.o
	$(ELM_CC) $(CFLAGS) hcheckout.o  getespsbas.o getstatfile.o $(ELMDIR)/libelm.a ss.o \
		 $(SOCKET) -o hcheckout
	$(STRIP) hcheckout

freewaves:	freewaves.o   getespsbas.o $(ELMDIR)/libelm.a 
	$(CC) $(CFLAGS) freewaves.o  getespsbas.o $(ELMDIR)/libelm.a  \
	$(BSDLIB) 	 -o freewaves
	$(STRIP) freewaves

hfree:	hfree.o   getespsbas.o $(ELMDIR)/libelm.a 
	$(ELM_CC) $(CFLAGS) hfree.o  getespsbas.o $(ELMDIR)/libelm.a  \
	$(SOCKET) 	 -o hfree
	$(STRIP) hfree

hfree.o: freewaves.c
	$(ELM_CC) -c -o hfree.o -DHTK $(CFLAGS) freewaves.c

efree:	getstatfile.o efree.c
	$(ELM_CC) -DESPS $(CFLAGS) efree.c getstatfile.o -o efree

headers.o:
	$(ELM_CC) -c $(CFLAGS) ../lib_header/headers.c

ss.o:
	$(CC) -c $(CFLAGS) ../lib/ss.c

echeckout.o: $(SINC)/header.h $(SINC)/esps.h 
	$(ELM_CC) -c -DESPS $(CFLAGS) echeckout.c

holdalign.o: $(SINC)/header.h $(SINC)/esps.h 
	$(CC) -c -DALIGNER $(CFLAGS) holdalign.c

check_daemon.o: $(SINC)/header.h $(SINC)/esps.h 
	$(CC) -c -DALIGNER $(CFLAGS) check_daemon.c

testalign.o: $(SINC)/header.h $(SINC)/esps.h 
	$(CC) -c -DALIGNER $(CFLAGS) testalign.c

relalign.o: $(SINC)/header.h $(SINC)/esps.h 
	$(CC) -c -DALIGNER $(CFLAGS) relalign.c

hcheckout.o: $(SINC)/header.h $(SINC)/esps.h echeckout.c
	$(ELM_CC) -c -o hcheckout.o -DHTK $(CFLAGS) echeckout.c

freewaves.o: $(SINC)/header.h $(SINC)/esps.h  freewaves.c
	$(ELM_CC) -c -DWAVES $(CFLAGS) freewaves.c

getespsbas.c:
	ln ../lib/getespsbas.c .

clean:
	-rm -f *.o echeckout  efree getespsbas.c testalign hfree hcheckout 
	-rm -f holdalign relalign

install:	$(BINDIR)/echeckout $(MANDIR)/man1/echeckout.1 \
		$(BINDIR)/efree $(MANDIR)/man1/efree.1 \
		$(BINDIR)/freewaves $(MANDIR)/man1/freewaves.1 \
		$(MANDIR)/man1/echeckin.1 \
		$(BINDIR)/echeckout51 \
		$(BINDIR)/efree51 

install_echeckout: $(BINDIR)/echeckout $(MANDIR)/man1/echeckout.1 \
		$(BINDIR)/echeckout51 
		
lint:	echeckout.c  
	$(LINT) $(LINTFLAGS) echeckout.c  $(LINTLIB) > lint

$(BINDIR)/echeckout51: echeckout
	-rm -f $(BINDIR)/echeckout51
	mv echeckout $(BINDIR)/echeckout51
	-$(STRIP) $(BINDIR)/echeckout51
	chmod $(PROGMOD) $(BINDIR)/echeckout51

$(MANDIR)/man1/echeckout.1: echeckout.1
	-diffmk  $(MANDIR)/man1/echeckout.1 echeckout.1 man.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/echeckout.1
	cp echeckout.1 $(MANDIR)/man1/echeckout.1
	chmod $(MANMOD) $(MANDIR)/man1/echeckout.1

$(BINDIR)/echeckout: echeckout.sh
	-rm -f $(BINDIR)/echeckout
	cp echeckout.sh $(BINDIR)/echeckout
	chmod $(PROGMOD) $(BINDIR)/echeckout

$(BINDIR)/efree: efree.sh
	-rm -f $(BINDIR)/efree
	cp efree.sh $(BINDIR)/efree
	chmod $(PROGMOD) $(BINDIR)/efree

$(BINDIR)/freewaves: freewaves
	-rm -f $(BINDIR)/freewaves
	mv freewaves $(BINDIR)/freewaves
	-$(STRIP) $(BINDIR)/freewaves
	chmod $(PROGMOD) $(BINDIR)/freewaves

$(MANDIR)/man1/freewaves.1: freewaves.1
	-diffmk  $(MANDIR)/man1/freewaves.1 freewaves.1 man.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/freewaves.1
	cp freewaves.1 $(MANDIR)/man1/freewaves.1
	chmod $(MANMOD) $(MANDIR)/man1/freewaves.1

$(BINDIR)/hfree: hfree
	-rm -f $(BINDIR)/hfree
	mv hfree $(BINDIR)/hfree
	-$(STRIP) $(BINDIR)/hfree
	chmod $(PROGMOD) $(BINDIR)/hfree

$(MANDIR)/man1/hfree.1: hfree.1
	-rm -f $(MANDIR)/man1/hfree.1
	cp hfree.1 $(MANDIR)/man1/hfree.1
	chmod $(MANMOD) $(MANDIR)/man1/hfree.1

$(BINDIR)/efree51: efree
	-rm -f $(BINDIR)/efree51
	mv efree $(BINDIR)/efree51
	-$(STRIP) $(BINDIR)/efree51
	chmod $(PROGMOD) $(BINDIR)/efree51

$(MANDIR)/man1/efree.1: efree.1
	-diffmk  $(MANDIR)/man1/efree.1 efree.1 man.diff
	@echo diffmked man in man.diff
	-rm -f $(MANDIR)/man1/efree.1
	cp efree.1 $(MANDIR)/man1/efree.1
	chmod $(MANMOD) $(MANDIR)/man1/efree.1

$(MANDIR)/man1/echeckin.1: efree.1
	cp efree.1 $(MANDIR)/man1/echeckin.1
	chmod $(MANMOD) $(MANDIR)/man1/echeckin.1

test:	
	-echo sorry no test yet for echeckout
