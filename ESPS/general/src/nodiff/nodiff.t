#!/bin/sh
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)nodiff.t	1.2	7/23/91	ERL
# 
# Written by:  Bill Byrne
# Checked by:
# Revised by:
# 
# Brief description: nodiff test script
# 
echo "Nodiff expects to find filters in ESPS_BASE/lib/filters "
echo "following should find diff1.filt, ..., diff5.filt"
echo "ls `get_esps_base`/lib/filters/diff[1-5].filt"
ls `get_esps_base`/lib/filters/diff[1-5].filt
echo " " 
cat > asc << EOD
1 -1
0  0
0  0
0  0
0  0
0  0
0  0
0  0
0  0
0  0
EOD
addfea -f field1 -t double -s 2 -c" " asc test
echo "nodiff -o5 -f field1 test test.d5"
nodiff -o5 -f field1 test test.d5
echo "Testing fifth order difference: output should be "
cat << EOD
  1  -1
 -5   5
 10 -10
-10  10
  5  -5
  0   0
  0   0
  0   0
  0   0
  0   0
EOD
echo "pplain -f field1_d5 test.d5"
pplain -f field1_d5 test.d5
echo " "
echo "Testing first order difference: output should be "
cat << EOD
1 -1   1  -1   1  -1
0  0  -5   5  -6   6
0  0  10 -10  15 -15
0  0 -10  10 -20  20
0  0   5  -5  15 -15
0  0   0   0  -5   5
0  0   0   0   0   0
0  0   0   0   0   0
0  0   0   0   0   0
0  0   0   0   0   0
EOD
echo "nodiff -f field1_d5 test.d5 test.d1"
nodiff -f field1_d5 test.d5 test.d1
pplain test.d1
echo "check that following fields are defined in test.d1"
cat << EOD
Item name: field1, type: DOUBLE, size: 2, rank: 1
Item name: field1_d5, type: DOUBLE, size: 2, rank: 1
Item name: field1_d5_d1, type: DOUBLE, size: 2, rank: 1
EOD
echo " "
echo "psps -Dv test.d1 | grep Item"
psps -Dv test.d1 | grep Item
echo " "
echo " "
echo "test feasd processing"

rm asc
cat > asc << EOD
1 -1
0  0
0  0
0  0
0  0
EOD
testsd -t double -c -a  asc test.sd
echo "testsd -t double -c -a  asc test.sd"
nodiff -o5 -f samples test.sd test.d6
echo "Testing fifth order difference: output should be "
cat << EOD
Record 1:
samples:  [1, -1]
samples_d5:  [1, -1]

Record 2:
samples:  [0, 0]
samples_d5:  [-5, 5]

Record 3:
samples:  [0, 0]
samples_d5:  [10, -10]

Record 4:
samples:  [0, 0]
samples_d5:  [-10, 10]

Record 5:
samples:  [0, 0]
samples_d5:  [5, -5]
EOD
echo " "
echo "psps test.d6"
psps test.d6
rm asc test test.d5 test.d1 test.sd test.d6
