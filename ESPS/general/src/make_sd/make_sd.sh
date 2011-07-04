#! /bin/sh
################################################################################
#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987,1990 Entropic Speech, Inc.; All rights reserved"
# 				
#
#
#  Sccs info: @(#)make_sd.sh	1.4 3/20/90 ESI
#  Written by: Alan Parker, ESI 
#  mods by J. Shore
#
################################################################################

USE_ESPS_COMMON=off 
export USE_ESPS_COMMON
ESPS_VERBOSE=0
export ESPS_VERBOSE

trap 'rm -f /tmp/pf$$; echo "caught signal - exiting"; exit' 1 2 3 9 15

range=1
sf=1

while test x$1 != x
do
	case "$1" in
	-r) range=$2; shift;;
	-r*) range=`echo $1 | awk '{print substr($0, 3)}'` ;;
	-f) field=$2; shift;;
	-f*) field=`echo $1 | awk '{print substr($0, 3)}'` ;;
	-S) sf=$2; shift;;
	-S*) sf=`echo $1 | awk '{print substr($0, 3)}'` ;;
	-x) debug=$2; shift;;
	-x*) debug=`echo $1 | awk '{print substr($0, 3)}'` ;;
 	-?*) echo "make_sd: unknown option $1" ; exit 1 ;;
	*) break;;
	esac
	shift
done

if test $# != 2
 then
 echo 'Usage: make_sd -f field [-r range] [-S sf] in_file out_file'
 exit 1
fi

 infile=$1
 in=$1
 outfile=$2

if test $infile = "-"
 then
 cat >/tmp/pf$$ -
 infile=/tmp/pf$$
 in='<stdin>'
fi


if test ! -r $infile
 then
  echo make_sd: cannot read file $infile
  exit 1
fi

if test x$field = x
 then
 echo 'make_sd: fieldname must be given with -f option'
 exit 1
fi

type=`fea_element -n -f $field $infile | awk '{print $2}'`

cplx=`echo $type | fgrep CPLX`

#echo $cplx

if test $cplx
  then
    echo "make_sd: sorry, can't work on complex field" $field
    exit 1
fi

element=`fea_element -c -f $field $infile` || exit

line="pplain -r$range -e$element $infile | testsd -r$sf -t$type -a- $outfile "

if test x$debug != x
  then
   echo $line
   eval $line
   exit 0
fi
  
eval $line
  
rm -f /tmp/pf$$

