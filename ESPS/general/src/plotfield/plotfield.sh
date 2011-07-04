#! /bin/sh
################################################################################
#
#  This material contains proprietary software of Entropic Speech, Inc.   
#  Any reproduction, distribution, or publication without the the prior	   
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice			
# 								
#      "Copyright (c) 1987, 1990 Entropic Speech, Inc.; All rights reserved"
# 	"Copyriight (c) 1998 Entropic Group; All rights reserved"	
#
#
#  Sccs info: @(#)plotfield.sh	1.4 7/20/98 ESI
#  Written by: Alan Parker, ESI 
#
################################################################################

USE_ESPS_COMMON=off 
export USE_ESPS_COMMON
ESPS_VERBOSE=0
export ESPS_VERBOSE

trap 'rm -f /tmp/pf$$; echo "caught signal - exiting"; exit' 1 2 3 9 15

range=1
text=-t" "

while test x"$1" != x
do
	case "$1" in
	-r) range=$2; shift;;
	-r*) range=`echo $1 | awk '{print substr($0, 3)}'` ;;
	-f) field=$2; shift;;
	-f*) field=`echo $1 | awk '{print substr($0, 3)}'` ;;
	-t) text=-t"$2"; shift;;
	-t*) text=-t`echo "$1" | awk '{print substr($0, 3)}'` ;;
	-x) debug=$2; shift;;
	-x*) debug=`echo $1 | awk '{print substr($0, 3)}'` ;;
	-y) yopt=-y$2; shift;;
	-y*) yopt=-y`echo $1 | awk '{print substr($0, 3)}'` ;;
	-Y) Yopt=-Y$2; shift;;
	-Y*) Yopt=-Y`echo $1 | awk '{print substr($0, 3)}'` ;;
	-) infile='-';;
	-*) echo "plotfield: unknown option $i" ; exit 1 ;;
	*) break;;
	esac
	shift
done

if test x$infile != x- -a $# != 1
 then
 echo 'Usage: plotfield -f field [-r range] [-t text] [-y range] [-Y range] file'
 exit 1
fi

if test x$infile != x-
 then
 infile=$1
 in=$1
else
 cat >/tmp/pf$$ -
 infile=/tmp/pf$$
 in='<stdin>'
fi

if test ! -r $infile
 then
  echo plotfield: cannot read file $infile
  exit 1
fi

if test x$field = x
 then
 echo 'plotfield: fieldname must be given with -f option'
 exit 1
fi

#
# Check for complex data
#
fea_element -f $field $infile | grep -s CPLX
if test $? -eq 0
then
	echo "plotfield: Input filed ($field) is complex - complex data is not supported yet."
	exit 1
fi

title="File: $in, field: $field, record: $range"

element=`fea_element -c -f $field $infile` || exit

line="pplain -r$range -e$element $infile | testsd -a-  - | plotsd -t'$title' '$text' $Topt $Yopt $yopt -"

if test x$debug != x
  then
   echo "$line"
   exit 0
fi
  
eval "$line"
  
rm -f /tmp/pf$$
