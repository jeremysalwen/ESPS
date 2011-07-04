s;^BINDIR = .*$;BINDIR = /usr/esps/bin;
s;^OLDBIN = .*$;OLDBIN = /usr/esps/bin/old;
s;^MANDIR = .*$;MANDIR = /usr/esps/man;
s;^SPSLIB = .*$;SPSLIB = /usr/lib/libesps.a\
LIBES = -lesps;
s;-lsps;$(LIBES);g
