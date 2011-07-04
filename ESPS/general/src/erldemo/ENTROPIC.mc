#! /bin/sh 
# @(#)ENTROPIC.mc	1.2 1/10/91  - starting point of whole demo
#
# this version is a rework of the original Sun demo, with a 
# different slideshow script; the script uses xloadimage 
# rather than screenload and therefore is general for X

cd erldemos
PATH=.:esps/bin:$PATH; export PATH
#put fonts on PATH
xset -fp /usr/lib/X11/fonts,/usr/lib/X11/fonts/75dpi,`pwd`/../xview/fonts/misc,`pwd`/../xview/fonts/75dpi
xset -fp rehash

rm -f /tmp/erldemo.lock
rm -f /tmp/ewaves.lock
rm -f /tmp/eslides.lock
rm -f /tmp/erlwtry.lock




erldemo -m masscomp -S xslideshow.sh -W wavesdemo.sh  -I edemo.info -A edemo.about -t "Entropic Demo"  > /tmp/ENTROPIC.log 2>&1 &
