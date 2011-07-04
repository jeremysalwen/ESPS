/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Derek Lin
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *
 */

static char *sccs_id = "@(#)dac_codec.c	1.4	5/22/93	ERL";

#include <Objects.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stropts.h>
#include <esps/epaths.h>
#include <sys/param.h>
#include <sun/audioio.h>
#include <esps/esps.h>
#include <esps/feasd.h>

#define SPARC_SAMPLE_FREQ 8000
#define MAX_GAIN 100
struct feasd *rec;                /* ESPS record -- for ereader.c */

/* externals required by calling program */
extern int debug_level;
extern int left, right, bflag, gflag, aflag, external;
extern float gain;
extern int da_location;
extern unsigned int gain_codec;
extern int gain_codecflag;

static long long_da_loc;

static int nresamp;               /* length of data buffer after resampling */
static char *bytedata = NULL;            /* data after mu-law conversion */
static int readlen = 0;                  /* read buffer length */
static int sizeof_sample = sizeof(short);
static long maxsamps;                     /* total number of samples */
static long sams_read=0;                  /* samples read so far */
static long error_at;                     /* sample position when error */
static int da_done;                      /* signal for D/A completion */
static int shifts;                       /* automatic scaling */

/* 
 * four main arguments passed by calling program
 */
static int  infd = -1;                   /* file descriptor */
static void *inhead=NULL;                /* ESPS file header */
static FILE *instream=NULL;              /* file stream */
static short *aft_resamp_data = NULL;    /* memory buffer */

static int Audio_fd;

void (*sig_save[4])();
audio_info_t info;                 /* see /dev/audio.h */
double input_rate;
/*
 *
 */
void stop_codec_da_waves(sig)
     int sig;
{
  long number_left;

  da_done = 1;
  completion();

  if(debug_level)
    fprintf(stderr,"Caught a signal in waves (%d); da_location:%d\n",
	    sig, long_da_loc);
  if((number_left = maxsamps - long_da_loc) > 0) {
    char str[200];
    sprintf(str,"set da_location %d\n", long_da_loc);
    send_xwaves2(NULL, NULL, str, debug_level);
    /* This pause seems to be necessary to prevent the possibility that
       the child's death will be registered before the send_xwaves is! */
    usleep(200000);
  }
  exit(0);
}

/*
 *
 */
void interrupt_codec_da(sig)
     int sig;
{
  error_at = sams_read;
  da_done = 1;
  completion();
  if(debug_level)
    fprintf(stderr,"Caught a signal from intrp (%d); da_location:%d\n",
	    sig, long_da_loc);
  exit(0);
}

/*
 *
 */
void stop_codec_da()
{
  error_at = sams_read;
  da_done = 1;
  completion();
  if(debug_level)
    fprintf(stderr,"stopping codec D/A; da_location:%d\n",
	    long_da_loc);
}


/*
 *
 */

/* normal completion will have its da_done set here.  completion
   due to abnormal condition(intrpt) has da_done set in interrupt routine*/
static completion()
{
  int err;
  unsigned int sleep = 10000;
  extern int errno;
  
  if(Audio_fd >0)
    {
      if(da_done){          /* if interrupt, flush everything */
	if((err = ioctl(Audio_fd, I_FLUSH, FLUSHW)) < 0)
	  {
	    fprintf(stderr,"Error flushing device\n");
	    exit(-1);
	  }
      }
      else                  /* if normal exit, wait till finish draining */
	{
	  do
	    {
	      if(ioctl(Audio_fd, AUDIO_GETINFO, &info) < 0)
		{
		  fprintf(stderr,"Error draining, errno %d\n", errno);
		  exit(-1);
		}
	      usleep(sleep);
	    } while ( (int) info.play.active );
	}
      if((err = ioctl(Audio_fd, AUDIO_GETINFO, &info)) < 0){
	fprintf(stderr,"Error reading Audio_info_t, errno %d\n", errno);
	exit(-1);
      }
      long_da_loc += (int) info.play.samples * input_rate / SPARC_SAMPLE_FREQ; 
      close(Audio_fd);
    }
  else
    {
      fprintf(stderr,"Device was closed pre-maturely\n");
      exit(-1);
    }
  da_done = TRUE;
  /* Restore the original signal handlers. */
  signal(SIGINT,sig_save[1]);
  signal(SIGQUIT,sig_save[2]);
  signal(SIGUSR1,sig_save[3]);
}


/*
 *
 */

dac_codec(inputfile, inbuffer, sig_size, freq, sigmax)
     int inputfile, sig_size, sigmax;
     short *inbuffer;
     double *freq;
{
  return(dacmaster_codec(inputfile, inbuffer, sig_size, freq,
			  sigmax, NULL, NULL, 1));
}

/*
 * allocate arrays, sets up device, read and play from:
 *      a memory buffer, or a file pointed by file descriptor or file stream.
 * If frequency is not 8000 for an ESPS file, resampling is done.
 * operation: read -> resample -> mu-law convert -> play
 * Non-ESPS file and memory buffer are treated as SHORT, 8000 Hz
 */

dacmaster_codec(inputfile, membuffer, sig_size, freq, sigmax, stream, eheader,
		repeats )
     int inputfile;                  /* file descriptor */
     int sig_size, sigmax;           /* signal length and signal max value */
     short *membuffer;               /* a memory buffer containing data */
     double *freq;                   /* sampling frequency of input */
     FILE *stream;                   /* file stream */
     void *eheader;                  /* header of ESPS file */
     int repeats;
{
  int shifts = 0, tmp, err;
  long offset;

  long_da_loc = da_location;
  input_rate = *freq;
  sams_read = 0;
  error_at = -1;

  if(!sigmax) sigmax = 32767;         /* automatic scaling to full range */
  if(aflag){
    tmp = sigmax;
    if(tmp < 16384)
      while((tmp << shifts) < 16384) shifts++;
    left +=shifts;
    shifts=0;
    if(sigmax > 16384)
      while((sigmax >> shifts) > 16384) shifts++;
    right +=shifts;
  }
  if((maxsamps = sig_size) <= 0) {
    printf("Bogus sample count sams_read to dac(%d)\n",sig_size);
    return(-1);
  }
  if((inputfile >= 0) || stream) {      /* input from a file */
    infd = inputfile;
    instream = stream;
    inhead = eheader;
    aft_resamp_data = NULL;
    offset = ftell(instream);
  } else                                 /* input from a memory buffer */
    if(membuffer) {
      infd = -1;
      aft_resamp_data = membuffer;
      instream = NULL;
      inhead = NULL;
    } else {
      printf("Bogus input file and buffer specified to dacmaster_codec()\n");
      return(-1);
    }

  /* Setup the signal handlers. */
  sig_save[1] = signal( SIGINT, interrupt_codec_da);
  sig_save[2] = signal( SIGQUIT, interrupt_codec_da);
  sig_save[3] = signal( SIGUSR1, stop_codec_da_waves);
  da_done = FALSE;
  
  /* open audio device and initialize */
  if(( Audio_fd = open("/dev/audio",2)) < 0 ){
    fprintf(stderr,"dac_codec: Can't open audio device.\n");
    exit(-1);
  }
  AUDIO_INITINFO(&info);
  if(external)
    info.play.port = AUDIO_HEADPHONE;
  else
    info.play.port = AUDIO_SPEAKER;
  if ( gain_codecflag && (int) gain_codec >= 0 && (int)gain_codec <= MAX_GAIN)
    info.play.gain = AUDIO_MIN_GAIN + (unsigned) irint ((double) 
      (AUDIO_MAX_GAIN - AUDIO_MIN_GAIN) * (double) gain_codec /
	(double) MAX_GAIN);
  else{
    if (gain_codecflag){
      fprintf(stderr, "dac_codec: codec gain level is out of range.\n");
      exit(-1);
    }
  }
  if( (err = ioctl(Audio_fd, AUDIO_SETINFO, &info)) < 0){
    fprintf(stderr, "dac_codec: Can't AUDIO_SETINFO.\n");
    exit(-1);
  }
  
  readlen = input_rate * 0.5;                     /* 1/2-second read buffer */
  nresamp = (int)(((float) readlen)*(SPARC_SAMPLE_FREQ/(float) input_rate)); 
  
  /* allocate data arrays */
  if(inhead)
    rec = allo_feasd_recs( inhead, SHORT, readlen, (char *)NULL, NO);
  if( !aft_resamp_data )
    if(!(aft_resamp_data = (short *) malloc(sizeof(short) * nresamp))){
      fprintf(stderr,"dacmaster_codec: Can't allocate aft_resamp_data.\n");
      return(-1);
    }
  if(!(bytedata = (char *) malloc((unsigned) nresamp))){
    fprintf(stderr,"dac_codec: Can't allocate bytedata.\n");
    return(-1);
  }

  if(debug_level)
    fprintf(stderr,"tot_samples:%d membuffer:%x freq:%f readlen:%d sizeof_sample: %d\n", sig_size, membuffer, input_rate, readlen, sizeof_sample);

  /* main loop */
  while(repeats--){
    while(!da_done && (error_at < 0)) {
      if(codec_handle() == 0) break;
    }
    /* Error or stdin input exit */
    if(error_at >= 0){
      if(debug_level)
	fprintf(stderr,"codec_handle(): closing audio device\n");
      completion();
      return(error_at);
    }

    /* if repeating, reset everything back to start */
    if(repeats){
      sams_read = 0;
      nresamp = (int)(((float) readlen)*(SPARC_SAMPLE_FREQ/input_rate));
      if(fseek(instream, offset, 0) < 0){
	fprintf(stderr,"dacmaster_codec(): Can't fseek\n");
	exit(-1);
      }
    }
  }

  /* Normal exit */
  if(debug_level)
    fprintf(stderr,"codec_handle(): closing audio device\n");
  completion();

  return(sams_read);
}

/*
 * read, resample memory buffer and file 
 */

static codec_handle()
{
  int sams_frm;
  int err;

  if((sams_frm = (maxsamps - sams_read)) > readlen)
    sams_frm = readlen;

  if(instream || (infd >= 0)) {		/* samples from unix file? */
    int nread;

    /* read sams_frm long data, force termination if sams_frm is not read */
    if((nread = read_any_file(aft_resamp_data, sizeof_sample, sams_frm,
			      nresamp,instream, inhead, infd, 
			      input_rate)) != sams_frm){
      if(debug_level)
	fprintf(stderr,"Read error or end of stdin at %d(%d:%d)\n",sams_read,sams_frm,nread);
      error_at = sams_read;
      sams_frm = nread;
      maxsamps = sams_read + sams_frm;  /* To force termination */
      if(debug_level)
	fprintf(stderr,"Reached file end at sample %d\n",sams_read);
    }

    /* if sams_frm is less than 1/2 sec (readlen) */
    if(sams_frm < readlen) nresamp = (int) nread*SPARC_SAMPLE_FREQ/input_rate;

    if (gflag)
      {
	register short *sp;
	register float ft, g=gain, round = 0.5;
	register int i, nshifts = shifts;
	for (i=nresamp, sp = aft_resamp_data ; i--;)
	  if((ft = *sp * g) >= 0.0)
	    *sp++ = ft + round;
	  else
	    *sp++ = ft - round;
      } 
    else
      {
	register short *sp;
	register int i;
	if ( left > right && (aflag || bflag))
	  {
	    register int shift;
	    shift = left - right;
	    for (i=nresamp, sp = aft_resamp_data; i--; )
	      (*sp++) <<= shift;
	  } 
	else
	  if (right > left && (aflag || bflag)) 
	    {
	      register int shift, i;
	      shift = right - left;
	      for (i=nresamp, sp = aft_resamp_data; i--; )
		(*sp++) >>= shift;
	    }
      }
  }

  if(linear_to_mu(aft_resamp_data, bytedata, (long)nresamp) !=0 ){
    fprintf(stderr, "codec_handle: Error converting to mu data.-exiting.\n");
    exit(-1);
  }
  if( write( Audio_fd, bytedata, nresamp) < 0) perror("/dev/audio");

    if(Audio_fd >0){
      if((err=ioctl(Audio_fd, AUDIO_GETINFO, &info)) < 0){
	fprintf(stderr,"Error reading Audio_info_t\n");
	exit(-1);
      }
    }

  sams_read += sams_frm;

  /* adjust the pointer to memory buffer */
  if(infd < 0 && !instream && !inhead)
    aft_resamp_data += (nresamp * (sizeof_sample/sizeof(short)));

  if(error_at >= 0 || sams_read >= maxsamps)
    return(0);
  else
    return(1);
}




