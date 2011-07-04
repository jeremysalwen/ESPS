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
# @(#)me_sgram.sh	1.17	3/16/92	ESI/ERL
# 
# Written by:  David Burton
# Checked by:
# Revised by:  John Shoren
# 
# Brief description:
#						
#  script to compute maximum-entropy-based FSPEC files for viewing as  
#  speech spectrograms - see also plotspgm.sh for plotting 		
#
#  This command file runs the C programs filter, refcof and me_spec.    |
#									|
#-----------------------------------------------------------------------+

#set -x

# Check syntax

case $# in
0|1)
	echo "Usage: me_sgram [-{pr} range] [-s range] [-P param_file] [-m method]"
	echo "     [-a analysis_mthd] [-E pre-emphasis] [-S step_size] [-o analysis_order] "
	echo "     [-w window_length] [-d data_window] -D input.SD output.FSPEC" 1>&2
	exit 1 ;;
esac


# initialize variables

LIBDIR=`get_esps_base`/lib
BINDIR=`get_esps_base`/bin
prgm=`get_esps_base`/bin/me_sgram
method=
pre_emp= 
step_size= 
window_len=
data_win=
analysis_order=
analysis_mthd=
infilename=
outfilename=-
sf=
num_points=
start_p=
nan=
range=
Dflag=0
conv_test=
max_iter=
S=no
P=NO
paramfile=

# Get command-line options

letter_opt=
for i in $*
do
	case $letter_opt in
	-s)
		range=$i
		S=yes
		letter_opt= ;;
	-p)
		range=$i
		P=yes
		letter_opt= ;;
	-r)
		range=$i
		P=yes
		letter_opt= ;;
	-m)
		method=$i
		letter_opt= ;;
	-P)
		paramfile=$i
		letter_opt= ;;
	-a)
		analysis_mthd=$i
		letter_opt= ;;
	-c)
		conv_test=$i
		letter_opt= ;;
	-i)
		max_iter=$i
		letter_opt= ;;
	-E)
		pre_emp=$i
		letter_opt= ;;
	-S)
		step_size=$i
		letter_opt= ;;
	-w)
		window_len=$i
		letter_opt= ;;
	-d)
		data_win=$i
		letter_opt= ;;
	-o)
		analysis_order=$i
		letter_opt= ;;
	"")
		case $i in
		-s)
			letter_opt=-s ;;
		-s*)
			range=`echo $i | awk '{print substr($0, 3)}'` 
			S=yes
			letter_opt= ;;
		-p)
			letter_opt=-p ;;
		-p*)
			range=`echo $i | awk '{print substr($0, 3)}'` 
			P=yes
			letter_opt= ;;
		-r)
			letter_opt=-r ;;
		-r*)
			range=`echo $i | awk '{print substr($0, 3)}'` 
			P=yes
			letter_opt= ;;
		-m)
			letter_opt=-m ;;
		-m*)
			method=`echo $i | awk '{print substr($0, 3)}'` 
			letter_opt= ;;
		-P)
			letter_opt=-P ;;
		-P*)
			paramfile=`echo $i | awk '{print substr($0, 3)}'` 
			letter_opt= ;;
		-a)
			letter_opt=-a ;;
		-a*)
			analysis_mthd=`echo $i | awk '{print substr($0, 3)}'` 
			letter_opt= ;;
		-E)
			letter_opt=-E ;;
		-E*)
			pre_emp=`echo $i | awk '{print substr($0, 3)}'`
			letter_opt= ;;
		-S)
			letter_opt=-S ;;
		-S*)
			step_size=`echo $i | awk '{print substr($0, 3)}'`
			letter_opt= ;;
		-c)
			letter_opt=-c ;;
		-c*)
			conv_test=`echo $i | awk '{print substr($0, 3)}'`
			letter_opt= ;;
		-i)
			letter_opt=-i ;;
		-i*)
			max_iter=`echo $i | awk '{print substr($0, 3)}'`
			letter_opt= ;;
		-w)
			letter_opt=-w ;;
		-w*)
			window_len=`echo $i | awk '{print substr($0, 3)}'`
			letter_opt= ;;
		-d)
			letter_opt=-d ;;
		-d*)
			data_win=`echo $i | awk '{print substr($0, 3)}'`
			letter_opt= ;;
		-o)
			letter_opt=-o ;;
		-o*)
			analysis_order=`echo $i | awk '{print substr($0, 3)}'`
			letter_opt= ;;
		-D)
			Dflag=1
			letter_opt= ;;
		*)
			# first is input, last is output

			if test $infilename
			then
				outfilename=$i 
			else
				infilename=$i
			fi ;;
		esac
	esac
done

# check for silly input

case $letter_opt in
-m)
	echo $prgm: -m requires argument. 1>&2
	exit 1 ;;
-a)
	echo $prgm: -a requires argument. 1>&2
	exit 1 ;;
-p)
	echo $prgm: -p requires argument. 1>&2
	exit 1 ;;
-P)
	echo $prgm: -P requires argument. 1>&2
	exit 1 ;;
-s)
	echo $prgm: -s requires argument. 1>&2
	exit 1 ;;
-w)
	echo $prgm: -w requires argument. 1>&2
	exit 1 ;;
-d)
	echo $prgm: -d requires argument. 1>&2
	exit 1;;
-o)
	echo $prgm: -o requires argument. 1>&2
	exit 1;;
-c)
	echo $prgm: -c requires argument. 1>&2
	exit 1;;
-i)
	echo $prgm: -i requires argument. 1>&2
	exit 1;;
esac

# Check to see if both -s and -p were used

if test $S = $P
then
	echo "$prgm: -s and -p options cannot both be used" 1>&2
	exit 1
fi

# Check if input file is readable

if test ! -r $infilename 
then
	if test $infilename != "-"
	then
	    echo "$prgm: \"$infilename\" is not a readable file" 1>&2
	    exit 1
	fi
fi

#create -P option

Poption=

if test ! -z "$paramfile"
then
   Poption="-P $paramfile"
fi


if test -z "$range"
then
   P=yes
   start_p=`getparam -p start $Poption -c $infilename -z`
   if test -z "$start_p"
   then
     start_p=1
   fi
   nan=`getparam -p nan $Poption -c $infilename -z`
   if test -z "$nan"
   then
     range=$start_p:
   else 
     if test $nan = "0"
     then
        range=$start_p:
     else
	nan=`echo $nan -1 | bc -l`
        range=$start_p:+$nan
     fi
   fi
fi

# if -m not used, set method (including default) 

if test -z "$method"
then
  method=`getparam -p method $Poption -c $infilename -z`
  if test -z "$method"
  then
	method=wb
  fi
fi

#echo check method for WB NB wb and nb

if test "$method" != "wb" 
then
 if test "$method" != "nb" 
 then
  if test "$method" != "WB" 
  then
   if test "$method" != "NB"
   then
    if test "$method" != "other"
    then
	echo "invalid spectrogram type: $method" 1>&2
	exit 1
    fi
   fi
  fi
 fi
fi

# set parameters for commands

#echo set wb and nb parameters

if test "$method" = "wb" 
then
# initialize for wide band spectrogram
p_e=.94
s_s=2
w_l=8
d_w=RECT
a_o=10
a_m=fburg
fi

if test "$method" = "nb"
then
# initialize for narrow band spectrogram
p_e=.94
s_s=2
w_l=40
d_w=RECT
a_o=10
a_m=fburg
fi

if test "$method" = "WB"
then
# initialize for wide band spectrogram
p_e=.94
s_s=2
w_l=8
d_w=RECT
a_o=10
a_m=fburg
fi

if test "$method" = "NB"
then
# initialize for narrow band spectrogram
p_e=.94
s_s=2
w_l=40
d_w=RECT
a_o=10
a_m=fburg
fi

if test "$method" = "other"
then
# initialize for other spectrogram
p_e=.94
s_s=2
w_l=8
d_w=RECT
a_o=10
a_m=fburg
fi

# Over ride defaults by command line options

#echo Over riding defaults

if test -z "$pre_emp"
then
  pre_emp=`getparam -p pre_emphasis $Poption -c $infilename -z`
  if test -z "$pre_emp"
  then
	pre_emp=$p_e
  fi	
fi

if test -z "$step_size"
then
  step_size=`getparam -p step_size $Poption -c $infilename -z`
  if test -z "$step_size"
  then
	step_size=$s_s
  fi	
fi

if test -z "$window_len"
then
  window_len=`getparam -p window_len $Poption -c $infilename -z`
  if test -z "$window_len"
  then
	window_len=$w_l
  fi	
fi

if test -z "$data_win"
then
  data_win=`getparam -p window_type $Poption -c $infilename -z`
  if test -z "$data_win"
  then
	data_win=$d_w
  fi	
fi

if test -z "$analysis_order"
then
  analysis_order=`getparam -p order $Poption -c $infilename -z`
  if test -z "$analysis_order"
  then
	analysis_order=$a_o
  fi	
fi

if test -z "$analysis_mthd"
then
  analysis_mthd=`getparam -p lpc_method $Poption -c $infilename -z`
  if test -z "$analysis_mthd"
  then
	analysis_mthd=$a_m
  fi	
fi

if test -z "$conv_test"
then
  conv_test=`getparam -p conv_test $Poption -c $infilename -z`
  if test -z "$conv_test"
  then
	conv_test=1e-5
  fi	
fi

if test -z "$max_iter"
then
  max_iter=`getparam -p max_iter $Poption -c $infilename -z`
  if test -z "$max_iter"
  then
	max_iter=20
  fi	
fi

#use special common location for passing range option now that paramters
#have been obtained as usual; turn verbosity off

if test -z "$HOME"
then
  priv_common=.me_sgram_com
else
  priv_common=$HOME/.me_sgram_com
fi

rm -f $priv_common

ESPSCOM=$priv_common; export ESPSCOM
USE_ESPS_COMMON=yes; export USE_ESPS_COMMON
ESPS_VERBOSE=0; export ESPS_VERBOSE

# Set range in points in COMMON

if test $S = yes
then
setrange -z -s$range $infilename
else
setrange -z -p$range $infilename
fi

# Getting sampling frequency
#echo Getting sampling frequency

sf=`hditem -irecord_freq $infilename`

# Computing step and window size
#echo Computing step and window size

if test $step_size -le 0
then
	echo "$prgm: step_size ($step_size) <= 0" 1>&2
	exit 1
fi
step_size=`echo $step_size*$sf/1000 | bc -l`

if test $window_len -le 0
then
	echo "$prgm: window_len ($window_len) <= 0" 1>&2
	exit 1
fi
window_len=`echo $window_len*$sf/1000 | bc -l`

# Build pre-emphasis filter
cat > /tmp/num$$ <<EOD
2
1 
-$pre_emp
EOD

$BINDIR/atofilt -cpre-emphasis  /tmp/num$$ /tmp/pre$$

# filter data and compute power spectrums

#echo "step_size = $step_size, win_len = $window_len, analysis_order = $analysis_order, data_win = $data_win"

if test $Dflag -eq 0
then
	$BINDIR/filter -F/tmp/pre$$ $infilename - | $BINDIR/refcof -p1:2147483646 -S$step_size -l$window_len -w$data_win -m$analysis_mthd -o$analysis_order -c$conv_test -i$max_iter -z - - | $BINDIR/me_spec -n257 - $outfilename 2>/dev/null

else
	$BINDIR/filter -F/tmp/pre$$ $infilename - | $BINDIR/refcof -p1:2147483646 -S$step_size -l$window_len -w$data_win -m$analysis_mthd -o$analysis_order -c$conv_test -i$max_iter -z - - | $BINDIR/me_spec -n257 - - | $BINDIR/dither - $outfilename 2>/dev/null

fi

# clean up any left overs

rm /tmp/pre$$ /tmp/num$$ 2>/dev/null
