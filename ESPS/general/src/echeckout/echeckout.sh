#! /bin/csh -f
# @(#)echeckout.sh	1.1 12/14/95 ERL
# script to run both old and new echeckouts

$ESPS_BASE/bin/echeckout50 >& /dev/null
$ESPS_BASE/bin/echeckout51
