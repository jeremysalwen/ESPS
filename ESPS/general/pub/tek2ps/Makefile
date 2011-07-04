# %W% Makefile
CFLAGS= -g $(PROGCFLAGS)

OBJ=tek2ps.o

all: tek2ps


tek2ps: ${OBJ} tek.h
	cc ${CFLAGS} -o $@ ${OBJ} $(SPSLIB)

install: tek2ps
	-rm -f $(BINDIR)/tek2ps
	mv tek2ps $(BINDIR)/tek2ps
	chmod $(PROGMOD) $(BINDIR)/tek2ps

clean:
	rm -f *.o tek2ps
