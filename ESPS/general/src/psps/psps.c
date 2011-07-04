/* psps - prints ESPS data files
 *
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * psps - prints an ESPS data file on standard output.   This verison by
 *        Alan Parker, based on the SDS version by Joe Buck.
 *      - Revised and checked for ESPS 3.0 by John Shore
 */


#include <stdio.h>
#include <esps/esps.h>
#include <esps/sd.h>
#include <esps/spec.h>
#include <esps/fea.h>
#include <esps/filt.h>
#include <esps/scbk.h>
#include <esps/unix.h>
#include <esps/feaspec.h>
#include <esps/feasd.h>
#include <esps/feafilt.h>

#ifdef ESI
#include <esps/ana.h>
#include <esps/pitch.h>
#include <esps/ros.h>
#endif ESI

#define HUGE 2000000000

#ifndef lint
	static char *sccs_id = "@(#)psps.c	3.16	8/31/95	ESI";
#endif

#define SYNTAX USAGE("psps [-aDghHlvx] [-r range] [-t range] [-f field_name] file")

#define ERROR_EXIT(text) {(void) fprintf(stderr, "psps: %s - exiting\n", text); exit(1);}

int	getopt();
static	char *myrealloc();

int	size_rec();
char	*file_typeX();
short	get_rec_len();
void	lrange_switch();

void	pr_full_header();
void	pr_history();
void	pr_sd_data();
void	pr_feasd_data();

#ifdef ESI
void	print_ana();
void	print_pitch();
void	print_ros();
#endif

void	print_common();
void	print_fea();
void	print_filt();
void	print_generic();
void	print_scbk();
void	print_sd();
void	print_spec();
void	print_txtcomment();
void	tab();

int	vflag = 0;  /* global needed in prnt_hdr.c (print_fea)*/
int	full = 0;   /* if true, print full header(s) */

extern char	*optarg;
extern int	optind;

	    	/* array of strings containing FEA fields */
char	**field_name = NULL;
int	fflag = 0;	/* FEA field name given if fflag set */
int	nfield = 0;	/* number of field names given */
int     eflag = 0;      /* causes EFILE and AFILE links to be followed */
int	Recursive = 0;	/* if true, recursively print all headers */
int     debug_level = 0;


main (argc, argv)
int argc;
char **argv;
{
    int		c;
    long	i;

    int		history = 0;	/* if true, print a history */
    int		nohead = 0;	/* if true, don't print header ' */
    int		nodata = 0;	/* -D given? */
    int		isfile = 0;	/* non-zero if input is not a pipe*/
    char	*range = NULL;	/* -r argument */
    char	*trange = NULL;  /* -t argument */

    char   *inp_file;		/* input file name */
    FILE   *istrm = stdin;

    struct header	*h;
    struct spec_data	*spec_rec;
    struct filt_data	*filt_rec;
    struct scbk_data	*scbk_rec;
    struct fea_data	*fea_rec;
    struct feaspec	*feaspec_rec;
    struct feasd	*feasd_rec;
    struct feafilt	*feafilt_rec;

#ifdef ESI
    struct ana_data	*ana_rec;
    struct pitch	*pit_rec;
    struct ros_data	*ros_rec;
#endif ESI

    long	srec = 1;	/* first record to print */
    long	erec = HUGE;	/* last record to print */

    long	stag = 0;	/* tag start */
    long	etag = LONG_MAX;

    double	*data;
    long	tag;
    long	rec_size;


    int		k = 0;
    int		tflag = 0;

    int		oflag = 0;
    int		gflag = 0;	/* print any ESPS file in generic format */
    short	type;		/* ESPS filt type */
    /*
     * process command line options
     */
    while ((c = getopt (argc, argv, "hlHaxDr:t:vgf:e")) != EOF) {
	switch (c) {
	    case 'h':
		history++;
		break;
	    case 'a':
		Recursive++;	/* fall through to case 'f' intentional */
	    case 'l':
		full++;
		break;
	    case 'H':
		nohead++;
		break;
	    case 'x':
		debug_level++;
		break;
	    case 'D':
		nodata++;
		break;
	    case 'r':
	        range = optarg;
		oflag++;
		break;
	    case 't':
		trange = optarg;
		tflag++;
		break;
	    case 'v':
		vflag++;
		full++;
		break;
	    case 'g':
		gflag++;
		break;
	    case 'f':
		nfield++;
		field_name = (char **) myrealloc ((char *) field_name,
				(nfield+1) * sizeof (char *));
		assert (field_name);
	        field_name[nfield-1] = optarg;
		field_name[nfield] = NULL;
		assert (field_name[nfield-1]);
		fflag++;
		break;
	    case 'e':
		eflag++;
		full++;
		break;
	    default:
		SYNTAX;
	}
    }

/* check for conflicting switches */

    if (nodata && range || nohead && full || history && full)
	ERROR_EXIT("conflicting switches");

    if (tflag && oflag)
	ERROR_EXIT("conflicting switches: -t and -r options");

    if (optind == argc)
	SYNTAX;
    /*
     * open input file
     */
    inp_file = argv[optind];
    if(strcmp(inp_file,"-") != 0) {
    	if ((istrm = fopen (inp_file, "r")) == NULL)
    	   CANTOPEN ("psps", inp_file);
    }
    else
      	inp_file = "<stdin>";

    if ((h = read_header (istrm)) == NULL) NOTSPS ("psps", inp_file);

    if (tflag && (h->common.tag == NO))
	ERROR_EXIT("Data is not tagged, cannot use -t option");

    if (fflag && (h->common.type != FT_FEA))
	ERROR_EXIT("Use the -f option with ESPS Feature files only");

    if (!fflag)
	nfield = 1;
    /*
     * check for file or pipe and process range
     */
    if (h->common.ndrec != -1) isfile++;
    srec = 1;
    erec = (isfile ? h->common.ndrec : LONG_MAX);
    lrange_switch (range, &srec, &erec, 1);
/*
 Process the t option arguments (range of tags)
*/
    if (tflag)
	lrange_switch (trange, &stag, &etag, 1);
    if (stag > etag)
	ERROR_EXIT("starting tag > ending tag with -t option");

    if (debug_level) {
	Fprintf(stderr, "psps: srec = %ld, erec = %ld\n", srec, erec);
	Fprintf(stderr, "psps: input is %s pipe\n", (isfile ? "not" : ""));
    }
    if (srec == 0)
	Fprintf(stderr,
	"psps: records start at 1, using range %ld:%ld\n",++srec,++erec);
    if(isfile && (srec > h->common.ndrec))
	Fprintf(stderr,"psps: WARNING - starting point > than h->ndrec\n");
    if(isfile && (erec > h->common.ndrec))
	Fprintf(stderr,"psps: WARNING - not enough records in file\n");
    /*
     * print header
     */
    if (full)
	/*
	 * full header for -v, -l, and -a options
         */
	pr_full_header (inp_file, h, Recursive, 0);
    else if (history)
	/*
         * -h option prints common portion of all subheaders
	 */
	pr_history (inp_file, h, 0);
    else if (!nohead) {
	/*
	 * -H suppresses header (except for common portion)
	 */	
	(void) printf("File: %s\n",inp_file);
	print_common(0,h);
    }
    /*
     * exit if only the header is wanted
     */
    if (nodata) exit(0);

    fea_skiprec(istrm, srec - 1, h);

    /*
     * now print data records
     */
    if (gflag)
	type = -1;
    else
	type = h->common.type;

    switch (type) {
	case FT_SD:
	   pr_sd_data (istrm, srec, erec, h);
	   break;

#ifdef ESI
	case FT_ANA:
	   ana_rec = allo_ana_rec(h);
	   i = srec;
	   while (i++ <= erec && (get_ana_rec(ana_rec,h,istrm) != EOF))  {
		if (!tflag || (tflag && ana_rec->tag >= stag &&
				ana_rec->tag <= etag)) {
		   (void) printf("Record %d: ",i-1);
		   (void) print_ana_rec(ana_rec,h,stdout);
		}
	   }
	   break;

	case FT_PIT:
	   pit_rec = allo_pitch_rec();
	   i = srec;
	   while (i++ <= erec && (get_pitch_rec(pit_rec,istrm) != EOF)) {
		if (!tflag || (tflag && pit_rec->tag >= stag &&
				pit_rec->tag <= etag)) {
		   (void) printf("Record %d: ",i-1);
		   (void) print_pitch_rec(pit_rec,stdout);
		}
	   }
	   break;
#endif ESI

	case FT_SPEC:
	   spec_rec  = allo_spec_rec(h);
	   i = srec;
	   while (i++ <= erec && (get_spec_rec(spec_rec,h,istrm) != EOF)) {
		if (!tflag || (tflag && spec_rec->tag >= stag &&
				spec_rec->tag <= etag)) {
		   (void) printf("Record %d: ",i-1);
		   (void) print_spec_rec(spec_rec,h,stdout);
		}
	   }
	   break;

#ifdef ESI
	case FT_ROS:
	   ros_rec  = allo_ros_rec(h);
	   i = srec;
	   while (i++ <= erec && (get_ros_rec(ros_rec,h,istrm) != EOF)) {
		if (!tflag || (tflag && ros_rec->tag >= stag &&
				ros_rec->tag <= etag)) {
		   (void) printf("Record %d: ",i-1);
		   (void) print_ros_rec(ros_rec,h,stdout);
		}
	   }
	   break;
#endif ESI

	case FT_FILT:
	   filt_rec  = allo_filt_rec(h);
	   i = srec;
	   while (i++ <= erec && (get_filt_rec(filt_rec,h,istrm) != EOF)) {
		if (!tflag || (tflag && filt_rec->tag >= stag &&
				filt_rec->tag <= etag)) {
		   (void) printf("Record %d: ",i-1);
		   (void) print_filt_rec(filt_rec,h,stdout);
		}
	   }
	   break;

	case  FT_SCBK:
	   scbk_rec  = allo_scbk_rec(h);
	   i = srec;
	   while (i++ <= erec && (get_scbk_rec(scbk_rec,h,istrm) != EOF)) {
		if (tflag) {
		   ERROR_EXIT("No tags exist in Scalar Quantization Codebook");
		} else {
		   (void) printf("Record %d: ",i-1);
		   (void) print_scbk_rec(scbk_rec,h,stdout);
		}
	   }
	   break;

	case FT_FEA:
	   switch (h->hd.fea->fea_type) {
	    case FEA_SPEC:
	      feaspec_rec = allo_feaspec_rec(h, FLOAT);
	      i = srec;
	      while (i++ <= erec &&
		(get_feaspec_rec(feaspec_rec,h,istrm) != EOF)) {
		if (!tflag || (tflag && *feaspec_rec->tag >= stag &&
				*feaspec_rec->tag <= etag)) {
		   (void) printf("Record %d: ",i-1);
		   (void) print_feaspec_rec(feaspec_rec,h,stdout);
		}
	      }
	      break;
	    case FEA_FILT:
	      feafilt_rec = allo_feafilt_rec(h);
	      i = srec;
	      while (i++ <= erec &&
		(get_feafilt_rec(feafilt_rec,h,istrm) != EOF)) {
		(void) printf("Record %d: ",i-1);
		(void) print_feafilt_rec(feafilt_rec,h,stdout);
	      }
	      break;
	     case FEA_SD:
	      if(get_fea_siz("samples",h,(short *)NULL, (long **)NULL) > 1
		 || h->hd.fea->field_count > 1){
	        fea_rec = allo_fea_rec(h);
	        i = srec;
	        while (i++ <= erec && (get_fea_rec(fea_rec,h,istrm) != EOF)) {
		  if (!tflag || (tflag && fea_rec->tag >= stag &&
				  fea_rec->tag <= etag)) {
		     (void) printf("Record %d: ",i-1);
		     (void) print_fea_recf(fea_rec,h,stdout,field_name);
		  }
	        }
	      }
	      else if (complex_type(h))
		pr_feasd_data (istrm, srec, erec, h);
	      else
		pr_sd_data (istrm, srec, erec, h);
	       break;
	    default:
	      fea_rec = allo_fea_rec(h);
	      i = srec;
	      while (i++ <= erec && (get_fea_rec(fea_rec,h,istrm) != EOF)) {
		if (!tflag || (tflag && fea_rec->tag >= stag &&
				fea_rec->tag <= etag)) {
		   (void) printf("Record %d: ",i-1);
		   (void) print_fea_recf(fea_rec,h,stdout,field_name);
		}
	      }
	      break;
	   }
	   break;

	default:
	   if (!gflag)
	      Fprintf(stderr,
		"psps: Unknown file type, code: %d\n",h->common.type);
	   rec_size = get_rec_len (h);
	   if ((data = calloc_d((unsigned)rec_size)) == NULL)
		ERROR_EXIT("calloc could not allocate memory for data");

	   i = srec;
	   while (i++ <= erec && (get_gen_recd(data,&tag,h,istrm) != EOF)) {
		if (!tflag || (tflag && tag >= stag &&
				tag <= etag)) {
		    (void) printf ("Record: %d", i-1);
		    if (h->common.tag == YES)
		        (void) printf (" Tag: %ld\n", tag);
		    else
			(void) printf ("\n");
		    for (k = 0; k < rec_size; k++)
		        (void) printf ("   element%d: %g\n",k+1,data[k]);
		 }
	   }
     }
     exit (0);
/* NOTREACHED */
}


void
pr_history(name,h,level)
char *name;
struct header *h;
int level;
{
/*
 * prints common portion of all embedded headers
 */
    int     i;
    tab(level);
    (void) printf("File: %s\n",name);
    print_common(level,h);
    level++;
    for (i = 0; h->variable.srchead[i] && i < h->variable.nheads; i++) {
	tab (level);
	(void) printf ("----------------\n");
	pr_history (h->variable.source[i], h->variable.srchead[i], level);
    }
    return;
}

char *
file_typeX(type)
short type;
{
    switch (type) {

#ifdef ESI
	case FT_ANA:
	    return "ANA (Analysis File)";
	case FT_PIT:
	    return "PIT (Pitch Data File)";
	case FT_ROS:
	    return "RPS (Rosetta Speech Frame File)";
#endif

	case FT_SD:
	    return "SD (Sampled Data File)";
	case FT_SPEC:
	    return "SPEC (Spectral Records)";
	case FT_FILT:
	    return "FILT (Filter Coefficient File)";
        case FT_SCBK:
   	    return "SCBK (Scaler Codebook File)";
	case FT_FEA:
	    return "FEA (Feature File)";
	default:
	    return "Unknown type";
    }
}


void
pr_sd_data (istrm, srec, erec, h)
FILE *istrm;
struct header *h;
long srec, erec;
{
/* print a range of data from a ESPS sampled data file	
 * for integer types, 10/line. For floating types, 5/line.
 */
    int itype;
    double data[10];
    long i, npl, ngot, nleft = erec - srec + 1;
    itype = (h->common.nlong || h->common.nshort || h->common.nchar);
    if (srec > erec) return;
    npl = (itype ? 10 : 5);
    while (nleft > 0) {
	if (npl > nleft) npl = nleft;
 	ngot = get_sd_recd (data, (int)npl, h, istrm);
	if (ngot == 0)
	    break;
	(void) printf ("%6d: ", srec);	
	if (itype) for (i = 0; i < ngot; i++)
		(void) printf ("%7d", (int) data[i]);
	else for (i = 0; i < ngot; i++) {
		(void) printf (" %10.4f", data[i]);
	     }
	putchar ('\n');
	nleft -= ngot;
	srec += ngot;
    }
}

void
pr_full_header(name,h,recursive,level)
char *name;
struct header *h;
int recursive,level;
{
/*
 * prints full headers, including type specific and generics;
 * embedded headers printed if recursive set
 */
    tab (level);
    (void) printf("File: %s\n",name);
    print_common(level,h);
    if (vflag && h->variable.refhd) {
        (void) printf("Reference Header:\n");
	pr_full_header("reference header",h->variable.refhd,0,level+1);
    }
#ifdef ESI
    if (h->common.type == FT_ANA) print_ana(level,h);
    if (h->common.type == FT_PIT) print_pitch(level,h);
    if (h->common.type == FT_ROS) print_ros(level,h);
#endif ESI
    if (h->common.type == FT_SD)   print_sd(level,h);
    if (h->common.type == FT_SPEC) print_spec(level,h);
    if (h->common.type == FT_FILT) print_filt(level,h);
    if (h->common.type == FT_SCBK) print_scbk(level,h);
    if (h->common.type == FT_FEA)  print_fea(level,h);
    print_generic(level,h);
    (void) printf("\n");
    if (recursive) {
	int     i = 0;
	level++;
	while (h->variable.srchead[i] && i < h->variable.nheads) {
	    tab (level);
	    (void) printf ("----------------\n");
	    pr_full_header (h->variable.source[i], h->variable.srchead[i],
		    recursive, level);
	    i++;
	}
    }
}


/* myrealloc is like realloc, except that if ptr is NULL, it acts like
   a calloc
*/

static char *
myrealloc(ptr, size)
char *ptr;
unsigned size;
{
	spsassert(size > 0, "psps/myrealloc: size <= 0");
	if (ptr == NULL)
	    return calloc((unsigned)1,size);
	else
	    return realloc(ptr, size);
}


int
complex_type(hdr)
struct header *hdr;
{
	spsassert(hdr->common.type == FT_FEA && hdr->hd.fea->fea_type == FEA_SD,
	  "complex_type() in psps.c called with wrong arg type");
	if (hdr->hd.fea->ndcplx || hdr->hd.fea->nfcplx || hdr->hd.fea->nlcplx ||
	  hdr->hd.fea->nscplx || hdr->hd.fea->nbcplx)
		return 1;
	else
		return 0;
}

void
pr_feasd_data (istrm, srec, erec, h)
FILE *istrm;
struct header *h;
long srec, erec;
{
/* multi-channel or complex or both */

    int itype;
    double_cplx *cplx_data;
    struct feasd *feasd_data;
    long i, npl, ngot, nleft = erec - srec + 1;
    itype = (h->common.nlong || h->common.nshort || h->common.nchar);
    if (srec > erec) return;
    npl = (itype ? 4 : 3);
    feasd_data = allo_feasd_recs(h,DOUBLE_CPLX,4L,NULL,YES);
      while (nleft > 0) {
        if (npl > nleft) npl = nleft;
        ngot = get_feasd_recs(feasd_data,0L,npl,h,istrm);
        if (ngot == 0)
            break;
        (void) printf ("%6d: ", srec);
	cplx_data = (double_cplx *)feasd_data->data;
        if (itype) for (i = 0; i < ngot; i++)
                (void) printf ("[%7d,%7d]",(int)cplx_data[i].real,
				           (int)cplx_data[i].imag);
        else for (i = 0; i < ngot; i++) {
                (void) printf ("[%10.4f,%10.4f]",cplx_data[i].real,
				           cplx_data[i].imag);
             }
        putchar ('\n');
        nleft -= ngot;
        srec += ngot;
      }
}
