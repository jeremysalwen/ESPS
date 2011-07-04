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
 * Written by:  Joe Buck
 * Checked by:
 * Revised by:  Ken Nelson
 *
 * Brief description: reads any ESPS file and outputs in reverse order.
 *
 */

static char *sccs_id = "@(#)ereverse.c	3.5	9/21/98	ESI/ERL";
int debug_level = 0;

/*********************************************************
*
*  Module Name: ereverse.c
*
*  Written By:   Joseph Buck; revised for ESPS 3.0 by John Shore
*
*  Description: This program reads any ESPS file and writes out
*	        the records in reverse.
*		
*  Secrets:     Not too tricky.  The file is divided into blocks,
*		and the blocks are read in using seek directives.
*		The records are treated as size_rec-byte records
*		of undefined structure.
*********************************************************/

#include <stdio.h>
#include <esps/esps.h>
#include <esps/esignal_fea.h>
#include <esps/pc_wav.h>

#define DBUFSIZ 65536
static char dbuf[DBUFSIZ];
static char *Program = "ereverse";
static char *Version = "3.5";
static char *Date = "9/21/98";

static FILE *istrm, *ostrm = NULL;
static char *infile, *outfile;
static int sizrec;
static long fpos;
extern void lrange_switch();
extern int optind;
extern char *get_cmd_line(), *strcpy(), *optarg;
extern char *get_sphere_hdr();

main(argc,argv)
char **argv;
{
    struct header *ih, *oh;
    long nrecords, hpos, srec = 0, erec = -1, nrec_move;
    int nrec_shot, remainder, c, r_flag = 0;
    char *range;

    ostrm = stdout;

    while ((c = getopt (argc, argv, "x:r:")) != EOF)
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg); 
		break;
	    case 'r': 
		range = optarg;
		r_flag++;
		break;
	    default:  syntax();
	}
    if (argc != optind + 2) syntax();
    infile = argv[optind++];
    outfile = argv[optind];
/* Open file and read header */
    if (strcmp (infile, "-") == 0) {
	Fprintf(stderr, "ereverse: can't use stdin (-)\n");
	exit(1);
    }
    TRYOPEN (argv[0], infile, "r", istrm);
    if ((ih = read_header (istrm)) == NULL)
	NOTSPS (argv[0], infile);

    if(!ih->common.edr && edr_default(ih->common.machine_code))
	ih->common.edr = YES;
    else if (!ih->common.edr && ih->common.machine_code != MACH_CODE) {
	Fprintf(stderr, "ereverse: Cannot process a NATIVE format file from another machine type.\n");
	exit(1);
    }

    if (get_sphere_hdr(ih))
    {
	Fprintf(stderr, "ereverse: not all Sphere formats supported; ");
	Fprintf(stderr, "data may be corrupted.\n");
    }

    if (get_esignal_hdr(ih))
    {
	Fprintf(stderr, "ereverse: sorry, Esignal format not supported.\n");
	exit(1);
    }

    if (get_pc_wav_hdr(ih))
    {
	Fprintf(stderr, "ereverse: sorry, PC WAVE format not supported.\n");
	exit(1);
    }    

/* Build output header */
    oh = copy_header (ih);

/* Since fread and fwrite are used, the EDR status of the output
   file must be same as the input file
*/
    oh->common.edr = ih->common.edr;

    add_source_file (oh, infile, ih);
    add_comment (oh, get_cmd_line (argc, argv));
    Strcpy (oh->common.prog, Program);
    Strcpy (oh->common.vers, Version);
    Strcpy (oh->common.progdate, Date);
    sizrec = size_rec (ih);
    if (sizrec == -1)
    {
	Fprintf(stderr, "ereverse: variable record size not supported.\n");
	exit(1);
    }
    if (sizrec != size_rec(oh))
    {
	Fprintf(stderr, "ereverse: sorry, can't handle this format.\n");
	exit(1);
    }

    if (sizrec > DBUFSIZ) {
	Fprintf (stderr, "ereverse: records too large to fit in buffer\n");
	exit (1);
    }
    nrec_shot = DBUFSIZ / sizrec;	/* # we can handle at one time */
    nrecords = ih->common.ndrec;
    if (nrecords < 0) {
	Fprintf(stderr, "ereverse: input from pipe not allowed\n");
	exit(1);
    }
/*
 * read range from SPS common, if range option not used; 
 * the filename in common match that of the input SD file
 */
    if (!r_flag) (void) read_params((char *)NULL, SC_CHECK_FILE, infile);
    if (r_flag) 
	lrange_switch (range, &srec, &erec, 1);	
    else {
	if(symtype("start") != ST_UNDEF) srec = getsym_i("start");
	if(symtype("nan") != ST_UNDEF) erec = srec + getsym_i("nan") - 1; 
    }
    symerr_exit();  /*exit if any of the parameters were missing
	              -- not really needed*/
    if (srec <= 0) srec = 1;
    if (erec < 0) erec = nrecords;
    else if (erec > nrecords) {
	Fprintf (stderr, "ereverse: only %ld records in %s\n", nrecords, infile);
	exit (1);
    }
/* Open the output file and write the header */
    if (strcmp (outfile, "-") != 0)
	TRYOPEN (argv[0], outfile, "w", ostrm);
    (void)write_header (oh, ostrm);
    nrec_move = erec - srec + 1;
    remainder = nrec_move % nrec_shot;
    hpos = ftell (istrm);		/* Where we are now, -> first record */
    fpos = hpos + (erec - remainder) * sizrec;
    if (debug_level) {
	Fprintf (stderr,
		"%ld records total, moving %ld records, %d bytes/record\n",
		nrecords, nrec_move, sizrec);
	Fprintf (stderr, "%d records/shot, remainder %d\n", nrec_shot, remainder);
	Fprintf (stderr, "hpos = %ld, seeking to offset %ld\n", hpos, fpos);
    }
/* Set hpos to starting position -- it's an end marker */
    hpos += (srec - 1) * sizrec;
/* Do the last (odd-size) block */
    if (remainder > 0)
	revrec (remainder);
/* Do all the other blocks */
    while (fpos > hpos) {
	fpos -= sizrec * nrec_shot;
	if (debug_level)
	    Fprintf (stderr, "seeking to offset %ld\n", fpos);
	revrec (nrec_shot);
    }
/*
 * put output file info in ESPS common
 */
    if (ostrm != stdout) {
	(void) putsym_s("filename", outfile);
	(void) putsym_s("prog", "ereverse");
	(void) putsym_i("start", (int) 1);
	(void) putsym_i("nan", (int) nrec_move);
    }
    return 0;
}

/*
 * This function reads a block, and writes it backwards
 */
revrec (nrecs)
register int nrecs;
{
    register char *dp;
    if (fseek (istrm, fpos, 0) != 0) {
	    Fprintf (stderr, "ereverse: seek error on ");
	    perror (infile);
	    exit (1);
    }
    if (fread (dbuf, sizrec, nrecs, istrm) != nrecs) {
	Fprintf (stderr, "ereverse: read error on ");
	perror (infile);
	exit (1);
    }
    for (dp = dbuf + (nrecs - 1) * sizrec; dp >= dbuf; dp -= sizrec)
	if (fwrite (dp, sizrec, 1, ostrm) != 1) {
	    Fprintf (stderr, "ereverse: write error on ");
	    perror (infile);
	    exit (1);
	}
}

syntax() {
    USAGE ("ereverse [-x level] [-r range] infile outfile");
}

