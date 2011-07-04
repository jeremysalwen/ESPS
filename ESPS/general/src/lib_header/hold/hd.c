/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Joseph T. Buck
 * Checked by:
 * Revised by:  Alan Parker
 *
 * Entry points:							     
 *    read_header(stream):  reads a header from stream and returns
 *        a pointer to it.					
 *    write_header(h, stream):  writes the header *h to stream.
 *    new_zfunc(ns,ds,num,den):  creates a new zfunc structure.
 *    new_header(type):  creates a new, zero header.		
 *    copy_header(h):  returns a pointer to a copy of h.
 *    add_source_file(oh, name, ih):  adds a source file and a source
 *        header to the output header.				
 *    add_comment(h, text):  appends a string to the comment field
 *
 */

static char *sccs_id = "@(#)headers.c	1.108	5/2/98	ESI/ERL";

#include <stdio.h>
#include <pwd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#ifdef DEC_ALPHA
#undef MIN
#undef MAX
#endif
#ifndef APOLLO_68K
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <esps/esps.h>
#include <esps/filt.h>
#include <esps/unix.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/esignal_fea.h>
#include <esps/pc_wav.h>

/* Minimum number of bytes to read to recognize an ESPS file.
 * The ESPS magic number is a 4-byte long following four others
 * (if there is a preamble).
 */
#define ESPS_PREFIX_SIZE    (ESPS_MAGIC_OFFSET + 4)
#define ESPS_MAGIC_OFFSET   16

#define ESPS_BYTE  8
#define ESPS_SHORT  4

#undef BYTE
#undef SHORT

extern int errno;
extern int debug_level; 

#define MAX_PARAM_LEN 6000
/*
  This is a temp hack to work around a bug whereby a comment could be
  longer than the maximum size of a variable header item.   In the next
  version of esps change MAX_PARAM_LEN above to 8192, and removed the redefine
  of MAX_STRING
*/

#undef MAX_STRING	
#define MAX_STRING 6000*4


#define SIZINT (sizeof (int))
#define SIZFLT (sizeof (float))
#define SIZSHO (sizeof (short))
#define SIZLO  (sizeof (long))
#define SIZCHAR (sizeof (char))
#define SIZZF(z) (z ? (2*SIZSHO + (z->nsiz+z->dsiz)*SIZFLT)/SIZINT : 0)
#define BADHEAD(lev,text) { badhead (lev, text); return NULL; }

char    head_errtxt[100];
static int  buf[MAX_PARAM_LEN];
#define buf2 &buf[MAX_PARAM_LEN/2]/* 2nd half of buf */
long long_dummy;
char   *savestring ();
static void badhead (); 
static void recursive_wh (), put_shorts ();
static short  *get_shorts (), *copy_shorts ();
static float  *copy_floats (), *get_floats ();
static void write_genhd ();
char   *add_genhd ();
static void hd_error (), put_longs ();
static long   *copy_longs (), *get_longs ();
static void set_ana_type(), set_pit_type(), set_spec_type(), set_ros_type();
void set_filt_type(), set_scbk_type();
void add_comment();
#ifndef DEC_ALPHA
static long swap_longs();
#else
static int swap_longs();
#endif
void add_genzfunc();
int read_preamble();
static void rec_free_hdr();	/* used by free_header */
static void free_zfunc();	/* used by free_header */
static void free_strlist();	/* used by free_header */
static void f_copy(), putnulls(), write_par();
static int old_version();

static long edr_bytes_to_long();    /* used by recursive_rh */
static long lsbf_bytes_to_long();   /* used by recursive_rh */

double atof();

#ifdef DEMO
#define NOKEY 1
#endif

#ifndef NO_LIC
void check_header();
#endif


#define PRE_DERIVED "1.34"	/* last version of header.h without 
				   derived fields for feature files */
#define PRE_NEWSIZ  "1.44"	/* last version with ndouble, nfloat,
				   nlong, nshort, nchar, and fixpartsiz
				   as shorts */
#define PRE_FIELD_ORDER "1.57"	/* last version without ability to read or
				   write feature files in field order */
#define PRE_COMPLEX "1.70"	/* last version without complex fea fields */
#define PRE_ALIGNMENT "1.73"	/* last version without alignment pad */


#include <esps/mach_codes.h>

static int	edr_flag=0;		/* global EDR flag for reading */
static int	machine_code;		/* global machine_code flag */
int		EspsDebug = 0;		/* debug code enabled in this module */
#ifdef DEMO
static int	hash();			/* simple hash function */
#endif


/* copy zfunc, copies a zfunc from one header to another
*/

struct zfunc   *
copy_zfunc (src)
struct zfunc   *src;
{
    if (src == NULL)
	return (NULL);
    return (new_zfunc (src -> nsiz, src -> dsiz, src -> zeros, src -> poles));
}

/*
	new_zfunc creates a new zfunc structure from dynamic memory.
*/

struct zfunc   *
new_zfunc (ns, ds, num, den)
int     ns, ds;
float   num[], den[];
{
    struct zfunc   *z = (struct zfunc  *) malloc ((unsigned) sizeof *z);
    spsassert(z,"new_zfunc: malloc failed");
    if (ns > 0) {
	z -> zeros = (float *) malloc ((unsigned) (ns * SIZFLT));
        spsassert(z->zeros,"new_zfunc: malloc failed");
	f_copy (z -> zeros, num, ns);
    }
    else
	z -> zeros = NULL;
    if (ds > 0) {
	z -> poles = (float *) malloc ((unsigned) (ds * SIZFLT));
        spsassert(z->poles,"new_zfunc: malloc failed");
	f_copy (z -> poles, den, ds);
    }
    else
	z -> poles = NULL;
    z -> nsiz = ns;
    z -> dsiz = ds;
    return z;
}

static struct zfunc *
read_zfunc (lev, fd) int    lev;
FILE * fd; {
    short   ns, ds;
    if (!miio_get_short (&ns, 1, edr_flag, machine_code, fd) ||
	    !miio_get_short (&ds,  1, edr_flag, machine_code, fd))
	BADHEAD (lev, "zfunc read error");
    if (ns != 0)
	if (!miio_get_float ((float *)buf,  ns, edr_flag, machine_code, fd))
	    BADHEAD (lev, "zfunc read error");
    if (ds != 0)
	if (!miio_get_float ((float *)buf2,  ds, edr_flag, machine_code, fd))
	    BADHEAD (lev, "zfunc read error");
    return new_zfunc (ns, ds, (float *) buf, (float *) buf2);
}

static  void
write_zfunc (code, z, fd)
short   code;
struct zfunc   *z;
FILE * fd;
{
    short   len = SIZZF (z);
    if (!miio_put_short(&code, 1, edr_flag, fd) ||
	    !miio_put_short(&len, 1, edr_flag, fd) ||
	    !miio_put_short(&z -> nsiz, 1, edr_flag, fd) ||
	    !miio_put_short(&z -> dsiz, 1, edr_flag, fd))
	hd_error ("write_zfunc");
    if (z -> nsiz != 0)
	if (!miio_put_float(z -> zeros,  z -> nsiz, edr_flag, fd))
	    hd_error ("write_zfunc");

    if (z -> dsiz != 0)
	if (!miio_put_float(z -> poles, z -> dsiz, edr_flag, fd))
	    hd_error ("write_zfunc");
}

static int
typesiz (type)
int     type;
{
    switch (type) {
	case DOUBLE: 
	    return sizeof (double);
	case FLOAT: 
	    return sizeof (float);
	case LONG: 
	    return sizeof (long);
	case ESPS_SHORT: 
	    return sizeof (short);
	case CODED: 
	    return sizeof (short);
	case CHAR: 
	case ESPS_BYTE: 
	case EFILE:
	case AFILE:
	    return sizeof (char);
	case DOUBLE_CPLX:
            return sizeof(double_cplx);
    	case FLOAT_CPLX:
            return sizeof(float_cplx);
    	case LONG_CPLX:
            return sizeof(long_cplx);
    	case SHORT_CPLX:
            return sizeof(short_cplx);
    	case BYTE_CPLX:
            return sizeof(byte_cplx);
    }
    spsassert(NO, "typesiz: unrecognized type code");

    return 0;	/* not reached, just to keep lint happy */
}

static  void
write_genhd (code, np, fp)
short   code;
struct gen_hd  *np;
FILE * fp;
{
    short len;
/*
 * do not write out generic for sphere_hd_ptr, esignal_hd_ptr,
 * or pc_wav_hd_ptr
 */
    if (!strcmp(np->name, "sphere_hd_ptr")
	|| !strcmp(np->name, "esignal_hd_ptr")
	|| !strcmp(np->name, "pc_wav_hd_ptr"))
	return;
    write_par (code, np -> name, fp);
    if (!miio_put_int (&np -> size, 1, edr_flag, fp) ||
	    !miio_put_short(&np -> type, 1, edr_flag, fp))
	hd_error ("write_genhd");
    if (np->type == CODED) {
	char **p = np->codes;
	while (*p != NULL) {
		len = strlen(*p)+1;
		if(!miio_put_short(&len, 1, edr_flag, fp) ||
		   !miio_put_char(*p, len, edr_flag, fp)) 
			hd_error ("write_genhd");
		p++;
	}
	len = 0;
	if(!miio_put_short(&len, 1, edr_flag, fp))
		hd_error ("write_genhd");
    } 
    switch (np->type) {
	case DOUBLE:
	if (!miio_put_double((double *)np->d_ptr, (int)np->size, 
	     edr_flag, fp))
		hd_error ("write_genhd");
	break;
	case FLOAT:
	if (!miio_put_float((float *)np->d_ptr, (int)np->size, 
	     edr_flag, fp))
		hd_error ("write_genhd");
	break;
	case LONG:
	if (!miio_put_long((long *)np->d_ptr, (int)np->size, 
	     edr_flag, fp))
		hd_error ("write_genhd");
	break;
	case ESPS_SHORT:
	case CODED:
	if (!miio_put_short((short *)np->d_ptr, (int)np->size, 
	     edr_flag, fp))
		hd_error ("write_genhd");
	break;
	case CHAR:
	case ESPS_BYTE:
	case EFILE:
	case AFILE:
	if (!miio_put_char((char *)np->d_ptr, (int)np->size, 
	     edr_flag, fp))
		hd_error ("write_genhd");
	break;
     }
	
}

static
read_genhd (lev, fd, hd, name)
int     lev;
FILE * fd;
struct header  *hd;
char   *name;
{
    unsigned int    size;
    short   type, len;
    char   *ptr;
    char **codes;
    int data_rep = hd->common.edr;
    int mach_code = hd->common.machine_code;
    if (!miio_get_int (&size,  1, data_rep, mach_code, fd) ||
	    !miio_get_short (&type, 1, data_rep, mach_code, fd))
	BADHEAD (lev, "read_genhd read error");

    if (type != CODED) {
    	ptr = add_genhd (name, type, (int) size, (char *)NULL, 
		(char **)NULL, hd);
    }
    else {
	int i=0;
	codes = (char **)calloc(1,sizeof(char *));
	spsassert(codes != NULL,"read_genhd: calloc failed!");
        if (!miio_get_short (&len, 1, data_rep, mach_code, fd)) 
	    BADHEAD (lev, "read_genhd read error");
	while (len != 0) {
	    ptr = malloc(SIZCHAR*len);
	    spsassert(ptr != NULL,"read_genhd: malloc failed!");
	    if (!miio_get_char (ptr, len, data_rep, mach_code, fd))
	        BADHEAD (lev, "read_genhd read error");
	    codes[i++] = ptr;
	    codes = (char **)realloc((char *)codes, sizeof(char *)*(i+1));
	    spsassert(codes != NULL,"read_genhd: realloc failed!");
	    codes[i] = NULL;
            if (!miio_get_short (&len, 1, data_rep, mach_code, fd))
	        BADHEAD (lev, "read_genhd read error");
	}
    	ptr = add_genhd (name, type, (int) size, (char *)NULL, 
		codes, hd);
    }
    switch (type) {
	case DOUBLE:
	if (!miio_get_double((double *)ptr, (int)size, data_rep, mach_code, fd))
        	BADHEAD (lev, "read_genhd read error");
	break;
	case FLOAT:
	if (!miio_get_float((float *)ptr, (int)size, data_rep, mach_code, fd))
        	BADHEAD (lev, "read_genhd read error");
	break;
	case LONG:
	if (!miio_get_long((long *)ptr, (int)size, data_rep, mach_code, fd))
        	BADHEAD (lev, "read_genhd read error");
	break;
	case ESPS_SHORT:
	case CODED:
	if (!miio_get_short((short *)ptr, (int)size, data_rep, mach_code, fd))
        	BADHEAD (lev, "read_genhd read error");
	break;
	case CHAR:
	case ESPS_BYTE:
	case EFILE:
	case AFILE:
	if (!miio_get_char((char *)ptr, (int)size, data_rep, mach_code, fd))
        	BADHEAD (lev, "read_genhd read error");
	break;
     }
    return 0;
}



/*
	read_header reads in a file header. recursive_rh really does the
	work.
*/


/* #define NDREC1  */

/* define NDREC1 to get beta case of ndrec not being
   set to -1 for pipes.   Remove this to get esps 3.0 code 
*/

static long data_offset;
static long align_pad_size;
#ifdef DEMO
static long check_code;
#endif

extern int sphere_to_sd_ctrl;		/* defined in sphere.c */

struct header  *
read_header (fd)
FILE * fd;
{
    static struct header  *recursive_rh ();
    struct header  *sphere_to_feasd();
    long	   nrec(), k;
    struct header  *p;
    long   foreign_hd_length;


    if (getenv("ESPSDEBUG"))
	EspsDebug = 1;
#ifndef NO_LIC
       if((sphere_to_sd_ctrl != 123))
         check_header(MACH_CODE);
#endif
    p = recursive_rh(fd, 0);
    if(p == NULL) {           
	if (EspsDebug) Fprintf(stderr,"read_header: recursive_rh == NULL\n");
/* 
  check and see if it is a NIST sphere header 
*/
    	if (fseek(fd, 0L, 0) == 0) { 	/* its not a pipe */
         if (EspsDebug) Fprintf(stderr,"not a pipe\n");
	 if((p = sphere_to_feasd(fd)) == NULL)	{ /* not a NIST either */
           char *def_header;
	   FILE *header_fd;
	   if (EspsDebug) Fprintf(stderr,"not a NIST, rewinding\n");
           (void)fseek(fd, 0L, 0); /* rewind */
/*
  if the file is a SIG header file, then just return
  NULL, so that the code in waves does the right thing
*/
	   if (EspsDebug) Fprintf(stderr,"read_header: see if its SIG\n");
           if (is_SIG_file(fd)) {
	        if (EspsDebug) Fprintf(stderr,"read_header: it is SIG\n");
		return NULL;
	   }
/* 
 In this case its not an ESPS, NIST or SIG file, so if DEF_HEADER
 is defined, then try and add a default ESPS header to it
*/
	   def_header = getenv("DEF_HEADER");
	   if(EspsDebug) Fprintf(stderr,"read_header: not a Sphere\n");
	   if (def_header && (header_fd = fopen(def_header,"r"))) {
	        if(EspsDebug) 
		 Fprintf(stderr,"read_header: def_header: %s\n",def_header);
		p = recursive_rh(header_fd, 0);
		(void)fclose(header_fd);
		if (p == NULL) {
		  fprintf(stderr,"DEF_HEADER not ESPS\n");
		  return NULL;
		 }
		(void)fseek(fd, 0L, 0);
	   }
	   else {	
	        if(EspsDebug) 
		 Fprintf(stderr,"read_header: no default header\n");
		return NULL;
	   }
	 }
        }
	else {
         if(EspsDebug) Fprintf(stderr,"read_header: cannot rewind\n");
	 return NULL;
	}
    }
    if (!getenv("ESPS_NOALIGN")) {
     if(EspsDebug) Fprintf(stderr,"read_header: NOALIGN not set\n");
     if (!old_version(p->common.hdvers,PRE_ALIGNMENT)){
     	if(EspsDebug && (align_pad_size != 0))
	     Fprintf(stderr,"read_header: skipping alignment pad\n");
	if(EspsDebug) {
		Fprintf(stderr,"offset: %x\n",data_offset);
		Fprintf(stderr,"align pad size: %x\n",align_pad_size);
	}
    	if(align_pad_size != 0) {
		char *align_pad = malloc((unsigned)align_pad_size);
		spsassert(align_pad,"read_header: failed to alloc align pad");
		if (!miio_get_byte(align_pad, align_pad_size, YES, NONE, fd))
	  	  BADHEAD (0, "error reading alignment pad");
		free(align_pad);
	}
     }
     else
     	if(EspsDebug)
      	     Fprintf(stderr,"read_header: old_version or alignpad_size == 0\n");
    }

    foreign_hd_length = (long)get_genhd_val("foreign_hd_length",p,0.0);
    if(EspsDebug) 
	Fprintf(stderr,
	 "read_header: foreign_hd_length: %ld\n",foreign_hd_length);
    if (foreign_hd_length > 0) {
	char * foreign_hd_ptr = malloc((unsigned)foreign_hd_length);
	spsassert(foreign_hd_ptr,"read_header: failed to alloc foreign hd\n");
	if (!fread(foreign_hd_ptr, 1, foreign_hd_length,  fd))
	  	  BADHEAD (0, "error reading foreign header");
	*add_genhd_l("foreign_hd_ptr",(long *)NULL,1,p)=(long)foreign_hd_ptr;
    }
    

#ifdef DEC_ALPHA
    if (edr_default(p->common.machine_code)) /* default edr */
	p->common.edr = 1;
#endif
    k = nrec(p, fd);
#ifdef NDREC1
    if(k != -1) 
#endif
    p->common.ndrec = k;

#define ALTERNATE1
#ifdef  ALTERNATE1
/* if the header has the encoding generic and is FEA_SD then set the
   data type to SHORT
*/

    if (p->common.type == FT_FEA && p->hd.fea->fea_type == FEA_SD &&
	get_genhd_val("encoding",p,(double)0.0)) {
		int i;
		for(i=0;i<p->hd.fea->field_count;i++) {
			if (strcmp("samples",p->hd.fea->names[i]) == 0)  {
				p->hd.fea->types[i] = ESPS_SHORT;
/*
 This assumes that the file might have other fields,  I think files with
 encoding cannot have any other fields other than samples.  
				p->hd.fea->nshort += p->hd.fea->sizes[i];
				p->hd.fea->nbyte -= p->hd.fea->sizes[i];
				p->common.nshort += p->hd.fea->sizes[i];
				p->common.nchar -= p->hd.fea->sizes[i];
*/
/* This will only work if the file with encoding only has the samples
   field
*/
				p->hd.fea->nshort = p->hd.fea->sizes[i];
				p->hd.fea->nbyte = 0;
        			p->common.nshort = p->hd.fea->nshort;
				p->common.nchar = 0;
	
                        }
		}
    }
#endif
		
		
    return p;
}

static struct header   *
recursive_rh (fd, lev)
FILE * fd;
int     lev;
{
    struct header  *p = new_header (0);
				/* new_header makes an all zero header */
    short   code, len;
    int     nh = 0, ns = 0;
    int	    got_esps_preamble = NO;
    int	    got_common_part = NO;


/* some debug code
*/
    if (EspsDebug)
	Fprintf(stderr,"recursive_rh, lev: %d\n",lev);

/* see if this file has a preamble (older esps files might not) 
   only the outer most header has a preamble 
*/
    if (lev == 0)
    {
	struct preamble	pream;
	char		*prefix, *next_byte;
	int		prefix_size, num_read;

#define CANT_EXTEND(NewSize) \
	(num_read < NewSize \
	 && ((num_read \
	      += fread(prefix + num_read, 1, NewSize - num_read, fd)) \
	     < NewSize))


	prefix_size = ESPS_PREFIX_SIZE;
	if (prefix_size < ESIGNAL_PREFIX_SIZE)
	    prefix_size = ESIGNAL_PREFIX_SIZE;
	if (prefix_size < PC_WAV_PREFIX_SIZE)
	    prefix_size = PC_WAV_PREFIX_SIZE;

	prefix = malloc(prefix_size);
	num_read = 0;

	data_offset = -1;
	align_pad_size = 0;
#ifdef DEMO
	check_code = 3000;
#endif

	if (CANT_EXTEND(ESPS_PREFIX_SIZE))
	{
	    BADHEAD(lev, "error reading preamble");
	}
	if (edr_bytes_to_long(prefix + ESPS_MAGIC_OFFSET)
		 == HD_CHECK_VAL)
	{
	    if (EspsDebug) Fprintf(stderr, "ESPS preamble, EDR order.\n");

	    prefix = realloc(prefix, sizeof(struct preamble));

	    if (CANT_EXTEND(sizeof(struct preamble)))
		BADHEAD(lev, "error reading preamble");

	    pream.machine_code = edr_bytes_to_long(next_byte = prefix);
	    pream.check_code = edr_bytes_to_long(next_byte += 4);
	    pream.data_offset = edr_bytes_to_long(next_byte += 4);
	    pream.record_size = edr_bytes_to_long(next_byte += 4);
	    pream.check = edr_bytes_to_long(next_byte += 4);
	    pream.edr = edr_bytes_to_long(next_byte += 4);
	    pream.align_pad_size = edr_bytes_to_long(next_byte += 4);
	    pream.foreign_hd = edr_bytes_to_long(next_byte += 4);

	    free(prefix);

	    got_esps_preamble = YES;
	}
	else if (lsbf_bytes_to_long(prefix + ESPS_MAGIC_OFFSET)
		 == HD_CHECK_VAL)
	{
	    if (EspsDebug) Fprintf(stderr, "ESPS preamble, byte-swapped.\n");

	    prefix = realloc(prefix, sizeof(struct preamble));

	    if (CANT_EXTEND(sizeof(struct preamble)))
		BADHEAD(lev, "error reading preamble");

	    pream.machine_code = lsbf_bytes_to_long(next_byte = prefix);
	    pream.check_code = lsbf_bytes_to_long(next_byte += 4);
	    pream.data_offset = lsbf_bytes_to_long(next_byte += 4);
	    pream.record_size = lsbf_bytes_to_long(next_byte += 4);
	    pream.check = lsbf_bytes_to_long(next_byte += 4);
	    pream.edr = NO;
	    next_byte += 4;
	    pream.align_pad_size = lsbf_bytes_to_long(next_byte += 4);
	    pream.foreign_hd = lsbf_bytes_to_long(next_byte += 4);

	    free(prefix);

	    got_esps_preamble = YES;
	}
	else if (CANT_EXTEND(ESIGNAL_PREFIX_SIZE))
	{
	    BADHEAD(lev, "error reading preamble");
	}
	else if (esignal_check_prefix(prefix))
	{
	    if (EspsDebug) Fprintf(stderr, "Esignal preamble.\n");

	    free((char *) p);
	    p = esignal_to_fea(prefix, num_read, fd);
	    free(prefix);
	    return p;
	}
	else if (CANT_EXTEND(PC_WAV_PREFIX_SIZE))
	{
	    BADHEAD(lev, "error reading prefix");
	}
	else if (pc_wav_check_prefix(prefix))
	{
	    if (EspsDebug) Fprintf(stderr, "PC WAVE prefix.\n");

	    free((char *) p);
	    p = pc_wav_to_feasd(prefix, num_read, fd);
	    free(prefix);
	    return p;
	}
	else
	{
	    if (EspsDebug) Fprintf(stderr, "no preamble here.\n");

/* if the preamble is not valid, we must copy the stuff we read back
 * to a standard common header structure and then read the rest of it 
 */
	    if (num_read > FIXPART_LEN)
		BADHEAD(lev, "read too far ahead");

	    (void) bcopy((char *) prefix, (char *) p, num_read);
	    if (fread((char *) p + num_read,
		      FIXPART_LEN - num_read, 1, fd) == 0)
		BADHEAD(lev, "error reading rest of common, after preamble");

/* if this is an older type file, then it is not EDR_ESPS */

	    edr_flag = NO;
	    p->common.edr = NO;
	    machine_code = p->common.machine_code;

	    got_common_part = YES;
	}

	if (got_esps_preamble)
	{
	    if (EspsDebug)
	  	Fprintf(stderr,"found preamble\n");

	    p->common.edr = pream.edr;
	    edr_flag = pream.edr;
	    p->common.machine_code = pream.machine_code;
	    machine_code = pream.machine_code;
	    if(EspsDebug)
		Fprintf(stderr,"read_header: edr_flag: %d, machine: %s\n",
			edr_flag, machine_codes[machine_code]);

	    data_offset = pream.data_offset;
	    align_pad_size = pream.align_pad_size;
#ifdef DEMO
	    check_code = pream.check_code;
#endif
	}
    }

    if (!got_common_part) {

	  p->common.machine_code = machine_code;
	  p->common.edr = edr_flag;
	  if (miio_get_short(&p->common.type, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
#ifndef NOPAD
	  if (miio_get_short(&p->common.pad1, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
#endif
	  if (miio_get_long(&p->common.check, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_char(p->common.date, DATESIZE, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_char(p->common.hdvers, VERSIONSIZE, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_char(p->common.prog, PROGSIZE, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_char(p->common.vers, VERSIONSIZE, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_char(p->common.progdate, DATESIZE, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_long(&p->common.ndrec, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_short(&p->common.tag, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_short(&p->common.nd1, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_long(&p->common.ndouble, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_long(&p->common.nfloat, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_long(&p->common.nlong, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_long(&p->common.nshort, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_long(&p->common.nchar, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_long(&p->common.fixpartsiz, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_long(&p->common.hsize, 1, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_char(p->common.user, USERSIZ, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
	  if (miio_get_short(p->common.spares, NSPARES, edr_flag, 
	      machine_code, fd) == 0)
		BADHEAD (lev, "error reading common part of header");
          if (EspsDebug)
		fprintf(stderr,"read_header: header version: %s\n",
			p->common.hdvers);
#ifdef DEMO
	  if (got_esps_preamble && check_code != (3000 + hash(p->common.date)))
	      return NULL;
#endif
    }

/* validate the header 
*/
    if (p -> common.check != HD_CHECK_VAL) {
        free(p);
	BADHEAD (lev, "bad check value");
    }


#if defined(M5500) || defined(M5600) 
/* if its an old header, then we have to shift ndouble through fixpartsiz
   from shorts to longs
*/
    if (old_version(p->common.hdvers,PRE_NEWSIZ)){
	short ndouble, nfloat, nlong, nshort, nchar, fixpartsiz;
	long hsize;
	char *uname;

/*
	(void)fprintf(stderr,"old header found, lev: %d, version: %s\n",
		lev,p->common.hdvers);
*/
	ndouble = p->common.nd1;
	nfloat = p->common.ndouble >> 16;
	nlong = (p->common.ndouble << 16) >> 16;
	nshort = p->common.nfloat >> 16;
	nchar = (p->common.nfloat << 16) >> 16;
	fixpartsiz = p->common.nlong >> 16;
	hsize = p->common.nshort;
	uname = savestring((char *)&p->common.nchar);

/*
	(void)fprintf(stderr,"ndouble: %d, nfloat: %d, nlong: %d, nshort: %d\n",
		ndouble, nfloat, nlong, nshort);
	(void)fprintf(stderr,"nchar: %d, fixpartsiz: %d, hsize: %ld, uname: %s\n",
		nchar, fixpartsiz, hsize, uname);
*/

	p->common.ndouble = ndouble;
	p->common.nfloat = nfloat;
	p->common.nlong = nlong;
	p->common.nshort = nshort;
	p->common.nchar = nchar;
	p->common.fixpartsiz = fixpartsiz;
	p->common.hsize = hsize;
	(void)strcpy(p->common.user, uname);

    }
#endif

#ifndef DEC_ALPHA
    if ((p->common.machine_code != DEC_ALPHA_CODE)
	&& (p -> common.fixpartsiz >= p -> common.hsize ||
	    p -> common.fixpartsiz < FIX_HEADER_SIZE-sizeof(p->common.edr)-
				 sizeof(p->common.machine_code)))
	BADHEAD (lev, "bad header size");
#endif


    

/* get type specific part of header */
    switch (p -> common.type) {

#ifdef ESI
	case FT_ANA: 
	    p -> hd.ana = (struct ana_header   *) calloc (1, ANA_SIZE);
	    if (fread ((char *) p -> hd.ana, 1, ANA_SIZE, fd) == 0)
		BADHEAD (lev, "bad specfic part -ana");
	    break;

	case FT_PIT: 
	    p -> hd.pit = (struct pit_header   *) calloc (1, PIT_SIZE);
	    if (fread ((char *) p -> hd.pit, 1, PIT_SIZE, fd) == 0)
		BADHEAD (lev, "bad specfic part -pit");
	    break;

	case FT_ROS: {
		struct ros_header  *ros;
		p -> hd.ros = (struct ros_header   *) calloc (1, ROS_SIZE);
		if (fread ((char *) p -> hd.ros, 1, ROS_SIZE, fd) == 0)
		    BADHEAD (lev, "bad specfic part -ros");
		ros = p -> hd.ros;
		ros -> type_bits = get_shorts (fd, lev, ros -> maxtype);
		ros -> rc_ubits = get_shorts (fd, lev, ros -> order_unvcd);
		ros -> rc_vbits = get_shorts (fd, lev, ros -> order_vcd);
		ros -> pulse_bits = get_shorts (fd, lev, ros -> maxpulses);
		ros -> pow_bits = get_shorts (fd, lev, ros -> maxpow);
		break;
	    }
#endif ESI

	case FT_SD: 
	    p -> hd.sd = (struct sd_header *) calloc (1, SD_SIZE);
	    if (!miio_get_short(&p->hd.sd->equip, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
#ifndef NOPAD
	      if (!miio_get_short(&p->hd.sd->pad1, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
#endif
	    if (!miio_get_float(&p->hd.sd->max_value, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_float(&p->hd.sd->sf, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_float(&p->hd.sd->src_sf, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_short(&p->hd.sd->synt_method, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
#ifndef NOPAD
	      if (!miio_get_short(&p->hd.sd->pad2, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
#endif
	    if (!miio_get_float(&p->hd.sd->scale, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_float(&p->hd.sd->dcrem, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_short(&p->hd.sd->q_method, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_short(&p->hd.sd->v_excit_method, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_short(&p->hd.sd->uv_excit_method, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_short(&p->hd.sd->spare1, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_short(&p->hd.sd->nchan, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_short(&p->hd.sd->synt_interp, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_short(&p->hd.sd->synt_pwr, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_short(&p->hd.sd->synt_rc, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_short(&p->hd.sd->synt_order, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
#ifndef NOPAD
	      if (!miio_get_short(&p->hd.sd->pad3, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
#endif
	    if (!miio_get_long(&p->hd.sd->start, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_long(&p->hd.sd->nan, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_short(p->hd.sd->spares, SD_SPARES, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
/* 
 the following two are to account for the fact that pointers
 were written out with the structure in the old way of doing things.
 This assume that a pointer == a long in size
*/
	    if (!miio_get_long(&long_dummy, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    if (!miio_get_long(&long_dummy, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -sd");
	    p -> hd.sd ->prefilter = NULL;
	    p -> hd.sd ->de_emp = NULL;
	    
	    break;

	case FT_SPEC: 
	    p -> hd.spec = (struct spec_header *) calloc (1, SPEC_SIZE);
	    if (!miio_get_long(&p->hd.spec->start, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_long(&p->hd.spec->nan, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(&p->hd.spec->frmlen, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(&p->hd.spec->order_vcd, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(&p->hd.spec->order_unvcd, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(&p->hd.spec->win_type, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_float(&p->hd.spec->sf, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(&p->hd.spec->spec_an_meth, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
#ifndef NOPAD
	      if (!miio_get_short(&p->hd.spec->pad1, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
#endif
	    if (!miio_get_float(&p->hd.spec->dcrem, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(&p->hd.spec->post_proc, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(&p->hd.spec->frame_meth, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(&p->hd.spec->voicing, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(&p->hd.spec->freq_format, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(&p->hd.spec->spec_type, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(&p->hd.spec->contin, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_long(&p->hd.spec->num_freqs, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_short(p->hd.spec->spares, SPEC_SPARES, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_long(&long_dummy, 1, edr_flag, 
	      machine_code, fd)) /* pre_emp */
		BADHEAD (lev, "bad specfic part -spec");
	    if (!miio_get_long(&long_dummy, 1, edr_flag, 
	      machine_code, fd)) /* pre_emp */
		BADHEAD (lev, "bad specfic part -spec");
	    p -> hd.spec -> freqs = NULL;
	    p -> hd.spec -> pre_emp = NULL;
	    break;

	case FT_FILT: {
		struct filt_header *filt;
		long dummy_long[4];
		p -> hd.filt = (struct filt_header *) calloc (1, FILT_SIZE);
	    
	    if (!miio_get_short(&p->hd.filt->max_num, 1, edr_flag, 
	      machine_code, fd)) 
		BADHEAD (lev, "bad specfic part -filt");
	    if (!miio_get_short(&p->hd.filt->max_den, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -filt");
	    if (!miio_get_short(&p->hd.filt->func_spec, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -filt");
	    if (!miio_get_short(&p->hd.filt->nbands, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -filt");
	    if (!miio_get_short(&p->hd.filt->npoints, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -filt");
	    if (!miio_get_short(&p->hd.filt->g_size, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -filt");
	    if (!miio_get_short(&p->hd.filt->nbits, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -filt");
	    if (!miio_get_short(&p->hd.filt->type, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -filt");
	    if (!miio_get_short(&p->hd.filt->method, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -filt");
	    if (!miio_get_short(p->hd.filt->spares, FILT_SPARES, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -filt");
#ifndef NOPAD
	      if (!miio_get_short(&p->hd.filt->pad1, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -filt");
#endif
	    if (!miio_get_long(dummy_long, 4, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -filt");
	    p -> hd.filt -> bandedges = NULL;
	    p -> hd.filt -> points = NULL;
	    p -> hd.filt -> gains = NULL;
	    p -> hd.filt -> wts = NULL;
	    
	    filt = p -> hd.filt;
	    if (filt -> nbands != 0)
                 filt -> bandedges = get_floats (fd, lev, 2 * filt -> nbands);
            if (filt -> npoints != 0)
                 filt -> points = get_floats (fd, lev, filt -> npoints);
            if (filt -> func_spec == BAND && filt -> nbands != 0)
                 filt -> gains = get_floats (fd, lev, filt -> nbands);
            if (filt -> func_spec == POINT && filt -> npoints != 0)
                 filt -> gains = get_floats (fd, lev, filt -> npoints);
            if (filt -> func_spec == BAND && filt -> nbands != 0)
                 filt -> wts = get_floats (fd, lev, filt -> nbands);
            if (filt -> func_spec == POINT && filt -> npoints != 0)
                 filt -> wts = get_floats (fd, lev, filt -> npoints);
		break;
	    }



	case FT_SCBK: 
	    p -> hd.scbk = (struct scbk_header *) calloc (1, SCBK_SIZE);
	    if (!miio_get_long(&p->hd.scbk->num_items, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -scbk");
	    if (!miio_get_short(&p->hd.scbk->distortion, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -scbk");
	    if (!miio_get_short(&p->hd.scbk->num_cdwds, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -scbk");
	    if (!miio_get_float(&p->hd.scbk->convergence, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -scbk");
	    if (!miio_get_short(&p->hd.scbk->codebook_type, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -scbk");
	    if (!miio_get_short(&p->hd.scbk->element_num, 1, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -scbk");
	    if (!miio_get_short(p->hd.scbk->spares, SCBK_SPARES, edr_flag, 
	      machine_code, fd))
		BADHEAD (lev, "bad specfic part -scbk");
	    break;

	case FT_FEA: {
		struct fea_header  *fea;
		unsigned    size;
		short   len, i, j;
		long dummy[9];
		p -> hd.fea = (struct fea_header   *) calloc (1, FEA_SIZE);
		fea = p -> hd.fea;
		if (!miio_get_short(&p->hd.fea->fea_type, 1, edr_flag, 
	      machine_code, fd))
		    BADHEAD (lev, "bad specific part -fea");
		if (!miio_get_short(&p->hd.fea->segment_labeled,1,edr_flag, 
	      machine_code, fd))
		    BADHEAD (lev, "bad specific part -fea");
		if (!miio_get_short(&(p->hd.fea->field_count),1,
		edr_flag, machine_code, fd))
		    BADHEAD (lev, "bad specific part -fea");
#ifndef NOPAD
		  if (!miio_get_short(&p->hd.fea->field_order,1,edr_flag, 
	            machine_code, fd))
		    BADHEAD (lev, "bad specific part -fea");
#endif
		if (!miio_get_long(dummy,9,edr_flag, 
	      machine_code, fd))
		    BADHEAD (lev, "bad specific part -fea");
		if (!miio_get_short(p->hd.fea->spares,FEA_SPARES,edr_flag, 
	      machine_code, fd))
		    BADHEAD (lev, "bad specific part -fea");
		
		size = fea -> field_count;
		spsassert(fea -> field_count != 0,
			"recursive_rh: internal error");
#ifndef NOPAD
/* if an old header, ignore the
   field_order flag
*/
		if (fea -> field_order == YES && 
		    old_version(p->common.hdvers, PRE_FIELD_ORDER))
			fea->field_order = NO;
#endif

		fea -> names = (char **) calloc (size + 1, sizeof (char *));
		fea -> dimens = (long **) calloc (size, sizeof (long *));
		fea -> enums = (char ***) calloc (size, sizeof (char **));
		fea -> srcfields = (char ***)calloc(size,sizeof(char **));
		fea -> sizes = get_longs (fd, lev, (int)size);
                fea -> starts = get_longs (fd, lev, (int)size);
                fea -> ranks = get_shorts (fd, lev, (int)size);
                fea -> types = get_shorts (fd, lev, (int)size);
/* for versions of the header with complex data type in feature files
*/
		if (!old_version(p->common.hdvers,PRE_COMPLEX)) {
		    if(EspsDebug) 
			Fprintf(stderr,"read_header: new FEA header.\n");
		    if (!miio_get_long(&p->hd.fea->ndouble, 1, edr_flag, 
	             machine_code, fd))
		       BADHEAD (lev, "bad specific part -fea");
		    if (!miio_get_long(&p->hd.fea->nfloat, 1, edr_flag, 
	             machine_code, fd))
		       BADHEAD (lev, "bad specific part -fea");
		    if (!miio_get_long(&p->hd.fea->nlong, 1, edr_flag, 
	             machine_code, fd))
		       BADHEAD (lev, "bad specific part -fea");
		    if (!miio_get_long(&p->hd.fea->nshort, 1, edr_flag, 
	             machine_code, fd))
		       BADHEAD (lev, "bad specific part -fea");
		    if (!miio_get_long(&p->hd.fea->nbyte, 1, edr_flag, 
	             machine_code, fd))
		       BADHEAD (lev, "bad specific part -fea");
		    if (!miio_get_long(&p->hd.fea->ndcplx, 1, edr_flag, 
	             machine_code, fd))
		       BADHEAD (lev, "bad specific part -fea");
		    if (!miio_get_long(&p->hd.fea->nfcplx, 1, edr_flag, 
	             machine_code, fd))
		       BADHEAD (lev, "bad specific part -fea");
		    if (!miio_get_long(&p->hd.fea->nlcplx, 1, edr_flag, 
	             machine_code, fd))
		       BADHEAD (lev, "bad specific part -fea");
		    if (!miio_get_long(&p->hd.fea->nscplx, 1, edr_flag, 
	             machine_code, fd))
		       BADHEAD (lev, "bad specific part -fea");
		    if (!miio_get_long(&p->hd.fea->nbcplx, 1, edr_flag, 
	             machine_code, fd))
		       BADHEAD (lev, "bad specific part -fea");
		 } else {
		    if(EspsDebug) 
			Fprintf(stderr,"read_header: old FEA header.\n");
		    p->hd.fea->ndouble = p->common.ndouble;
		    p->hd.fea->nfloat = p->common.nfloat;
		    p->hd.fea->nlong = p->common.nlong;
		    p->hd.fea->nshort = p->common.nshort;
		    p->hd.fea->nbyte = p->common.nchar;
		}


/* don't read this
   for old versions
*/
		if (!old_version(p->common.hdvers,PRE_DERIVED)){
		    fea -> derived = get_shorts (fd, lev, (int)size);
		}
		for (i = 0; i < size; i++) {
    		    if (!miio_get_short(&len, 1, edr_flag, machine_code, fd))
			BADHEAD (lev, "bad specific part-len of fea name");
		    fea -> names[i] = calloc ((unsigned)(len+1), sizeof (char));
		    if (!miio_get_char(fea->names[i], len, edr_flag, 
					machine_code, fd))
			BADHEAD (lev, "bad specific part -fea names");
		    fea -> names[i][len] = '\0';
		    if (fea->ranks[i] != 0) {
		    	fea -> dimens[i] = 
			(long *)calloc((unsigned) fea->ranks[i], sizeof (long));
			if (!miio_get_long(fea->dimens[i], (int)fea->ranks[i], 
				edr_flag, machine_code, fd))
			BADHEAD (lev, "bad specific part -fea ranks");
		    }
		    if (fea -> types[i] == CODED) {
    			if (!miio_get_short(&len, 1, edr_flag,
						machine_code, fd))
			    BADHEAD (lev, "bad specific part-len of fea enums");
			fea -> enums[i] = (char **) calloc ((unsigned)len+1,
				sizeof (char *));
			fea->enums[i][0] = NULL;
			for (j = 0; j < len; j++) {
			    short   len2;

    			    if (!miio_get_short(&len2, 1, edr_flag,
						machine_code, fd))
				BADHEAD (lev,
					"bad specific part-len of fea enum");
			    fea -> enums[i][j] = calloc ((unsigned)(len2+1), 
					sizeof (char));
			    if (!miio_get_char(fea->enums[i][j], (int)len2,
				edr_flag, machine_code, fd))
				BADHEAD (lev, "bad specific part -fea enums");
			    fea -> enums[i][j][len2] = '\0';
			}
		    }
		    if (!old_version(p->common.hdvers,PRE_DERIVED)) {
    			if (!miio_get_short(&len, 1, edr_flag,
						machine_code, fd))
			    BADHEAD (lev, "bad specific part-len of src flds");
			fea -> srcfields[i] = (char **) 
				calloc ((unsigned)len+1, sizeof (char *));
			fea->srcfields[i][0] = NULL;
			for (j = 0; j < len; j++) {
			    short   len2;

    			    if (!miio_get_short(&len2, 1, edr_flag,
						machine_code, fd))
				BADHEAD (lev,
					"bad specific part-len of src fld");
			    fea -> srcfields[i][j] = 
				calloc ((unsigned)(len2+1), sizeof (char));
			    if (!miio_get_char(fea->srcfields[i][j], (int)len2,
				edr_flag, machine_code, fd))
				BADHEAD (lev, 
					"bad specific part -fea srcfields");
			    fea -> srcfields[i][j][len2] = '\0';
			}
		    }
		}
		break;
	    }

	default: 
	    BADHEAD (lev, "unknown header type");
	    break;
    }

/* process variable part of the header */
    while (1) {
	if (!miio_get_short(&code, 1, edr_flag, machine_code, fd))
	    BADHEAD (lev, "read error: code");
	if (code < 0 || code > PT_MAX)
	    BADHEAD (lev, "illegal parameter code");
	if (!miio_get_short(&len, 1, edr_flag, machine_code, fd))
	    BADHEAD (lev, "bad parameter format");
/* at end of header we have code==PT_ENDPAR, len==0 */
	if (code == PT_ENDPAR)
	    break;
/* make sure parameter will fit */
	if (code != PT_HEADER && len > MAX_PARAM_LEN)
	    BADHEAD (lev, "optional parameter too long");

/* Do the read here for string parameters */
	if (code == PT_SOURCE || code == PT_TYPTXT
		|| code == PT_REFER || code == PT_COMMENT
		|| code == PT_GENHD || code == PT_CWD) {
	    if (!miio_get_char((char *)buf, (int)(len*SIZINT), 
		 edr_flag, machine_code, fd))
		BADHEAD (lev, "read error: opt");
	    buf[len] = 0;	/* ensure null termination */
	}
/* when adding file types, be sure to check this for valid options */
	switch (code) {
	    case PT_SOURCE: 
		p -> variable.source[ns++] = savestring ((char *) buf);
		break;
	    case PT_TYPTXT: 
		p -> variable.typtxt = savestring ((char *) buf);
		break;
	    case PT_COMMENT: 
		p -> variable.comment = savestring ((char *) buf);
		break;
	    case PT_CWD:
		p -> variable.current_path = savestring ((char *) buf);
		break;
	    case PT_REFER: 
		p -> variable.refer = savestring ((char *) buf);
		break;
	    case PT_PRE: 	/* pre_emp */
		switch (p -> common.type) {

#ifdef ESI
		    case FT_ANA: 
			if ((p->hd.ana->pre_emp = read_zfunc(lev, fd)) == NULL)
			    return NULL;
			break;
#endif ESI
		    case FT_SPEC: 
			if ((p->hd.spec->pre_emp = read_zfunc(lev, fd)) == NULL)
			    return NULL;
			break;
		    default: 
			BADHEAD (lev, "wrong zfunc");
			break;
		}
		break;
	    case PT_PREFILTER: 	/* prefilter */
		switch (p -> common.type) {

#ifdef ESI
		    case FT_PIT: 
			if ((p->hd.pit->prefilter = read_zfunc(lev, fd)) ==NULL)
			    return NULL;
			break;
#endif ESI

		    case FT_SD: 
			if ((p->hd.sd->prefilter = read_zfunc(lev, fd))==NULL)
			    return NULL;
			break;
		    default: 
			BADHEAD (lev, "wrong zfunc");
			break;
		}
		break;
	    case PT_LPF: 
		switch (p -> common.type) {

#ifdef ESI
		    case FT_PIT: 
			if ((p -> hd.pit -> lpf = read_zfunc (lev, fd)) == NULL)
			    return NULL;
			break;
#endif ESI

		    default: 
			BADHEAD (lev, "wrong zfunc");
			break;
		}
		break;
	    case PT_DEEMP: 	/* de_emp */
		switch (p -> common.type) {
		    case FT_SD: 
			if ((p -> hd.sd->de_emp = read_zfunc(lev, fd)) == NULL)
			    return NULL;
			break;
		    default: 
			BADHEAD (lev, "wrong zfunc");
			break;
		}
		break;
	    case PT_GENHD: 
		(void) read_genhd (lev, fd, p, (char *) buf);
		break;
	    case PT_HEADER: 
		if ((p->variable.srchead[nh]=recursive_rh(fd, lev+1)) == NULL)
		    return NULL;
		nh++;
		break;
	    case PT_REFHD:
		if ((p->variable.refhd = recursive_rh(fd, lev+1)) == NULL)
		    return NULL;
		break;
	    default: 
		BADHEAD (lev, "bad optional parameter type");
		break;
	}
    }
    p -> variable.nnames = ns;
    p -> variable.nheads = nh;
    return p;
}

/*
	Error messages for read_header and its subroutines are stuffed
	into head_errtxt by badhead.
*/
static  void
badhead (level, msg)
int     level;
char   *msg;
{
    (void) sprintf (head_errtxt,
	    "read_header: %s%s\n", msg, level ? " in included header" : "");
    if(EspsDebug)
    	Fprintf (stderr,
	    "read_header: %s%s\n", msg, level ? " in included header" : "");
}

/*
	f_copy copies a floating vector.
*/
static void
f_copy (dst, src, n)
float  *dst, *src;
int     n;
{
    while (n-- > 0)
	*dst++ = *src++;
}

/*
	divceil does integer division rounding upward
*/
#define divceil(j,k) ((j)+(k)-1) / (k)

/*
	fix_header repairs the following fields:
	fixpartsiz, hsize, check 
        and the type fields
	It calls itself recursively for included headers.
*/
static
fix_header (p)
struct header  *p;
{
    int     i, size, sumop = FIX_HEADER_SIZE;
    p -> common.fixpartsiz = FIX_HEADER_SIZE;
fprintf(stderr,"point b1\n");
    for (i = 0; i < MAX_SOURCES; i++) {
	if (p -> variable.srchead[i]) {
fprintf(stderr,"point b2\n");
	    fix_header (p -> variable.srchead[i]);
	    sumop += p -> variable.srchead[i] -> common.hsize;
	}
fprintf(stderr,"point b3\n");
	if (p -> variable.source[i]) {
	    sumop += divceil (strlen (p -> variable.source[i]) + 1, SIZINT);
	}
    }
fprintf(stderr,"point b4\n");
    if (p -> variable.refhd) {
	fix_header (p -> variable.refhd);
	sumop += p -> variable.refhd -> common.hsize;
    }
fprintf(stderr,"point b5\n");
    if (p -> variable.typtxt)
	sumop += divceil (strlen (p -> variable.typtxt) + 1, SIZINT);
    if (p -> variable.comment)
	sumop += divceil (strlen (p -> variable.comment) + 1, SIZINT);
    if (p -> variable.refer)
	sumop += divceil (strlen (p -> variable.refer) + 1, SIZINT);
fprintf(stderr,"point b6\n");
    for (i = 0; i < GENTABSIZ; i++) {
	struct gen_hd  *np;
	np = p -> variable.gentab[i];
	while (np != NULL) {
	    sumop += divceil (strlen (np -> name) + 1, SIZINT);
	    sumop += SIZINT + SIZSHO + np -> size * typesiz (np -> type);
	    np = np -> next;
	}
    }
    switch (p -> common.type) {

#ifdef ESI
	case FT_ANA: 
	    sumop += divceil (ANA_SIZE, sizeof (int));
	    sumop += SIZZF (p -> hd.ana -> pre_emp);
	    break;

	case FT_PIT: 
	    sumop += divceil (PIT_SIZE, sizeof (int));
	    sumop += SIZZF (p->hd.pit->prefilter) + SIZZF (p -> hd.pit -> lpf);
	    break;

	case FT_ROS: 
	    sumop += divceil (ROS_SIZE, sizeof (int));
	    if (p -> hd.ros -> type_bits != NULL)
		sumop += SIZSHO * p -> hd.ros -> maxtype;
	    if (p -> hd.ros -> rc_ubits != NULL)
		sumop += SIZSHO*p->hd.ros->order_unvcd;
	    if (p -> hd.ros -> rc_vbits != NULL)
		sumop += SIZSHO*p->hd.ros->order_vcd;
	    if (p -> hd.ros -> pulse_bits != NULL)
		sumop += SIZSHO * p -> hd.ros -> maxpulses;
	    if (p -> hd.ros -> pow_bits != NULL)
		sumop += SIZSHO * p -> hd.ros -> maxpow;
	    break;
#endif ESI

	case FT_SD: 
	    sumop += divceil (SD_SIZE, sizeof (int));
	    sumop += SIZZF(p -> hd.sd -> prefilter) + SIZZF(p->hd.sd->de_emp);
	    break;

	case FT_SPEC: 
	    sumop += divceil (SPEC_SIZE, sizeof (int));
	    sumop += SIZZF (p -> hd.spec -> pre_emp);
	    break;

	case FT_FILT: 
	    sumop += divceil (FILT_SIZE, sizeof (int));
	    if (p -> hd.filt -> bandedges != NULL)
		sumop += SIZFLT * p -> hd.filt -> nbands * 2;
	    if (p -> hd.filt -> points != NULL)
		sumop += SIZFLT * p -> hd.filt -> npoints;
	    if (p -> hd.filt -> gains != NULL && 
		p -> hd.filt -> func_spec == BAND)
		sumop += SIZFLT * p -> hd.filt -> nbands;
	    if (p -> hd.filt -> gains != NULL &&
	 	p -> hd.filt -> func_spec == POINT)
		sumop += SIZFLT * p -> hd.filt -> npoints;
	    if (p -> hd.filt -> wts != NULL && 
		p -> hd.filt -> func_spec == BAND)
		sumop += SIZFLT * p -> hd.filt -> nbands;
	    if (p -> hd.filt -> wts != NULL && 
		p -> hd.filt -> func_spec == POINT)
		sumop += SIZFLT * p -> hd.filt -> npoints;
	    break;

	case FT_SCBK: 
	    sumop += divceil (SCBK_SIZE, sizeof (int));
	    break;

	case FT_FEA: 
fprintf(stderr,"point b7\n");
	    size = p -> hd.fea -> field_count;
	    sumop += divceil (FEA_SIZE, sizeof (int));
	    sumop += size * ((2*SIZLO) + (3*SIZSHO));
	    for (i = 0; i < size; i++) {
fprintf(stderr,"point b8\n");
fprintf(stderr,"p: %x\n",p);
fprintf(stderr,"p->hd.fea: %x\n",p->hd.fea);
fprintf(stderr,"p->hd.fea->names: %x\n",p->hd.fea->names);
fprintf(stderr,"names: %x\n",p->hd.fea->names[i]);
fprintf(stderr,"name: %s\n",p->hd.fea->names[i]);
fprintf(stderr,"ranks: %x\n",p->hd.fea->ranks[i]);
		sumop += strlen (p -> hd.fea -> names[i]);
		sumop += SIZLO * p -> hd.fea -> ranks[i];
fprintf(stderr,"point b9\n");
		if (p -> hd.fea -> types[i] == CODED) {
		    char  **s = p -> hd.fea -> enums[i];
		    while (s && *s) {
			sumop += strlen (*s) + SIZSHO;
			s++;
		    }
		}
		{
		    char  **s = p -> hd.fea -> srcfields[i];
fprintf(stderr,"point b10\n");
		    if (s != NULL) while (*s) {
			sumop += strlen (*s) + SIZSHO;
			s++;
		    }
		}
	    }
fprintf(stderr,"point b11\n");
	    break;

    }

/* if any optional parameters, we need a zero at the end */
    if (sumop > p -> common.fixpartsiz)
	sumop++;
    p -> common.hsize = sumop;
    p -> common.check = HD_CHECK_VAL;
}


/* Assume the 4 bytes starting at address "ptr" are the
 * EDR (most-significant-byte-first) representation of
 * a long.  Return the value of the long.
 */
static long
edr_bytes_to_long(ptr)
    char    *ptr;
{
    unsigned long   item;
    unsigned long   mask = 0xff;

    item = ((ptr[3] & mask)
	    | (ptr[2] & mask) << 8
	    | (ptr[1] & mask) << 16
	    | (ptr[0] & mask) << 24);

    return (((item & 0x80000000) == 0) ? item
	    : -1L - (long) (item ^ 0xffffffff));
}


/* Assume the 4 bytes starting at address "ptr" are the
 * "byte-reversed EDR" (least-significant-byte-first)
 * representation of a long.  Return the value of the long.
 */
static long
lsbf_bytes_to_long(ptr)
    char    *ptr;
{
    unsigned long   item;
    unsigned long   mask = 0xff;

    item = ((ptr[0] & mask)
	    | (ptr[1] & mask) << 8
	    | (ptr[2] & mask) << 16
	    | (ptr[3] & mask) << 24);

    return (((item & 0x80000000) == 0) ? item
	    : -1L - (long) (item ^ 0xffffffff));
}


static int update_date = 1;	/* if 1 then write_header will update
				   date in the header */

/* this function is called to cause the next write_header not to update
   the date in the file header.  The flag is reset by write_header 
*/

void
inhibit_hdr_date()
{
	update_date = 0;
}


/*	write_header writes out a header.  It calls recursive_wh to
        do all the work.   write_header just sets the the date and
        version field of the outmost header
*/
void
write_header (p, fd)
FILE * fd;
struct header  *p;
{
#define PWD_SIZE 200

#ifdef DEC_ALPHA
    time_t tloc;
#else
    long    tloc; 
#endif
#ifndef DEC_ALPHA
    long    time ();
#endif
#if !defined(hpux) && !defined(OS5)
    char   *getwd(); 
#endif
    char   *ctime (), *user, *getlogin();
    struct passwd *pass, *getpwnam(), *getpwuid();
    int i, userid;
    char *pwd, *pwd2;		
    struct preamble *pre = (struct preamble *)calloc(1, sizeof(*pre));
    struct header *ref_hold_hd[MAX_SOURCES];
    char *ref_hold_src[MAX_SOURCES];
    int ref_nnames, ref_nheads;
    int foreign_hd_length;
#ifndef NO_LIC
       check_header(MACH_CODE);
#endif
    edr_flag = p->common.edr;
    if(getenv("ESPSDEBUG"))
	EspsDebug=1;

    if(EspsDebug)
	Fprintf(stderr,"write_header: edr: %d\n",edr_flag);

    if(update_date) {
        tloc = time (0);
        (void) strcpy (p -> common.date, ctime (&tloc));
        p -> common.date[24] = ' ';
    }
    else
	update_date = 1;
    if(EspsDebug)
	Fprintf(stderr,"write_header: point a\n");

    if (!(user = getlogin())  && (userid = getuid()) ) {
	pass = getpwuid(userid);
	if(pass) user = pass->pw_name;
    }
    if(EspsDebug)
	Fprintf(stderr,"write_header: point b\n");
    if(user) (void)strncpy(p->common.user,user,USERSIZ);
    p->common.user[USERSIZ-1] = '\0';
    pwd = malloc((unsigned)PWD_SIZE);	/* buffer for gethostname */
    spsassert(pwd,"write_header: malloc failed!");
    (void)gethostname(pwd,PWD_SIZE);
    (void)strcat(pwd,":");
    if(EspsDebug)
	Fprintf(stderr,"write_header: point bb\n");
    pwd2 = malloc((unsigned)PWD_SIZE);	/* buffer for getwd */
    spsassert(pwd2,"write_header: malloc failed!");
#if !defined(hpux) && !defined(OS5) && !defined(LINUX_OR_MAC)
    if(getwd(pwd2) == NULL)
#else
    if(getcwd(pwd2,PWD_SIZE) == NULL)
#endif
	(void)strcpy(pwd2,"Couldn't get directory name");
    (void)strcat(pwd,pwd2);
    if(EspsDebug)
	Fprintf(stderr,"write_header: point bc\n");
    p->variable.current_path = savestring(pwd);
    free(pwd); free(pwd2);
    if (p->variable.refhd) {
	struct header *h = p->variable.refhd;
/*
 For a reference header, hold all the embedded source headers.  We don't
 want to write them out, but we can't just zap them here, because it
 alters the header in memory.
*/
        if(EspsDebug)
	  Fprintf(stderr,"write_header: point c\n");

       	/* zap all the source files and headers */
       	for (i = 0; i < MAX_SOURCES; i++) {
	  ref_hold_hd[i] = h->variable.srchead[i];
	  ref_hold_src[i] = h->variable.source[i];
	  h -> variable.source[i] = NULL;
	  h -> variable.srchead[i] = NULL;
        }
	ref_nnames = h->variable.nnames;
	ref_nheads = h->variable.nheads;
	h -> variable.nnames = h -> variable.nheads = 0;
        if(EspsDebug)
	  Fprintf(stderr,"write_header: point d\n");
    }

#ifndef NOPAD
/* see if we should write feature files in "field_order", instead of
   by data type as is normal.   If the env variable FIELD_ORDER is
   defined, but not equal to "off", then do this.
*/
    if(EspsDebug)
	Fprintf(stderr,"write_header: checking field order\n");

   if(p->common.type == FT_FEA) {
	char *ptr = getenv("FIELD_ORDER");
	if(ptr && strcmp(ptr,"off") != 0) {
	   p->hd.fea->field_order = YES;
	   Fprintf(stderr,
	    "write_header: FIELD_ORDER environment variable is set\n");
	}
   }
#endif



/* call function to fix up the type fields in the header.  Note that
   we don't do this for Feature Files.   See add_fea_fld
*/

#ifdef ESI
    if (p -> common.type == FT_ANA)
	(void) set_ana_type (p);
    if (p -> common.type == FT_PIT)
	(void) set_pit_type (p);
    if (p -> common.type == FT_ROS)
	(void) set_ros_type (p);
#endif ESI

    if (p -> common.type == FT_SPEC)
	(void) set_spec_type (p);
    if (p -> common.type == FT_FILT)
	(void) set_filt_type (p);
    if (p -> common.type == FT_SCBK)
	(void) set_scbk_type (p);

/* write out the preamble the first time, without knowing the data
   offset
*/
    pre->machine_code = MACH_CODE;
    p->common.machine_code = pre->machine_code;
    pre->check_code = 3000;
    pre->record_size = size_rec2(p);
    pre->check = HD_CHECK_VAL;
    pre->edr = p->common.edr;
    pre->data_offset = 0;

    if(EspsDebug)
	Fprintf(stderr,"write_header: writing preamble\n");

#ifndef DEC_ALPHA
    if (!miio_put_long(&pre->machine_code, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->check_code, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->data_offset, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->record_size, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->check, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->edr, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->align_pad_size, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->foreign_hd, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
#else
    if (!miio_put_int(&pre->machine_code, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->check_code, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->data_offset, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->record_size, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->check, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->edr, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->align_pad_size, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->foreign_hd, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
#endif


/* write out all headers 
*/
    if(EspsDebug)
	Fprintf(stderr,"write_header: calling recursive_wh\n");
    recursive_wh (p, fd);

/* if there is a foreign header, then write it out here */


    if((foreign_hd_length = get_genhd_val("foreign_hd_length", p, 0.0)) > 0) {

/* the above is supposed to be = not == */
/* there is a foreign header (generic defined and non-zero),  if
   the generic foreign_hd_ptr is defined and non-null, then write 
   out that data after the esps header.   We are using a long generic
   too hold a char * here.
*/
	long *foreign_hd_ptr;
        pre->foreign_hd = -1;
	if((foreign_hd_ptr=get_genhd_l("foreign_hd_ptr",p)) && *foreign_hd_ptr){
	    pre->foreign_hd = ftell(fd);
	    if (!fwrite ((char *)(*foreign_hd_ptr), 1, foreign_hd_length, fd))
		hd_error ("write_header - foreign header");
	} else {
	    char *foreign_hd_ptr = malloc((unsigned)foreign_hd_length);
	    fprintf(stderr,
	    "write_header: Warning, foreign_hd_length is non-zero (%d), but\n",
	    foreign_hd_length);
	    fprintf(stderr,
	    "write_header: foreign_hd_ptr is undefined or NULL\n");
	    pre->foreign_hd = ftell(fd);
	    if (!fwrite (foreign_hd_ptr, 1, foreign_hd_length, fd))
		hd_error ("write_header - foreign header");
	    free(foreign_hd_ptr);
	}
    }


    pre->data_offset = ftell(fd);	/* get the data offset */

/* if possible, rewind the output and update the preamble
*/

    if (fseek(fd, 0L, 0) == 0) {	/* its not a pipe */

#if defined(SUN3) || defined(SUN4) || defined(SUN386i) 
	if (EspsDebug) Fprintf(stderr,"write_header: data_offset: %x\n",
		pre->data_offset);
    	if ( getenv("ESPS_ALIGN") && ((pre->data_offset & 0xf) != 0)) { 
		char *pad;
/* means that the start of data is not double word aligned
*/
    		(void)fseek(fd, pre->data_offset, 0);
		pre->align_pad_size = 0x10 - (pre->data_offset & 0xf);
		pad = malloc((unsigned)pre->align_pad_size);
		miio_put_char(pad,pre->align_pad_size,pre->edr,fd);
		pre->data_offset = pre->data_offset + pre->align_pad_size;
		free(pad);
		assert(pre->data_offset == ftell(fd));
		(void)fseek(fd, 0L, 0);
		if (EspsDebug) {
		 Fprintf(stderr,
		 "write_header: align check: required pad size is 0x%x\n", 
		 pre->align_pad_size);
		 Fprintf(stderr,
		 "write_header: new data offset is 0x%x\n", pre->data_offset);
		}
	
	} else
#endif
	{
		if(EspsDebug)
		 Fprintf(stderr,"write_header: no pad needed\n");
	}
		

/*
    (void)fprintf(stderr,"write_header: just rewound output file\n");
    (void)fprintf(stderr,"write_header: data offset is %ld\n",pre->data_offset);
*/
#ifndef DEC_ALPHA
    if (!miio_put_long(&pre->machine_code, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->check_code, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->data_offset, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->record_size, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->check, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->edr, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->align_pad_size, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_long(&pre->foreign_hd, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
#else
    if (!miio_put_int(&pre->machine_code, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->check_code, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->data_offset, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->record_size, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->check, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->edr, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->align_pad_size, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
    if (!miio_put_int(&pre->foreign_hd, 1, (int)pre->edr, fd))
	hd_error("error writing preamble 1");
#endif

    (void)fseek(fd, pre->data_offset, 0);
    }

/* restore the subheaders to the reference header
*/
    if (p->variable.refhd) {
	struct header *h = p->variable.refhd;

       	/* zap all the source files and headers */
       	for (i = 0; i < MAX_SOURCES; i++) {
	  h->variable.srchead[i] = ref_hold_hd[i];
	  h->variable.source[i] = ref_hold_src[i];
        }
	h->variable.nnames = ref_nnames;
	h->variable.nheads = ref_nheads;
    }

    free(pre);
    
}
/*
	rec_write_header writes out a header. It calls fix_header to repair
	some fields.
*/
static void
recursive_wh (p, fd)
FILE * fd;
struct header  *p;
{
    int     i;
    short   code, len;
    char I[4];
    long dummy=0;

    fix_header (p);
    I[0] = '\0';		/* dirty trick to keep sccs from */
    (void)strcpy(I,"%");		/* killing the % I % , symbol */
    (void)strcat(I,"I%");
/*
 if the header version define (from header.h) is equal to percentIpercent
 then that means that header.h is an sccs edit version.   Substitute a large
 sccs version instead of putting the sccs keyword into the header
*/
    if(strcmp(I,HD_VERSION) != 0)
        (void) strcpy (p -> common.hdvers, HD_VERSION);
    else
        (void) strcpy (p -> common.hdvers, "999");

    if (EspsDebug)
	fprintf(stderr,"write header: header version: %s\n",p->common.hdvers);
	

    if ((p -> common.type == FT_SD) && (get_sd_type (p) == NONE))
	(void) fprintf (stderr,
		"write_header: Warning - type fields for SD file not set.");
    if (p->common.type == FT_SPEC && p -> hd.spec -> freq_format == ARB_FIXED) {
	(void ) fprintf (stderr, 
		"ARB_FIXED type not implemented yet -- seek help.\n");
	exit (1);
    }
    if (miio_put_short(&p->common.type, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
#ifndef NOPAD
    if (miio_put_short(&p->common.pad1, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
#endif
    if (miio_put_long(&p->common.check, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_char(p->common.date, DATESIZE, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_char(p->common.hdvers, VERSIONSIZE, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_char(p->common.prog, PROGSIZE, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_char(p->common.vers, VERSIONSIZE, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_char(p->common.progdate, DATESIZE, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_long(&p->common.ndrec, 1, edr_flag,fd) == 0)
	hd_error ("write_header");
    if (miio_put_short(&p->common.tag, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_short(&p->common.nd1, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_long(&p->common.ndouble, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_long(&p->common.nfloat, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_long(&p->common.nlong, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_long(&p->common.nshort, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_long(&p->common.nchar, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_long(&p->common.fixpartsiz, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_long(&p->common.hsize, 1, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_char(p->common.user, USERSIZ, edr_flag, fd) == 0)
	hd_error ("write_header");
    if (miio_put_short(p->common.spares, NSPARES, edr_flag, fd) == 0)
	hd_error ("write_header");
    switch (p -> common.type) {

#ifdef ESI
	case FT_ANA: 
	    if (fwrite ((char *) p -> hd.ana, ANA_SIZE, 1, fd) == 0)
		hd_error ("write_header");
	    break;
	case FT_PIT: 
	    if (fwrite ((char *) p -> hd.pit, PIT_SIZE, 1, fd) == 0)
		hd_error ("write_header");
	    break;
	case FT_ROS: 
	    {
		struct ros_header  *ros;
		if (fwrite ((char *) p -> hd.ros, ROS_SIZE, 1, fd) == 0)
		    hd_error ("write_header");
		ros = p -> hd.ros;
		(void) put_shorts (fd, ros -> type_bits, ros -> maxtype);
		(void) put_shorts (fd, ros -> rc_ubits, ros -> order_unvcd);
		(void) put_shorts (fd, ros -> rc_vbits, ros -> order_vcd);
		(void) put_shorts (fd, ros -> pulse_bits, ros -> maxpulses);
		(void) put_shorts (fd, ros -> pow_bits, ros -> maxpow);
		break;
	    }
#endif ESI

	case FT_SD: 
	    if (miio_put_short(&p->hd.sd->equip, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
#ifndef NOPAD
	    if (miio_put_short(&p->hd.sd->pad1, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
#endif
	    if (miio_put_float(&p->hd.sd->max_value, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_float(&p->hd.sd->sf, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_float(&p->hd.sd->src_sf, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.sd->synt_method, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
#ifndef NOPAD
	    if (miio_put_short(&p->hd.sd->pad2, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
#endif
	    if (miio_put_float(&p->hd.sd->scale, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_float(&p->hd.sd->dcrem, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.sd->q_method, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.sd->v_excit_method, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.sd->uv_excit_method, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.sd->spare1, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.sd->nchan, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.sd->synt_interp, 1, edr_flag, fd) == 0) 
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.sd->synt_pwr, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.sd->synt_rc, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.sd->synt_order, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
#ifndef NOPAD
	    if (miio_put_short(&p->hd.sd->pad3, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
#endif
	    if (miio_put_long(&p->hd.sd->start, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_long(&p->hd.sd->nan, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(p->hd.sd->spares, SD_SPARES, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_long(&dummy, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_long(&dummy, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    break;
	case FT_SPEC: 
	    if (miio_put_long(&p->hd.spec->start, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_long(&p->hd.spec->nan, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.spec->frmlen, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.spec->order_vcd, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.spec->order_unvcd, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.spec->win_type, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_float(&p->hd.spec->sf, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.spec->spec_an_meth, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
#ifndef NOPAD
	    if (miio_put_short(&p->hd.spec->pad1, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
#endif
	    if (miio_put_float(&p->hd.spec->dcrem, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.spec->post_proc, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.spec->frame_meth, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.spec->voicing, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.spec->freq_format, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.spec->spec_type, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(&p->hd.spec->contin, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_long(&p->hd.spec->num_freqs, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_short(p->hd.spec->spares, SPEC_SPARES, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_long(&dummy, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    if (miio_put_long(&dummy, 1, edr_flag, fd) == 0)
		hd_error ("write_header");
	    break;
	case FT_FILT: 
	    {
		struct filt_header *filt;
		filt = p -> hd.filt;
	        if (miio_put_short(&p->hd.filt->max_num, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
	        if (miio_put_short(&p->hd.filt->max_den, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
	        if (miio_put_short(&p->hd.filt->func_spec, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
	        if (miio_put_short(&p->hd.filt->nbands, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
	        if (miio_put_short(&p->hd.filt->npoints, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
	        if (miio_put_short(&p->hd.filt->g_size, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
	        if (miio_put_short(&p->hd.filt->nbits, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
	        if (miio_put_short(&p->hd.filt->type, 1, edr_flag, fd) == 0) 
		    hd_error ("write_header");
	        if (miio_put_short(&p->hd.filt->method, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
	        if (miio_put_short(p->hd.filt->spares, FILT_SPARES, edr_flag, fd) == 0)
		    hd_error ("write_header");
#ifndef NOPAD
	        if (miio_put_short(&p->hd.filt->pad1, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
#endif
	        if (miio_put_long(&dummy, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
	        if (miio_put_long(&dummy, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
	        if (miio_put_long(&dummy, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");
	        if (miio_put_long(&dummy, 1, edr_flag, fd) == 0)
		    hd_error ("write_header");


		if (filt -> nbands != 0)
		    if(!miio_put_float(filt->bandedges, 2*filt -> nbands, 
			edr_flag, fd))
		      hd_error ("write_header");
		if (filt -> npoints != 0)
		    if(!miio_put_float(filt -> points, filt -> npoints,
			edr_flag, fd))
		      hd_error ("write_header");
		if (filt -> func_spec == BAND && filt -> nbands != 0)
		    if(!miio_put_float(filt -> gains, filt -> nbands,
			edr_flag, fd))
		      hd_error ("write_header");
		if (filt -> func_spec == POINT && filt -> npoints != 0)
		    if(!miio_put_float(filt -> gains, filt -> npoints,
			edr_flag, fd))
		      hd_error ("write_header");
		if (filt -> func_spec == BAND && filt -> nbands != 0)
		    if(!miio_put_float(filt -> wts, filt -> nbands,
			edr_flag, fd))
		      hd_error ("write_header");
		if (filt -> func_spec == POINT && filt -> npoints != 0)
		    if(!miio_put_float(filt -> wts, filt -> npoints,
			edr_flag, fd))
		      hd_error ("write_header");
	    }
	    break;
	case FT_SCBK: 
	    if (!miio_put_long(&p->hd.scbk->num_items, 1, edr_flag, fd))
		hd_error ("write_header");
	    if (!miio_put_short(&p->hd.scbk->distortion, 1, edr_flag, fd))
		hd_error ("write_header");
	    if (!miio_put_short(&p->hd.scbk->num_cdwds, 1, edr_flag, fd))
		hd_error ("write_header");
	    if (!miio_put_float(&p->hd.scbk->convergence, 1, edr_flag, fd))
		hd_error ("write_header");
	    if (!miio_put_short(&p->hd.scbk->codebook_type, 1, edr_flag, fd))
		hd_error ("write_header");
	    if (!miio_put_short(&p->hd.scbk->element_num, 1, edr_flag, fd))
		hd_error ("write_header");
	    if (!miio_put_short(p->hd.scbk->spares, SCBK_SPARES, edr_flag, fd))
		hd_error ("write_header");
	    break;
	case FT_FEA: {
		struct fea_header  *fea = p -> hd.fea;
		long dummy[9];
/* do a check here
*/
		assert(p->common.ndouble >= fea->ndouble);
		assert(p->common.nfloat >= fea->nfloat);
		assert(p->common.nlong >= fea->nlong);
		assert(p->common.nshort >= fea->nshort);
		assert(p->common.nchar >= fea->nbyte);
		if (fea->field_count == 0)
		    hd_error("cannot write null fea header");
		if (!miio_put_short(&fea->fea_type, 1, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_short(&fea->segment_labeled, 1, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_short(&fea->field_count, 1, edr_flag, fd))
		    hd_error("write_header");
#ifndef NOPAD
		if (!miio_put_short(&(fea->field_order), 1, edr_flag, fd))
		    hd_error("write_header");
#endif
		if (!miio_put_long(dummy, 9, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_short(fea->spares, FEA_SPARES, edr_flag, fd))
		    hd_error("write_header");
		put_longs (fd, fea -> sizes, (int)fea -> field_count);
		put_longs (fd, fea -> starts, (int)fea -> field_count);
		put_shorts (fd, fea -> ranks, (int)fea -> field_count);
		put_shorts (fd, fea -> types, (int)fea -> field_count);
/* write out feature type size information
*/
		if (!miio_put_long(&fea->ndouble, 1, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_long(&fea->nfloat, 1, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_long(&fea->nlong, 1, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_long(&fea->nshort, 1, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_long(&fea->nbyte, 1, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_long(&fea->ndcplx, 1, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_long(&fea->nfcplx, 1, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_long(&fea->nlcplx, 1, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_long(&fea->nscplx, 1, edr_flag, fd))
		    hd_error("write_header");
		if (!miio_put_long(&fea->nbcplx, 1, edr_flag, fd))
		    hd_error("write_header");
		  
		put_shorts (fd, fea -> derived, (int)fea -> field_count);
		for (i = 0; i < fea -> field_count; i++) {
		    short   len;
		    len = strlen (fea->names[i]);
		    if (!miio_put_short(&len, 1, edr_flag, fd) ||
		        !miio_put_char(fea->names[i], len, edr_flag, fd))
			    hd_error("write_header");
		    if (fea->ranks[i] != 0)
			    if(!miio_put_long(fea -> dimens[i], 
				(int)fea -> ranks[i], edr_flag, fd))
			        hd_error ("write_header");
		    if (fea -> types[i] == CODED) {
			char   **str;
			short   j;
			str = fea -> enums[i];
			for (j = 0; str && *str++ != NULL; j++);;
			str = fea -> enums[i];
			put_shorts (fd, &j, 1);
			while (str && *str != NULL) {
			    short   len = strlen (*str);
			    if (!miio_put_short(&len, 1, edr_flag, fd) ||
				    !miio_put_char(*str++, (int)len, edr_flag, 
			            fd))
				hd_error ("write_header");
			}
		    }
		    if (fea->derived[i] != 0) {
		    /* write out the derived field information */
			char   **str;
			short   j;
			str = fea -> srcfields[i];
			for (j = 0; *str++ != NULL; j++);;
			str = fea -> srcfields[i];
			put_shorts (fd, &j, 1);
			while (*str != NULL) {
			    short   len = strlen (*str);
			    if (!miio_put_short(&len, 1, edr_flag, fd) ||
				    !miio_put_char(*str++, (int)len, edr_flag, 
				     fd))
				hd_error ("write_header");
			}
		    }
		    else {
			short j=0;
			put_shorts (fd, &j, 1);
		    }
		}
		break;
	    }
    }

    if (p -> common.fixpartsiz == p -> common.hsize)
	return;
/* write optional parameters. */
    if (p -> variable.ngen != 0) {
	struct gen_hd  *np;
	int     i;
	for (i = 0; i < GENTABSIZ; i++) {
	    np = p -> variable.gentab[i];
	    while (np != NULL) {
		write_genhd (PT_GENHD, np, fd);
		np = np -> next;
	    }
	}
    }
    if (p -> variable.typtxt)
	write_par (PT_TYPTXT, p -> variable.typtxt, fd);
    if (p -> variable.refer)
	write_par (PT_REFER, p -> variable.refer, fd);
    if (p -> variable.comment)
	write_par (PT_COMMENT, p -> variable.comment, fd);
    if (p -> variable.current_path)
	write_par (PT_CWD, p -> variable.current_path, fd);
    for (i = 0; p -> variable.source[i] && i < MAX_SOURCES; i++)
	write_par (PT_SOURCE, p -> variable.source[i], fd);
    code = PT_HEADER;
    for (i = 0; p -> variable.srchead[i] && i < MAX_SOURCES; i++) {
	(void) miio_put_short(&code, 1, edr_flag, fd);
	len = p -> variable.srchead[i] -> common.hsize;
	(void) miio_put_short(&len, 1, edr_flag, fd);
	recursive_wh (p -> variable.srchead[i], fd);
    }
    code = PT_REFHD;
    if (p -> variable.refhd) {
	(void) miio_put_short(&code, 1, edr_flag, fd);
	len = p -> variable.refhd -> common.hsize;
	(void) miio_put_short(&len, 1, edr_flag, fd);
	recursive_wh (p-> variable.refhd, fd);
    }

    switch (p -> common.type) {
#ifdef ESI
	case FT_ANA: 
	    if (p -> hd.ana -> pre_emp)
		write_zfunc (PT_PRE, p -> hd.ana -> pre_emp, fd);
	    break;

	case FT_PIT: 
	    if (p -> hd.pit -> lpf)
		write_zfunc (PT_LPF, p -> hd.pit -> lpf, fd);
	    if (p -> hd.pit -> prefilter)
		write_zfunc (PT_PREFILTER, p -> hd.pit -> prefilter, fd);
	    break;
#endif ESI

	case FT_SD: 
	    if (p -> hd.sd -> prefilter)
		write_zfunc (PT_PREFILTER, p -> hd.sd -> prefilter, fd);
	    if (p -> hd.sd -> de_emp)
		write_zfunc (PT_DEEMP, p -> hd.sd -> de_emp, fd);
	    break;

	case FT_SPEC: 
	    if (p -> hd.spec -> pre_emp)
		write_zfunc (PT_PRE, p -> hd.spec -> pre_emp, fd);
	    break;
    }
/* add zeros at end */
    putnulls (SIZINT, fd);
}

/*
	new_header creates a new, all-zero header structure.
*/
struct header  *
new_header (type)
int     type;
{
    struct header  *p = (struct header *) calloc (1, sizeof *p);

    if (getenv("ESPS_EDR") && !strcmp(getenv("ESPS_EDR"),"on"))
	p->common.edr = YES;
    else
	p->common.edr = NO;
    p->common.machine_code = MACH_CODE;
    if (type == 0)
	return p;
    switch (type) {
#ifdef ESI
	case FT_ANA: 
	    p -> hd.ana = (struct ana_header   *)
	                                        calloc (1, sizeof *p -> hd.ana);
	    break;
	case FT_PIT: 
	    p -> hd.pit = (struct pit_header   *)
	                                        calloc (1, sizeof *p -> hd.pit);
	    break;
	case FT_ROS: 
	    p -> hd.ros = (struct ros_header   *)
	                                    calloc (1, sizeof *p -> hd.ros);
	    break;
#endif ESI
	case FT_SD: 
	    p -> hd.sd = (struct sd_header *)
	                                    calloc (1, sizeof *p -> hd.sd);
	    break;
	case FT_SPEC: 
	    p -> hd.spec = (struct spec_header *)
	                                    calloc (1, sizeof *p->hd.spec);
	    break;
	case FT_FILT: 
	    p -> hd.filt = (struct filt_header *)
	                                    calloc (1, sizeof *p->hd.filt);
	    break;
	case FT_SCBK: 
	    p -> hd.scbk = (struct scbk_header *)
	                                    calloc (1, sizeof *p->hd.scbk);
	    break;
	case FT_FEA:
	    p -> hd.fea = (struct fea_header *)
					    calloc (1, sizeof *p->hd.fea);
/* initialize the size parameters to zero
*/
	    break;
	default: 
	    BADHEAD (0, "new_header: bad type");

    }
    p -> common.type = type;
    return p;
}
/* 
	copy_header makes a copy of a header
*/
struct header  *
copy_header (src)
struct header  *src;
{
    int     i, j;

    struct header  *p = (struct header *) malloc (sizeof *p);
    *p = *src;
    if (getenv("ESPS_EDR") && !strcmp(getenv("ESPS_EDR"),"on"))
	p->common.edr = YES;
    else
	p->common.edr = NO;
    switch (p -> common.type) {
#ifdef ESI
	case FT_ANA: 
	    p -> hd.ana = (struct ana_header   *) malloc (sizeof *p -> hd.ana);
	    *p -> hd.ana = *src -> hd.ana;
	    p -> hd.ana -> pre_emp = copy_zfunc (src -> hd.ana -> pre_emp);
	    break;

	case FT_PIT: 
	    p -> hd.pit = (struct pit_header   *) malloc (sizeof *p -> hd.pit);
	    *p -> hd.pit = *src -> hd.pit;
	    p -> hd.pit -> prefilter = copy_zfunc (src -> hd.pit -> prefilter);
	    p -> hd.pit -> lpf = copy_zfunc (src -> hd.pit -> lpf);
	    break;

	case FT_ROS: 
	    p -> hd.ros = (struct ros_header   *) malloc (sizeof *p -> hd.ros);
	    *p -> hd.ros = *src -> hd.ros;
	    p -> hd.ros -> type_bits = copy_shorts (src -> hd.ros -> type_bits,
		    p -> hd.ros -> maxtype);
	    p -> hd.ros -> rc_ubits = copy_shorts (src -> hd.ros -> rc_ubits,
		     p -> hd.ros -> order_unvcd);
	    p -> hd.ros -> rc_vbits = copy_shorts (src -> hd.ros -> rc_vbits,
		     p -> hd.ros -> order_vcd);
	    p -> hd.ros -> pulse_bits = copy_shorts(src -> hd.ros -> pulse_bits,
		    p -> hd.ros -> maxpulses);
	    p -> hd.ros -> pow_bits = copy_shorts (src -> hd.ros -> pow_bits,
		    p -> hd.ros -> maxpow);
	    break;
#endif ESI

	case FT_SD: 
	    p -> hd.sd = (struct sd_header *) malloc (sizeof *p -> hd.sd);
	    *p -> hd.sd = *src -> hd.sd;
	    p -> hd.sd -> prefilter = copy_zfunc (src -> hd.sd -> prefilter);
	    p -> hd.sd -> de_emp = copy_zfunc (src -> hd.sd -> de_emp);
	    break;

	case FT_SPEC: 
	    p -> hd.spec = (struct spec_header *) malloc (sizeof *p -> hd.spec);
	    *p -> hd.spec = *src -> hd.spec;
	    p -> hd.spec -> pre_emp = copy_zfunc (src -> hd.spec -> pre_emp);
	    break;


	case FT_FILT: 
	    p -> hd.filt = (struct filt_header *) malloc (sizeof *p -> hd.filt);
	    *p -> hd.filt = *src -> hd.filt;
	    p -> hd.filt -> bandedges = copy_floats(src -> hd.filt -> bandedges,
		    p -> hd.filt -> nbands * 2);
	    p -> hd.filt -> points = copy_floats (src -> hd.filt -> points,
		    p -> hd.filt -> npoints);
	    if (p -> hd.filt -> func_spec == BAND)
		p -> hd.filt -> gains = copy_floats (src -> hd.filt -> gains,
			p -> hd.filt -> nbands);
	    if (p -> hd.filt -> func_spec == POINT)
		p -> hd.filt -> gains = copy_floats (src -> hd.filt -> gains,
			p -> hd.filt -> npoints);
	    if (p -> hd.filt -> func_spec == BAND)
		p -> hd.filt -> wts = copy_floats (src -> hd.filt -> wts,
			p -> hd.filt -> nbands);
	    if (p -> hd.filt -> func_spec == POINT)
		p -> hd.filt -> wts = copy_floats (src -> hd.filt -> wts,
			p -> hd.filt -> npoints);
	    break;

	case FT_SCBK: 
	    p -> hd.scbk = (struct scbk_header *) malloc (sizeof *p -> hd.scbk);
	    *p -> hd.scbk = *src -> hd.scbk;
	    break;

	case FT_FEA: {
		unsigned int     size = src -> hd.fea -> field_count;
		p -> hd.fea = (struct fea_header *)malloc(sizeof *p -> hd.fea);
		*p -> hd.fea = *src -> hd.fea;

#ifndef NOPAD
/* clear the field_order flag; default is always NO
*/
		p -> hd.fea -> field_order = NO;
#endif

		p -> hd.fea -> sizes = copy_longs(src -> hd.fea -> sizes, 
				(int)size);
		p -> hd.fea -> starts = copy_longs(src -> hd.fea -> starts, 
				(int)size);
		p -> hd.fea -> ranks = copy_shorts(src ->hd.fea -> ranks, 
				(int)size);
		p -> hd.fea -> types = copy_shorts(src->hd.fea -> types, 
				(int)size);
/* 
  if old version feature file don't try to copy derived field,
  just allocate some memory for it for safety
*/
		if (!old_version(p->common.hdvers,PRE_DERIVED)){
		    p->hd.fea->derived = copy_shorts(src->hd.fea -> derived, 
				(int)size);
		}
		else	
		    p->hd.fea->derived = (short *)calloc(size, sizeof(short));
		p->hd.fea->names = (char **)calloc(size+1, sizeof (char *));
		p->hd.fea->dimens = (long **)calloc(size, sizeof (long *));
		p->hd.fea->enums = (char ***)calloc(size, sizeof (char **));
		p->hd.fea->srcfields = (char ***)calloc(size, sizeof (char **));
		for (i = 0; i < size; i++) {
		    p -> hd.fea ->names[i]=savestring(src->hd.fea->names[i]);
		    p -> hd.fea ->dimens[i]=copy_longs(src->hd.fea->dimens[i],
			    src -> hd.fea -> ranks[i]);
		    if (src -> hd.fea -> types[i] != CODED)
			p -> hd.fea -> enums[i] = NULL;
		    else {
			char  **s = src -> hd.fea -> enums[i];
			int   j;
			for (j = 0; *s++ != NULL; j++);;
			p -> hd.fea -> enums[i] = (char **)
				         calloc((unsigned)j+1,sizeof(char *));
			p -> hd.fea->enums[i][j] = NULL;
			s = src -> hd.fea -> enums[i];
			j=0;
			while (*s != NULL)
			    p -> hd.fea -> enums[i][j++] = savestring (*s++);
		    }
		    if (old_version(p->common.hdvers,PRE_DERIVED) ||
		     p->hd.fea->derived[i] == 0)
			p -> hd.fea -> srcfields[i] = NULL;
		    else {
			char  **s = src -> hd.fea -> srcfields[i];
			int   j;
			for (j = 0; *s++ != NULL; j++);;
			p -> hd.fea -> srcfields[i] = (char **)
				         calloc((unsigned)j+1,sizeof(char *));
			p -> hd.fea->srcfields[i][j] = NULL;
			s = src -> hd.fea -> srcfields[i];
			j=0;
			while (*s != NULL)
			    p->hd.fea->srcfields[i][j++] = savestring (*s++);
		    }
		}
	    }

    }
/* zap all the source files and headers */
    for (i = 0; i < MAX_SOURCES; i++) {
	p -> variable.source[i] = NULL;
	p -> variable.srchead[i] = NULL;
    }
    p -> variable.nnames = p -> variable.nheads = 0;

/* zap prog and version */
    for (i = 0; i < PROGSIZE; i++)
	p -> common.prog[i] = '\0';
    for (i = 0; i < VERSIONSIZE; i++)
	p -> common.vers[i] = '\0';
    for (i = 0; i < DATESIZE; i++)
	p -> common.date[i] = '\0';
    for (i = 0; i < DATESIZE; i++)
	p -> common.progdate[i] = '\0';

/* copy the typtxt and refer fields */
    p -> variable.refer = savestring (src -> variable.refer);
    p -> variable.typtxt = savestring (src -> variable.typtxt);
    p -> variable.comment = NULL;
/*
    p -> variable.comment = savestring (src -> variable.comment);
*/

/* copy generic header items */
    for (i = 0; i < GENTABSIZ; i++) {
	struct gen_hd  *np = src -> variable.gentab[i];
	char   *ptr;
	p -> variable.gentab[i] = NULL;
	while (np) {
	    static int ok_to_copy();
	    char **codes = NULL;
	    if (np->type == CODED) 
		codes = np->codes;
	    if (ok_to_copy(np -> name)) {
	    	ptr = add_genhd (np -> name, np -> type, (int)np -> size, 
			(char *)NULL, codes, p);
	    	for (j = 0; j < np -> size * typesiz (np -> type); j++)
			ptr[j] = np -> d_ptr[j];
            }
	    np = np -> next;
	}
    }

/* store the header version and return */
    (void) strcpy (p -> common.hdvers, HD_VERSION);
    return p;
}

/* We do not want to copy certain generic items in copy_header
   This function will return 1 if the item is ok to copy, 0 otherwise
   For now, fields "encoding", "sphere_hd_ptr", "esignal_hd_ptr",
   and "pc_wav_hd_ptr" are not to be copied.
*/
static int
ok_to_copy(name)
char *name;
{
    return (name != NULL
	    && strcmp(name, "encoding") != 0
	    && strcmp(name, "sphere_hd_ptr") != 0
	    && strcmp(name, "esignal_hd_ptr") != 0
	    && strcmp(name, "pc_wav_hd_ptr") != 0);
}

/* 
	sdtofea converts an old SD header to FEA_SD
*/
struct header  *
sdtofea (src)
struct header  *src;
{
    int     i, j;
    double  start_time = 0.0;

    struct header  *p = (struct header *) calloc (1, sizeof *p);
    struct sd_header *sd_hd = src->hd.sd;
    spsassert(p,"malloc failed");
    spsassert(src->common.type == FT_SD,"sdtofea(): src must be FT_SD");
    *p = *src;
    p->common.ndouble = 0;
    p->common.nfloat = 0;
    p->common.nlong = 0;
    p->common.nshort = 0;
    p->common.nchar = 0;
    p->common.type = FT_FEA;
    p -> hd.fea = (struct fea_header *)calloc (1, sizeof *p->hd.fea);
    spsassert(p->hd.fea,"calloc failed");
/* zap all the source files and headers */
    for (i = 0; i < MAX_SOURCES; i++) {
	p -> variable.source[i] = NULL;
	p -> variable.srchead[i] = NULL;
    }
    p -> variable.nnames = p -> variable.nheads = 0;

/* copy the typtxt and refer fields */
    p -> variable.refer = savestring (src -> variable.refer);
    p -> variable.typtxt = savestring (src -> variable.typtxt);
    p -> variable.comment = NULL;

/* copy generic header items */
    for (i = 0; i < GENTABSIZ; i++) {
	struct gen_hd  *np = src -> variable.gentab[i];
	char   *ptr;
	p -> variable.gentab[i] = NULL;
	while (np) {
	    char **codes = NULL;
	    if (np->type == CODED) 
		codes = np->codes;
	    ptr = add_genhd (np -> name, np -> type, (int)np -> size, 
		(char *)NULL, codes, p);
	    for (j = 0; j < np -> size * typesiz (np -> type); j++)
		ptr[j] = np -> d_ptr[j];
	    np = np -> next;
	}
    }
    if (sd_hd->equip != NONE)
        (void) add_genhd_e("equip", &sd_hd->equip, 1, equip_codes, p);
    if (sd_hd->max_value != 0.0)
        (void) add_genhd_f("max_value", &sd_hd->max_value, 1, p);
    if (sd_hd->src_sf != 0.0)
        (void) add_genhd_f("src_sf", &sd_hd->src_sf, 1, p);
    if (sd_hd->synt_method != NONE)
        (void) add_genhd_e("synt_method",
                           &sd_hd->synt_method, 1, synt_methods, p);
   if (sd_hd->scale != 0.0)
        (void) add_genhd_f("scale", &sd_hd->scale, 1, p);
    if (sd_hd->dcrem != 0.0)
        (void) add_genhd_f("dcrem", &sd_hd->dcrem, 1, p);
    if (sd_hd->q_method != NONE)
        (void) add_genhd_e("q_method",
                           &sd_hd->q_method, 1, quant_methods, p);
    if (sd_hd->v_excit_method != NONE)
        (void) add_genhd_e("v_excit_method",
                           &sd_hd->v_excit_method, 1, excit_methods, p);
    if (sd_hd->uv_excit_method != NONE)
        (void) add_genhd_e("uv_excit_method",
                           &sd_hd->uv_excit_method, 1, excit_methods, p);
    if (sd_hd->synt_interp != NONE)
	(void) add_genhd_e("synt_interp",
                           &sd_hd->synt_interp, 1, synt_inter_methods, p);
    if (sd_hd->synt_pwr != NONE)
        (void) add_genhd_e("synt_pwr",
                           &sd_hd->synt_pwr, 1, synt_pwr_codes, p);
    if (sd_hd->synt_rc != NONE)
        (void) add_genhd_e("synt_rc",
                           &sd_hd->synt_rc, 1, synt_ref_methods, p);
    if (sd_hd->synt_order != 0)
        (void) add_genhd_s("synt_order", &sd_hd->synt_order, 1, p);
    if (sd_hd->start != 0)
        (void) add_genhd_l("start", &sd_hd->start, 1, p);
    if (sd_hd->nan != 0)
        (void) add_genhd_l("nan", &sd_hd->nan, 1, p);

    if (sd_hd->prefilter)
        add_genzfunc("prefilter", sd_hd->prefilter, p);

    if (sd_hd->de_emp)
        add_genzfunc("de_emp", sd_hd->de_emp, p);

    start_time = get_genhd_val("start_time", src, (double) 0.0);

/* initialize the FEA_SD header */
    (void)init_feasd_hd(p,get_sd_type(src),1,&start_time,NO,(double)sd_hd->sf);

/* save embedded headers */
    add_source_file(p,"Original old style SD header",src);

/* add a comment to explain what we did here */
    add_comment(p, "Old SD header converted to FEA_SD header by sdtofea.3");

/* store the header version and return */
    (void) strcpy (p -> common.hdvers, HD_VERSION);
    return p;
}

/*
	add_source_file adds a source filename and included header to
	a header. Only pointers are copied.
*/
void
add_source_file (hd, name, srchd)
struct header  *hd, *srchd;
char   *name;
{
    long foreign_hd_length, *foreign_hd_ptr;

    spsassert(hd,"add_source_file: hd is NULL");

    if (hd -> variable.nheads >= MAX_SOURCES ||
	    hd -> variable.nnames >= MAX_SOURCES) {
	(void) fprintf(stderr, 
		"add_source_file: warning: too many source files\n");
	return;
    }

    if (srchd && !get_genhd_val("foreign_hd_length", hd, 0.0)) {
      foreign_hd_length = get_genhd_val("foreign_hd_length",srchd,0.0);
      if (foreign_hd_length > 0) { 
        foreign_hd_ptr = get_genhd_l("foreign_hd_ptr",srchd);
	if (foreign_hd_ptr && *foreign_hd_ptr) {
	   copy_genhd(hd, srchd, "foreign_hd_length");
	   copy_genhd(hd, srchd, "foreign_hd_ptr");
	}
      }
    }

    if (name)
    	hd -> variable.source[hd -> variable.nnames++] = name;
    else
    	hd -> variable.source[hd -> variable.nnames++] = "NONE";

    if (srchd)
    	hd -> variable.srchead[hd -> variable.nheads++] = srchd;
}

/*
	putnulls writes out n null characters to fd.
*/
static void
putnulls (n, fd)
int     n;
FILE * fd;
{
    while (n-- > 0)
	(void)putc ('\0', fd);
}

/*
	write_par is used to write out string parameters.
*/
static void
write_par (code, text, fd)
short   code;
char   *text;
FILE * fd;
{
    int     trulen;
    short   len;

    (void) miio_put_short(&code, 1, edr_flag, fd);
    trulen = strlen (text) + 1;
    len = divceil (trulen, SIZINT);
    (void) miio_put_short(&len, 1, edr_flag, fd);
    (void) fputs (text, fd);
    putnulls ((int)(len * SIZINT - trulen + 1), fd);
}

/*
	add_comment is used to appen a string to the comment field
*/
void
add_comment (hd, text)
struct header  *hd;
char   *text;
{

    int comment_len=0;

    if (text == NULL)
	return;

    spsassert(hd != NULL, "add_comment: hd NULL");

    if (hd -> variable.comment != NULL)
	comment_len = strlen (hd -> variable.comment);

    if (strlen (text) + comment_len > MAX_STRING) {
	(void)fprintf(stderr,"add_comment: comment field at max length (%d) ",
		MAX_STRING);
	(void)fprintf(stderr,"comment not added.\n");
	return;
    }

    if (hd -> variable.comment == NULL) {
	hd -> variable.comment = malloc((unsigned) (strlen (text) + 1));
	spsassert(hd->variable.comment != NULL,"add_comment: malloc failed");
        (void) strcpy (hd -> variable.comment, text);
    }
    else {
        hd -> variable.comment = realloc (hd -> variable.comment,
	    (unsigned)(strlen (text) + strlen (hd -> variable.comment) + 2));
	spsassert(hd->variable.comment != NULL,"add_comment: realloc failed");
        (void) strcat (hd -> variable.comment, text);
    }
    return;
}

static short   *
get_shorts (fd, lev, size)
FILE * fd;
int     lev;
int     size;
{
    short  *p;
    if (size == 0)
	return (short *) NULL;
    p = (short *) calloc ((unsigned)size, sizeof (short));
    if (!miio_get_short(p, size, edr_flag, machine_code, fd))
	BADHEAD (lev, "error trying to read short vector");
    return p;
}

static  void
put_shorts (fd, p, size)
FILE * fd;
short  *p;
int     size;
{
    if (size == 0)
	return;
    if (miio_put_short(p, size, edr_flag, fd) == 0) 
	hd_error("put_shorts: I/O error");
    return;
}

static long   *
get_longs (fd, lev, size)
FILE * fd;
int     lev;
int     size;
{
    long  *p;
    if (size == 0)
	return (long *) NULL;
    p = (long *) calloc ((unsigned)size, sizeof (long));
    if (!miio_get_long(p, size, edr_flag, machine_code, fd))
	BADHEAD (lev, "error trying to read long vector");
    return p;
}

static  void
put_longs (fd, p, size)
FILE * fd;
long  *p;
int     size;
{
    if (size == 0)
	return;
    if (miio_put_long(p, size, edr_flag, fd) == 0)
	hd_error("put_longs: I/O error");
    return;
}

static short   *
copy_shorts (src, size)
short  *src;
int     size;
{
    short  *p;
    int     i;
    if (size == 0)
	return (short *) NULL;
    p = (short *) calloc ((unsigned)size, sizeof (short));
    for (i = 0; i < size; i++)
	p[i] = src[i];
    return p;
}

static long *
copy_longs (src, size)
long   *src;
int     size;
{
    long   *p;
    int     i;
    if (size == 0)
	return NULL;
    p = (long *) calloc ((unsigned)size, sizeof (long));
    for (i = 0; i < size; i++)
	p[i] = src[i];
    return p;
}

static float   *
copy_floats (src, size)
float  *src;
int     size;
{
    float  *p;
    int     i;
    if (size == 0)
	return NULL;
    p = (float *) calloc ((unsigned)size, sizeof (float));
    for (i = 0; i < size; i++)
	p[i] = src[i];
    return p;
}

static float   *
get_floats (fd, lev, size)
FILE * fd;
int     lev;
int     size;
{
    float  *p;
    if (size == 0)
	return (float *) NULL;
    p = (float *) calloc ((unsigned)size, sizeof (float));
    if (!miio_get_float(p, size, edr_flag, machine_code, fd))
	BADHEAD (lev, "error trying to read float vector");
    return p;
}

static  void
hd_error (s)
char   *s;
{
    (void)fprintf (stderr, "internal header routine error:\nheader.c: %s\n", s);
    exit (1);
}

static int
old_version(a,b)
char *a, *b;
{
    if(atof(a) <= atof(b)) return 1;
    return 0;
}
/*
|									|
|  nrec - number of records in ESPS file				|
|									|
|  Rodney W. Johnson							|
|									|
|  If hd points to the header of an ESPS file that is being read from	|
|  strm, and strm is a file, not a pipe, then the function returns the	|
|  number of records in the file.  If strm is a pipe, the function	|
|  returns -1.								|
|									|  
*/

static int old_sphere_flag = 0;	/* If this flag is set, then interpret
			         * the sphere header number of samples
				 * to mean total samples, rather than
				 * samples per channel (or records).
				 */

void
set_old_sphere_flag(value)
int value;
{
	old_sphere_flag = (value != 0);
}

int
get_old_sphere_flag()
{
	return old_sphere_flag;
}


long
nrec(hd, strm)
    struct  header *hd;
    FILE    *strm;
{
    int	    fd = fileno(strm);
    struct stat buf;
    long    filsiz,
	    offset;
    int	    recsiz;
    long    numrec;
    extern long atol();
    extern char *get_sphere_hdr();
    char    *s;

    if (lseek(fd, 0L, 1) < 0)
    {
	return -1;
    }
/*
 * we need to return -1 if the file is a pipe, even if the file is a
 * sphere file and we know how many records there are in it.  This is
 * because many ESPS programs use the fact that ndrec is -1 to mean 
 * that the input is a pipe.  Otherwise, they'll probably try and 
 * rewind it.
*/

/* if the file is sphere file, then return the number of samples from
 * the sphere header.  If old_sphere_flag is true, then divide this by
 * the sample count, since the number is the total number of samples,
 * rather than the samples/per channel.
*/

    if (s = getenv("OLD_SPHERE_FORMAT")) 
	old_sphere_flag = atol(s);

    if (get_sphere_hdr(hd)) {
	return ((long)get_genhd_val("sample_count", hd, (double)0.0)/
	       (old_sphere_flag ? 
			(long)get_genhd_val("channel_count",hd, (double)1.0) :
		        1));
    }
/*
 *
 *
*/

    if (fstat(fd, &buf) < 0)
    {
	Fprintf(stderr, "nrec: can't get file size.\n");
	exit(1);
    }
    filsiz = buf.st_size;    
    offset = ftell(strm);
    recsiz = size_rec2(hd);

    /* For an Esignal file, "record size" -1 signals variable record size.
     * Treat like a file being read from a pipe, since we can't use the
     * record size to find the number of records or to seek to the position
     * of a given record.
     */
    if (recsiz == -1) return -1;

    if ((int)get_genhd_val("encoding", hd, (double)0.0)) 
	recsiz = recsiz / 2;
    if (recsiz != 0) numrec = (filsiz - offset)/recsiz;
    if (numrec < 0 || numrec*recsiz != filsiz - offset)
    {
	Fprintf(stderr,
	    "Warning: inconsistent file size, offset, and record size.\n");
	return -1;

    }
    return numrec;
}

#ifdef ESI

static void
set_ana_type(hd)
struct header *hd;

{
    int order,i;
    spsassert(hd->common.type == FT_ANA,"set_ana_type: hd not ANA");
    order = (hd->hd.ana->order_vcd > hd->hd.ana->order_unvcd) ? 
	hd->hd.ana->order_vcd : hd->hd.ana->order_unvcd;
    i = hd->hd.ana->maxpulses+hd->hd.ana->maxlpc+hd->hd.ana->maxraw;
    hd->common.ndouble = 0;
    hd->common.nfloat  = i + order;
    hd->common.nlong = 0;
    hd->common.nshort = 1;
    hd->common.nchar = 0;
    hd->common.tag = YES;	/* our ana record has tags */
    return;
}

#endif ESI

#ifndef lint
static char *cr = "Copyright(C) 1986-1991 Entropic Speech Inc.\nCopyright(C) 1991 Entropic Research Laboratory, Inc\nAll Rights Reserved." ;
#endif

/* set the type fields in common for the filt record in hd
*/

void 
set_filt_type(hd)
struct header *hd;

{
    hd->common.tag = YES;
    hd->common.nshort = FDSPARES + 2;  /* spares plus two shorts in zfunc */
    hd->common.nfloat = hd->hd.filt->max_num + hd->hd.filt->max_den;
    return;
}

#ifdef ESI

/* set the type fields in common for the pitch record in hd
 */

static void
set_pit_type(hd)
struct header *hd;

/* tag doesn't count here */

{
    spsassert(hd->common.type == FT_PIT,"set_pit_type: hd not PIT");
    hd->common.ndouble = 0;
    hd->common.nfloat  = 2;
    hd->common.nlong = 0;
    hd->common.nshort = 0;
    hd->common.nchar = 0;
    hd->common.tag = YES;	/* our pitch record has tags */
    return;
}

#endif ESI

#ifdef ESI

/* set the type fields in common for the ros record in hd
*/

static void
set_ros_type(hd)
struct header *hd;

{
    int order,i;
    spsassert(hd->common.type == FT_ROS,"set_ros_type: hd not ROS");
    hd->common.ndouble = 0;
    hd->common.nfloat = 0;
    hd->common.nlong = 1;
    hd->common.nshort = (hd->hd.ros->maxtype * 2)
		+ (hd->hd.ros->maxpulses * 2)
		+ (MAX(hd->hd.ros->order_vcd,hd->hd.ros->order_unvcd) * 2)
		+ (hd->hd.ros->maxpow * 2)
		+ 1;
    hd->common.nchar = 0;
    hd->common.tag = YES;
    return;
}

#endif ESI

/* set the type fields in common for the scbk record in hd
 */

void 
set_scbk_type(hd)
struct header *hd;

{
  if (hd->common.type != FT_SCBK) {
	(void)fprintf(stderr,"set_scbk_type: called with non-SCBK header.\n");
	exit (1);
  }
  hd->common.nlong = hd->hd.scbk->num_cdwds; 
  hd->common.nfloat = (hd->hd.scbk->num_cdwds*3)+1; 
  hd->common.nshort = hd->hd.scbk->num_cdwds; 
  hd->common.nchar = 0; hd->common.ndouble = 0; 
  hd->common.tag=0;
  hd->common.ndrec=1;
  return;
}


/* set the type fields in common for the spec record in hd
 */

static void
set_spec_type(hd)
struct header *hd;

{
  int floats=0, longs=0, shorts=0;

  spsassert(hd != NULL, "set_spec_type: hd is NULL");
  spsassert(hd->common.type == FT_SPEC, "set_spec_type: file not SPEC");

  floats += hd->hd.spec->num_freqs;
  if (hd->hd.spec->spec_type == ST_CPLX) floats += hd->hd.spec->num_freqs;
  if (hd->hd.spec->freq_format == ARB_VAR) {
	floats += hd->hd.spec->num_freqs;
	longs++;
  }
  if (hd->hd.spec->frame_meth != FM_NONE && 
	hd->hd.spec->frame_meth != FM_FIXED) shorts++;
  if (hd->hd.spec->voicing == YES) shorts++;

  hd->common.nlong = longs;
  hd->common.nfloat = floats+1;
  hd->common.nshort = shorts;
  hd->common.nchar = 0;
  hd->common.ndouble = 0; 
  return;
}

#ifndef DEC_ALPHA
static long
#else
static int
#endif
swap_longs(t)
long t;
{
char tmp;
union {
#ifndef DEC_ALPHA
	long l_val;
#else
	int l_val;
#endif
	char byte[4];
} u2;
	u2.l_val = t; 

	tmp= u2.byte[0];
	u2.byte[0] = u2.byte[3];
	u2.byte[3]=tmp; 
	tmp= u2.byte[1];
	u2.byte[1] = u2.byte[2];
	u2.byte[2] = tmp;

 	return u2.l_val;
}



#ifdef DEMO
/* very simple hash function
*/
static int
hash (s)
char   *s;
{
    int     sum = 0;
    while (*s)
        sum += *s++;
    return (sum % 15000);
}
#endif

/*
  Skip over an ESPS file header in an input stream without reading
  it into memory.  Depends on data_offset value in preamble.
*/
  
int
skip_header(file)
    FILE	    *file;
{
    struct preamble *pre = NULL;
    int		    success;

    if (getenv("ESPSDEBUG")) EspsDebug = YES;
#ifndef NO_LIC
    check_header(MACH_CODE);
#endif

    spsassert(file, "skip_header: file is NULL");

    pre = (struct preamble *) malloc(sizeof(*pre));
    spsassert(pre, "skip_header: couldn't alloc preamble.\n");

    if (!read_preamble(pre, file))
    {
	Fprintf(stderr, "skip_header: couldn't read preamble.\n");
	success = NO;
    }
    else
    if (pre->check != HD_CHECK_VAL)
    {
	Fprintf(stderr, "skip_header: no preamble.\n");
	success = NO;
    }
    else
    if (data_offset == 0)
    {
	Fprintf(stderr, "skip_header: no data offset in preamble.\n");
	success = NO;
    }
    else
    if (data_offset < sizeof(struct preamble))
    {
	Fprintf(stderr, "skip_header: data offset too small.\n");
	success = NO;
    }
    else
    if (lseek(fileno(file), 0L, SEEK_CUR) < 0)	/* Cf. skiprec.c */
    {				/* We're on a pipe. */
	long	n;

	/* Assumption: preamble has same size in file and in memory */
	for (n = data_offset - sizeof(struct preamble);
	     n > 0 && getc(file) != EOF;
	     n--)
	{ }
	if (n > 0)
	{
	    Fprintf(stderr, "skip_header: premature end of file.\n");
	    success = NO;
	}
	else
	    success = YES;
    }
    else
    {				/* Not a pipe, so fseek will work. */
#ifdef hpux
	/* HP fseek bug.  Cf. skiprec.c */
	int	c;

	if ((c = getc(file)) == EOF)
	{
	    Fprintf(stderr, "skip_header: premature end of file.\n");
	    success = NO;
	}
	else
#endif
	if (fseek(file, data_offset, SEEK_SET) != 0)
	{
	    Fprintf(stderr, "skip_header: fseek error.\n");
	    success = NO;
	}
	else
	    success = YES;
    }

    free(pre);
    return success;
}

/*
  Read ESPS file-header preamble from input stream.  Sets global
  variables data_offset, align_pad_size, and (possibly) check_code.
*/

int
read_preamble(pre, file)
    struct preamble *pre;
    FILE	    *file;
{
#ifndef DEC_ALPHA
    if (!(miio_get_long(&pre->machine_code, 1, YES, NONE, file) &&
	  miio_get_long(&pre->check_code, 1, YES, NONE, file) &&
	  miio_get_long(&pre->data_offset, 1, YES, NONE, file) &&
	  miio_get_long(&pre->record_size, 1, YES, NONE, file) &&
	  miio_get_long(&pre->check, 1, YES, NONE, file) &&
	  miio_get_long(&pre->edr, 1, YES, NONE, file) &&
	  miio_get_long(&pre->align_pad_size, 1, YES, NONE, file) &&
	  miio_get_long(&pre->foreign_hd, 1, YES, NONE, file)))
#else
    if (!(miio_get_int(&pre->machine_code, 1, YES, NONE, file) &&
	  miio_get_int(&pre->check_code, 1, YES, NONE, file) &&
	  miio_get_int(&pre->data_offset, 1, YES, NONE, file) &&
	  miio_get_int(&pre->record_size, 1, YES, NONE, file) &&
	  miio_get_int(&pre->check, 1, YES, NONE, file) &&
	  miio_get_int(&pre->edr, 1, YES, NONE, file) &&
	  miio_get_int(&pre->align_pad_size, 1, YES, NONE, file) &&
	  miio_get_int(&pre->foreign_hd, 1, YES, NONE, file)))
#endif
    {
	return NO;
    }

    if (pre->check != HD_CHECK_VAL)
    {
	if (EspsDebug)
	    Fprintf(stderr,"trying byte swapping..");
	pre->check = swap_longs(pre->check);	
	pre->machine_code = swap_longs(pre->machine_code);
	pre->check_code = swap_longs(pre->check_code);	
	pre->data_offset = swap_longs(pre->data_offset);
	pre->record_size = swap_longs(pre->record_size);
	pre->edr = NO;
	pre->align_pad_size = swap_longs(pre->align_pad_size);
	pre->foreign_hd = swap_longs(pre->foreign_hd);
    }

    data_offset = pre->data_offset;
    align_pad_size = pre->align_pad_size;
#ifdef DEMO
    check_code = pre->check_code;
#endif

    return YES;
}

/*
  Free storage for all parts of an ESPS file header, including embedded
  headers.
*/

void
free_header(hd, flags, ptr)
    struct header   *hd;	/* header to be freed */
    unsigned long   flags;	/* reserved for use in later versions */
    char	    *ptr;	/* reserved for use in later versions */
{
    if(getenv("ESPSDEBUG"))
	EspsDebug=1;

    rec_free_hdr(hd);
}

/*
  Recursive free-header function.  Does all the work for free_header.
*/

static void
rec_free_hdr(hd)
    struct header   *hd;
{
    int		    i;

fprintf(stderr,"rec_free_hd\n");
    if (hd)
    {
fprintf(stderr,"point a1\n");
	fix_header (hd);
fprintf(stderr,"point a2\n");

	switch (hd->common.type)
	{
#ifdef ESI
	case FT_ANA: 
	    if (hd->hd.ana)
	    {
		free_zfunc(hd->hd.ana->pre_emp);
		free((char *) hd->hd.ana);
	    }
	    break;
	case FT_PIT: 
	    if (hd->hd.pit)
	    {
		free_zfunc(hd->hd.pit->lpf);
		free_zfunc(hd->hd.pit->prefilter);
		free((char *) hd->hd.pit);
	    }
	    break;
	case FT_ROS: 
	    if (hd->hd.ros)
	    {
		struct ros_header   *ros = hd->hd.ros;

		if (ros->type_bits)
		    free((char *) ros->type_bits);
		if (ros->rc_ubits)
		    free((char *) ros->rc_ubits);
		if (ros->rc_vbits)
		    free((char *) ros->rc_vbits);
		if (ros->pulse_bits)
		    free((char *) ros->pulse_bits);
		if (ros->pow_bits)
		    free((char *) ros->pow_bits);

		free((char *) hd->hd.ros);
	    }
	    break;
#endif ESI
	case FT_SD: 
	    if (hd->hd.sd)
	    {
		free_zfunc(hd->hd.sd->prefilter);
		free_zfunc(hd->hd.sd->de_emp);
		free((char *) hd->hd.sd);
	    }
	    break;
	case FT_SPEC: 
	    if (hd->hd.spec)
	    {
		/* Don't bother with freqs:  ARB_FIXED wasn't implemented. */
		free_zfunc(hd->hd.spec->pre_emp);
		free ((char *) hd->hd.spec);
		free ((char *) hd->hd.spec);
	    }
	    break;
	case FT_FILT: 
	    if (hd->hd.filt)
	    {
		struct filt_header  *filt = hd->hd.filt;

		if (filt->bandedges)
		    free((char *) filt->bandedges);
		if (filt->points)
		    free((char *) filt->points);
		if (filt->gains)
		    free((char *) filt->gains);
		if (filt->wts)
		    free((char *) filt->wts);
		free((char *) hd->hd.filt);
	    }
	    break;
	case FT_SCBK: 
	    if (hd->hd.scbk)
	    {
		free((char *) hd->hd.scbk);
	    }
	    break;
	case FT_FEA:
	    if (hd->hd.fea)
	    {
		struct fea_header   *fea = hd->hd.fea;

fprintf(stderr,"point a\n");
		if (fea->names)
		{
		    for (i = 0; i < fea->field_count; i++)
			if (fea->names[i])
			    free((char *) fea->names[i]);
		    free((char *) fea->names);
		}

fprintf(stderr,"point b\n");
		if (fea->sizes)
		    free((char *) fea->sizes);

		if (fea->dimens)
		{
		    if (fea->ranks)
		    {
			for (i = 0; i < fea->field_count; i++)
			    if (fea->ranks[i] && fea->dimens[i])
				free((char *) fea->dimens[i]);
		    }
		    else    /* this shouldn't happen */
			fprintf(stderr,
				"free_header: non-NULL dimens but NULL ranks");

		    free((char *) fea->dimens);
		}

		if (fea->ranks)
		    free((char *) fea->ranks);


fprintf(stderr,"point c\n");
		if (fea->types)
		{
		    if (fea->enums)
		    {
			for (i = 0; i < fea->field_count; i++)
			    if (fea->types[i] == CODED)
				free_strlist(fea->enums[i]);
			free((char *) fea->enums);
		    }
		    free((char *) fea->types);
		}

		if (fea->starts)
		    free((char *) fea->starts);

fprintf(stderr,"point d\n");
		if (fea->types)
		if (fea->derived)
		{
		    if (fea->srcfields)
		    {
			for (i = 0; i < fea->field_count; i++)
			    /* if (fea->derived[i]) */
			    free_strlist(fea->srcfields[i]);
			free((char *) fea->srcfields);
		    }
		    free((char *) fea->derived);
		}

		free((char *) hd->hd.fea);
	    }
	    break;
	}

	if (hd->common.fixpartsiz == hd->common.hsize)
	    return;

	/* free optional parameters. */

	for (i = 0; i < hd->variable.nnames; i++)
	    if (hd->variable.source[i])
		free((char *) hd->variable.source[i]);

	for (i = 0; i < hd->variable.nheads; i++)
	    rec_free_hdr(hd->variable.srchead[i]);

	if (hd->variable.typtxt)
	    free((char *) hd->variable.typtxt);
	if (hd->variable.comment)
	    free((char *) hd->variable.comment);
	if (hd->variable.refer)
	    free((char *) hd->variable.refer);

	if (hd->variable.ngen != 0)
	{
	    struct gen_hd   *np, *nq;
	    int		    j;

	    for (j = 0; j < GENTABSIZ; j++)
	    {
		for (np = hd->variable.gentab[j]; np; )
		{
		    if (np->name)
			free((char *) np->name);
		    if (np->type == CODED)
			free_strlist(np->codes);
		    if (np->d_ptr)
			free((char *) np->d_ptr);
		    nq = np->next;
		    free((char *) np);
		    np = nq;
		}
	    }
	}

	rec_free_hdr(hd->variable.refhd);

	if (hd->variable.current_path)
	    free((char *) hd->variable.current_path);

	free((char *) hd);
    }
}

/*
  Free storage for a zfunc.
*/

static void
free_zfunc(z)
    struct zfunc    *z;
{
    if (z)
    {
	if (z->zeros)
	    free((char *) z->zeros);
	if (z->poles)
	    free((char *) z->poles);
	free((char *) z);
    }
}

/*
  Free storage for a NULL-terminated list of strings.
*/

static void
free_strlist(strlist)
    char    **strlist;
{
    char    **str;

    if (strlist)
    {
	for (str = strlist; *str; str++)
	    free((char *) *str);
	free((char *) strlist);
    }
}

int
is_SIG_file(fd)
FILE *fd;
{
        char buf[3];
	if(fread(buf,1,3,fd) &&
	   buf[0] == 'S' && buf[1] == 'I' && buf[2] == 'G') return 1;
	return 0;
}

is_an_esps_header(fd)
     int fd;
{
  int ok = 0;
  if(fd >= 0) {
    int here = lseek(fd, 0L, SEEK_CUR);
    int newd = dup(fd);
    FILE *s;
    struct header *h = NULL;

    if(newd > 0) {
      if((s = fdopen(newd,"r"))) {
        if((h = read_header(s))) {
          free_header(h,(long)0,(char*)NULL);
          ok = TRUE;
        }
        fclose(s);
      } else
        close(newd);
    }
    lseek(fd, here, L_SET);
  }
  return(ok);
}


#ifdef OS5
int
bcopy(a,b,n)
char *a,*b;
int n;
{
	memcpy(b,a,n);
	return 0;
}
#endif
