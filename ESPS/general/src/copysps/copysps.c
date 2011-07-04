/*
 * This material contains unpublished, proprietary software of Entropic
 * Research Laboratory, Inc. Any reproduction, distribution, or publication
 * of this work must be authorized in writing by Entropic Research
 * Laboratory, Inc., and must bear the notice:
 * 
 * "Copyright (c) 1987-1990  Entropic Speech, Inc. "Copyright (c) 1990-1996
 * Entropic Research Laboratory, Inc. All rights reserved"
 * 
 * The copyright notice above does not evidence any actual or intended
 * publication of this source code.
 * 
 * Written by:  Ajaipal S. Virdy Checked by: Revised by: Alan Parker
 * 
 * Copy records of an ESPS file to a new ESPS file or append to an existing ESPS
 * file.
 * 
 */

#ifdef SCCS
static char    *sccs_id = "@(#)copysps.c	3.24	6/3/98	ESI/ERL";
#define VERSION "3.24"
#define DATE "6/3/98"
#endif

#ifndef VERSION
#define VERSION "debug"
#endif
#ifndef DATE
#define DATE "none"
#endif

#include <stdio.h>
#include <errno.h>

#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/spec.h>
#include <esps/filt.h>
#include <esps/scbk.h>
#include <esps/unix.h>
#include <esps/esignal_fea.h>


#define SYNTAX USAGE ("copysps [-x debug_level] [-f] [-r|-p gen_range] [-s time_range ] [-z] [infile] outfile")
#define ERROR_EXIT(text) {(void) fprintf(stderr, "copysps: %s - exiting\n", text); exit(1);}

int             debug_level = 0;

extern void     trange_switch();
char           *e_temp_name();
char           *get_sphere_hdr();

main(argc, argv)
	int             argc;
	char          **argv;
{				/* begin main */

	int             c, getopt();
	extern int      optind;
	extern int      errno;
	extern char    *optarg;

	char           *get_cmd_line();

	char           *f1name, *f2name, *tmpname;
	FILE           *f1strm, *f2strm, *tstrm;

	struct header  *f1hd, *f2hd;

	char           *dbuf;	/* array to hold data read in */

	int             debug = 0;
	int             rflag = 0;
	int             gflag = 0;
	long           *rec_range = NULL;
	long            tot_rec;
	int             sflag = 0;
	char           *srange = NULL;
	char           *rrange = NULL;
	long            input_ndrec = 0;

	long            i, s_rec = 1, e_rec;

	int             fast = 0;
	int             nohead = 0;
	int             update = 0;	/* set if we have to update output
					 * file */
	int             nofile = 0;	/* set if no input file on command
					 * line */
	int             zflag = 0;

	long            cur_pos, new_pos;	/* variables for positioning
						 * seek() */


	char           *template = "copyspsXXXXXX";
	struct fea_data *f1rec, *f2rec;

	/* ESPS functions referenced */

	long           *grange_switch();
	int             size_rec();
	int             recsize1, recsize2;
	char           *eopen();
	char           *savestring();
	int             j;
	/* Local functions referenced */

	void            check_sd();
	void            check_spec();
	void            check_filt();
	void            add_sd_comment();
	void            add_spec_comment();
	void            add_filt_comment();
	long            common_nan;
	while ((c = getopt(argc, argv, "x:fr:p:s:hzg:")) != EOF)
		switch (c) {
		case 'x':
			debug = atoi(optarg);
			break;
		case 'p':
		case 'g':
			rec_range = grange_switch(optarg, &tot_rec);
			gflag = 1;
			break;
		case 's':
			srange = optarg;
			sflag = 1;
			break;
		case 'r':
			rrange = optarg;
			rflag = 1;
			break;
		case 'f':
			fast++;
			break;
		case 'h':
			nohead++;
			break;
		case 'z':
			zflag++;
			break;
		default:
			SYNTAX;
		}

	/* Determine if output file is specified */
	if (argc - optind < 1) {
		Fprintf(stderr, "copysps: no output file specified.\n");
		SYNTAX;
	}
	/*
	 * If f2name is "-", then use stdout for f2strm. If f2name exists,
	 * then update the file, otherwise create it.
	 */

	f2name = argv[argc - 1];
	if (strcmp(f2name, "-") != 0)
		if ((f2strm = fopen(f2name, "r")) == NULL)
			update = 0;	/* outfile doesn't exist */
		else
			update = 1;	/* outfile exists */

	if (argc - optind < 2) {/* only output file name specified */
		/* user must supply two file names (or "-") */
		/* SYNTAX; */
		nofile = 1;
		f1name = NULL;
	} else {		/* open input file */
		f1name = eopen("copysps", argv[optind], "r", NONE, NONE,
			       &f1hd, &f1strm);
		if (strcmp(f1name, "<stdin>") == 0) {
			ERROR_EXIT("Standard input not supported");
		}
	}


	/*
	 * If command line has no input file name, then it might be in ESPS
	 * common; we need to read_params anyway (provided stdin not used),
	 * so we do it here (checking common only); However, we don't check
	 * common if more than one input file is given since this is error
	 * prone.
	 */

	if (nofile || ((f1strm != stdin) && (argc - optind <= 2))) {
		if (debug)
			Fprintf(stderr, "copysps: checking ESPS common\n");
		(void) read_params((char *) NULL, SC_CHECK_FILE, f1name);

	}
	if (nofile) {
		if (symtype("filename") == ST_UNDEF) {
			ERROR_EXIT("no input file name on command line or in common");
		} else {
			f1name = getsym_s("filename");

			if (debug)
				Fprintf(stderr,
					"copysps: input file name from common is %s\n",
					f1name);

			if (strcmp(f2name, f1name) == 0)
				ERROR_EXIT("input name from common same as output file");

			(void) eopen("copysps", f1name, "r",
				     NONE, NONE, &f1hd, &f1strm);
			if (!f1hd->common.edr && edr_default(f1hd->common.machine_code))
				f1hd->common.edr = YES;
			if (!zflag)
				Fprintf(stderr,
					"copysps: input file name %s taken from ESPS Common.\n",
					f1name);
		}
	}
	s_rec = 1;
	e_rec = (f1hd->common.ndrec != -1 ? f1hd->common.ndrec : LONG_MAX);

	if (rflag)
		range_switch(rrange, &s_rec, &e_rec, 1);

	if (sflag) {
		s_rec = 1;
		e_rec = f1hd->common.ndrec;
		trange_switch(srange, f1hd, &s_rec, &e_rec);
		if (s_rec < 0) {
			ERROR_EXIT("requested start time is less than start_time of file");
		}
	}
	if (!gflag && !sflag && !rflag) {
		s_rec = 1;
		e_rec = f1hd->common.ndrec;
		if (symtype("start") != ST_UNDEF) {
			s_rec = getsym_i("start");
			if (!zflag)
				Fprintf(stderr,
					"copysd: starting record = %ld taken from ESPS Common\n",
					s_rec);
		}
		if (symtype("nan") != ST_UNDEF) {
			common_nan = getsym_i("nan");
			if (!zflag)
				Fprintf(stderr,
					"copysd: number records = %ld taken from ESPS Common\n",
					common_nan);
			if (common_nan != 0)
				e_rec = s_rec + common_nan - 1;
		}
	}
	if (strcmp(f1name, f2name) == 0) {
		ERROR_EXIT("infile and outfile cannot be the same");
	}
	if (debug)
		Fprintf(stderr,
			"copysps: infile = %s, outfile = %s, update = %d, fast = %d\n",
			f1name, f2name, update, fast);

	/* Check if outfile exists and is an ESPS file */

	if (update) {		/* outfile exists */

		if (!(f2hd = read_header(f2strm)))
			NOTSPS("copysps", f2name);
		if (get_esignal_hdr(f2hd)) {
			ERROR_EXIT("cannot append to an Esignal file");
		}
		if (get_sphere_hdr(f2hd)) {
			ERROR_EXIT("cannot append to a Sphere file");
		}
		if (get_pc_wav_hdr(f2hd)) {
			ERROR_EXIT("cannot append to a PC WAV (RIFF) file");
		}
		input_ndrec = f2hd->common.ndrec;

		/* Check for compatibility */

		if (debug)
			Fprintf(stderr,
				"copysps: %s already exists; check for compatibility.\n",
				f2name);

		switch (f2hd->common.type) {
		case FT_FEA:{
				short         **trans;	/* translation table
							 * returned */
				int             com_lev;	/* compatibility level */

				if (debug)
					Fprintf(stderr,
					  "copysps: calling fea_compat.\n");

				com_lev = fea_compat(f1hd, f2hd, (char **) NULL, &trans);

				if (debug)
					Fprintf(stderr,
						"copysps: com_lev = %d, trans is %s.\n", com_lev,
						(trans == NULL) ? "NULL" : "not NULL");

				if ((com_lev != 1) || (trans != NULL)) {
					Fprintf(stderr,
						"copysps: fea_compat: feature fields in %s and %s incompatible.\n",
						f1name, f2name);
					exit(1);
				}
				/*
				 * check record_freq, if it exists and files
				 * FEA_SD
				 */
				if (f2hd->hd.fea->fea_type == FEA_SD) {
					if (get_genhd_val("record_freq", f1hd, (double) 0) !=
					    get_genhd_val("record_freq", f2hd, (double) 0)) {
						Fprintf(stderr,
							"copysps: %s and %s have different record_freq - exiting.\n", f1name, f2name);
						exit(1);
					}
				}
			}
			break;
		default:
			Fprintf(stderr,
			 "copysps: cannot update ESPS file type code: %d\n",
				f1hd->common.type);
			exit(1);
			break;
		}		/* end switch (f2h->common.type) */

		if (debug)
			Fprintf(stderr,
				"copysps: input files compatible; now check for fast mode.\n");

		if (fast) {
			/*
			 * fast flag is set, append selected records to the
			 * output file.
			 */
			if (debug)
				Fprintf(stderr,
					"copysps: fast mode set: append to %s.\n", f2name);
			(void) fclose(f2strm);
			if ((f2strm = fopen(f2name, "a+")) == NULL) {
				/*
				 * ugly hack to fix r+ on SYS 5.2
				 * tstrm->_flag |= _IOWRT;
				 */
				Fprintf(stderr,
					"copysps: could not open %s for appending.\n", f2name);
				exit(1);
			}
		} else {
			/*
			 * We have to update outfile, but not in fast mode!
			 * This means that we have to retain all header
			 * information and include as a subheader the header
			 * of infile. Therefore, create a temporary file to
			 * store results.
			 */
			tmpname = e_temp_name(template);
			if ((tstrm = fopen(tmpname, "w")) == NULL) {
				Fprintf(stderr,
				   "copysps: could not open %s\n", tmpname);
				exit(1);
			}
			if (debug)
				Fprintf(stderr,
					"copysps: fast mode not set: creating tmp file %s.\n",
					tmpname);
		}		/* end if (fast) */

		/*
		 * check record_freq values
		 */
		{
			double          record_freq = 0;
			if (get_genhd_val("record_freq", f1hd, (double) 0) !=
			    get_genhd_val("record_freq", f2hd, (double) 0))
				(void) add_genhd_d("record_freq", &record_freq,
						   (int) 1, f2hd);
		}

	}
	/* end outfile exists */
	else {			/* outfile doesn't exist */

		if (debug)
			Fprintf(stderr, "copysps: %s does not exit, create it.\n",
				f2name);

		if (strcmp(f2name, "-") == 0) {
			f2strm = stdout;
			f2name = savestring("<stdout>");
		} else if ((f2strm = fopen(f2name, "w")) == NULL) {
			Fprintf(stderr,
			"copysps: could not open %s for writing.\n", f2name);
			exit(1);
		}
		fast = 0;	/* reset fast flag if on */
		f2hd = copy_header(f1hd);
		f2hd->common.edr = f1hd->common.edr;

		if (debug)
			Fprintf(stderr,
			     "copysps: copied infile header to outfile.\n");

		/*
		 * update start time and record freq
		 */
		if (rec_range == NULL)
			update_waves_gen(f1hd, f2hd, (double) s_rec, 1.0);
		else
			update_waves_gen(f1hd, f2hd, (double) rec_range[0], 1.0);


	}			/* end if (update) */

	if (!fast) {
		if ((debug) && !nohead)
			Fprintf(stderr,
			"copysps: adding source file header to outfile.\n");
		if (nohead)
			(void) add_source_file(f2hd, f1name, (struct header *) NULL);
		else
			(void) add_source_file(f2hd, f1name, f1hd);
	}
	if (debug)
		Fprintf(stderr,
			"copysps: s_rec: %d, e_rec: %d\n", s_rec, e_rec);

	if (gflag)
		if (rec_range[tot_rec - 1] > e_rec) {
			Fprintf(stderr, "copysps: only %d records in %s\n",
				f1hd->common.ndrec, f1name);
			exit(1);
		}
	recsize1 = size_rec(f1hd);
	if (recsize1 == -1) {
		ERROR_EXIT("variable record size not supported")
	}
	(void) skiprec(f1strm, s_rec - 1, recsize1);

	if (!gflag)
		tot_rec = e_rec - s_rec + 1;

	if (debug)
		Fprintf(stderr,
			"copysps: recsize1 = %ld, tot_rec = %ld\n", recsize1, tot_rec);

	if (!fast) {		/* retain all new header information in
				 * outfile */
		(void) strcpy(f2hd->common.prog, "copysps");
		(void) strcpy(f2hd->common.vers, VERSION);
		(void) strcpy(f2hd->common.progdate, DATE);
		if (!nohead) {
			(void) add_comment(f2hd, get_cmd_line(argc, argv));
		}
		/*
		 * If we have to update an existing file, then we should add
		 * any warnings in the comment field if any inconsistencies
		 * occur.
		 */

		if (update)
			switch (f2hd->common.type) {
			case FT_FEA:	/* do nothing */
				break;
			default:
				Fprintf(stderr,
					"copysps: cannot update ESPS file type code: %d\n",
					f1hd->common.type);
				exit(1);
				break;
			}	/* end switch (f2h->common.type) */

		/*
		 * If in update mode, then we must write header information
		 * out into a temporary file (pointed to by tstrm), otherwise
		 * just write it into our new outfile (pointed to by f2strm).
		 */

		if (debug)
			Fprintf(stderr,
				"copysps: writing header to %s stream.\n",
				(update == 0) ? f2name : tmpname);

		if (update)
			(void) write_header(f2hd, tstrm);
		else
			(void) write_header(f2hd, f2strm);
	}
	if (debug)
		Fprintf(stderr, "copysps: Process records\n");

	/*
	 * If we have to update outfile and we're not in fast mode, then we
	 * have to do the following: Copy all records from outfile into a
	 * temporary file.  Then append to the temporary file the the new
	 * records from infile.  Finally, we'll have to get everything from
	 * the temporary file back into outfile.
	 */

	if (update && !fast) {	/* append mode */
		long            tmp_rec;
		struct fea_data *tmpbuf;

		if (debug)
			Fprintf(stderr,"copysps: to temp ");
		recsize2 = size_rec(f2hd);
		if (recsize2 == -1) {
			ERROR_EXIT("variable record size not supported")
		}
		if (recsize2 != recsize1) {
			Fprintf(stderr,
				"copysps: record sizes in %s and %s are different.\n",
				f1name, f2name);
			exit(1);
		}
		tmp_rec = f2hd->common.ndrec;
		tmpbuf = allo_fea_rec(f2hd);
		for (i = 0; i < tmp_rec; i++) {
			if (get_fea_rec(tmpbuf, f2hd, f2strm) < 1) {
				Fprintf(stderr, "copysps: read error on %s\n", f2name);
				exit(1);
			}
			put_fea_rec(tmpbuf, f2hd, tstrm);
			if (debug && (i%1000 == 0)) fputc('#',stderr);
		}
		if(debug) fputc('\n',stderr);
		(void) free_fea_rec(tmpbuf);
	}			/* end if (update && !fast) */
	/*
	 * The pointer in the input stream is now set to the correct record
	 * position (because of read_header and skiprec). 
	 */
	cur_pos = ftell(f1strm);/* current position in f1strm (record 1) */

	/* for (i = 0; i < tot_rec; i++) */

	i = 0;
	f1rec = allo_fea_rec(f1hd);
	f2rec = allo_fea_rec(f2hd);
	if (debug) 
		Fprintf(stderr,"copysps: to temp or output ");
	while (i < tot_rec) {

		if (gflag) {
			new_pos = (rec_range[i] - 1) * recsize1 + cur_pos;

			if ((j = fseek(f1strm, new_pos, 0)) != 0) {
				break;
				/*
				 * Fprintf (stderr, "copysps: seek error on
				 * %s\n", f1name); exit (1);
				 */
			}
		}
		if (get_fea_rec(f1rec, f1hd, f1strm) < 1) {
			break;
			/*
			 * Fprintf (stderr, "copysps: read error on %s\n",
			 * f1name); exit (1);
			 */
		}
		copy_fea_rec(f1rec, f1hd, f2rec, f2hd, NULL, NULL);
		if (update && !fast) {
			put_fea_rec(f2rec, f2hd, tstrm);
		} else {
			put_fea_rec(f2rec, f2hd, f2strm);
		}
		if (debug && (i%1000 == 0)) fputc('#',stderr);
		i++;

	}			/* end while (i < tot_rec) */

	/* i is the total number of records processed */

	if (debug)
		Fprintf(stderr,
			"\ncopysps: number of records processed = %d\n", i);

	if (update && !fast) {
		if (rename(tmpname, f2name) != 0) {
			if (errno != EXDEV) {
				Fprintf(stderr,
					"copysps: could not rename %s to %s\n", tmpname, f2name);
				exit(1);
			} else {
				int i;
				if (debug) 
					Fprintf(stderr,"copysps: from temp ");
				(void) fclose(tstrm);
				if ((tstrm = fopen(tmpname, "r")) == NULL) {
					Fprintf(stderr, "copysps: cannot reopen temp file!\n");
					exit(1);
				}
				(void) unlink(tmpname);
				if ((f2strm = fopen(f2name, "w")) == NULL) {
					Fprintf(stderr, "copysps: cannot open %s\n", f2name);
					exit(1);
				}
				i=0;
				while ((c = fgetc(tstrm)) != EOF) {
					(void) fputc(c, f2strm);
					if (debug && (i++%1000 == 0)) { 
						fputc('#',stderr);
					}
				}
				if(debug) 
					Fprintf(stderr,
					   "\ncopysps: %d bytes processed\n",i);
			}
		}
		(void) fclose(tstrm);
	}
	(void) fclose(f1strm);
	(void) fclose(f2strm);

	/* put output file info in ESPS common */

	if (strcmp(f2name, "<stdout>") != 0) {
		(void) putsym_s("filename", f2name);
		(void) putsym_s("prog", argv[0]);
		(void) putsym_i("start", 1);
		(void) putsym_i("nan", (int) (tot_rec + input_ndrec));
	}
	exit(0);
	/* NOTREACHED */

}				/* end main() */
