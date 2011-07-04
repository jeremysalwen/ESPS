# Makefile for waves+ libsig directory for Sunview version of waves+
#
#
# @(#)sunview.mk	1.2 1/4/93 ESI, ATT



INC = ../waves/src/h
CFLAGS = -g -I$(INC) -DHEAD_STANDALONE -DFOR_SUNVIEW $(PROGCFLAGS)
LPATH = $(SPSDIR)
LBIN = $(BINDIR)
LIB = $(WAVESDIR)/libsig.a
# The following is for test purposes only
# LIB = ./libsig.a
DSP32C_BIN = $(SPSDIR)/32cbin
DSP32_BIN = $(SPSDIR)/32bin

LIB_OBJS =  read_data.o parse.o header.o convert_data.o write_data.o \
	signal.o environment.o copheader.o

SUTILS = hget addhead rmhead convert scale


libsig.a: $(LIB_OBJS)
	ar rv $@   $?
	ranlib $@

install: libsig.a
	-rm -f $(WAVESDIR)/libsig.a
	cp libsig.a $(WAVESDIR)
	chmod $(LIBMOD) $(WAVESDIR)/libsig.a
	ranlib $(WAVESDIR)/libsig.a

sutils: $(SUTILS)

all:	$(LIB) $(SUTILS)

clean: 
	-rm -f *.o $(SUTILS) 

clobber:
	-rm -f libsig.a

environment.o: environment.c
	cc -c ${CFLAGS}  -DDPATH=\"${LPATH}\" -DLBIN=\"${LBIN}\" \
	 -DDSP32_BIN=\"${DSP32_BIN}\" -DDSP32C_BIN=\"${DSP32C_BIN}\" \
	environment.c

scale: scale.o $(LIB) $(SPSLIB)
	rm -f $@
	cc -o scale ${CFLAGS} scale.o $(LIB) $(SPSLIB)

hget: hget.o  $(LIB) $(SPSLIB)
	rm -f $@
	cc -o hget ${CFLAGS} hget.o   $(LIB) $(SPSLIB)

rmhead: rmhead.o $(LIB) $(SPSLIB)
	rm -f $@
	cc -o rmhead ${CFLAGS} rmhead.o $(LIB) $(SPSLIB)

addhead: addhead.o $(LIB) $(SPSLIB)
	rm -f $@
	cc -o addhead ${CFLAGS} addhead.o $(LIB) $(SPSLIB)

convert: convert.o $(LIB) $(SPSLIB)
	rm -f $@
	cc -o convert ${CFLAGS} convert.o $(LIB) $(SPSLIB)

doc: ../../text/*.help.src
	cd ../../text; make_doc `pwd | sed 's/\/text//'`


