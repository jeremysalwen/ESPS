#!/bin/sh -xv

# Usage: $0 input.wav name

size=0.020
step=0.005
width=1000
case $# in
    2)
if test -s $1 ; then
tmp=$HOME/waves/tmp

if test ! -d $tmp ; then
    echo The directory $tmp, and possibly its parent, does not exist.
    echo Create these, and then try again.
    exit
fi

infile=$tmp/sig.wav
rm -f $infile
copysd $1 $infile
fbfile=$tmp/sig.fb
hpfile=$tmp/sig.hp
polefile=$tmp/sig.pole
f0file=$tmp/sig.f0
f0edfile=$tmp/sig.ed.f0
dsfile=$tmp/sig.ds
synfile=$tmp/sig.syn
simp=$tmp/sig
rm -f $fbfile $f0file $f0edfile $dsfile $synfile $polefile $hpfile $simp.ed.fb
sgram=$dsfile.fspec
send_xwaves kill name $2 file $f0edfile
send_xwaves kill name $2 file $sgram
send_xwaves kill name $2 file $dsfile
rate=`hditem -i record_freq $infile`
order=`echo $rate 1000.0 / 2 + p q | dc`
nstep=`echo $rate $step \* p q | dc`
nwind=`echo $rate $size \* p q | dc`

$ESPS_BASE/bin/formant -i $step -p .7 $infile
#/Users/davidt/src/signal/bin/formant -i $step -p .7 $infile
get_f0 -i $step $infile $f0file
send_xwaves make name $2 file $dsfile loc_y 1 height 120  width $width
send_xwaves make name $2 file $f0file loc_y 40 height 300 width $width
send_xwaves $2 spectrogram file $dsfile loc_y 350 height 400 width $width
send_xwaves $2 overlay file $fbfile
send_xwaves $2 activate file $fbfile op set numbers 0 1 2 3
send_xwaves $2 set file $sgram left_op mark formants
send_xwaves set srate $rate fbfile $simp
send_xwaves $2 set  file $f0file left_op modify signal

fi
;;
esac
