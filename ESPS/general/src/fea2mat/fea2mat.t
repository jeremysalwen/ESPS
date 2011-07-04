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
# @(#)fea2mat.t	1.1	7/12/91	ERL
# 
# Written by:  Bill Byrne
# Checked by:
# Revised by:
# 
# Brief description: test matlab conversion programs fea2mat and mat2fea
# 
echo "Testing matlab conversion programs."

rm -f test.fea test2.fea test.mat asc

echo "10 20 30 40" > asc
echo "10 20 30 40" >> asc
echo "100 200 300 400" >> asc
echo "1 2 3 4" >> asc

echo "Testing type double_cplx."
echo "addfea -c "aa" -f testfield -t double_cplx -s 2 asc test.fea"
addfea -c "aa" -f testfield -t double_cplx -s 2 asc test.fea

echo "fea2mat -f testfield test.fea test.mat"
fea2mat -x2 -f testfield test.fea test.mat

echo "mat2fea -f testfield test.mat test2.fea"
mat2fea -x2 -f testfield test.mat test2.fea

echo "pspsdiff test.fea test2.fea ... differences should be small"
pspsdiff -H test.fea test2.fea

echo "Testing type float."

rm -f test.fea test2.fea test.mat 

echo "addfea -c "aa" -f testfield -t float -s 2 asc test.fea"
addfea -c "aa" -f testfield -t double_cplx -s 2 asc test.fea

echo "fea2mat -f testfield test.fea test.mat"
fea2mat -x2 -f testfield test.fea test.mat

echo "mat2fea -f testfield test.mat test2.fea"
mat2fea -x2 -f testfield test.mat test2.fea

echo "pspsdiff test.fea test2.fea ... differences should be small"
pspsdiff -H test.fea test2.fea

rm -f test.fea test2.fea test.mat 

echo "Testing -r option."

rm -f test.fea test2.fea test.mat 

echo "addfea -c "aa" -f testfield -t float -s 2 asc test.fea"
addfea -c "aa" -f testfield -t double_cplx -s 2 asc test.fea

echo "fea2mat -r2:3 -f testfield test.fea test.mat"
fea2mat -r2:3 -x2 -f testfield test.fea test.mat

echo "mat2fea -f testfield test.mat test2.fea"
mat2fea -x2 -f testfield test.mat test2.fea

echo "echo should see ..."
echo "10 20 30 40"
echo "100 200 300 400"
echo "pplain -f testfield test2.fea"
pplain -f testfield test2.fea

rm -f test.fea test2.fea test.mat 

echo "Testing -m option."

rm -f test.fea test2.fea test.mat 

echo "addfea -c "aa" -f testfield -t float -s 2 asc test.fea"
addfea -c "aa" -f testfield -t double_cplx -s 2 asc test.fea

echo "fea2mat -f testfield test.fea test.mat"
fea2mat -x2 -f testfield test.fea test.mat

echo "mat2fea -f testfield -m test.mat test2.fea"
mat2fea -x2 -f testfield test.mat test2.fea

echo "fea2mat -f testfield test.fea test.mat"
fea2mat -x2 -f testfield test.fea test.mat

echo "mat2fea -f testfield -m test.mat test3.fea"
mat2fea -x2 -f testfield test.mat test3.fea

echo "pspsdiff test.fea test2.fea ... differences should be small"
pspsdiff -H test2.fea test3.fea

rm -f test.fea test2.fea test3.fea test.mat asc

