#!/bin/csh -f -x
# @(#)formant.sh	1.3 9/9/93

set object=$1
set wanted=$2
set range=$3
set input=$4

set base=$input:r

echo $base

#rm -f $base.f0 $base.fb $base.fb.sig $base.hp $base.pole

formant $range $input

send_xwaves make name $object file $base.$wanted 











