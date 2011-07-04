/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/* @(#)Signals.h	1.3 2/15/96 ATT/ERL/ESI */
/* Signals.h */

/* include <stdio.h> and <esps/header.h> first if this file included */

/* >>>>>>>>>> SIGNAL TYPES <<<<<<<<<< */

/* Vector signals may be periodic one-dimensional signals (like
single-channel sampled speech), and multi-dimensional signals (like
periodic LPC parameters), or aperiodic signals (like pitch-synchronous
parameters), and variable dimension signals (like LPC complex poles).
The "P_" prefix below is used for convenience to denote periodic
signals.  Aperiodic signals might be similarly defined as "#define
A_SHORTS 0x14" etc. */

#define APERIODIC_SIGNALS 0x30	/* Three types may be distinguished, like:
				 time stamp, interval stamp, and ??? */
#define VAR_REC_SIGNALS 0xc0	/* Three types may be distinguished, like:
				 vector (one dimension must be specified at
				 each frame), matrix (two dims. must be spec.),
				 and generalized tensor (the number of
				 dimensions must also be specified). */
#define SIG_DATA_TYPE	0xf	/* specifies the (homogeneous) C type */
#define P_CHARS		0x1
#define P_UCHARS	0x2
#define P_USHORTS	0x3
#define P_SHORTS	0x4
#define P_INTS		0x5
#define P_UINTS		0x6
#define P_FLOATS	0x7
#define P_DOUBLES	0x8
#define P_BOOLEAN 	0x9
#define P_MIXED		0xa
#define VECTOR_SIGNALS \
    	(P_CHARS|P_UCHARS|P_USHORTS|P_SHORTS|P_INTS|P_UINTS|P_FLOATS|P_DOUBLES|P_MIXED)
/* (should really be called something like PERIODIC_FIXED_VECTOR_SIGNALS...) */

#define IS_GENERIC(type) \
    	((!(((type)&(APERIODIC_SIGNALS|VAR_REC_SIGNALS)))) && ((type)&VECTOR_SIGNALS))

/* The definition of IS_GENERIC() will become broader as more access
and display methods are written (hint, hint).  Every attempt will be
made to supply "GENERIC" read, write, display, etc.  methods for
VECTOR_SIGNALS; however, other signal types will be required.  The
following SPECIAL_SIGNALS definitions are meant to cover these cases.
A signal may be "special," but have a form which allows it to be
manipulated by the generic routines, in which case it should also have
the appropriate generic field settings. */

#define IS_TAGGED_FEA(s) (s && s->header && s->header->esps_hdr && \
	 (s->header->magic == ESPS_MAGIC) && s->header->esps_hdr->hd.fea && \
	  s->header->esps_hdr->common.tag)


#define SPECIAL_SIGNALS	0xff000
#define SIG_GENERAL	0x00000
#define SIG_LPC_POLES	0x01000
#define SIG_F0		0x02000
#define SIG_FORMANTS	0x03000
#define SIG_SPECTROGRAM 0x06000
#define SIG_LPCA	0x07000
#define SIG_LPCRC	0x08000

/* SIGNAL FORMATS  for external (disc file) representation */
#define SIG_FORMAT 0xf00	/* probably don't need all these bits... */
#define SIG_BINARY 0x000
#define SIG_ASCII 0x100
#define SIG_REVERSED 0x800
/* The SIG_REVERSED flag indicates whether the data is to be byte-swapped
   during binary read and write operations.
   SIG_REVERSED is not yet implemented in a general way. */

#ifndef HEADER_MAGIC
/* Nirvonics magic number */
#define HEADER_MAGIC 0xffff0100ff /* FOR VAX FILES PROCESSED BY dd conv=swab */
#endif
#define RHEADR_MAGIC 33554176	  /* FOR NON-BYTE-SWAPPED VAX FILES */
#define SPPACK_MAGIC 16579	  /* SPPACK 2.0 (Peter Kroon) */

/* use a magic number that makes sense as ASCII: "SIG\n" */
/* #define SIGNAL_MAGIC 0x5349470a	Machine-dependent!!, but, if defined
this way, the magic number may be used to generate the permutation function. */
/* #define SIGNAL_MAGIC  (*(int*)("SIG\n")) alignment problems on SPARC */
/* This should be statically defined.  How?? */
#define SIGNAL_MAGIC signal_magic()

#define ESPS_MAGIC HD_CHECK_VAL

/* signal file state */
#define SIG_CLOSED    -1    /* closed but unaltered and named file exists */
#define SIG_NEW       -2    /* newly created, must create file and file name*/
#define SIG_UNKNOWN   -3    /* status of file not specified */


typedef struct head {
    int		    magic;	/* header type */
    int		    nbytes;	/* number of bytes pointed to by header */
    char	    *header;	/* header data (usually a simple byte array) */
    struct header   *esps_hdr;	/* ESPS header (if ESPS file) */
    int		    esps_nbytes; /* number of bytes to skip for ESPS header */
    int             e_scrsd;	/* flag for single-channel, real (non-complex)
                                   ESPS FEA_SD file (i.e., can be treated
                                   like old SD files -- read as SHORT)*/
    int             e_short;	/* flag to indicate same info as e_scrsd 
				   and furthermore that "samples" field 
				   is a SHORT in the disk file*/
    int		    esps_clean;
				/* 1 means that the header was read and
				   it is OK to free it;
				   otherwise
				   it is not safe to free it and this
				   should be set to zero.
			        */
    FILE	    *strm;	/* stream pointer (if ESPS file) */
    int		    npad;	/* number of padding bytes at end of header
				   (to maintain modulo-4) */
} Header;

typedef struct list {
  char *str;
  struct list *next;
} List;

/* The structure used to describe all signals. */
typedef struct sig {
  char *name;			/* the file name (if any)  */
  int file;			/* file descriptor (<0 if closed) */
  /* general info, mostly settable by signal header */
  int dim;			/* if <= 0 signal is variable dimension */
  int type;			/* a key to the structure of the data */
  int *types;			/* vector of types when type is P_MIXED */
  int version;			/* version (generation) number of signal */
  int file_size;		/* number of samples in the file */
  int buff_size;		/* number of samples in data buffer */
  int start_samp;		/* first sample in buffer ( current signal) */
  int bytes_skip;		/* # of bytes to skip to access signal start */
  double start_time;		/* start time (sec) re "original" signal */
  double end_time;		/* used by aperiodic signals */
  double freq;			/* if <= 0, then signal is aperiodic */
  double band;			/* effective bandwidth (e.g. for LPC) */
  double band_low;		/* band lower limit */
  double *y_dat;	/* y values for vector elements; e.g. ARB_FIXED freq */
  double *x_dat;	/* x values corresponding to indices (e.g. time) */
  List *idents;			/* labels for each vector element (channel) */
  /* misc. internal data */
  Header *header;		/* complete file header (if any) */
  double *smax, *smin;		/* max. and min.arrays of dimension dim */
  caddr_t data;		/* data buffer (coerced to appropriate type) */
  caddr_t params;      /* pointer to parameters used to compute this Signal */
  /* internal & external methods */
  Utils *utils;			/* internal "utility" methods list */
  Methods *methods;		/* `message' methods list */
  /* connections */
  caddr_t obj;			/* pointer to "parent" object */
  struct sig *others;		/* related signals */
  struct view *views;		/* linked list of views on this signal */
} Signal;

struct general {		/* generalized sample descriptor */
  double time;			/* event time */
  char *type;			/* type of data element */
  int  dim;			/* number of dimensions */
  int *sizes;			/* size of each dimension */
  caddr_t data;		/* coerced pointer to actual data elements */
  struct general *next;
};

struct asample {		/* aperiodic sample descriptor */
  double time;			/* event time */
  int size;			/* number of elements comprising sample */
  caddr_t data;		/* gets coerced to appropriate type */
  struct asample *next;
};

struct var_rec {		/* periodic, variable size sample descriptor */
  int size;
  caddr_t data;
  struct var_rec *next;
};
