#! /bin/csh -x
#
# test script for acf  @(#)acf.t	1.7 1/21/97 ERL
#
set PROG = acf
setenv USE_ESPS_COMMON off
alias vecho 'if $VERBOSE echo'
alias ndrec 'hditem -i ndrec'
alias errex 'exit(1)'
alias passex 'exit(0)'
unset noclobber
# test script parameters
# ----------------------
# if test script finds an error, it exits with errex
# if program passes the test, test scripts exits with passex
# VERBOSE: 0 script works silently
#          1 script actual and correct test results are echoed
set VERBOSE = 1
#


# -----
# start
# -----
vecho "$PROG test script: `date`"
if !(-x $PROG) then
	vecho "$PROG test: Can't find program."
	errex
endif
#testsd -p2000 test.sd >& /dev/null
testsd -p2000 test.sd >& /dev/null
testsd -Tgauss -p2000 test_speech.sd

# test program I/0
# ----------------
vecho "Checking I/O"
set OPT = "-r1:512 -Pacf_test_params"
echo "float frame_len = 512;" > acf_test_params
echo "int sd_flag = 1;" >> acf_test_params
set CMD = "$PROG $OPT test.sd test.acf"
vecho $CMD
if { $CMD >& /dev/null } then
	vecho "Command passed."
else
	vecho "Command failed."
	errex
endif

set CMD = "$PROG $OPT - test.acf"
vecho "$CMD < test.sd"
($CMD < test.sd) >& /dev/null
if (-e test.acf && ! -z test.acf) then
	vecho "Command passed."
else
	vecho "Command failed."
	errex
endif

set CMD = "$PROG $OPT test.sd -"
vecho "$CMD > test.acf"
($CMD > test.acf) >& /dev/null
if (-e test.acf && ! -z test.acf) then
	vecho "Command passed."
else
	vecho "Command failed."
	errex
endif

# check Common processing
# -----------------------
vecho "Checking common processing."
unsetenv USE_ESPS_COMMON
setenv ESPSCOM acf_comm.t
setrange -p513:1024 test.sd
echo "int sd_flag = 1;" > acf_test_params
$PROG -Pacf_test_params test.sd test.acf >& /dev/null
if (`hditem -i start test.acf` == 513 && `hditem -i nan test.acf` == 512) then
	vecho "Common processing ok."
else
	vecho "Common processing failed. Try again, it often works second time around."
endif	
\rm acf_comm.t acf_test_params >& /dev/null
setenv USE_ESPS_COMMON off
unsetenv ESPSCOM

# Test parameters
# -P -r -s
# --------
vecho "testing -P option."
echo "int    start   =   513;" > acf_test_params
echo "int    nan      =   512;" >> acf_test_params
echo "int    sd_flag =  1;">> acf_test_params
$PROG -P acf_test_params -R test.sd test.acf >& /dev/null
if (`hditem -i start test.acf` == 513 && `hditem -i nan test.acf` == 512) then
	vecho "-P option ok."
else
	vecho "-P option failed."
	echo " expected start to equal 513, got `hditem -i start test.acf`"
	echo " expected nan to equal 513, got `hditem -i nan test.acf`"
endif	
\rm acf_param.t >& /dev/null

vecho "testing range options."
echo "int sd_flag =  1;"> acf_test_params
$PROG -P acf_test_params -r 513:1024 test.sd test.acf >& /dev/null
set l1 = `hditem -i start test.acf`
set l2 = `hditem -i nan test.acf`
echo 'string units =   "seconds";' >>  acf_test_params
$PROG -P acf_test_params -s0.064:0.128 test.sd test.acf >& /dev/null
set l3 = `hditem -i start test.acf`
set l4 = `hditem -i nan test.acf`
if ( $l1 == 513 && $l3 == 513 && $l2 == 512 && $l4 == 513) then
	vecho "range ok."
else
	vecho "range failed."
	errex
endif
\rm acf_test_params >& /dev/null

vecho "testing frame_len."
echo "int sd_flag =  1;"> acf_test_params
set CMD = "$PROG -P acf_test_params"
$CMD test.sd test.acf >& /dev/null
set l0 = `hditem -i ndrec test.sd`
set l1 = `hditem -i frmlen test.acf`
$CMD -r1:128 test.sd test.acf >& /dev/null
set l2 = `hditem -i frmlen test.acf`
echo "float frame_len  = 128;" >> acf_test_params
$CMD test.sd test.acf >& /dev/null
set l3 = `hditem -i frmlen test.acf`
if ( $l1 == $l0 && $l2 == 128 && $l3 == 128 ) then
	vecho "frame length ok."
else
	vecho "frame length failed."
	errex
endif
\rm acf_test_params >& /dev/null

vecho "testing step size."
set CMD = "$PROG -r1:1024 -P acf_test_params"
echo "int sd_flag = 1;" > acf_test_params
$CMD test.sd test.acf >&  /dev/null
set l1 = `hditem -i step test.acf`
echo "float step = 512;" >> acf_test_params
$CMD test.sd test.acf >&  /dev/null
set l2 = `hditem -i step test.acf`
if ($l1 == 1024 && $l2 == 512) then
	vecho "step ok."
else
	vecho "step failed."
	errex
endif
\rm acf_test_params >& /dev/null

vecho "testing window type."
set CMD = "$PROG -r1:512 -P acf_test_params "
echo "int sd_flag = 1;" > acf_test_params
$CMD test.sd test.acf >& /dev/null
set l1 = `hditem -i window_type test.acf`
echo "int sd_flag = 1;" > acf_test_params
echo 'string window_type = "TRIANG";' >> acf_test_params
$CMD test.sd test.acf >& /dev/null
set l2 = `hditem -i window_type test.acf`
echo "int sd_flag = 1;" > acf_test_params
echo 'string  window_type = "HAMMING";' >> acf_test_params
$CMD test.sd test.acf >& /dev/null
set l3 = `hditem -i window_type test.acf`
if ($l1 == "WT_RECT" && $l2 == "WT_TRIANG" && $l3 == "WT_HAMMING") then
	vecho "window_type ok."
else
	vecho "window_type failed."
	errex
endif
\rm acf_test_params >& /dev/null

vecho "testing preemphasis."
set CMD = "$PROG -r 1:1024 -P acf_test_params"
echo "int sd_flag = 1;" > acf_test_params
echo "float preemphasis = 0.97;" >> acf_test_params
$CMD test.sd test.acf >& /dev/null
set l1 = `hditem -i preemphasis test.acf`
if ($l1 == 0.97) then
	vecho "preemphasis ok."
else
	vecho "preemphasis failed - expected 0.97, got $l1 ."
endif
\rm acf_test_params >& /dev/null

FRAME:
echo " "
vecho "testing framing."
echo "float start = 1;" >  acf_test_params
echo "float frame_len = 512;" >>  acf_test_params
echo "float step = 128;" >>  acf_test_params
echo "float nan = 1024;" >>  acf_test_params
echo "int sd_flag = 1;" >> acf_test_params

echo " acf -P acf_test_params test_speech.sd t2.acf"
acf -P acf_test_params test_speech.sd t2.acf

echo " frame -r1:1023 -f sd -S128 -l512 test_speech.sd test_speech.frame"
frame -r1:1023 -f sd -S128 -l512 test_speech.sd test_speech.frame

echo " "
echo "the following comparison should yield no output:"
echo " pspsdiff -H -f sd test_speech.frame t2.acf"
pspsdiff -H -f sd test_speech.frame t2.acf
echo " "
echo "framing comparison finished."
echo " "
#following is based on acf.t2, written by John Shore

vecho "testing processing."

echo "running acf..."
echo "acf -r1:630 -P params.t test_speech.sd t2.acf"
acf -r1:630 -P params.t test_speech.sd t2.acf

# create pre-emphasis filter

echo "creating preemphasis filter..."

cat > preemp_data << ZAP
2
1
-.97
ZAP

atofilt -c "preemphasis filter" preemp_data preemp.filt

#run various other programs

#preemphasize and frame 


echo "computing preemphasized data ..."

echo "filter2 -F preemp.filt -d float test_speech.sd sp_pe.sd"

filter2 -F preemp.filt -d float test_speech.sd sp_pe.sd

\rm preemp_data preemp.filt

# do power
echo "   "
echo "computing power using pwr  ..."
echo " frame -r1:512 -whamming -f sd sp_pe.sd - |"
echo " pwr -fsd - - | pplain -r1 -f raw_power -"
frame -r1:512 -whamming -f sd sp_pe.sd - |\
pwr -fsd - - | pplain -r1 -f raw_power -

echo "power computed by acf should agree:"
echo " pplain -r1 -f power -r1 t2.acf"
pplain -f power -r1 t2.acf


# do zero crossings 
echo "  "
echo "computing zero crossings using zcross  ..."
echo " frame -r1:512 -whamming -f sd sp_pe.sd - |"
echo " zcross -fsd - - | pplain -r1 -f zero_cross_rate -"
frame -r1:512 -whamming -f sd sp_pe.sd - |\
zcross -fsd - - | pplain -r1 -f zero_cross_rate -

echo "zero crossings found by acf should agree:"
echo " pplain -r1 -fzero_crossing t2.acf"
pplain -r1 -fzero_crossing t2.acf


echo "  "
echo "computing autocorrelations using auto  ..."
echo " auto -l512 -o20 -r1:512 -whamming sp_pe.sd - |"
echo " pplain -r1 -fspec_param - "
auto -l512 -o20 -r1:512 -whamming sp_pe.sd - |\
pplain -r1 -fspec_param - 

echo "auto corr found by acf should agree (first term is the power):"
echo " pplain -r1 -fauto_corr t2.acf"
pplain -r1 -fauto_corr t2.acf

#test refcofs
echo " "
echo "computing reflection coefficients using refcof  ..."
echo " refcof -whamming -r1:512 -l512 -o20 sp_pe.sd sp_pe.rc"
echo " pplain -r1 -fspec_param sp_pe.rc "
refcof -whamming -r1:512 -l512 -o20 sp_pe.sd sp_pe.rc
pplain -r1 -fspec_param sp_pe.rc

echo "refl coeffs found by acf should agree:"
echo " pplain -r1 -frefcof t2.acf"
pplain -r1 -frefcof t2.acf

#test lpc coeffs
echo " "
echo "computing lpc coefficients using spectrans  ..."
echo " spectrans -m AFC sp_pe.rc - | pplain -r1 -fspec_param -"
spectrans -m AFC sp_pe.rc - | pplain -r1 -fspec_param -

echo "lpc coeffs found by acf should agree:"
echo " pplain -r1 -flpc_coeffs t2.acf"
pplain -r1 -flpc_coeffs t2.acf

#test lpc cep coeffs
echo " "
echo "computing lpc cepstral coefficients using spectrans  ..."
echo " spectrans -m CEP sp_pe.rc - | pplain -r1 -fspec_param -"
spectrans -m CEP sp_pe.rc - | pplain -r1 -fspec_param  -

echo "lpc cepstral coeffs found by acf should agree:"
echo "first 5 coeffs should agree with first 5 above;"
echo "last 5 coeffs should agree with last 5 above - ignore middle 10"
echo " pplain -r1 -flpc_cepstrum t2.acf"
pplain -r1 -flpc_cepstrum t2.acf

#test lar coeffs
echo " "
echo "computing lar coefficients using spectrans  ..."
echo " spectrans -m LAR sp_pe.rc - | pplain -r1 -fspec_param -"
spectrans -m LAR sp_pe.rc - | pplain -r1 -fspec_param -

echo "lars found by acf should agree:"
echo " pplain -r1 -flog_area_ratio t2.acf"
pplain -r1 -flog_area_ratio t2.acf

#test lsf
echo " "
echo "computing lsfs using spectrans  ..."
echo " spectrans -m LSF -w 9 sp_pe.rc - | pplain -r1 -fspec_param -"
spectrans -m LSF -w 9 sp_pe.rc - | pplain -r1 -fspec_param -

echo "lsfs found by acf should agree:"
echo " pplain -r1 -fline_spec_freq t2.acf"
pplain -r1 -fline_spec_freq t2.acf

#test fftcep
echo " "
echo "computing fft cepstrum using fftcep ..."
echo " fftcep -e0:7,15:19 -F -R -wHAMMING -r1:512 -l512 -o6 sp_pe.sd - |"
echo " pplain -f cepstrum_0 -"
fftcep -e0:7,15:19 -F -R -wHAMMING -r1:512 -l512 -o6 sp_pe.sd - |\
pplain -f cepstrum_0 -

echo "fft cepstrum found by acf should agree:"
echo " pplain -r1 -ffft_cepstrum t2.acf"
pplain -r1 -ffft_cepstrum t2.acf

#test fft
echo " "
echo "computing fft using fft ..."
echo " fft -wHAMMING -r1:512 -l512 -o6 sp_pe.sd - |"
echo " pplain -f re_spec_val -"
fft -wHAMMING -r1:512 -l512 -o6 sp_pe.sd - |\
pplain -f re_spec_val -
echo "fft found by acf should agree:"
echo " pplain -r1 -fre_spec_val t2.acf"
pplain -r1 -fre_spec_val t2.acf

WARP:
#test warping
echo " "
echo "testing warping and field deriv"
grep -v fftcep_deriv params.t |\
grep -v warp_param >! tp1
cat >> tp1 << ZAP
float warp_param = 0.6;
string fftcep_deriv = "0:7,59:63";
ZAP
grep -v deriv tp1 >! tp2

echo "calling acf with warp_param = 0.6,"
echo "lpccep_deriv = 0:5,15:19, fftcep_deriv = 0:7,59:63"
echo " acf -r1:630 -P tp1 test_speech.sd t2.acf"
acf -r1:630 -P tp1 test_speech.sd t2.acf
echo " "
echo "calling acf with warp_param = 0.6 and "
echo "no fftcep_deriv or lpccep_deriv"
echo " acf -r1:630 -P tp2 test_speech.sd t3.acf"
acf -r1:630 -P tp2 test_speech.sd t3.acf

echo "lpccep:"
echo " pplain -r1 -f lpc_cepstrum t2.acf"
pplain -r1 -f lpc_cepstrum t2.acf
echo " "
echo "first 6 and last 5 should agree with above..."
echo " pplain -r1 -f lpc_cepstrum t3.acf"
pplain -r1 -f lpc_cepstrum t3.acf

echo " "
echo "fftcep:"
echo " pplain -r1 -f fft_cepstrum t2.acf"
pplain -r1 -f fft_cepstrum t2.acf
echo " "
echo "first 8 and last 5 should agree with above..."
echo " pplain -r1 -f fft_cepstrum t3.acf"
pplain -r1 -f fft_cepstrum t3.acf



# Done
#-----
echo " "
echo "cleaning up ..."
\rm test.sd test.acf t2.acf t3.acf sp_pe.rc sp_pe.sd 
\rm tp1 tp2 
vecho "`date`: $PROG test script finished."
passex
