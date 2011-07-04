#attach.com  @(#)attach.com	1.4 9/6/90	
#shows off spectrum and label attachments
set scrollbar_height 10
#waves make name s1 file files/e_attach.sd start 0 duration 1.5 loc_x 10 loc_y 420 height 280 width 1050
sleep seconds .2
attach function esps/bin/xspectrum
sleep seconds 1
waves shell lock_play audio/e_attach1.sd audio/e_attach2.sd &
make name s2 file files/e_attach.sd loc_y 350 loc_x 50 height 240 width 1000
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.15
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.55
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.2
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.6
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.25
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.65
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.3
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.7
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.35
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.75
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.4
sleep seconds 1
send make name s2 file files/e_attach.sd time 2.8
sleep seconds 1
shell wait_for 0
detach
kill 
sleep seconds 2

set scrollbar_height 15
make name s1 file files/e_attach.sd start 0 duration 4.1 loc_x 10 loc_y 450 height 200 width 1100
sleep seconds 1
attach function esps/bin/xlabel
sleep seconds 1
shell lock_play  audio/e_attach3.sd&
sleep seconds 1
send make name s1 file files/e_attach.lab
send make name s1 file files/e_attach.lab2
sleep seconds 3
s1 bracket start 1 end 2.5
sleep seconds 3
s1 bracket start 1.5 end 2.0
sleep seconds 1
shell wait_for 2
detach
kill 
sleep seconds 2
return





