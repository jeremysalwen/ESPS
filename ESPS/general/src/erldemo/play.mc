# play.mc @(#)play.mc	1.2 12/4/90 ERL	
# play for Concurrent using EF12 
# Adjust the clock and D/A path for other systems and configurations

mcplay  -D/dev/dacp0/daf0 -C/dev/dacp0/efclk0 -b2 $*
