#!/bin/sh
#
# Install the docs
#

DOCDIR=`get_esps_base`/doc

if test ! -d $DOCDIR
 then
  echo $DOCDIR does not exist, check install scripts or seek help
  exit 1
fi

if test x$MANMOD = x
 then
  MANMOD=0640
fi

echo Comparing dates of files here and in $DOCDIR
echo This might take a while....

for name in $*
 do
  srcname=$name.*me*
  if newer $srcname $DOCDIR/$srcname
   then
    echo Updating $srcname...
    case $name in
      history)
	set -x 
	 refer -n -p esps.refs -e history.prme |  \
	   nroff -me -D'prersterization on' \
	    | col | ul -tadm3a > $DOCDIR/$name.doc
	set +x
	 ;;
      intro)
	set -x
	 refer -n -p esps.refs -e $srcname | \
	  nroff -me -n | col | ul -tadm3a  > $DOCDIR/$name.doc
	set +x
	;;
      *)
    	set -x
    	typeset -n $srcname |  col | ul -tadm3a  > $DOCDIR/$name.doc
    	set +x
	;;
    esac
  fi
done

exit 0
