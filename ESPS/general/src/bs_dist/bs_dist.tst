#!/bin/sh
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
# @(#)bs_dist.tst	1.4	9/22/98	ERL
# 
# Written by:  Rod Johnson
# Checked by:
# Revised by:
# 
# Brief description:  Test script for bs_dist program.
#   "bs_dist.tst clean" just removes the test files.
#   "bs_dist.tst" runs the tests and leaves the test files in place.
# 

##!# So far, only BSD computation is checked; MBSD not yet implemented

## CLEAN UP TEST FILES

rm -f tstA.bark tstB.bark log1.txt err1.txt out1.fea ref1.fea dif1.fea tmp.fea \
	tmp.sd dif.sd \
	log2.txt err2.txt out2.fea ref2.fea dif2.fea log3.txt err3.txt \
	testparam log4.txt err4.txt out4.fea ref4.fea dif4.fea
	

case "$1" in
clean)
    exit 0;;
esac

## TURN OFF COMMON PROCESSING

USE_ESPS_COMMON=off; export USE_ESPS_COMMON
ESPS_VERBOSE=0; export ESPS_VERBOSE

## MAKE TEST FILE

echo "Making test FEA_SPEC file tstA.bark."

esig2fea - tstA.bark <<aArDvArK
Esignal
   0.0B
  ASCII
     48
   2391
     -1
Tag: LONG <r>
tot_power: FLOAT <r>
re_spec_val: FLOAT 15 <r>
band_high: DOUBLE 1 <g> [0]        4232.134445996039
nan: LONG 1 <g> [0]           10
bark_high: DOUBLE 1 <g> [0]        15.91000000000000
band_low: DOUBLE 1 <g> [0]        91.34927693803206
src_sf: DOUBLE 1 <g> [0]        8000.000000000000
add_const: DOUBLE 1 <g> [0]        0.000000000000000
frmlen: LONG 1 <g> [0]           64
freqs: FLOAT 15 <g> 
  [0]       151.56738      257.29684      370.19000      493.39005
  [4]       630.32709      784.81384      961.15143      1164.2496
  [8]       1399.7629      1674.2487      1995.3490      2372.0042
  [12]       2814.7009      3335.7651      3949.7041
frame_meth: SHORT 1 <g> [0]       1
frame_meth.enumStrings: CHAR 3 9 <g> 
  [0][0]  "NONE\0\0\0\0\0"
  [1][0]  "FIXED\0\0\0\0"
  [2][0]  "VARIABLE\0"
bark_low: DOUBLE 1 <g> [0]       0.9100000000000000
contin: SHORT 1 <g> [0]       0
contin.enumStrings: CHAR 2 4 <g> 
  [0][0]  "NO\0\0"
  [1][0]  "YES\0"
bark_freqs: FLOAT 15 <g> 
  [0]       1.5000000      2.5000000      3.5000000      4.5000000
  [4]       5.5000000      6.5000000      7.5000000      8.5000000
  [8]       9.5000000      10.500000      11.500000      12.500000
  [12]       13.500000      14.500000      15.500000
start: LONG 1 <g> [0]          231
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
num_freqs: LONG 1 <g> [0]           15
start_time: DOUBLE 1 <g> [0]       0.4640000000000000
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
startTime: DOUBLE 1 <g> [0]       0.4640000000000000
[Record 0]
  [tot_power]       242555.52
  [re_spec_val]
    [0]       29.663557      28.396418      30.219484      28.876633
    [4]       21.482067      13.720979      10.888761      11.709397
    [8]       17.294838      30.010786      43.255474      42.783257
    [12]       42.324333      39.251869      31.454790
  [Tag]         3681
[Record 1]
  [tot_power]       174389.59
  [re_spec_val]
    [0]       30.743521      33.547001      33.439911      29.605940
    [4]       22.213465      13.075278      6.3877974      9.1696949
    [8]       12.798846      28.049150      41.557106      41.516144
    [12]       40.040428      35.797543      27.943287
  [Tag]         3697
[Record 2]
  [tot_power]       86975.258
  [re_spec_val]
    [0]       28.405716      27.902861      29.256514      26.689030
    [4]       18.940176      9.9866962      7.9458594      9.8666840
    [8]       13.559008      26.825006      37.589119      38.206085
    [12]       37.591721      35.140175      28.534016
  [Tag]         3713
[Record 3]
  [tot_power]       129552.16
  [re_spec_val]
    [0]       27.952274      28.903540      29.411940      26.646486
    [4]       19.329670      11.644895      6.0899677      8.7205639
    [8]       11.806746      25.650608      35.932850      39.725712
    [12]       40.715481      38.432133      31.121347
  [Tag]         3729
[Record 4]
  [tot_power]       99775.734
  [re_spec_val]
    [0]       28.753832      31.423605      30.557861      26.522779
    [4]       19.482857      11.082741      7.5231328      6.8076739
    [8]       10.115880      21.537640      35.529922      38.626617
    [12]       38.585304      37.689140      30.944572
  [Tag]         3745
[Record 5]
  [tot_power]       83910.047
  [re_spec_val]
    [0]       25.954548      23.927059      23.438707      20.714729
    [4]       13.488000      6.9182453      6.1874480      5.3739300
    [8]       13.200861      21.067284      35.856934      37.075962
    [12]       35.364731      39.540543      33.286533
  [Tag]         3761
[Record 6]
  [tot_power]       69719.844
  [re_spec_val]
    [0]       25.626835      26.512770      25.430645      22.093824
    [4]       15.265566      8.8331690      6.4415836      2.8061519
    [8]       9.6351757      21.139547      35.278156      37.634693
    [12]       36.046173      36.760857      30.213367
  [Tag]         3777
[Record 7]
  [tot_power]       42249.426
  [re_spec_val]
    [0]       25.891169      27.302652      24.902618      20.003052
    [4]       13.753987      6.8842916      3.6105883     0.90215135
    [8]       5.3185997      14.947868      28.620888      33.137825
    [12]       35.471382      35.012844      29.311127
  [Tag]         3793
[Record 8]
  [tot_power]       48271.855
  [re_spec_val]
    [0]       23.011572      21.580339      16.709719      10.136030
    [4]       7.8402333      4.5077543      1.2551110      4.7663202
    [8]       5.2770996      14.549771      22.676336      29.975960
    [12]       36.041245      38.024395      32.582455
  [Tag]         3809
[Record 9]
  [tot_power]       46305.453
  [re_spec_val]
    [0]       21.462326      19.949072      15.450261      11.750896
    [4]       4.3953686     -1.3462790      4.0604372      4.8471174
    [8]       5.3158236      12.938688      23.592873      28.327595
    [12]       36.237144      37.826572      31.319035
  [Tag]         3825
aArDvArK

echo "Making test FEA_SPEC file tstB.bark."

esig2fea - tstB.bark <<aArDvArK
Esignal
   0.0B
  ASCII
     48
   2481
     -1
Tag: LONG <r>
tot_power: FLOAT <r>
re_spec_val: FLOAT 15 <r>
band_high: DOUBLE 1 <g> [0]        4232.134445996039
nan: LONG 1 <g> [0]           10
bark_high: DOUBLE 1 <g> [0]        15.91000000000000
band_low: DOUBLE 1 <g> [0]        91.34927693803206
src_sf: DOUBLE 1 <g> [0]        8000.000000000000
add_const: DOUBLE 1 <g> [0]        0.000000000000000
frmlen: LONG 1 <g> [0]           64
freqs: FLOAT 15 <g> 
  [0]       151.56738      257.29684      370.19000      493.39005
  [4]       630.32709      784.81384      961.15143      1164.2496
  [8]       1399.7629      1674.2487      1995.3490      2372.0042
  [12]       2814.7009      3335.7651      3949.7041
frame_meth: SHORT 1 <g> [0]       1
frame_meth.enumStrings: CHAR 3 9 <g> 
  [0][0]  "NONE\0\0\0\0\0"
  [1][0]  "FIXED\0\0\0\0"
  [2][0]  "VARIABLE\0"
bark_low: DOUBLE 1 <g> [0]       0.9100000000000000
contin: SHORT 1 <g> [0]       0
contin.enumStrings: CHAR 2 4 <g> 
  [0][0]  "NO\0\0"
  [1][0]  "YES\0"
bark_freqs: FLOAT 15 <g> 
  [0]       1.5000000      2.5000000      3.5000000      4.5000000
  [4]       5.5000000      6.5000000      7.5000000      8.5000000
  [8]       9.5000000      10.500000      11.500000      12.500000
  [12]       13.500000      14.500000      15.500000
start: LONG 1 <g> [0]          231
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
num_freqs: LONG 1 <g> [0]           15
start_time: DOUBLE 1 <g> [0]       0.4640000000000000
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
startTime: DOUBLE 1 <g> [0]       0.4640000000000000
[Record 0]
  [tot_power]       397994.44
  [re_spec_val]
    [0]       32.018623      31.108500      32.563793      31.758924
    [4]       24.988129      19.581486      15.488555      20.978283
    [8]       22.781315      32.585117      45.199688      44.812080
    [12]       44.410549      41.360691      33.895874
  [Tag]         3681
[Record 1]
  [tot_power]       270401.06
  [re_spec_val]
    [0]       32.813587      36.175819      35.783970      31.572130
    [4]       24.197098      18.802086      17.654106      19.309597
    [8]       20.881960      31.569962      43.477551      42.963657
    [12]       41.996323      37.973164      31.265919
  [Tag]         3697
[Record 2]
  [tot_power]       145414.94
  [re_spec_val]
    [0]       30.932507      30.306520      31.367304      29.157520
    [4]       21.316811      13.917584      15.785312      14.362242
    [8]       17.995085      29.523840      39.412792      40.518703
    [12]       39.978100      37.639103      31.622145
  [Tag]         3713
[Record 3]
  [tot_power]       243444.05
  [re_spec_val]
    [0]       30.698753      31.671505      32.073204      29.510229
    [4]       22.263414      14.955903      10.139651      13.165384
    [8]       15.072941      28.609068      38.771400      42.662777
    [12]       43.549992      41.277531      34.002434
  [Tag]         3729
[Record 4]
  [tot_power]       181956.33
  [re_spec_val]
    [0]       31.607059      33.692760      32.861916      28.746243
    [4]       21.627264      14.046458      13.250893      13.401849
    [8]       13.185095      24.165268      37.871647      40.964314
    [12]       41.025005      40.276840      33.732368
  [Tag]         3745
[Record 5]
  [tot_power]       157657.42
  [re_spec_val]
    [0]       28.473217      26.525639      26.222191      23.240984
    [4]       16.025423      9.7309465      10.007570      9.6405325
    [8]       15.268126      23.800482      38.534725      39.981258
    [12]       38.240608      42.485420      36.204845
  [Tag]         3761
[Record 6]
  [tot_power]       132161.28
  [re_spec_val]
    [0]       28.602467      29.179214      27.958626      24.610664
    [4]       17.791395      11.496871      9.3453283      5.6887913
    [8]       12.417234      24.072973      38.103664      40.329197
    [12]       38.650494      39.288956      32.775776
  [Tag]         3777
[Record 7]
  [tot_power]       80088.234
  [re_spec_val]
    [0]       28.886436      30.288443      27.700891      22.903807
    [4]       16.707336      9.7523394      6.3724313      3.6298389
    [8]       8.1320410      17.887991      31.578312      35.989456
    [12]       38.262886      37.682182      32.003349
  [Tag]         3793
[Record 8]
  [tot_power]       91504.445
  [re_spec_val]
    [0]       25.769403      24.424160      19.434307      12.848168
    [4]       10.384578      7.0570717      4.0224533      7.4407725
    [8]       7.8347178      17.166056      25.488081      32.799145
    [12]       38.812801      40.685162      35.200878
  [Tag]         3809
[Record 9]
  [tot_power]       87776.914
  [re_spec_val]
    [0]       24.127790      22.603765      18.285845      14.589011
    [4]       7.1649194      1.3761685      6.9716907      7.6187549
    [8]       8.1290894      15.518770      26.212460      31.045504
    [12]       39.174236      40.793091      34.230946
  [Tag]         3825
aArDvArK

## RUN PROGRAM (1) --- near-minimal options

echo 'Running command (1):'
echo '    ./bs_dist -A tstA.bark tstB.bark out1.fea >log1.txt 2>err1.txt'
./bs_dist -A tstA.bark tstB.bark out1.fea >log1.txt 2>err1.txt
cat err1.txt

## CHECK OUTPUT (1)

## --- STDOUT OUTPUT (1)

echo '... checking stdout output'

x=`cat log1.txt`
pat="0.0393814424239233225964"

case "$x" in
'') echo "* * * ERROR: no output to stdout";;
*)
    case `echo	"scale=20;" \
		"if (($x) < 0.9999 * ($pat)) \"NOT\";" \
		"if (($pat) < 0.9999 * ($x)) \"NOT\";" \
		"\"OK\"" | bc`
    in
    OK) : ;;
    *)  echo "* * * ERROR: output distortion is $x; should be $pat" ;;
    esac
;;
esac

## --- FEA OUTPUT (1)

## --- Make FEA comparison file (1)

echo '... making comparison file ref1.fea'

esig2fea - ref1.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
    902
     -1
Tag: LONG <r>
BSD: FLOAT <r>
nan: LONG 1 <g> [0]            0
perceptual_weights: FLOAT 15 <g> 
  [0]       1.0005122      1.0014822      1.0030890      1.0055434
  [4]       1.0091827      1.0145425      1.0224882      1.0344750
  [8]       1.0530953      1.0833555      1.1360345      1.2380757
  [12]       1.4722099      2.1588106      3.6894841
src_sf: DOUBLE 1 <g> [0]        8000.000000000000
distortion_type: SHORT 1 <g> [0]       1
distortion_type.enumStrings: CHAR 3 5 <g> 
  [0][0]  "NONE\0"
  [1][0]  "BSD\0\0"
  [2][0]  "MBSD\0"
start: LONG 2 <g> 
  [0]            1           1
record_freq: DOUBLE 1 <g> [0]        500.0000000000000
threshold: DOUBLE 1 <g> [0]        0.000000000000000
start_time: DOUBLE 1 <g> [0]       0.4640000000000000
recordFreq: DOUBLE <g>       500.0000000000000
startTime: DOUBLE 1 <g> [0]       0.4640000000000000
[Record 0]
  [BSD]      0.259441035758503373843589
  [Tag]         3681
[Record 1]
  [BSD]      0.225806772656761456036976
  [Tag]         3697
[Record 2]
  [BSD]      0.171354368212255086191079
  [Tag]         3713
[Record 3]
  [BSD]      0.293333686325420494010447
  [Tag]         3729
[Record 4]
  [BSD]      0.199340179579040831664097
  [Tag]         3745
[Record 5]
  [BSD]      0.238965539533852768877493
  [Tag]         3761
[Record 6]
  [BSD]      0.176473200845952222026501
  [Tag]         3777
[Record 7]
  [BSD]      0.146906793039507086591554
  [Tag]         3793
[Record 8]
  [BSD]      0.144211633144469068103498
  [Tag]         3809
[Record 9]
  [BSD]      0.157219425631123442882853
  [Tag]         3825
aArDvArK

## --- Check FEA output (1)

echo '... checking output'

## --- Header (1)

# tagged?

if psps -D out1.fea | grep ' not  *tagged' >/dev/null
then echo "* * * ERROR: file is not tagged; should be tagged"
fi

# generic string values

gv_distortion_type="BSD"

for i in \
    distortion_type
do
    x=`hditem -i $i out1.fea`
    eval "pat=\$gv_$i"

    case "$x" in
    "$pat") : ;;
    '') echo "* * * ERROR: header item $i is missing" ;;
    *)  echo "* * * ERROR: header item $i is \"$x\"; should be \"$pat\"" ;;
    esac
done

# generic integer values

    gv_nan="0"
    gv_ndrec="10"

    for i in \
	nan	ndrec
    do
	x=`hditem -i $i out1.fea`
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

    x=`hditem -i start out1.fea`
    start_0=1
    start_1=1
    case "$x" in
    '') echo "* * * ERROR: header item start is missing" ;;
    *)
	n=0
	for i in $x
	do
	    n=`expr $n + 1`
	done
	if test $n -ne 2
	then
	    echo "* * * ERROR: header item start has wrong length"
	else
	    n=0
	    for i in $x
	    do
		eval "y=\$start_$n"
		if test $i -ne $y
		then
		    echo "* * * ERROR: header item start[$n] is $i;" \
			 "should be $y"
		fi
		n=`expr $n + 1`
	    done
	fi
	;;
    esac

    # generic floating-point values

    gv_record_freq="500"
    gv_src_sf="8000"
    gv_start_time="0.464"

    for i in \
	record_freq   src_sf        start_time
    do
	x=`hditem -i $i out1.fea`
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

    x=`hditem -i perceptual_weights out1.fea`
    y=`echo "1.00051224 1.00148221 1.00308900 1.00554334 1.00918270" \
	"1.01454247 1.02248820 1.03447494 1.05309536 1.08335560" \
	"1.13603451 1.23807577 1.47220992 2.15881055 3.68948410"`
    case "$x" in
    '') echo "* * * ERROR: header item perceptual_weights is missing" ;;
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
	    echo "* * * ERROR: header item perceptual_weights is:"
	    echo "$x"
	    echo "should be:"
	    echo "$y"
	fi
    esac

## --- Data (1)

if feaop -f BSD -f BSD \
	 -f BSD_dif -O SUB out1.fea ref1.fea - \
   | feafunc -fBSD_dif -f- -FABS - dif1.fea
then
    rm -f tmp.fea
    `get_esps_base`/bin/select -o tmp.fea \
	    -q'max(BSD_dif) > 1.0e-5' dif1.fea 2>/dev/null 1>&2
    n=`hditem -i ndrec tmp.fea`

    if test "$n" -ne 0
    then
	echo "* * * ERROR: BSD disagrees with ref in $n records"
    fi
else
    echo "* * * ERROR: couldn't form BSD difference " \
	 "between out1.fea and ref1.fea"
fi

## RUN PROGRAM (2) --- non-default command-line options except -mBSD

echo 'Running command (2):'
echo '    ./bs_dist -e81000 -mBSD -r2:8 -r3: \
            tstA.bark tstB.bark out2.fea >log2.txt 2>err2.txt'
./bs_dist -e81000 -mBSD -r2:8 -r3: \
	tstA.bark tstB.bark out2.fea >log2.txt 2>err2.txt
grep -v 'tags mismatch' err2.txt

## CHECK OUTPUT (2)

## --- STDOUT OUTPUT (2)

echo '... checking stdout output'

x=`cat log2.txt`

case "$x" in
'') : ;;
*) echo "* * * ERROR: output to stdout";;
esac

## --- FEA OUTPUT (2)

## --- Make FEA comparison file (2)

echo '... making comparison file ref2.fea'

esig2fea - ref2.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
  14446
     -1
Tag: LONG <r>
BSD: FLOAT <r>
nan: LONG 1 <g> [0]            7
perceptual_weights: FLOAT 15 <g> 
  [0]       1.0005122      1.0014822      1.0030890      1.0055434
  [4]       1.0091827      1.0145425      1.0224882      1.0344750
  [8]       1.0530953      1.0833555      1.1360345      1.2380757
  [12]       1.4722099      2.1588106      3.6894841
src_sf: DOUBLE 1 <g> [0]        8000.000000000000
distortion_type: SHORT 1 <g> [0]       1
distortion_type.enumStrings: CHAR 3 5 <g> 
  [0][0]  "NONE\0"
  [1][0]  "BSD\0\0"
  [2][0]  "MBSD\0"
start: LONG 2 <g> 
  [0]            2           3
record_freq: DOUBLE 1 <g> [0]        500.0000000000000
threshold: DOUBLE 1 <g> [0]        81000.00000000000
start_time: DOUBLE 1 <g> [0]       0.4660000000000000
recordFreq: DOUBLE <g>       500.0000000000000
startTime: DOUBLE 1 <g> [0]       0.4660000000000000
[Record 0]
  [BSD]      0.125100796891321760742254
  [Tag]         3697
[Record 1]
  [BSD]      0.732483504072416105093803
  [Tag]         3713
[Record 2]
  [BSD]      0.150571254998363218451734
  [Tag]         3729
[Record 3]
  [BSD]      0.400365376452316757398863
  [Tag]         3745
[Record 4]
  [BSD]      0.166817687882281132201733
  [Tag]         3761
[Record 5]
  [BSD]      0.103057459831735630700981
  [Tag]         3777
[Record 6]
  [BSD]      0.397332098269955608145606
  [Tag]         3793
aArDvArK

## --- Check FEA output (2)

echo '... checking output'

## --- Header (2)

# tagged?

if psps -D out2.fea | grep ' not  *tagged' >/dev/null
then echo "* * * ERROR: file is not tagged; should be tagged"
fi

# generic string values

gv_distortion_type="BSD"

for i in \
    distortion_type
do
    x=`hditem -i $i out2.fea`
    eval "pat=\$gv_$i"

    case "$x" in
    "$pat") : ;;
    '') echo "* * * ERROR: header item $i is missing" ;;
    *)  echo "* * * ERROR: header item $i is \"$x\"; should be \"$pat\"" ;;
    esac
done

# generic integer values

    gv_nan="7"
    gv_ndrec="7"

    for i in \
	nan         ndrec
    do
	x=`hditem -i $i out2.fea`
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

    x=`hditem -i start out2.fea`
    start_0=2
    start_1=3

    case "$x" in
    '') echo "* * * ERROR: header item start is missing" ;;
    *)
	n=0
	for i in $x
	do
	    n=`expr $n + 1`
	done
	if test $n -ne 2
	then
	    echo "* * * ERROR: header item start has wrong length"
	else
	    n=0
	    for i in $x
	    do
		eval "y=\$start_$n"
		if test $i -ne $y
		then
		    echo "* * * ERROR: header item start[$n] is $i;" \
			 "should be $y"
		fi
		n=`expr $n + 1`
	    done
	fi
	;;
    esac

    # generic floating-point values

    gv_record_freq="500"
    gv_src_sf="8000"
    gv_start_time="0.466"

    for i in \
	record_freq   src_sf        start_time
    do
	x=`hditem -i $i out2.fea`
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

    x=`hditem -i perceptual_weights out2.fea`
    y=`echo "1.00051224 1.00148221 1.00308900 1.00554334 1.00918270" \
	"1.01454247 1.02248820 1.03447494 1.05309536 1.08335560" \
	"1.13603451 1.23807577 1.47220992 2.15881055 3.68948410"`
    case "$x" in
    '') echo "* * * ERROR: header item perceptual_weights is missing" ;;
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
	    echo "* * * ERROR: header item perceptual_weights is:"
	    echo "$x"
	    echo "should be:"
	    echo "$y"
	fi
    esac

## --- Data (2)

if feaop -f BSD -f BSD \
	 -f BSD_dif -O SUB out2.fea ref2.fea - \
   | feafunc -fBSD_dif -f- -FABS - dif2.fea
then
    rm -f tmp.fea
    `get_esps_base`/bin/select -o tmp.fea \
	    -q'max(BSD_dif) > 1.0e-5' dif2.fea 2>/dev/null 1>&2
    n=`hditem -i ndrec tmp.fea`

    if test "$n" -ne 0
    then
	echo "* * * ERROR: BSD disagrees with ref in $n records"
    fi
else
    echo "* * * ERROR: couldn't form BSD difference " \
	 "between out2.fea and ref2.fea"
fi

## RUN PROGRAM (3) --- same as (2), but use -a (and no output filename)

echo 'Running command (3):'
echo '    ./bs_dist -a -e81000 -mBSD -r2:8 -r3: \
            tstA.bark tstB.bark >log3.txt 2>err3.txt'
./bs_dist -a -e81000 -mBSD -r2:8 -r3: \
	tstA.bark tstB.bark >log3.txt 2>err3.txt
grep -v 'tags mismatch' err3.txt

## CHECK OUTPUT (3)

## --- STDOUT OUTPUT (3)

echo '... checking stdout output'

x=`cat log3.txt`
pat="0.0437978823269627804894"

case "$x" in
'') echo "* * * ERROR: no output to stdout";;
*)
    case `echo	"scale=20;" \
		"if (($x) < 0.9999 * ($pat)) \"NOT\";" \
		"if (($pat) < 0.9999 * ($x)) \"NOT\";" \
		"\"OK\"" | bc`
    in
    OK) : ;;
    *)  echo "* * * ERROR: output distortion is $x; should be $pat" ;;
    esac
;;
esac

## RUN PROGRAM (4) --- parameter file (non-default weights)

## --- MAKE PARAM FILE (4)

echo 'Making param file testparam'

cat > testparam <<aArDvArK
int	start = {2, 3};
int	nan = 7;
string  distortion_type = "BSD";
float	threshold = 8.1e4;
float	perceptual_weights = {
    1.000607550366242951819855,
    1.001758006620397924595494,
    1.003663779788576426690981,
    1.006574805820023141166304,
    1.010891344359015169714892,
    1.017248419079103983758377,
    1.026672637140569448031075,
    1.040889779505518395102462,
    1.062974940421044839783725,
    1.098865783019753666090255,
    1.161346779304804727821266,
    1.282375112927449676974550,
    1.560075177063605713651781,
    2.374433256110570872003378,
    4.189922977476159962080278};
aArDvArK

echo 'Running command (4):'
echo '    ./bs_dist -A -P testparam \
            tstA.bark tstB.bark out4.fea >log4.txt 2>err4.txt'
./bs_dist -A -P testparam \
	tstA.bark tstB.bark out4.fea >log4.txt 2>err4.txt
grep -v 'tags mismatch' err4.txt

## CHECK OUTPUT (4)

## --- STDOUT OUTPUT (4)

echo '... checking stdout output'

x=`cat log4.txt`
pat="0.0440990096095552297778"

case "$x" in
'') echo "* * * ERROR: no output to stdout";;
*)
    case `echo	"scale=20;" \
		"if (($x) < 0.9999 * ($pat)) \"NOT\";" \
		"if (($pat) < 0.9999 * ($x)) \"NOT\";" \
		"\"OK\"" | bc`
    in
    OK) : ;;
    *)  echo "* * * ERROR: output distortion is $x; should be $pat" ;;
    esac
;;
esac

## --- FEA OUTPUT (4)

echo '... making comparison file ref4.fea'

esig2fea - ref4.fea <<aArDvArK
Esignal
   0.0B
  ASCII
     48
  14446
     -1
Tag: LONG <r>
BSD: FLOAT <r>
nan: LONG 1 <g> [0]            7
perceptual_weights: FLOAT 15 <g> 
  [0]       1.000607550    1.001758007    1.003663780    1.006574806
  [4]       1.010891344    1.017248419    1.026672637    1.040889780
  [8]       1.062974940    1.098865783    1.161346779    1.282375113
  [12]       1.560075177    2.374433256    4.189922977
src_sf: DOUBLE 1 <g> [0]        8000.000000000000
distortion_type: SHORT 1 <g> [0]       1
distortion_type.enumStrings: CHAR 3 5 <g> 
  [0][0]  "NONE\0"
  [1][0]  "BSD\0\0"
  [2][0]  "MBSD\0"
start: LONG 2 <g> 
  [0]            2           3
record_freq: DOUBLE 1 <g> [0]        500.0000000000000
threshold: DOUBLE 1 <g> [0]        81000.00000000000
start_time: DOUBLE 1 <g> [0]       0.4660000000000000
recordFreq: DOUBLE <g>       500.0000000000000
startTime: DOUBLE 1 <g> [0]       0.4660000000000000
[Record 0]
  [BSD]      0.129398393852935876783138
  [Tag]         3697
[Record 1]
  [BSD]      0.764496249400356633897283
  [Tag]         3713
[Record 2]
  [BSD]      0.154410894933697592945573
  [Tag]         3729
[Record 3]
  [BSD]      0.421274287454009042311889
  [Tag]         3745
[Record 4]
  [BSD]      0.169792954154190195343840
  [Tag]         3761
[Record 5]
  [BSD]      0.105436918226478077450091
  [Tag]         3777
[Record 6]
  [BSD]      0.420087694548040124689940
  [Tag]         3793
aArDvArK

## --- Check FEA output (2)

echo '... checking output'

## --- Header (4)

# tagged?

if psps -D out4.fea | grep ' not  *tagged' >/dev/null
then echo "* * * ERROR: file is not tagged; should be tagged"
fi

# generic string values

gv_distortion_type="BSD"

for i in \
    distortion_type
do
    x=`hditem -i $i out4.fea`
    eval "pat=\$gv_$i"

    case "$x" in
    "$pat") : ;;
    '') echo "* * * ERROR: header item $i is missing" ;;
    *)  echo "* * * ERROR: header item $i is \"$x\"; should be \"$pat\"" ;;
    esac
done

# generic integer values

    gv_nan="7"
    gv_ndrec="7"

    for i in \
	nan         ndrec
    do
	x=`hditem -i $i out4.fea`
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

    x=`hditem -i start out4.fea`
    start_0=2
    start_1=3
    case "$x" in
    '') echo "* * * ERROR: header item start is missing" ;;
    *)
	n=0
	for i in $x
	do
	    n=`expr $n + 1`
	done
	if test $n -ne 2
	then
	    echo "* * * ERROR: header item start has wrong length"
	else
	    n=0
	    for i in $x
	    do
		eval "y=\$start_$n"
		if test $i -ne $y
		then
		    echo "* * * ERROR: header item start[$n] is $i;" \
			 "should be $y"
		fi
		n=`expr $n + 1`
	    done
	fi
	;;
    esac

    # generic floating-point values

    gv_record_freq="500"
    gv_src_sf="8000"
    gv_start_time="0.466"

    for i in \
	record_freq   src_sf        start_time
    do
	x=`hditem -i $i out4.fea`
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

    x=`hditem -i perceptual_weights out4.fea`
    y=`echo "1.000607550366242951819855 1.001758006620397924595494" \
	"1.003663779788576426690981 1.006574805820023141166304" \
	"1.010891344359015169714892 1.017248419079103983758377" \
	"1.026672637140569448031075 1.040889779505518395102462" \
	"1.062974940421044839783725 1.098865783019753666090255" \
	"1.161346779304804727821266 1.282375112927449676974550" \
	"1.560075177063605713651781 2.374433256110570872003378" \
	"4.189922977476159962080278"`
    case "$x" in
    '') echo "* * * ERROR: header item perceptual_weights is missing" ;;
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
	    echo "* * * ERROR: header item perceptual_weights is:"
	    echo "$x"
	    echo "should be:"
	    echo "$y"
	fi
    esac

## --- Data (4)

if feaop -f BSD -f BSD \
	 -f BSD_dif -O SUB out4.fea ref4.fea - \
   | feafunc -fBSD_dif -f- -FABS - dif4.fea
then
    rm -f tmp.fea
    `get_esps_base`/bin/select -o tmp.fea \
	    -q'max(BSD_dif) > 1.0e-5' dif4.fea 2>/dev/null 1>&2
    n=`hditem -i ndrec tmp.fea`

    if test "$n" -ne 0
    then
	echo "* * * ERROR: BSD disagrees with ref in $n records"
    fi
else
    echo "* * * ERROR: couldn't form BSD difference " \
	 "between out4.fea and ref4.fea"
fi



## RUN PROGRAM (5) --- MBSD
