#!/bin/sh
#
# This script assumes that save_state.WC has run.  If there's a current
# ~/.wave_pro, we move it to ~/.wave_pro_save provided that the latter 
# doesn't exist already.  We then construct a new .wave_pro that puts
# waves into the state it was when the save_state.WC ran.  This is 
# accomplished by having the globals go into .wave_pro, and having 
# a new init_file that runs the three command files left by 
# save_state.WC (one each for addops, menus, and panels).  
# 
# @(#)fix_wavepro.sh	1.5 9/9/93

#set -x

cust_init=`get_esps_base`/lib/waves/commands/cust_xw_init.WC

#for testing:
#cust_init=cust_xw_init.WC

cp $cust_init $HOME/.xw_init.WC

if test -f $HOME/.wave_pro
 then 
   if test ! -f $HOME/.wave_pro_save
     then
       mv $HOME/.wave_pro $HOME/.wave_pro_save
   fi
fi

egrep -v "commandname|init_file" $HOME/.wave_pro_new > $HOME/.wave_pro

echo "init_file $HOME/.xw_init.WC" >> $HOME/.wave_pro

rm -f $HOME/.wave_pro_new
