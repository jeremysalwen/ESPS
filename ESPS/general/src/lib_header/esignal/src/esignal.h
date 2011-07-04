/*
 * Copyright (c) 1995 Entropic Research Laboratory, Inc.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this Esignal (TM)
 * I/O software and its documentation for any purpose and without fee
 * is hereby granted, provided that the above copyright notice and this
 * statement of permission appears in all copies of the software as
 * well as in supporting documentation.  Entropic Research Laboratory,
 * Inc. makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * Esignal (TM) is a Trademark of Entropic Research Laboratory, Inc.
 */

/*
 * @(#)esignal.h	1.10 10/6/97 ERL
 *
 * Example programs for Esignal public external file format.
 * Include file.
 *
 * Author:  Rod Johnson
 */

#ifndef esignal_h
#define esignal_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>

/*
 * CONSTANTS
 */

#define TRUE	1
#define FALSE	0

/* Preamble */

#define PREAM_MAX   8		/* longest preamble line (incl. '\n') */
#define MAGIC	    "Esignal"	/* "magic number" */
#define VERSION	    "0.0B"	/* Esignal external format version */


/* Characters */

#define DOT	'.'	/* Field name separator. */

/* Types */

    /* If you add a new type: 
     * (1) Don't change the existing type code definitions.
     * (2) Update MAX_TYPE below.
     * (3) If the new type name is longer than "DOUBLE_COMPLEX",
     *     update MAX_TYPE_LEN below.
     * (4) Update the string array Types in esignal.c,
     *     the functions ValidType and TypeName in esignal.c.
     * (5) Update other functions in esignal.c and esig_*.c
     *     that need to give meaning to the new type (e.g.
     *     InternTypeSize, TypeOrder in esignal.c,
     *     EdrTypeSize, EdrRead, EdrWrite in esig_edr.c
     *     AsciiRead, AsciiWrite, ApproxWidth in esig_asc.c).
     */
#define	NO_TYPE		1
#define	ARRAY		2
#define	DOUBLE		3
#define	FLOAT		4
#define	LONG		5
#define	ULONG		6
#define	SHORT		7
#define	USHORT		8
#define	SCHAR		9
#define	UCHAR		10
#define	BOOL		11
#define	DOUBLE_COMPLEX	12
#define	FLOAT_COMPLEX	13
#define	LONG_COMPLEX	14
#define	SHORT_COMPLEX	15
#define	SCHAR_COMPLEX	16
#define	CHAR		17
#define	WCHAR		18
#define MAX_TYPE	WCHAR	/* Largest type code */

    /* length of longest type name (including terminating null) */

#define MAX_TYPE_LEN	sizeof("DOUBLE_COMPLEX")

/* Occurrence Classes */

#define	GLOBAL		1
#define	REQUIRED	2
#define	OPTIONAL	3
#define	VIRTUAL		4
#define	INCLUDED	5

/* Field_order? */

#define TYPE_ORDER	0
#define FIELD_ORDER	1

/* Format Variant */

#define UNKNOWN		0
#define EDR1		1
#define EDR2		2
#define NATIVE		3
#define ASCII		4

/*
 * TYPEDEFS AND STRUCTURES
 */

/* Types */

typedef unsigned long			Ulong;
typedef unsigned short			Ushort;
typedef signed char			Schar;
typedef unsigned char			Uchar;
typedef struct {double	real, imag;}	DoubleComplex;
typedef struct {float	real, imag;}	FloatComplex;
typedef struct {long	real, imag;}	LongComplex;
typedef struct {short	real, imag;}	ShortComplex;
typedef struct {Schar	real, imag;}	ScharComplex;
typedef unsigned char			Bool;
typedef unsigned short			Wchar;

/* Arrays */

typedef struct Array	Array;

struct Array {
    short       type;           /* data type code */
    short       rank;           /* number of dimensions */
    long        *dim;           /* vector of dimensions */
    void        *data;          /* storage area for data */
};

/* Field Specifications */

typedef struct FieldSpec    FieldSpec;
typedef FieldSpec	    **FieldList;

struct FieldSpec {
    short       type;           /* data type code */
    short       rank;           /* number of dimensions */
    long        *dim;           /* vector of dimensions */
    short       occurrence;     /* REQUIRED, GLOBAL, OPTIONAL, etc. */
    char        *name;          /* identifying character string */
    FieldList   subfields;      /* field specs of subfields */
    char        *units;         /* string giving physical units */
    double      scale, offset;  /* scale factor and offset relating
				   raw numbers to physical quantities */
    char        **axis_names;   /* optional strings identifying axes */
    void        *data;          /* GLOBAL data area */
    Bool	present;	/* is OPTIONAL field present in record? */
    char	*fullname;	/* name including parent name, if any,
				   prefixed with connecting "." */
};

/* Ascii Annotations */

typedef struct Annot	Annot;

struct Annot {
    int     position;
    int     indent;
    int     width;
    long    recnum;
};

/*
 * FUNCTION DECLARATIONS
 */

/*
 * Miscellaneous.
 */

char	    *StrDup(char *str);
long	    LongProd(int n, long *arr);

/*
 * Debug Output
 */

void 	    DebugPrint(char *msg);

/*
 * Types.
 */

int	    ValidType(int type);
char	    *TypeName(int type);
int	    TypeCode(char *name);
long	    InternTypeSize(int type);
long	    ExternTypeSize(int type, int arch);

/*
 * Field specifications and lists.
 */

FieldSpec   *NewFieldSpec(int type, int rank);
void	    FreeFieldSpec(FieldSpec *spec);
void	    FreeFieldList(FieldList list);
void	    FreeAxisNames(char **axis_names, int rank); /* Not part of
							 * public interface */
long	    FieldLength(FieldSpec *field);
int	    FieldListLength(FieldList list);
FieldSpec   *FindField(FieldList list, char *name);
FieldSpec   *GetField(FieldList list, char *name);	/* Not part of
							 * public interface */
char	    *FirstComponent(char *name);		/* Not part of
							 * public interface */
int	    AddField(FieldList *list, FieldSpec *field);
int	    AddSubfield(FieldSpec *field, FieldSpec *subfield);
FieldSpec   **FieldOrder(FieldList list);
FieldSpec   **TypeOrder(FieldList list);
int	    GetFieldOrdering(FieldList list, int *order);
int	    SetFieldOrdering(FieldList *list, int order);

/*
 * General I/O.
 */

int	    ReadPreamble(char **version, char **arch, long *pre_size,
			 long *hdr_size, long *rec_size, FILE *file);
int	    ReadFieldList(FieldList *list, int arch, FILE *file);
FieldList   ReadHeader(char **version, int *arch, long *pre_size,
		       long *hdr_size, long *rec_size, FILE *file);
FieldList   OpenIn(char *filename, char **version, int *arch,
		   long *pre_size, long *hdr_size, long *rec_size,
		   FILE **file);
int	    WritePreamble(char *arch,
			 long fld_size, long rec_size, FILE *file);
int	    WriteFieldList(FieldList list,
			   int arch, FILE *file, Annot *annotate);
int	    WriteHeader(FieldList list,
			int arch, FILE *file, Annot *annotate);
int	    OpenOut(char *filename, FieldList list,
		    int arch, FILE **file, Annot *annotate);
long	    RecordSize(FieldList list, int arch);
int	    ReadRecord(FieldSpec **fields, int arch, FILE *file);
int	    WriteRecord(FieldSpec **fields,
			int arch, FILE *file, Annot *annotate);
long        ReadSamples(void *data, long nrec, FieldSpec **fields,
                        int arch, FILE *file);
long        WriteSamples(void *data, long nrec, FieldSpec **fields,
                         int arch, FILE *file, Annot *annotate);
void	    *AllocSamples(long nrec, FieldSpec **fields);

/*
 * Native binary I/O.
 */

long	    NativeTypeSize(int type);
int	    ReadNativeFieldList(FieldList *list, FILE *file);
int	    WriteNativeFieldList(FieldList list, FILE *file);
long	    NativeRecordSize(FieldList list);
int	    ReadNativeRecord(FieldSpec **fields, FILE *file);
int	    WriteNativeRecord(FieldSpec **fields, FILE *file);
long        ReadNativeSamples(void *data, long nrec, FieldSpec **fields,
                              FILE *file);
long        WriteNativeSamples(void *data, long nrec, FieldSpec **fields,
                               FILE *file);

/*
 * EDR binary I/O.
 */

long	    EdrTypeSize(int type, int longlong);
int	    ReadEdrFieldList(FieldList *list, FILE *file, int longlong);
int	    WriteEdrFieldList(FieldList list, FILE *file, int longlong);
long	    EdrRecordSize(FieldList list, int longlong);
int	    ReadEdrRecord(FieldSpec **fields, FILE *file, int longlong);
int	    WriteEdrRecord(FieldSpec **fields, FILE *file, int longlong);
long        ReadEdrSamples(void *data, long nrec, FieldSpec **fields,
                           FILE *file, int longlong);
long        WriteEdrSamples(void *data, long nrec, FieldSpec **fields,
                            FILE *file, int longlong);

/*
 * Ascii I/O.
 */

int	    ReadAsciiFieldList(FieldList *list, FILE *file);
int	    WriteAsciiFieldList(FieldList list,
				FILE *file, Annot *annotate);
int	    ReadAsciiRecord(FieldSpec **fields, FILE *file);
int	    WriteAsciiRecord(FieldSpec **fields,
			     FILE *file, Annot *annotate);
long        ReadAsciiSamples(void *data, long nrec, FieldSpec **fields,
                             FILE *file);
long        WriteAsciiSamples(void *data, long nrec, FieldSpec **fields,
                              FILE *file, Annot *annotate);

/*
 * EXTERN VARIABLES AND MACROS
 */

extern char	EsignalArch[];	    /* Architecture string in native-mode
				     * preambles.  Defined in Makefile.
				     */

extern int	DebugMsgLevel;
extern void	(*DebugMsgFunc)(char *msg);

#ifndef NoDEBUG
#define DebugMsg(Level, Msg) \
((DebugMsgLevel >= (Level) && DebugMsgFunc != NULL) \
 ? (void) (*DebugMsgFunc)(Msg) \
 : (void) 0)
#else
#define DebugMsg(Level, Msg)
#endif

#ifdef __cplusplus
}
#endif

#endif /* esignal_h */
