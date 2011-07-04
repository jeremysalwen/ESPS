# curs.sny.com  	@(#)curs.sny.com	1.3 1/15/91
# command file for moving cursors through set of windows 
waves sleep seconds .2
b cursor time .1 freq 100
b cursor time .124 freq 310.667
b cursor time .154 freq 574
b cursor time .178 freq 784.667
b cursor time .208 freq 1048
b cursor time .226 freq 1206
b cursor time .25 freq 1416.67
b cursor time .274 freq 1627.33
b cursor time .298 freq 1838
b cursor time .322 freq 2048.67
b cursor time .358 freq 2364.67
b cursor time .376 freq 2522.67
b cursor time .406 freq 2786
b cursor time .43 freq 2996.67
b cursor time .46 freq 3260
b cursor time .49 freq 3523.33
b cursor time .52 freq 3786.67
b cursor time .544 freq 3997.33
b cursor time .574 freq 4260.67
b cursor time .598 freq 4471.33
b cursor time .628 freq 4734.67
b cursor time .658 freq 4998
b cursor time .682 freq 5208.67
b cursor time .724 freq 5577.33
b cursor time .754 freq 5840.67
b cursor time .772 freq 5998.67
b cursor time .796 freq 6209.33
b cursor time .808 freq 6314.67
b cursor time .826 freq 6472.67
b cursor time .85 freq 6683.33
b cursor time .874 freq 6894
b cursor time .898 freq 7104.67
b cursor time .922 freq 7315.33
b cursor time .952 freq 7578.67
return
