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
 * Revised by:
 *
 * Brief description:
 *
 */

#include <stdio.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/unix.h>


char	*Version = "1.7";
char	*Date = "8/31/95";

#define ERROR_EXIT(text) {(void) fprintf(stderr, "%s: %s - exiting\n", \
                ProgName, text); exit(1);}

#define Fprintf (void)fprintf

#define SYNTAX \
USAGE("demux [-e grange] [-o prototype] [-{prs} range] [-x debug] [-S]\n [-P param_file] input.fsd [output1.fsd [output2.fsd] ...]")


/*
 * global variables
 */
char	*ProgName = "demux";
int	debug_level = 0;                /* debug level */
long	get_chans();
char	*get_cmd_line();
void	lrange_switch();
long	*grange_switch();
int	symsize();
char	*eopen();
int	symsize();
int	typesiz();
	


main(argc, argv)
int	argc;
char	**argv;

{
	char	*param_file = NULL;  /*parameter file name*/
	FILE    **out_fd;	 	/* output stream */
	char	**ofile;
	struct header **oh;
	extern int	optind;
	extern char	*optarg;
	int	ch;
	int	num_out_files=0;

	char	*r_range=NULL;
	char	*s_range=NULL;
	long	start_point=1, nan=0;
	long	end_point=LONG_MAX;
	struct feasd *in_feasd;
	struct feasd **out_feasd;
	

	FILE * in_fd;
	struct header *ih;
	long	*channels=NULL;
	long	num_channels=0;
	char	*prototype=NULL;
	int	Sflag=0;
	char	*input_file=NULL;
	char	*cptr;
	int	i;
	int	input_type;
	double	*in_start_time;
	int	start_type,out_type;
	int	start_size;
	long	num_channels_file;
	double	input_sf;
	int	nread=0;
	int	done=0;
	int	max_channels;
	int	use_stdin=0;
	


	
	while ((ch = getopt(argc, argv, "p:r:s:x:e:o:SP:")) != EOF)
		switch (ch)  {
		case 'e':
			channels=grange_switch(optarg,&num_channels);
			break;
		case 'o':
			prototype = optarg;
			break;
		case 'r':
		case 'p':
			r_range = optarg;
			break;
		case 's':
			s_range = optarg;
			break;
		case 'x':
			debug_level = atoi(optarg);
			break;
		case 'S':
			Sflag=1;
			break;
		case 'P':
			param_file = optarg;
			break;
		default:
			SYNTAX;
		}

			
		
	if (s_range) {
		ERROR_EXIT("s option not implemented in this version");
	}

	if (argc - optind < 1) {
                Fprintf(stderr, "demux: no files specified.\n");
                SYNTAX;
        }

	input_file = eopen(ProgName,argv[optind++],"r",FT_FEA,FEA_SD,
		&ih,&in_fd);

	if ((input_type = get_fea_type("samples", ih)) == UNDEF){
		ERROR_EXIT("Input file has no field named samples!");
	}

	input_sf = get_genhd_val("record_freq",ih,(double)1);

	if(debug_level)
		Fprintf(stderr,"record_freq: %g\n",input_sf);


	(void)read_params(param_file,SC_CHECK_FILE,input_file);

	if(!Sflag && (symtype("make_real") != ST_UNDEF)) {
		cptr = getsym_s("make_real");
		if(strcmp(cptr,"yes") || strcmp(cptr,"YES"))
			Sflag=1;
	}

	if(Sflag && !is_field_complex(ih,"samples")){
		ERROR_EXIT("S option used, but input is not complex");
	}
	
	if (!channels && (symtype("channels") != ST_UNDEF)) {
		num_channels = symsize("channels");
		channels = (long *)calloc((unsigned)num_channels,sizeof(long));
		spsassert(channels,"calloc failed");
		(void)getsym_ia("channels",(int *)channels,symsize("channels"));
	}

	if(!prototype && (symtype("prototype") != ST_UNDEF)) 
		prototype = getsym_s("prototype");


	if (ih->common.ndrec == -1) ih->common.ndrec = LONG_MAX;
	end_point = ih->common.ndrec;
	if(r_range)
		lrange_switch(r_range,&start_point,&end_point,1);
	else {
		if(symtype("start") != ST_UNDEF)
			start_point = getsym_i("start");

		if (symtype("nan") != ST_UNDEF) {
			nan = getsym_i("nan");
			if(nan == 0) nan=end_point;
			end_point=start_point+nan-1;
		}
	}

	if(start_point>end_point)
		ERROR_EXIT("demux: start point > end_point");

	if(end_point > ih->common.ndrec) {
		Fprintf(stderr,"demux: end_point > number of records.\n");
		Fprintf(stderr,"demux: end_point reset to %d.\n",
			ih->common.ndrec);
		end_point = ih->common.ndrec;
	}

	if (!channels) {
		num_channels = get_chans(ih);
		if(Sflag) num_channels *= 2;
		channels = (long *)calloc((unsigned)num_channels,sizeof(long));
		spsassert(channels,"calloc failed");
		for(i=0;i<num_channels;i++)
			channels[i]=i;
	}

	max_channels = get_chans(ih);
	if(Sflag) max_channels *= 2;
	for(i=0;i<num_channels; i++)	
		if(channels[i]<0 || channels[i]>max_channels-1){
			ERROR_EXIT("Channel number out of bounds.");
		}

	
	if((optind == argc) && !prototype) {
	    ERROR_EXIT("Either an output file or a prototype must be given.");
	}
	
	ofile = (char **)calloc((unsigned)num_channels,sizeof(char *));
	spsassert(ofile,"calloc failed");
	i=0;
	while(optind < argc) {
		ofile[i]=argv[optind++];
		if(strcmp(ofile[i++],"-")==0) use_stdin++;
	}
	if(use_stdin>1)
		ERROR_EXIT("Only one output file can be standard output.");
	num_out_files=i;

	if(num_out_files == 0) {
		for(i=0;i<num_channels;i++) {
			ofile[i]=malloc((unsigned)strlen(prototype)+10);
			spsassert(ofile[i],"calloc failed");
			(void)sprintf(ofile[i],"%s%03d",prototype,channels[i]);
		}
		num_out_files = num_channels;
	}


	if(num_out_files != 1 && num_out_files != num_channels) {
		ERROR_EXIT(
              "Number of output files must be 1 or number of output channels");
	}

	if(num_out_files == 1)
		num_channels_file = num_channels;
	else
		num_channels_file = 1;

	start_type = genhd_type("start_time",&start_size,ih);
	if(start_type != DOUBLE) {
		start_size=0;
		Fprintf(stderr,
		 "demux: Warning, start_time in input file not double.\n");
		Fprintf(stderr,
		 "demux: start_time in input file being ignored.\n");
	}
	if(start_size == 0) {
		in_start_time = (double *)calloc((unsigned)num_channels,sizeof (double));
		for(i=0;i<num_channels;i++)
			in_start_time[i]=0;
	} else if (start_size != get_chans(ih) && start_size != 1) {
		Fprintf(stderr,
		 "demux: input start size array is wrong size, just using the first value\n");
		start_size = 1;
	
	} else if (start_size == 1) {
		double tmp = get_genhd_val("start_time",ih,(double)0);
		in_start_time = (double *)calloc((unsigned)num_channels,sizeof (double));
		for(i=0;i<num_channels;i++)
			in_start_time[i]=tmp;
	} else if(start_size == get_chans(ih)) {
		double *ptr=get_genhd_d("start_time",ih);
		in_start_time = (double *)calloc((unsigned)num_channels,sizeof (double));
		spsassert(ptr,"get_genhd_d failed");
		for(i=0;i<num_channels;i++)
			in_start_time[i] = ptr[channels[i]];
	} 
	if(debug_level) {
		Fprintf(stderr,"Input start_time: ");
		for(i=0; i<num_channels;i++)
			Fprintf(stderr,"%g ",in_start_time[i]);
		Fprintf(stderr,"\n");
	}

	for(i=0;i<num_channels;i++) 
		in_start_time[i] = in_start_time[i]+(start_point-1)/
			input_sf;

	if(debug_level) {
		Fprintf(stderr,"Output start_time: ");
		for(i=0; i<num_channels;i++)
			Fprintf(stderr,"%g ",in_start_time[i]);
		Fprintf(stderr,"\n");
	}
		

	if(!Sflag) {
		out_type = input_type;
	}else {
		switch (input_type){
		case DOUBLE_CPLX:
			out_type=DOUBLE;
			break;
		case FLOAT_CPLX:
			out_type=FLOAT;
			break;
		case LONG_CPLX:
			out_type=LONG;
			break;
		case SHORT_CPLX:
			out_type=SHORT;
			break;
		case BYTE_CPLX:
			out_type=BYTE;
			break;
		default:
			fprintf(stderr,
			 "demux: invalid input data type with -S option.\n");
			exit(1);
		}
	}

	if(debug_level) {
		Fprintf(stderr,"Input File: %s\n",input_file);
		Fprintf(stderr,"Number of channels: %d\n  ",num_channels);
		for(i=0;i<num_channels;i++)
			Fprintf(stderr,"%d ",channels[i]);
		Fprintf(stderr,"\n");
		Fprintf(stderr,"Prototype: %s\n",prototype);
		Fprintf(stderr,"Range: %s\n",r_range);
		Fprintf(stderr,"Sflag: %d\n",Sflag);
		Fprintf(stderr,"param_file: %s\n",param_file);
		Fprintf(stderr,"start_point: %d, end_point: %d\n",
			start_point, end_point);
		Fprintf(stderr,"Out Files: ");
		for(i=0;i<num_out_files;i++)
			Fprintf(stderr,"%s ",ofile[i]);
		Fprintf(stderr,"\n");
	}


	oh = (struct header **)calloc((unsigned)num_channels,sizeof(struct header *));
	out_fd = (FILE **)calloc((unsigned)num_channels,sizeof(FILE *));
	for(i=0;i<num_out_files;i++) {
		ofile[i] = eopen(ProgName,ofile[i],"w",FT_FEA,FEA_SD,
			&oh[i], &out_fd[i]);
		oh[i] = new_header(FT_FEA);	
		(void)copy_genhd(oh[i],ih,NULL);
		if(init_feasd_hd(oh[i],out_type,(long)num_channels_file,
			in_start_time,YES,input_sf)){
			ERROR_EXIT("error creating output header");
		}
		add_source_file(oh[i],input_file,ih);
		add_comment (oh[i], get_cmd_line(argc, argv));
		(void) strcpy (oh[i] ->common.prog, ProgName);
		(void) strcpy (oh[i] ->common.vers, Version);
		(void) strcpy (oh[i] ->common.progdate, Date);
		write_header(oh[i],out_fd[i]);
	}

	fea_skiprec(in_fd,start_point-1,ih);
	in_feasd = allo_feasd_recs(ih,input_type,(long)BUFSIZ,NULL,YES);
	out_feasd = (struct feasd **)calloc((unsigned)num_out_files,
			sizeof(struct feasd *));

	if(num_out_files == 1) {
		long to_read, total_recs;
	        long total_read = 0;
		short **s_ptr_i = (short **)in_feasd->ptrs;
		long **l_ptr_i = (long **)in_feasd->ptrs;
		float **f_ptr_i = (float **)in_feasd->ptrs;
		double **d_ptr_i = (double **)in_feasd->ptrs;
		char **c_ptr_i = (char **)in_feasd->ptrs;

		long_cplx **lc_ptr_i = (long_cplx **)in_feasd->ptrs;
		float_cplx **fc_ptr_i = (float_cplx **)in_feasd->ptrs;
		double_cplx **dc_ptr_i = (double_cplx **)in_feasd->ptrs;
		short_cplx **sc_ptr_i = (short_cplx **)in_feasd->ptrs;
		byte_cplx **bc_ptr_i = (byte_cplx **)in_feasd->ptrs;

		short **s_ptr_o;  short_cplx **sc_ptr_o;
		long **l_ptr_o;	  long_cplx **lc_ptr_o;
		float **f_ptr_o;  float_cplx **fc_ptr_o;
		double **d_ptr_o; double_cplx **dc_ptr_o;
		char **c_ptr_o;	  byte_cplx **bc_ptr_o;

		int k=typesiz(out_type);
		*out_feasd = allo_feasd_recs(oh[0],out_type,(long)BUFSIZ,
			NULL,YES);
		s_ptr_o = (short **)out_feasd[0]->ptrs;
		l_ptr_o = (long **)out_feasd[0]->ptrs;
		f_ptr_o = (float **)out_feasd[0]->ptrs;
		d_ptr_o = (double **)out_feasd[0]->ptrs;
		c_ptr_o = (char **)out_feasd[0]->ptrs;
		sc_ptr_o = (short_cplx **)out_feasd[0]->ptrs;
		lc_ptr_o = (long_cplx **)out_feasd[0]->ptrs;
		fc_ptr_o = (float_cplx **)out_feasd[0]->ptrs;
		dc_ptr_o = (double_cplx **)out_feasd[0]->ptrs;
		bc_ptr_o = (byte_cplx **)out_feasd[0]->ptrs;


		total_recs = end_point-start_point+1;
		if (total_recs < BUFSIZ) 
			to_read = total_recs; 
		else
			to_read = BUFSIZ;

		while(nread=get_feasd_recs(in_feasd,0L,to_read, ih,in_fd)){
			total_read += nread;
			if (total_read + to_read > total_recs) {
				to_read = total_recs-total_read;
			}

			for (i=0; i<nread; i++) {
			  int j;
			  switch (out_type) {
			    case DOUBLE:
			      for(j=0; j<num_channels; j++) 
			        d_ptr_o[i][j] = d_ptr_i[i][channels[j]];
			      break;
			    case FLOAT:
			      for(j=0; j<num_channels; j++) 
			        f_ptr_o[i][j] = f_ptr_i[i][channels[j]];
			      break;
			    case LONG:
			      for(j=0; j<num_channels; j++) 
			        l_ptr_o[i][j] = l_ptr_i[i][channels[j]];
			      break;
			    case SHORT:
			      for(j=0; j<num_channels; j++) 
			        s_ptr_o[i][j] = s_ptr_i[i][channels[j]];
			      break;
			    case BYTE:
			      for(j=0; j<num_channels; j++) 
			        c_ptr_o[i][j] = c_ptr_i[i][channels[j]];
			      break;
			    case DOUBLE_CPLX:
			      for(j=0; j<num_channels; j++) 
			        dc_ptr_o[i][j] = dc_ptr_i[i][channels[j]];
			      break;
			    case FLOAT_CPLX:
			      for(j=0; j<num_channels; j++) 
			        fc_ptr_o[i][j] = fc_ptr_i[i][channels[j]];
			      break;
			    case LONG_CPLX:
			      for(j=0; j<num_channels; j++) 
			        lc_ptr_o[i][j] = lc_ptr_i[i][channels[j]];
			      break;
			    case SHORT_CPLX:
			      for(j=0; j<num_channels; j++) 
			        sc_ptr_o[i][j] = sc_ptr_i[i][channels[j]];
			      break;
			    case BYTE_CPLX:
			      for(j=0; j<num_channels; j++) 
			        bc_ptr_o[i][j] = bc_ptr_i[i][channels[j]];
			      break;
			    default:
			      spsassert(0,"unkown type in switch");
			  }
			}
		        if(put_feasd_recs(out_feasd[0],0L,nread,
			     oh[0], out_fd[0]))
					ERROR_EXIT("error writing out file");
		if (to_read == 0) break;
		}
	}
	else if(num_out_files == num_channels){
		long to_read, total_recs;
		int k=typesiz(out_type);

		short **s_ptr = (short **)in_feasd->ptrs, *s_data;
		long **l_ptr = (long **)in_feasd->ptrs, *l_data;
		float **f_ptr = (float **)in_feasd->ptrs, *f_data;
		double **d_ptr = (double **)in_feasd->ptrs, *d_data;
		char **c_ptr = (char **)in_feasd->ptrs, *c_data;
		short_cplx **sc_ptr = (short_cplx **)in_feasd->ptrs, *sc_data;
		long_cplx **lc_ptr = (long_cplx **)in_feasd->ptrs, *lc_data;
		float_cplx **fc_ptr = (float_cplx **)in_feasd->ptrs, *fc_data;
		double_cplx **dc_ptr = (double_cplx **)in_feasd->ptrs, *dc_data;
		byte_cplx **bc_ptr = (byte_cplx **)in_feasd->ptrs, *bc_data;

	        long total_read = 0;
		for(i=0;i<num_out_files;i++)
			out_feasd[i]=allo_feasd_recs(oh[i],out_type,
				(long)BUFSIZ,NULL,NO);

		total_recs = end_point-start_point+1;
		if (total_recs < BUFSIZ) 
			to_read = total_recs; 
		else
			to_read = BUFSIZ;

		while(nread=get_feasd_recs(in_feasd,0L,to_read, ih,in_fd)){
			total_read += nread;
			if (total_read + to_read > total_recs) {
				to_read = total_recs-total_read;
			}

			for (i=0; i<num_out_files; i++) {
			  int j;
			  switch (out_type) {
			    case DOUBLE:
			      d_data   = (double *)out_feasd[i]->data;
			      for(j=0; j<nread; j++) 
			        d_data[j] = d_ptr[j][channels[i]];
			      break;
			    case FLOAT:
			      f_data   = (float *)out_feasd[i]->data;
			      for(j=0; j<nread; j++) 
			        f_data[j] = f_ptr[j][channels[i]];
			      break;
			    case LONG:
			      l_data   = (long *)out_feasd[i]->data;
			      for(j=0; j<nread; j++) 
			        l_data[j] = l_ptr[j][channels[i]];
			      break;
			    case SHORT:
			      s_data   = (short *)out_feasd[i]->data;
			      for(j=0; j<nread; j++) 
			        s_data[j] = s_ptr[j][channels[i]];
			      break;
			    case BYTE:
			      c_data   = (char *)out_feasd[i]->data;
			      for(j=0; j<nread; j++) 
			        c_data[j] = c_ptr[j][channels[i]];
			      break;
			    case DOUBLE_CPLX:
			      dc_data   = (double_cplx *)out_feasd[i]->data;
			      for(j=0; j<nread; j++) 
			        dc_data[j] = dc_ptr[j][channels[i]];
			      break;
			    case FLOAT_CPLX:
			      fc_data   = (float_cplx *)out_feasd[i]->data;
			      for(j=0; j<nread; j++) 
			        fc_data[j] = fc_ptr[j][channels[i]];
			      break;
			    case LONG_CPLX:
			      lc_data   = (long_cplx *)out_feasd[i]->data;
			      for(j=0; j<nread; j++) 
			        lc_data[j] = lc_ptr[j][channels[i]];
			      break;
			    case SHORT_CPLX:
			      sc_data   = (short_cplx *)out_feasd[i]->data;
			      for(j=0; j<nread; j++) 
			        sc_data[j] = sc_ptr[j][channels[i]];
			      break;
			    case BYTE_CPLX:
			      bc_data   = (byte_cplx *)out_feasd[i]->data;
			      for(j=0; j<nread; j++) 
			        bc_data[j] = bc_ptr[j][channels[i]];
			      break;
			    default:
			      spsassert(0,"unkown type in switch");
			  }

			  if(put_feasd_recs(out_feasd[i],0L,nread,
			     oh[i], out_fd[i]))
					ERROR_EXIT("error writing out file");
			}
		if (to_read == 0) break;
		}
	}
				


	return 0;
		
}

long
get_chans(hd)
struct header *hd;
{
	return get_fea_siz("samples",hd,(short *)NULL, (long **)NULL);
}

