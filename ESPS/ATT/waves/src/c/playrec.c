/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* playrec.c */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)playrec.c	1.3	9/26/95	ATT/ERL";

/*	play and record subroutine for DSP32/VME board
 *
 *	uses output buffering in PGA DSP32 ( 8 Buffers with 1024 samples )
 *
 *	dac(file,buffer,size,freq,amp)
 *
 *	file:	open file descriptor or zero (no file)
 *	buffer:	pointer to sample buffer (shorts) or zero
 *	size:	number of samples to play (dac will quit on eof)
 *	*freq:	double precision sampling freq.; will be converted to integer
                to access '(freq)to' and 'to(freq)'; exact freq. used is returned
 *		sample rate converter programs
 *      amp:    the absolute maximum in the signal for D/A
 *
 *	keyboard intr signal will kill playing
 *
 *
 *	dadc(file,buffer,inputfile,inputbuffer,size,freq,amax,amin,mode)
 *
 *	this entry will play and also record (e.g. for impulse response meas.)
 *	inputfile:	file pointer to writable file
 *	inputbuffer:	pointer to buffer for input
 *      amax, amin      return limits encountered during recording
 *                      dadc() returns the number of samples written
 *      mode		Determines play-only, play-record, or record-only
 *			(see setrate.c).
 *
 */

/* 3/30/88:mcb - `dadc' sends (up to) 8 buffers to dsp and leaves one in
 *		readiness.  `irq' then grabs the remaining data one buffer
 *		at a time.  `dadc' was fixed so that `ocnt' now reflects
 *              data SENT TO THE BOARD and *not* data read from file or
 *		buffer.  A delay of 1/3 second was added to the fab2_quit
 *		code in `irq' to prevent stopping before playing a buffer.
 */

#include <stdio.h>
#include <signal.h>
#include <dsplock.h>

#define	DSP_PCR	DMA_MODE
#define	DSP_RUN	(EMR_DEF<<16|DSP_PCR)

#define	DSPBUF	0x2000	/* dsp output buffer hardwired in dsp prog. */
#define	NBUF	1024	/* transfer size to dpr ... ditto */
#define	NBYTES	(2*NBUF)

#define  TRUE 1
#define FALSE 0

static	short*	dpr;	/* pointer to dual ported ram */
static	int	ofd;	/* file pointer for output */
static  int   have_ofd;  /* indicate data comes from a file. */
static	int	ifd;
static  void    *v32_head, /* args for read_any_file */
                *v32_stream;
static	short*	optr;	/* pointer to current buffer location */
static	short*	iptr;
	int	fab2_maxsamps,	/* total requested count */
                fab2_sent = 0,	/* number actually sent */
                fab2_error_at = -1; /* non-negative indicates error at sample */
static	int	icnt;	/* input count */
static	int	qcnt;	/* buffers in queue */
static	int	iwait;	/* void input buffers */
static	int	atime=0;	/* means something like "interrupts active" */
static  int     lastpir;	/* error from dsp */
static  int     smax, smin;	/* maintain limits if A/D performed */
static	short	obuf[NBUF];
static	short	tobuf[NBUF];
static  short   shift;	/* for scaling to max during D/A */

extern	int	debug_level;

int use_dsp32 = 0;

extern int  da_location, da_done;

stop_fab2_da() {
  fab2_completion();
}

extern int  use_dsp32;
extern int  dsp32_wait;
static int  locked = 0;		/* lock file set? */
extern short  *dspmap();

/*************************************************************************/
static 
cpy(from,to,size,bufsize)	/* copy and zero padding */
     register short *from;
     register short *to;
{
  register int cnt = size, sh = shift;

  if ( cnt > 0 ) {
    if(from)
      do { *to++ = (*from++) << sh; } while ( --cnt );
    else
      do { *to++ = 0; } while ( --cnt );
  }
  if ( size < bufsize ) {
    cnt = (size > 0)? (bufsize-size) : bufsize;
    do { *to++ = 0; } while ( --cnt );
  }
}

/*************************************************************************/
static 
cpymaxmin(from,to,size)		/* copy and zero padding and maxmin*/
     register short *from;
     register short *to;
{
  register int cnt = size;
  register int max = 0;
  register short c;

  if ( cnt ) 
    do {
      *to++ = c = *from++;
      if ( c > smax ) smax = c;
      else
	if ( c < smin ) smin = c;
    } while ( --cnt );
}

/*************************************************************************/
static 
irq(signal)
{
#ifdef HELL_FREEZES
  register int n;
  register short *p = dpr, lmax = smax, lmin = smin, stemp, nw;

				/* input data */
  if ( iwait )
    iwait--;			/* wait for first real input */
  else {
    if ( (n=icnt) > 0 ) {	/* read input data from dpr */
      if ( n > NBUF ) n = NBUF;
      if ( ifd ) {
	cpymaxmin(dpr,tobuf,n);
	nw = write(ifd,tobuf,NBYTES);
	if(debug_level)
	  fprintf(stderr,"w%d",nw);
      }
      else {
	cpy(dpr, iptr, n, n);
	iptr += n;
      }
      
      icnt -= n;
    }
  }
  if ( atime == 0 ) goto fab2_quit;

				/* now do output */
  if ( (n=fab2_maxsamps) > NBUF )
    n = NBUF;
  cpy(optr,dpr,n,NBUF);		/* load new data into dual ported ram */
  fab2_maxsamps -= n;

  da_location += n;
  fab2_sent += n;

  if(optr) optr += n;
  if ( (n=(short)dsprg(0,C_PIR)) <= 0 ) /* check buffer queue */
    if ( signal ) {
      lastpir = n;
      fab2_error_at = fab2_sent;
      goto fab2_quit;
    }
  if ( fab2_maxsamps > 0 ) qcnt = n; 
  if ( have_ofd && fab2_maxsamps > 0 ) {	/* reload buffer */
    /*     n=read(ofd,optr=obuf,NBYTES)/2; */
    n=read_any_file(optr=obuf,sizeof(short),NBYTES/sizeof(short),v32_stream,
	   v32_head,ofd);
    if ( n < NBUF ) fab2_maxsamps = n;
  }
  if ( fab2_maxsamps <= 0 )
    /* if record is active we
       will wait a little longer */
    if ( --qcnt < ( (icnt)? -3 : -2)  ) {
    fab2_quit:
      fab2_completion();
      return;
    }
  if (dsprg(0,C_SIG) == -1)  ;	/* who knows what this once did... */
#endif
}

/*************************************************************************/
static 
timeout(signal)
{
  if ( (signal == SIGINT) || (signal == SIGQUIT) )
    fprintf(stderr,"\nINTR\n");
  else {
    fprintf(stderr,"\nTIMEOUT");
    if ( lastpir )
      fprintf(stderr," pir:0x%x",lastpir&0xffff);
    fprintf(stderr,"\n");
  }
  atime = 0;
}

/*************************************************************************/
static
dadc(outfd, outbuf, inpfd, inpbuf, size, freq, amp, amax, amin, mode)
short *outbuf;
short *inpbuf;
int outfd, inpfd, size, amp, *amax, *amin, mode;
double *freq;
{
}

/*************************************************************************/
static 
adc(infd,inbuf,size,freq,amax,amin)
     short *inbuf;
     int infd, size, *amax, *amin;
     double *freq;
{
     return(dadc(0, NULL, infd, inbuf, size, freq, 16000, amax, amin, 2));
}
     
/*************************************************************************/
dacmaster_32(outfd,outbuf,size,freq,amp,strm,ehead)
     short *outbuf;
     int outfd, size, amp;
     double *freq;
     void *strm, *ehead;
{
  int amax, amin;

  v32_stream = strm;
  v32_head = ehead;
  dadc(outfd, outbuf, 0, (short*) 0, size, freq, amp, &amax, &amin, 0);
}

/*************************************************************************/
dac_32(outfd,outbuf,size,freq,amp)
short *outbuf;
int outfd, size, amp;
double *freq;
{
  int amax, amin;

  v32_stream = NULL;
  v32_head = NULL;
  dadc(outfd, outbuf, 0, (short*) 0, size, freq, amp, &amax, &amin, 0);
}
/*************************************************************************/
fab2_completion()
{
#ifdef HELL_FREEZES

  da_done = TRUE;
  atime=0;

#if defined(SUN4) && !defined(OS5)
  usleep(333333);		/* 1/3 second for buffers to clear */
#else
  fprintf(stderr,
	  "%s: This statement shouldn't be executed on non-Sun machines.\n",
	  "irq (playrec.c) DSP-dependent code");
#endif
  /* usleep not available on some systems (e.g. DecStation 3100). */

  dsprg(0, C_STOP);
  if (locked) {
      DSP_UNLOCK;
      locked = 0;
    }
  clear_fab2_dac_callbacks();
#endif
}

