#!/bin/sh
# @(#)wacf.sh	1.1 6/29/93

range=$1
input=$2
output=$3

param=.wvtmp_param$$

xacf $param

acf -P$param $range $input $output

rm $param

