/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
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
 * Written by:  Jim Snyder & David Talkin
 * Revised by:  Alan Parker,  David Talkin
 *
 * Brief description: play routines for Surfboard with Ariel module
 *
 */

static char *sccs_id = "@(#)nplay.c	1.7 10/8/96 ERL";

#include <Objects.h>
#include <sys/ioctl.h>
#include <dsp32c.h>
#include <vme32c.h>
#include <signal.h>

#define HOST_RESPONSE   (1234)
#define	STARTUP_CODE	(1)
#define	PARAM_CODE	(2)
#define	DATA_CODE	(3)
#define DSP_COMPLETE    (7)

#ifdef DEC_ALPHA
#define DS3100
#endif


/* At initialization 32c tells host where a buffer can be put and how
much space is available.  Host sends initial batch of data to 32c
DRAM; tells 32c how much was sent; and then starts the main 32c output
process.  32c sends an interrupt to host when it starts using the most
recently sent buffer.  At this time 32c tells host where data can be
put and how much.  Host decides how much to actually output in the
next chunk; copies data to DRAM; and notifies 32c.  */

/* 32c will use DRAM as a large circular buffer for sampled data.  It
sets a sample count and base address and issues an interrupt to the
host when some data is ready.  Host service routine writes data to
disk or array, then sends a done flag to the 32c.  32c never changes
base and count until after host acknowledge, but if buffer overrun
occurs, a distinguished flag will be set. */

/* Here is the parameter exchange block in shared DRAM: */
typedef struct header {
  int count,			/* size of buff. sent from host */
       base,			/* byte offset to host buff re DPM start */
       oops,			/* flag (from DSP): loss of real time */
       host_done,		/* set by host after buffer sent */
       chunk;			/* Minimum size of transfer buffer. */
} Head;

int channel=1;

int     surf_maxsamps, surf_sent=0, surf_error_at;

static char *dsp_program[] = { "play32", "a2d12", "a2d12"};
static int shifts;
extern int da_location, debug_level;
extern char *DSP_io, *DSP_SharedMemory;
static Head *header = NULL;
static int  dpm_size = 0x100000, 
     sizeof_sample = 2*sizeof(short), 
                            /* size of each (possibly stereo) sample on host */
     sizeof_dsp_sample = 2*sizeof(short),
                     /* size of each sample (always stereo) exchanged with DSP*/
     infd = -1, amax, amin;
extern int dsp32c[];
static short *inbuff=NULL;
static struct ioctlio dcmd;
static double frequency;
static void *inhead=NULL;
static FILE *instream=NULL;

extern int da_done;

/*************************************************************************/
static scale(data, nsamps, channel, shift)
     register int nsamps, channel, shift;
     short *data;
{
  register short *to, *from;

  to = data + nsamps*sizeof(short) - 1;
  from = (channel)? data + nsamps -1 : to;
  if(channel == 3)
    for( ;nsamps--; ) {
      *to-- = *from << shift;
      *to-- = *from-- << shift;
    }
  if(channel == 2)
    for( ;nsamps--; ) {
      *to-- = *from-- << shift;
      *to-- = 0;
    }
  if(channel == 1)
    for( ;nsamps--; ) {
      *to-- = 0;
      *to-- = *from-- << shift;
    }
  if(!channel)
    for( ;nsamps--; ) {
      *to-- = *from-- << shift;
      *to-- = *from-- << shift;
    }
}

/*************************************************************************/
surf_handle(sig)
     int sig;
{
#if defined(DS3100) || defined(APOLLO_68K) || defined(hpux) || defined(IBM_RS6000) || defined(SG) || defined(M5600) || defined(SUN4) || defined(LINUX_OR_MAC)
  return;
#else
  static int startup=0;
  int sent_here = 0;
  if(!sig) startup = 0;
  else
    if(!(dcgets2(0, DC_PIO_PCRL) & 0x40))
      return(FALSE);
  if(sig && (startup > 1) && header->oops) {	/* loss of real time? */
    surf_error_at = surf_sent;
    fprintf(stderr,"Lost real time operation near sample %d.\n", surf_sent);
    goto da_quit;
  }

  if((surf_sent + header->count) > surf_maxsamps)
    header->count = surf_maxsamps - surf_sent;
  if(header->count > header->chunk)
    header->count = header->chunk;

  if(instream || (infd >= 0)) {		/* samples from unix file? */
    int to_read, nready, nread, pinx;
    short *start;

    if((to_read = (dpm_size - header->base)/((channel)? 2 : 1)) >
       (nready = header->count*sizeof_sample))
      to_read = nready;
    start =  (short*)(DSP_SharedMemory + header->base);
    if((nread = read_any_file(start, sizeof_sample, to_read/sizeof_sample, instream,
        inhead, infd)*sizeof_sample) != to_read) {
      if(debug_level)
	fprintf(stderr,"Read1 error at %d(%d:%d)\n",surf_sent,to_read,nread);
      surf_error_at = surf_sent;
      header->count = (nread>0)? nread/sizeof_sample : 0;
      goto da_quit;
    }
    scale(start,nread/sizeof_sample,channel,shifts);
    sent_here += nread/sizeof_sample;
    surf_sent += nread/sizeof_sample;
    da_location += nread/sizeof_sample;
    if(nread < nready) {
      to_read = nready - nread;
      start = (short*)(DSP_SharedMemory+sizeof(Head));
      if((nread = read_any_file(start, sizeof_sample,
				to_read/sizeof_sample, instream,
				inhead, infd)*sizeof_sample)
	 != to_read) {
	if(debug_level)
	  fprintf(stderr,"Read2 error at %d(%d:%d)\n",surf_sent,to_read,nread);
	surf_error_at = surf_sent;
	header->count = sent_here + ((nread>0)? (nread/sizeof_sample) : 0);
	goto da_quit;
      }
      scale(start,nread/sizeof_sample,channel,shifts);
      sent_here += nread/sizeof_sample;
      surf_sent += nread/sizeof_sample;
      da_location += nread/sizeof_sample;
    }
  } else {			/* must come from a memory buffer */
    register int first_copy,	/* # stereo samples copyable in first pass */
    to_copy,			/* total # of stereo samples to copy */
    i, nshifts=shifts;
    register short *from, *to, smax, smin;
    int poff;
    
    poff = (channel)? channel : 0;
    if((first_copy = (dpm_size - header->base)/sizeof_dsp_sample) >
       (to_copy = header->count))
      first_copy = to_copy;
    if(channel)	{		/* mono */
      i=first_copy;
      to = (short*)(DSP_SharedMemory+header->base);
      from = inbuff;
      switch(poff) {
      case 3:
	for(; i--; ) {
	  *to++ = (*from)<<nshifts;
	  *to++ = (*from++)<<nshifts;
	}
	break;
      case 2:
	for(; i--; ) {
	  *to++ = 0;
	  *to++ = (*from++)<<nshifts;
	}
	break;
      case 1:
      default:
	for(; i--; ) {
	  *to++ = (*from++)<<nshifts;
	  *to++ = 0;
	}
	break;
      }
      if(to_copy > first_copy) {
	i = to_copy-first_copy;
	to = (short*)(DSP_SharedMemory+sizeof(Head));
	switch(poff) {
	case 3:
	  for(; i--; ) {
	    *to++ = (*from)<<nshifts;
	    *to++ = (*from++)<<nshifts;
	  }
	  break;
	case 2:
	  for(; i--; ) {
	    *to++ = 0;
	    *to++ = (*from++)<<nshifts;
	  }
	  break;
	case 1:
	default:
	  for(; i--; ) {
	    *to++ = (*from++)<<nshifts;
	    *to++ = 0;
	  }
	  break;
	}
      }
    } else {			/* stereo */
      for(i=first_copy,	to = (short*)(DSP_SharedMemory+header->base),
	  from = inbuff; i--; ) {
	*to++ = (*from++)<<nshifts;
	*to++ = (*from++)<<nshifts;
      }
      if(to_copy > first_copy)
	for(i = to_copy-first_copy,
	    to = (short*)(DSP_SharedMemory+sizeof(Head)); i-- ; ) {
	  *to++ = (*from++)<<nshifts;
	  *to++ = (*from++)<<nshifts;
	}
    }
    sent_here += header->count;
    surf_sent += header->count;
    da_location += header->count;
    inbuff += to_copy;
  }
  header->host_done = HOST_RESPONSE;
  startup++;

  dcgets2(0,DC_PIO_PIRL);	/* clear the interrupt flag */
  dcgets2(0,DC_PIO_PIRH);

  if(header->count <= 0) { /* wait for A/D completion of last buffer */
    int waiting = 1000;
da_quit:
    dcgets2(0,DC_PIO_PDRL);	/* let 32c know interaction is done */
    dcgets2(0,DC_PIO_PDRH);
    if(debug_level)
      fprintf(stderr,"Buffer drain %d %d\n",header->count,startup);
    sent_here = 0;
    while(waiting-- && (sent_here != DSP_COMPLETE)) {
      usleep(10000);
      header->count = 0;
      sent_here = dcgets2(0,DC_PIO_PDRL);
      dcgets2(0,DC_PIO_PDRH);
    }
    if(debug_level)
      fprintf(stderr,"Calling surf_completion() %d\n",sent_here);
    surf_completion();
    return(TRUE);
  }

  dcgets2(0,DC_PIO_PDRL);	/* let 32c know data was written */
  dcgets2(0,DC_PIO_PDRH);
  return(TRUE);
#endif
}

/*************************************************************************/
dadc_32C()
{return(-1);}

/*************************************************************************/
surf_completion()
{
#if defined(DS3100) || defined(APOLLO_68K) || defined(hpux) || defined(IBM_RS6000) || defined(SG) || defined(M5600) || defined(SUN4) || defined(LINUX_OR_MAC)
  return;
#else
  int chip;

  if(debug_level)
    fprintf(stderr,"Finishing SURF operations(%d)\n",da_done);
  
  if(!da_done) {
    da_done = TRUE;
    /* Turn off the chips. */
    ioctl(dsp32c[1], DC_STOP);
    ioctl(dsp32c[0], DC_STOP);
    ioctl(dsp32c[2], DC_STOP);

    /* Restore the original signal handlers. */
    clear_dac_callbacks();

    /* close the devices */
    close_dsp32cs();
    close_io_mem();
  }
#endif
}

/*************************************************************************/
stop_32c_vme_da(sig)
     int sig;
{
  surf_error_at = surf_sent;
  surf_completion();
}

/*************************************************************************/
dacmaster_32C(inputfile, inbuffer, sig_size, freq, sigmax, stream, eheader)
     int inputfile, sig_size, sigmax;
     short *inbuffer;
     double *freq;
     FILE *stream;
     void *eheader;
{
#if defined(DS3100) || defined(APOLLO_68K) || defined(hpux) || defined(IBM_RS6000) || defined(SG) || defined(M5600) || defined(SUN4) || defined(LINUX_OR_MAC)
  return(FALSE);
#else
  int chip;

  /* Initialize all host variables. */
  surf_sent = 0;
  surf_error_at = -1;
  shifts = 0;
  if(!sigmax) sigmax = 32767;
  if(sigmax < 16384)
    while((sigmax << shifts) < 16384) shifts++;

  if(channel)
    sizeof_sample = sizeof(short);
  else
    sizeof_sample = sizeof(int);
  if((surf_maxsamps = sig_size) <= 0) {
    fprintf(stderr,"Bogus sample count sent to dac(%d)\n",sig_size);
    return(FALSE);
  }

  if(inputfile >= 0 || stream) {	  /* input from either a file OR a memory buffer */
    infd = inputfile;
    instream = stream;
    inhead = eheader;
    inbuff = NULL;
  } else
    if(inbuffer) {
      infd = -1;
      inbuff = inbuffer;
      instream = NULL;
      inhead = NULL;
    } else {
      fprintf(stderr,"Bogus input file and buffer specified to dac()\n");
      return(FALSE);
    }
  
  /* Initialize the DSP's */
    init_dsp32cs();
    
  /* Set the sample rate. */
  if(set_sam_rate(freq,"a")) {
    frequency = *freq;
    /* Setup the signal handlers. */
    set_dac_callbacks(10000);

    /* Load the DSP programs. */
    {
      char *path;
      for(chip=0; chip < 3; chip++ ){
        path = FIND_SURF_BIN(NULL,dsp_program[chip]);
	if(path && dcld(dsp32c[chip], path)) {
	  fprintf(stderr,"Problems loading %s to DSP %d\n",path,chip);
	  return(FALSE);
	}
	free(path);
      }
    }

    /* Map in the DRAM and register file. */
    if(!(open_32c_io("/dev/dc0io") && open_32c_sm("/dev/dc0mem"))) {
      fprintf(stderr,"Problems opening I/O or mem in dac_32C()\n");
      surf_completion();
      return(FALSE);
    }
   
    header = (Head*)DSP_SharedMemory;

    /* Start the DSP's. */
    dcrun(dsp32c[2]);
    dcrun(dsp32c[0]);
    /* Start the input chip last.  (Not actually used during D/A only.) */
    dcrun(dsp32c[1]);

    /* The maximum chunk size is determined by sample frequency. */
    header->count = header->chunk = 0.5 * *freq;     /* 500ms for now... */
    header->host_done = 0;
    header->oops = 0;
    header->base =  sizeof(Head);

    surf_handle(0);	/* Send first buffer to dsp and start "interrupts". */

    /* This starts the actual A/D process on the 32c. */
    if(dsp_wait(0, 1000, STARTUP_CODE) < 1000) 
      da_done = FALSE;
    else {
      surf_completion();
      fprintf(stderr,"32C failed to respond in dac()\n");
      surf_error_at = 0;
    }
    return(TRUE);
  } else
    fprintf(stderr,"Failure while setting sample rate in dac()\n");
  return(FALSE);
#endif
}

