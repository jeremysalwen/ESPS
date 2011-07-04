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

static char *sccs_id = "%W%	%G%	ERL";


#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <signal.h>
#include <esps/esps.h>
#include <esps/epaths.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/ss.h>
#include <Objects.h>

#define SYNTAX USAGE("splay [-x debug-level] [-{ps} range] [-g gain] [-R count] [-b shift-value] [-a] [-e] file [more-files]")

#define SPARC_SAMPLE_FREQ 8000
#define REALLY_HUGE_INTEGER 1000000000
#define DEBUG 0

int debug_level = 0, max_buff_bytes = 2000000, w_verbose = 0;
int da_location = 0;                    /* Waves variable */
int gflag = 0, bflag =0, aflag =0;      /* gain, shift, auto-scaling flag */
float gain;                             /* command-line gain*/
unsigned int gain_codec = 0;            /* codec gain */
int gain_codecflag = 0;
int left= 0; right = 0;                 /* left and right shifts */
int external = 0;                       /* external jack */
short get_fea_type();

main(ac,av )
     int ac;
     char **av;
{
  extern int optind;
  extern char *optarg;

  int ifd = 0;                          /* input file descriptor */
  char *ifile = NULL;                    /* input file name */
  int doseek = 0;                       /* samples to skip for istr */
  int sizeof_sample = sizeof(short);    /* sample size */
  int amp;                              /* signal amplitude */
  int length;                           /* file length */
  int noheader = FALSE;                 /* esps header signal */
  double srate = 8000;                   /* sampling rate */
  double sstart, start = -1.0, end = -1.0, tmp;  /* start/end time */
  long start_p = 1, end_p;               /* start/end sample point */
  char *r_switch = NULL, *s_switch = NULL; /* range switch */
  int fnum;
  int repeats = 1;
  int bits;
  int c;
  int common_range = 0;
  int espsfile = 0;
  int qflag = 0;
  int type;

#if !DEBUG
  while((c = getopt(ac,av,"p:s:r:R:g:x:b:qae?")) != EOF) 
    {
      switch(c) 
	{
	case 'x':
	  debug_level = atoi(optarg);
	  break;
	case 'g':
	  gain = atof(optarg);
	  gflag++;
	  break;
	case 'R':
	  repeats = atoi(optarg);
	  break;
	case 'r':
	case 'p':
	  if(s_switch){
	    fprintf(stderr,"Cannot use -r/-p with -s\n"); exit(1);}
	  r_switch = optarg;
	  break;
	case 's':
	  if(r_switch){
	    fprintf(stderr,"Cannot use -s with -r/-p\n"); exit(1);}
	  s_switch = optarg;
	  break;
	case 'e':
	  external = 1;
	  break;
	case 'a':
	  aflag++;
	  break;
	case 'b':
	  bits = atoi(optarg);
	  bflag++;
	  break;
	case 'q':
	  qflag++;
	  break;
	case '?':
	default:
	  SYNTAX;
	  break;
	}
    }
#endif

#if DEBUG
  debug_level = 3;
  fprintf(stderr,"debug_level %d\n",debug_level);
  ac = 1;
  optind = 1;
  external = 1;
  bflag = 1;
  bits = 3;
#endif

#if !DEBUG
  if( optind >= ac )
    {
      (void) read_params((char*)NULL, SC_CHECK_FILE, (char*)NULL);
      if(symtype("filename") == ST_UNDEF){
	fprintf(stderr,"%s: no input file\n", av[0]);
	SYNTAX;      /* exit */
      }
      ifile = getsym_s("filename");
      if(symtype("start") != ST_UNDEF || symtype("nan")!= ST_UNDEF)
	common_range = 1;
    }
  if(!external && getenv("SPLAY_X")) external = atoi(getenv("SPLAY_X"));
  if(getenv("SPLAY_GAIN")) {
    gain_codecflag++;
    gain_codec = atoi(getenv("SPLAY_GAIN"));
  }
#endif

  do {
    fnum = optind;
    do {				/* for all input specifications... */
      Header *h;                         /* SIG file header */
      Signal *sig;                       /* SIG file type */
      FILE *istr = stdin;                /* file stream */
      int hseek = 0;                     /* header bytes to skip */
      struct header *ehead = NULL;       /* esps header */
      
      if(ac > fnum)
	ifile = av[fnum];

      if(ifile && !strcmp(ifile,"-"))
	{
	  if(repeats > 1){
	    fprintf(stderr,"Can't replay for stdin file\n");
	    exit(1);
	  }
	  ifile = NULL;
	  ifd = 0;
	  istr = stdin;
	}
      if(ifile && *ifile)
	if((ifd = open(ifile, O_RDONLY)) < 0) 
	  {
	    fprintf(stderr,"Can't open %s for input.\n",ifile);
	    exit(-1);
	  }
      if(ifd > 0)			/* i.e. input is not stdin */
	if(!(istr = fdopen(ifd,"r"))) 
	  {
	    fprintf(stderr,"Can't open a stream from %d\n",ifd);
	    exit(-1);
	  }
      
      if(!noheader && (h = (Header*)get_header(ifd)) &&
	 (sig = (Signal*)new_signal(ifile,ifd,h,NULL,0,0.0,0))) 
	{
	  /* Input is ESPS file */
	  if(h->magic == ESPS_MAGIC)
	    {
	      espsfile = 1;
	      *(sig->smax) = *(sig->smin) = get_genhd_val("max_value",h->esps_hdr,0.0);
	      istr = h->strm;
	      hseek = h->esps_nbytes;
	      if (h->esps_hdr->common.type == FT_SD) 
		ehead = (struct header*)sdtofea(h->esps_hdr);
	      else
		ehead = h->esps_hdr;
	      
 	      switch(get_fea_type("samples", ehead)) {
	      case BYTE:
		sizeof_sample = sizeof(char);
		break;
	      case SHORT:
		sizeof_sample = sizeof(short);
		break;
	      case CHAR:
		sizeof_sample = sizeof(char);
		break;
	      case FLOAT:
		sizeof_sample = sizeof(float);
		break;
	      case DOUBLE:
		sizeof_sample = sizeof(double);
		break;
	      default:
		fprintf(stderr,"Unknown data type -- exiting\n");
		exit(1);
	      }
	      srate = *(double *) get_genhd_d("record_freq",ehead);
	      if( 1 != get_fea_siz("samples", ehead, (short *) NULL,
				   (long **) NULL)){
		fprintf(stderr,"Multichannel data not supported.\n");
		exit(1);
	      }
	      if( is_field_complex(ehead,"samples") == YES){
		fprintf(stderr,"Complex data not supported\n");
		exit(1);
	      }
	    }
	  else 
	    {		/* might be a SIGnal file... */
	      if((type_of_signal(sig) & VECTOR_SIGNALS) != P_SHORTS)
		{
		  fprintf(stderr,"Can't play signals of type 0x%x %d\n",
			  type_of_signal(sig));
		  exit(-1);
		}
	      hseek = sig->bytes_skip;
	      srate = sig->freq;
	      sizeof_sample = sizeof(short);
	    }

	  /* Position file stream to data */
	  if(ifd)
	    {
	      if(debug_level)
		fprintf(stderr,"Seeking in header setup\n");
	      fseek(istr, hseek, 0);
	    }
	  
	  if((amp = ((*sig->smax > -(*sig->smin))? *sig->smax : - *sig->smin))
	     < 128) amp = 32767;
	  
	  if((length = sig->file_size) <= 0) 
	    {
	      if(debug_level)
		fprintf(stderr,"No length info available in header.\n");
	      length = REALLY_HUGE_INTEGER;
	    }
	  
	  if (bflag)
	    if (bits < 0) left = bits*-1;
	    else right = bits;
	  
	  if (r_switch) 
	    {
	      end_p = length;
	      lrange_switch (r_switch, &start_p, &end_p, 1);
	      if (end_p > length){
		end_p = length;
		if(!qflag){
		  fprintf(stderr,"%s: end point (%d) > signal length\n",
			  av[0], end_p);
		  fprintf(stderr,"%s: end point reset to last point.\n");
		}
	      }
	      if(!end_p && length) 
		{ /* end not specified; play to end of file */
		end_p = length;
		length -= start_p;
	      }
	      length = end_p - start_p + 1;
	      if(debug_level)
		fprintf(stderr,"Range specified (-r); start_p:%d end_p:%d length:%d\n",
			start_p,end_p,length);
	      doseek = start_p-1;

	    } else if (s_switch){ 
	      frange_switch(s_switch, &start, &end);
	      if(((end < 0.0) || (end > start)) && (start > sig->start_time)) 
		{
		  doseek = 0.5 + (start - sig->start_time)*srate;
		  sstart = start;
		} else {
		  sstart = sig->start_time;
		  doseek = 0;
		}
	      length -= doseek;
	      if((end > sig->start_time) && (end > start) &&
		 (end < (tmp = (sig->start_time +
				((double)sig->file_size)/srate))))
		length -= ((tmp - end)*srate);
	    } else if(common_range){
	      if(symtype("start")!=ST_UNDEF)
		start_p = getsym_i("start");
	      if(symtype("nan")!=ST_UNDEF)
		end_p = start_p + getsym_i("nan");
	      if (end_p > length)
		end_p = length;
	      if(!end_p && length) 
		{ /* end not specified; play to end of file */
		end_p = length;
		length -= start_p;
	      }
	      length = end_p - start_p + 1;
	      if(debug_level)
		fprintf(stderr,"Common range specified: start_p:%d end_p:%d length:%d\n",
			start_p,end_p,length);
	      doseek = start_p-1;
	    }
	}
      else 
	{			/* Headerless file */
	  if(ifd) 
	    {	/* Is the input from a pipe? */
	      if(debug_level)
		fprintf(stderr,"Seeking after header setup\n");
	      fseek(istr,0,2);
	      if((length = ftell(istr)) > 10) {
		fseek(istr,0,0);
		length /= sizeof_sample;
		if((start>0.0)&&((start*srate)<length)) {
		  doseek = start*srate;
		  sstart = start;
		} else {
		  doseek = 0;
		  sstart = 0.0;
		}
		if((end > start) && ((end*srate) < length))
		  length = end*srate;
		length -= doseek;
		amp = 32767;
	      } else
		printf("Unreasonably small file (%s)\n",ifile);
	    } else {	      /* Can't actually seek or determine file size. */
	      if(start > 0.0) {
		char dummy[10000];
		int toskip = (0.5 + (start*srate))*sizeof_sample, skipped,
		da_location = (0.5 + (start*srate));
		
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
	  fprintf(stderr,"Seeking to sample # %d; sizeof_sample:%d\n",
		  doseek,sizeof_sample);
	da_location = doseek;
	fseek(istr, doseek*sizeof_sample, 1);
      }
      
      if(debug_level)
	fprintf(stderr,
		"Calling play_from_file. length=%d srate=%f amp=%d\n",
		length,srate,amp);
      if(length <= 0){
	fprintf(stderr,"The requested range is not present in the file, or the file has zero length.\n");
	exit(-1);
      }
      if( srate != SPARC_SAMPLE_FREQ && debug_level > 0 )
	fprintf(stderr,"%s: WARNING: sampling freq in %s != %d\n",
		av[0], ifile, SPARC_SAMPLE_FREQ);

      play_from_file(istr, length, &srate, amp, ehead,
		     (optind >= ac)? repeats : 1);
      fclose(istr);
    } while (ac > ++fnum);
  } while (--repeats && optind < ac);

  /* Write common file if it isn't standard input and it's esps file*/
  if(ifd !=0 && espsfile){
    (void) putsym_i("start", (int) start_p);
    (void) putsym_i("nan", (int) length);
    (void) putsym_s("filename", ifile);
    (void) putsym_s("prog", "splay");
    }
}


char    *ProgName = "splay";
char    *Version = "2.0";
char    *Date = "10/10/92";

void
set_pvd(hdr)
    struct header   *hdr;
{
    (void) strcpy(hdr->common.prog, ProgName);
    (void) strcpy(hdr->common.vers, Version);
    (void) strcpy(hdr->common.progdate, Date);

  }



