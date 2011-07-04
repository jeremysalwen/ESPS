/* LINTLIBRARY */

/* @(#)mrlintcheck.c	1.2 2/1/88 ESI */

int
mropen(p_devpn,dadev,code)
int *p_devpn; char *dadev; int code;
{ return 0;}

int
mrclk1(clkpn,i,sf,act_sf,j,clkwid,act_wid,k)
int clkpn, i, j, k;
double sf, *act_sf, clkwid, *act_wid;
{ return 0;}

int
mrclktrig(devpn,cod,clkpn)
int devpn, clkpn, cod;
{ return 0;}

int
mrdaxout(devpn,clkpn,clkpn2,channel,i,j,np,data,k)
int devpn,clkpn,clkpn2,channel,i,j,k,np;
short *data;
{ return 0;}

int
mrevwt(devpn,cod,wait)
int devpn, *cod, wait;
{ return 0;}

int
mrclosall()
{ return 0;}

int 
mradmod(pathno, range, sediff)
int pathno, range, sediff;
{ return 0;}

int mrlock(start, numbytes, byteslocked)
int start, numbytes, *byteslocked;
{ return 0;}

int mradxin(adpn, clkpn1, clkpn2, fchan, nchans, incr, gain, nframes, space)
int adpn, clkpn1, clkpn2, fchan, nchans, incr, gain, nframes;
short *space;
{ return 0;}

