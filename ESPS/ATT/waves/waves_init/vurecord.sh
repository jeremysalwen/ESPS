#!/bin/sh
# @(#)vurecord.sh	1.6 8/4/93
#arguments should be the name of the record program and the channel option and
#rat options if any.  
#set -x

file=sp$$.sd

$* -s10000 | meter $file 2>/tmp/vu$$
fgrep -v Virt /tmp/vu$$ | fgrep -v Warn
rm -f /tmp/vu$$
send_xwaves make file $file


