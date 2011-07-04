#!/bin/csh

# @(#)zst.t	1.2	6/22/93	ERL

# create initial lab and data files
cat > test1.phn << ZAP
initial header stuff
#
 2.2 151 aaa
 2.5 151 bbb
ZAP
testsd -s 1 test1.dat
addgen -v 2.0 -t double -F -g start_time test1.dat

echo "original start time"
echo " hditem -i start_time test1.dat"
hditem -i start_time test1.dat

echo "original label file"
echo "cat test1.phn"
cat test1.phn

echo "testing..."
echo "zst -d dat -g phn test1.dat"
zst -d dat -g phn test1.dat

echo "check that new times are down by 2 seconds, but otherwise the same"
echo "new start time"
echo " hditem -i start_time test1.dat"
hditem -i start_time test1.dat

echo "new label file"
echo "cat test1.phn"
cat test1.phn

echo "check backup file"
echo "ls test1.phnBAK"
ls test1.phnBAK

echo "done"
\rm test1*
