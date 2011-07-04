/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1990-1992  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Bill Byrne
 * Checked by:
 * Revised by:
 *
 * Brief description: zero_pad copies the fea file from the input to
 * to the output, writing additional data eithe before or after the 
 * source data.
 */

static char *sccs_id = "@(#)zero_pad.c	1.2	10/20/93	ERL";
int    debug_level = 0;

/* INCLUDE FILES */
#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>

#define PROG "zero_pad"
#define EC_SCCS_DATE "10/20/93"
#define EC_SCCS_VERSION "1.2"

int
main(argc, argv)
     int  argc;
     char **argv;
{
    extern char *optarg;
    extern optind;
    char c;

    struct header *ihd, *ohd;
    struct fea_data *irec, *orec, *zrec;
    
    FILE *fpin, *fpout;
    char *iname, *oname;

    double rec_freq = 0.0;
    int pad_length = 0;
    float pad_lengthS = 0.0;
    int lflag = 0;
    int Lflag = 0;
    int iflag = 0;
    int dflag = 0;
    int start = 1, end = LONG_MAX;
    int rec_cnt = 0;
    int atoi();
#ifndef DEC_ALPHA
    double atof();
#endif

    while ( (c= getopt( argc, argv, "x:l:L:ir:d")) != EOF )  {
	switch(c) {
	    case 'x': {
		debug_level = atoi(optarg);
		break;
	    }
	    case 'l' : {
		lflag++;
		pad_length = atoi(optarg);
		break;
	    }
	    case 'L' : {
		Lflag++;
		pad_lengthS = atof(optarg);
	    }
	    case 'i': {
		iflag++;
		break;
	    }
	    case 'r': {
		range_switch( optarg, &start, &end, 0);
		spsassert( start <= end, "in -r option, the end must be greate than the start.");
		break;
	    }
	    case 'd' :{
		dflag++;
		break;
	    }
	}
    }

    /* get input file */
    spsassert( optind < argc, "Must specify input file.");
    iname = eopen( PROG, argv[optind++], "r", FT_FEA, NONE, &ihd, &fpin);
    irec = allo_fea_rec( ihd );


    spsassert( !(lflag && Lflag), "Cannot specify both -L and -l.");
    if ( Lflag ) {
	rec_freq = get_genhd_val( "record_freq", ihd, 0.0);
	spsassert( rec_freq != 0, "-L requires header item record_freq defined in the input header.");
	pad_length = (int) (pad_lengthS / rec_freq + 0.5);
    }

    /* get output file */
    spsassert( optind < argc, "Must specify output file.");
    oname = eopen( PROG, argv[optind++], "w", NONE, NONE, NONE, &fpout);

    if ( debug_level > 1 )
	fprintf( stderr, " zero_pad: input - %s\n output - %s\n number to pad %d\n", iname, oname, pad_length);

    ohd = copy_header( ihd );
    add_comment( ohd, get_cmd_line(argc, argv));
    (void) strcpy(ohd->common.prog, PROG);
    (void) strcpy(ohd->common.vers, EC_SCCS_VERSION);
    (void) strcpy(ohd->common.progdate, EC_SCCS_DATE);
    add_source_file(ohd, iname, ihd);
    *add_genhd_l( "records_padded", NULL, 1, ohd) = pad_length;
    write_header( ohd, fpout);
    orec = allo_fea_rec( ohd );

    /* either duplicate record or use null record */
    zrec = (dflag) ? orec : allo_fea_rec( ohd );

    rec_cnt = 0;
    /* skip initial records if necessary */
    if ( start > 1 ) {
	spsassert( get_fea_rec( irec, ihd, fpin) != EOF, "Premature EOF in input file.");
	rec_cnt++;
    }

    /* prepend if necessary */
    if ( iflag ) {
	get_fea_rec( irec, ihd, fpin);
	rec_cnt++;
	copy_fea_rec( irec, ihd, orec, ohd, (char **)NULL, (long **)NULL);
	while ( pad_length-- )
	    put_fea_rec( zrec, ohd, fpout);
	put_fea_rec( orec, ohd, fpout);
    }

    /* duplicate records */
    while ( get_fea_rec( irec, ihd, fpin) != EOF && rec_cnt++ < end) {
	copy_fea_rec( irec, ihd, orec, ohd, (char **)NULL, (long **)NULL);
	put_fea_rec( orec, ohd, fpout);
    }

    /* append if necessary */
    if ( !iflag ) 
	while ( pad_length-- )
	    put_fea_rec( zrec, ohd, fpout);
    
    fclose( fpin );
    fclose( fpout );
    exit(0);
}

