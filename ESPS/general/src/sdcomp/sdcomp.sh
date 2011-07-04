#! /bin/sh
#-----------------------------------------------------------------------+
#									|
#   This material contains proprietary software of Entropic Speech,	|
#   Inc.  Any reproduction, distribution, or publication without the	|
#   prior written permission of Entropic Speech, Inc. is strictly	|
#   prohibited.  Any public distribution of copies of this work		|
#   authorized in writing by Entropic Speech, Inc. must bear the	|
#   notice								|
#									|
#   "Copyright (c) 1986, 1987, 1989 Entropic Speech, Inc.               |
#   All rights reserved."	                                        |
#									|
#-----------------------------------------------------------------------+
#									|
#  sdcomp -- compare sampled-data files by listening			|
#									|
#  Rodney W. Johnson, Entropic Speech, Inc.				|
#									|
#  This program cycles through one or more ESPS SD files playing a	|
#  portion from each file by means of the local "play" program (e.g.	|
#  mcplay on the MASSCOMP).  It assumes that the local play program	|
#  has the same command line options as mcplay.  After sdcomp starts	|
#  and after a portion is played from each file, the user is prompted	|
#  for the next action.  The following commands are then accepted	|
#  (<CR> refers to the "return" key):					|
#									|
#  <CR>     play next portion from each file				|
#  r<CR>    repeat current portion from each file			|
#  b<CR>    back up:  play previous portion from each file		|
#  c<CR>    continue playing portions without stopping for		|
#           commands							|
#  q<CR>    quit							|
#									|
#-----------------------------------------------------------------------+
# Sccs information:  @(#)sdcomp.sh	3.2 12/13/89 ESI

# Name of local "play" program
play=play

usage="Usage: sdcomp [-b shift-value] [-c channel] [-d delay]\n\
\t[-{fprs} range] [-g gain] [-i] [-R nrepeat] [-n step] [-w width]\n\
\t[-x debug-level] file.sd [file.sd ...]"
xoption=
goption=
woption=
roption=
coption=
ioption=
boption=
# Default range is -s0:+1
rangeswitch="-s"
range="0:+1"
start=
incr=
delay=
step=
# "c" command not yet seen.
continuing=no

# disable ESPS common
USE_ESPS_COMMON=off
export USE_ESPS_COMMON

# Get command-line options.

while :
do
    case $1 in
    -b)
        boption="$1$2"
        shift 2 ;;
    -b*)
        boption=$1
        shift ;;
    -c)
        coption="$1$2"
        shift 2 ;;
    -c*)
        coption=$1
        shift ;;
    -d)
        delay=$2
        shift 2 ;;
    -d*)
        delay=`echo $1 | awk '{print substr($0, 3)}'`
        shift ;;
    -[fps])
        rangeswitch=$1
        range=$2
        shift 2 ;;
    -[fprs]*)
        rangeswitch=`echo $1 | awk '{print substr($0, 1, 2)}'`
        range=`echo $1 | awk '{print substr($0, 3)}'`
        shift ;;
    -g)
        goption="$1$2"
        shift 2 ;;
    -g*)
        goption=$1
        shift ;;
    -i)
        ioption="$1"
        shift ;;
    -n)
        step=$2
        shift 2 ;;
    -n*)
        step=`echo $1 | awk '{print substr($0, 3)}'`
        shift ;;
    -R)
        roption="$1$2"
        shift 2 ;;
    -R*)
        roption=$1
        shift ;;
    -w)
        woption="$1$2"
        shift 2 ;;
    -w*)
        woption=$1
        shift ;;
    -x)
        xoption="$1$2"
        shift 2 ;;
    -x*)
        xoption=$1
        shift ;;
    -?*)
        echo $usage
        exit 1 ;;
    *)
        break ;;
    esac
done

# What's left must be filenames.  Require at least 1.

case $# in
0)
    echo $usage
    exit 1 ;;
esac

# Starting location is the part of the range specification
# before the ":" if any.  Defaults are 0 sec, point 1, and
# frame 1.

case $range in
""|:*)
    case $rangeswitch in
    -s)
        start=0 ;;
    *)
        start=1 ;;
    esac ;;
*:*)
    start=`echo $range | awk -F: '{print $1}'` ;;
*)
    start=$range ;;
esac

# The portion length is (incr) if the range is given in sec,
# (incr+1) if points or frames.  The incr is as given if the
# range has the form <start>:+<incr> and is end-start if the
# range has the form <start>:<end>; defaults are 7999 points,
# 1 sec, and 79 frames.  These give equivalent portion lengths
# when the sampling frequency is 8000 Hz and the frame size is
# 100.

case $range in
""|*:)
    case $rangeswitch in
    -r)
        incr=7999 ;;
    -p)
        incr=7999 ;;
    -s)
        incr=1 ;;
    -f)
        incr=79 ;;
    esac ;;
*:+*)
    incr=`echo $range | awk -F: '{print 0 + $2}'` ;;
*:*)
    incr=`echo $range | awk -F: '{print $2 - '$start'}'` ;;
*)
    incr=0 ;;
esac

echo Start: $start
echo Incr: $incr

# Default step size is the portion length.

case $step in
"")
    case $rangeswitch in
    -s)
        step=$incr ;;
    *)
        step=`expr $incr + 1` ;;
    esac ;;
esac

# Finally play some speech.

while
    echo $rangeswitch$start:+$incr
    case $delay in
    "")
        if $play $boption $coption $goption $ioption $roption \
            $rangeswitch$start:+$incr $woption $xoption $* 
	 then
	  continuing=$continuing
	 else
	  continuing=no
	fi
	;;
    *)
        for i in $*
        do
         if   $play $boption $coption $goption $ioption $roption \
                $rangeswitch$start:+$incr $woption $xoption $i \
            && sleep $delay
	 then
	  continuing=$continuing
	 else
	  continuing=no
	fi
        done ;;
    esac
do
    case $continuing in
    no)
        echo "sdcomp: \c"
        read command
        while :
        do
            case $command in
                "")
		    start=`echo $start $step | awk '{print $1 + $2}'`
                    break ;;
                r)
                    break ;;
                b)
		    start=`echo $start $step | awk '{print $1 - $2}'`
                    break ;;
                q)
                    exit 0
                    break ;;
                c)
                    continuing=yes
		    start=`echo $start $step | awk '{print $1 + $2}'`
                    break ;;
                *)
                    echo \
"sdcomp commands:  empty line plays next portion; \"r\" repeats;"
                    echo \
"         \"b\" backs up; \"q\" quits; \"c\" continues to end."
                    echo "sdcomp: \c"
                    read command ;;
            esac
        done ;;
    yes)
	start=`echo $start $step | awk '{print $1 + $2}'` ;;
    esac
done
exit 1
