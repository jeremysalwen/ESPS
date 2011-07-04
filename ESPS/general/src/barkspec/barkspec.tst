#! /bin/sh
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1998  Entropic Research Laboratory, Inc. 
#                   All rights reserved."
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)barkspec.tst	1.3	7/12/98	ERL
# 
# Written by:  Rod Johnson
# Checked by:
# Revised by:
# 
# Brief description:  Test script for barkspec program.
#   "barkspec.tst clean" just removes the test files.
#   "barkspec.tst" runs the tests and leaves the test files in place.
#

## CLEAN UP TEST FILES

rm -f tstA.spec out1.spec log1.txt log1.ref ref1.spec dif1.spec \
    tmp.spec tmp.sd dif.sd \
    out2.spec log2.txt log2.ref \
    out3.spec testparam

case "$1" in
clean)
    exit 0;;
esac

## TURN OFF COMMON PROCESSING

USE_ESPS_COMMON=off; export USE_ESPS_COMMON

## MAKE TEST FEA_SPEC FILE

echo "Making test FEA_SPEC file tstA.spec."

esig2fea - tstA.spec <<aArDvArK
Esignal
   0.0B
  ASCII
     48
   1471
     -1
Tag: LONG <r>
tot_power: FLOAT <r>
re_spec_val: SCHAR 129 <r>
sf: FLOAT 1 <g> [0]       8000.0000
src_sf: FLOAT 1 <g> [0]       8000.0000
nan: LONG 1 <g> [0]            0
frmlen: LONG 1 <g> [0]           64
step: LONG 1 <g> [0]           16
frame_meth: SHORT 1 <g> [0]       1
frame_meth.enumStrings: CHAR 3 9 <g> 
  [0][0]  "NONE\0\0\0\0\0"
  [1][0]  "FIXED\0\0\0\0"
  [2][0]  "VARIABLE\0"
contin: SHORT 1 <g> [0]       1
contin.enumStrings: CHAR 2 4 <g> 
  [0][0]  "NO\0\0"
  [1][0]  "YES\0"
start: LONG 1 <g> [0]            1
spec_type: SHORT 1 <g> [0]       2
spec_type.enumStrings: CHAR 5 5 <g> 
  [0][0]  "NONE\0"
  [1][0]  "PWR\0\0"
  [2][0]  "DB\0\0\0"
  [3][0]  "REAL\0"
  [4][0]  "CPLX\0"
freq_format: SHORT 1 <g> [0]       2
freq_format.enumStrings: CHAR 7 10 <g> 
  [0][0]  "NONE\0\0\0\0\0\0"
  [1][0]  "SYM_CTR\0\0\0"
  [2][0]  "SYM_EDGE\0\0"
  [3][0]  "ASYM_CTR\0\0"
  [4][0]  "ASYM_EDGE\0"
  [5][0]  "ARB_VAR\0\0\0"
  [6][0]  "ARB_FIXED\0"
num_freqs: LONG 1 <g> [0]          129
FeaSubtype: SHORT <g>      7
FeaSubtype.enumStrings: CHAR 10 10 <g> 
  [0][0]  "NONE\0\0\0\0\0\0"
  [1][0]  "FEA_VQ\0\0\0\0"
  [2][0]  "FEA_ANA\0\0\0"
  [3][0]  "FEA_STAT\0\0"
  [4][0]  "FEA_QHIST\0"
  [5][0]  "FEA_DST\0\0\0"
  [6][0]  "FEA_2KB\0\0\0"
  [7][0]  "FEA_SPEC\0\0"
  [8][0]  "FEA_SD\0\0\0\0"
  [9][0]  "FEA_FILT\0\0"
recordFreq: DOUBLE <g>       500.0000000000000
startTime: DOUBLE 1 <g> [0]        1.022000000000000
[Record 0]
  [tot_power]       42.399658
  [Tag]         8145
  [re_spec_val]
    [0]    23   40   46   50   52   53   54   53   53   51   49   46
    [12]    46   48   50   51   51   51   51   50   48   46   43   39
    [24]    35   32   31   32   33   34   35   36   36   37   38   40
    [36]    41   42   42   41   38   33   18   29   35   37   37   36
    [48]    35   33   31   30   31   32   33   34   35   36   37   39
    [60]    41   42   43   43   43   41   40   40   41   44   46   47
    [72]    47   46   44   41   35   24   31   34   32   25   32   38
    [84]    41   42   42   42   41   42   43   44   45   46   46   45
    [96]    44   42   39   37   35   34   34   34   37   40   42   43
    [108]    43   43   42   41   40   40   39   39   38   36   34   31
    [120]    32   35   37   38   38   37   35   29    0
[Record 1]
  [tot_power]       215.90933
  [Tag]         8161
  [re_spec_val]
    [0]    37   39   42   45   48   50   52   53   53   53   52   51
    [12]    50   49   47   45   44   45   47   49   51   51   51   50
    [24]    49   49   50   50   50   50   50   51   52   52   53   52
    [36]    51   49   47   45   46   48   50   51   52   52   52   51
    [48]    51   50   50   51   51   52   52   52   52   51   49   46
    [60]    44   45   48   50   51   52   52   52   53   53   54   53
    [72]    52   50   47   45   46   48   50   51   52   53   54   55
    [84]    56   57   58   58   59   59   58   58   56   55   52   48
    [96]    43   38   37   39   42   45   48   50   52   52   53   52
    [108]    51   49   48   48   48   49   49   47   45   42   37   36
    [120]    40   43   45   47   48   48   48   48   48
[Record 2]
  [tot_power]       1402.1790
  [Tag]         8177
  [re_spec_val]
    [0]    40   41   45   48   52   54   56   57   56   56   54   52
    [12]    52   53   54   54   54   54   54   54   54   54   54   54
    [24]    56   57   58   59   59   59   59   59   59   59   59   59
    [36]    60   61   62   62   63   64   64   64   65   64   64   63
    [48]    63   61   60   59   58   57   57   57   57   57   57   57
    [60]    57   57   57   57   57   58   59   60   60   61   61   61
    [72]    61   60   60   59   58   57   57   57   58   59   61   63
    [84]    64   65   65   66   66   65   64   63   62   61   59   58
    [96]    57   56   56   57   58   59   59   59   60   59   59   59
    [108]    58   57   57   56   55   55   55   54   54   54   54   54
    [120]    55   55   55   54   53   52   49   44   34
[Record 3]
  [tot_power]       1747.7202
  [Tag]         8193
  [re_spec_val]
    [0]    42   44   49   53   56   58   61   62   63   64   64   64
    [12]    64   64   63   63   62   61   59   57   54   51   50   52
    [24]    55   57   59   60   60   60   60   59   57   53   43   46
    [36]    55   59   62   64   66   67   67   68   68   67   66   65
    [48]    63   61   58   53   43   42   50   53   55   57   57   57
    [60]    56   55   52   47   43   49   53   56   58   59   60   60
    [72]    61   61   61   61   60   59   57   54   56   60   63   65
    [84]    66   66   66   65   64   61   59   57   56   57   58   59
    [96]    59   60   60   60   60   60   59   58   57   56   54   53
    [108]    52   52   52   52   51   50   47   45   47   51   53   55
    [120]    56   57   57   56   55   54   53   52   51
[Record 4]
  [tot_power]       942.25507
  [Tag]         8209
  [re_spec_val]
    [0]    30   40   48   53   56   59   62   63   64   65   64   64
    [12]    63   62   61   61   60   59   58   56   55   53   53   53
    [24]    54   54   54   54   53   52   52   52   53   55   56   57
    [36]    58   58   58   59   60   61   62   62   63   63   62   61
    [48]    60   58   56   53   52   50   49   49   48   48   49   49
    [60]    50   50   50   50   50   50   49   49   49   48   48   48
    [72]    48   49   49   50   52   55   59   61   63   65   65   66
    [84]    65   65   63   61   58   55   50   46   43   44   46   48
    [96]    49   50   51   51   51   51   51   50   49   47   45   41
    [108]    37   34   35   37   39   41   43   45   47   49   50   51
    [120]    52   53   53   52   50   48   45   41   40
[Record 5]
  [tot_power]       315.31058
  [Tag]         8225
  [re_spec_val]
    [0]    37   44   50   55   58   60   61   61   60   59   56   54
    [12]    52   49   41   46   52   55   55   54   52   49   47   45
    [24]    44   46   48   50   50   50   49   49   48   47   46   47
    [36]    47   46   43   43   46   47   49   50   52   53   53   52
    [48]    49   44   41   44   45   44   43   42   44   45   46   47
    [60]    48   47   44   40   29   28   32   27   23   36   41   43
    [72]    43   42   39   44   48   52   54   56   59   61   62   63
    [84]    64   63   61   58   54   47   40   44   45   44   44   44
    [96]    43   41   35   18   28   30   32   37   40   42   42   41
    [108]    38   33   26   31   34   35   34   33   36   40   43   45
    [120]    46   47   47   47   46   44   42   37   30
[Record 6]
  [tot_power]       3602.5481
  [Tag]         8241
  [re_spec_val]
    [0]    48   51   55   58   62   64   66   67   67   67   67   67
    [12]    67   67   68   69   69   70   70   70   69   69   69   68
    [24]    67   67   66   66   65   65   65   64   64   64   64   65
    [36]    65   66   66   66   66   66   65   66   66   66   65   65
    [48]    64   63   62   61   60   60   59   58   57   57   57   56
    [60]    56   56   55   54   54   53   53   54   55   56   57   58
    [72]    59   61   62   63   63   63   63   63   64   65   67   68
    [84]    68   68   67   66   64   62   60   59   58   56   55   53
    [96]    50   48   47   47   47   48   48   48   48   47   46   45
    [108]    44   44   46   47   49   50   50   51   52   53   53   53
    [120]    52   51   51   51   52   53   54   55   55
[Record 7]
  [tot_power]       10057.778
  [Tag]         8257
  [re_spec_val]
    [0]    45   51   57   61   64   66   68   69   70   71   72   72
    [12]    73   73   74   74   74   74   74   74   73   72   70   69
    [24]    67   66   65   65   65   65   65   65   65   66   67   68
    [36]    70   71   72   73   74   74   74   74   74   73   72   70
    [48]    68   66   63   60   58   57   56   55   55   54   54   54
    [60]    54   54   54   54   54   53   54   55   56   57   58   58
    [72]    59   60   62   64   66   68   70   71   72   73   73   73
    [84]    73   72   70   68   66   63   60   58   56   55   55   53
    [96]    52   51   50   50   50   50   49   49   49   48   48   48
    [108]    48   49   49   50   52   54   55   57   59   60   60   61
    [120]    61   61   60   60   59   57   55   52   50
[Record 8]
  [tot_power]       5620.1919
  [Tag]         8273
  [re_spec_val]
    [0]    37   51   57   61   64   66   67   68   67   66   63   62
    [12]    64   66   68   69   69   69   69   69   69   67   65   62
    [24]    56   52   56   57   56   52   49   54   57   58   55   50
    [36]    59   65   69   71   73   74   74   74   73   72   70   67
    [48]    64   60   57   53   48   40   42   46   46   45   44   44
    [60]    45   45   45   43   40   39   43   46   48   48   46   43
    [72]    45   46   44   52   60   65   69   71   73   74   74   74
    [84]    73   71   68   64   58   53   51   48   39   41   48   50
    [96]    49   47   44   46   48   48   47   43   39   42   45   46
    [108]    45   41   32   41   46   49   50   52   54   57   59   61
    [120]    62   62   61   60   58   55   51   46   43
[Record 9]
  [tot_power]       3671.2175
  [Tag]         8289
  [re_spec_val]
    [0]    48   55   60   63   65   66   67   67   66   65   64   63
    [12]    62   61   60   58   55   50   53   58   61   63   64   65
    [24]    66   66   66   65   64   62   61   58   56   55   57   59
    [36]    62   64   66   67   69   70   71   71   70   68   64   58
    [48]    39   52   55   55   52   49   49   50   49   47   44   39
    [60]    40   45   47   47   45   40   28   39   44   46   46   44
    [72]    38   40   43   47   55   62   66   70   72   73   74   73
    [84]    72   71   68   65   61   57   57   58   57   56   55   53
    [96]    52   53   53   54   54   54   54   53   51   50   49   49
    [108]    49   49   50   49   49   48   46   47   51   55   58   60
    [120]    61   61   61   59   58   55   51   47   45
[Record 10]
  [tot_power]       45199.863
  [Tag]         8305
  [re_spec_val]
    [0]    50   54   59   64   68   70   73   74   75   75   75   75
    [12]    75   75   76   77   78   78   78   78   77   77   76   76
    [24]    76   77   77   78   78   78   77   77   77   77   77   78
    [36]    78   78   78   78   79   80   80   81   81   81   80   79
    [48]    77   76   75   74   73   73   72   71   70   69   68   68
    [60]    67   67   67   68   68   68   69   69   70   70   71   72
    [72]    72   73   73   74   75   76   78   78   79   78   77   74
    [84]    69   66   69   73   74   75   74   74   73   72   71   70
    [96]    70   69   69   69   68   67   66   65   65   65   65   66
    [108]    66   66   66   65   65   65   65   66   67   67   67   66
    [120]    64   61   56   44   49   53   53   48    2
[Record 11]
  [tot_power]       186006.64
  [Tag]         8321
  [re_spec_val]
    [0]    11   55   61   66   69   72   74   75   76   78   79   80
    [12]    81   82   84   84   85   86   86   86   85   85   84   82
    [24]    81   80   79   78   78   78   79   79   79   79   80   80
    [36]    82   83   84   86   87   87   88   88   88   88   87   86
    [48]    84   82   80   77   75   72   71   70   70   69   68   68
    [60]    68   68   68   69   69   70   70   71   72   72   73   73
    [72]    74   75   77   79   81   82   84   85   85   86   85   85
    [84]    85   84   83   82   81   80   79   77   76   75   74   74
    [96]    73   73   72   71   71   70   70   70   69   69   68   68
    [108]    68   68   69   70   71   72   73   73   74   74   74   74
    [120]    73   72   71   70   69   69   68   68   68
[Record 12]
  [tot_power]       139175.20
  [Tag]         8337
  [re_spec_val]
    [0]    53   57   62   66   68   70   71   72   72   71   70   68
    [12]    71   76   80   83   85   86   86   86   85   84   81   77
    [24]    71   60   59   65   66   66   66   65   65   67   68   69
    [36]    69   73   78   81   84   86   88   88   88   88   87   85
    [48]    82   79   74   69   63   61   60   56   51   49   51   52
    [60]    50   49   50   53   56   58   59   60   60   60   60   62
    [72]    63   64   66   70   74   78   81   84   85   87   87   87
    [84]    87   86   84   81   78   73   68   66   66   65   64   63
    [96]    63   63   63   62   61   61   61   61   59   58   57   57
    [108]    57   56   57   60   63   66   68   70   72   73   73   74
    [120]    74   73   73   71   70   69   67   66   65
[Record 13]
  [tot_power]       67697.734
  [Tag]         8353
  [re_spec_val]
    [0]    50   56   61   64   66   67   67   67   66   65   60   50
    [12]    66   73   77   80   82   84   84   84   84   83   81   78
    [24]    74   67   64   67   68   69   69   68   68   66   65   66
    [36]    69   72   75   78   80   82   83   84   84   84   83   81
    [48]    79   76   71   65   56   54   55   53   48   44   48   49
    [60]    49   47   44   39   38   43   47   50   52   52   51   43
    [72]    49   57   61   65   68   72   75   79   81   83   84   84
    [84]    84   83   82   79   76   72   65   55   57   60   59   57
    [96]    55   52   52   53   54   54   54   53   53   51   50   49
    [108]    50   52   54   56   57   57   58   59   61   63   65   66
    [120]    66   66   65   63   61   58   55   53   52
[Record 14]
  [tot_power]       82619.055
  [Tag]         8369
  [re_spec_val]
    [0]    46   51   59   64   69   71   73   75   75   76   77   77
    [12]    76   73   65   68   77   81   84   85   85   84   83   79
    [24]    74   63   71   76   78   80   81   81   82   81   80   79
    [36]    78   79   80   81   81   79   75   65   75   80   82   83
    [48]    83   81   79   76   75   75   75   75   74   73   73   73
    [60]    72   72   72   72   72   72   72   73   73   74   74   74
    [72]    74   74   75   76   76   74   73   76   80   83   84   84
    [84]    83   81   77   66   66   74   76   76   75   74   72   71
    [96]    70   69   69   69   70   70   69   68   67   65   64   63
    [108]    64   64   64   64   64   63   62   61   59   57   58   61
    [120]    63   64   63   62   59   57   55   54   54
[Record 15]
  [tot_power]       476313.34
  [Tag]         8385
  [re_spec_val]
    [0]    45   55   62   66   70   73   75   77   78   79   80   80
    [12]    81   82   83   84   86   87   88   89   90   89   89   88
    [24]    87   86   85   85   84   84   84   84   85   85   85   85
    [36]    86   86   87   88   88   89   89   90   90   90   91   90
    [48]    90   89   88   86   84   82   80   78   77   76   76   76
    [60]    76   76   76   76   76   76   77   77   78   78   79   79
    [72]    80   81   83   84   85   87   88   89   90   91   91   91
    [84]    91   90   89   87   86   85   84   83   82   81   80   79
    [96]    78   77   77   76   76   75   74   74   73   72   72   71
    [108]    71   71   71   72   72   72   73   73   73   74   74   74
    [120]    74   74   73   73   72   70   69   68   68
[Record 16]
  [tot_power]       474768.25
  [Tag]         8401
  [re_spec_val]
    [0]    56   58   63   67   70   72   74   75   75   75   75   73
    [12]    72   75   79   82   85   87   89   90   90   90   89   88
    [24]    86   83   79   76   76   76   77   77   76   77   77   78
    [36]    78   78   79   82   85   88   90   91   92   93   93   92
    [48]    91   89   87   83   79   74   70   69   68   66   65   65
    [60]    66   67   67   67   67   68   68   69   70   70   71   72
    [72]    73   75   76   79   82   85   88   90   92   93   93   93
    [84]    93   92   91   89   86   83   80   76   75   75   75   74
    [96]    73   72   72   71   71   70   69   68   67   66   65   64
    [108]    64   64   65   66   67   69   70   71   73   74   74   75
    [120]    75   75   75   75   74   73   73   72   72
[Record 17]
  [tot_power]       198083.11
  [Tag]         8417
  [re_spec_val]
    [0]    53   56   61   65   67   68   69   67   66   63   60   61
    [12]    67   72   76   79   81   83   84   85   85   85   85   84
    [24]    83   80   77   73   67   65   67   67   66   61   58   63
    [36]    65   66   67   72   78   82   85   87   89   90   90   89
    [48]    88   86   83   79   74   66   62   62   60   54   47   52
    [60]    52   49   40   21   33   45   51   53   52   46   50   59
    [72]    64   67   67   67   70   77   81   85   88   89   90   91
    [84]    90   89   88   85   82   78   73   66   57   48   53   57
    [96]    57   55   49   48   52   53   52   49   45   47   49   51
    [108]    50   49   47   50   55   58   60   61   62   62   62   64
    [120]    65   66   67   67   66   65   62   57   51
[Record 18]
  [tot_power]       82090.984
  [Tag]         8433
  [re_spec_val]
    [0]    33   46   58   65   69   71   72   72   72   71   71   70
    [12]    67   61   68   74   77   78   77   73   64   73   79   81
    [24]    82   81   79   75   68   63   71   74   75   75   75   74
    [36]    74   73   71   64   68   77   82   84   85   86   85   84
    [48]    83   82   81   80   78   76   73   71   72   72   71   70
    [60]    69   69   69   69   68   68   67   68   70   70   70   69
    [72]    69   69   68   62   62   74   79   82   84   85   86   85
    [84]    85   84   83   82   80   76   69   61   69   71   71   68
    [96]    65   63   63   63   63   64   65   65   64   62   61   60
    [108]    59   59   60   61   62   61   60   58   55   53   48   39
    [120]    51   57   60   61   61   60   57   52   32
aArDvArK

## RUN PROGRAM (1) --- minimal options

echo 'Running command (1):'
echo '    ./barkspec -X tstA.spec out1.spec 2>log1.txt'
./barkspec -X tstA.spec out1.spec 2>log1.txt

## CHECK OUTPUT (1)

## --- STDERR OUTPUT (1)

## --- Make comparison text file (1)

echo '... making comparison file log1.ref'

cat > log1.ref <<aArDvArK
-------------bark_freqs------------  ---------------freqs---------------
  low edge      peak     high edge     low edge      peak     high edge 
      0.000       0.590       1.000          0.0        59.1       100.5
      0.972       1.562       1.972         97.6       157.9       200.7
      1.943       2.533       2.943        197.7       260.9       306.3
      2.915       3.505       3.915        303.1       370.8       419.9
      3.887       4.477       4.887        416.4       490.4       544.5
      4.858       5.448       5.858        540.7       622.9       683.5
      5.830       6.420       6.830        679.2       771.7       840.4
      6.802       7.392       7.802        835.5       940.9      1019.4
      7.773       8.363       8.773       1013.8      1134.7      1225.2
      8.745       9.335       9.745       1218.7      1358.4      1463.1
      9.717      10.307      10.717       1455.7      1617.8      1739.6
     10.688      11.278      11.688       1730.9      1919.7      2061.8
     11.660      12.250      12.660       2051.6      2272.1      2438.1
     12.632      13.222      13.632       2426.3      2684.2      2878.6
     13.603      14.193      14.603       2864.7      3166.9      3394.7
     14.575      15.165      15.575       3378.4      3732.7      4000.0
aArDvArK

## --- Check stderr output (1)

echo '... checking output'

if sed 's/-0/0/g' log1.txt | diff -b log1.ref -
then :
else echo "* * * ERROR: bad frequency table in log1.txt"
fi

## --- FEA_SPEC OUTPUT (1)

## --- Make FEA_SPEC comparison file (1)

echo '... making comparison file ref1.spec'

esig2fea - ref1.spec <<aArDvArK
Esignal
   0.0B
  ASCII
     48
   2421
     -1
Tag: LONG <r>
tot_power: FLOAT <r>
re_spec_val: FLOAT 16 <r>
band_high: DOUBLE 1 <g> [0]        4000.000000000000
nan: LONG 1 <g> [0]            0
bark_high: DOUBLE 1 <g> [0]        15.57507173489807
band_low: DOUBLE 1 <g> [0]        0.000000000000000
src_sf: DOUBLE 1 <g> [0]        8000.000000000000
add_const: DOUBLE 1 <g> [0]        0.000000000000000
frmlen: LONG 1 <g> [0]           64
freqs: FLOAT 16 <g> 
  [0]       59.095127      157.93637      260.92877      370.77933
  [4]       490.37534      622.86011      771.71594      940.85535
  [8]       1134.7238      1358.4169      1617.8140      1919.7334
  [12]       2272.1101      2684.2063      3166.8530      3732.7361
frame_meth: SHORT 1 <g> [0]       1
frame_meth.enumStrings: CHAR 3 9 <g> 
  [0][0]  "NONE\0\0\0\0\0"
  [1][0]  "FIXED\0\0\0\0"
  [2][0]  "VARIABLE\0"
bark_low: DOUBLE 1 <g> [0]        0.000000000000000
contin: SHORT 1 <g> [0]       0
contin.enumStrings: CHAR 2 4 <g> 
  [0][0]  "NO\0\0"
  [1][0]  "YES\0"
bark_freqs: FLOAT 16 <g> 
  [0]      0.58999997      1.5616714      2.5333428      3.5050144
  [4]       4.4766860      5.4483571      6.4200287      7.3917003
  [8]       8.3633718      9.3350430      10.306714      11.278386
  [12]       12.250057      13.221729      14.193400      15.165071
start: LONG 1 <g> [0]            1
record_freq: DOUBLE 1 <g> [0]        500.0000000000000
spec_type: SHORT 1 <g> [0]       2
spec_type.enumStrings: CHAR 5 5 <g> 
  [0][0]  "NONE\0"
  [1][0]  "PWR\0\0"
  [2][0]  "DB\0\0\0"
  [3][0]  "REAL\0"
  [4][0]  "CPLX\0"
freq_format: SHORT 1 <g> [0]       6
freq_format.enumStrings: CHAR 7 10 <g> 
  [0][0]  "NONE\0\0\0\0\0\0"
  [1][0]  "SYM_CTR\0\0\0"
  [2][0]  "SYM_EDGE\0\0"
  [3][0]  "ASYM_CTR\0\0"
  [4][0]  "ASYM_EDGE\0"
  [5][0]  "ARB_VAR\0\0\0"
  [6][0]  "ARB_FIXED\0"
num_freqs: LONG 1 <g> [0]           16
start_time: DOUBLE 1 <g> [0]        1.022000000000000
mult_const: DOUBLE 1 <g> [0]        1.000000000000000
FeaSubtype: SHORT <g>      7
FeaSubtype.enumStrings: CHAR 10 10 <g> 
  [0][0]  "NONE\0\0\0\0\0\0"
  [1][0]  "FEA_VQ\0\0\0\0"
  [2][0]  "FEA_ANA\0\0\0"
  [3][0]  "FEA_STAT\0\0"
  [4][0]  "FEA_QHIST\0"
  [5][0]  "FEA_DST\0\0\0"
  [6][0]  "FEA_2KB\0\0\0"
  [7][0]  "FEA_SPEC\0\0"
  [8][0]  "FEA_SD\0\0\0\0"
  [9][0]  "FEA_FILT\0\0"
recordFreq: DOUBLE <g>       500.0000000000000
startTime: DOUBLE 1 <g> [0]        1.022000000000000
[Record 0]
  [tot_power]       42.399658
  [re_spec_val]
    [0]       3.5103896      8.9551206      9.1528567      6.1995770
    [4]       8.0092156      7.0534674     0.90036376     -3.9100679
    [8]     -0.43934887     -2.6168313     -4.2253807      2.4062002
    [12]       5.5793132      4.8672800      5.2945107      3.1609402
  [Tag]         8145
[Record 1]
  [tot_power]       215.90933
  [re_spec_val]
    [0]     -0.19742317      6.4291090      9.3519240      8.0724287
    [4]       4.8894380      7.6132725      8.4866250      9.8758284
    [8]       10.368521      10.958322      12.178205      12.043165
    [12]       13.833406      18.727071      16.129450      12.736018
  [Tag]         8161
[Record 2]
  [tot_power]       1402.1790
  [re_spec_val]
    [0]       3.0528512      10.328858      12.606077      10.879074
    [4]       11.515579      12.215033      15.170611      17.945575
    [8]       20.449600      23.807027      21.784277      19.948273
    [12]       22.128998      25.884228      23.819357      20.361325
  [Tag]         8177
[Record 3]
  [tot_power]       1747.7202
  [re_spec_val]
    [0]       7.3769630      15.035285      19.659208      21.040455
    [4]       20.177151      16.121975      15.638508      17.942649
    [8]       20.195782      26.406428      22.476255      18.077668
    [12]       21.795112      25.604008      23.053860      19.574099
  [Tag]         8193
[Record 4]
  [tot_power]       942.25507
  [re_spec_val]
    [0]       7.1561550      15.866911      20.460105      20.559318
    [4]       18.563381      15.132984      12.887361      12.681875
    [8]       17.281877      21.587135      18.572040      13.417715
    [12]       17.765526      24.347539      18.299556      14.440763
  [Tag]         8209
[Record 5]
  [tot_power]       315.31058
  [re_spec_val]
    [0]       8.7469584      15.705661      16.517569      12.366046
    [4]       9.8408344      10.524865      7.1421009      8.0940017
    [8]       6.7357185      10.376430      8.5263505      6.5687421
    [12]       13.725818      21.612761      14.849239      9.3949666
  [Tag]         8225
[Record 6]
  [tot_power]       3602.5481
  [re_spec_val]
    [0]       13.028597      20.374237      23.537091      24.401106
    [4]       26.468661      27.438309      26.233884      24.551618
    [8]       25.079388      25.965104      23.534752      19.349798
    [12]       23.211125      27.632472      21.647959      17.415465
  [Tag]         8241
[Record 7]
  [tot_power]       10057.778
  [re_spec_val]
    [0]       15.181086      22.480017      26.925159      29.688561
    [4]       31.406706      31.180413      27.586726      25.263117
    [8]       30.010072      33.195538      28.451735      21.288694
    [12]       27.274023      32.326931      25.523942      22.951821
  [Tag]         8257
[Record 8]
  [tot_power]       5620.1919
  [re_spec_val]
    [0]       15.016516      21.866560      23.335628      22.440321
    [4]       25.931436      26.286725      21.370601      16.491377
    [8]       26.320554      32.213492      26.639319      18.146571
    [12]       26.071596      32.098661      24.746720      22.212130
  [Tag]         8273
[Record 9]
  [tot_power]       3671.2175
  [re_spec_val]
    [0]       16.968750      22.077517      22.723364      20.413547
    [4]       16.552463      19.000582      23.539648      21.638446
    [8]       23.952344      28.701189      22.681718      14.801234
    [12]       24.826339      31.486063      24.740534      21.720849
  [Tag]         8289
[Record 10]
  [tot_power]       45199.863
  [re_spec_val]
    [0]       18.528567      26.967931      31.195838      32.374255
    [4]       34.704573      35.446581      35.539743      36.577876
    [8]       37.747670      40.120947      37.281431      32.483146
    [12]       35.785726      37.501883      33.920892      29.760346
  [Tag]         8305
[Record 11]
  [tot_power]       186006.64
  [re_spec_val]
    [0]       20.169967      28.320176      33.561004      38.036749
    [4]       42.036665      43.243270      40.668066      38.635715
    [8]       42.677709      47.128336      43.340903      36.266693
    [12]       41.154982      45.459673      40.003634      36.834299
  [Tag]         8321
[Record 12]
  [tot_power]       139175.20
  [re_spec_val]
    [0]       19.809521      25.976629      28.116779      31.787438
    [4]       41.084548      42.753046      37.352343      30.084587
    [8]       37.565282      46.364320      42.120159      33.535901
    [12]       39.138023      45.787162      38.787015      35.624014
  [Tag]         8337
[Record 13]
  [tot_power]       67697.734
  [re_spec_val]
    [0]       17.960643      22.700172      22.700388      28.093759
    [4]       38.553510      41.170229      36.707814      30.429001
    [8]       34.274466      42.281407      38.222526      29.605742
    [12]       35.020817      42.618432      35.623702      29.160325
  [Tag]         8353
[Record 14]
  [tot_power]       82619.055
  [re_spec_val]
    [0]       18.847870      27.548280      32.033001      32.859434
    [4]       35.735078      41.561801      38.061727      39.147071
    [8]       39.674382      39.872203      39.695179      35.932341
    [12]       37.956260      41.939042      36.665670      30.030471
  [Tag]         8369
[Record 15]
  [tot_power]       476313.34
  [re_spec_val]
    [0]       20.868224      29.504841      34.833331      38.148520
    [4]       42.850995      46.655637      45.817149      44.170409
    [8]       46.021996      49.588144      48.068435      42.120177
    [12]       46.253214      50.881930      45.232106      39.421472
  [Tag]         8385
[Record 16]
  [tot_power]       474768.25
  [re_spec_val]
    [0]       21.259512      28.409810      31.471663      32.644153
    [4]       42.002035      47.075016      44.912580      39.145808
    [8]       40.906055      50.477543      48.479198      40.483559
    [12]       45.774472      52.217668      45.449234      39.559759
  [Tag]         8401
[Record 17]
  [tot_power]       198083.11
  [re_spec_val]
    [0]       18.648035      23.846822      22.921707      27.457425
    [4]       37.884083      42.331019      40.917916      34.047869
    [8]       34.110705      46.902093      45.177360      36.848591
    [12]       41.247452      48.842519      41.763727      33.627019
  [Tag]         8417
[Record 18]
  [tot_power]       82090.984
  [re_spec_val]
    [0]       18.725599      26.667253      28.338981      26.779033
    [4]       32.499975      33.844179      38.081215      34.413605
    [8]       34.408130      42.892437      41.745036      35.570413
    [12]       37.975532      44.444011      37.863330      29.764970
  [Tag]         8433
aArDvArK

## --- Check FEA_SPEC output (1)

echo '... checking output'

if test `featype out1.spec` = "FEA_SPEC"
then

## --- Header (1)

    # tagged?

    if psps -D out1.spec | grep ' not  *tagged' >/dev/null
    then echo "* * * ERROR: file is not tagged; should be tagged"
    fi

    # generic string values

    gv_contin="NO"
    gv_frame_meth="FIXED"
    gv_freq_format="ARB_FIXED"
    gv_spec_type="DB"

    for i in \
	contin      frame_meth  freq_format spec_type
    do
	x=`hditem -i $i out1.spec`
	eval "pat=\$gv_$i"

	case "$x" in
	"$pat") : ;;
	'') echo "* * * ERROR: header item $i is missing" ;;
	*)  echo "* * * ERROR: header item $i is \"$x\"; should be \"$pat\"" ;;
	esac
    done

    # generic integer values

    gv_start="1"
    gv_nan="0"
    gv_num_freqs="16"
    gv_frmlen="64"
    gv_ndrec="19"

    for i in \
	start       nan         num_freqs   frmlen      ndrec
    do
	x=`hditem -i $i out1.spec`
	eval "pat=\$gv_$i"

	case "$x" in
	'') echo "* * * ERROR: header item $i is missing" ;;
	*)
	    case `echo "if ($x == $pat) \"OK\"" | bc` in
	    OK) : ;;
	    *)  echo "* * * ERROR: header item $i is $x; should be $pat" ;;
	    esac
	;;
	esac
    done

    # generic floating-point values

    gv_add_const="0"
    gv_band_high="4000"
    gv_band_low="0"
    gv_bark_high="15.5751"
    gv_bark_low="0"
    gv_mult_const="1"
    gv_record_freq="500"
    gv_src_sf="8000"
    gv_start_time="1.022"

    for i in \
	add_const   band_high   band_low    bark_high   bark_low \
	mult_const  record_freq src_sf      start_time
    do
	x=`hditem -i $i out1.spec`
	eval "pat=\$gv_$i"

	case "$x" in
	'') echo "* * * ERROR: header item $i is missing" ;;
	*)
	    case `echo	"scale=20;" \
			"if (($pat) >= 0) {" \
			"    if (($x) < 0.9999 * ($pat)) \"NOT\";" \
			"    if (($pat) < 0.9999 * ($x)) \"NOT\"};" \
			"if (($pat) < 0) {" \
			"    if (($x) > 0.9999 * ($pat)) \"NOT\";" \
			"    if (($pat) > 0.9999 * ($x)) \"NOT\"};" \
			"\"OK\"" | bc`
	    in
	    OK) : ;;
	    *)  echo "* * * ERROR: header item $i is $x; should be $pat" ;;
	    esac
	;;
	esac
    done

    x=`hditem -i bark_freqs out1.spec`
    y=`echo "0.59 1.56167 2.53334 3.50501 4.47669" \
	"5.44836 6.42003 7.3917 8.36337 9.33504" \
	"10.3067 11.2784 12.2501 13.2217 14.1934" \
	"15.1651"`
    case "$x" in
    '') echo "* * * ERROR: header item bark_freqs is missing" ;;
    *)
	echo "$y" | testsd -a- tmp.sd
	echo "$x" \
	    | testsd -a- - \
	    | addsd -g-1 tmp.sd - - \
	    | feafunc -f samples -f- -FABS - dif.sd
	rm -f tmp.sd
	`get_esps_base`/bin/select -o tmp.sd -q'samples > 1.0e-4' \
						dif.sd 2>/dev/null 1>&2
	n=`hditem -i ndrec tmp.sd`
	if test "$n" -ne 0
	then
	    echo "* * * ERROR: header item bark_freqs is:"
	    echo "$x"
	    echo "should be:"
	    echo "$y"
	fi
    esac

    x=`hditem -i freqs out1.spec`
    y=`echo "59.0951 157.936 260.929 370.779 490.375" \
	"622.860 771.716 940.855 1134.72 1358.42" \
	"1617.81 1919.73 2272.11 2684.21 3166.85" \
	"3732.74"`
    case "$x" in
    '') echo "* * * ERROR: header item freqs is missing" ;;
    *)
	echo "$y" | testsd -a- tmp.sd
	echo "$x" \
	    | testsd -a- - \
	    | addsd -g-1 tmp.sd - - \
	    | feafunc -f samples -f- -FABS - dif.sd
	rm -f tmp.sd
	`get_esps_base`/bin/select -o tmp.sd -q'samples > 1.0e-2' \
						dif.sd 2>/dev/null 1>&2
	n=`hditem -i ndrec tmp.sd`
	if test "$n" -ne 0
	then
	    echo "* * * ERROR: header item freqs is:"
	    echo "$x"
	    echo "should be:"
	    echo "$y"
	fi
    esac

## --- Data (1)

    if feaop -f tot_power -f tot_power \
	     -f tot_p_dif -O SUB out1.spec tstA.spec - \
       | feafunc -ftot_p_dif -f- -FABS - dif1.spec
    then
	rm -f tmp.spec
	`get_esps_base`/bin/select -o tmp.spec -q'tot_p_dif > 0' \
						dif1.spec 2>/dev/null 1>&2
	n=`hditem -i ndrec tmp.spec`

	if test "$n" -ne 0
	then
	    echo "* * * ERROR: tot_power differs from original in $n records"
	fi
    else
	echo "* * * ERROR: couldn't form tot_power difference " \
					"between out1.spec and tstA.spec"
    fi

    if feaop -f re_spec_val -f re_spec_val \
	     -f re_spec_dif -O SUB out1.spec ref1.spec - \
       | feafunc -f re_spec_dif -f- -FABS - dif1.spec 
    then
	rm -f tmp.spec
	`get_esps_base`/bin/select -o tmp.spec \
		-q'max(re_spec_dif) > 1.0e-5' dif1.spec 2>/dev/null 1>&2
	n=`hditem -i ndrec tmp.spec`

	if test "$n" -ne 0
	then
	    echo "* * * ERROR: re_spec_val disagrees with ref in $n records"
	fi
    else
	echo "* * * ERROR: couldn't form re_spec_val difference " \
	     "between out1.spec and ref1.spec"
    fi

else
    echo "* * * ERROR: output file out1.spec is not a FEA_SPEC file"
fi

## RUN PROGRAM (2) --- non-default command-line options

echo 'Running command (2):'
echo '    ./barkspec -a5 -m2 -n12 -r6:14 -B1:14 -SPWR -X tstA.spec out2.spec 2>log2.txt'
./barkspec -a5 -m2 -n12 -r6:14 -B1:14 -SPWR -X tstA.spec out2.spec 2>log2.txt

## CHECK OUTPUT (2)

## --- STDERR OUTPUT (2)

## --- Make comparison text file (2)

echo '... making comparison file log2.ref'

cat > log2.ref <<aArDvArK
-------------bark_freqs------------  ---------------freqs---------------
  low edge      peak     high edge     low edge      peak     high edge 
      1.000       1.590       2.000        100.5       160.9       203.7
      2.091       2.681       3.091        213.3       277.1       322.9
      3.182       3.772       4.182        333.3       402.5       452.9
      4.273       4.863       5.273        464.3       541.3       597.8
      5.364       5.954       6.364        610.7       698.0       762.6
      6.455       7.045       7.455        777.4       877.8       952.6
      7.545       8.135       8.545        969.8      1086.8      1174.2
      8.636       9.226       9.636       1194.3      1331.7      1434.7
      9.727      10.317      10.727       1458.5      1620.9      1742.8
     10.818      11.408      11.818       1771.0      1963.7      2108.7
     11.909      12.499      12.909       2142.2      2371.6      2544.5
     13.000      13.590      14.000       2584.4      2858.2      3064.6
aArDvArK

## --- Check stderr output (2)

echo '... checking output'

if sed 's/-0/0/g' log2.txt | diff -b log2.ref -
then :
else echo "* * * ERROR: bad frequency table in log2.txt"
fi

## --- FEA_SPEC OUTPUT (2)

## --- Check FEA_SPEC output (2)

echo '... checking FEA_SPEC output header'

if test `featype out2.spec` = "FEA_SPEC"
then

## --- Header (2)

    # tagged?

    if psps -D out1.spec | grep ' not  *tagged' >/dev/null
    then echo "* * * ERROR: file is not tagged; should be tagged"
    fi

    # generic string values

    gv_contin="NO"
    gv_frame_meth="FIXED"
    gv_freq_format="ARB_FIXED"
    gv_spec_type="PWR"

    for i in \
	contin      frame_meth  freq_format spec_type
    do
	x=`hditem -i $i out2.spec`
	eval "pat=\$gv_$i"

	case "$x" in
	"$pat") : ;;
	'') echo "* * * ERROR: header item $i is missing" ;;
	*)  echo "* * * ERROR: header item $i is \"$x\"; should be \"$pat\"" ;;
	esac
    done

    # generic integer values

    gv_start="6"
    gv_nan="9"
    gv_num_freqs="12"
    gv_frmlen="64"
    gv_ndrec="9"

    for i in \
	start       nan         num_freqs   frmlen      ndrec
    do
	x=`hditem -i $i out2.spec`
	eval "pat=\$gv_$i"

	case "$x" in
	'') echo "* * * ERROR: header item $i is missing" ;;
	*)
	    case `echo "if ($x == $pat) \"OK\"" | bc` in
	    OK) : ;;
	    *)  echo "* * * ERROR: header item $i is $x; should be $pat" ;;
	    esac
	;;
	esac
    done

    # generic floating-point values

    gv_add_const="5"
    gv_band_high="3064.59"
    gv_band_low="100.464"
    gv_bark_high="14"
    gv_bark_low="1"
    gv_mult_const="2"
    gv_record_freq="500"
    gv_src_sf="8000"
    gv_start_time="1.032"

    for i in \
	add_const   band_high   band_low    bark_high   bark_low \
	mult_const  record_freq src_sf      start_time
    do
	x=`hditem -i $i out2.spec`
	eval "pat=\$gv_$i"

	case "$x" in
	'') echo "* * * ERROR: header item $i is missing" ;;
	*)
	    case `echo	"scale=20;" \
			"if (($pat) >= 0) {" \
			"    if (($x) < 0.9999 * ($pat)) \"NOT\";" \
			"    if (($pat) < 0.9999 * ($x)) \"NOT\"};" \
			"if (($pat) < 0) {" \
			"    if (($x) > 0.9999 * ($pat)) \"NOT\";" \
			"    if (($pat) > 0.9999 * ($x)) \"NOT\"};" \
			"\"OK\"" | bc`
	    in
	    OK) : ;;
	    *)  echo "* * * ERROR: header item $i is $x; should be $pat" ;;
	    esac
	;;
	esac
    done

else
    echo "* * * ERROR: output file out2.spec is not a FEA_SPEC file"
fi

## RUN PROGRAM (3) --- parameter file

## --- MAKE PARAM FILE (3)

echo 'Making param file testparam'

cat > testparam <<aArDvArK
int	start = 3;
int	nan = 0;
int     num_freqs = 0;
float   band_low = 150;
float   band_high = 0;
string  spec_type = "DB";
float   add_const = 2.0;
float   mult_const = -1.0;
aArDvArK

echo 'Running command (3):'
echo '    ./barkspec -P testparam tstA.spec out3.spec 2>/dev/null'
./barkspec -P testparam tstA.spec out3.spec 2>/dev/null

## CHECK OUTPUT (3)

## --- FEA_SPEC OUTPUT (3)

## --- Check FEA_SPEC output (3)

echo '... checking FEA_SPEC output header'

if test `featype out3.spec` = "FEA_SPEC"
then

## --- Header (3)

    # tagged?

    if psps -D out1.spec | grep ' not  *tagged' >/dev/null
    then echo "* * * ERROR: file is not tagged; should be tagged"
    fi

    # generic string values

    gv_contin="NO"
    gv_frame_meth="FIXED"
    gv_freq_format="ARB_FIXED"
    gv_spec_type="DB"

    for i in \
	contin      frame_meth  freq_format spec_type
    do
	x=`hditem -i $i out3.spec`
	eval "pat=\$gv_$i"

	case "$x" in
	"$pat") : ;;
	'') echo "* * * ERROR: header item $i is missing" ;;
	*)  echo "* * * ERROR: header item $i is \"$x\"; should be \"$pat\"" ;;
	esac
    done

    # generic integer values

    gv_start="3"
    gv_nan="0"
    gv_num_freqs="14"
    gv_frmlen="64"
    gv_ndrec="17"

    for i in \
	start       nan         num_freqs   frmlen      ndrec
    do
	x=`hditem -i $i out3.spec`
	eval "pat=\$gv_$i"

	case "$x" in
	'') echo "* * * ERROR: header item $i is missing" ;;
	*)
	    case `echo "if ($x == $pat) \"OK\"" | bc` in
	    OK) : ;;
	    *)  echo "* * * ERROR: header item $i is $x; should be $pat" ;;
	    esac
	;;
	esac
    done

    # generic floating-point values

    gv_add_const="2"
    gv_band_high="4000"
    gv_band_low="150"
    gv_bark_high="15.5751"
    gv_bark_low="1.4848"
    gv_mult_const="-1"
    gv_record_freq="500"
    gv_src_sf="8000"
    gv_start_time="1.026"

    for i in \
	add_const   band_high   band_low    bark_high   bark_low \
	mult_const  record_freq src_sf      start_time
    do
	x=`hditem -i $i out3.spec`
	eval "pat=\$gv_$i"

	case "$x" in
	'') echo "* * * ERROR: header item $i is missing" ;;
	*)
	    case `echo	"scale=20;" \
			"if (($pat) >= 0) {" \
			"    if (($x) < 0.9999 * ($pat)) \"NOT\";" \
			"    if (($pat) < 0.9999 * ($x)) \"NOT\"};" \
			"if (($pat) < 0) {" \
			"    if (($x) > 0.9999 * ($pat)) \"NOT\";" \
			"    if (($pat) > 0.9999 * ($x)) \"NOT\"};" \
			"\"OK\"" | bc`
	    in
	    OK) : ;;
	    *)  echo "* * * ERROR: header item $i is $x; should be $pat" ;;
	    esac
	;;
	esac
    done

else
    echo "* * * ERROR: output file out3.spec is not a FEA_SPEC file"
fi
