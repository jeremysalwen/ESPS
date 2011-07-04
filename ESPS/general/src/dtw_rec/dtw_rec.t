#! /bin/csh -x
#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice
#
#      "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
#
#  testscript for dtw_rec @(#)dtw_rec.t	1.1 2/25/91 ERL

set PROG = dtw_rec
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
copysd -r2: t1.sd t2.sd
vq -f samples cbk.dist t2.sd t2.v
copysd -r3: t1.sd t3.sd
vq -f samples cbk.dist t3.sd t3.v

vecho "test time warping comparisons"
echo "t1.v" > tlist 
echo "t2.v" >> tlist 
echo "t3.v" >> tlist 
$PROG -f samples tlist tlist res
set cval = `awk 'BEGIN{i=1}{if ($2 != $3 &&$4 != 0.0) i = 0} END{print i}'<res`
if ( $cval == 1 ) then
	vecho "self test works."
else
	vecho "self test failed."
	errex
endif

vecho "testing delta option"
echo "t1.v" > tlist
echo "t2.v" > rlist
$PROG -f samples tlist rlist res
set cval = `awk 'BEGIN{i=1}{if ($2 == $3 && $4 == 0.0) i = 0} END{print i}'<res`
$PROG -f samples -d 1 tlist rlist res
set dval = `awk 'BEGIN{i=1}{if ($2 != $3 && $4 != 0.0) i = 0} END{print i}'<res`
echo "int delta = 1;" > dtw.params
$PROG -f samples -P dtw.params tlist rlist res
set eval = `awk 'BEGIN{i=1}{if ($2 != $3 && $4 != 0.0) i = 0} END{print i}'<res`
if ( $cval == 1 && $dval == 1 && $eval == 1 ) then
	vecho "delta option/parameter ok."
else
	vecho "delta option/parameter fails."
	errex
endif

vecho "checking table lookup"
echo "t1.v" >  tlist
echo "t2.v" >> tlist
echo "t2.v" > rlist
echo "t3.v" >> rlist
$PROG -f samples rlist tlist res1
$PROG -f samples_cwndx -c cbk.dist rlist tlist res2
echo 'string distance_table_file = "cbk.dist";' > dtw.params
echo 'string sequence_field = "samples_cwndx";' >> dtw.params
$PROG -P dtw.params -c cbk.dist rlist tlist res3
diff -b res1 res2 >& /dev/null
set c1 = $status
diff -b res1 res3 >& /dev/null
if ( $c1 == 0 && $status == 0 ) then
	vecho "table lookup agrees with direct computation"
else
	vecho "table lookup and direct computation don't agree."
	errex
endif

vecho "checking list length"
echo "t1.v" > tlist
echo "t1.v" >  rlist
echo "t2.v" >> rlist
echo "t3.v" >> rlist
$PROG -f samples -l 2 rlist tlist res1
echo 'string sequence_field = "samples";' > dtw.params
echo "int best_list_length = 3;" >> dtw.params
$PROG -P dtw.params rlist tlist res2
if ( `awk 'END{print NR}'<res1` == 2 && `awk 'END{print NR}'<res2` == 3 ) then
	vecho "list length option/parameter ok."
else
	vecho "list length option/parameter failure."
	errex
endif

vecho "comparing dtw to dtw_rec"
echo "t1.v" > tlist
echo "t2.v" > rlist
$PROG -f samples rlist tlist res1


set d2 = `dtw -f samples t2.v t1.v`
set d1 = `awk '{print $4}' < res1`
if ( $d2 == $d1 ) then
	vecho "numerical results of dtw and dtw_rec agree."
else
	vecho "numerical results of dtw and dtw_rec disagree."
	errex
endif


\rm cbk cbk.dist t1.sd t1.v vqdes.chk vqdeshist vqhist res1 res2 res3
\rm dtw.params t2.sd t2.v t3.sd t3.v tlist rlist res
vecho "$PROG passed: `date`"
passex

