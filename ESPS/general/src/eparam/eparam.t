#! /bin/sh
# @(#)eparam.t	1.2 6/14/91 ESI
# test script for eparam
echo "This test script runs fft via eparam"
echo  "  %testsd -Tsine test.sd"
testsd -Tsine test.sd
echo "  %eparam fft test.sd test.spec"
eparam fft test.sd test.spec
echo "  %psps -D test.spec"
psps -Dl test.spec

