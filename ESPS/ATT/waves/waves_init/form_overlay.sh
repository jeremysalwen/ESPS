#!/bin/csh -f
# @(#)form_overlay.sh	1.3 9/9/93

set object=$1
set trange=$2":"$3
set input=$4

set source=`epsps -D -lv $input | grep 'Source files:' | sed 's/Source files\://'`

set base=$source:r

set wave="$base"$$

rm -f $wave.sd

copysd -S $trange $source $wave.sd

rm -f $wave.f0 $wave.fb $wave.fb.sig $wave.hp $wave.pole

formant $wave.sd

rm -f $wave.sd

send_xwaves $object overlay file $wave.fb.sig on_file $input













