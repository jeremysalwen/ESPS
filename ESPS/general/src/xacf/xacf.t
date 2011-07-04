#!/bin/csh
#  This material contains proprietary software of Entropic Speech, Inc.
#  Any reproduction, distribution, or publication without the prior
#  written permission of Entropic Speech, Inc. is strictly prohibited.
#  Any public distribution of copies of this work authorized in writing by
#  Entropic Speech, Inc. must bear the notice
#
#      "Copyright (c) 1990 Entropic Speech, Inc.; All rights reserved"
#
#  test script for xacf @(#)xacf.t	1.1 2/25/91 ERL
#
echo "**************************************************************" 
echo "Interactive test for xacf"
echo "  "
echo "You must be using xwindows to run this script."
echo "If you're not using xwindows, you have 5 seconds to kill this program."
sleep 5
echo "   "
echo "**************************************************************"
echo "Test 1: When the xacf window appears, click the DONE button."
xacf -P Pxacf test.res
echo "results of this diff should be insignificant"
diff -b test.res Pxacf

echo " "
echo "**************************************************************"
echo "Test 2: When the xacf window appears, please do the following:" 
echo "  "
echo "Set the following frame values..."
echo "Input Field Name: sample_data   Framing Units: seconds"
echo "Preemphasis:      0.9           Frame Length:  0.02    Start:  0.01"
echo "Window:           HAMMING       Step:          0.008   Nan:    1.00"
echo "Store Windowed Data Frame: YES  Field Name:  input_samples"
echo "  "
echo "Set the following acoustic feature parameters..."
echo "Click all the YES buttons"
echo "Power                          Field Name: power_"
echo "Zero Crossings                 Field Name: zero_crossing_"
echo "Auto Correlation               Field Name: auto_corr_"
echo "                               Order: 20"
echo "Reflection Coefficients        Field Name: refcof_"
echo "Linear Prediction Coefficients Field Name: lpc_coeffs_"
echo "Line Spectral Frequencies      Field Name: line_spec_freq_"
echo "                               Resolution: 10"
echo "Log Area Ratios                Field Name: log_area_ratio_"
echo "LPC Cepstrum                   Field Name: lpc_cepstrum_"
echo "                               Element Subrange: 1:10"
echo "                               Order: 11"
echo "                               Warping Parameter: 0.6"
echo "FFT Cepstrum                   Field Name: fft_cepstrum_"
echo "                               Order: 256"
echo "                               Element Subrange: 1:128"
echo "FFT"
echo "                               Order: 9"
echo "  "
echo "Click the DONE button when all values have been entered correctly."
xacf -P Pxacf test.res
echo "results of this diff should be insignificant"
diff test.res params.t
echo " "
echo "finished testing xacf"
\rm test.res 
