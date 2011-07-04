#! /bin/csh
#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice
#
#      "Copyright (c) 1991 Entropic Speech, Inc.; All rights reserved"
#
#  makefile for @(#)dtw.t	1.5  1/4/95 ERL
#
# test script for dtw
set PROG = dtw
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

vecho "generating test data"
# generate sample data to test dtw 
testsd -p100 t1.sd >& /dev/null
vecho "constructing vq codebook to use for testing..."
echo "double conv_ratio = 0.1;" > vqdes.p
echo "int    vq_size    = 8;" >> vqdes.p 
echo 'string cbk_struct = "FULL_SEARCH";' >> vqdes.p
echo 'string dist_type = "MSE";' >> vqdes.p
echo 'string fea_field = "samples";' >> vqdes.p
echo "int fea_dim = 1;" >> vqdes.p
vqdes -P vqdes.p t1.sd cbk >& /dev/null
vecho "finding codebook distances..."
cbkd cbk cbk.dist >& /dev/null
if ( `ndrec cbk` == 4 && `ndrec cbk.dist` == 4 ) then
	vecho "test codebook constructed"
	\rm in.asc vqdes.p vqdesaschist vqdesasc.chk >& /dev/null
else
	vecho "can't make test codebook"
	errex
endif
#
vq -f samples cbk.dist t1.sd t1.v

vecho "dtw of signal with itself."
set d1 =  `$PROG -f samples t1.sd t1.sd`
set d2 =  `$PROG -c cbk.dist -f samples_cwndx t1.v t1.v`
set d1 = `echo $d1 | awk '{ if ($1 != 0.0) {print 0} else {print 1} } '`
set d2 = `echo $d2 | awk '{ if ($1 != 0.0) {print 0} else {print 1} } '`
if ( $d1 == 1 && $d2 == 1 ) then
	vecho "comparison of signal to itself ok."
else
	vecho "comparison of signal to itself fails."
	errex
endif

vecho "checking parameters."
echo 'string sequence_field = "samples_cwndx";' > dtw.params
echo 'double best_so_far = 10.0;' >> dtw.params
echo 'int delta = 1;' >> dtw.params
echo 'string distance_table_file = "cbk.dist";' >> dtw.params
echo 'string distance_table_field = "distance_table";' >> dtw.params
echo 'int distance_table_recno = 4;' >> dtw.params
$PROG -P dtw.params t1.v t1.v res >& /dev/null
set d1 = `hditem -i dtw_result res`
set d2 = `hditem -i dtw_best_so_far res`
set d3 = `hditem -i dtw_delta res`
if ( $d1 == 0 && $d2 == 10 && $d3 == 1) then
	vecho "parameter processing ok."
else
	vecho "parameter processing failed."
	errex
endif

vecho "checking delta option"
copysd -p2: t1.sd t2.sd >& /dev/null
$PROG -f samples t1.sd t2.sd res >& /dev/null
set d1 = `hditem -i dtw_result res`
$PROG -f samples -d1 t1.sd t2.sd res >& /dev/null
set d2 = `hditem -i dtw_result res`
if ( $d1 != 0 && $d2 == 0 ) then
	vecho "delta option ok."
else
	vecho "delta option failed."
	errex
endif

vecho "checking best_so_far option"
set cval = ` $PROG -f samples t1.sd t2.sd | awk '{ printf "%e", $1 / 2.0}'`
set rval = ` $PROG -b $cval -f samples t1.sd t2.sd `
if ( $cval == $rval ) then
	vecho "best_so_far option ok."
else
	vecho "best_so_far option failed."
	errex
endif

vecho "checking quantized sequences"
vq -f samples cbk.dist t2.sd t2.v
set cval = ` $PROG -f samples t1.v t2.v `
set qval = ` $PROG -f samples_cwndx -c cbk.dist t1.v t2.v `
if ( $cval == $qval ) then
	vecho "results found by table lookup agree with results using quantized field"
else
	vecho "discrepancy between comparisons using table lookup and quantized field"
	errex
endif

vecho "functional dtw test - deletions/insertions"
\rm t3.sd >& /dev/null
copysps -g 1:49,51:60,60:100 t1.sd t3.sd
set d1 = `$PROG -f samples t1.sd t3.sd res`
pplain -fmapping res > t1
awk 'BEGIN {for (i=1; i<=100; i++) \
{ if ( i != 50 ) { print i } ; if ( i == 60 ) {print i} } }' < /dev/null > t2
diff -b t1 t2 >& /dev/null
if ( $status == 0 ) then
	vecho "deletions/insertions found ok."
else
	vecho "dtw algorithm failed in finding deletions/insertions."
	errex
endif 

\rm cbk cbk.dist dtw.params res t1.sd t2.sd t1.v t2.v vqhist vqdes.chk vqdeshist t1 t2 t3.sd
vecho "$PROG passed test `date`"
passex

