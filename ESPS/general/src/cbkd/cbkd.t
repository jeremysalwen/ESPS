#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice
#
#      "Copyright (c) 1991 Entropic Speech, Inc.; All rights reserved"
#
#  makefile for @(#)cbkd.t	1.1  2/22/91 ERL
#! /bin/csh 
#
# test script for cbkd
set PROG = cbkd
setenv USE_ESPS_COMMON off
alias vecho 'if $VERBOSE echo'
alias ndrec 'hditem -i ndrec'
alias errex 'exit(1)'
alias passex 'exit(0)'
\rm params >& /dev/null
# test script parameters
# ----------------------
# if test script finds an error, it exits with errex
# if program passes the test, test scripts exits with passex
# VERBOSE: 0 script works silently
#          1 script actual and correct test results are echoed
set VERBOSE = 1

vecho "$PROG test script: `date`"

vecho "construct vq codebook to use for testing"
echo "0 0 0 0" > in.asc
echo "1 1 1 1" >> in.asc
echo "double conv_ratio = 0.0005;" > vqdesasc.p
echo "int    vq_size    = 2;" >> vqdesasc.p
echo 'string cbk_struct = "FULL_SEARCH";' >> vqdesasc.p
echo 'string dist_type = "MSE";' >> vqdesasc.p
vqdesasc -P vqdesasc.p -c ignore in.asc cbk.in >& /dev/null
if ( `ndrec cbk.in` == 2 ) then
	vecho "test codebook constructed"
	\rm in.asc vqdesasc.p vqdesaschist vqdesasc.chk >& /dev/null
else
	vecho "can't make test codebook"
	errex
endif

# test program i/o and -r parameter
# ---------------------------------
$PROG -r1 cbk.in cbk.out >& /dev/null
set r1 = `ndrec cbk.out`
$PROG - cbk.out < cbk.in >& /dev/null
set r2 = `ndrec cbk.out`
($PROG cbk.in - > cbk.out) >& /dev/null
set r3 = `ndrec cbk.out`
if ( $r1 == 1 && $r2 == 2 && $r3 == 2 ) then 
	vecho "i/o okay."
else
	vecho "output codebooks contain incorrect number of records."
	errex
endif


# check that distances are computed correctly
# -------------------------------------------
vecho "compute codebook distances"
$PROG -r2 cbk.in cbk.out  >& /dev/null
set d0 = `pplain -f distance_table"[0]" cbk.out`
set d1 = `pplain -f distance_table"[1]" cbk.out`
set d2 = `pplain -f distance_table"[2]" cbk.out`
set d3 = `pplain -f distance_table"[3]" cbk.out`
if ( $d0 == 0 && $d1 == 2 && $d2 == 2 && $d3 == 0 ) then
	vecho "distances computed correctly."
else
	vecho "error computing distances."
	errex
endif

# check -t option
# ---------------
vecho "checking -t option"
$PROG -t table -r2 cbk.in cbk.out  >& /dev/null
set d0 = `pplain -f table"[0]" cbk.out`
set d1 = `pplain -f table"[1]" cbk.out`
set d2 = `pplain -f table"[2]" cbk.out`
set d3 = `pplain -f table"[3]" cbk.out`
if ( $d0 == 0 && $d1 == 2 && $d2 == 2 && $d3 == 0) then
	vecho "-t option works."
else
	vecho "error testing -t option."
	errex
endif

# checking paramter processing
# ----------------------------
vecho "checking parameter processing"
echo  "int start = 2;" > test.params
echo  "int nan = 1;" >> test.params
echo 'string table_field = "table";' >>  test.params
$PROG -P test.params cbk.in cbk.out >& /dev/null
set r1 = `ndrec cbk.out`
set d0 = `pplain -f table"[0]" cbk.out`
set d1 = `pplain -f table"[1]" cbk.out`
set d2 = `pplain -f table"[2]" cbk.out`
set d3 = `pplain -f table"[3]" cbk.out`
if ( $r1 == 1 && $d0 == 0 && $d1 == 2 && $d2 == 2 && $d3 == 0) then
	vecho "parameter processing okay."
else
	vecho "error in parameter processing."
	errex
endif

# checking added fields
# ---------------------
vecho "checking that added fields are copied from input to output"
echo "10" > asc.in
echo "20" >> asc.in
addfea -f arf -t LONG -s 1 -c "no comment" asc.in cbk.in
$PROG cbk.in cbk.out >& /dev/null
set r1 = `ndrec cbk.out`
set d0 = `pplain -r 2 -f distance_table"[0]" cbk.out`
set d1 = `pplain -r 2 -f distance_table"[1]" cbk.out`
set d2 = `pplain -r 2 -f distance_table"[2]" cbk.out`
set d3 = `pplain -r 2 -f distance_table"[3]" cbk.out`
set d4 = `pplain -r 1 -f arf cbk.out`
set d5 = `pplain -r 2 -f arf cbk.out`
if ( $r1 == 2 && $d0 == 0 && $d1 == 2 && $d2 == 2 && $d3 == 0 && $d4 == 10 && $d5 == 20 ) then
	vecho "field copied okay."
else
	vecho "error in copying added field."
	errex
endif

# finished test
# -------------
\rm asc.in test.params cbk.in cbk.out >& /dev/null
vecho "$PROG test script passed: `date`"
passex

