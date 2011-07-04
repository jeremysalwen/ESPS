#! /bin/csh -f
# @(#)efree.sh	1.1 12/14/95 ERL
# script to run both old and new efree

$ESPS_BASE/bin/efree50 >& /dev/null
$ESPS_BASE/bin/efree51
