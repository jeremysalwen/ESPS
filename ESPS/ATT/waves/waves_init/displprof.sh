#!/bin/sh
# @(#)displprof.sh	1.4 7/6/93

tmp=$HOME/.wvtmp_prof$$

send_xwaves $* output $tmp

sleep 1

xtext -F $tmp &

sleep 10
rm $tmp
