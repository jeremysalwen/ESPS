/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1993  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by Ken Hornstein
 *
 * Based heavily on code written by David Talkin and Alan Parker
 *
 */


static char *sccs_id = "@(#)dac_hpaudiolib.c	1.2 10/1/93 ERL/ATT";

#include <sys/ioctl.h>
#include <esps/epaths.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <audio/Alib.h>

int hp_maxsamps, hp_sent=0, hp_error_at;

int hp_sampling_rates[32];

extern int channel, debug_level;
static short *input_buffer = NULL;
static int buff_bytes = 0, buff_samps = 0, shifts;
extern int da_done, da_location;
static void *inhead=NULL;
static FILE *instream=NULL;
static int  infd = -1, obuffersize, sizeof_sample = 2, amax, amin;
static short *inbuff=NULL;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* HP audio structures */
Audio           *audio;
AudioAttrMask   AttribsMask, PlayAttribsMask, ignoredMask;
SStream		audioStream;
ATransID	xid;
int		streamSocket;

/*************************************************************************/
static scale(data, nsamps, channel, shift)
     register int nsamps, channel, shift;
     short *data;
{
  if(nsamps > 0) {
    register short *to;

    to = data;
    if(!channel)		/* stereo */
      do {
	(*to++) <<= shift;

	(*to++) <<= shift;
      } while(--nsamps);
    else			/* mono */
      do {
	(*to++) <<= shift;
      } while(--nsamps);
  }
}

/*************************************************************************/
hp_handle(not_first_call, canwrite_arg)
     int not_first_call;
     long canwrite_arg;
{
  long  sams_out;
  static long canwrite;
  
  if(canwrite_arg > 0)
	canwrite = canwrite_arg;

      if(debug_level)
	fprintf(stderr,"entering hp_handle\n");

  if((sams_out = (hp_maxsamps - hp_sent)) > buff_samps)
    sams_out = buff_samps;

  if(instream || (infd >= 0)) { /* samples from unix file? */
    int nread;

  if (debug_level && !input_buffer)
	fprintf(stderr,"Yikes, input_buffer is null!!\n");

    if((nread = read_any_file(input_buffer, sizeof_sample, sams_out,
			      instream, inhead, infd)) != sams_out) {
      if(debug_level)
	fprintf(stderr,"Read error at %d(%d:%d)\n",hp_sent,sams_out,nread);
      hp_error_at = hp_sent;
      sams_out = nread;
      hp_maxsamps = hp_sent+sams_out; /* To force buffer drain and termination. */
      if(debug_level)
	fprintf(stderr,"Reached file end at sample %d\n",hp_sent);
    }
    scale(input_buffer,sams_out,channel,shifts);
  } else {			/* must come from a memory buffer */
    register int i, nshifts=shifts;
    register short *from, *to, smax, smin;

    if(channel)	{		/* mono */
      i=sams_out;
      to = input_buffer;
      from = inbuff;
      for(; i--; ) {
	*to++ = (*from++)<<nshifts;
      }
    } else {			/* stereo */
      for(i=sams_out,	to = input_buffer,
	  from = inbuff; i--; ) {
	*to++ = (*from++)<<nshifts;
	*to++ = (*from++)<<nshifts;
      }
    }
  }
  if(write(streamSocket, (char *)input_buffer, sams_out*(sizeof_sample)) <0) {
	perror("Error writing audio samples");
	return (-1);
  }
  hp_sent += sams_out;
  da_location += sams_out;
  if(inbuff)
    inbuff += (sams_out * (sizeof_sample/sizeof(short)));
  if(hp_sent >= hp_maxsamps) { /* Wait for the last samples to drain. */
    hp_completion();
  }
  return(sams_out);
}

/*************************************************************************/

/*************************************************************************/
hp_completion()
{

  if(!da_done) {
    if(debug_level) fprintf(stderr,"\n");
    da_done = TRUE;
  close( streamSocket );
  /*
   * set close mode to prevent playback from stopping 
   *  when we close audio connection
   */
  ASetCloseDownMode( audio, AKeepTransactions, NULL );

  /*
   *  That's all, folks!
   */
  ACloseAudio( audio, NULL );
    clear_dac_callbacks();
  }

}

/*************************************************************************/
closest_srate(freq, rates)
     double *freq;
     register int *rates;
{
  if(freq && rates && (*rates > 0)) {
    register int i, ifr = *freq + 0.5, best, min;

    best = *rates++;
    min = abs(best-ifr);
    while(*rates) {
      if((i = abs(*rates - ifr)) < min) {
	best = *rates;
	if((min = i) == 0) break;
      }
      rates++;
    }
    if(best != ifr) {
      fprintf(stderr,"Closest rate available to that requested (%f) is %d\n",
	      *freq,best);
      *freq = best;
    }
    return(best);
  } else
    fprintf(stderr,"Bad args to closest_srate()\n");
  return(0);
}

/*************************************************************************/
dac_hp(inputfile, inbuffer, sig_size, freq, sigmax)
     int inputfile, sig_size, sigmax;
     short *inbuffer;
     double *freq;
{
  return(dacmaster_hp(inputfile, inbuffer, sig_size, freq,
			  sigmax, NULL, NULL));
}

/*************************************************************************/
dacmaster_hp(inputfile, inbuffer, sig_size, freq, sigmax, stream, eheader)
     int inputfile, sig_size, sigmax;
     short *inbuffer;
     double *freq;
     FILE *stream;
     void *eheader;
{
  long            stat;
  long buflen, samps_per_frame, frames_per_sec;
  unsigned long *rates;
  SSPlayParams    streamParams;
  AByteOrder      byte_order, play_byte_order;
  AGainEntry      gainEntry[4];
  AudioAttributes Attribs, PlayAttribs;
  long length, soffset;
  int i,j;

  hp_sent = 0;
  hp_error_at = -1;
  shifts = 0;
  length = 0; soffset = 0;
  if(!sigmax) sigmax = 32767;
  if(sigmax < 16384)
    while((sigmax << shifts) < 16384) shifts++;

  if(channel)
    sizeof_sample = sizeof(short);
  else
    sizeof_sample = sizeof(int);

  samps_per_frame = sizeof_sample/sizeof(short);

  if((hp_maxsamps = sig_size) <= 0) {
    printf("Bogus sample count sent to dac(%d)\n",sig_size);
    return(-1);
  }

  if((inputfile >= 0) || stream) {  /* input from either a file OR a memory buffer */
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
      printf("Bogus input file and buffer specified to dacmaster_hp()\n");
      return(-1);
    }

  /* Connect to the HP audio server and setup necessary junk */

  audio = AOpenAudio( NULL, NULL );
  AttribsMask = 0;
  rates = ASamplingRates(audio);

  for(i = ANumSamplingRates(audio) - 1, j = 0; i >= 0;  i--)
	hp_sampling_rates[j++]=rates[i];

  hp_sampling_rates[j]=0;
	

  frames_per_sec = Attribs.attr.sampled_attr.sampling_rate = closest_srate(freq,
						hp_sampling_rates);
  AttribsMask = (AttribsMask | ASSamplingRateMask | ASChannelsMask);
  Attribs.attr.sampled_attr.channels = channel ? 1 : 2;
  streamParams.priority = APriorityNormal;          /* normal priority */
  AChooseSourceAttributes(audio, NULL, NULL, AFFRawLin16, AttribsMask,
	&Attribs,  &soffset, &length, &byte_order, NULL );

  AChoosePlayAttributes (audio, &Attribs, 0, &PlayAttribs, &play_byte_order, NULL );

  switch(PlayAttribs.attr.sampled_attr.channels ) {
  case 1:
	gainEntry[0].u.o.out_ch = AOCTMono;
	gainEntry[0].gain = AUnityGain;
	gainEntry[0].u.o.out_dst = AODTMonoJack;
	break;
  case 2:
	gainEntry[0].u.o.out_ch = AOCTLeft;
	gainEntry[0].gain = AUnityGain;
	gainEntry[0].u.o.out_dst = AODTLeftJack;
	gainEntry[1].u.o.out_ch = AOCTLeft;
	gainEntry[1].gain = AUnityGain;
	gainEntry[1].u.o.out_dst = AODTRightJack;
	break;
}

  streamParams.gain_matrix.type = AGMTOutput;       /* gain matrix */
  streamParams.gain_matrix.num_entries = PlayAttribs.attr.sampled_attr.channels;
  streamParams.gain_matrix.gain_entries = gainEntry;
  streamParams.play_volume = AUnityGain;            /* play volume */
  streamParams.priority = APriorityNormal;          /* normal priority */
  streamParams.event_mask = 0;                      /* don't solicit any events */

  /*
   * create an audio stream
   */
  xid = APlaySStream( audio, ~0, &PlayAttribs, &streamParams,
		         &audioStream, NULL );
    /*
     * create a stream socket
     */
  streamSocket = socket( AF_INET, SOCK_STREAM, 0 );
  if( streamSocket < 0 ) {
    perror( "Socket creation failed" );
    exit(1);
  }

  /*
   * connect the stream socket to the audio stream port
   */
  stat = connect( streamSocket, (struct sockaddr *)&audioStream.tcp_sockaddr,
		   sizeof(struct sockaddr_in) );
  if( stat < 0 ) {
    perror( "Connect failed" );
    exit(1);
  }
  
  buflen = sizeof_sample * frames_per_sec * .5; /* 2-second read buffer */
  if (debug_level)
	fprintf(stderr, "buff_bytes=%d, buflen=%d\n",buff_bytes,buflen);
  if (debug_level)
	fprintf(stderr, "sizeof_sample=%d, frames_per_sec=%d\n",sizeof_sample,frames_per_sec);
  if(buflen > buff_bytes) {
    if(buff_bytes) {
      free(input_buffer);
      input_buffer = NULL;
      buff_bytes = 0;
    }
    if(!(input_buffer = (short*)malloc(buflen))) {
      fprintf(stderr,"Allocation problems in dacmaster_hp()\n");
      return(-1);
    }
    buff_bytes = buflen;
  }
  buff_samps = buff_bytes/sizeof_sample;

  if (debug_level)
	if (input_buffer)
		fprintf(stderr, "Hmm, input_buffer fine here\n");
	else
		fprintf(stderr, "Yikes, input_buffer should NOT be NULL!!\n");
  da_done = FALSE;
  /* Send first buffer to dsp and start interrupts. */

  set_dac_callbacks(100000);
  hp_handle(0,buff_samps);

  if(debug_level)
    fprintf(stderr,"tot_samples:%d D/A_buf_size:%d input_buffer:%x buff_bytes:%d frames_per_sec:%d buflen:%d sizeof_sample:%d\n",
	  sig_size, obuffersize,input_buffer,buff_bytes, frames_per_sec,buflen,sizeof_sample);

}

int
hp_samps_sent()
{
	
  return (hp_sent);
}


#ifdef 0
{
  char            server[80], afile[80], c;
  int             arg;
  AEvent          event;
  AFileFormat     switchFormat;
  int             n, switchCode, loopCount;
  char            *pSpeaker;
  AErrorHandler   prevHandler;  /* pointer to previous handler */
  int		  fd, len, inLen, len_written;
  FILE            *pfile;
  char            *inBuf, *bufBase, *buf;
  SunHeader       auFileHeader;
  Bool            filenameFound, audioPaused;
  int             useIntSpeaker;
  long            seekOffset, data_length, bytes_written,bytes_read, pauseCount;
  AConvertParams  *convert_params;

  pSpeaker = getenv( "SPEAKER" );         /* get user speaker preference */
  if ( pSpeaker ) {
    useIntSpeaker = ( (*pSpeaker == 'i') || (*pSpeaker == 'I') );
  } else {
    /* SPEAKER environment variable not found - use internal speaker */  
    useIntSpeaker = 1;
  }
  switch(PlayAttribs.attr.sampled_attr.channels ) {
  case 1:
    gainEntry[0].u.o.out_ch = AOCTMono;
    gainEntry[0].gain = AUnityGain;
    gainEntry[0].u.o.out_dst = (useIntSpeaker) ? AODTMonoIntSpeaker : AODTMonoJack;
    break;
  case 2:
  default:    /* assume no more than 2 channels */
    gainEntry[0].u.o.out_ch = AOCTLeft;
    gainEntry[0].gain = AUnityGain;
    gainEntry[0].u.o.out_dst = (useIntSpeaker) ? AODTLeftIntSpeaker : AODTLeftJack;
    gainEntry[1].u.o.out_ch = AOCTRight;
    gainEntry[1].gain = AUnityGain;
    gainEntry[1].u.o.out_dst = (useIntSpeaker) ? AODTRightIntSpeaker : AODTRightJack;
    break;
  }
  
  
  /*
   * prevent attempting to loop if audio data source is stdin
   */
  if ( !filenameFound ) {
    loopCount = 1;
  }
  
  /*
   * allocate a buffer for the converted data
   */
  bufBase = malloc( audio->block_size );
  
  /*
   * calculate the required buffer size for the data prior to conversion
   */
  inLen = ACalculateLength (audio, audio->block_size, &PlayAttribs, &Attribs, NULL); 
   
  /*
   * allocate a buffer for the pre-converted data
   */
  inBuf = malloc(inLen );

  /*
   * start stream paused so we can transfer enough data (3 seconds worth) before
   *  playing starts to prevent stream from running out 
   */
  APauseAudio( audio, xid, NULL, NULL );
  pauseCount = 3 * PlayAttribs.attr.sampled_attr.channels
                 * PlayAttribs.attr.sampled_attr.sampling_rate
                 * (PlayAttribs.attr.sampled_attr.bits_per_sample >> 3);
  audioPaused = True;

  /*
   * prepare for conversion. Must remember to free convert_params by calling AEndConversion
   */
  convert_params = ASetupConversion (audio, &Attribs, &byte_order, &PlayAttribs, &play_byte_order, NULL );

  /*
   * Loop # of times specified by user
   */
  while ( loopCount ) {

    /*
     * seek to beginning of audio data
     */
    lseek( fd, seekOffset, SEEK_SET );

    while(( len = read( fd, inBuf, inLen )) > 0 ) {
     /*
      * read a buffer-full of data from the file & convert it
      */
      AConvertBuffer(audio, convert_params, inBuf, len, bufBase, audio->block_size, 
                     &bytes_read, &bytes_written, NULL) ; 

      len = bytes_written;
      buf = bufBase;
     /*
      * write the converted data to the stream socket
      */
      while( len ) {
        /*
	 * write converted data to stream socket until we have emptied buffer
	 */
	if(( len_written = write( streamSocket, buf, len )) < 0 ) {
          perror( "write failed" );
	  exit(1);
        }
	buf += len_written;
	len -= len_written;
	if ( audioPaused ) {
	  pauseCount -= len_written;
	  if ( (len_written == 0) || ( pauseCount <= 0) ) {
	    AResumeAudio( audio, xid, NULL, NULL );
	    audioPaused = False;
	  }
	}
      }
    }
    loopCount--;
  }
  if ( audioPaused ) {
    AResumeAudio( audio, xid, NULL, NULL );
  }

  /* free the convert_params structure and flush out the conversion pipeline */
  AEndConversion(audio, convert_params, bufBase, audio->block_size, &bytes_written, NULL) ;   

  len = bytes_written;
  buf = bufBase;
  /*
   * write the converted data to the stream socket
  */
  while( len ) {
      /*
       * write converted data to stream socket until we have emptied buffer
      */
      buf += len_written;
      len -= len_written;
  }

  if ( filenameFound ) {
    close( fd );
  }

  exit(0);
}
#endif
