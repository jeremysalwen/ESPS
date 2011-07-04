#!/bin/sh
#-------------------------------------------------------------------
#
#  This material contains proprietary software of Entropic Research Lab., Inc. 
#  Any reproduction, distribution, or publication without the the prior    
#  written permission of Entropic Research Lab., Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Research Lab., Inc. must bear the notice                    
#                                                               
#   "Copyright (c) 1996, 1997 Entropic Research Lab, Inc.; All rights reserved"
#                               
#
#  Sccs info: @(#)decrecord.sh	1.7 1/21/97 ERL
#  Written by: David Burton
#
# Brief Descrption: 
#
# This cover script uses the DEC Alpha supplied program
# audiorecord to record data and then converts the
# recorded data to an ESPS file by using SOX (the 
# SOund eXchange utility) and btosps(1-ESPS). 
#
# The audiorecord program cannot write to standard output, so
# we use a temporary file (and remove it when we are done).
# The audio hardware only seems to support sample rates that
# are multiples of 8000 or 11025, but not 24000. 
#
# The record program is not too precise about the number
# of samples it records (1 second at 11025 = 11032 samples),
# and the duration you specify is truncated to an integer.
# So, for example, if you specify a time of 1.9 seconds, you
# only record for 1 second.
#-------------------------------------------------------------------

#
# Turn of ESPS stuff
#

USE_ESPS_COMMON=off 
export USE_ESPS_COMMON
ESPS_VERBOSE=0
export ESPS_VERBOSE

# Set signal trap to exit on interrupt and clean up.
# We need this because the audiorecord program uses
# Cntrl-c to pause the recording, and we don't support this.
# 

Rpid=$$
trap 'kill -INT $Rpid;kill -INT $Rpid;sleep 1' 1 2 3 15

#
# Specify location of DEC audio utilities and SOX
#

DEC_AUDIO_BIN=/usr/bin/mme
DEC_AUDIO_RECORD=audiorecord
if test ! -x $DEC_AUDIO_BIN/$DEC_AUDIO_RECORD
then
	echo decrecord error:
	echo The DEC Alpha audio utilities are no longer in \"$DEC_AUDIO_BIN\",
	echo or the Alpha record program is no longer \"$DEC_AUDIO_RECORD\".
	echo The script decrecord needs to be fixed.
	exit 1
fi
SOX_BIN=$ESPS_BASE/bin


#
# Specify temp file directory
#

if test x$ESPS_TEMP_PATH != x
       	then
        tmpDir=$ESPS_TEMP_PATH
elif test x$TEMP != x
        then
        tmpDir=$TEMP
else
        tmpDir=/usr/tmp
fi


#
# Initialize variables
#

ShowInXwaves=0
Rate=8000
Duration=5
Channels=1
RecordPrompt=0


#
# Process command line arguments
#

while test x$1 != x
do
        case "$1" in
        -s) Duration=$2; shift;;
        -s*) Duration=`echo $1 | awk '{print substr($0, 3)}'` ;;

        -c) Channels=$2; shift;;
        -c*) Channels=`echo $1 | awk '{print substr($0, 3)}'` ;;

        -f) Rate=$2; shift;;
        -f*) Rate=`echo $1 | awk '{print substr($0, 3)}'` ;;

        -P) RecordPrompt=1;;

        -S) ShowInXwaves=1;;

        -?*) echo "decrecord: unknown option $1" ; exit 1 ;;
        *) break;;
        esac
        shift
done

# There should only be a file name left

if test $# -ne 1
        then
        echo \
	'Usage: decrecord [-P] [-S] [-s seconds] [-c channels] [-f rate] filename'
        exit 1
fi


#
# Set output file name
#

Outfile=$1


#
# Is output file writable?
# Create file, if it doesn't exist. If this fails, or
# the file exists and is not writable, warn and exit.
#

touch  $Outfile 2> /dev/null 
if test ! $? -eq 0
	then
	echo\
    "decrecord: Could not create output file ($Outfile) - check permissions.\n"
	exit 1
fi

if test ! -w $Outfile
	then
	echo\
    "decrecord: Could not create output file ($Outfile) - check permissions.\n"
	exit 1
fi


#
# Is number of channels valid?
#

case $Channels in
	[1-2])
		# things OK
		;;
	*) 	
		echo \
		  "decrecord: Invalid number of channels specified: $Channels."
		echo "           Only 1 or 2 channels are supported.\n";
		exit 1;;

esac


#
# Is sample rate valid?
#

case $Rate in
	
	# Following rates OK
	8000) 		;;
	11025)		;;
	16000)		;;
	22050)		;;
	32000)		;;
	33075)		;;
	44100)		;;
	48000)		;;

	24000)
		echo "decrecord: Invalid sample rate specified: $Rate."
		echo "     Try a multiple of 8000 or 11025 (but not 24000).\n";
		exit 1;;
		

	*)
		echo "decrecord: Invalid sample rate specified: $Rate."
		echo "     Try a multiple of 8000 or 11025 (but not 24000).\n";
		exit 1;;		
esac


#
# Is duration valid?
#

if test $Duration -lt 1
	then
	echo "decrecord: Invalid record duration specified: $Duration"
	echo "           Duration must be an integer and >= 1"
	exit 1
fi


#
# Record data and convert to an an ESPS file.
# We put audiorecord in background and save the process ID, so we can
# kill it on clean up, if the user interrupts.
#

$DEC_AUDIO_BIN/$DEC_AUDIO_RECORD -encoding pcm -bitspersample 16 -rate $Rate\
	-channels $Channels -time $Duration \
	-filename $tmpDir/drecord$$ >/dev/null &

#
# save process ID 
#

Rpid=$!

#
# Prompt after record begins to allow hard a chance to initialize
#

if test $RecordPrompt -eq 1
	then
	echo "BEGIN RECORDING NOW..."
fi

#
#  wait for record to finish
#

wait $Rpid

if test $RecordPrompt -eq 1
	then
	echo "RECORDING DONE"
fi


# Convert to ESPS file

$SOX_BIN/sox -t wav $tmpDir/drecord$$ -t raw - | $ESPS_BASE/bin/btosps \
	-f $Rate -n $Channels -t short \
	-c"Converted via $DEC_AUDIO_RECORD and SOX."\
	 - $Outfile


#
# clean up
#

rm -f $tmpDir/drecord$$ &


# If display in xwaves is specified, give it a try.
# Note that we do not do any checking to see if xwaves
# already exists. We use the send_xwaves error output
# to inform the user about the unavailability of xwaves.

if test $ShowInXwaves -eq 1
	then
	# if full path given  (check for leading "/")
	# then
	#	use it
	# else
	# add path so that xwaves can find file

	if echo $Outfile | grep -q \^/
	then 
		send_xwaves -D 0 make file $Outfile
	else
		send_xwaves -D 0 make file `pwd`/$Outfile
	fi
fi



