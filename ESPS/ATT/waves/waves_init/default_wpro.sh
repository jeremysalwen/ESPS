#!/bin/sh
# If there's a current ~/.wave_pro, we move it to ~/.wave_pro_save provided 
# that the latter doesn't exist already.  We then copy the system .wave_pro. 
# 
# @(#)default_wpro.sh	1.2 6/29/93

if test -f $HOME/.wave_pro
 then 
   if test -f $HOME/.wave_pro_save
     then
       mv $HOME/.wave_pro $HOME/.wave_pro_save
   fi
fi

cp `get_esps_base`/lib/waves/.wave_pro $HOME/.wave_pro

