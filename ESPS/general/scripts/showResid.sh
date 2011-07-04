#!/bin/sh

# Usage: $0 input.wav name

size=0.020
step=0.005

case $# in
    2)
if test -s $1 ; then
tmp=$HOME/waves/tmp

if test ! -d $tmp ; then
    echo The directory $tmp, and possibly its parent, does not exist.
    echo Create these, and then try again.
fi

tmpin=$tmp/sigin.wav
rcfile=$tmp/sig.rc
resfile=$tmp/sig.res
send_xwaves kill name $2 file $resfile
cp $1 $tmpin
rate=`hditem -i sample_rate $tmpin`
order=`echo $rate 1000.0 / 2 + p q | dc`
nstep=`echo $rate $step \* p q | dc`
nwind=`echo $rate $size \* p q | dc`
echo $rate $order $nstep $nwind

refcof  -m AUTOC -r 1:10000000 -z -o $order  -l $nwind -e .9999 -S $nstep -w HANNING $tmpin $rcfile
get_resid $tmpin $rcfile $resfile
send_xwaves make name $2 file $resfile loc_y 300 height 300

fi
;;
esac
