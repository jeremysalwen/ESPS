# ENTROPIC.sld @(#)ENTROPIC.sld	1.1 9/11/90 - starting point of whole demo
# this version assumes Sun OpenWindows is installed

rm -f /tmp/erldemo.lock
rm -f /tmp/erldemo.stop
rm -f /tmp/erlplay.lock

erldemo -S erld.s.slide -C erld.c.slide -W erldemo.wtry -I edemo.info -A edemo.about -t "Entropic Slide Demo" &



