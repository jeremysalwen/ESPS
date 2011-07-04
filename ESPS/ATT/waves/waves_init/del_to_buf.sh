#!/bin/sh
# @(#)del_to_buf.sh	1.1 7/6/93
#set -x
send_xwaves set outputname wcutbuffer 
send_xwaves $1 op file $2 op save segment in file
send_xwaves $1 op file $2 op delete segment

