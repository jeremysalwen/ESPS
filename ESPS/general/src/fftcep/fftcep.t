# test script for fftcep @(#)fftcep.t	1.5 1/22/97 ERL
#! /bin/csh -x
#
# test script for fftcep
set PROG = fftcep
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
#
# VISUAL: 0 script produces no plots
#         1 script plots program output and described how it should appear
set VISUAL = 0

# -----
# start
# -----
vecho "$PROG test script: `date`"
testsd -p2000 test.sdd >& /dev/null
testsd -p2000 -Tgauss -l 2 noise
addsd test.sdd noise test.sd

# test program I/0
# ----------------
vecho "Checking I/O"
set OPT = "-r1:512"
set CMD = "$PROG $OPT test.sd test.cep"
vecho $CMD
if { $CMD >& /dev/null } then
	vecho "Command passed."
else
	vecho "Command failed."
	errex
endif

set CMD = "$PROG $OPT - test.cep"
vecho "$CMD < test.sd"
($CMD < test.sd) >& /dev/null
if (-e test.cep && ! -z test.cep) then
	vecho "Command passed."
else
	vecho "Command failed."
	errex
endif

set CMD = "$PROG $OPT test.sd -"
vecho "$CMD > test.cep"
($CMD > test.cep) >& /dev/null
if (-e test.cep && ! -z test.cep) then
	vecho "Command passed."
else
	vecho "Command failed."
	errex
endif

# check Common processing
# -----------------------
vecho "Checking common processing."
unsetenv USE_ESPS_COMMON
setenv ESPSCOM fft_cep_comm.t
setrange -p513:1024 test.sd >& /dev/null
$PROG -R -l 512 test.sd test.cep >& /dev/null
if (`hditem -i start test.cep` == 513 && `hditem -i nan test.cep` == 512) then
	vecho "Common processing ok."
else
	vecho "Common processing failed."
	errex
endif	
\rm fft_cep_comm.t >& /dev/null
setenv USE_ESPS_COMMON off
unsetenv ESPSCOM

# test parameters
# -P -r -l -S -w -o -F -R -e -z 
# -----------------------------
vecho "testing -P option."
echo "int    start  =   513;" > fft_cep_param.t
echo "int    nan    =   512;" >> fft_cep_param.t
$PROG -P fft_cep_param.t -R test.sd test.cep >& /dev/null
if (`hditem -i start test.cep` == 513 && `hditem -i nan test.cep` == 512) then
	vecho "-P option ok."
else
	vecho "-P option failed."
	errex
endif	
\rm fft_cep_param.t >& /dev/null

vecho "testing range."
$PROG -R -r 513:1024 test.sd test.cep >& /dev/null
set l1 = `hditem -i start test.cep`
set l2 = `hditem -i nan test.cep`
echo "int    start  =   513;" > params
echo "int    nan    =   512;" >> params
$PROG -R test.sd test.cep >& /dev/null
set l3 = `hditem -i start test.cep`
set l4 = `hditem -i nan test.cep`
if ( $l1 == 513 && $l3 == 513 && $l2 == 512 && $l4 == 512) then
	vecho "range ok."
else
	vecho "range failed."
	errex
endif
\rm params >& /dev/null

vecho "testing frame_len."
set CMD = "$PROG -R -r 513:1024"
$CMD -l 0 test.sd test.cep >& /dev/null
set l1 = `hditem -i frmlen test.cep`
$CMD -l 128 test.sd test.cep >& /dev/null
set l2 = `hditem -i frmlen test.cep`
echo "int  frame_len  = 128;" > params
$CMD -l 128 test.sd test.cep >& /dev/null
set l3 = `hditem -i frmlen test.cep`
if ( $l1 == 1024 && $l2 == 128 && $l3 == 128 ) then
	vecho "frame length ok."
else
	vecho "frame length failed."
	errex
endif
\rm params >& /dev/null

vecho "testing step size."
set CMD = "$PROG -R -r 513:1024 -l128"
$CMD test.sd test.cep >& /dev/null
set l1 = `hditem -i step test.cep`
$CMD -S 0 test.sd test.cep >& /dev/null
set l2 = `hditem -i step test.cep`
$CMD -S 128 test.sd test.cep >& /dev/null
set l3 = `hditem -i step test.cep`
echo "int  step  =  128;" > params
$CMD test.sd test.cep >& /dev/null
set l4 = `hditem -i step test.cep`
if ($l1 == 128 && $l2 == 128 && $l3 == 128 && $l4 == 128) then
	vecho "step ok."
else
	vecho "step failed."
	errex
endif
\rm params >& /dev/null

vecho "testing window type."
set CMD = "$PROG -R -r 513:1024 -l128"
$CMD test.sd test.cep >& /dev/null
set l1 = `hditem -i window_type test.cep`
$CMD -w TRIANG test.sd test.cep >& /dev/null
set l2 = `hditem -i window_type test.cep`
echo 'string  window_type = "HAMMING";' > params
$CMD test.sd test.cep >& /dev/null
set l3 = `hditem -i window_type test.cep`
if ($l1 == "WT_RECT" && $l2 == "WT_TRIANG" && $l3 == "WT_HAMMING") then
	vecho "window_type ok."
else
	vecho "window_type failed."
	errex
endif
\rm params >& /dev/null

vecho "testing order."
set CMD = "$PROG -R -r 513:1024 -l128"
$CMD test.sd test.cep >& /dev/null
set l1 = `hditem -i order test.cep`
$CMD -o 8 test.sd test.cep >& /dev/null
set l2 = `hditem -i order test.cep`
echo "int order = 8;" > params
$CMD test.sd test.cep >& /dev/null
set l3 = `hditem -i order test.cep`
if ($l1 == 10 && $l2 == 8 && $l3 == 8) then
	vecho "order ok."
else
	vecho "order failed."
	errex
endif
\rm params >& /dev/null

vecho "testing float option."
set CMD = "$PROG -r513:1024 -l128"
$CMD test.sd test.cep >& /dev/null
set l1 = `hditem -i data_type test.cep`
set CMD = "$PROG -r513:1024 -l128 -F"
$CMD test.sd test.cep >& /dev/null
set l2 = `hditem -i data_type test.cep`
set CMD = "$PROG -r513:1024 -l128" 
echo 'string data_type = "FLOAT";' > params
$CMD test.sd test.cep >& /dev/null
set l3 = `hditem -i data_type test.cep`
if ($l1 == "FLOAT_CPLX" && $l2 == "FLOAT" && $l3 == "FLOAT") then
	vecho "float ok."
else
	vecho "float failed."
	errex
endif
\rm params >& /dev/null

vecho "testing method option."
set CMD = "$PROG -r513:1024 -l128"
$CMD test.sd test.cep >& /dev/null
set l1 = `hditem -i cplx_cepstrum test.cep`
set CMD = "$PROG -r513:1024 -l128 -R"
$CMD test.sd test.cep >& /dev/null
set l2 = `hditem -i cplx_cepstrum test.cep`
set CMD = "$PROG -r513:1024 -l128"
echo 'string  method = "CEPSTRUM";' > params
$CMD test.sd test.cep >& /dev/null
set l3 = `hditem -i cplx_cepstrum test.cep`
if ($l1 == "YES" && $l2 == "NO" && $l3 == "NO") then
	vecho "method ok."
else
	vecho "method failed."
	errex
endif
\rm params >& /dev/null

vecho "testing extraction option."
set CMD = "$PROG -r513:1024 -l128"
$CMD test.sd test.cep >& /dev/null
set l1 = `hditem -i element_range test.cep`
set CMD = "$PROG -r513:1024 -l128 -e0:10,20"
$CMD test.sd test.cep >& /dev/null
set l2 = `hditem -i element_range test.cep`
set CMD = "$PROG -r513:1024 -l128"
echo 'string element_range = "0:10,20";' > params
$CMD test.sd test.cep >& /dev/null
set l3 = `hditem -i element_range test.cep`
if ($l1 == "0:1023" && $l2 == "0:10,20" && $l3 == "0:10,20") then
	vecho "extraction ok."
else
	vecho "extraction failed."
	errex
endif
\rm params >& /dev/null

vecho "testing zeroing option."
set CMD = "$PROG -r513:1024 -l128 -z0:10,20"
$CMD test.sd test.cep >& /dev/null
set l1 = `hditem -i zeroing_range test.cep`
set CMD = "$PROG -r513:1024 -l128"
echo 'string zeroing_range = "0:10,20";' > params
$CMD test.sd test.cep >& /dev/null
set l2 = `hditem -i zeroing_range test.cep`
if ($l1 == "0:10,20" && $l2 == "0:10,20") then
	vecho "zeroing ok."
else
	vecho "zeroing failed."
	errex
endif
\rm params >& /dev/null

vecho "testing multichannel input data."
set CMD = "$PROG -r513:1024"
mux test.sd test.sd test.mcsd >& /dev/null
$CMD test.mcsd test.cep  >& /dev/null
(pplain -f cepstrum_0 test.cep > ccep0 ) >& /dev/null
(pplain -f cepstrum_1 test.cep > ccep1 ) >& /dev/null
if ( ! -z ccep0 && ! -z ccep1 ) then
	vecho "multichannel input ok."
else
	vecho "multichannel input failed."
	errex
endif
\rm ccep0 ccep1 test.mcsd >& /dev/null

vecho "Checking timing."
testsd -r 1024 test.sd >& /dev/null
$PROG -r513:+1023 test.sd test.cep >& /dev/null
if (`hditem -i start_time test.cep` == 1) then
	vecho "start_time ok."
else
	vecho "start_time failed."
	errex
endif
$PROG -R -r512:1023 -l512 test.sd test.cep >& /dev/null
if (`hditem -i record_freq test.cep` == 2) then
	vecho "record_freq ok."
else
	vecho "record_freq failed."
	errex
endif

# check functionality
# -------------------
vecho "Checking fftcep: cepstrum."
testsd -f 500 -p 512 test.sd >& /dev/null
$PROG -F -R -l512 -r1:512 -o9 -w HAMMING test.sd test.cep >& /dev/null
make_sd -f cepstrum_0 -S 8000 test.cep test.cepsd >& /dev/null
fft -o9 -l512 -r1:512 test.cepsd test.cspec >& /dev/null
psps -H test.cspec | awk '{if (NR>4) { if (NR==5) {max=$2; freq=$1} \
if (max<=$2) {max=$2;freq=$1}}} END {printf "%d\n", freq}' > test.res
set l1 = `cat test.res`
if ($l1 == 500) then
	vecho "cepstrum computed ok."
else
	vecho "error in computing cepstrum."
	vecho "expecting max value at 500 Hz; actual max at $l1 Hz"
	errex
endif
\rm test.sd test.cep test.cepsd test.cspec test.res >& /dev/null

vecho "Checking fftcep: complex cepstrum."
testsd -f 500 -p 512 test.sd >& /dev/null
$PROG -F -l512 -r1:512 -o9 -w HAMMING test.sd test.cep >& /dev/null
make_sd -f cepstrum_0 -S 8000 test.cep test.cepsd >& /dev/null
fft -c -o9 -l512 -r1:512 test.cepsd test.cspec >& /dev/null
psps -H test.cspec | awk '{if (NR>4) { if (NR==5) {max=$2; freq=$1} \
if (max<=$2) {max=$2;freq=$1}}} END {printf "%d\n", freq}' > test.res
set l1 = `cat test.res`
if ($l1 == 500) then
	vecho "complex cepstrum computed ok."
else
	vecho "error in computing complex cepstrum."
	vecho "expecting max value at 500 Hz; actual max at $l1 Hz"
	errex
endif
\rm test.sd test.cep test.cepsd test.cspec test.res >& /dev/null

if ($VISUAL == 1) then
	echo "Visual Test: removing glottal pulse from voiced speech."
	if ( ! -e /usr/esps/demo/speech.sd ) then 
		echo "Sorry, can't find speech file."
	else
		vecho "plot1: log magnitude of fft spectrum."
	endif
endif
# finished test
# -------------
# \rm param test.cep test.sd params >& /dev/null
\rm test.sdd noise
vecho "$PROG test script passed: `date`"
passex

