#! /bin/sh
# @(#)decplay.sh	1.1 11/21/96 ERL
ESPS_EDR=off
export ESPS_EDR
copysd -d short $* /usr/tmp/dplay$$
sz=`psps -D -v /usr/tmp/dplay$$ | grep samples | grep size | awk '{print $7}'`
rate=`hditem -i record_freq /usr/tmp/dplay$$`
bhd /usr/tmp/dplay$$ - | /usr/bin/mme/audioplay -channels $sz -bitspersample 16 -encoding pcm -rate $rate -filename - >/dev/null
rm /usr/tmp/dplay$$
