#!/bin/sh

#Usage: $0 F0_file sample_rate fb_file name
tmp=$HOME/waves/tmp
synfile=$tmp/sig.syn
tmpfile=$tmp/sig.tmp
resfile=$tmp/sig.res
formants=$3.ed.fb
if test ! -s $formants ; then
    formants=$3.fb
fi
send_xwaves kill name $4 file $synfile
if test -s $resfile ; then
    formsy -b 20.05 -h 6 -e $resfile $formants $synfile
else
    formsy -b 20.05 -h 6 $1 $formants $synfile
fi
send_xwaves make name $4 file $synfile loc_y 300 height 150
Mplay $synfile
