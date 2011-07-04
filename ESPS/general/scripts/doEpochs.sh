#!/bin/sh

case $# in
    2)
input=$1
output=$2
tmp=$HOME/waves/tmp/epochsTmp
rm -f ${tmp}*
# Determine the sample rate of the original speech file.
sf=`hditem -i record_freq $input`
#
# Establish the window size and frame step for periodic analyses.
size=.02
step=.005
#
# Get analysis step size and window length in samples.
ssize=`echo $sf $size \* p q | dc`
sstep=`echo $sf $step \* p q | dc`
#
# Standard rule-of-thumb computation for LPC order.
order=`echo $sf 1000 / 2 + p q | dc`
#
# Optionally, Hilbert transform the signal to improve the phase linearity.
# (The copysd-e2sphere is to make certain that the input to hilbert is a NIST file.)
copysd $input - | e2sphere - $tmp.tmp
hilbert $tmp.tmp  $tmp.nist
#
# Compute reflection coefficients using a standard set of parameters
refcof -z -r1:1000000 -e.97 -x0 -wHANNING -l$ssize -S$sstep \
       -o$order $tmp.nist $tmp.rc
#
# Get a high-resolution estimate of F0 and a reasonably accurate
#  voicing-state estimate.
get_f0 -i $step $tmp.nist $tmp.f0
#
# Compute the LPC residual (approximates the glottal flow derivative).
get_resid -a 1 -i 0.0 $tmp.nist $tmp.rc $tmp.res
#
# Blank out the residual signal in the unvoiced regions.
mask $tmp.f0 $tmp.res $tmp.resm
#
# Find the points of glottal closure in the voiced regions.
epochs  -o $output.lab $tmp.resm $output
;;
    *)
echo ' '
echo Usage: $0 input output
echo where \'input\' is a RIFF, NIST-Sphere or ESPS speech waveform file.
echo  \'output\' will be a file of pulses at the estimated GCIs.
echo ' '
echo A file called output.lab will also be created that contains the
echo estimated times of the GCIs in xwaves xlabel format.
;;
esac
