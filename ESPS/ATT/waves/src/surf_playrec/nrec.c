/* nrec.c */
/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
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

static char *sccs_id = "@(#)nrec.c	1.3	11/8/91	ATT/ESI/ERL";


/* Replacement for the "record_data.c" module.  This new one is tailor
made for the SURF board with an Ariel A/D-D/A interface. */

#include <Objects.h>
#include <sys/ioctl.h>
#include <dsp32c.h>
#include <vme32c.h>
#include <signal.h>
#include <esps/epaths.h>

#define HOST_RESPONSE   (1234)
#define	STARTUP_CODE	(1)
#define	PARAM_CODE	(2)
#define	DATA_CODE	(3)

/* 32c will use DRAM as a large circular buffer for sampled data.  It
sets a sample count and base address and issues an interrupt to the
host when some data is ready.  Host service routine writes data to
disk or array, then sends a done flag to the 32c.  32c never changes
base and count until after host acknowledge, but if buffer overrun
occurs, a distinguished flag will be set. */

/* Here is the parameter exchange block in shared DRAM: */
typedef struct header {
  long count,			/* size of buff. sent to host */
       base,			/* byte offset to host buff re DPM start */
       oops,			/* flag (from DSP): loss of real time */
       host_done,		/* set by host after buffer grabbed */
       chunk;			/* Minimum size of transfer buffer. */
} Head;

char *program[] = { "record32", "a2d12", "a2d12"};
extern int dsp32c_io, dsp32c_sm, dsp32c[];
extern char *DSP_io, *DSP_SharedMemory;
Head *header = NULL;
int  dpm_size = 0x100000, channel=0;
     sizeof_sample = 4, outfd = -1, maxsamps, collected, error_at,
     amax, amin;
short *outbuff=NULL;
static struct ioctlio dcmd;

/*************************************************************************/
/* Not only does this detect the bounding values of the buffer at
"start," it also compresses two-channel data into one channel when
channel != 0. */
get_signal_max(start, nshorts, amax, amin, channel)
     int *amax, *amin, channel;
     register int nshorts;
     register short *start;
{
  register short t, smax = *amax, smin = *amin;

  if(!channel) {
    for( ;nshorts-- ; )
      if((t = *start++) > smax) smax = t;
      else
	if(t < smin) smin = t;
  } else {
    register short *from;

    for(from = start + channel -1; nshorts-- ; from += 2)
      if((*start++ = t = *from) > smax) smax = t;
      else
	if(t < smin) smin = t;
  }
  *amax = smax;
  *amin = smin;
}


/*************************************************************************/
stop_analog(sig)
     int sig;
{
  maxsamps = collected;		/* adc() will do the rest... */

  if (ioctl(dsp32c[0], DC_DISPIF, &dcmd) != 0) { /* disable interrupt */
    printf("DC_DISPIF rejected in stop_analog()\n");
    error_at = collected;
    return;
  }
}

/*************************************************************************/
dsp_handle(sig)
     int sig;
{
  int bmax = -32760, bmin = 32760;

  fflush(stdout);
  if(header->oops) {		/* loss of real time? */
    error_at = collected;
    printf("Error signal from DSP!(%d)\n",header->oops);
    return;
  }

  if(header->host_done == HOST_RESPONSE) { /* should be changed by 32c */
    printf("Unexplained interrupt at %d\n", collected);
    error_at = collected;
    return;
  }

  if((collected + header->count) > maxsamps)
    header->count = maxsamps - collected;
  if(header->count < 0)
    header->count = 0;

  if(outfd >= 0) {		/* samples to unix file? */
    int to_write, nready, written, pinx;
    short *start;

    if((to_write = (dpm_size - header->base)/((channel)? 2 : 1)) >
       (nready = header->count*sizeof_sample))
      to_write = nready;
    start =  (short*)(DSP_SharedMemory + header->base);
    get_signal_max(start, to_write/sizeof(short), &bmax, &bmin, channel);
    if((written = write(outfd, start, to_write))
       != to_write) {
      printf("Write error at %d\n",collected);
      error_at = collected;
      return;
    }
    collected += written/sizeof_sample;
    if(written < nready) {
      to_write = nready - written;
      start = (short*)(DSP_SharedMemory+sizeof(Head));
      get_signal_max(start, to_write/sizeof(short), &bmax, &bmin, channel);
      if((written = write(outfd, start, to_write))
	 != to_write) {
	printf("Write error at %d\n",collected);
	error_at = collected;
	return;
      }
      collected += written/sizeof_sample;
    }
  } else {			/* must go into a memory buffer */
    register int to_copy, nshorts, i, t, pinc;
    register short *from, *to, smax, smin;
    int poff;

    fflush(stdout);
    pinc = (channel)? 2 : 1;
    poff = (channel)? channel - 1 : 0;
    if((to_copy = (dpm_size - header->base)/(sizeof(short)*pinc)) >
       (nshorts = header->count*sizeof_sample/sizeof(short)))
      to_copy = nshorts;
    for(i=to_copy, from = (short*)(DSP_SharedMemory+header->base)+poff,
	smax = bmax, smin = bmin,
	to = outbuff; i--; from += pinc) {
      *to++ = t = *from;
      if(t > smax) smax = t;
      else
	if(t < smin) smin = t;
    }
    if(nshorts > to_copy)
      for(i = nshorts-to_copy,
	  from = (short*)(DSP_SharedMemory+sizeof(Head))+poff; i-- ;
	  from += pinc) {
	*to++ = t = *from;
	if(t > smax) smax = t;
	else
	  if(t < smin) smin = t;
      }
    bmax = smax;
    bmin = smin;
    collected += header->count;
    outbuff += nshorts;
  }
  header->host_done = HOST_RESPONSE;
  if((bmax >= 32767) || (bmin <= -32767))
    printf("Signal clipping near sample %d.\n",collected);
  if(bmax > amax)
    amax = bmax;
  if(bmin < amin)
    amin = bmin;
  printf("max:%5d  min:%5d     ", bmax, bmin);
  fflush(stdout);
  dcgets2(0,DC_PIO_PIRL);	/* clear the interrupt flag */
  dcgets2(0,DC_PIO_PIRH);
  if (ioctl(dsp32c[0], DC_SIG, &dcmd) != 0) {
    printf("DC_SIG rejected in dsp_handle()\n");
    error_at = collected;
    return;
  }
  if (ioctl(dsp32c[0], DC_ENBPIF, &dcmd) != 0) { /* reenable interrupt */
    printf("DC_ENBPIF rejected in dsp_handle()\n");
    error_at = collected;
    return;
  }
  dcgets2(0,DC_PIO_PDRL);	/* let 32c know data was read */
  dcgets2(0,DC_PIO_PDRH);
  return;
}

/*************************************************************************/
adc_32C(outputfile, outbuffer, maxsamples, freq, sigmax, sigmin, do_echo, do_beep)
     int outputfile, maxsamples, *sigmax, *sigmin, do_echo, do_beep;
     short *outbuffer;
     double *freq;
{
  int chip;
  void (*sig_save[3])();

  /* Initialize all host variables. */
  collected = 0;
  error_at = -1;
  amax = -32767;
  amin = 32767;
  if(channel)
    sizeof_sample = sizeof(short);
  else
    sizeof_sample = sizeof(int);
  if((maxsamps = maxsamples) <= 0) {
    printf("Bogus sample count sent to adc(%d)\n",maxsamples);
    return(-1);
  }

  if(outputfile >= 0) {	  /* output to either a file OR a memory buffer */
    outfd = outputfile;
    outbuff = NULL;
  } else
    if(outbuffer) {
      outfd = -1;
      outbuff = outbuffer;
    } else {
      printf("Bogus output file and buffer specified to adc()\n");
      return(-1);
    }
  
  /* Initialize the DSP's */
  init_dsp32cs();

  /* Set the sample rate. */
  if(set_sam_rate(freq,"a")) {

    /* Setup the signal handlers. */
    sig_save[0] = signal(SIGDSP, dsp_handle);
    sig_save[1] = signal( SIGINT, stop_analog);
    sig_save[2] = signal( SIGQUIT, stop_analog);

    /* Load the DSP programs. */
    {
      char *path;
      for(chip=0; chip < 3; chip++ ){
        path = FIND_SURF_BIN(NULL,program[chip]);
	if(!path) {
	  printf("Can't find DSP file %s\n",program[chip]);
	  return(-1);
        }
	if(dcld(dsp32c[chip], path)) {
	  printf("Problems loading %s to DSP %d\n",path,chip);
	  return(-1);
	}
	free(path);
      }
    }

    /* Map in the DRAM and register file. */
    if(open_32c_io(global_dc_device("io")) && open_32c_sm(global_dc_device("mem"))) {
      
      header = (Head*)DSP_SharedMemory;

      /* Start the DSP's. */
      dcrun(dsp32c[2]);
      dcrun(dsp32c[0]);

      /* The minimum chunk size is determined by sample frequency. */
      header->chunk = 0.3 * *freq; /* 300ms for now... */

      if (ioctl(dsp32c[0], DC_SIG, &dcmd) != 0) {
	printf("DC_SIG rejected in adc()\n");
	return(-1);
      }
      /* Enable interrupts. */
      if (ioctl(dsp32c[0], DC_ENBPIF, &dcmd) != 0) {
	printf("DC_ENBPIF rejected in adc()\n");
	return(-1);
      }

      /* This starts the actual A/D process on the 32c. */
      if(dsp_wait(0, 1000, STARTUP_CODE) >= 1000) {
	printf("32C failed to respond in adc()\n");
	return(-1);
      }
       /* Finally, start the chip connected to the A/D. */
       dcrun(dsp32c[1]);

      /* Wait for samples to come in. */
      while((collected < maxsamps) && (error_at < 0)) {
	sleep(1);
      }

      /* Disable interrupt from 32c. */
      if (ioctl(dsp32c[0], DC_DISPIF, &dcmd) != 0) {
	printf("DC_DISPIF rejected in adc()\n");
      }

      /* Turn off the chips. */
       ioctl(dsp32c[1], DC_STOP);
       ioctl(dsp32c[0], DC_STOP);
       ioctl(dsp32c[2], DC_STOP);

      /* Restore the original signal handlers. */
      signal(SIGDSP,sig_save[0]);
      signal(SIGINT,sig_save[1]);
      signal(SIGQUIT,sig_save[2]);

      printf("                                          ");
      fflush(stdout);
      *sigmax = amax;
      *sigmin = amin;

      /* close the devices */
       close_dsp32cs();
       close_io_mem();
      if(error_at >= 0)
	return(error_at);
      else
	return(collected);
    } else
    printf("Problems mapping mem or io in adc()\n");
  } else
    printf("Failure while setting sample rate in adc()\n");
  return(-1);
}

