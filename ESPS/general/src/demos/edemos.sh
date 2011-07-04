#! /bin/sh
# top level script for ERL demos
# @(#)edemos.sh	1.7 11/2/92 ERL
#
# Called with one (optional) argument
#
# $1 = path to top directory for temporary files
#set -x

EXTRASMENU=/dev/null
export EXTRASMENU

# check ELM_HOST

if test x$ELM_HOST = x
then
  echo ""
  echo You do not have ELM_HOST set.  Please set this environment variable
  echo to the hostname on which the Entropic license manager is running.   
  exit 1

fi

# check syntax

case $# in
1)
        # any output files go in directory specified by $1
	# note that this may lead to collisions if it's reused
	demo_temp=$1
	demo_temp_specified=1
	;;
*)
	demo_temp=$HOME/Edemo$$
	echo "no temporary directory specified, will use $demo_temp"
	;;
esac

# make sure ESPS license is checked out, if possible.  This is 
# harmless is one is already checked out.  

get_lic=`get_esps_base`/bin/echeckout

$get_lic > /dev/null 2>&1

# create temp directory if necessary, check that it is writable

if test ! -d $demo_temp
then
  mkdir $demo_temp > /dev/null 2>&1
fi

touch $demo_temp/foo.$$ > /dev/null 2>&1

if test -f $demo_temp/foo.$$
 then
   rm $demo_temp/foo.$$
 else 
   echo "$0: can't create $demo_temp or can't write in it; exiting. "
   exit 1
fi

# make sure we have full path for temp directory
cur_dir=`pwd`
cd $demo_temp
demo_temp=`pwd`
cd $cur_dir

# If we are not in the main demo directory, we try to find it 
# and switch to it.  

if test -f demo_menu
  then
    demo_dir=`pwd`
  else
    demo_dir=`get_esps_base`/newdemos
fi

if test ! -d $demo_dir
 then 
   echo "$0: Couldn't find demo directory; exiting."
   exit 1
fi

demo_menu=$demo_dir/demo_menu
# cd $demo_dir > /dev/null 2>&1
cd $demo_dir 

# insert temp directory in standard demo menu file

sed s%@DEMO_TEMP@%$demo_temp% $demo_menu > $demo_temp/entropic.MB

# bring up main button panel

mbuttons -w "Entropic Demos" -i Edemos -X1 -Y1 -q1 $demo_temp/entropic.MB 

# after mbuttons exits, we remove the temp directory
# unless it appears that a demo is running
# NOTE: here we exploit the lock file set from edemos.sh
# if the user specified the temp directory, we don't remove it

if test x$demo_temp_specified != x
then
 exit
fi

lockfile=$demo_temp/demo.lock
message=$demo_temp/errormess

if test -f $lockfile
then
 rm -f $message
 cat > $message << EOF
NOTE: It appears that a demo is currently running, so the temp 
directory $demo_temp was not removed.  
EOF

  xtext -Wp 0 0 -t "DEMO ERROR" -F $message &
  exit 1

else
 rm -rf $demo_temp
fi











