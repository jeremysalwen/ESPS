/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1991  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:  Rodney Johnson
 *
 * Multiplexes sampled-data files into a single multichannel or complex file.
 *
 */

static char *sccs_id = "@(#)mux.c	1.5	8/31/95	ESI/ERL";

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/unix.h>


char	*Version = "1.5";
char	*Date = "8/31/95";

#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
                ProgName, text); exit(1);}

#define Fprintf (void)fprintf

#define SYNTAX \
USAGE("mux [-{prs} range] ... [-x debug] [-J] [-P param_file] \n input1.fsd [input2.fsd ...] output.fsd")


#define INITIAL_INPUT_SIZE 20

/*
 * global variables
 */
char	*ProgName = "mux";
int	debug_level = 0;                /* debug level */
long	get_chans();
int	typesiz();
char	*get_cmd_line();
void	lrange_switch();
int	symsize();
int	typesiz();


main(argc, argv)
int	argc;
char	**argv;

{
	char	*param_file = NULL;  /*parameter file name*/
	FILE    *out_fd = stdout;	 /* output stream */
	char	*ofile;
	struct header	*oh;
	extern int	optind;
	extern char	*optarg;
	int	ch;

	char	**r_range;
	char	**s_range;
	int	range_size = INITIAL_INPUT_SIZE;
	int	range_count = 0;
	int	*ps_point=NULL, *pnan=NULL;
	

	FILE	**in_fd;
	struct header **ih;
	char	**in_files;
	int	input_size = 0;
	long	*e_point;
	long	*s_point;
	double  *start_time_file;
	double  *start_times;
	int	start_time_chan = 0;
	long	*n_read;

	int	havestdin = 0;
	int	nfiles = 0;
	int	Jflag = 0;
	double	first_sf;
	int	i,j;
	int	first_type;
	long	num_channels;
	int	total_channels = 0;
	int	out_type;
	double	*max_value_file;
	double	*max_values;
	int	max_value_chan = 0;
	int	genhd_size;
	int	no_max_value = NO, multi_start_time;
	double	start_time_tmp, start_time_offset;
	struct	feasd **in_rec;
	struct	feasd *out_rec;
	char	*in_data, *out_data;
	int	files_done=0;
	int	prev_num_chans;
	long	output_count=0;
	int	start_size=0, nan_size=0;
	char	*cptr;


	r_range = (char **)malloc(sizeof(char *) * INITIAL_INPUT_SIZE);
	s_range = (char **)malloc(sizeof(char *) * INITIAL_INPUT_SIZE);
	in_files = (char **)malloc(sizeof(char *) * INITIAL_INPUT_SIZE);
	in_fd = (FILE * *)malloc(sizeof(FILE * ) * INITIAL_INPUT_SIZE);
	ih = (struct header **)malloc(sizeof(struct header *) * 
		INITIAL_INPUT_SIZE);
	spsassert(r_range,"mux: malloc failed");
	spsassert(s_range,"mux: malloc failed");
	spsassert(in_files,"mux: malloc failed");
	spsassert(in_fd,"mux: malloc failed");
	spsassert(ih,"mux: malloc failed");


	while ((ch = getopt(argc, argv, "p:r:s:x:JP:")) != EOF)
		switch (ch)  {
		case 'x':
			debug_level = atoi(optarg);
			break;
		case 'J':
			Jflag = 1;
			break;
		case 'P':
			param_file = optarg;
			break;
		case 'r':
		case 'p':
			range_count++;
			if (range_count > range_size) {
				r_range = (char **)realloc((char *)r_range, 
				    sizeof(char *) * range_count);
				s_range = (char **)realloc((char *)s_range, 
				    sizeof(char *) * range_count);
				range_size++;
			}
			r_range[range_count-1] = optarg;
			s_range[range_count-1] = NULL;
			break;
		case 's':
			ERROR_EXIT("s option not done yet");
			range_count++;
			if (range_count > range_size) {
				r_range = (char **)realloc((char *)r_range, 
				    sizeof(char *) * range_count);
				s_range = (char **)realloc((char *)s_range, 
				    sizeof(char *) * range_count);
				range_size++;
			}
			s_range[range_count-1] = optarg;
			r_range[range_count-1] = NULL;
			break;
		default:
			SYNTAX;
		}

/* Determine and open output file
 */
	if (argc - optind <= 1) {
		Fprintf(stderr, "mux: Input and output filenames needed.\n");
		SYNTAX;
	}

	ofile = eopen("mux", argv[argc - 1], "w", NONE, NONE, &oh, &out_fd);
	if (debug_level) 
		Fprintf(stderr, "mux: output file is %s\n", ofile);
	oh = new_header(FT_FEA);



/* open the parameter file
 */

	(void) read_params(param_file, SC_NOCOMMON, (char *) NULL);

	if(!Jflag && symtype("make_complex") != ST_UNDEF){
		cptr = getsym_s("make_complex");
		if (!strcmp(cptr, "YES") || !strcmp(cptr, "yes"))
			Jflag = 1;
	}

	if(!range_count && symtype("start") != ST_UNDEF) {
		start_size = symsize("start");
		if (start_size != -1) {
			ps_point = (int *)
				calloc((unsigned)start_size, sizeof(int));
			(void)getsym_ia("start", ps_point, start_size);
		}
	}
	if(!range_count && symtype("nan") != ST_UNDEF) {
		nan_size = symsize("nan");
		if (nan_size != -1) {
			pnan = (int *)
				calloc((unsigned)nan_size, sizeof(int));
			(void)getsym_ia("nan", pnan, nan_size);
		}
	}


/*
 * Determine input files and check them -- start by getting the first 
 * one; at the same time, we set up to create the output header
 */
	if (argc - optind < 2) {
		Fprintf(stderr, "mux: no input files on command line\n");
		SYNTAX;
	}

	while (optind < argc - 1) {
		nfiles++;
		if (nfiles > input_size) {
			ih = (struct header **)realloc(ih, 
				sizeof(struct header *) * nfiles);
			in_fd = (FILE **)realloc((char *)in_fd, 
				sizeof(FILE *) * nfiles);
			in_files = (char **)realloc((char *)in_files, 
				sizeof(char *) * nfiles);
			input_size++;
		}
		in_files[nfiles-1] = eopen("mux", argv[optind], 
			"r", FT_FEA, FEA_SD, &ih[nfiles-1], &in_fd[nfiles-1]);

		if (in_fd[nfiles-1] == stdin) {
			if (havestdin) {
				ERROR_EXIT("only one stdin allowed");
			} else
				havestdin = 1;
		}
		add_source_file(oh, in_files[nfiles-1], ih[nfiles-1]);
		optind++;
	}
	add_comment (oh, get_cmd_line(argc, argv));
	(void) strcpy (oh ->common.prog, ProgName);
	(void) strcpy (oh ->common.vers, Version);
	(void) strcpy (oh ->common.progdate, Date);

	if (range_count) {
		if (nfiles > range_size) {
			r_range = (char **)realloc((char *)r_range, 
			    sizeof(char *) * nfiles);
			s_range = (char **)realloc((char *)s_range, 
			    sizeof(char *) * nfiles);
			spsassert(r_range && s_range, "mux: realloc failed");
			range_size = nfiles;
		}
		while (nfiles > range_count++) {
			r_range[range_count-1] = r_range[range_count-2];
			s_range[range_count-1] = s_range[range_count-2];
		}
	}
	else {
		if (start_size && nfiles > start_size){
			ps_point = (int *)
				realloc((char *)ps_point, nfiles*sizeof(int));
			while (start_size++ < nfiles) {
				ps_point[start_size-1]=ps_point[start_size-2];
			}
		}
		if (nan_size && nfiles > nan_size){
			pnan = (int *)
				realloc((char *)pnan, nfiles*sizeof(int));
			while (nan_size++ < nfiles) {
				pnan[nan_size-1]=pnan[nan_size-2];
			}
		}
	}

	if(debug_level) {

		for (i = 0; i < nfiles; i++) {
			Fprintf(stderr, "File: %s, range: %s - %s\n",
		    	in_files[i], r_range[i], s_range[i]);
		}
		Fprintf(stderr, "Jflag: %d\n", Jflag);
		Fprintf(stderr, "Param File: %s", param_file);
		Fprintf(stderr, " Output File: %s\n", ofile);
	}

/* check the input files for various problems
 */
	first_sf = get_genhd_val("record_freq", ih[0], (double)-1);
	first_type = get_fea_type("samples", ih[0]);

	s_point = (long *)calloc((unsigned)nfiles, sizeof(long));
	spsassert(s_point, "mux: calloc failed!");
	e_point = (long *)calloc((unsigned)nfiles, sizeof(long));
	spsassert(s_point, "mux: calloc failed!");
	n_read = (long *)calloc((unsigned)nfiles, sizeof(long));
	spsassert(n_read, "mux: calloc failed!");

	for (i = 0; i < nfiles; i++)
		total_channels += get_chans(ih[i]);

	if (Jflag && total_channels % 2) {
		ERROR_EXIT(
		 "J flag is used and total number of channels is odd.\n");
	}

	start_times = (double *) calloc((unsigned)total_channels, 
		sizeof(double));
	spsassert(start_times, "mux: calloc failed!");
	max_values = (double *) calloc((unsigned)total_channels, 
		sizeof(double));
	spsassert(max_values, "mux: calloc failed!");

	for (i = 0; i < nfiles; i++) {
		if (get_fea_type("samples", ih[i]) == UNDEF) {
			Fprintf(stderr,
		    	 "mux: Input file: %s has no field named samples!\n",
			 in_files[i]);
			exit(1);
		}
		if (Jflag && is_field_complex(ih[i], "samples")) {
			Fprintf(stderr,
			 "mux: J option is used and file %s has a complex field- exiting.\n",
			    in_files[i]);
			exit(1);
		}
		if (get_genhd_val("record_freq",ih[i],(double)0) != first_sf) {
			Fprintf(stderr,
			 "mux: all files must have same sampling freq\n");
			Fprintf(stderr,
			 "mux: freq. in first file is %g, freq., in %s is %g\n",
			    first_sf, in_files[i],
			    get_genhd_val("record_freq", ih[i], (double)0));
			exit(1);
		}
		if (get_fea_type("samples", ih[i]) != first_type) {
			Fprintf(stderr,
			 "mux: all files must have the same data type\n");
			Fprintf(stderr,
			 "mux: type of first file is %s, type of %s is %s.\n",
			    type_codes[first_type], in_files[i],
			    type_codes[get_fea_type("samples",ih[i])]);
			exit(1);
		}

		num_channels = get_chans(ih[i]);

		if (genhd_type("start_time", &genhd_size, ih[i]) != DOUBLE) {
			Fprintf(stderr,
			 "mux: header item \"start_time\" in file %s is undefined or not DOUBLE.\n", 
				in_files[i]);
			exit(1);
		}
		if (genhd_size != num_channels && genhd_size != 1) {
			Fprintf(stderr,
			 "mux: header item \"start_time\" in file %s has wrong size (%d).\n",
				in_files[i], genhd_size);
			exit(1);
		}
		start_time_file = get_genhd_d("start_time", ih[i]);

		s_point[i]=1;
		if(ih[i]->common.ndrec == -1) ih[i]->common.ndrec = LONG_MAX;
		e_point[i] = ih[i]->common.ndrec;	

		if (!range_count) {
			if (start_size) s_point[i] = ps_point[i];
			if (nan_size && pnan[i] != 0) 
				e_point[i] = s_point[i] + pnan[i] - 1;
		}
		else if (r_range[i])
			lrange_switch(r_range[i], s_point+i, e_point+i, 1);
		else { /* Handle s_range here. */ }

		if (e_point[i] > ih[i]->common.ndrec) {
			Fprintf(stderr,
			 "mux: end point %d greater than ndrec for file %s",
			 e_point[i], in_files[i]);
			Fprintf(stderr,", reset to %d\n",ih[i]->common.ndrec);
		        e_point[i] = ih[i]->common.ndrec;
		}

		start_time_offset = (s_point[i] - 1) / first_sf;

		if (genhd_size == 1)
			for (j = 0; j < num_channels; j++)
				start_times[start_time_chan++] = 
					*start_time_file + start_time_offset;
		else
			for (j = 0; j < num_channels; j++)
				start_times[start_time_chan++] =
					start_time_file[j] + start_time_offset;

		switch (genhd_type("max_value", &genhd_size, ih[i])) {
		case HD_UNDEF:
			no_max_value = YES;
			break;
		case DOUBLE:
			if (genhd_size != num_channels && genhd_size != 1)
			{
				Fprintf(stderr,
				 "mux: header item \"max_value\" in file %s has wrong size (%d).\n",
					in_files[i], genhd_size);
				exit(1);
			}
			break;
		default:
			Fprintf(stderr,
			 "mux: header item \"max_value\" in file %s is not DOUBLE.\n",
				in_files[i]);
		}

		if (!no_max_value)
		{
			max_value_file = get_genhd_d("max_value", ih[i]);
			if (genhd_size == 1)
				for (j = 0; j < num_channels; j++)
					max_values[max_value_chan++] =
						*max_value_file;
			else
				for (j = 0; j < num_channels; j++)
					max_values[max_value_chan++] =
						max_value_file[j];
		}

		
/* skip to the first record to be processed */
		if (s_point[i]-1)
			fea_skiprec(in_fd[i], s_point[i]-1, ih[i]);
	}


	if (Jflag) {
		int k=0;
		for(j=0; j<total_channels; j=j+2) {
	   	   if (start_times[j] != start_times[j+1]) {
			Fprintf(stderr,
			 "mux: Warning, start times of channels to be joined inconsistent,\n");
			Fprintf(stderr,
			 "mux: start time for first channel (real part) used.\n");
		   }
		start_times[k++]=start_times[j];
		}
		total_channels /= 2;
	}

	if (debug_level) {
		Fprintf(stderr, "Total Channels: %d\n", total_channels);
		Fprintf(stderr, "First_sf: %g", first_sf);
		Fprintf(stderr, " First_type: %s\n", type_codes[first_type]);
		for(i=0;i<nfiles;i++) {
			Fprintf(stderr,"s_point[%d]: %d, e_point[%d]: %d, ",
		 	i,s_point[i],i,e_point[i]);
		}
		for(i=0;i<total_channels;i++)
			Fprintf(stderr,"start_times[%d]: %g\n",
		 	i,start_times[i]);
	}

	if (!Jflag)
		out_type = first_type;
	else
		switch (first_type) {
		case DOUBLE:
			out_type = DOUBLE_CPLX;
			break;
		case FLOAT:
			out_type = FLOAT_CPLX;
			break;
		case LONG:
			out_type = LONG_CPLX;
			break;
		case SHORT:
			out_type = SHORT_CPLX;
			break;
		case BYTE:
		case CHAR:
			out_type = BYTE_CPLX;
			break;
		}

	start_time_tmp = start_times[0];
	multi_start_time = NO;
	for (i=1; i<total_channels; i++) 
		if (start_time_tmp != start_times[i]) {
			multi_start_time = YES;
			break;
		}

	if(init_feasd_hd(oh, out_type, total_channels, start_times,
			 multi_start_time, first_sf)) {
		ERROR_EXIT("error creating output header");
	}

	if(!no_max_value) 
		(void) add_genhd_d("max_value", max_values, total_channels, oh);
	write_header(oh, out_fd);

/* allocate output and input records */
	
	out_rec = allo_feasd_recs(oh, out_type, 1L, (char *) NULL, NO);
	out_data = out_rec->data;

	in_rec = (struct feasd **) calloc((unsigned) nfiles, 
					  sizeof(struct feasd *));
	if (Jflag)
	{
		in_data = malloc(2*total_channels*typesiz(first_type));
		spsassert(in_data, "mux: malloc failed.");
	}
	else	in_data = out_data;

	prev_num_chans = 0;
	for (i=0; i<nfiles; i++) {
		in_rec[i] = allo_feasd_recs(ih[i], first_type, 1L,
			in_data + typesiz(first_type)*prev_num_chans,
			NO);
		prev_num_chans += get_chans(ih[i]);
	}

/* read-write loop */

	while (!files_done) {
		for (i=0; i<nfiles; i++) {
		    if(get_feasd_recs(in_rec[i], 0L, 1L, ih[i], in_fd[i]) == 0)
			files_done = 1;
		    if(n_read[i]++ > e_point[i]-s_point[i])
			files_done = 1;
		}

		if (!files_done)
		{
		    if (Jflag)
		    {
#define			CASE(TYPE, real_t, cplx_t)			\
			case TYPE:					\
			    {						\
				real_t *in = (real_t *) in_data;	\
				cplx_t *out = (cplx_t *) out_data;	\
									\
				for (j = 0; j < total_channels; j++)	\
				{					\
				    out[j].real = *in++;		\
				    out[j].imag = *in++;		\
				}					\
			    }						\
			    break

			switch (first_type)
			{
			CASE(DOUBLE, double, double_cplx);
			CASE(FLOAT, float, float_cplx);
			CASE(LONG, long, long_cplx);
			CASE(SHORT, short, short_cplx);
			case CHAR:	/* FALL THROUGH */
			CASE(BYTE, char, byte_cplx);
			}

#undef			CASE
		    }

		    if(put_feasd_recs(out_rec,0L,1L,oh,out_fd)!=0) {
			ERROR_EXIT("error writing output file");
		    }

		    output_count++;
		}
	}


/*
 * put output file info in ESPS common
 */
    	if (strcmp(ofile, "<stdout>") != 0) {
        	(void)putsym_s("filename",in_files[0]);
        	(void)putsym_s("prog",ProgName);
        	(void)putsym_i("start",1);
        	(void)putsym_i("nan",(int) output_count);
    	}
    	if (debug_level) Fprintf(stderr, "mux: normal exit\n");

	exit(0);
	/*NOTREACHED*/
}


long
get_chans(hd)
struct header *hd;
{
	return get_fea_siz("samples",hd,(short *)NULL, (long **)NULL);
}
