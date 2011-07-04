#!/bin/sh
# @(#)insert_buf.sh	1.2 7/7/93
# $1 = object
# $2 = file name
# $3 = cursor time
# $4 = output file 
# $5 = loc_x
# $6 = loc_y

#set -x

cmd_file=.wvtmp_cmd$$

xloc=`echo $5 45 - p q | dc`   #input is graphic loc not window

echo $1 insert file $2 source wcutbuffer.d time $3 output $4 > $cmd_file
echo kill name $1 file $4.d >> $cmd_file
echo make name $1 file $4.d loc_x $xloc loc_y $6 >> $cmd_file
echo shell rm -f $cmd_file >> $cmd_file

send_xwaves @$cmd_file
