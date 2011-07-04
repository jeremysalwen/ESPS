#!/bin/sh
# @(#)xframerun.sh	1.1 6/29/93

USE_ESPS_COMMON=no
export USE_ESPS_COMMON

program=$1
field=$2
range=$3
input=$4
output=$5

param=.wvtmp_param$$

exprompt -P PWframe -h frame $param 

# We use mergefea to eliminate tags

frame -P $param $range $input - | $program - - | mergefea -f $field - $output

rm $param






