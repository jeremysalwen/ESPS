#!/bin/sh

send_xwaves attach function xspectrum
send_xwaves add_op name setXs op \#send function xspectrum op xspectrum make name _name file _file time _cursor_time
send_xwaves key_map key x op setXs
send_xwaves add_op name setZs op \#send function xspectrum op xspectrum make name _name file _file rstart _l_marker_time rend _r_marker_time
send_xwaves key_map key z op setZs
send_xwaves $1 setXs


