#!/bin/sh 
# This material contains unpublished, proprietary software of 
# Entropic Research Laboratory, Inc. Any reproduction, distribution, 
# or publication of this work must be authorized in writing by Entropic 
# Research Laboratory, Inc., and must bear the notice: 
#
#    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc. 
#                   All rights reserved"
#
# The copyright notice above does not evidence any actual or intended 
# publication of this source code.     
#
# @(#)zero_pad.tst	1.1	1/5/93	ERL
# 
# Written by:  Bill Byrne
# Checked by:
# Revised by:
# 
# Brief description: zero_pad test script
# 

USE_ESPS_COMMON=off

rm -f test.fea dummy.asc
cat > dummy.asc  <<  ZED 
0 1
2 3
4 5
6 7
ZED
addfea -f data -s 2 -t float -c"duh" dummy.asc test.fea

echo "test append"
zero_pad -l2 test.fea test.fea2
cat > dummy.asc << ZED
0 1
2 3
4 5
6 7
0 0
0 0
ZED
cat dummy.asc
echo "pplain results should look like above:"
pplain -r1: test.fea2

echo " "
echo "test prepend"
zero_pad -i -l2 test.fea test.fea2
cat > dummy.asc << ZED
0 0
0 0
0 1
2 3
4 5
6 7
ZED
cat dummy.asc
echo "pplain results should look like above:"
pplain -r1: test.fea2

echo " "
echo "test skip"
zero_pad -i -r2:3 -l2 test.fea test.fea2
cat > dummy.asc << ZED
0 0
0 0
2 3
4 5
ZED
cat dummy.asc
echo "pplain results should look like above:"
pplain -r1: test.fea2

echo " "
echo "test piping and duplicatation"
cat > dummy.asc << ZED
0 1
0 1
0 1
2 3
4 5
6 7
6 7
6 7
ZED
cat dummy.asc
zero_pad -d -l2 test.fea - | zero_pad -d -i -l2 - test.fea2
echo "pplain results should look like above:"
pplain -r1: test.fea2

echo " "
echo "testing -L option"
cat > dummy.asc << ZED
0 0
0 0
0 0
0 0
0 1
2 3
4 5
6 7
ZED
cat dummy.asc
echo "pplain results should look like above:"
addgen -t double -v 0.5 -g record_freq test.fea
zero_pad -L 2.0 test.fea test.fea2
pplain -r1: test.fea2

\rm -f dummy.asc test.fea test.fea2 
