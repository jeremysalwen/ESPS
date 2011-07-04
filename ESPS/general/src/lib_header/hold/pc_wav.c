/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1998 Entropic Research Laboratory, Inc. 
 *                   All rights reserved."
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Brief description:  Functions for use by ESPS I/O routines in dealing
 * with sampled-data files in RIFF WAVE format.
 * See "Multimedia Programming Interface and Data Specifications 1.0",
 * IBM Corp. and Microsoft Corp., 1991,
 * http://www.users.lith.com/~matts/riffmci/riffmci.rtf.  See also
 * http://www.users.lith.com/~matts/riffnew/riffnew.htm.
 */

static char *sccs_id = "@(#)pc_wav.c	1.1	5/1/98	ERL";


#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/unix.h>
#include <esps/pc_wav.h>


#define WAVE_FORMAT_PCM	(0x0001)

long	    get_genhd_val_l(char *, struct header *, long); /* See genhd.c */

extern int  EspsDebug;		/* set in headers.c for header debugging */
int	    PcWavPrefixSize = 12; /* "RIFF", <size>, "WAVE": 4 bytes each */

/*
 * LOCAL TYPEDEFS AND STRUCTURES
 */

typedef struct {
    unsigned short	BitsPerSample;
} PCM_specific;

struct pc_wav_hd {
    unsigned short	FormatTag;
    unsigned short	Channels;
    unsigned long	SamplesPerSec;
    unsigned long	AvgBytesPerSec;
    unsigned short	BlockAlign;
    union {
	PCM_specific	pcm;
    } spec;
    unsigned long	data_len;
};

/*
 *  LOCAL FUNCTION DECLARATIONS
 */

static pc_wav_hd    *get_hd(char *prefix, int num_read, FILE *file);
static int	    get_ck_hd(char *ckID, unsigned long *ckSize,
			      char *end_pref, int *num_read, FILE *file);
static int	    skip_ck(unsigned long ckSize,
			    char *end_pref, int *num_read, FILE *file);
static int	    get_lsbf_ushort(unsigned short *item, char *end_pref,
				    int *num_read, FILE *file);
static int	    get_lsbf_ulong(unsigned long *item, char *end_pref,
				   int *num_read, FILE *file);
static int	    next_char(char *end_pref, int *num_read, FILE *file);
static long	    read_lsbf_short(short *ptr, long len, FILE *file);

static void	    error_msg(char *func_name, char *msg);

/*
 * PUBLIC FUNCTION DEFINITIONS
 */

/*
 * Check whether the characters in "prefix" look like the beginning of
 * a PC WAVE file:  the four characters "RIFF" followed by four bytes of
 * binary data and the four characters "WAVE".  Return TRUE if so and
 * FALSE otherwise.
 */

int
pc_wav_check_prefix(char *prefix)
{
    return (prefix != NULL
	    && strncmp(prefix, "RIFF", 4) == 0
	    && strncmp(prefix + 8, "WAVE", 4) == 0);
}


/*
 * Read a PC WAVE "header" and construct a FEA_SD header that can be
 * passed to ESPS data input routines to allow them to read the PC WAVE
 * data samples.
 *
 * Assume "prefix" contains "num_read" bytes read from the beginning of
 * file "file" and that "file" is positioned for reading the following
 * characters.  If the catenation of "prefix"  with the remainder of
 * "file" begins with a PC WAVE header, this function reads in the rest
 * of the header material, creates a corresponding ESPS FEA_SD header,
 * and leaves the file positioned at the beginning of the data, ready for
 * reading of the binary samples that follow.
 *
 * In more detail, this means reading the "RIFF" chunk ID, the
 * four-byte chunk size, and the form type "WAVE"; skipping any
 * irrelevant chunks; extracting the information from the "fmt" chunk;
 * and reading the initial chunk ID "data" and the four-byte chunk size
 * of the data chunk.  No "LIST" data chunks are supported, only a simple
 * "data" chunk, and the format must be 16-bit PCM.  Multiple channels
 * are allowed, however.
 *
 * The items in the "fmt" chunk are saved in generic header items in the
 * FEA_SD header.  They are also saved in a "pc_wav_hd" structure, and a
 * pointer to the structure is saves as a long in the generic header item
 * "pc_wav_hd_ptr".  The generic header items "start_time" and
 * "record_freq" are initialized respectively to zero and to the value of
 * the WAVE "fmt" item "SamplesPerSec".
 *
 * The return value is a pointer to the FEA_SD header, or NULL in case
 * of error.
 *
 * This function is intended for use when it has been necessary to read
 * a number of characters at the beginning of a file to determine whether
 * it is in PC WAVE format or some other (such as FEA, Esignal, or NIST).
 * On a seekable input stream, it would be possible to simply rewind and
 * read all the header information directly from the file; however, the
 * function is intended to work on pipes.
 */

struct header *
pc_wav_to_feasd(char	*prefix,
		int	num_read,
		FILE	*file)
{
    pc_wav_hd	    *wav_hd;
    double	    start_time;
    double	    sf;
    struct header   *ohd;

    wav_hd = get_hd(prefix, num_read, file);

    if (wav_hd == NULL)
    {
	error_msg("pc_wav_to_feasd", "read header failed.");
	return NULL;
    }

    switch (wav_hd->FormatTag)
    {
    case WAVE_FORMAT_PCM:
	if (wav_hd->spec.pcm.BitsPerSample != 16)
	{
	    error_msg("pc_wav_to_feasd", "only 16-bit audio supported.");
	    return NULL;
	}
	break;
    default:
	error_msg("pc_wav_to_feasd", "only WAVE_FORMAT_PCM supported.");
	return NULL;
    }

    ohd = new_header(FT_FEA);

    if (ohd == NULL)
    {
	error_msg("pc_wav_to_feasd", "couldn't create FEA header.");
	return NULL;
    }

    strcpy(ohd->common.prog, "pc_wav_to_feasd");
    add_comment(ohd, "Converted from PC WAVE header\n");
    ohd->common.tag = NO;
    ohd->common.edr = NO;
    start_time = 0.0;
    sf = (double) wav_hd->SamplesPerSec;
    init_feasd_hd(ohd, SHORT, (int) wav_hd->Channels, &start_time, NO, sf);
    *add_genhd_l("FormatTag", (long *) NULL, 1, ohd) =
	(long) wav_hd->FormatTag;
    *add_genhd_l("Channels", (long *) NULL, 1, ohd) =
	(long) wav_hd->Channels;
    *add_genhd_l("SamplesPerSec", (long *) NULL, 1, ohd) =
	(long) wav_hd->SamplesPerSec;
    *add_genhd_l("AvgBytesPerSec", (long *) NULL, 1, ohd) =
	(long) wav_hd->AvgBytesPerSec;
    *add_genhd_l("BlockAlign", (long *) NULL, 1, ohd) =
	(long) wav_hd->BlockAlign;
    *add_genhd_l("BitsPerSample", (long *) NULL, 1, ohd) =
	(long) wav_hd->spec.pcm.BitsPerSample;

    *add_genhd_l("pc_wav_hd_ptr", (long *) NULL, 1, ohd) =
	(long) wav_hd;

    return ohd;
}


/*
 * From a FEA_SD header structure created "pc_wav_to_feasd", recover a
 * pointer to the original PC WAVE header information, stored by
 * "pc_wav_to_feasd" in the generic header item "pc_wav_hd_ptr".
 */

pc_wav_hd *
get_pc_wav_hdr(struct header *hdr)
{
    long    gen;

    if (hdr == NULL)
	return NULL;
    else
    {
	gen = get_genhd_val_l("pc_wav_hd_ptr", hdr, 0L);
	if (gen == 0)
	    return NULL;
	else
	    return (pc_wav_hd *) gen;
    }
}


/*
 * Obtain the record size (in bytes) from PC WAVE header data stored in a
 * "pc_wav_hd" structure.
 */

long
pc_wav_rec_size(pc_wav_hd *wav_hd)
{
    if (wav_hd == NULL)
    {
	error_msg("pc_wav_rec_size", "PC WAVE header pointer is NULL.");
	return -1;
    }

    switch (wav_hd->FormatTag)
    {
    case WAVE_FORMAT_PCM:
	return wav_hd->BlockAlign;
	break;
    default:
	error_msg("pc_wav_rec_size", "only WAVE_FORMAT_PCM supported.");
	return -1;
    }
}


/*
 * Advance the file position in a PC WAVE input file by a given number
 * of records, given a pointer to a "pc_wav_hd" structure describing the
 * file data format.
 */

void
pc_wav_skip_recs(FILE *file, long nrec, pc_wav_hd *wav_hd)
{
    long    size;

    if (wav_hd == NULL)
    {
	error_msg("pc_wav_skip_recs", "PC WAVE header pointer is NULL.");
	return;
    }

    if (nrec == 0)
	return;

    size = pc_wav_rec_size(wav_hd);

    if (size > 0)
    {
	skiprec(file, nrec, (int) size);
    }
    else
    {
	error_msg("pc_wav_skip_recs", "bad record size");
    }
}


/*
 * Read one "record" (i.e. a sample, possibly multi-channel) from a
 * PC WAVE file into a FEA record structure, given a "pc_wav_hd"
 * structure describing the file data format and an ESPS FEA header
 * structure describing the FEA record format.
 */

int
pc_wav_get_rec(struct fea_data	*rec,
	       pc_wav_hd	*wav_hd,
	       struct header	*fea_hd,
	       FILE		*file)
{
    long    num_read;

    if (rec == NULL || wav_hd == NULL || fea_hd == NULL || file == NULL)
    {
	error_msg("pc_wav_get_rec", "NULL argument.");
	return EOF;
    }

    /*!*//* ASSUME: format is WAVE_FORMAT_PCM; BitsPerSample is 16;
     * data type of field "samples" is SHORT. */

    num_read = read_lsbf_short(rec->s_data, (long) wav_hd->Channels, file);

    return (num_read == wav_hd->Channels) ? 1 : EOF;
}


/*
 * Read a given number of records from a PC WAVE file into a data
 * buffer (as shorts) given a "pc_wav_hd" structure describing the
 * format of the data in the file.
 */

long
pc_wav_getsd_recs(char		*buffer,
		  long		num_records,
		  pc_wav_hd	*wav_hd,
		  FILE		*file)
{
    if (buffer == NULL || wav_hd == NULL || file == NULL)
    {
	error_msg("pc_wav_getsd_recs", "NULL arguments.");
	return 0;
    }

    if (num_records == 0)
	return 0;

    /*!*//* ASSUME: format is WAVE_FORMAT_PCM; BitsPerSample is 16;
     * data type of field "samples" is SHORT. */

    return read_lsbf_short((short *) buffer,
			   num_records * wav_hd->Channels, file);
}


/*
 * LOCAL FUNCTION DEFINITIONS
 */

/*
 * Function used by "pc_wav_to_feasd" to "read" PC WAVE "header"
 * information and create a pc_wav_hd structure containing it.  The
 * return value is a pointer to the structure, or NULL in case of error.
 *
 * The arguments are as for "pc_wav_to_feasd": the string "prefix"
 * contains a number "num_read" of bytes previously read from "file", so
 * that the complete PC WAVE file consists of the contents of "prefix"
 * followed by the remaining bytes in "file".  This function, and
 * functions used by it, keep track of progress through "prefix" by means
 * of a fixed pointer "end_pref" to the _end_ of the sequence of pre-read
 * bytes, and by decrementing "num_read".  When that count reaches zero,
 * the functions switch over from obtaining characters from "prefix" to
 * reading characters from "file".  Data-reading functions such as
 * "pc_wav_get_rec" and "pc_wav_getsd_recs" are not expected to go
 * through such contortions, so this function returns an error indication
 * if "num_read" has not become zero by the time the beginning of the
 * data samples has been reached.
 *
 * It is assumed that "prefix" contains at least the initial chunk ID
 * "RIFF", the RIFF chunk size, and the form type "WAVE", and that these
 * have been checked by "pc_wav_check_prefix".  Within the RIFF WAVE
 * chunk, any chunks with chunk IDs other than "fmt " or "data" are
 * skipped, and a "fmt " chunk is required before the "data" chunk.  The
 * function reads the items FormatTag, Channels, SamplesPerSec,
 * AvgBytesPerSec, and BlockAlign and for WAVE_FORMAT_PCM (the only
 * format currently supported) the format-specific item BitsPerSample.
 * It stores these items in the pc_wav_hd structure.  It exits after
 * reading the chunk ID and size of the "data" chunk.
 */

static pc_wav_hd *
get_hd(char	*prefix,
       int	num_read,
       FILE	*file)
{
    char	    *end_pref;
    pc_wav_hd	    *wav_hd;
    int		    got_ck_hd;
    char	    ckID[4];
    unsigned long   ckSize;

    end_pref = prefix + num_read;
    num_read -= PcWavPrefixSize; /* skip "RIFF", <size>, "WAVE" */

    spsassert(num_read >= 0,
	      "get_hd: error reading PC WAVE header");

    wav_hd = malloc(sizeof(pc_wav_hd));
    if (wav_hd == NULL)
    {
	error_msg("get_hd",
		  "couldn't allocate storage for PC WAVE header.");
	return NULL;
    }

    got_ck_hd = get_ck_hd(ckID, &ckSize, end_pref, &num_read, file);
    while (got_ck_hd
	   && strncmp(ckID, "fmt ", 4) != 0
	   && strncmp(ckID, "data", 4) != 0)
    {
	got_ck_hd = (skip_ck(ckSize, end_pref, &num_read, file)
		     && get_ck_hd(ckID, &ckSize, end_pref, &num_read, file));
    }

    if (!got_ck_hd)
    {
	error_msg("get_hd", "end of file within PC WAVE chunk.");
	return NULL;
    }

    if (strncmp(ckID, "fmt ", 4) != 0)
    {
	error_msg("get_hd",
		  "no \"fmt\" chunk before data in PC WAVE file.");
	return NULL;
    }

    if (!(get_lsbf_ushort(&wav_hd->FormatTag, end_pref, &num_read, file)
	  && get_lsbf_ushort(&wav_hd->Channels, end_pref, &num_read, file)
	  && get_lsbf_ulong(&wav_hd->SamplesPerSec, end_pref, &num_read, file)
	  && get_lsbf_ulong(&wav_hd->AvgBytesPerSec, end_pref, &num_read, file)
	  && get_lsbf_ushort(&wav_hd->BlockAlign, end_pref, &num_read, file)))
    {
	error_msg("get_hd", "failed to read PC WAVE \"fmt\" chunk.");
	return NULL;
    }

    ckSize -= 14;		/* 3 ushort & 2 ulong */

    switch (wav_hd->FormatTag)
    {
    case WAVE_FORMAT_PCM:
	if (!get_lsbf_ushort(&wav_hd->spec.pcm.BitsPerSample,
			     end_pref, &num_read, file))
	{
	    error_msg("get_hd",
		      "failed to read PC WAVE \"fmt\" chunk (PCM part).");
	    return NULL;
	}

	ckSize -= 2;		/* 1 ushort */
	break;
    default:
	error_msg("get_hd", "only WAVE_FORMAT_PCM supported.");
	return NULL;
    }

    if (ckSize < 0)
    {
	error_msg("get_hd",
		  "inconsistent chunk size for PC WAVE \"fmt\".");
	return NULL;
    }

    do
    {
	got_ck_hd = (skip_ck(ckSize, end_pref, &num_read, file)
		     && get_ck_hd(ckID, &ckSize, end_pref, &num_read, file));

    } while(got_ck_hd
	    && strncmp(ckID, "data", 4) != 0);

    if (!got_ck_hd)
    {
	error_msg("get_hd", "failed to find PC WAVE \"data\" chunk.");
	return NULL;
    }

    if (num_read > 0)
    {
	error_msg("get_hd", "read too far ahead.");
	return NULL;
    }

    wav_hd->data_len = ckSize;

    return wav_hd;
}


/*
 * Function used by "get_hd" to "read" a RIFF chunk header, consisting of
 * a four-character chunk ID and a four-byte unsigned integer chunk size
 * (in least-significant-first byte order).  These are returned via the
 * pointers "ckID" and "ckSize".  The eight bytes are obtained from the
 * sequence consisting of the catenation of (1) "num_read" bytes just
 * before the byte indicated by "end_pref" and (2) the unread bytes in
 * "file".  See "get_hd" for the usage of these last three arguments. The
 * return value is TRUE for success and FALSE for failure.
 */

static int
get_ck_hd(char		*ckID,
	  unsigned long	*ckSize,
	  char		*end_pref,
	  int		*num_read,
	  FILE		*file)
{
    return (get_fourcc(ckID, end_pref, num_read, file)
	    && get_lsbf_ulong(ckSize, end_pref, num_read, file));
}


/*
 * Function used by "get_hd" to skip over the body of a RIFF chunk after
 * its chunk ID and size have been obtained by "get_ck_hd".  The number
 * of bytes to skip is passed as "ckSize"; if this odd, an additional
 * byte of padding is skipped to make up an even number.  The bytes
 * skipped are in the sequence consisting of the catenation of
 * (1) "num_read" bytes just before the byte indicated by "end_pref" and
 * (2) the unread bytes in "file".  See "get_hd" for the usage of these
 * last three arguments.  Bytes are skipped by decrementing "num_read",
 * reading from "file", or both.  The return values is TRUE for success
 * and NO for failure.
 */

static int
skip_ck(unsigned long	ckSize,
	char		*end_pref,
	int		*num_read,
	FILE		*file)
{
    ckSize += (ckSize & 1);	/* round up to multiple of 2 bytes */

    if (ckSize <= *num_read)
    {
	*num_read -= ckSize;
    }
    else
    {
	ckSize -= *num_read;
	*num_read = 0;

	while (ckSize > 0 && getc(file) != EOF)
	    ckSize--;

	if (ckSize != 0)
	{
	    error_msg("skip_ck", "end of file within chunk.");
	    return NO;
	}
    }

    return YES;
}


/*
 * Function used by "get_hd" to "read" a two-byte unsigned short integer
 * in least-significant-first byte order from the sequence consisting of
 * the catenation of (1) "num_read" bytes just before the byte indicated
 * by "end_pref" and (2) the unread bytes in "file".  See "get_hd" for
 * the usage of these last three arguments.  The value obtained is
 * returned via the pointer "ptr".  The function return value is TRUE
 * for success and FALSE for failure.
 */

static int
get_lsbf_ushort(unsigned short	*ptr,
		char		*end_pref,
		int		*num_read,
		FILE		*file)
{
    int		    ch;
    unsigned short  val;

    ch = next_char(end_pref, num_read, file);

    if (ch == EOF)
	return NO;

    val = (ch & 0xff);
    ch = next_char(end_pref, num_read, file);

    if (ch == EOF)
	return NO;

    *ptr = val | (ch & 0xff);

    return YES;
}


/*
 * Function used by "get_hd" and "get_ck_hd" to "read" a two-byte
 * unsigned short integer in least-significant-first byte order from the
 * sequence consisting of the catenation of (1) "num_read" bytes just
 * before the byte indicated by "end_pref" and (2) the unread bytes in
 * "file".  See "get_hd" for the usage of these last three arguments.
 * The value obtained is returned via the pointer "ptr".  The function
 * return value is TRUE for success and FALSE for failure.
 */

static int
get_lsbf_ulong(unsigned long	*ptr,
	       char		*end_pref,
	       int		*num_read,
	       FILE		*file)
{
    int		    i;
    int		    ch;
    unsigned long   val;

    val = 0;

    for (i = 0; i < 4; i++)
    {
	ch = next_char(end_pref, num_read, file);

	if (ch != EOF)
	    val |= (ch & 0xff) << (8*i);
	else
	    return NO;
    }

    *ptr = val;

    return YES;
}


/*
 * Function used by "get_ck_hd" to "read" a four-character chunk ID from
 * the sequence consisting of the catenation of (1) "num_read" bytes just
 * before the byte indicated by "end_pref" and (2) the unread bytes in
 * "file".  See "get_hd" for the usage of these last three arguments.
 * The value obtained is returned via the pointer "ptr".  The function
 * return value is TRUE for success and FALSE for failure.
 */

static int
get_fourcc(char     *ptr,
	   char     *end_pref,
	   int	    *num_read,
	   FILE     *file)
{
    int	    i;
    int	    ch;

    for (i= 0; i < 4; i++)
    {
	ch = next_char(end_pref, num_read, file);

	if (ch != EOF)
	    ptr[i] = ch;
	else
	    return NO;
    }

    return YES;
}


/*
 * Function used by "get_lsbf_ushort", "get_lsbf_ulong", "get_fourcc", to
 * "read" the next byte (unsigned char) from the sequence consisting of
 * the catenation of (1) "num_read" bytes just before the byte indicated
 * by "end_pref" and (2) the unread bytes in "file".  See "get_hd" for
 * the usage of these last three arguments.  The return value is the
 * value obtained in case of success and EOF in case of failure.
 */

static int
next_char(char	*end_pref,
	  int	*num_read,
	  FILE	*file)
{
    return ((*num_read > 0)
	    ? (unsigned char) *(end_pref - (*num_read)--)
	    : getc(file));
}


/*
 * Function used by "pc_wav_get_rec" and "pc_wav_getsd_recs" to read a
 * given number "len" of two-byte signed short integers in
 * least-significant-first byte order from a file.  The values obtained
 * are returned via the pointer "ptr".  The function return value is the
 * actual number of values read and is less than "len" in case of
 * failure.
 */

static long
read_lsbf_short(short	*ptr,
		long	len,
		FILE	*file)
{
#if defined(DEC_ALPHA) || defined(LINUX_OR_MAC)

    /* Code for Vax-like architectures (including Intel) that
     * match the least-significant-first byte ordering of the
     * RIFF standard.
     */

    return fread(ptr, 2, len, file);

#elif defined(SUN4) || defined(HP700) || defined(SG)

    /* Code for Sun-like architectures with most-significant-first
     * byte ordering, requiring byte-swapping to read RIFF files.
     */

    unsigned char   *p = (unsigned char *) ptr;
    int		    ch;
    long	    n, i;


    if (len == 1)
    {
	if ((ch = getc(file)) == EOF)
	    return 0;

	p[1] = (unsigned char) ch;

	if ((ch = getc(file)) == EOF)
	    return 0;
	
	p[0] = (unsigned char) ch;
	return 1;
    }

    if (len <= 0)
	return 0;

    /* Read the data offset by 1 so that the low-order bytes end up in the
     * right places and don't have to be moved when we "swap".
     */
    n = fread(p + 1, 1, 2*len - 1, file);

    /* Now move the high-order bytes into place.
     */
    for (i = 0; i < n - 1; i += 2)
	p[i] = p[i+2];

    if (n != 2*len - 1)
	return n/2;

    if ((ch = getc(file)) == EOF)
	return n/2;

    p[n - 1] = (unsigned char) ch;
    return len;

#else

    /* Code for unknown architecture.
     * This should work regardless of the native byte order of the
     * maching on which it runs.  However, when porting to another
     * machine, try adding it to one of the #ifdef's above, as it
     * is likely to be faster.
     */
    long            n;
    unsigned int    item;
    int             ch;

    for (n = 0; n < len; n++)
    {
	if ((ch = getc(file)) == EOF)
	    break;
	item = ch & 0xff;

	if ((ch = getc(file)) == EOF)
	    break;
	item |= (ch & 0xff) << 8;

	ptr[n] =
	    (item & 0x8000)
		? (-1 - (int) (item ^ 0xffff))
		    : item;
    }

    return n;

#endif
}


/*
 * Function used by various functions in this module to output error
 * messages.
 */

static void
error_msg(char *func_name, char *msg)
{
    (void) fprintf(stderr, "%s: %s\n", func_name, msg);
}
