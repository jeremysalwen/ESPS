/*
 * This material contains unpublished, proprietary software of
 * Entropic Research Laboratory, Inc. Any reproduction, distribution,
 * or publication of this work must be authorized in writing by Entropic
 * Research Laboratory, Inc., and must bear the notice:
 *
 *    "Copyright (c) 1998 Entropic Research Laboratory, Inc.
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 *
 * Written by:  Rod Johnson
 * Checked by:
 * Revised by:
 *
 * Brief description:
 *   HTK input filter for ESPS FEA files.
 */

static char *sccs_id = "@(#)featohtk.c	1.1\t3/17/98\tERL";

/* INCLUDE FILES */

#include <stdlib.h>
#include <math.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/array.h>
#include <esps/types.h>
#include <esps/range_switch.h>
#include <esps/strlist.h>

/* LOCAL CONSTANTS */

#define EC_SCCS_DATE	"3/17/98"
#define EC_SCCS_VERSION	"1.1"
#define MAX_INT		(0x7fffffff)	/* INT_MAX for EDR machines */
#define MAX_SHORT	(0x7fff)	/* SHRT_MAX for EDR machines */
#define SD_BUF_LEN	(1000)		/* length of FEA_SD intput buffer */

/* LOCAL MACROS */

#define ERROR(text) { \
  (void) fprintf(stderr, "%s: %s - exiting\n", ProgName, (text)); \
  exit(1);}

#define REQUIRE(test, text) {if (!(test)) ERROR(text)}

#define SYNTAX \
USAGE("featohtk [-f field[range]] [-k parmKind] [-r range] [-x debug_level]\n\t[-E field[index]] [-L] [-O field[index]] [-P param_file] input output")

/* LOCAL TYPEDEFS AND STRUCTURES */

/* HTK DEFINITIONS AND DECLARATIONS */
    
    /* These are taken from the HTK version 2.1.1 source files
     * HTKLib/HParm.h and HTKLib/HParm.c.
     * They may need to be revised when a later version is released.
     * Simply including HParm.h (the "right thing to do") required
     * including a cascade of further header files, extending to the
     * Esignal header file esignal.h, which contains definitions that
     * conflict with definitions in esps/esps.h.
     * Likewise linking the HTK libraries and the ESPS libraries in one
     * binary resulted in multiple definitions of numerous symbols
     * (apparently in the license manager).
     * All in all, too much complication to be worth doing.
     */

enum
{
    WAVEFORM,   LPC,        LPREFC,     LPCEPSTRA,
    LPDELCEP,   IREFC,      MFCC,       FBANK,
    MELSPEC,    USER,       DISCRETE,   ANON
};

static char	*pmkmap[] =
{
    "WAVEFORM", "LPC",      "LPREFC",   "LPCEPSTRA",
    "LPDELCEP", "IREFC",    "MFCC",     "FBANK",
    "MELSPEC",  "USER",     "DISCRETE", "ANON",
    NULL /* NULL termination, not in HTK source, needed for lin_search2 */
};

typedef short	ParmKind;

#define HASENERGY   (0100)
#define HASZEROC  (020000)
#define BASEMASK     (077)

/* SYSTEM FUNCTIONS AND VARIABLES */

extern int	getopt();
extern int	optind;
extern char	*optarg;

/* ESPS FUNCTIONS AND VARIABLES */

int		debug_level = 0;

/* LOCAL FUNCTION DECLARATIONS */

/* STATIC (LOCAL) GLOBAL VARIABLES */
 
static char	*ProgName = "featohtk";
static char	*Version = EC_SCCS_VERSION;
static char	*Date = EC_SCCS_DATE;

static char	*no_yes[] = {"NO", "YES", NULL};

/* MAIN PROGRAM */

int
main(argc, argv)
    int 	    argc;
    char	    **argv;
{
    int	    	    ch;			/* command-line option letter */

    char	    *fld = NULL;	/* basic param or sample field
					 * name with range */
    char	    *fld_name;		/* field name part of fld */
    long	    *fld_range;		/* indices of subrange of fld */
    long	    rng_len;		/* length of subrange of fld */
    int		    contig;		/* is fld_range a single sequence
					 * of consecutive integers without
					 * gaps? */
    int 	    fld_type;		/* data type of field named fld */
    char	    *fld_data;		/* data from subrange of fld */
    char	    *fldptr;		/* pointer to field data in rec */
    long	    fld_length;		/* length of field named fld */
    char	    *kind = NULL;	/* parameter kind string */
    short	    parmKind;		/* parameter kind binary code */
    int		    out_type;		/* output data type */
    char	    *rrange = NULL;	/* range of records to process */
    long	    start_rec;		/* number of first record to process */
    long	    end_rec;		/* number of last record to process */
    long	    num_recs;		/* number of records to process */
    int		    nSamples;		/* num_recs as int */
    char	    *Efld = NULL;	/* energy field name with index */
    char	    *Efld_name;		/* field name part of Efld */
    long	    *Efld_range;	/* pointer to index in Efld */
    long	    Erng_len;		/* length of subrange in Efld */
    int 	    Efld_type;		/* data type of E field (energy) */
    char	    *Eptr = NULL;	/* pointer to E data in rec */
    int 	    log_requested = NO;	/* compute log of energy? */
    char	    *comp_log_param;	/* value of "compute_log" in ESPS
					 * param file */
    char	    *Ofld = NULL;	/* 0-th cepstral coeff field name */
    char	    *Ofld_name;		/* field name part of Ofld */
    long	    *Ofld_range;	/* pointer to index in Ofld */
    long	    Orng_len;		/* length of subrange in Ofld */
    int 	    Ofld_type;		/* data type of O field (0-th order
					 * cepstral coefficient) */
    char	    *Optr = NULL;	/* pointer to O data in rec */
    char    	    *param_name = NULL;	/* parameter file name */
    char	    *inname;		/* input file name */
    FILE	    *infile;		/* input stream */
    struct header   *hd;		/* input file header */
    double	    record_freq;	/* input file record frequency (Hz) */
    int		    sampPeriod;		/* sampling period (100 ns) */
    double	    sampP_dbl;		/* sampPeriod as double */
    long	    ndrec;		/* input header item: number of
					 * data records */
    int		    fea_sd;		/* is input file FEA_SD? */
    struct fea_data *rec;		/* FEA input data record */
    struct feasd    *sd_rec;		/* FEA_SD input data structure */
    long	    chunk_len;		/* number of FEA_SD samples to try
					 * to read at one time */
    long	    act_len;		/* number of samples actually read */
    int		    eof;		/* end of input file reached? */
    char	    *outname;		/* output file name */
    FILE	    *outfile;		/* output stream */
    long	    samp_len;		/* number of items in output sample */
    int		    typesize;		/* size of one output item */
    short	    sampSize;		/* number of bytes in output sample */
    int		    out_len;		/* number of items of output sample
					 * actually written */
    char	    *buf;		/* output data buffer */
    float	    *Ebuf;		/* pointer to E data in buf */
    float	    *Obuf;		/* pointer to O data in buf */
    long	    num_read;		/* number of input records read
					 * or skipped over */
    long	    i, j;		/* loop and array indices */

    /*
     * PARSE COMMAND-LINE OPTIONS.
     */

    while ((ch = getopt(argc, argv, "f:k:r:x:E:LO:P:")) != EOF)
	switch (ch)
	{
	case 'f':
	    fld = optarg;
	    break;
	case 'k':
	    kind = optarg;
	    break;
	case 'r':
	    rrange = optarg;
	    break;
	case 'x':
	    debug_level = atoi(optarg);
	    break;
	case 'E':
	    Efld = optarg;
	    break;
	case 'L':
	    log_requested = YES;
	    break;
	case 'O':
	    Ofld = optarg;
	    break;
	case 'P':
	    param_name = optarg;
	    break;
	default:
	    SYNTAX;
	    break;
	}

    /*
     * PROCESS FILE NAMES AND OPEN FILES.
     */

    if (argc - optind > 2)
    {
	Fprintf(stderr,
		"%s: too many file names specified.\n", ProgName);
	SYNTAX;
    }
    if (argc - optind < 2)
    {
	Fprintf(stderr,
		"%s: too few file names specified.\n", ProgName);
	SYNTAX;
    }

    inname = eopen(ProgName,
		   argv[optind++], "r", FT_FEA, NONE, &hd, &infile);

    if (debug_level)
	Fprintf(stderr, "%s: input file \"%s\"\n", ProgName, inname);

    if (infile != stdin)
	REQUIRE(strcmp(inname, argv[optind]),
		"output file same as input");

    outname = eopen(ProgName,
		    argv[optind++], "w", NONE, NONE, NULL, &outfile);

    if (debug_level)
	Fprintf(stderr, "%s: output file: \"%s\"\n", ProgName, outname);

    /*
     * GET PARAMETER VALUES.
     */

    (void) read_params(param_name, SC_NOCOMMON, inname);

    /* Parameter kind (-k) */

    if (kind == NULL && symtype("parmKind") == ST_STRING)
    {
	kind = getsym_s("parmKind");
    }

    REQUIRE (kind != NULL,
	     "can't determine parameter kind");

    if (debug_level)
	Fprintf(stderr, "%s: parameter kind \"%s\"\n", ProgName, kind);

    parmKind = (short) lin_search2(pmkmap, kind);

    if (debug_level)
	Fprintf(stderr, "%s: parmKind code %x\n", ProgName, parmKind);

    switch (parmKind)
    {
	/* Most cases FALL THROUGH. */
    case WAVEFORM:
    case IREFC:
    case DISCRETE:
	out_type = SHORT;
	typesize = 2;
	break;
    case LPC:
    case LPREFC:
    case LPCEPSTRA:
    case MFCC:
    case FBANK:
    case MELSPEC:
    case USER:
	out_type = FLOAT;
	typesize = 4;
	break;
    case LPDELCEP:
	ERROR("delta-parameter fields not supported");
	break;
    default:
	ERROR("unsupported parmKind");
	break;
    }

    if (debug_level)
	Fprintf(stderr, "%s: out_type %d (%s), typesize %d\n",
		ProgName, out_type, ((out_type == SHORT) ? "SHORT"
				     : (out_type == FLOAT) ? "FLOAT"
				     : "other"),
		typesize);

    /* Base parameter field (-f) */

    if (fld == NULL && symtype("field") == ST_STRING)
    {
	fld = getsym_s("field");
    }

    REQUIRE (fld != NULL,
	     "can't determine field name");

    if (debug_level)
	Fprintf(stderr, "%s: base parameter field \"%s\"\n",
		ProgName, fld);

    fld_range = fld_range_switch(fld, &fld_name, &rng_len, hd);
    fld_type = get_fea_type(fld_name, hd);
    fld_length = get_fea_siz(fld_name, hd, NULL, NULL);

    if (debug_level)
    {
	Fprintf(stderr, "%s: field name \"%s\", range length %ld\n",
		ProgName, fld_name, rng_len);
	Fprintf(stderr, "%s: field type %d (%s), field length %ld\n",
		ProgName, fld_type, type_codes[fld_type], fld_length);
    }

    REQUIRE(is_type_numeric(fld_type),
	    "field undefined or non-numeric in input file header");
    spsassert(fld_range != NULL,
	      "fld_range_switch returned NULL");

    if (debug_level >= 2)
    {
	Fprintf(stderr, "%s: field elements\n\t[", ProgName);
	for (i = 0; i < rng_len; i++)
	    Fprintf(stderr, "%ld%s", fld_range[i],
		    (i == rng_len - 1) ? "]\n"
		    : (i%10 == 9) ? ",\n\t "
		    : ", ");
    }

    if (parmKind == WAVEFORM)
	REQUIRE(rng_len == 1,
		"range size must be 1 for WAVEFORM");

    contig = YES;
    for (i = 0; i < rng_len; i++)
    {
	j = fld_range[i];
	REQUIRE(j >= 0 && j < fld_length,
		"index out of range for field");
	if (j != fld_range[0] + i)
	    contig = NO;
    }

    if (debug_level)
	Fprintf(stderr, "%s: range %s contiguous\n",
		ProgName, (contig) ? "IS" : "is NOT");

    /* Energy parameter field (-E) */

    if (Efld == NULL && symtype("Efield") == ST_STRING)
    {
	Efld = getsym_s("Efield");
    }

    if (debug_level)
	Fprintf(stderr, "%s: E field \"%s\"\n",
		ProgName, (Efld == NULL) ? "<null>" : Efld);

    if (Efld != NULL)
    {
	REQUIRE(out_type == FLOAT,
		"energy term not supported with non-FLOAT output");
	Efld_range = fld_range_switch(Efld, &Efld_name, &Erng_len, hd);
	Efld_type = get_fea_type(Efld_name, hd);

	if (debug_level)
	{
	    Fprintf(stderr, "%s: Efld name \"%s\", range len %ld\n",
		    ProgName, Efld_name, Erng_len);
	    Fprintf(stderr, "%s: Efld type %d (%s)\n",
		    ProgName, Efld_type, type_codes[Efld_type]);
	    Fprintf(stderr, "%s: Efld element %ld\n", ProgName, Efld_range[0]);
	}

	REQUIRE(is_type_numeric(Efld_type),
		"Energy field undefined or non-numeric in input file header");
	spsassert(Efld_range != NULL,
		  "fld_range_switch returned NULL");
	REQUIRE(Erng_len == 1,
		"Energy field must have exactly one element");
	REQUIRE(Efld_range[0] >= 0
		&& Efld_range[0] < get_fea_siz(Efld_name, hd, NULL, NULL),
		"index out of range for Energy field");

	parmKind |= HASENERGY;

	if (debug_level)
	    Fprintf(stderr, "%s: now parmKind code is %x\n",
		    ProgName, parmKind);
    }

    /* Log energy computation flag (-L) */

    if (!log_requested && symtype("compute_log") == ST_STRING)
    {
	comp_log_param = getsym_s("compute_log");

	switch (lin_search(no_yes, comp_log_param))
	{
	case 0:
	    /* value remains NO */
	    break;
	case 1:
	    log_requested = YES;
	    break;
	default:
	    ERROR("ambiguous value of parameter \"compute_log\"");
	    break;
	}
    }

    if (debug_level)
	Fprintf(stderr, "%s: log computation %s requested\n",
		ProgName, (log_requested) ? "IS" : "is NOT");

    /* Zeroth cepstral parameter field (-O) */

    if (Ofld == NULL && symtype("Ofield") == ST_STRING)
    {
	Ofld = getsym_s("Ofield");
    }

    if (debug_level)
	Fprintf(stderr, "%s: O field \"%s\"\n",
		ProgName, (Ofld == NULL) ? "<null>" : Ofld);

    if (Ofld != NULL)
    {
	REQUIRE((parmKind & BASEMASK) == MFCC,
		"for 0-order cepstral coefficient, paramKind must be MFCC");
	Ofld_range = fld_range_switch(Ofld, &Ofld_name, &Orng_len, hd);
	Ofld_type = get_fea_type(Ofld_name, hd);

	if (debug_level)
	{
	    Fprintf(stderr, "%s: Ofld name \"%s\", range len %ld\n",
		    ProgName, Ofld_name, Orng_len);
	    Fprintf(stderr, "%s: Ofld type %d (%s)\n",
		    ProgName, Ofld_type, type_codes[Ofld_type]);
	    Fprintf(stderr, "%s: Ofld element %ld\n", ProgName, Ofld_range[0]);
	}

	REQUIRE(is_type_numeric(Ofld_type),
		"Zero-order cepstral field undefined or non-numeric in input file header");
	spsassert(Ofld_range != NULL,
		  "fld_range_switch returned NULL");
	REQUIRE(Orng_len == 1,
		"Zero-order cepstral field must have exactly one element");
	REQUIRE(Ofld_range[0] >= 0
		&& Ofld_range[0] < get_fea_siz(Ofld_name, hd, NULL, NULL),
		"index out of range for 0-order cepstral coefficient field");

	parmKind |= HASZEROC;

	if (debug_level)
	    Fprintf(stderr, "%s: now parmKind code is %x\n",
		    ProgName, parmKind);
    }

    /* Range of records to process (-r) */

    if (rrange != NULL)
    {
	start_rec = 1;
	end_rec = LONG_MAX;
	lrange_switch(rrange, &start_rec, &end_rec, 0);
	num_recs = (end_rec != LONG_MAX)
	    ? end_rec - (start_rec - 1)
		: 0;
    }
    else
    {
	start_rec =
	    (symtype("start") == ST_INT)
		? getsym_i("start")
		    : 1;
	num_recs =
	    (symtype("nan") == ST_INT)
		? getsym_i("nan")
		    : 0;
	end_rec =
	    (num_recs != 0)
		? start_rec - 1 + num_recs
		    : LONG_MAX;
    }

    if (debug_level)
	Fprintf(stderr, "%s: start_rec %ld, end_rec %ld, num_recs %ld\n",
		ProgName, start_rec, end_rec, num_recs);
	

    if (num_recs == 0)			/* implies continue to end of file */
    {
	ndrec = hd->common.ndrec;
	REQUIRE(ndrec != -1,		/* pipe or variable record length */
		"explicit end of range needed for piped input");

	end_rec = ndrec;
	num_recs = end_rec - (start_rec - 1);

	if (debug_level)
	    Fprintf(stderr, "%s: now end_rec %ld, num_recs %ld\n",
		ProgName, end_rec, num_recs);
    }

    REQUIRE(start_rec >= 1,
	    "can't start before beginning of file");
    REQUIRE(end_rec >= start_rec,
	    "empty range of records specified");

    /*
     * SET UP I/O BUFFERS.
     */

    fea_sd = (parmKind == WAVEFORM
	      && hd->hd.fea->fea_type == FEA_SD
	      && strcmp(fld_name, "samples") == 0
	      && fld_length == 1);

    if (fea_sd)  /* prepare to use FEA_SD support functions for input */
    {
	if (debug_level)
	    Fprintf(stderr, "%s: allocating FEA_SD record structure\n",
		    ProgName);

	sd_rec = allo_feasd_recs(hd, out_type, SD_BUF_LEN, NULL, NO);
	REQUIRE(sd_rec != NULL,
		"FEA_SD record allocation failed");
	buf = sd_rec->data;

	samp_len = 1;

	if (debug_level)
	    Fprintf(stderr, "%s: samp_len %ld item\n", ProgName, samp_len);
    }
    else	 /* prepare to use general FEA support functions for input */
    {
	if (debug_level)
	    Fprintf(stderr, "%s: allocating general FEA record\n",
		    ProgName);

	rec = allo_fea_rec(hd);

	REQUIRE(rec != NULL,
		"FEA record allocation failed");

	fldptr = get_fea_ptr(rec, fld_name, hd);

	if (Efld != NULL)
	    Eptr = get_fea_ptr(rec, Efld_name, hd);

	if (Ofld != NULL)
	    Optr = get_fea_ptr(rec, Ofld_name, hd);

	samp_len = rng_len;
	if (Ofld != NULL)
	    samp_len++;
	if (Efld != NULL)
	    samp_len++;

	if (debug_level)
	    Fprintf(stderr, "%s: samp_len %ld items\n", ProgName, samp_len);

	buf = ((fld_type == out_type && contig && Ofld == NULL && Efld == NULL)
	       ? fldptr + typesize * fld_range[0]
	       : arr_alloc(1, &samp_len, out_type, 0));

	fld_data = ((contig) ? fldptr + typesiz(fld_type) * fld_range[0]
		    : (fld_type == out_type) ? buf
		    : arr_alloc(1, &rng_len, fld_type, 0));

	if (debug_level)
	    Fprintf(stderr, "%s: field ptr %p, fld_data %p, buf %p\n",
		    ProgName, fldptr, fld_data, buf);

	if (Ofld != NULL)
	{
	    Obuf = (float *) buf + rng_len;

	    if (debug_level)
		Fprintf(stderr, "%s: Obuf %p\n", ProgName, Obuf);
	}

	if (Efld != NULL)
	{
	    Ebuf = (Ofld != NULL) ? Obuf + 1 : (float *) buf + rng_len;

	    if (debug_level)
		Fprintf(stderr, "%s: Ebuf %p\n", ProgName, Ebuf);
	}
    }

    /*
     * WRITE OUTPUT HEADER INFO.
     */

    REQUIRE(num_recs <= MAX_INT,
	    "range of records too long");
    nSamples = (int) num_recs;
    miio_put_int(&nSamples, 1, YES, outfile);

    record_freq = get_genhd_val("record_freq", hd, 0.0);
    REQUIRE(record_freq > 0.0,
	    "header item \"record_freq\" missing or invalid");
    sampP_dbl = floor(0.5 + 1.0e7/record_freq);
    REQUIRE(sampP_dbl >= 1 && sampP_dbl <= MAX_INT,
	    "header item \"record_freq\" invalid");
    sampPeriod = (int) sampP_dbl;
    miio_put_int(&sampPeriod, 1, YES, outfile);

    REQUIRE(samp_len <= MAX_SHORT/typesize,
	    "parameter field too big");
    sampSize = samp_len * typesize;
    miio_put_short(&sampSize, 1, YES, outfile);

    miio_put_short(&parmKind, 1, YES, outfile);

    if (debug_level)
    {
	Fprintf(stderr,
		"%s: nSamples %d, sampPeriod %d, sampSize %d, parmKind %d\n",
		ProgName, nSamples, sampPeriod, sampSize, parmKind);
    }

    /*
     * MAIN READ-WRITE LOOP.
     */

    num_read = start_rec - 1;

    if (debug_level)
	Fprintf(stderr, "%s: skipping %ld initial records\n",
		ProgName, num_read);

    fea_skiprec(infile, num_read, hd);

    if (fea_sd)  /* use FEA_SD support functions for input */
    {
	chunk_len = MIN(end_rec - num_read, SD_BUF_LEN);
	eof = NO;

	while (chunk_len > 0 && !eof)
	{
	    if (debug_level >= 2)
		Fprintf(stderr, "%s: reading chunk of %ld samples . . . .\n",
			ProgName, chunk_len);

	    act_len = get_feasd_recs(sd_rec, 0L, chunk_len, hd, infile);

	    if (debug_level >= 2)
		Fprintf(stderr, "%s: got %ld samples\n",
			ProgName, act_len);

	    num_read += act_len;

	    if (debug_level >= 2)
		Fprintf(stderr, "%s: read sample %ld\n",
			ProgName, num_read);

	    out_len = miio_put_short((short *) buf,
				     (int) act_len, YES, outfile);

	    if (debug_level >= 2)
		Fprintf(stderr, "%s: wrote %d samples\n", ProgName, out_len);

	    if (out_len != act_len)
	    {
		if (debug_level >= 2)
		    Fprintf(stderr,
			    "%s: failed to write FEA_SD output chunk - exiting\n",
			    ProgName);

		exit(1);
	    }
	    
	    eof = act_len < chunk_len;
	    if (end_rec - num_read < SD_BUF_LEN)
		chunk_len = end_rec - num_read;
	}
    }
    else         /* use general FEA support functions for input */
    {
	while (num_read < end_rec && get_fea_rec(rec, hd, infile) != EOF)
	{
	    num_read++;

	    if (debug_level >= 2)
		Fprintf(stderr, "%s: read record %ld . . . .\n",
			ProgName, num_read);

#define COPY(TYPE) \
	    {   \
		TYPE    *src = (TYPE *) fldptr; \
		TYPE    *dst = (TYPE *) fld_data; \
		for (i = 0; i < rng_len; i++) dst[i] = src[fld_range[i]]; \
	    }

	    if (!contig)
	    {
		if (debug_level >= 2)
		    Fprintf(stderr, "%s: copying subrange\n", ProgName);

		switch (fld_type)
		{
		case DOUBLE:      COPY(double);      break;
		case FLOAT:       COPY(float);       break;
		case LONG:        COPY(long);        break;
		case SHORT:       COPY(short);       break;
		case BYTE:        COPY(char);        break;
		case DOUBLE_CPLX: COPY(double_cplx); break;
		case FLOAT_CPLX:  COPY(float_cplx);  break;
		case LONG_CPLX:   COPY(long_cplx);   break;
		case SHORT_CPLX:  COPY(short_cplx);  break;
		case BYTE_CPLX:   COPY(byte_cplx);   break;
		}
	    }

	    if (buf != fld_data)
	    {
		if (debug_level >= 2)
		    Fprintf(stderr, "%s: type-converting subrange\n",
			    ProgName);

		(void) type_convert(rng_len, fld_data, fld_type,
				    buf, out_type, warn_on_clip);
	    }

	    if (Ofld != NULL)
	    {
		if (debug_level >= 2)
		    Fprintf(stderr, "%s: copying or converting Ofld data\n",
			    ProgName);

		if (Ofld_type == FLOAT)
		    *Obuf = *(float *) Optr;
		else
		    (void) type_convert(1L, (char *) Optr, Ofld_type,
					(char *) Obuf, FLOAT, warn_on_clip);
	    }

	    if (Efld != NULL)
	    {
		if (debug_level >= 2)
		    Fprintf(stderr, "%s: copying or converting Efld data\n",
			    ProgName);

		if (Efld_type == FLOAT)
		    *Ebuf = *(float *) Eptr;
		else
		    (void) type_convert(1L, (char *) Eptr, Efld_type,
					(char *) Ebuf, FLOAT, warn_on_clip);
		if (log_requested)
		{
		    if (debug_level >= 2)
			Fprintf(stderr, "%s: computing log of Efld data\n",
				ProgName);

		    *Ebuf = log(*Ebuf);
		}
	    }

	    switch (out_type)
	    {
	    case SHORT:
		out_len = miio_put_short((short *) buf,
					 (int) samp_len, YES, outfile);
		break;
	    case FLOAT:
		out_len = miio_put_float((float *) buf,
					 (int) samp_len, YES, outfile);
		break;
	    default:
		spsassert(0, "invalid data type for output");
	    }

	    if (debug_level >= 2)
		Fprintf(stderr, "%s: wrote %d items\n", ProgName, out_len);

	    if (out_len != samp_len)
	    {
		if (debug_level)
		    Fprintf(stderr,
			    "%s: failed to write output vector - exiting\n",
			    ProgName);
		exit(1);
	    }
	}
    }

    if (debug_level)
	Fprintf(stderr, "%s: end of loop; read %ld records\n",
		ProgName, num_read);
    
    REQUIRE(num_read == end_rec,
	    "premature end of file");

    exit(0);
    /*NOTREACHED*/
}
