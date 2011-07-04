/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "CopyRight 1991 Massachusetts Institute of Technology
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by: David Talkin
 *
 *  newplay.c
 *  a flexible single/dual-channel D/A program
 *
 */
static char *sccs_id = "@(#)newplay.c	1.5 6/11/96 ERL";

#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <esps/esps.h>
#include <esps/epaths.h>
#include <esps/sd.h>			/* SD file stuff */
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/ss.h>
#include <Objects.h>


/*
  This program plays PCM data, reading input from stdin or files;
  with or without SIG or ESPS headers;
  mono to either or both channels, or stereo.
*/

#ifdef SG
#define SYNTAX { \
fprintf(stderr,"Usage: %s [-f<sample frequency>] [-c<channel> (1 or 2)] [-r<range>] [-s<start time>] [-e<end time>] [-x<debug level>] [-H] [[-i]<input file>]\n",av[0]); \
  fprintf(stderr, "If no channel is specified and files are headerless, input is assumed to be mono.\n"); \
  fprintf(stderr,"For headerless files, channel=2 selects stereo data, stereo output.\n"); \
  fprintf(stderr, "If no input file is specified, input is taken from stdin.\n"); \
  fprintf(stderr, "If -H is specified, the file is treated as headerless.\n"); \
  fprintf(stderr,"The program \"apanel\" may be used to adjust output level.\n"); \
      exit(ERR); \
}		  
#else
#define SYNTAX { \
fprintf(stderr,"Usage: %s [-f<sample frequency>] [-c<channel> (0, 1 or 2)] [-r<range>] [-s<start time>] [-e<end time>] [-x<debug level>] [-H] [[-i]<input file>]\n",av[0]); \
  fprintf(stderr, "If no channel is specified and files are headerless, input is assumed to be mono.\n"); \
  fprintf(stderr,"For mono files with headers, Channel == 2: send to both outputs; 1: to channel 1; 0: to channel 0.\n"); \
  fprintf(stderr,"For headerless files, Channel == 2: stereo data, stereo output; 1: to channel 1; 0: to channel 0.\n"); \
  fprintf(stderr, "If no input file is specified, input is taken from stdin.\n"); \
  fprintf(stderr, "If -H is specified, the file is treated as headerless.\n"); \
      exit(ERR); \
}		  
#endif

#define ERR 4
#define REALLY_HUGE_INTEGER 1000000000
#define MAXDA  32767

int debug_level = 0, da_location = 0, max_buff_bytes = 2000000, w_verbose = 0;

char *waves_name = NULL;
char *display_name = NULL;

extern int qflag, clip, gflag, right, left, bflag;
extern float gain;

dim_ok(dim)
     int dim;
{
  return((dim == 1) || (dim == 2));
}

main(ac, av)
     int ac;
     char **av;

{
  int ifd = 0, doseek, sizeof_sample = sizeof(short);
  int sleep_time, channel = -1, readf = FALSE, start_p, end_p, amp, length;
  int noheader = FALSE;
  double srate = 16.0, fr = srate*1000.0, sstart, start = -1.0, end = -1.0, tmp;
  char *ifile = NULL, *r_switch = NULL;
  int c;
  extern int optind;
  extern char *optarg;
  
  while((c = getopt(ac,av,"f:s:e:c:x:i:r:qhHwkRgCd:n:?")) != EOF) {
    switch(c) {
    case 'f':
      srate = atof(optarg);
      if(srate > 96.0)		/* Assume user means Hz. */
	srate /= 1000.0;
      readf = TRUE;
      fr = srate * 1000.0;
      break;
    case 'q':	/* q ignored for backwards compatibility */
      break;
    case 'w':
    case 'k':
    case 'R':
    case 'g':
    case 'C':
      if(debug_level) {
	fprintf(stderr,"Options [wkRgC] are now obsolete.\n");
        SYNTAX
	}
      break;
    case 'x':
      debug_level = atoi(optarg);
      break;
    case 'c':
      channel = atoi(optarg);
      if((channel > 2) || (channel < 0)) {
	fprintf(stderr,"Bad channel specification (%d)\n",channel);
	exit(ERR);
      }
      break;
    case 'i':
      ifile = optarg;
      break;
    case 's':
      if (r_switch) {
	fprintf(stderr,"Cannot use -s with -r\n"); exit(1);}
      start = atof(optarg);
      break;
    case 'e':
      if (r_switch) {
	fprintf(stderr,"Cannot use -e with -r\n"); exit(1);}
      end = atof(optarg);
      break;
    case 'r': 
      if (start != -1.0 || end != -1.0) {
	fprintf(stderr,"Cannot use -r with -s or -e\n"); exit(1);}
      r_switch = optarg;
      break;
    case 'H':
      noheader = TRUE;
      break;
    case 'd':
      display_name = optarg;
      break;
    case 'n':
      waves_name = optarg;
      break;
    case 'h':
    case '?':
    default:
      SYNTAX
      }
  }

  if(ac < 2) SYNTAX ;

  do {				/* for all input specifications... */
    Header *h;
    Signal *sig;
    FILE *istr = stdin;
    int hseek = 0;
    struct header *ehead = NULL;

    if(ac > optind)
      ifile = av[optind];
    if(ifile && !strcmp(ifile,"-")) {
      ifile = NULL;
      ifd = 0;
      istr = stdin;
    }
    if(ifile && *ifile)
      if((ifd = open(ifile, O_RDONLY)) < 0) {
	fprintf(stderr,"Can't open %s for input.\n",ifile);
	exit(ERR);
      }
    if(ifd > 0)			/* i.e. input is not stdin */
      if(!(istr = fdopen(ifd,"r"))) {
	fprintf(stderr,"Can't open a stream from %d\n",ifd);
	exit(-1);
      }

    if(!noheader && (h = (Header*)get_header(ifd)) &&
       (sig = (Signal*)new_signal(ifile,ifd,h,NULL,0,0.0,0))) {
      if(dim_ok(sig->dim)) {
        if(h->magic == ESPS_MAGIC) {
	  
	  *(sig->smax) = *(sig->smin) = get_genhd_val("max_value",h->esps_hdr,0.0);
	  istr = h->strm;
	  hseek = h->esps_nbytes;
	  if (h->esps_hdr->common.type == FT_SD) 
	    ehead = (struct header*)sdtofea(h->esps_hdr);
	  else
	    ehead = h->esps_hdr;
	  if ((sig->dim ==1 || sig->dim == 2) &&
	      (h->esps_hdr->common.type == FT_SD  ||
	       (h->esps_hdr->common.type == FT_FEA || 
		h->esps_hdr->hd.fea->fea_type == FEA_SD))) {   
	    switch(get_sd_type(ehead)) {
	    case SHORT:
	      sizeof_sample = sizeof(short) * sig->dim;
	      break;
	    case CHAR:
	      sizeof_sample =  sig->dim;
	      break;
	    default:
	      sizeof_sample = sizeof(float) * sig->dim;
	      break;
	    }
	    if(debug_level)
	      fprintf(stderr,"Setting sizeof_sample %d in 1-D ESPS sect.\n",sizeof_sample);
	  } else {
	    if(get_sd_type(ehead) == SHORT) {
	      sizeof_sample = sizeof(short) * sig->dim;
	      ehead = NULL;	/* Indicate that FT_SD processing CAN NOT be used! */
	      if(debug_level)
		fprintf(stderr,"Setting sizeof_sample %d in 2-D ESPS sect.\n",sizeof_sample);
	    } else {
	      fprintf(stderr,"Can't handle 2-D data of type %d yet\n",get_sd_type(ehead));
	      exit(-1);
	    }
	  }
	} else {		/* might be a SIGnal file... */
	  if((type_of_signal(sig) & VECTOR_SIGNALS) != P_SHORTS) {
	    fprintf(stderr,"Can't play signals of type 0x%x and dimension %d\n",
		    type_of_signal(sig), sig->dim);
	    exit(-1);
	  }
	  hseek = sig->bytes_skip;
	  sizeof_sample = sizeof(short) * sig->dim;
	  if(debug_level)
	    fprintf(stderr,"Setting sizeof_sample %d in\n",sizeof_sample);
	}
	
	if(ifd) {
	  if(debug_level)
	    fprintf(stderr,"Seeking in header setup\n");
	  fseek(istr, hseek, 0);	/* position just past the header */
	}
	if((sig->dim < 2) && (channel < 0)) /* If channel not specified, */
	  channel = 2;		/* cause mono signal to come out of both channels. */
	if(sig->dim > 1)	/* Force stereo play of stereo signals. */
	  channel = -1;

	if(!readf)
	  fr = sig->freq;

	if((amp = ((*sig->smax > -(*sig->smin))? *sig->smax : - *sig->smin))
	   < 128) amp = 32767;

	if((length = sig->file_size) <= 0) {
	  if(debug_level)
	    fprintf(stderr,"No length info available in header.\n");
	  length = REALLY_HUGE_INTEGER;
	}

        if (r_switch) {
          start_p = 1;
	  end_p = length;
          lrange_switch (r_switch, &start_p, &end_p, 1);
	  if(debug_level)
	  if (end_p > length)
	    end_p = length;
	  if(!end_p && length) { /* end not specified; play to end of file */
	    end_p = length;
	    length -= start_p;
	  }
          length = end_p - start_p + 1;
	  if(debug_level)
	    fprintf(stderr,"Range specified (-r); start_p:%d end_p:%d length:%d\n",
		    start_p,end_p,length);
	  doseek = start_p-1;

	} else { /* Were start and/or end specified as seconds? */

	  if(((end < 0.0) || (end > start)) && (start > sig->start_time)) {
	    doseek = 0.5 + (start - sig->start_time)*fr;
	    sstart = start;
	  } else {
	    sstart = sig->start_time;
	    doseek = 0;
	  }
	  length -= doseek;
	  if((end > sig->start_time) && (end > start) &&
	     (end < (tmp = (sig->start_time +
			    ((double)sig->file_size)/fr)))) {
	    length -= ((tmp - end)*fr);
	  }
	}
      } else {
	fprintf(stderr,"Can't play signals of type 0x%x and dimension %d\n",
	       type_of_signal(sig), sig->dim);
	exit(-1);
      }
    } else {			/* punt if there is no usable header */
      if(channel < 0)	/* No user spec: assume input is mono; play to channel B */
	channel = 1;
      if(channel == 2) { /* User specified stereo data; play as stereo */
	channel = -1;
	sizeof_sample = sizeof(short) * 2;
	if(debug_level)
	  fprintf(stderr,"Setting sizeof_sample %d for no-header case.\n",sizeof_sample);
      }
      if(ifd) {	/* Is the input from a pipe? */
	if(debug_level)
	  fprintf(stderr,"Seeking after header setup\n");
	fseek(istr,0,2);
	if((length = ftell(istr)) > 10) {
	  fseek(istr,0,0);
	  length /= sizeof_sample;
	  if((start>0.0)&&((start*fr)<length)) {
	    doseek = start*fr;
	    sstart = start;
	  } else {
	    doseek = 0;
	    sstart = 0.0;
	  }
	  if((end > start) && ((end*fr) < length))
	    length = end*fr;
	  length -= doseek;
	  amp = 32767;
	} else
	  printf("Unreasonably small file (%s)\n",ifile);
      } else {			/* Can't actually seek or determine file size. */
	if(start > 0.0) {
	  char dummy[10000];
	  int toskip = (0.5 + (start*fr))*sizeof_sample, skipped,
	  da_location = (0.5 + (start*fr));

	  while(toskip > 0) {
	    if((skipped = toskip) > 10000) skipped = 10000;
	    if((skipped = fread(dummy, 1, skipped, istr)) <= 0) {
	      fprintf(stderr,"Exhausted samples while seeking to requested start time (%f)\n",start);
	      exit(-1);
	    }
	    toskip -= skipped;
	  }
	}
	length = REALLY_HUGE_INTEGER;
	doseek = 0;
      }
     amp = 32767;
   }    /* end of handler for headerless input */

    if(doseek) {
      if(debug_level)
	fprintf(stderr,"Seeking to sample # %d; sizeof_sample:%d\n",doseek,sizeof_sample);
      da_location = doseek;
      if (ehead) 
	fea_skiprec(istr, doseek, ehead);
      else
        fseek(istr, doseek*sizeof_sample, 1);
    }
    srate = fr/1000.0;
    
    if(debug_level)
      fprintf(stderr,
	      "Calling play_from_file. length=%d srate=%f channel=%d amp=%f\n",
	      length,srate,channel,amp);
    
    if(length <= 0)
      fprintf(stderr,"The requested range is not present in the file, or the file has zero length.\n");
    else
      play_from_file(istr, length, &srate, channel, amp, ehead);
    fclose(istr);
  } while (ac > ++optind);
  exit(0);
}


