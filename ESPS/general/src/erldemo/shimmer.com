#shimmer.com  	@(#)shimmer.com	1.2 6/5/90
#command file to shimmer colors 

b colormap range 114  threshold 0
waves sleep seconds .3
b colormap range 103  threshold 11
waves sleep seconds .2
b colormap range 99  threshold 15
waves sleep seconds .2
b colormap range 96  threshold 18
waves sleep seconds .2
b colormap range 91  threshold 23
waves sleep seconds .2
b colormap range 83  threshold 31
waves sleep seconds .2
b colormap range 79  threshold 35
waves sleep seconds .2
b colormap range 77  threshold 37
waves sleep seconds .2
b colormap range 74  threshold 40
waves sleep seconds .2
b colormap range 69  threshold 45
waves sleep seconds .2
b colormap range 64  threshold 50
waves sleep seconds .2
b colormap range 59  threshold 55
waves sleep seconds .2
b colormap range 56  threshold 58
waves sleep seconds .2
b colormap range 54  threshold 60
waves sleep seconds .2
b colormap range 39  threshold 65
waves sleep seconds .2
b colormap range 44  threshold 70
waves sleep seconds .2
b colormap range 39  threshold 75
waves sleep seconds .2
b colormap range 34  threshold 80
waves sleep seconds .2
b colormap range 24  threshold 90
waves sleep seconds .2
b colormap range 14  threshold 100
waves sleep seconds .2
b colormap range 1  threshold 113
waves sleep seconds .3
b colormap range 14  threshold 100
waves sleep seconds .3
b colormap range 24  threshold 90
waves sleep seconds .3
b colormap range 34  threshold 80
waves sleep seconds .3
b colormap range 39  threshold 75
waves sleep seconds .3
b colormap range 44  threshold 70
waves sleep seconds .3
b colormap range 49  threshold 65
waves sleep seconds .3
b colormap range 54  threshold 60
waves sleep seconds .3
b colormap range 56  threshold 58
waves sleep seconds .3
b colormap range 59  threshold 55
waves sleep seconds .3
b colormap range 64  threshold 50
waves sleep seconds .3
b colormap range 69  threshold 45
waves sleep seconds .3
b colormap range 74  threshold 40
waves sleep seconds .3
b colormap range 77  threshold 37
waves sleep seconds .3
b colormap range 79  threshold 35
waves sleep seconds .3
b colormap range 83  threshold 31
waves sleep seconds .3
b colormap range 87  threshold 27
waves sleep seconds .3
b colormap range 91  threshold 23
waves sleep seconds .3
b colormap range 96  threshold 18
waves sleep seconds .3
b colormap range 99  threshold 15
waves sleep seconds .3
b colormap range 103  threshold 11
waves sleep seconds .3
b colormap range 114  threshold 0
waves return

