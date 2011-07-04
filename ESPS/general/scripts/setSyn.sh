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
f0file=$tmp/sig.f0
f0edfile=$tmp/sig.ed.f0
synfile=$tmp/sig.syn
# ch_wave -scale .4 -o $tmpin $1
copysd -s .4 $1 $tmpin
send_xwaves kill name $2 file $f0edfile
rate=`hditem -i record_freq $tmpin`
order=`echo $rate 1000.0 / 2 + p q | dc`
nstep=`echo $rate $step \* p q | dc`
nwind=`echo $rate $size \* p q | dc`
echo $rate $order $nstep $nwind

refcof  -m AUTOC -r 1:10000000 -z -o $order -l $nwind -e .97 -S $nstep -w HANNING $tmpin $rcfile
get_f0 -i $step $tmpin $f0file
send_xwaves make name $2 file $f0file loc_y 40 height 350
send_xwaves set srate $rate rcfile $rcfile
send_xwaves $2 set  file $f0file left_op modify signal

fi
;;
esac
