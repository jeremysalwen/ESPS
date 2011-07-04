#! /bin/sh
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1998  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)featohtk.tst	1.1	3/17/98	ERL
# 
# Written by:  Rod Johnson
# Checked by:
# Revised by:
# 
# Brief description:  Test script for featohtk program.
#   "featohtk.tst clean" just removes the test files.
#   "featohtk.tst" runs the tests and leaves the test files in place.
# 

## CLEAN UP TEST FILES

case "$1" in
clean)
    rm -f \
	tst[ABCD].fea tst[EF].sd tst[1-9].htk tst1[0-9].htk tst2[0-3].htk \
	htk[1-9].fea htk1[0].fea htk2[0-3].sd \
	cmp[1-9].fea cmp1[0].fea cmp2[013].sd fmt[1-9] fmt1[0] fmt2[0-3]
    exit 0 ;;
esac

## MAKE TEST FEA FILE --- FLOAT parameters

echo "Making test FEA file tstA.fea (FLOAT parameters)."

esig2fea - tstA.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    186
     -1
lpc_power: DOUBLE 1 <r>
raw_power: FLOAT 1 <r>
spec_param: FLOAT 12 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
  [lpc_power] 927.42932
  [raw_power] 1423.2400
  [spec_param]
    [0]      0.28137207    -0.39353985     0.17607225    -0.22316065
    [4]      0.15006225    -0.14152966     0.11980563    -0.10191841
    [8]     0.090150878   -0.084244661    0.062338550   -0.088505752
[Record 1]
  [lpc_power] 2922.3127
  [raw_power] 4258.8999
  [spec_param]
    [0]      0.27897111   -0.049545702     0.24509712    -0.27239212
    [4]    -0.034412865    -0.10871434    -0.19859451   -0.085225292
    [8]     -0.19471112    -0.12768260    -0.15221642     0.12193012
[Record 2]
  [lpc_power] 2058.4460
  [raw_power] 3493.5901
  [spec_param]
    [0]      0.36667153    0.037211671     0.24771327    -0.22217131
    [4]     -0.13403705   -0.092992097    -0.33920684   -0.094665490
    [8]     -0.15213959   -0.093598701    -0.19457534     0.19896972
[Record 3]
  [lpc_power] 9424.0449
  [raw_power] 77480.953
  [spec_param]
    [0]      0.89542192    0.017055569    -0.45795065    -0.29343635
    [4]     -0.31801096   -0.021213403    0.013620988    0.016556688
    [8]      0.14626795    0.039126333    -0.16830522   -0.057130225
aArDvArK

## RUN PROGRAM (1) --- minimal options

echo 'Running command (1):'
echo '    featohtk -k LPREFC -f spec_param tstA.fea tst1.htk'
featohtk -k LPREFC -f spec_param tstA.fea tst1.htk

## CHECK OUTPUT (1)

echo '... checking output.'

## --- HEADER (1)

if od -b tst1.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 060 000 002 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst1.htk'
fi

## --- DATA (1)

## --- Convert to ESPS

cat >fmt1 <<aArDvArK
spec_param FLOAT 12
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(1)" -E -S12 fmt1 tst1.htk htk1.fea 2>/dev/null

## --- Make comparison file

esig2fea - cmp1.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    139
     -1
spec_param: FLOAT 12 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
    [0]      0.28137207    -0.39353985     0.17607225    -0.22316065
    [4]      0.15006225    -0.14152966     0.11980563    -0.10191841
    [8]     0.090150878   -0.084244661    0.062338550   -0.088505752
[Record 1]
    [0]      0.27897111   -0.049545702     0.24509712    -0.27239212
    [4]    -0.034412865    -0.10871434    -0.19859451   -0.085225292
    [8]     -0.19471112    -0.12768260    -0.15221642     0.12193012
[Record 2]
    [0]      0.36667153    0.037211671     0.24771327    -0.22217131
    [4]     -0.13403705   -0.092992097    -0.33920684   -0.094665490
    [8]     -0.15213959   -0.093598701    -0.19457534     0.19896972
[Record 3]
    [0]      0.89542192    0.017055569    -0.45795065    -0.29343635
    [4]     -0.31801096   -0.021213403    0.013620988    0.016556688
    [8]      0.14626795    0.039126333    -0.16830522   -0.057130225
aArDvArK

## --- Compare

if pspsdiff -H htk1.fea cmp1.fea | grep '.*'
then echo '* * * ERROR: bad data in tst1.htk'
fi

## RUN PROGRAM (2) --- contiguous subrange

echo 'Running command (2):'
echo '    featohtk -k LPREFC -f spec_param"[4:7]" tstA.fea tst2.htk'
featohtk -k LPREFC -f spec_param"[4:7]" tstA.fea tst2.htk

## CHECK OUTPUT (2)

echo '... checking output.'

## --- HEADER (2)

if od -b tst2.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 020 000 002 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst2.htk'
fi

## --- DATA (2)

## --- Convert to ESPS

cat >fmt2 <<aArDvArK
spec_param FLOAT 4
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(2)" -E -S12 fmt2 tst2.htk htk2.fea 2>/dev/null

## --- Make comparison file

esig2fea - cmp2.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    138
     -1
spec_param: FLOAT 4 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
    [0]      0.15006225    -0.14152966     0.11980563    -0.10191841
[Record 1]
    [0]    -0.034412865    -0.10871434    -0.19859451   -0.085225292
[Record 2]
    [0]     -0.13403705   -0.092992097    -0.33920684   -0.094665490
[Record 3]
    [0]     -0.31801096   -0.021213403    0.013620988    0.016556688
aArDvArK

## --- Compare

if pspsdiff -H htk2.fea cmp2.fea | grep '.*'
then echo '* * * ERROR: bad data in tst2.htk'
fi

## RUN PROGRAM (3) --- noncontiguous subrange

echo 'Running command (3):'
echo '    featohtk -k LPREFC -f spec_param"[2:3,6:11]" tstA.fea tst3.htk'
featohtk -k LPREFC -f spec_param"[2:3,6:11]" tstA.fea tst3.htk

## CHECK OUTPUT (3)

echo '... checking output.'

## --- HEADER (3)

if od -b tst3.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 040 000 002 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst3.htk'
fi

## --- DATA (3)

## --- Convert to ESPS

cat >fmt3 <<aArDvArK
spec_param FLOAT 8
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(3)" -E -S12 fmt3 tst3.htk htk3.fea 2>/dev/null

## --- Make comparison file

esig2fea - cmp3.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    138
     -1
spec_param: FLOAT 8 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
    [0]      0.17607225    -0.22316065     0.11980563    -0.10191841
    [4]     0.090150878   -0.084244661    0.062338550   -0.088505752
[Record 1]
    [0]      0.24509712    -0.27239212    -0.19859451   -0.085225292
    [4]     -0.19471112    -0.12768260    -0.15221642     0.12193012
[Record 2]
    [0]      0.24771327    -0.22217131    -0.33920684   -0.094665490
    [4]     -0.15213959   -0.093598701    -0.19457534     0.19896972
[Record 3]
    [0]     -0.45795065    -0.29343635    0.013620988    0.016556688
    [4]      0.14626795    0.039126333    -0.16830522   -0.057130225
aArDvArK

## --- Compare

if pspsdiff -H htk3.fea cmp3.fea | grep '.*'
then echo '* * * ERROR: bad data in tst3.htk'
fi

## RUN PROGRAM (4) --- contiguous subrange, Energy parameter (FLOAT)

echo 'Running command (4):'
echo '    featohtk ' \
	'-k LPREFC -f spec_param"[0:2]" -E raw_power tstA.fea tst4.htk'
featohtk -k LPREFC -f spec_param"[0:2]" -E raw_power tstA.fea tst4.htk

## CHECK OUTPUT (4)

echo '... checking output.'

## --- HEADER (4)

if od -b tst4.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 020 000 102 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst4.htk'
fi

## --- DATA (4)

## --- Convert to ESPS

cat >fmt4 <<aArDvArK
spec_param FLOAT 3
raw_power FLOAT 1
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(4)" -E -S12 fmt4 tst4.htk htk4.fea 2>/dev/null

## --- Make comparison file

esig2fea - cmp4.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    161
     -1
spec_param: FLOAT 3 <r>
raw_power: FLOAT 1 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
    [0]      0.28137207    -0.39353985     0.17607225
    [0]       1423.2400
[Record 1]
    [0]      0.27897111   -0.049545702     0.24509712
    [0]       4258.8999
[Record 2]
    [0]      0.36667153    0.037211671     0.24771327
    [0]       3493.5901
[Record 3]
    [0]      0.89542192    0.017055569    -0.45795065
    [0]       77480.953
aArDvArK

## --- Compare

if pspsdiff -H htk4.fea cmp4.fea | grep '.*'
then echo '* * * ERROR: bad data in tst4.htk'
fi

## RUN PROGRAM (5) --- noncontiguous subrange,
#	0-order cepstral parameter (DOUBLE)

echo 'Running command (5):'
echo '    featohtk ' \
	'-k MFCC -f spec_param"[0:3,6:8]" -O lpc_power tstA.fea tst5.htk'
featohtk -k MFCC -f spec_param"[0:3,6:8]" -O lpc_power tstA.fea tst5.htk

## CHECK OUTPUT (5)

echo '... checking output.'

## --- HEADER (5)

if od -b tst5.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 040 040 006 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst5.htk'
fi

## --- DATA (5)

## --- Convert to ESPS

cat >fmt5 <<aArDvArK
spec_param FLOAT 7
lpc_power FLOAT 1
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(5)" -E -S12 fmt5 tst5.htk htk5.fea 2>/dev/null

## --- Make comparison file

esig2fea - cmp5.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    161
     -1
spec_param: FLOAT 7 <r>
lpc_power: FLOAT 1 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
    [0]      0.28137207    -0.39353985     0.17607225    -0.22316065
    [4]      0.11980563    -0.10191841    0.090150878
    [0]       927.42932
[Record 1]
    [0]      0.27897111   -0.049545702     0.24509712    -0.27239212
    [4]     -0.19859451   -0.085225292    -0.19471112
    [0]       2922.3127
[Record 2]
    [0]      0.36667153    0.037211671     0.24771327    -0.22217131
    [4]     -0.33920684   -0.094665490    -0.15213959
    [0]       2058.4460
[Record 3]
    [0]      0.89542192    0.017055569    -0.45795065    -0.29343635
    [4]     0.013620988    0.016556688     0.14626795
    [0]       9424.0449
aArDvArK

## --- Compare

if pspsdiff -H htk5.fea cmp5.fea | grep '.*'
then echo '* * * ERROR: bad data in tst5.htk'
fi

## MAKE TEST FEA FILE --- DOUBLE parameters

echo "Making test FEA file tstB.fea (DOUBLE parameters)."

esig2fea - tstB.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    186
     -1
raw_power: DOUBLE 1 <r>
lpc_power: FLOAT 1 <r>
spec_param: FLOAT 12 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
  [raw_power] 1423.2400
  [lpc_power] 927.42932
  [spec_param]
    [0]      0.28137207    -0.39353985     0.17607225    -0.22316065
    [4]      0.15006225    -0.14152966     0.11980563    -0.10191841
    [8]     0.090150878   -0.084244661    0.062338550   -0.088505752
[Record 1]
  [raw_power] 4258.8999
  [lpc_power] 2922.3127
  [spec_param]
    [0]      0.27897111   -0.049545702     0.24509712    -0.27239212
    [4]    -0.034412865    -0.10871434    -0.19859451   -0.085225292
    [8]     -0.19471112    -0.12768260    -0.15221642     0.12193012
[Record 2]
  [raw_power] 3493.5901
  [lpc_power] 2058.4460
  [spec_param]
    [0]      0.36667153    0.037211671     0.24771327    -0.22217131
    [4]     -0.13403705   -0.092992097    -0.33920684   -0.094665490
    [8]     -0.15213959   -0.093598701    -0.19457534     0.19896972
[Record 3]
  [raw_power] 77480.953
  [lpc_power] 9424.0449
  [spec_param]
    [0]      0.89542192    0.017055569    -0.45795065    -0.29343635
    [4]     -0.31801096   -0.021213403    0.013620988    0.016556688
    [8]      0.14626795    0.039126333    -0.16830522   -0.057130225
aArDvArK

## RUN PROGRAM (6) --- convert param type

echo 'Running command (6):'
echo '    featohtk -k LPREFC -f spec_param tstB.fea tst6.htk'
featohtk -k LPREFC -f spec_param tstB.fea tst6.htk

## CHECK OUTPUT (6)

echo '... checking output.'

## --- HEADER (6)

if od -b tst6.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 060 000 002 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst6.htk'
fi

## --- DATA (6)

## --- Convert to ESPS

cat >fmt6 <<aArDvArK
spec_param FLOAT 12
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(6)" -E -S12 fmt6 tst6.htk htk6.fea 2>/dev/null

## --- Make comparison file

esig2fea - cmp6.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    139
     -1
spec_param: FLOAT 12 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
    [0]      0.28137207    -0.39353985     0.17607225    -0.22316065
    [4]      0.15006225    -0.14152966     0.11980563    -0.10191841
    [8]     0.090150878   -0.084244661    0.062338550   -0.088505752
[Record 1]
    [0]      0.27897111   -0.049545702     0.24509712    -0.27239212
    [4]    -0.034412865    -0.10871434    -0.19859451   -0.085225292
    [8]     -0.19471112    -0.12768260    -0.15221642     0.12193012
[Record 2]
    [0]      0.36667153    0.037211671     0.24771327    -0.22217131
    [4]     -0.13403705   -0.092992097    -0.33920684   -0.094665490
    [8]     -0.15213959   -0.093598701    -0.19457534     0.19896972
[Record 3]
    [0]      0.89542192    0.017055569    -0.45795065    -0.29343635
    [4]     -0.31801096   -0.021213403    0.013620988    0.016556688
    [8]      0.14626795    0.039126333    -0.16830522   -0.057130225
aArDvArK

## --- Compare

if pspsdiff -H htk6.fea cmp6.fea | grep '.*'
then echo '* * * ERROR: bad data in tst6.htk'
fi

## RUN PROGRAM (7) --- contiguous subrange, convert param type

echo 'Running command (7):'
echo '    featohtk -k LPREFC -f spec_param"[4:7]" tstB.fea tst7.htk'
featohtk -k LPREFC -f spec_param"[4:7]" tstB.fea tst7.htk

## CHECK OUTPUT (7)

echo '... checking output.'

## --- HEADER (7)

if od -b tst7.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 020 000 002 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst7.htk'
fi

## --- DATA (7)

## --- Convert to ESPS

cat >fmt7 <<aArDvArK
spec_param FLOAT 4
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(7)" -E -S12 fmt7 tst7.htk htk7.fea 2>/dev/null

## --- Make comparison file

esig2fea - cmp7.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    138
     -1
spec_param: FLOAT 4 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
    [0]      0.15006225    -0.14152966     0.11980563    -0.10191841
[Record 1]
    [0]    -0.034412865    -0.10871434    -0.19859451   -0.085225292
[Record 2]
    [0]     -0.13403705   -0.092992097    -0.33920684   -0.094665490
[Record 3]
    [0]     -0.31801096   -0.021213403    0.013620988    0.016556688
aArDvArK

## --- Compare

if pspsdiff -H htk7.fea cmp7.fea | grep '.*'
then echo '* * * ERROR: bad data in tst7.htk'
fi

## RUN PROGRAM (8) --- noncontiguous subrange, convert param type

echo 'Running command (8):'
echo '    featohtk -k LPREFC -f spec_param"[2:3,6:11]" tstB.fea tst8.htk'
featohtk -k LPREFC -f spec_param"[2:3,6:11]" tstB.fea tst8.htk

## CHECK OUTPUT (8)

echo '... checking output.'

## --- HEADER (8)

if od -b tst8.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 040 000 002 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst8.htk'
fi

## --- DATA (8)

## --- Convert to ESPS

cat >fmt8 <<aArDvArK
spec_param FLOAT 8
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(8)" -E -S12 fmt8 tst8.htk htk8.fea 2>/dev/null

## --- Make comparison file

esig2fea - cmp8.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    138
     -1
spec_param: FLOAT 8 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
    [0]      0.17607225    -0.22316065     0.11980563    -0.10191841
    [4]     0.090150878   -0.084244661    0.062338550   -0.088505752
[Record 1]
    [0]      0.24509712    -0.27239212    -0.19859451   -0.085225292
    [4]     -0.19471112    -0.12768260    -0.15221642     0.12193012
[Record 2]
    [0]      0.24771327    -0.22217131    -0.33920684   -0.094665490
    [4]     -0.15213959   -0.093598701    -0.19457534     0.19896972
[Record 3]
    [0]     -0.45795065    -0.29343635    0.013620988    0.016556688
    [4]      0.14626795    0.039126333    -0.16830522   -0.057130225
aArDvArK

## --- Compare

if pspsdiff -H htk8.fea cmp8.fea | grep '.*'
then echo '* * * ERROR: bad data in tst8.htk'
fi

## RUN PROGRAM (9) --- contiguous subrange,
#	Energy parameter (DOUBLE) & 0-order cepstral parameter (FLOAT),
#	convert param type

echo 'Running command (9):'
echo '    featohtk -k MFCC ' \
	'-f spec_param"[0:2]" -E raw_power -O lpc_power tstB.fea tst9.htk'
featohtk -k MFCC \
	-f spec_param"[0:2]" -E raw_power -O lpc_power tstB.fea tst9.htk

## CHECK OUTPUT (9)

echo '... checking output.'

## --- HEADER (9)

if od -b tst9.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 024 040 106 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst9.htk'
fi

## --- DATA (9)

## --- Convert to ESPS

cat >fmt9 <<aArDvArK
spec_param FLOAT 3
lpc_power FLOAT 1
raw_power FLOAT 1
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(9)" -E -S12 fmt9 tst9.htk htk9.fea 2>/dev/null

## --- Make comparison file

esig2fea - cmp9.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    138
     -1
spec_param: FLOAT 3 <r>
lpc_power: FLOAT 1 <r>
raw_power: FLOAT 1 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
    [0]      0.28137207    -0.39353985     0.17607225
    [0]       927.42932
    [0]       1423.2400
[Record 1]
    [0]      0.27897111   -0.049545702     0.24509712
    [0]       2922.3127
    [0]       4258.8999
[Record 2]
    [0]      0.36667153    0.037211671     0.24771327
    [0]       2058.4460
    [0]       3493.5901
[Record 3]
    [0]      0.89542192    0.017055569    -0.45795065
    [0]       9424.0449
    [0]       77480.953
aArDvArK

## --- Compare

if pspsdiff -H htk9.fea cmp9.fea | grep '.*'
then echo '* * * ERROR: bad data in tst9.htk'
fi

## RUN PROGRAM (10) --- noncontiguous subrange,
#	Energy parameter (DOUBLE), compute log, convert param type

echo 'Running command (10):'
echo '    featohtk ' \
	'-k LPREFC -f spec_param"[0:3,6:8]" -E raw_power -L tstB.fea tst10.htk'
featohtk -k LPREFC -f spec_param"[0:3,6:8]" -E raw_power -L tstB.fea tst10.htk

## CHECK OUTPUT (10)

echo '... checking output.'

## --- HEADER (10)

if od -b tst10.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 040 000 102 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst10.htk'
fi

## --- DATA (10)

## --- Convert to ESPS

cat >fmt10 <<aArDvArK
spec_param FLOAT 7
raw_power FLOAT 1
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(10)" -E -S12 fmt10 tst10.htk htk10.fea 2>/dev/null

## --- Make comparison file

esig2fea - cmp10.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    161
     -1
spec_param: FLOAT 7 <r>
raw_power: FLOAT 1 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
    [0]      0.28137207    -0.39353985     0.17607225    -0.22316065
    [4]      0.11980563    -0.10191841    0.090150878
    [0]       7.2606912
[Record 1]
    [0]      0.27897111   -0.049545702     0.24509712    -0.27239212
    [4]     -0.19859451   -0.085225292    -0.19471112
    [0]       8.3567662
[Record 2]
    [0]      0.36667153    0.037211671     0.24771327    -0.22217131
    [4]     -0.33920684   -0.094665490    -0.15213959
    [0]       8.1586852
[Record 3]
    [0]      0.89542192    0.017055569    -0.45795065    -0.29343635
    [4]     0.013620988    0.016556688     0.14626795
    [0]       11.257788
aArDvArK

## --- Compare

if pspsdiff -H htk10.fea cmp10.fea | grep '.*'
then echo '* * * ERROR: bad data in tst10.htk'
fi

## MAKE TEST FEA FILE --- SHORT parameters

echo "Making test FEA file tstC.fea (SHORT parameters)."

esig2fea - tstC.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    185
     -1
lpc_power: FLOAT 1 <r>
raw_power: FLOAT 1 <r>
spec_param: SHORT 12 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
  [lpc_power] 927.42932
  [raw_power] 1423.2400
  [spec_param]
    [0]       2813      3935      1760      2231
    [4]       1500      1415      1198      1019
    [8]        901       842       623       885
[Record 1]
  [lpc_power] 2922.3127
  [raw_power] 4258.8999
  [spec_param]
    [0]       2789       495      2450      2723
    [4]        344      1087      1985       852
    [8]       1947      1276      1522      1219
[Record 2]
  [lpc_power] 2058.4460
  [raw_power] 3493.5901
  [spec_param]
    [0]       3666       372      2477      2221
    [4]       1340       929      3392       946
    [8]       1521       935      1945      1989
[Record 3]
  [lpc_power] 9424.0449
  [raw_power] 77480.953
  [spec_param]
    [0]       8954       170      4579      2934
    [4]       3180       212       136       165
    [8]       1462       391      1683       571
aArDvArK

## CHECK SUPPORTED PARM KINDS

## RUN PROGRAM (11) --- parmKind LPC

echo 'Running command (11):'
echo '    featohtk -k LPC -f spec_param tstA.fea tst11.htk'
featohtk -k LPC -f spec_param tstA.fea tst11.htk

## CHECK OUTPUT HEADER (11)

echo '... checking output header.'

if od -b tst11.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 060 000 001 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst11.htk'
fi

## RUN PROGRAM (12) --- parmKind LPREFC

echo 'Running command (12):'
echo '    featohtk -k LPREFC -f spec_param tstA.fea tst12.htk'
featohtk -k LPREFC -f spec_param tstA.fea tst12.htk

## CHECK OUTPUT HEADER (12)

echo '... checking output header.'

if od -b tst12.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 060 000 002 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst12.htk'
fi

## RUN PROGRAM (13) --- parmKind LPCEPSTRA

echo 'Running command (13):'
echo '    featohtk -k LPCEPSTRA -f spec_param tstA.fea tst13.htk'
featohtk -k LPCEPSTRA -f spec_param tstA.fea tst13.htk

## CHECK OUTPUT HEADER (13)

echo '... checking output header.'

if od -b tst13.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 060 000 003 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst13.htk'
fi

## RUN PROGRAM (14) --- parmKind IREFC

echo 'Running command (14):'
echo '    featohtk -k IREFC -f spec_param tstC.fea tst14.htk'
featohtk -k IREFC -f spec_param tstC.fea tst14.htk

## CHECK OUTPUT HEADER (14)

echo '... checking output header.'

if od -b tst14.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 030 000 005 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst14.htk'
fi

## RUN PROGRAM (15) --- parmKind MFCC

echo 'Running command (15):'
echo '    featohtk -k MFCC -f spec_param tstA.fea tst15.htk'
featohtk -k MFCC -f spec_param tstA.fea tst15.htk

## CHECK OUTPUT HEADER (15)

echo '... checking output header.'

if od -b tst15.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 060 000 006 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst15.htk'
fi

## RUN PROGRAM (16) --- parmKind FBANK

echo 'Running command (16):'
echo '    featohtk -k FBANK -f spec_param tstA.fea tst16.htk'
featohtk -k FBANK -f spec_param tstA.fea tst16.htk

## CHECK OUTPUT HEADER (16)

echo '... checking output header.'

if od -b tst16.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 060 000 007 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst16.htk'
fi

## RUN PROGRAM (17) --- parmKind MELSPEC

echo 'Running command (17):'
echo '    featohtk -k MELSPEC -f spec_param tstA.fea tst17.htk'
featohtk -k MELSPEC -f spec_param tstA.fea tst17.htk

## CHECK OUTPUT HEADER (17)

echo '... checking output header.'

if od -b tst17.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 060 000 010 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst17.htk'
fi

## RUN PROGRAM (18) --- parmKind USER

echo 'Running command (18):'
echo '    featohtk -k USER -f spec_param tstA.fea tst18.htk'
featohtk -k USER -f spec_param tstA.fea tst18.htk

## CHECK OUTPUT HEADER (18)

echo '... checking output header.'

if od -b tst18.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 060 000 011 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst18.htk'
fi

## RUN PROGRAM (19) --- parmKind DISCRETE

echo 'Running command (19):'
echo '    featohtk -k DISCRETE -f spec_param tstC.fea tst19.htk'
featohtk -k DISCRETE -f spec_param tstC.fea tst19.htk

## CHECK OUTPUT HEADER (19)

echo '... checking output header.'

if od -b tst19.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 030 000 012 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst19.htk'
fi

## RUN PROGRAM (20) --- parmKind WAVEFORM

echo 'Running command (20):'
echo '    featohtk -k WAVEFORM -f "spec_param[0]" tstC.fea tst20.htk'
featohtk -k WAVEFORM -f "spec_param[0]" tstC.fea tst20.htk

## CHECK OUTPUT (20)

echo '... checking output.'

## --- HEADER (20)

if od -b tst20.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 002 000 000 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst20.htk'
fi

## --- DATA (20)

## --- Convert to ESPS

cat >fmt20 <<aArDvArK
samples SHORT 1
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(20)" \
	-t FEA_SD -E -S12 fmt20 tst20.htk htk20.sd 2>/dev/null

## --- Make comparison file

esig2fea - cmp20.sd <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    323
     -1
samples: SHORT 1 <r>
record_freq: DOUBLE 1 <g> 8000
start_time: DOUBLE 1 <g> 0
FeaSubtype: SHORT <g> 8
FeaSubtype.enumStrings: CHAR 10 10 <g> "NONE\0\0\0\0\0\0FEA_VQ\0\0\0\0FEA_ANA\0\0\0FEA_STAT\0\0FEA_QHIST\0FEA_DST\0\0\0FEA_2KB\0\0\0FEA_SPEC\0\0FEA_SD\0\0\0\0FEA_FILT\0\0"
    [0]       2813
    [0]       2789
    [0]       3666
    [0]       8954
aArDvArK

## --- Compare

if pspsdiff -H htk20.sd cmp20.sd | grep '.*'
then echo '* * * ERROR: bad data in tst20.htk'
fi

## MAKE TEST FEA FILE --- DOUBLE parameters, larger values

echo "Making test FEA file tstD.fea (DOUBLE parameters)."

esig2fea - tstD.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    186
     -1
spec_param: DOUBLE 12 <r>
lpc_power: FLOAT 1 <r>
raw_power: FLOAT 1 <r>
record_freq: DOUBLE 1 <g> 160.0
start_time: DOUBLE 1 <g> 0.068750
[Record 0]
  [spec_param]
    [0]       2813      3935      1760      2231
    [4]       1500      1415      1198      1019
    [8]        901       842       623       885
  [lpc_power] 927.42932
  [raw_power] 1423.2400
[Record 1]
  [spec_param]
    [0]       2789       495      2450      2723
    [4]        344      1087      1985       852
    [8]       1947      1276      1522      1219
  [lpc_power] 2922.3127
  [raw_power] 4258.8999
[Record 2]
  [spec_param]
    [0]       3666       372      2477      2221
    [4]       1340       929      3392       946
    [8]       1521       935      1945      1989
  [lpc_power] 2058.4460
  [raw_power] 3493.5901
[Record 3]
  [spec_param]
    [0]       8954       170      4579      2934
    [4]       3180       212       136       165
    [8]       1462       391      1683       571
  [lpc_power] 9424.0449
  [raw_power] 77480.953
aArDvArK

## RUN PROGRAM (21) --- parmKind WAVEFORM

echo 'Running command (21):'
echo '    featohtk -k WAVEFORM -f "spec_param[1]" tstD.fea tst21.htk'
featohtk -k WAVEFORM -f "spec_param[1]" tstD.fea tst21.htk

## CHECK OUTPUT (21)

echo '... checking output.'

## --- HEADER (21)

if od -b tst21.htk \
	| head -1 \
	| grep ' 000 000 000 004 000 000 364 044 000 002 000 000 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst21.htk'
fi

## --- DATA (21)

## --- Convert to ESPS

cat >fmt21 <<aArDvArK
samples SHORT 1
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(21)" \
	-t FEA_SD -E -S12 fmt21 tst21.htk htk21.sd 2>/dev/null

## --- Make comparison file

esig2fea - cmp21.sd <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    323
     -1
samples: SHORT 1 <r>
record_freq: DOUBLE 1 <g> 8000
start_time: DOUBLE 1 <g> 0
FeaSubtype: SHORT <g> 8
FeaSubtype.enumStrings: CHAR 10 10 <g> "NONE\0\0\0\0\0\0FEA_VQ\0\0\0\0FEA_ANA\0\0\0FEA_STAT\0\0FEA_QHIST\0FEA_DST\0\0\0FEA_2KB\0\0\0FEA_SPEC\0\0FEA_SD\0\0\0\0FEA_FILT\0\0"
    [0]       3935
    [0]        495
    [0]        372
    [0]        170
aArDvArK

## --- Compare

if pspsdiff -H htk21.sd cmp21.sd | grep '.*'
then echo '* * * ERROR: bad data in tst21.htk'
fi

## MAKE TEST FEA_SD FILE --- SHORT samples

echo "Making test FEA_SD file tstE.sd (SHORT samples)."

esig2fea - tstE.sd <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    323
     -1
samples: SHORT 1 <r>
record_freq: DOUBLE 1 <g> 8000
start_time: DOUBLE 1 <g> 0
FeaSubtype: SHORT <g> 8
FeaSubtype.enumStrings: CHAR 10 10 <g> "NONE\0\0\0\0\0\0FEA_VQ\0\0\0\0FEA_ANA\0\0\0FEA_STAT\0\0FEA_QHIST\0FEA_DST\0\0\0FEA_2KB\0\0\0FEA_SPEC\0\0FEA_SD\0\0\0\0FEA_FILT\0\0"
  [   0]   418   832  1562  2000  2078  2062  1856  1711  1397  1234
  [  10]  1014   591    49  -687 -1344 -2050 -2500 -2546 -2434 -2088
  [  20] -1658 -1224  -879  -572  -123   269   767  1253  1614  1799
  [  30]  1729  1508  1109   759   441   208   127   -63  -262  -556
  [  40]  -905 -1133 -1216 -1070  -812  -459  -159   -57  -153  -405
  [  50]  -598  -721  -757  -738  -476   314   920  1521  2283  2355
  [  60]  2310  2055  1761  1520  1146  1021   576   -68  -832 -1628
  [  70] -2325 -2923 -2914 -2691 -2317 -1589 -1019  -534  -178   206
  [  80]   591   922  1446  1748  1905  1797  1426   946   326   -75
  [  90]  -375  -450  -312  -296  -238  -371  -560  -705  -826  -641
  [ 100]  -420  -176    20    12  -178  -558  -853 -1155 -1330 -1269
  [ 110]  -773   448  1270  2205  3166  2974  2833  2286  1796  1467
  [ 120]   908   796   163  -699 -1586 -2541 -3275 -3699 -3284 -2699
  [ 130] -1933  -773  -150   397   737   947  1227  1383  1808  1909
  [ 140]  1722  1378   637  -154  -848 -1148 -1112  -855  -282   102
  [ 150]   233   226    52  -137  -203   -89    51   105    13  -261
  [ 160]  -659 -1103 -1429 -1586 -1652 -1550 -1133   151  1543  2446
  [ 170]  3966  4221  3561  3295  2197  1350   727   172  -526 -1547
  [ 180] -2291 -3480 -4167 -4075 -3884 -2671 -1373  -174   969  1447
  [ 190]  1800  1775  1742  1850  1665  1632  1138   353  -425 -1384
  [ 200] -1882 -1989 -1515  -780   153  1103  1330  1463  1243   649
  [ 210]   352    64  -190  -381  -578  -944 -1390 -1601 -1891 -1990
  [ 220] -1759 -1601 -1304  -681   959  2799  3609  5429  5346  3799
  [ 230]  3370  1549   259  -351  -859 -1886 -2742 -3281 -4535 -4754
  [ 240] -3737 -3023 -1293   801  1636  2377  2605  2247  1588  1552
  [ 250]  1343   598   493  -355 -1654 -2042 -2490 -2492 -1527  -206
  [ 260]   872  1919  2847  2348  1931  1531   348  -123  -368  -999
  [ 270] -1305 -1360 -1772 -1896 -1552 -1601 -1406  -865  -846  -776
  [ 280]  -428   -20  1866  3507  4078  5717  4298  2797  1858   -84
  [ 290]  -633 -1014 -1164 -2288 -2689 -3584 -4484 -3612 -2505 -1184
  [ 300]   946  2035  2292  2183  1808   994   703   979   153    46
  [ 310]  -748 -1887 -2379 -2371 -1896  -867   622  1436  2099  2507
  [ 320]  2242  1928  1608   944   139  -485 -1272 -1902 -1832 -1755
  [ 330] -1423  -851  -771  -778  -780  -832  -838  -620  -603  -597
  [ 340]   883  2700  2782  4813  4508  2357  2638   501  -157  -338
  [ 350]  -631 -1869 -2278 -2990 -4301 -3250 -2361 -1439   703  1585
  [ 360]  1655  1977  1572  1023   857  1098   -57   -81 -1091 -2136
  [ 370] -2260 -2141 -1592  -538   642   884  1603  1908  1855  2222
  [ 380]  2183  1406   830  -288 -1374 -1722 -1731 -1520  -834  -541
  [ 390]  -650  -623  -976 -1075  -849  -597  -712  -782 -1206  -490
  [ 400]  1856  2124  4546  5183  3053  3285   988    93  -160  -207
  [ 410] -1424 -1501 -2843 -4155 -3649 -3219 -1935   176  1397  1708
  [ 420]  2313  1411  1092   927   873   243   416  -978 -1643 -2283
  [ 430] -2569 -2035  -977   -23   734  1579  1334  1874  1828  2041
  [ 440]  2182  1892  1041   149  -959 -1706 -1700 -1563 -1141  -774
  [ 450]  -831 -1066 -1155 -1291 -1041  -792  -638  -718  -860  -807
  [ 460]   963  1728  2975  4922  3259  3706  2079  1085   346   446
  [ 470]  -728  -679 -1488 -2993 -2895 -3522 -2934 -2054  -417  -299
  [ 480]  1462   764  1202   756   989   449  1145   504   234  -138
  [ 490] -1088 -1318 -1445 -1072  -800   291   151   847   791  1059
  [ 500]  1048  1739  1441  1670  1232   457  -161  -735 -1249 -1283
  [ 510] -1279 -1453 -1201 -1451 -1408 -1295 -1069  -986  -570  -716
  [ 520]  -579    85  1351  1630  3438  3418  2924  2940  1637  1180
  [ 530]   992   831    39   488 -1321 -1627 -2650 -3029 -3272 -2236
  [ 540] -2176 -1057  -499  -403    37   241   362   896  1485  1346
  [ 550]  1859  1288   818   296   -74  -677  -351  -622  -549  -430
  [ 560]  -601  -546  -246   -89   567  1008  1305  1414  1139   606
  [ 570]   177  -245  -603  -578  -809  -931 -1145 -1388 -1581 -1515
  [ 580] -1521 -1331 -1202 -1038  -159  1011  1535  3145  3200  3223
  [ 590]  3128  2447  1866  1725  1141   421   214 -1298 -1938 -2888
  [ 600] -3438 -3652 -2875 -2725 -1402  -848  -197   243   755   753
  [ 610]  1432  1599  1791  1948  1635  1041   588  -222  -751  -914
  [ 620] -1193 -1086  -906  -859  -684  -226   -90   618  1109  1397
  [ 630]  1715  1640  1131   854   145  -425  -684 -1106 -1294 -1291
  [ 640] -1548 -1602 -1497 -1621 -1416 -1226 -1121  -659   573  1161
  [ 650]  2386  3437  3142  3434  2651  2046  1444  1276   278   356
  [ 660]  -629 -1414 -2090 -2736 -3361 -2893 -2696 -1930  -912  -342
  [ 670]   227   687   763   812  1205  1018  1342  1264  1078   617
  [ 680]   340  -564  -709 -1131 -1099  -894  -445  -313   197   352
  [ 690]   442   865  1000  1224  1409  1259   802   501  -319  -705
  [ 700] -1047 -1240 -1238 -1126 -1252 -1171 -1211 -1350 -1174 -1189
  [ 710] -1093  -773   303  1018  2121  3228  3065  3280  2586  2068
  [ 720]  1313  1248   326   355  -444 -1042 -1786 -2253 -2971 -2691
  [ 730] -2586 -2047 -1121  -611    82   364   665   443   897   601
  [ 740]  1047   988  1152   776   719   -40  -296  -702  -879  -795
  [ 750]  -564  -332   -68   284   257   634   787   945  1176  1200
  [ 760]   968   703   195  -392  -676 -1093 -1168 -1238 -1266 -1364
  [ 770] -1288 -1421 -1324 -1247 -1200  -927  -219   901  1573  3074
  [ 780]  3159  3376  2938  2339  1522  1192   611   128    -2  -935
  [ 790] -1315 -2170 -2508 -3113 -2465 -2587 -1342  -912    58   307
  [ 800]   874   648   862   813   805   977   961   799   570   124
  [ 810]  -443  -777 -1070 -1067  -732  -368    41   501   700   875
  [ 820]  1135  1148  1238  1266   964   614   136  -534  -941 -1247
  [ 830] -1420 -1386 -1282 -1298 -1179 -1161 -1158 -1049 -1022  -900
  [ 840]  -542   573  1219  2474  3283  3154  3202  2410  1865   963
  [ 850]   976  -123   221  -772 -1044 -1877 -2188 -2923 -2526 -2371
  [ 860] -1697  -745  -187   335   640   574   481   595   495   667
  [ 870]   822   680   511   272  -329  -528  -788  -806  -540  -168
  [ 880]   110   526   780   792  1107  1099  1136  1211   959   601
  [ 890]   178  -412  -928 -1143 -1416 -1397 -1314 -1351 -1252 -1210
  [ 900] -1215 -1071  -996  -932  -712   181  1217  1990  3429  3179
  [ 910]  3298  2478  1924  1018   930   342   -13  -176 -1238 -1592
  [ 920] -2337 -2716 -2724 -2129 -1768  -644   -62   283   399   684
  [ 930]   228   498   491   587   861   684   266   -64  -714 -1037
  [ 940]  -808  -669   -81   478   643   768   982   821  1117  1562
  [ 950]  1465  1491   954   -94  -793 -1410 -1792 -1454 -1316 -1249
  [ 960] -1027 -1299 -1375 -1079  -972  -575  -305  -596  -813  -280
  [ 970]  1583  2308  4320  4732  3214  2663  1009    69   270   242
  [ 980]  -880  -860 -2945 -3638 -3429 -2670 -1376   765   778  1169
  [ 990]   954   272   587  1155   809   741  -203 -1821 -2131 -2254
  [1000] -1746  -388   281   299   710   376   748  1714  2105  2224
  [1010]  2062   802   -92  -197  -510  -150   122  -599  -911 -1189
  [1020] -1430  -675  -257  -295  -213  -783 -1166  -856  -845  -703
  [1030]  -798 -1653 -2196 -1430  1816  3315  4754  5574  2447  2217
  [1040]  1687  1347  1693  1115 -2393 -2972 -4665 -4252 -2178  -884
  [1050]  -948    76  -606    89  2117  2448  2492  1801  -662 -1677
  [1060] -1489 -1887 -1016 -1120 -2079 -1742  -862    79  2065  2464
  [1070]  1915  1764  1185  1074  1760  1165   107  -445 -1295 -1169
  [1080]  -203  -224  -264  -477 -1032  -597   -79   -95   -79  -673
  [1090] -1232 -1080 -1020  -996 -1237 -2071 -2350 -1373  2504  4442
  [1100]  3763  5056  1955  2479  3636  2751   931  -555 -4230 -3378
  [1110] -2749 -2665 -2179 -2662 -2875  -316  1531  2301  2975  1144
  [1120]   584  1183   684    -8  -920 -3218 -2847 -1924 -1410  -465
  [1130]  -402  -686  1068  2168  2569  2868  1569   740  1166   739
  [1140]   281  -410 -1402  -970  -267  -155   -85  -503  -710    60
  [1150]   190  -118  -466 -1041  -925  -503  -911 -1252 -1639 -2037
  [1160] -1698 -1320  -336  3496  3506  2350  4829  2274  3810  4002
  [1170]   984  -463  -487 -3334 -1409 -2775 -4548 -3059 -2004 -1579
  [1180]  1003   377   257  2612  1996  1900  1790  -590  -891  -155
  [1190] -2146 -1893 -2131 -2667  -975  -106  -648  1026  1420  1818
  [1200]  3047  2075  1234  1790   943   402    90 -1079  -744  -466
  [1210] -1088  -834  -673  -723    28  -305  -693  -164  -288  -357
  [1220]  -343 -1139 -1081 -1002 -1792 -1823 -1594  -612  3692  2502
  [1230]   558  5055  2884  4319  3947   454   356  1863 -2520 -1777
  [1240] -3109 -4368 -2248 -2266 -3783  -714   124    84  2221   998
  [1250]  1246  2974  1080   -64   750 -1451 -1020 -1325 -2723 -1771
  [1260]  -541 -1635   -66   545   732  2288  2118  1396  2510  2061
  [1270]  1060   918   205    45  -354 -1343 -1331  -929 -1420 -1272
  [1280] -1083  -871  -322  -466  -679  -193  -196  -450  -556 -1037
  [1290] -1199 -1181 -1103  2238  2637 -1114  4727  3392  2016  3836
  [1300]  1923  1495  2648 -1288 -1986  -108 -3697 -2978 -2816 -3208
  [1310] -2170  -840 -2743   849  1216   503  1623  1873   903  1935
  [1320]   203  -425   638 -1193 -1806 -1059 -1431 -1251  -368  -892
  [1330]   689  1172   668  1459  2415  1822  1650  1283  1421   858
  [1340]  -125  -664  -535 -1251 -1850 -1662 -1396 -1248 -1280 -1094
  [1350]  -648  -464  -610  -197  -159  -554  -941  -843  -403  2411
  [1360]  1999 -1734  5221  3217   544  3929  2769  1726  1602  -603
  [1370]  -412   739 -4153 -2373 -1772 -3043 -3432 -1619 -2202  -123
  [1380]  -997  -548  1912  1236   256  1699  1627   521   688   -74
  [1390]   485  -324 -1322  -706    81 -1123  -637    10   171   302
  [1400]   451  1237  1489  1050  1573  1322  1011   754    39    -5
  [1410]  -494 -1341 -1293 -1244 -1746 -1634 -1519 -1318 -1196 -1175
  [1420]  -868  -740  -780  -443  -236   633  3496   247  1018  6903
  [1430]   245  2524  4373  1649  1163  1201  -627   617 -2058 -3598
  [1440]  -575 -3206 -3857 -2112 -1756 -2704  -864 -1161   641   374
  [1450]   -63  1598  2034   565  1470  1804   604   477   194   340
  [1460]  -224  -909  -669  -227 -1209  -830   -42   105   224   318
  [1470]  1485  1692   514  1643  1804    25   331   380  -715 -1037
  [1480] -1408 -1376 -1575 -2363 -1737 -1302 -1859 -1319  -723  -845
  [1490]  -445  -242   934  3682   261  1449  6834    90  2576  4675
  [1500]  1542  1422  1153   217   368 -2152 -2545  -486 -3906 -3658
  [1510] -1911 -2711 -3207 -1621 -1286  -503  -433   288  1961  1281
  [1520]  1099  2582  2091  1170  1531  1293   431  -217  -482  -355
  [1530] -1176 -1624  -846  -870  -951   -96   174   755  1179  1004
  [1540]  1810  1613   910  1180   553  -323  -472  -896 -1507 -1783
  [1550] -2007 -1881 -2077 -1858 -1279 -1314 -1138  -483  -236   240
  [1560]  2594  1741   268  5825  2085  1306  5286  1976  1828  1790
  [1570]  1026   360 -1054 -2076  -933 -3237 -4069 -2288 -3444 -3562
  [1580] -2431 -1666 -1486  -737   114  1376  1337  1417  2825  2224
  [1590]  1756  2028  1818   851   332    96  -309 -1036 -1290  -867
  [1600] -1319 -1294  -508  -212  -211   817   955   871  1596  1166
  [1610]  1120   907   283    85  -473 -1141 -1177 -1572 -1968 -1729
  [1620] -1849 -1684 -1565 -1322  -800  -626  -435   900  2882   239
  [1630]  2147  5879   393  3375  4653  1486  2247  1607  1128    43
  [1640] -1843 -1431 -1589 -4119 -3461 -2346 -4256 -3292 -2247 -1747
  [1650] -1500  -605   681  1279  1126  2040  3014  1947  2051  2464
  [1660]  1619   823   561   334  -564 -1148  -970 -1068 -1642 -1247
  [1670]  -407  -570  -233   907   866   784  1548  1213   855   831
  [1680]   223   -34  -624 -1066 -1184 -1712 -1852 -1656 -1728 -1671
  [1690] -1359 -1153  -819  -514  -188  1309  2741     5  3289  5150
  [1700]   159  4271  4321  1234  2339  1517   950  -445 -2024 -1151
  [1710] -2506 -4279 -3367 -2622 -4582 -3118 -2032 -1941 -1259  -273
  [1720]   985  1334  1326  2652  2903  1998  2240  2577  1231   758
  [1730]   598    71  -752 -1264  -882 -1308 -1664 -1001  -335  -655
  [1740]  -261  1174   916   572  1608  1598   530   649   680  -315
  [1750]  -868  -879 -1090 -1890 -1960 -1413 -1853 -1913 -1160 -1129
  [1760] -1057  -454  -234   668  2991   964  1472  6368  1152  2366
  [1770]  5736  1740  1376  2435   474    18 -2273 -1625 -2197 -4183
  [1780] -4209 -2577 -3942 -4055 -1769 -1568 -1426   -33   921  1824
  [1790]  1485  2345  3244  2199  1925  2342  1616   342   313   117
  [1800]  -898 -1378 -1148 -1113 -1571 -1322  -418  -297   -32   443
  [1810]  1421  1380   696  1869  1495    39   632   303  -912 -1087
  [1820] -1179 -1579 -2026 -1973 -1568 -1610 -1661  -891  -684  -710
  [1830]   -14   324   610  2983  2280   417  5852  2902   662  5063
  [1840]  2695  -126  2104   169  -679 -1957 -2607 -2132 -3656 -4333
  [1850] -2675 -2663 -3637 -1857  -272  -963   125  1627  1720  1896
  [1860]  1956  2763  2303  1198  1529  1301   -48  -461  -238  -805
  [1870] -1632 -1163  -874 -1139  -958  -274   214   182   812  1255
  [1880]  1388  1652  1120  1154  1172  -136  -249  -226 -1430 -1670
  [1890] -1417 -1844 -1990 -1681 -1391 -1180  -982  -446  -117   -44
  [1900]   190   626   800  1967  3269   202  3396  4612   -75  2649
  [1910]  3795  -408   406  1014 -1433 -1341 -2613 -2353 -2426 -3500
  [1920] -3091 -1441 -2340 -1993  -110   338  -146  1373  1723  1671
  [1930]  1655  1595  1827  1184   189   650   101  -839  -791  -646
  [1940]  -997 -1175  -703  -291  -346   -86   494   799   911  1249
  [1950]  1239  1331  1184   440   504   280  -937  -861  -825 -1748
  [1960] -1539 -1313 -1554 -1220  -989  -772  -285  -126   -29   307
  [1970]   213    93   286   379  2195  2231  -887  3941  3250  -729
  [1980]  2536  3326  -909   604   606 -1084 -1133 -2299 -2221 -1456
  [1990] -3111 -2789  -835 -1641 -1688    40   682   283  1022  1450
  [2000]  1608  1360   845  1043  1032  -312  -426   -26  -788 -1249
  [2010]  -708  -527  -543  -499   152   617   550   649  1139  1419
  [2020]   921   562  1158   936  -122  -476    66  -723 -1607 -1084
  [2030]  -755 -1540 -1254  -627  -541  -586  -380    50   220  -177
  [2040]  -165    32  -357  -645  -465   665  2819  -433   995  5151
  [2050]  1016   754  3829  1880  -254  1063  -816  -605 -1823 -3154
  [2060] -2408 -1506 -3555 -2256  -856  -625  -913   648  1235  1480
  [2070]  1110  1269  1431  1173  -261   -72    17 -1036 -1375  -819
  [2080]  -799  -709  -418   165   772   939  1025  1453  1505  1045
  [2090]   769   895   486  -364  -310  -282  -786 -1145  -684  -717
  [2100]  -776  -537  -328  -213   -30  -236  -120    -8  -336  -590
  [2110]  -420  -716 -1128 -1178 -1218 -1140  -695  1480  3403    -6
  [2120]  3093  5636  1992  1792  3822  1088    56  -306 -2316 -1961
  [2130] -2413 -4233 -3295 -1501 -2272 -1839    77  1005   764  1725
  [2140]  1682  1472  1431   460  -559  -169 -1080 -1738 -1536 -1205
  [2150]  -993  -352   132   856  1545  1846  1788  1877  1550   915
  [2160]   439  -159  -953 -1160 -1094 -1371 -1193  -302   434   480
  [2170]   599  1178  1030   511   308  -207  -663  -826 -1353 -1571
  [2180] -1506 -1513 -1425 -1126  -966  -834  -655  -285   169   626
  [2190]  2995  4718  1310  3206  4901  1998  1267  1751 -1059 -1032
  [2200] -1120 -3297 -3099 -2154 -2600 -2012  -704  -540   373  1342
  [2210]  1177   724  1318   566  -243  -671 -1264 -1586 -1182 -1383
  [2220] -1081  -297   612   924  1455  1890  1922  1709  1430   574
  [2230]    18  -511 -1074 -1398 -1505 -1448  -896  -245   843  1361
  [2240]  1420  1996  2038  1078   542  -101 -1097 -1345 -1523 -2000
  [2250] -1815 -1356 -1313  -901  -513  -323   126   231  -108    40
  [2260]  -154  -450  -263    96  1449  3574  2230  2186  3594  2223
  [2270]  1953  1708  -735 -1582  -979 -2489 -2681 -2728 -2546 -1617
  [2280]  -286  -325   327  1038  1247  1060   999  -247  -635  -712
  [2290] -1275 -1528 -1244 -1213  -264   498   828  1195  1663  1670
  [2300]  1595  1167   339  -264  -490  -917 -1173 -1185  -974  -264
  [2310]   751  1195  1351  1600  1683  1208   457  -317 -1136 -1476
  [2320] -1410 -1450 -1563 -1278  -859  -464  -265  -139  -149   -59
  [2330]   -72  -176  -507  -818  -984  -802   213  3086  4406  2435
  [2340]  3263  2864  1827  2138   922 -2226 -2444 -2418 -2545 -2253
  [2350] -1928 -2457  -824   997  1284  1329  1288   223   395   542
  [2360]  -848 -1895 -1959 -2164 -1387  -291  -113   139  1077  1861
  [2370]  2218  2091  1353   548    82  -194  -686 -1218 -1465 -1266
  [2380]  -663    90   568   747  1149  1780  1785  1389   758  -108
  [2390]  -631  -848 -1057 -1303 -1354 -1068  -714  -281   -86  -366
  [2400]  -460  -359  -391  -419  -547  -912 -1068 -1004  -880  -713
  [2410]  -289   908  3823  5036  3784  3878  2352   870  1358   564
  [2420] -1861 -2845 -3314 -3682 -2231  -770 -1139  -666   511  1029
  [2430]  1528  2256   864  -635  -939 -1448 -1857 -1485 -1688 -2056
  [2440] -1047   287  1020  2087  2514  1845  1557  1718   966   298
  [2450]  -279 -1433 -2030 -1550 -1184  -812   -61   283   793  1728
  [2460]  2064  2001  1723   993    66  -434  -930 -1398 -1431 -1468
  [2470] -1570 -1245  -761  -701  -533  -320  -367  -382  -197  -296
  [2480]  -407  -400  -619  -747  -186   900  2885  4027  3630  3546
  [2490]  2231  1192  1258   912  -378 -1464 -2498 -3548 -3090 -2019
aArDvArK

## RUN PROGRAM (22) --- FEA_SD input

echo 'Running command (22):'
echo '    featohtk -k WAVEFORM -f samples tstE.sd tst22.htk'
featohtk -k WAVEFORM -f samples tstE.sd tst22.htk

## CHECK OUTPUT (22)

echo '... checking output.'

## --- HEADER (22)

if od -b tst22.htk \
	| head -1 \
	| grep ' 000 000 011 304 000 000 004 342 000 002 000 000 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst22.htk'
fi

## --- DATA (22)

## --- Convert to ESPS

cat >fmt22 <<aArDvArK
samples SHORT 1
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(22)" \
	-t FEA_SD -E -S12 fmt22 tst22.htk htk22.sd 2>/dev/null

## --- Compare

if pspsdiff -H htk22.sd tstE.sd | grep '.*'
then echo '* * * ERROR: bad data in tst22.htk'
fi

## MAKE TEST FEA_SD FILE --- multichannel, FLOAT samples

echo "Making test FEA_SD file tstF.sd (multichannel, FLOAT samples)."

esig2fea - tstF.sd <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    323
     -1
samples: FLOAT 10 <r>
record_freq: DOUBLE 1 <g> 8000
start_time: DOUBLE 1 <g> 0
FeaSubtype: SHORT <g> 8
FeaSubtype.enumStrings: CHAR 10 10 <g> "NONE\0\0\0\0\0\0FEA_VQ\0\0\0\0FEA_ANA\0\0\0FEA_STAT\0\0FEA_QHIST\0FEA_DST\0\0\0FEA_2KB\0\0\0FEA_SPEC\0\0FEA_SD\0\0\0\0FEA_FILT\0\0"
  [  0][0]   418   832  1562  2000  2078  2062  1856  1711  1397  1234
  [  1][0]  1014   591    49  -687 -1344 -2050 -2500 -2546 -2434 -2088
  [  2][0] -1658 -1224  -879  -572  -123   269   767  1253  1614  1799
  [  3][0]  1729  1508  1109   759   441   208   127   -63  -262  -556
  [  4][0]  -905 -1133 -1216 -1070  -812  -459  -159   -57  -153  -405
  [  5][0]  -598  -721  -757  -738  -476   314   920  1521  2283  2355
  [  6][0]  2310  2055  1761  1520  1146  1021   576   -68  -832 -1628
  [  7][0] -2325 -2923 -2914 -2691 -2317 -1589 -1019  -534  -178   206
  [  8][0]   591   922  1446  1748  1905  1797  1426   946   326   -75
  [  9][0]  -375  -450  -312  -296  -238  -371  -560  -705  -826  -641
  [ 10][0]  -420  -176    20    12  -178  -558  -853 -1155 -1330 -1269
  [ 11][0]  -773   448  1270  2205  3166  2974  2833  2286  1796  1467
  [ 12][0]   908   796   163  -699 -1586 -2541 -3275 -3699 -3284 -2699
  [ 13][0] -1933  -773  -150   397   737   947  1227  1383  1808  1909
  [ 14][0]  1722  1378   637  -154  -848 -1148 -1112  -855  -282   102
  [ 15][0]   233   226    52  -137  -203   -89    51   105    13  -261
  [ 16][0]  -659 -1103 -1429 -1586 -1652 -1550 -1133   151  1543  2446
  [ 17][0]  3966  4221  3561  3295  2197  1350   727   172  -526 -1547
  [ 18][0] -2291 -3480 -4167 -4075 -3884 -2671 -1373  -174   969  1447
  [ 19][0]  1800  1775  1742  1850  1665  1632  1138   353  -425 -1384
  [ 20][0] -1882 -1989 -1515  -780   153  1103  1330  1463  1243   649
  [ 21][0]   352    64  -190  -381  -578  -944 -1390 -1601 -1891 -1990
  [ 22][0] -1759 -1601 -1304  -681   959  2799  3609  5429  5346  3799
  [ 23][0]  3370  1549   259  -351  -859 -1886 -2742 -3281 -4535 -4754
  [ 24][0] -3737 -3023 -1293   801  1636  2377  2605  2247  1588  1552
  [ 25][0]  1343   598   493  -355 -1654 -2042 -2490 -2492 -1527  -206
  [ 26][0]   872  1919  2847  2348  1931  1531   348  -123  -368  -999
  [ 27][0] -1305 -1360 -1772 -1896 -1552 -1601 -1406  -865  -846  -776
  [ 28][0]  -428   -20  1866  3507  4078  5717  4298  2797  1858   -84
  [ 29][0]  -633 -1014 -1164 -2288 -2689 -3584 -4484 -3612 -2505 -1184
  [ 30][0]   946  2035  2292  2183  1808   994   703   979   153    46
  [ 31][0]  -748 -1887 -2379 -2371 -1896  -867   622  1436  2099  2507
  [ 32][0]  2242  1928  1608   944   139  -485 -1272 -1902 -1832 -1755
  [ 33][0] -1423  -851  -771  -778  -780  -832  -838  -620  -603  -597
  [ 34][0]   883  2700  2782  4813  4508  2357  2638   501  -157  -338
  [ 35][0]  -631 -1869 -2278 -2990 -4301 -3250 -2361 -1439   703  1585
  [ 36][0]  1655  1977  1572  1023   857  1098   -57   -81 -1091 -2136
  [ 37][0] -2260 -2141 -1592  -538   642   884  1603  1908  1855  2222
  [ 38][0]  2183  1406   830  -288 -1374 -1722 -1731 -1520  -834  -541
  [ 39][0]  -650  -623  -976 -1075  -849  -597  -712  -782 -1206  -490
  [ 40][0]  1856  2124  4546  5183  3053  3285   988    93  -160  -207
  [ 41][0] -1424 -1501 -2843 -4155 -3649 -3219 -1935   176  1397  1708
  [ 42][0]  2313  1411  1092   927   873   243   416  -978 -1643 -2283
  [ 43][0] -2569 -2035  -977   -23   734  1579  1334  1874  1828  2041
  [ 44][0]  2182  1892  1041   149  -959 -1706 -1700 -1563 -1141  -774
  [ 45][0]  -831 -1066 -1155 -1291 -1041  -792  -638  -718  -860  -807
  [ 46][0]   963  1728  2975  4922  3259  3706  2079  1085   346   446
  [ 47][0]  -728  -679 -1488 -2993 -2895 -3522 -2934 -2054  -417  -299
  [ 48][0]  1462   764  1202   756   989   449  1145   504   234  -138
  [ 49][0] -1088 -1318 -1445 -1072  -800   291   151   847   791  1059
  [ 50][0]  1048  1739  1441  1670  1232   457  -161  -735 -1249 -1283
  [ 51][0] -1279 -1453 -1201 -1451 -1408 -1295 -1069  -986  -570  -716
  [ 52][0]  -579    85  1351  1630  3438  3418  2924  2940  1637  1180
  [ 53][0]   992   831    39   488 -1321 -1627 -2650 -3029 -3272 -2236
  [ 54][0] -2176 -1057  -499  -403    37   241   362   896  1485  1346
  [ 55][0]  1859  1288   818   296   -74  -677  -351  -622  -549  -430
  [ 56][0]  -601  -546  -246   -89   567  1008  1305  1414  1139   606
  [ 57][0]   177  -245  -603  -578  -809  -931 -1145 -1388 -1581 -1515
  [ 58][0] -1521 -1331 -1202 -1038  -159  1011  1535  3145  3200  3223
  [ 59][0]  3128  2447  1866  1725  1141   421   214 -1298 -1938 -2888
  [ 60][0] -3438 -3652 -2875 -2725 -1402  -848  -197   243   755   753
  [ 61][0]  1432  1599  1791  1948  1635  1041   588  -222  -751  -914
  [ 62][0] -1193 -1086  -906  -859  -684  -226   -90   618  1109  1397
  [ 63][0]  1715  1640  1131   854   145  -425  -684 -1106 -1294 -1291
  [ 64][0] -1548 -1602 -1497 -1621 -1416 -1226 -1121  -659   573  1161
  [ 65][0]  2386  3437  3142  3434  2651  2046  1444  1276   278   356
  [ 66][0]  -629 -1414 -2090 -2736 -3361 -2893 -2696 -1930  -912  -342
  [ 67][0]   227   687   763   812  1205  1018  1342  1264  1078   617
  [ 68][0]   340  -564  -709 -1131 -1099  -894  -445  -313   197   352
  [ 69][0]   442   865  1000  1224  1409  1259   802   501  -319  -705
  [ 70][0] -1047 -1240 -1238 -1126 -1252 -1171 -1211 -1350 -1174 -1189
  [ 71][0] -1093  -773   303  1018  2121  3228  3065  3280  2586  2068
  [ 72][0]  1313  1248   326   355  -444 -1042 -1786 -2253 -2971 -2691
  [ 73][0] -2586 -2047 -1121  -611    82   364   665   443   897   601
  [ 74][0]  1047   988  1152   776   719   -40  -296  -702  -879  -795
  [ 75][0]  -564  -332   -68   284   257   634   787   945  1176  1200
  [ 76][0]   968   703   195  -392  -676 -1093 -1168 -1238 -1266 -1364
  [ 77][0] -1288 -1421 -1324 -1247 -1200  -927  -219   901  1573  3074
  [ 78][0]  3159  3376  2938  2339  1522  1192   611   128    -2  -935
  [ 79][0] -1315 -2170 -2508 -3113 -2465 -2587 -1342  -912    58   307
  [ 80][0]   874   648   862   813   805   977   961   799   570   124
  [ 81][0]  -443  -777 -1070 -1067  -732  -368    41   501   700   875
  [ 82][0]  1135  1148  1238  1266   964   614   136  -534  -941 -1247
  [ 83][0] -1420 -1386 -1282 -1298 -1179 -1161 -1158 -1049 -1022  -900
  [ 84][0]  -542   573  1219  2474  3283  3154  3202  2410  1865   963
  [ 85][0]   976  -123   221  -772 -1044 -1877 -2188 -2923 -2526 -2371
  [ 86][0] -1697  -745  -187   335   640   574   481   595   495   667
  [ 87][0]   822   680   511   272  -329  -528  -788  -806  -540  -168
  [ 88][0]   110   526   780   792  1107  1099  1136  1211   959   601
  [ 89][0]   178  -412  -928 -1143 -1416 -1397 -1314 -1351 -1252 -1210
  [ 90][0] -1215 -1071  -996  -932  -712   181  1217  1990  3429  3179
  [ 91][0]  3298  2478  1924  1018   930   342   -13  -176 -1238 -1592
  [ 92][0] -2337 -2716 -2724 -2129 -1768  -644   -62   283   399   684
  [ 93][0]   228   498   491   587   861   684   266   -64  -714 -1037
  [ 94][0]  -808  -669   -81   478   643   768   982   821  1117  1562
  [ 95][0]  1465  1491   954   -94  -793 -1410 -1792 -1454 -1316 -1249
  [ 96][0] -1027 -1299 -1375 -1079  -972  -575  -305  -596  -813  -280
  [ 97][0]  1583  2308  4320  4732  3214  2663  1009    69   270   242
  [ 98][0]  -880  -860 -2945 -3638 -3429 -2670 -1376   765   778  1169
  [ 99][0]   954   272   587  1155   809   741  -203 -1821 -2131 -2254
  [100][0] -1746  -388   281   299   710   376   748  1714  2105  2224
  [101][0]  2062   802   -92  -197  -510  -150   122  -599  -911 -1189
  [102][0] -1430  -675  -257  -295  -213  -783 -1166  -856  -845  -703
  [103][0]  -798 -1653 -2196 -1430  1816  3315  4754  5574  2447  2217
  [104][0]  1687  1347  1693  1115 -2393 -2972 -4665 -4252 -2178  -884
  [105][0]  -948    76  -606    89  2117  2448  2492  1801  -662 -1677
  [106][0] -1489 -1887 -1016 -1120 -2079 -1742  -862    79  2065  2464
  [107][0]  1915  1764  1185  1074  1760  1165   107  -445 -1295 -1169
  [108][0]  -203  -224  -264  -477 -1032  -597   -79   -95   -79  -673
  [109][0] -1232 -1080 -1020  -996 -1237 -2071 -2350 -1373  2504  4442
  [110][0]  3763  5056  1955  2479  3636  2751   931  -555 -4230 -3378
  [111][0] -2749 -2665 -2179 -2662 -2875  -316  1531  2301  2975  1144
  [112][0]   584  1183   684    -8  -920 -3218 -2847 -1924 -1410  -465
  [113][0]  -402  -686  1068  2168  2569  2868  1569   740  1166   739
  [114][0]   281  -410 -1402  -970  -267  -155   -85  -503  -710    60
  [115][0]   190  -118  -466 -1041  -925  -503  -911 -1252 -1639 -2037
  [116][0] -1698 -1320  -336  3496  3506  2350  4829  2274  3810  4002
  [117][0]   984  -463  -487 -3334 -1409 -2775 -4548 -3059 -2004 -1579
  [118][0]  1003   377   257  2612  1996  1900  1790  -590  -891  -155
  [119][0] -2146 -1893 -2131 -2667  -975  -106  -648  1026  1420  1818
  [120][0]  3047  2075  1234  1790   943   402    90 -1079  -744  -466
  [121][0] -1088  -834  -673  -723    28  -305  -693  -164  -288  -357
  [122][0]  -343 -1139 -1081 -1002 -1792 -1823 -1594  -612  3692  2502
  [123][0]   558  5055  2884  4319  3947   454   356  1863 -2520 -1777
  [124][0] -3109 -4368 -2248 -2266 -3783  -714   124    84  2221   998
  [125][0]  1246  2974  1080   -64   750 -1451 -1020 -1325 -2723 -1771
  [126][0]  -541 -1635   -66   545   732  2288  2118  1396  2510  2061
  [127][0]  1060   918   205    45  -354 -1343 -1331  -929 -1420 -1272
  [128][0] -1083  -871  -322  -466  -679  -193  -196  -450  -556 -1037
  [129][0] -1199 -1181 -1103  2238  2637 -1114  4727  3392  2016  3836
  [130][0]  1923  1495  2648 -1288 -1986  -108 -3697 -2978 -2816 -3208
  [131][0] -2170  -840 -2743   849  1216   503  1623  1873   903  1935
  [132][0]   203  -425   638 -1193 -1806 -1059 -1431 -1251  -368  -892
  [133][0]   689  1172   668  1459  2415  1822  1650  1283  1421   858
  [134][0]  -125  -664  -535 -1251 -1850 -1662 -1396 -1248 -1280 -1094
  [135][0]  -648  -464  -610  -197  -159  -554  -941  -843  -403  2411
  [136][0]  1999 -1734  5221  3217   544  3929  2769  1726  1602  -603
  [137][0]  -412   739 -4153 -2373 -1772 -3043 -3432 -1619 -2202  -123
  [138][0]  -997  -548  1912  1236   256  1699  1627   521   688   -74
  [139][0]   485  -324 -1322  -706    81 -1123  -637    10   171   302
  [140][0]   451  1237  1489  1050  1573  1322  1011   754    39    -5
  [141][0]  -494 -1341 -1293 -1244 -1746 -1634 -1519 -1318 -1196 -1175
  [142][0]  -868  -740  -780  -443  -236   633  3496   247  1018  6903
  [143][0]   245  2524  4373  1649  1163  1201  -627   617 -2058 -3598
  [144][0]  -575 -3206 -3857 -2112 -1756 -2704  -864 -1161   641   374
  [145][0]   -63  1598  2034   565  1470  1804   604   477   194   340
  [146][0]  -224  -909  -669  -227 -1209  -830   -42   105   224   318
  [147][0]  1485  1692   514  1643  1804    25   331   380  -715 -1037
  [148][0] -1408 -1376 -1575 -2363 -1737 -1302 -1859 -1319  -723  -845
  [149][0]  -445  -242   934  3682   261  1449  6834    90  2576  4675
  [150][0]  1542  1422  1153   217   368 -2152 -2545  -486 -3906 -3658
  [151][0] -1911 -2711 -3207 -1621 -1286  -503  -433   288  1961  1281
  [152][0]  1099  2582  2091  1170  1531  1293   431  -217  -482  -355
  [153][0] -1176 -1624  -846  -870  -951   -96   174   755  1179  1004
  [154][0]  1810  1613   910  1180   553  -323  -472  -896 -1507 -1783
  [155][0] -2007 -1881 -2077 -1858 -1279 -1314 -1138  -483  -236   240
  [156][0]  2594  1741   268  5825  2085  1306  5286  1976  1828  1790
  [157][0]  1026   360 -1054 -2076  -933 -3237 -4069 -2288 -3444 -3562
  [158][0] -2431 -1666 -1486  -737   114  1376  1337  1417  2825  2224
  [159][0]  1756  2028  1818   851   332    96  -309 -1036 -1290  -867
  [160][0] -1319 -1294  -508  -212  -211   817   955   871  1596  1166
  [161][0]  1120   907   283    85  -473 -1141 -1177 -1572 -1968 -1729
  [162][0] -1849 -1684 -1565 -1322  -800  -626  -435   900  2882   239
  [163][0]  2147  5879   393  3375  4653  1486  2247  1607  1128    43
  [164][0] -1843 -1431 -1589 -4119 -3461 -2346 -4256 -3292 -2247 -1747
  [165][0] -1500  -605   681  1279  1126  2040  3014  1947  2051  2464
  [166][0]  1619   823   561   334  -564 -1148  -970 -1068 -1642 -1247
  [167][0]  -407  -570  -233   907   866   784  1548  1213   855   831
  [168][0]   223   -34  -624 -1066 -1184 -1712 -1852 -1656 -1728 -1671
  [169][0] -1359 -1153  -819  -514  -188  1309  2741     5  3289  5150
  [170][0]   159  4271  4321  1234  2339  1517   950  -445 -2024 -1151
  [171][0] -2506 -4279 -3367 -2622 -4582 -3118 -2032 -1941 -1259  -273
  [172][0]   985  1334  1326  2652  2903  1998  2240  2577  1231   758
  [173][0]   598    71  -752 -1264  -882 -1308 -1664 -1001  -335  -655
  [174][0]  -261  1174   916   572  1608  1598   530   649   680  -315
  [175][0]  -868  -879 -1090 -1890 -1960 -1413 -1853 -1913 -1160 -1129
  [176][0] -1057  -454  -234   668  2991   964  1472  6368  1152  2366
  [177][0]  5736  1740  1376  2435   474    18 -2273 -1625 -2197 -4183
  [178][0] -4209 -2577 -3942 -4055 -1769 -1568 -1426   -33   921  1824
  [179][0]  1485  2345  3244  2199  1925  2342  1616   342   313   117
  [180][0]  -898 -1378 -1148 -1113 -1571 -1322  -418  -297   -32   443
  [181][0]  1421  1380   696  1869  1495    39   632   303  -912 -1087
  [182][0] -1179 -1579 -2026 -1973 -1568 -1610 -1661  -891  -684  -710
  [183][0]   -14   324   610  2983  2280   417  5852  2902   662  5063
  [184][0]  2695  -126  2104   169  -679 -1957 -2607 -2132 -3656 -4333
  [185][0] -2675 -2663 -3637 -1857  -272  -963   125  1627  1720  1896
  [186][0]  1956  2763  2303  1198  1529  1301   -48  -461  -238  -805
  [187][0] -1632 -1163  -874 -1139  -958  -274   214   182   812  1255
  [188][0]  1388  1652  1120  1154  1172  -136  -249  -226 -1430 -1670
  [189][0] -1417 -1844 -1990 -1681 -1391 -1180  -982  -446  -117   -44
  [190][0]   190   626   800  1967  3269   202  3396  4612   -75  2649
  [191][0]  3795  -408   406  1014 -1433 -1341 -2613 -2353 -2426 -3500
  [192][0] -3091 -1441 -2340 -1993  -110   338  -146  1373  1723  1671
  [193][0]  1655  1595  1827  1184   189   650   101  -839  -791  -646
  [194][0]  -997 -1175  -703  -291  -346   -86   494   799   911  1249
  [195][0]  1239  1331  1184   440   504   280  -937  -861  -825 -1748
  [196][0] -1539 -1313 -1554 -1220  -989  -772  -285  -126   -29   307
  [197][0]   213    93   286   379  2195  2231  -887  3941  3250  -729
  [198][0]  2536  3326  -909   604   606 -1084 -1133 -2299 -2221 -1456
  [199][0] -3111 -2789  -835 -1641 -1688    40   682   283  1022  1450
  [200][0]  1608  1360   845  1043  1032  -312  -426   -26  -788 -1249
  [201][0]  -708  -527  -543  -499   152   617   550   649  1139  1419
  [202][0]   921   562  1158   936  -122  -476    66  -723 -1607 -1084
  [203][0]  -755 -1540 -1254  -627  -541  -586  -380    50   220  -177
  [204][0]  -165    32  -357  -645  -465   665  2819  -433   995  5151
  [205][0]  1016   754  3829  1880  -254  1063  -816  -605 -1823 -3154
  [206][0] -2408 -1506 -3555 -2256  -856  -625  -913   648  1235  1480
  [207][0]  1110  1269  1431  1173  -261   -72    17 -1036 -1375  -819
  [208][0]  -799  -709  -418   165   772   939  1025  1453  1505  1045
  [209][0]   769   895   486  -364  -310  -282  -786 -1145  -684  -717
  [210][0]  -776  -537  -328  -213   -30  -236  -120    -8  -336  -590
  [211][0]  -420  -716 -1128 -1178 -1218 -1140  -695  1480  3403    -6
  [212][0]  3093  5636  1992  1792  3822  1088    56  -306 -2316 -1961
  [213][0] -2413 -4233 -3295 -1501 -2272 -1839    77  1005   764  1725
  [214][0]  1682  1472  1431   460  -559  -169 -1080 -1738 -1536 -1205
  [215][0]  -993  -352   132   856  1545  1846  1788  1877  1550   915
  [216][0]   439  -159  -953 -1160 -1094 -1371 -1193  -302   434   480
  [217][0]   599  1178  1030   511   308  -207  -663  -826 -1353 -1571
  [218][0] -1506 -1513 -1425 -1126  -966  -834  -655  -285   169   626
  [219][0]  2995  4718  1310  3206  4901  1998  1267  1751 -1059 -1032
  [220][0] -1120 -3297 -3099 -2154 -2600 -2012  -704  -540   373  1342
  [221][0]  1177   724  1318   566  -243  -671 -1264 -1586 -1182 -1383
  [222][0] -1081  -297   612   924  1455  1890  1922  1709  1430   574
  [223][0]    18  -511 -1074 -1398 -1505 -1448  -896  -245   843  1361
  [224][0]  1420  1996  2038  1078   542  -101 -1097 -1345 -1523 -2000
  [225][0] -1815 -1356 -1313  -901  -513  -323   126   231  -108    40
  [226][0]  -154  -450  -263    96  1449  3574  2230  2186  3594  2223
  [227][0]  1953  1708  -735 -1582  -979 -2489 -2681 -2728 -2546 -1617
  [228][0]  -286  -325   327  1038  1247  1060   999  -247  -635  -712
  [229][0] -1275 -1528 -1244 -1213  -264   498   828  1195  1663  1670
  [230][0]  1595  1167   339  -264  -490  -917 -1173 -1185  -974  -264
  [231][0]   751  1195  1351  1600  1683  1208   457  -317 -1136 -1476
  [232][0] -1410 -1450 -1563 -1278  -859  -464  -265  -139  -149   -59
  [233][0]   -72  -176  -507  -818  -984  -802   213  3086  4406  2435
  [234][0]  3263  2864  1827  2138   922 -2226 -2444 -2418 -2545 -2253
  [235][0] -1928 -2457  -824   997  1284  1329  1288   223   395   542
  [236][0]  -848 -1895 -1959 -2164 -1387  -291  -113   139  1077  1861
  [237][0]  2218  2091  1353   548    82  -194  -686 -1218 -1465 -1266
  [238][0]  -663    90   568   747  1149  1780  1785  1389   758  -108
  [239][0]  -631  -848 -1057 -1303 -1354 -1068  -714  -281   -86  -366
  [240][0]  -460  -359  -391  -419  -547  -912 -1068 -1004  -880  -713
  [241][0]  -289   908  3823  5036  3784  3878  2352   870  1358   564
  [242][0] -1861 -2845 -3314 -3682 -2231  -770 -1139  -666   511  1029
  [243][0]  1528  2256   864  -635  -939 -1448 -1857 -1485 -1688 -2056
  [244][0] -1047   287  1020  2087  2514  1845  1557  1718   966   298
  [245][0]  -279 -1433 -2030 -1550 -1184  -812   -61   283   793  1728
  [246][0]  2064  2001  1723   993    66  -434  -930 -1398 -1431 -1468
  [247][0] -1570 -1245  -761  -701  -533  -320  -367  -382  -197  -296
  [248][0]  -407  -400  -619  -747  -186   900  2885  4027  3630  3546
  [249][0]  2231  1192  1258   912  -378 -1464 -2498 -3548 -3090 -2019
aArDvArK

## RUN PROGRAM (23) --- FEA_SD input

echo 'Running command (23):'
echo '    featohtk -k WAVEFORM -f "samples[2]" tstF.sd tst23.htk'
featohtk -k WAVEFORM -f "samples[2]" tstF.sd tst23.htk

## CHECK OUTPUT (23)

echo '... checking output.'

## --- HEADER (23)

if od -b tst23.htk \
	| head -1 \
	| grep ' 000 000 000 372 000 000 004 342 000 002 000 000 ' >/dev/null
then :
else echo '* * * ERROR: bad header on tst23.htk'
fi

## --- DATA (23)

## --- Convert to ESPS

cat >fmt23 <<aArDvArK
samples SHORT 1
aArDvArK

ESPS_EDR="on"; export ESPS_EDR
addfeahd -c "featohtk test(23)" \
	-t FEA_SD -E -S12 fmt23 tst23.htk htk23.sd 2>/dev/null

## --- Make comparison file

esig2fea - cmp23.sd <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    323
     -1
samples: SHORT 1 <r>
record_freq: DOUBLE 1 <g> 8000
start_time: DOUBLE 1 <g> 0
FeaSubtype: SHORT <g> 8
FeaSubtype.enumStrings: CHAR 10 10 <g> "NONE\0\0\0\0\0\0FEA_VQ\0\0\0\0FEA_ANA\0\0\0FEA_STAT\0\0FEA_QHIST\0FEA_DST\0\0\0FEA_2KB\0\0\0FEA_SPEC\0\0FEA_SD\0\0\0\0FEA_FILT\0\0"
  [  0]  1562    49  -879  1109 -1216  -757  1761 -2914  1446  -312
  [ 10]    20  1270   163  -150   637    52 -1429  3561 -4167  1742
  [ 20] -1515  -190 -1304   259 -1293   493  2847 -1772  1866 -1164
  [ 30]  2292 -2379  1608  -771  2782 -2278  1572 -1592   830  -976
  [ 40]  4546 -2843  1092  -977  1041 -1155  2975 -1488  1202 -1445
  [ 50]  1441 -1201  1351    39  -499   818  -246  -603 -1202  1866
  [ 60] -2875  1791  -906  1131 -1497  3142 -2090   763  -709  1000
  [ 70] -1238   303   326 -1121  1152   -68   195 -1324  2938 -2508
  [ 80]   862 -1070  1238 -1282  1219   221  -187   511   780  -928
  [ 90]  -996  1924 -2724   491   -81   954 -1375  4320 -2945   587
  [100]   281   -92  -257 -2196  1693  -606 -1016  1185  -264 -1020
  [110]  1955 -2179   684  1068 -1402  -466  -336  -487   257 -2131
  [120]  1234  -673 -1081  2884 -2248  1080   -66   205  -322 -1103
  [130]  2648 -2743   638   668  -535  -610  5221 -4153  1912 -1322
  [140]  1489 -1293  -780  4373 -3857  2034  -669   514 -1575   934
  [150]  1153 -3207  2091  -846   910 -2077   268 -1054 -1486  1818
  [160]  -508   283 -1565   393 -1589   681   561  -233  -624  -819
  [170]  4321 -3367  1326  -752   916 -1090  -234  1376 -3942  3244
  [180] -1148   696 -2026   610  2104 -3637  2303  -874  1120 -1990
  [190]   800   406 -2340  1827  -703  1184 -1554   286  -909  -835
  [200]   845  -543  1158 -1254  -357  3829 -3555  1431  -418   486
  [210]  -328 -1128  1992 -3295  1431   132  -953  1030 -1425  1310
  [220] -3099  1318   612 -1074  2038 -1313  -263  -735   327 -1244
  [230]   339  1351 -1563  -507  1827  -824 -1959  1353   568 -1057
  [240]  -391  3823 -3314   864  1020 -2030  1723  -761  -619  1258
aArDvArK

## --- Compare

if pspsdiff -H htk23.sd cmp23.sd | grep '.*'
then echo '* * * ERROR: bad data in tst23.htk'
fi

