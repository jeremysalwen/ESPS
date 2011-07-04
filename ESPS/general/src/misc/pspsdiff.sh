#! /bin/sh
# This shell script is used to compare the output of two ESPS files
# using psps and diff.
# @(#)pspsdiff.sh	3.2	8/19/87	ESI
#
trap 'echo removing tmp files; /bin/rm -f $psps1 $psps2; exit' 2
psps1=/tmp/psps1$$
psps2=/tmp/psps2$$
file2=\$$#
file1=\$`expr $# - 1`
opt=
case $# in
	0|1) echo Usage: $0 [-aDghHlvx] [-r start:end] [-t tag] [-f field_name] esps.file1 esps.file2
	     exit 0 ;;
	2) : ;;
	*) argflag=
 	   for i do
		if test $argflag
		then
		   opt="$opt $i"
		   argflag=
		else
		   case $i in
			-[aDghHlvx]) opt="$opt $i" ;;
			-[rtf]) opt="$opt $i"
				argflag=SET ;;
		   esac
		fi
	   done
esac
file1=`eval echo $file1`
file2=`eval echo $file2`
psps $opt $file1 >$psps1
psps $opt $file2 >$psps2
diff $psps1 $psps2
/bin/rm -f $psps1 $psps2
