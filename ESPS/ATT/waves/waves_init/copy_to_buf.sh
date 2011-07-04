#!/bin/sh
# @(#)copy_to_buf.sh	1.1 7/6/93 ERL
# Configure globals
send_xwaves set outputname wcutbuffer 
send_xwaves $1 op file $2 op save segment in file

