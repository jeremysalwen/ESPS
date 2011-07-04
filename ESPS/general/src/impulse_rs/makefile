#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
# 				
#
# makefile for impulse_resp @(#)makefile	3.3 11/24/92 ESI
 	
CFLAGS = -O $(PROGCFLAGS)
LIBFLAGS = $(SPSLIB) -lm 

OBJS = impulse_rs.o 
SRCS = impulse_rs.c

impulse_resp:	$(OBJS) $(SPSLIB)
	cc $(CFLAGS) $(OBJS) $(LIBFLAGS)  -o impulse_resp

impulse_resp.o: $(SINC)/esps.h $(SINC)/unix.h

install:	impulse_resp
	$(STRIP) impulse_resp
	-mv -f $(BINDIR)/impulse_resp $(OLDBIN)/impulse_resp
	-chmod $(PROGMOD) $(OLDBIN)/impulse_resp
	mv -f impulse_resp $(BINDIR)/impulse_resp
	chmod $(PROGMOD) $(BINDIR)/impulse_resp
	-diffmk $(MANDIR)/man1/impulseres.1 impulseres.1 man.diff
	@echo "diffmked man page left in man.diff"
	cp impulseres.1 $(MANDIR)/man1/impulseres.1
	chmod $(MANMOD) $(MANDIR)/man1/impulseres.1

clean:
	rm -f $(OBJS) impulse_resp

lint:		$(SRCS)
	$(LINT) $(LINTFLAGS) $(SRCS) $(LINTLIB) -lm > lint

test:		impulse_resp
	-sh testfile
