/*	Copyright (c) 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* record.c */
/* A/D to a file */

#ifndef lint
static char *sccs_id = "@(#)record.c	1.7 1/13/93 ERL/ATT";
#endif

#include <Objects.h>
#include <esps/esps.h>


#define TRUE 1
#define FALSE 0
#ifdef USE_DSP32C_PC
#define DEFAULT_SAMPLE_FREQ 8000.0
#else
#define DEFAULT_SAMPLE_FREQ 16000.0
#endif
#define DEFAULT_MAX_TIME 100.0


int debug = 0, ARIEL_HK = 0;
extern int use_dsp32;
#ifdef USE_DSP32C_VME
extern int channel;
#else
int channel=1;
#endif
int vox_do = 0,
    vox_sec = 0,
    vox_thr = 0,
    ivox_do = 0,		/* mcb: 2 Feb 90 */
    ivox_sec = 0,
    ivox_thr = 0,
    do_echo = FALSE,
    do_beep = FALSE;
double start_time = 0;
struct header *esps_hdr;
#ifdef USE_DSP32C_PC
char *dspdev = "/dev/ds0";
#else
extern char *dspdev;
#endif

void synopsis(progname)
	char *progname;
{
	char *pname = strrchr(progname,'/');
#ifndef USE_DSP32C_PC
    char *pargs = "[-f<freq>] [-d<dur>] [-e] [-r (or) -s] [-x<dev>] <output_file>";
#else
    char *pargs = "[-f<freq>] [-d<dur>]  <output_file>";
#endif
    
    if (pname++) progname = pname;
    printf("\nUsage: %s %s\n\n",progname,pargs);
    printf("  freq is the desired sampling frequency in Hz; default is %f;\n",
	   DEFAULT_SAMPLE_FREQ);
    printf("  dur is the maximum duration(sec) requested, default is %f.\n",
	   DEFAULT_MAX_TIME);
    printf("  '-e' causes the A/D values to be echoed to the D/A.\n");
#ifndef USE_DSP32C_PC
    printf("  dev is a dsp device; default is %s.\n",dspdev);
#endif
#ifdef USE_DSP32C_VME
    printf("  '-r' causes right channel to be stored.  Default is left channel\n");
    printf("  '-s' causes both channels (stereo) to be stored.\n");
#endif	
    printf("  Hit your interrupt key to stop recording early.\n");
    printf("  The absolute maximum before clipping is 32767.\n\n");
    exit(1);
}

main(argc,argv)
int argc; 
char *argv[];
{
  int i, fd, n, c;
  int errflg = 0;
  char prg[80];
  extern int optind;
  extern char *optarg;
  int readid, length, amax, amin, nsamps;
  static short *p;
  double atof(), t, fr;
  Signal *s;
  Header *h;
  char *thrp;
  char *strchr();

  fr = DEFAULT_SAMPLE_FREQ;
  t = DEFAULT_MAX_TIME;
  channel = 1;

  if ( argc < 2 ) errflg++;

  while((c = getopt(argc,argv,"Debv:i:f:d:x:rs")) != EOF) {
    switch(c) {
    case 'b':
      do_beep = TRUE;
      break;
    case 'e':
      do_echo = TRUE;
      break;
    case 'D':
      debug = 1;
      break;
    case 'f':
      fr = atof(optarg);
      break;
    case 'd':
      t = atof(optarg);
      break;
#ifdef USE_DSP32C_VME
    case 's':
      channel = 0;
      break;
    case 'r':
      channel = 2;
      break;
#endif
    case 'v':
      vox_do = 1;
      vox_sec = atoi(optarg);
      if (thrp=strchr(optarg,',')) vox_thr=atoi(thrp+1);
      else fprintf(stderr,"using default VOX threshold %d\n",vox_thr);
      break;
    case 'i':
      ivox_do = 1;
      ivox_sec = atoi(optarg);
      if (thrp=strchr(optarg,',')) ivox_thr=atoi(thrp+1);
      else fprintf(stderr,"using default VOX threshold %d\n",ivox_thr);
      break;
    case 'x':
      dspdev = optarg;
      break;
    case '?':
      errflg++;
      break;
    }
  }
  if (errflg)
      synopsis(argv[0]);

  if (! dsp32c_is_available()) {
      fprintf(stderr,"cannot open dsp board %s\n",dspdev);
      exit(1);
  }
  for(i=optind; i < argc; i++) {
    if(fr < 30.0) fr *= 1000.0;	/* assume user meant kHz! */
    n = fr * t;
    if ( (fd=open(argv[i],O_RDWR|O_TRUNC|O_CREAT,0666)) < 0 ) {
      fprintf(stderr,"Can't open %s for output\n",argv[i]);
      exit(1);
    }
    if((s = (Signal*)new_signal(argv[i],fd,NULL,NULL,100,fr,(channel)?1:2))&&
       (h = (Header*)w_write_header(s))) {
      char dline[300];
      int size0, diff;

      strcpy(dline,"record: padding xxxxxxxxxxxxxxxxxxxxxxxx");

      size0 = h->nbytes - h->npad;
      /* create a dummy header with enough room for expansion */
      head_printf(h,"operation",dline);
/*
      put_header(h,fd);		
*/

      esps_hdr = new_header(FT_FEA);
      (void)init_feasd_hd(esps_hdr, SHORT, (channel)?1:2, &start_time, NO, fr);
      (void)write_header(esps_hdr, fdopen(fd, "w"));

      free(h->header);
      free(h);
      if((nsamps = adc_32C(fd,NULL,n,&fr,&amax,&amin,do_echo,do_beep)) > 0) {
	s->file_size = nsamps;
	*s->smax = amax;
	*s->smin = amin;
	s->freq = fr;
	s->end_time = ((double)nsamps)/s->freq;
	s->band = s->freq/2.0;
/*
	if((h = (Header*)w_write_header(s))) {
	  diff = h->nbytes - h->npad - size0;
	  dline[strlen(dline) - diff] = 0; 
	  head_printf(h,"operation",dline);
	  lseek(fd,0,0);
	  put_header(h,fd);
	  printf("maximum:%d  minimum:%d\n",amax,amin);
	} else
	  printf("Can't perform REAL w_write_header()\n");
*/
      } else
	printf("Problems in adc()\n");
      close(fd);
    } else
      printf("Either can't new_signal() or w_write_header()\n");
  }
}

int   w_verbose =0;
int   max_buff_bytes;
void   set_pvd();
int    debug_level=0;
int   P8574_type=0;
int   ARIEL_16=0;

