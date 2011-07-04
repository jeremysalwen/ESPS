#!/bin/sh
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)xtext.sh	1.1	1/29/92	ERL
# 
# Written by:  Alan Parker
# Checked by:
# Revised by:  Ken Nelson
# 
# Brief description: a shell script replacement for xtext
# Used when xtext won't compile, or is buggy.
# 

# set -- `getopt W:Ft:i: $*`
# if test $? != 0
# then
# echo error
# exit 2
# fi


echo $*

for i in $*
do
	case $i in
	-Wp) geo=-geo" "+$2+$3; shift 3;;
	-F) filename=$2; shift 2;;
	-t) title=-T" \""$2"\""; shift 2;;
	-i) icontitle=-n" \""$2"\""; shift 2;;
	esac
done

if test x$filename != x
 then
  command=xterm" "$geo" "$title" "$icontitle" "-e" "more" "/tmp/xt$$
  cat $filename $ESPS_BASE/lib/blanks  > /tmp/xt$$
else
 if test x$1 = x
  then
   echo missing command
  else
  $* > /tmp/xt$$
  cat /tmp/xt$$ $ESPS_BASE/lib/blanks  > /tmp/xta$$
  command=xterm" "$geo" "$title" "$icontitle" "-e" more "/tmp/xta$$
 fi
fi
echo $command
echo $command | sh -s
rm -f /tmp/xt$$ /tmp/xta$$
