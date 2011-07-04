/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1987, 1988, 1989 AT&T	*/
/*	  and Entropic Speech, Inc.		*/
/*	  All Rights Reserved.			*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	AND ENTROPIC SPEECH, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* addhead.c */
/* Prepend a minimal header to a generic VECTOR_SIGNAL file */

#ifndef lint
static char *sccs_id = "@(#)addhead.c	1.10	9/26/95	ATT/ERL";
#endif

#include <Objects.h>

int debug_level = 0, max_buff_bytes = 2000000, w_verbose = 0;

set_pvd(){}

#define BUFF_SIZE 100000
char iobuf[BUFF_SIZE];

main(argc, argv)
     int argc;
     char **argv;
{
  Signal *s, *si, *new_signal();
  Header *h, *w_new_header(), *w_write_header();
  int i, fd, length, fdj, dimensions, samples, type, den, data_from_DARPA,
  sams, c, nread;
  register int j,k;
  extern int optind;
  extern char *optarg;
  unsigned int s1, s2, s3, s4;
  register char *p, *q, tem;
  double freq, band, strt_time, end_time, amax, amin, dur, time, tread, chunk;
  short *data, **dpp;
  char *new_ext(), tiname[100], str[200], *tex;
  struct cmu {
    short size, version, channels, rate;
    unsigned char s1, s2, s3, s4;
  } cmu;

  if(argc < 2) {
    help_doc("addhead.help");
    exit(-1);
  }

  freq = -1.0;
  band = -1.0;
  strt_time = -1.0;
  end_time = -1.0;
  samples = -1;
  dimensions = -1;
  data_from_DARPA = 0;
  type = -4;
  tex = NULL;
  while((c = getopt(argc,argv,"f:b:s:e:n:d:t:T:")) != EOF) {
    switch(c) {
    case 'f':
      freq = atof(optarg);
      break;
    case 'b':
      band = atof(optarg);
      break;
    case 's':
      strt_time = atof(optarg);
      break;
    case 'e':
      end_time = atof(optarg);
      break;
    case 'n':
      samples = atoi(optarg);
      break;
    case 'd':
      dimensions = atoi(optarg);
      break;
    case 't':
      sscanf(optarg,"%x",&type);
      break;
    case 'T':			/* put arbitrary text in header! */
      /* NOTE: this does not work properly for null-terminated types (#cstr)! */
      if((tex = malloc(strlen(optarg) + 2)))
	sprintf(tex,"%s\n",optarg);
      else {
	printf("allocation failure\n");
	exit(-1);
      }
      break;
    }
  }
  for(i=optind; i < argc; i++) {
    if((si = get_signal(argv[i],0.0,0.0,NULL))) {
      printf("File %s already has a header; creating %s.d\n",argv[i],argv[i]);
      if((freq > 0.0) && (si->freq != freq))
	head_printf(si->header,"frequency",&freq);
      if((type > 0) && (type != si->type))
	head_printf(si->header,"type_code",&type);
      if((strt_time >= 0.0) && (strt_time != si->start_time))
	head_printf(si->header,"start_time",&strt_time);
      if((end_time >= 0.0) && (end_time != si->end_time))
	head_printf(si->header,"end_time",&end_time);
      if((dimensions > 0) && (dimensions != si->dim))
	head_printf(si->header,"dimensions",&dimensions);
      if((band > 0.0) && (band != si->band))
	head_printf(si->header,"bandwidth",&band);
      if(tex && *tex)
	head_append(si->header,tex,strlen(tex));
      if((fdj = open(new_ext(argv[i],"d"),O_RDWR|O_CREAT,0664)) &&
	 put_header(si->header,fdj)) {
	while((nread = read(si->file,iobuf,BUFF_SIZE)) > 0)
	  write(fdj,iobuf,nread);
	close(fdj);
	free_signal(si);
      } else {
	printf("Problems opening output file %d or in put_header()\n",fdj);
	exit(-1);
      }
      continue;
    } else {
      if(freq < 0.0) freq = 10000.0;
      if(type < 0) type = 4;
      if(strt_time < 0.0) strt_time = 0.0;
      if((end_time > 0.0) && (end_time <= strt_time)) end_time = 0.0;
      if(dimensions < 0) dimensions = 1;
      if(band < 0.0) band = freq/2.0;
      if((fd = open(argv[i],O_RDONLY)) >= 0) {
	read(fd,&cmu,sizeof(cmu));
	if(((cmu.size >> 8) == 6) && ((cmu.channels >> 8) == 1)) {
	  /* it looks like a byte-swapped CMU .adc header */
	  data_from_DARPA = 1;
	  s1 = cmu.s1;
	  s2 = cmu.s2;	
	  s3 = cmu.s3;
	  s4 = cmu.s4;
	  samples =  s1 + ((s2<<8)&0xff00) + ((s3<<16)&0xff0000) +
	    ((s4<<24)&0xff000000);
/*     printf("Looks like swabbed CMU... samples:%d s1:%d s2:%d s3:%d s4:%d\n",
		   samples,s1,s2,s3,s4); */
	  type = P_SHORTS;
	  freq = 4000000.0/(double)(((cmu.rate<<8)&0xff00)+((cmu.rate>>8)&0xff));
	  band = freq/2.0;
	  dimensions = 1;
	  strt_time = 0.0;
	} else
	  lseek(fd,0,0);
	if(IS_GENERIC(type)) {
	  if(samples == -1) {
	    if((type & SIG_FORMAT) == SIG_ASCII) {
	      printf("Number of samples must be specified for an ASCII file\n");
	      exit(-1);
	    }
	    /* try to infer number of samples from file size and type */
	    if((length = lseek(fd,0,2))  <= 0) {
	      printf("Zero length file or problems during seek\n");
	      exit(-1);
	    }
	    lseek(fd,0,0);
	    switch(type & VECTOR_SIGNALS) {
	    case P_CHARS:
	      den =  1; break;
	    case P_UCHARS:
	      den =  1; break;
	    case P_USHORTS:
	      den =  sizeof(short); break;
	    case P_SHORTS:
	      den =  sizeof(short); break;
	    case P_INTS:
	      den =  sizeof(int); break;
	    case P_UINTS:
	      den =   sizeof(int); break;
	    case P_FLOATS:
	      den =   sizeof(float); break;
	    case P_DOUBLES:
	      den =   sizeof(double); break;
	    default:
	      printf("Unknown type specified (%x)\n",type);
	      exit(-1);
	    }
	    sams = length/(dimensions * den);
	  }
	  if(samples > 1) sams = samples;
	  if((s = new_signal(new_ext(argv[i],"d"), fd, NULL, NULL, sams,
			     freq, dimensions))) {
	    if(type & SIG_ASCII) chunk = ((double)sams)/freq;
	    else
	      chunk = 5.0;    /* can't seek into arbitrary-format ASCII files! */
	    s->band = band;
	    s->start_time = strt_time;
	    s->type = type;
	    if(data_from_DARPA)
	      s->bytes_skip = sizeof(cmu);
	    else
	      s->bytes_skip = 0;
	    /* Determine max and min values in file */
	    s->end_time = (dur = ((double)sams)/freq) + s->start_time;
	    for(time = strt_time; dur > 0.0; time += tread, dur -= tread) {
	      tread = (dur > chunk)? chunk : dur;
	      if(read_data(s,time,tread)) {
		if(data_from_DARPA) { /* byte-swap the data */
		  for(j=0, k = s->buff_size, p = *((char**)(s->data)),
		      q = p+1; j < k; j++, p += 2, q += 2) {
		    tem = *p;
		    *p = *q;
		    *q = tem;
		  }
		}
		get_maxmin(s);
		if(time == strt_time) {
		  amax = *s->smax;
		  amin = *s->smin;
		} else {
		  if(*s->smax > amax) amax = *s->smax;
		  if(*s->smin < amin) amin = *s->smin;
		}
	      } else {
		printf("Read_data(%s) problems during max-min scan\n",s->name);
		exit(-1);
	      }
	    }
	    *s->smax = amax;
	    *s->smin = amin;
	    /* Now prepend the header and transfer the data. */
	    if((s->header = w_write_header(s))) {
	      if((s->dim == 1) && (s->type == 4))
		head_ident(s->header,"PCM_signal");
	      if(data_from_DARPA) {
		head_printf(s->header,"remark", "Converted CMU header");
	      } else
		head_printf(s->header,"remark",
			    "Header independent of data creation");
	      if(tex && *tex)
		head_append(s->header,tex,strlen(tex));
	      if((fdj = open(s->name,O_RDWR|O_CREAT,0664)) &&
		 put_header(s->header,fdj)) {
		if(s->file < 0)	/* might have been closed by read_data ASCII */
		  s->file = open(argv[i],O_RDONLY);
		lseek(s->file,s->bytes_skip,L_SET);
		while((nread = read(s->file,iobuf,BUFF_SIZE)) > 0) {
		  if(data_from_DARPA) {	/* byte-swap the data */
		    for(j=0, k = nread/2, p = iobuf,
			q = p+1; j < k; j++, p += 2, q += 2) {
		      tem = *p;
		      *p = *q;
		      *q = tem;
		    }
		  }
		  write(fdj,iobuf,nread);
		}
		close(fdj);
		free_signal(s);
	      } else {
		printf("Problems opening %s or outputting its header.\n",
		       s->name);
		exit(-1);
	      }
	    } else {
	      printf("Couldn't w_write_header(%d)\n", s);
	      exit(-1);
	    }
	  } else printf("new_signal failed\n");
	} else printf("Data must be a generic VECTOR_SIGNAL for addhead!\n");
      } else printf("File open on %s failed\n",argv[i]);
    }
  }
}

/*      ----------------------------------------------------------      */
char *
new_ext(oldn,newex)
char *oldn, *newex;
{
  static char newn[256];
  static int len = 0;


  if((strlen(oldn)+strlen(newex)+1) > 255) return(NULL);
  sprintf(newn,"%s.%s",oldn,newex);
  return(newn);

}



