#!/bin/sh
# $1 = kill signal
# $2 = window title
# $3 = program name
# $4 = extra args (e.g., channel)

#set -x

menu_file=.wvtmp$$.WM
comm_file=.wvtmp$$.WC
speech_file=sp\$\$.sd
signal=$1
shift
title=$1
shift
program=$1

echo " \"Start Recording\"  shell $* -P -s100 -S $speech_file & echo \$! > w_record_pid" > $menu_file

echo "\"Stop Recording\" shell kill -$signal \`cat w_record_pid\`; rm w_record_pid" >> $menu_file

echo "enable_server" > $comm_file

echo "make_panel name $program file $menu_file icon_title $program title \"$title\" " >> $comm_file

echo "shell rm -f $menu_file $comm_file" >> $comm_file

send_xwaves @$comm_file

# @(#)nvurecord.sh	1.4 7/6/93 ERL






