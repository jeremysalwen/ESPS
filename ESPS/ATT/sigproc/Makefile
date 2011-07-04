#
#
# Makefile for waves+ sigproc directory
#
# @(#)Makefile	1.4 12/16/92 ESI



CFLAGS = -O3  $(PROGCFLAGS)
WAVESDIR = ../lib
LIBMOD = 0755

LIB_OBJS = dreflpc.o dlpcref.o dplympy.o dlpcwtd.o dcwmtrx.o dchlsky.o \
	   dlwrtrn.o  dcovlpc.o dfbwsos.o dsoslpc.o lpcfloat.o

libdsigproc.a:	$(LIB_OBJS)
	ar rv $@ $?
	$(RANLIB) $@

install: $(WAVESDIR)/libdsigproc.a

$(WAVESDIR)/libdsigproc.a: libdsigproc.a
	-rm -f $@
	cp $? $@
	chmod $(LIBMOD) $@
	$(RANLIB) $@

clean:
	rm -f $(LIB_OBJS) libdsigproc.a
