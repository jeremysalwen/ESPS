#! /bin/sh
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)plotsgram.sh	1.14	1/14/93	ESI/ERL
# 
# Written by:   David Burton
# Checked by:
# Revised by:   Ken Nelson, Alan Parker
# 
# Brief description:  script to plot SPEC files as speech spectrograms.	
#   		      This command file runs image with the appropriate options

USE_ESPS_COMMON=no; export USE_ESPS_COMMON
ESPS_VERBOSE=0; export ESPS_VERBOSE

#
#
# To edit the meaning of hardcopy with the -T option, search for hardcopy
# below.
#

# Check syntax

usage="Usage: plotsgram [-x] [-T device] [image(1-ESPS) options] input.spec"

case $# in
0)
	echo $usage 1>&2
	exit 1 ;;
esac


# initialize variables
#
# Change the following spool program names to be correct for your system
# if lpr is not correct

prgm=plotsgram
device=
IM_SPOOL_CMD=$SPOOL_CMD$	# imagen spool program
PS_SPOOL_CMD=$SPOOL_CMD$	# postscript spool program
HP_SPOOL_CMD=$SPOOL_CMD$	# hp laserjet spool program

image=`get_esps_base`/bin/image

options=
rangeopt=NO
infilename=

# Get command-line options

letter_opt=
for i
do
	case $letter_opt in
	-T)
		device=$i
		letter_opt= ;;
	-[tV])
		options=$options" "\"$i\"
		letter_opt= ;;
	"")
		case $i in
		-x)
			set -x ;;
		-T)
			letter_opt=-T ;;
		-T*)
			device=`echo $i | awk '{print substr($0, 3)}'` ;;
		-[tV])
			options=$options" "$i
			letter_opt=$i ;;
		-t*)
			options=$options" "-t" "\"`echo $i \
					| awk '{print substr($0, 3)}'`\" ;;
		-V*)
			options=$options" "-V" "\"`echo $i \
					| awk '{print substr($0, 3)}'`\" ;;
		-[prs]*)
			rangeopt=YES
			options=$options" "$i ;;
		-)
			infilename=$i
			options=$options" "$i ;;
		-*)
			options=$options" "$i ;;
		*)
			infilename=$i
			options=$options" "$i ;;
		esac ;;
	esac
done

# This is where you can change the meaning of -Thardcopy
# 

if test x$device = xhardcopy
 then
  device=postscript
# device=hp
# device=imagen
fi

# Debug Output

#echo options = $options
#echo infilename = $infilename, T = $device, rangeopt = $rangeopt

# check for silly input

case $letter_opt in
-[TtV])
	echo $prgm: $letter_opt requires argument. 1>&2
	echo $usage 1>&2
	exit 1 ;;
esac

# Check on input file

case $infilename in
""|-?*)
	echo "$prgm: No input file specified." 1>&2
	echo $usage 1>&2
	exit 1 ;;
esac

if test ! -r $infilename 
then
	if test $infilename != "-"
	then
	    echo "$prgm: \"$infilename\" is not a readable file" 1>&2
	    exit 1
	fi
fi

# Invoke image

case $rangeopt in
NO)
	rangeopt=-s: ;;
*)
	rangeopt= ;;
esac

case $device in
"")
	command="$image -G-25:0 $rangeopt -Ls -B0 -t\"Time (seconds)\" \
	      -V\"Freq. (Hz.)\" $options"
	sh -c "$command" 
;;
x11|mcd)
	command="$image -G-25:0 $rangeopt -Ls -B0 -t\"Time (seconds)\" \
	      -V\"Freq. (Hz.)\" -T$device $options"
	sh -c "$command" 
;;
imagen)
	command="$image -G-30:0 $rangeopt -Ls -B0 -t\"Time (seconds)\" \
	    -V\"Freq. (Hz.)\" -T$device $options"
	sh -c "$command" | $IM_SPOOL_CMD 1>/dev/null 
;;
postscript)
	command="$image -G-30:0 $rangeopt -Ls -B0 -t\"Time (seconds)\" \
	    -V\"Freq. (Hz.)\" -T$device $options"
	sh -c "$command" | $PS_SPOOL_CMD 1>/dev/null 
;;
hp)
	command="$image -G-30:0 $rangeopt -Ls -B0 -t\"Time (seconds)\" \
	    -V\"Freq. (Hz.)\" -T$device $options"
	sh -c "$command" | $HP_SPOOL_CMD 1>/dev/null 
;;
ras)
	command="$image -G-30:0 $rangeopt -Ls -B0 -t\"Time (seconds)\" \
	    -V\"Freq. (Hz.)\" -Tras $options"
	sh -c "$command"
;;
*)
	echo "$prgm: Invalid output device type ($device)" 1>&2
	exit 1
;;
esac
