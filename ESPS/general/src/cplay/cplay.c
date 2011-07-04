/*

  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
               "Copyright 1986,1987 Entropic Speech, Inc."
 				
 	
 									     
 Joseph T. Buck, Entropic Processing, Inc.				    
 Modified by Alan Parker to use DACP on Masscomp at WRL 2/13/86
 Modified for ESPS by Alan Parker, Entropic Speech, Inc.
 Modified by Joe Buck to play previous file while reading next one.
 Modified by Dale Lancaster, Convex Computer Corp to run on Convex Machines.

 This program is used to play a SD file on the Masscomp D/A. 
 							
						
 Syntax: mcplay [options] file		
   Options:			

	-x debug_level	

	-g gain	
		gain is a floating value. The file is multiplied by this    
		value to form the output data.
	-p range							  
		range is a range of points. Only those points are played 
		the first point is 1					
	-f range						
		range is a range of frames (default frame size = 100)	
		the first frame is 1				
	-w width					
		changes the frame size		
	-s range			
		range is a range of seconds. For LISTEN compatibility, the
		first second is zero					 
	-r repeats							
		number of repetitions				
	-c chan						
		channel number			
	
#ifdef CONVEX
	-k clock
		D/A clock number
#endif CONVEX

	-i
		Clip samples
	-h hist-file
		Specify alternate history file.
	-b
		bits to shift input data to form output data
		postive means right shift, negative means left shift

*/

#ifndef lint
	static char *sccs_id = "%W% %G%	ESI";
#endif


/* **************** check these defaults ********************** */

#ifdef  CONVEX
#define DADEV "/dev/ad0"
#define DA0DEV "/dev/ad0"   /* Play to DA port 0 on RTI */
#define DA1DEV "/dev/ad1"   /* Play to DA port 1 on RTI */
#define DATYPE "RTI-732"
#else
#define DADEV "/dev/dacp0/daf0"		/* default d to a device */
#endif

#define CLKDEV "/dev/dacp0/efclk"	/* default clock device */
#define CLKDEFAULT	0		/* default clock number */

#ifndef MAXDA
#define MAXDA 2048		/* max value of this D/A (12 bit) */
#endif

/* ************************************************************ */

#define NUM_BUFS 1
#define DUTY 50			/* duty cycle of clock */

#include <stdio.h>

#ifdef CONVEX
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include </sys/dev_iop/adcommon.h>
extern int errno ;
#endif
/* include ESPS header files */
#include <esps/esps.h>			/* main header stuff */
#include <esps/sd.h>			/* SD file stuff */
#include <esps/unix.h>			/* Unix function declarations */

#ifdef CONVEX
#define SYNTAX USAGE \
("play [-[spf] range] [-w fsiz] [-c chan] [-r nrpt] [-g gain] [-i] [-h hist-file] [-x debug-level] [-R sample-rate] file ...");
#else
#define SYNTAX USAGE \
("play [-[spf] range] [-w fsiz] [-c chan] [-k clock] [-r nrpt] [-g gain] [-i] [-h hist-file] [-x debug-level] file ...");
#endif

/* Globals */
int     debug_level = 0;

/* Externals */
void lrange_switch();
extern  int optind;
extern  char *optarg;
double	atof();
int     mropen(),  mrclktrig();
char    *getsym_s();
void    dac_error();
char	*eopen();

main (argc, argv)
int argc;
char **argv;
{
#ifdef CONVEX
    int ioctl_data ;
    int sam_rate ;
#endif
    int     c, i;
    int     nrep = 1, channel = 0;
    char    *p_switch = NULL, *s_switch = NULL, *f_switch = NULL;
    FILE    *istrm, *hfile;
    char    *hist = "play.his";
    struct  header  *ih;
    long    start_p = 1, end_p, start_s, end_s, start_f, end_f;
    int     isf, np=0, fwidth = 100, gflag = 0;
    int	    bflag=0, bits, left, right;
    int     nptot = 0, clk = CLKDEFAULT, nofile=0;
    int     max = 0, clip = 0, iflag = 0;
    float   *dataf;
    short   *data = NULL;
    int     argind, devpn = -1, clkpn = -1, write_hfile=0;
    double  retwidth, retfreq, clk_width, gain=1;
    char    dadev[30], clkdev[30], *filename=NULL, *firstfile;
    int     filecount=0;
    int	    in_progress = 0, prev_np = 0;

    (void)sprintf(dadev,DADEV); 
#ifndef CONVEX
    (void)sprintf(clkdev,"%s%d",CLKDEV,clk);
#endif

    while ((c = getopt (argc, argv, "p:s:r:c:k:w:f:g:x:ih:b:C:D:R:")) != EOF) {
	switch (c) {
	    case 'x':
		debug_level = atoi (optarg);
		break;
	    case 'g':
	        gflag++;
		gain = atof (optarg);
		break;
	    case 'c': 
    		channel = atoi (optarg);
#ifdef CONVEX
	        /* Use DA0 or DA1 ?  */
	        if (channel == 0)
		     strcpy(dadev,DA0DEV) ;
	        else strcpy(dadev,DA1DEV) ;
#endif
		break;
	    case 'r': 
		nrep = atoi (optarg);
		break;
	    case 'p': 
		p_switch = optarg;
		break;
	    case 'f':
		f_switch = optarg;
		break;
	    case 'w':
		fwidth = atoi (optarg);
		break;
	    case 's': 
		s_switch = optarg;
		break;
#ifndef CONVEX
	    case 'k':
		(void)sprintf(clkdev, "%s%s", CLKDEV, optarg);
		break;
#endif
	    case 'i':
		iflag++;
		break;
	    case 'h':
		hist = optarg;
		break;
	    case 'b':
		bflag++;
		bits = atoi(optarg);
		break;
	    case 'C':
		(void)strncpy(clkdev,optarg,sizeof clkdev);
		break;
	    case 'D':
		(void)strncpy(dadev,optarg,sizeof dadev);
		break;
#ifdef CONVEX
	    case 'R':
		sam_rate = atoi(optarg) ;
		break ;
#endif
	    default: 
		SYNTAX;
	}
    }

    if (bflag && gflag) {
	Fprintf(stderr,"play: can't use both -b and -g\n");
	exit(1);
    }

    if (bflag)
      if (bits < 0) left = bits*-1;
	else right = bits;

    if (optind >= argc)
      nofile = 1;
    else
      filename = argv[optind];

    (void)read_params(NULL,SC_CHECK_FILE,filename);
    if (nofile) {
      if(symtype("filename") == ST_UNDEF) {
	Fprintf(stderr,"play: no input file\n");
	SYNTAX;
      }
      filename = getsym_s("filename");
    }
    firstfile = filename;

    if ((hfile = fopen(hist,"w")) == NULL) {
	Fprintf(stderr,"play: warning can't open hist file.\n");
 	hfile = stderr;
    }

#ifdef CONVEX
    if(debug_level) Fprintf(stderr,"play: %s, d/a: %s\n",dadev);
#else
    if(debug_level) Fprintf(stderr,"play: clk: %s, d/a: %s\n",clkdev,dadev);
#endif
    argind = optind;

    while (nrep > 0) {
    optind = argind;
    while (optind < argc || nofile) {
	filecount++;
	if (!nofile) filename = argv[optind];
/* open the input file */
	filename = eopen("play", filename, "r", FT_SD, NONE, &ih, &istrm);

/* if input is a pipe (ndrec is -1) then assume 40000 points of data */
	if (ih->common.ndrec == -1)
	   ih->common.ndrec = 40000;

	end_p = ih -> common.ndrec;
	isf = ih -> hd.sd->sf;
	start_s = (start_p - 1) / isf;
	end_s = (end_p + isf - 1) / isf;
	start_f = (start_p - 1) / fwidth + 1;
	end_f = (end_p + fwidth - 1) / fwidth;

	if(symtype("start") != ST_UNDEF)
	   start_p = getsym_i("start");
	if(symtype("nan") != ST_UNDEF)
	   end_p = start_p + getsym_i("nan") -  1;

	if (p_switch)
	  lrange_switch (p_switch, &start_p, &end_p, 1);
	 else if (s_switch) {
	   lrange_switch (s_switch, &start_s, &end_s, 1);
	   start_p = start_s * isf + 1;
	   end_p = end_s * isf;
	   if (end_p <= start_p) end_p += isf;
	 }
	 else if (f_switch) {
	   lrange_switch (f_switch, &start_f, &end_f, 1);
	   start_p = (start_f - 1) * fwidth + 1;
	   end_p = end_f * fwidth;
	 }

	np = end_p - start_p + 1;

	if(start_p > ih->common.ndrec) {
	  Fprintf (stderr, "play: start point (%ld) > number of points (%ld)\n",
	  		start_p, ih -> common.ndrec);
	  exit(1);
	}

	if (start_p > end_p) {
	  Fprintf (stderr, "play: start point (%ld) > end point (%ld)\n",
		   start_p, end_p);
	  exit (1);
	}

	if (end_p > ih -> common.ndrec) {
	  Fprintf (stderr, "play: end point (%ld) > number of points (%ld)",
		   end_p, ih->common.ndrec);
	  Fprintf (stderr, "play: end point reset to last point.\n");
	  end_p = ih->common.ndrec;
	}

	Fprintf(stderr,"play: range (points) %ld:%ld, filename: %s\n",
	 start_p,np,filename);
	
	skiprec (istrm, start_p - 1, size_rec (ih));

#ifndef CONVEX
	clk_width = (double)((1.06e6/isf)*(DUTY/100.0));
#endif
	if(debug_level) Fprintf(stderr,"play: np: %ld\n",np);
	if((dataf = (float *)malloc((unsigned)np * sizeof FLOAT)) == NULL) {
	    Fprintf(stderr,"Cannot alloc %ld floats\n",np);
	    exit(1);
	}

	if ((ih->hd.sd->max_value == 0) && iflag) {
	    clip++; write_hfile=1;
	    Fprintf(hfile,
             "Samples greater than %d will be clipped to this value\n",
	      MAXDA);
	    Fprintf(hfile,"Sample\toriginal value\n");
	}

	if (!gflag && !bflag && (ih->hd.sd->max_value > MAXDA)) {
	    gflag++;  write_hfile=1;
	    gain = (float)MAXDA/ih->hd.sd->max_value;
	    Fprintf(hfile,"hd.sd->max_value: %g\n",ih->hd.sd->max_value);
	    Fprintf(hfile,"This D/A MAXDA is %d\n",MAXDA);
	    Fprintf(hfile,"Data scaled by %g\n",gain);
	    Fprintf(stderr,"play: Data scaled by %g\n",gain);
	}
	if ((i = get_sd_recf(dataf,np,ih,istrm)) != np) {
	    if (i == 0) {
		Fprintf (stderr, "play: %s is empty!\n");
		exit (1);
	    }
	    else {
		Fprintf (stderr, "play: only %d points in %s\n", i, filename);
		np = i;
	    }
	}
	(void) fclose (istrm);
	if (clip) for (i=0; i<np; i++) {
	    int j = abs((int)dataf[i]);
	    if (j > MAXDA) {
		Fprintf(hfile, "%d\t%d\n", i+2,j);
		if(j > max) max = j;
		if(dataf[i] < 0) dataf[i] = -MAXDA;
		 else dataf[i] = MAXDA;
	    }
	}

#ifndef CONVEX
/* Wait for previous play to complete.  Then allocate old array. */
	if (in_progress) {
	    wait_for_end(devpn,(int)(2000.0*prev_np/isf),data);
	    Fprintf(stderr,"previous transfer complete\n");
	}
	prev_np = np;
/* Re-open D/A */
	if(mropen(&devpn,dadev,0) == -1) {
	    Fprintf(stderr,"Can't open D/A %s\n",dadev);
	    exit(1);
        }
	if(mropen(&clkpn,clkdev,0) == -1) {
	    Fprintf(stderr,"Can't open clock %s\n",clkdev);
	    exit(1);
	}

	if(mrclk1(clkpn,0,(double)isf,&retfreq,0,clk_width,&retwidth,0) != 0)
    	 dac_error();
	if(mrclktrig(devpn,1,clkpn) != 0) dac_error();
#else
#ifdef CONVEX
    devpn = open (dadev, O_RDWR); 
#endif

    if (debug_level > 0) {
	Fprintf (stderr,"%s opened, pathno = %d, errno = %d\n", 
		 dadev, devpn,errno);
    }

    /* Make sure that the "software clock" is set to the proper value */
    ioctl_data = DEFAULT_DELAY ;
    ioctl(devpn,ADSETDELAY,&ioctl_data) ;
#endif

/* Allocate short buffer */
	if((data = (short *)malloc((unsigned)np*sizeof SHORT)) == NULL) {
	    Fprintf(stderr,"Cannot alloc %ld shorts\n",np);
	    exit(1);
	}

/* either, 1) multiply sample by gain factor (slow), 2) shift sample
   bits to right (faster), or 3) just copy sample (fastest)
 */
	if (gflag) for (i=0; i<np; i++) data[i] = dataf[i]*gain;
	 else 
	  if (bflag && left) 
	   for (i=0; i<np; i++) data[i] = (int)dataf[i] << left;
	    else
	     if (bflag && right)
	      for (i=0; i<np; i++) data[i] = (int)dataf[i] >> right;
	       else
	    	for (i=0; i<np; i++) data[i] = dataf[i];

#ifdef CONVEX
    if (debug_level) {
	/* Turn on level 1 debug messages inside ad driver */
	ioctl_data = 1 ;
	ioctl(devpn,ADDEBUG,&ioctl_data) ;
    } ;

	ioctl_data = gain ;
    ioctl(devpn,ADSETGAIN,&ioctl_data) ;
	ioctl_data = sam_rate ;
    ioctl(devpn,ADSETHZ,&ioctl_data) ;

    /* Adjust the data by shifting it 4 bits left */
    if (strcmp(DATYPE,"RTI-732") == 0) {
	int i ;
	fprintf(stderr,"adjusting data...%d\n",np) ;
	for (i=0; i < np; i++)
	    data[i] = data[i] << 4 ;
    }

    write(devpn,data,np*2) ;
    close(devpn) ;
#else
	(void)mrdaxout(devpn,clkpn,clkpn,channel,1,1,np,data,0);

        Fprintf(stderr,"play: Transfer started\n");
#endif
        (void) free((char *)dataf);
	in_progress = 1;
        nptot += np;
	if(nofile) break;
        optind++;
    }
    nrep--;
    }

#ifndef CONVEX
    wait_for_end(devpn,(np/isf)*2000,data);
#endif
    Fprintf (stderr, "play: final transfer complete\n");

/* write common file, unless the input was from a pipe or there
   was more than one input file
*/
    if(filecount == 1 && strcmp(filename, "<stdin>") != 0) {
    	(void) putsym_i("start",(int)start_p);
    	(void) putsym_i("nan",(int)np);
    	(void) putsym_s("filename",firstfile);
    	(void) putsym_s("prog","play");
    }

    if(debug_level) Fprintf(stderr,"play: Total points output: %ld\n",nptot);
    if(clip && max) Fprintf(hfile,"To avoid clipping use a gain of %g.\n",
	(float)MAXDA/(float)abs(max));
    if(clip && max && (hfile != stderr)) 
	Fprintf(stderr,"play: To avoid clipping use a gain of %g.\n",
	(float)MAXDA/(float)abs(max));
    (void) fclose(hfile); 
    if(!write_hfile && (hfile != stderr)) (void)unlink(hist);
    return 0;
}

void dac_error()
{
    Fprintf(stderr,"Error on dacp call.\n");
    exit(1);
}

wait_for_end (devpn,arg_wt,data)
int devpn, arg_wt;
short *data;
{
#ifndef CONVEX
    (void) mrevwt(devpn,0,arg_wt);
    if (data != NULL)
	(void) free((char *)data);
    if(mrclosall() != 0) {
	Fprintf (stderr, "Error from mrclosall call.\n");
	exit (1);
    }
#endif
}

