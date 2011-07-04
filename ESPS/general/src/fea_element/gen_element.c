/* gen_elem - print record structure information of an ESPS file 
 *
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 * 
 *  Module Name:  gen_element
 *
 *  Written By:  Ajaipal S. Virdy
 * 	
 *  Checked By:  
 *
 *  Purpose:  Give information about elements in a record of any ESPS file.
 *
 *  Secrets:  None
 *  
 */

#ifndef lint
static char *sccs_id = "@(#)gen_element.c	3.5 3/14/94 ESI";
#endif

#include <stdio.h>
#include <esps/esps.h>
#include <esps/unix.h>
#include <esps/sd.h>
#include <esps/spec.h>
#include <esps/filt.h>
#include <esps/scbk.h>
#include <esps/fea.h>

#ifdef ESI
#include <sps/ana.h>
#include <sps/pitch.h>
#include <sps/ros.h>
#endif 

#define SYNTAX USAGE("gen_elem [-x debug_level] [ESPS file]")

#define	FEA_ELE "fea_element"

extern	int	optind;
extern	char	*optarg;



char	*get_cmd_line();

char	*ProgName = "gen_elem";
int	debug_level = 0;

main (argc, argv)
int	argc;
char	**argv;
{
	int	c;

	struct header		*sps_hdr;

	char	*gen_file;

	FILE	*istrm = stdin;


	/*
	 * miscellaneous variables used in this program
	 *
	 */

	char	*cmd_options;
	char	SPS_command[100];

	int	i;
	int	comlen;

	/* parse command line options */

	while ((c = getopt (argc, argv, "x:cf:")) != EOF)
	    switch (c) {
		case 'x':
		   debug_level = atoi (optarg);
		   break;
		case 'c':
		case 'f':
		   break;	/* ignore these options */
		default:
		   SYNTAX;
	    }

	if (optind < argc) {
	   gen_file = argv[optind++];
	   if (strcmp (gen_file, "-") == 0)
		gen_file = "<stdin>";
	   else
	  	TRYOPEN (argv[0], gen_file, "r", istrm);
	}
	else
	{
	   (void) fprintf (stderr,
	   "%s: no input file specified.\n", ProgName);
	   SYNTAX;
	}

	if ((sps_hdr = read_header (istrm)) == NULL)
	   NOTSPS (ProgName, gen_file);

	if (sps_hdr->common.type == FT_FEA) {

	   /*
	    * Strip off the name of this program from the command line
	    * and send the rest of the command line to fea_element(1-ESPS).
	    *
	    */

	   cmd_options = get_cmd_line(argc, argv);
	   comlen =  strlen(argv[0]);
	   for (i = 0; i < comlen; i++)
		cmd_options++;

	   if (debug_level > 0)
		(void) fprintf (stderr,
		"%s: cmd_options = %s\n", ProgName, cmd_options);

	   (void) sprintf (SPS_command, "%s %s", FEA_ELE, cmd_options);
	   exit ( system (SPS_command) );
	}

	(void) fprintf (stdout,
	"Record Item\t\tType\t\tStart Element\t\tSize\n");
	(void) fprintf (stdout,
	"-----------\t\t----\t\t-------------\t\t----\n");

	if ((sps_hdr->common.tag == YES))
	   (void) fprintf (stdout,
	   "Tag        \t\tLONG\t\t0            \t\t1\n");

	(void) fflush (stdout);

	switch (sps_hdr->common.type) {

	   case FT_SD: {
		char	data_type[10];

		if (sps_hdr->common.ndouble)
		   (void)strcat (data_type, "DOUBLE");
		else
		if (sps_hdr->common.nfloat)
		   (void)strcat (data_type, "FLOAT");
		else
		if (sps_hdr->common.nlong)
		   (void)strcat (data_type, "LONG");
		else
		if (sps_hdr->common.nshort)
		   (void)strcat (data_type, "SHORT");
		else
		if (sps_hdr->common.nchar)
		   (void)strcat (data_type, "CHAR");

		(void) fprintf (stdout,
		"Sampled Data\t\t%s\t\t1             \t\t1\n",
		data_type);
	   }
	   break;

#ifdef ESI
	   case FT_ANA: {
		struct ana_header *ana_hdr = sps_hdr->hd.ana;

		short maxpulses = ana_hdr->maxpulses;
		short maxraw = ana_hdr->maxraw;
		short maxlpc = ana_hdr->maxlpc;
		short order = MAX (ana_hdr->order_vcd, ana_hdr->order_unvcd);

		(void) fprintf (stdout,
		"p_pulse_len\t\tFLOAT\t\t1           \t\t%d\n",
		maxpulses);
		(void) fprintf (stdout,
		"raw_power  \t\tFLOAT\t\t%d          \t\t%d\n",
		maxpulses + 1, maxraw);
		(void) fprintf (stdout,
		"lpc_power  \t\tFLOAT\t\t%d          \t\t%d\n",
		maxpulses + maxraw + 1, maxlpc);
		(void) fprintf (stdout,
		"ref_coeff  \t\tFLOAT\t\t%d          \t\t%d\n",
		maxpulses + maxraw + maxlpc + 1, order);
		(void) fprintf (stdout,
		"frame_len  \t\tSHORT\t\t%d          \t\t%d\n",
		maxpulses + maxraw + maxlpc + order + 1, 1);
	   }
	   break;
#endif

#ifdef ESI
	   case FT_PIT:
		(void) fprintf (stdout,
		"pulse_dist \t\tFLOAT\t\t1           \t\t1\n");
		(void) fprintf (stdout,
		"raw_pulse_dist\t\tFLOAT\t\t2        \t\t1\n");
	   break;
#endif

	   case FT_SPEC: {
		struct spec_header *spec_hdr = sps_hdr->hd.spec;

		long num_freqs = spec_hdr->num_freqs;

		(void) fprintf (stdout,
		"re_spec_val\t\tFLOAT\t\t1           \t\t%d\n",
		num_freqs);

		if (spec_hdr->spec_type == ST_CPLX)
		   (void) fprintf (stdout,
		   "im_spec_val\t\tFLOAT\t\t%d          \t\t%d\n",
		   num_freqs + 1, num_freqs);

		if (spec_hdr->freq_format == ARB_VAR) {
		   (void) fprintf (stdout,
		   "frqs       \t\tFLOAT\t\t%d          \t\t%d\n",
		   2 * num_freqs + 1, num_freqs);
		   (void) fprintf (stdout,
		   "n_frqs     \t\tLONG\t\t%d           \t\t%d\n",
		   3 * num_freqs + 1, 1);
		}

		if (spec_hdr->frame_meth != FM_NONE &&
		    spec_hdr->frame_meth != FM_FIXED)
		   (void) fprintf (stdout,
		   "frame_len  \t\tSHORT\t\t%d          \t\t%d\n",
		   3 * num_freqs + 2, 1);

		if (spec_hdr->voicing == YES)
		   (void) fprintf (stdout,
		   "voiced     \t\tSHORT\t\t%d          \t\t%d\n",
		   3 * num_freqs + 3, 1);

	   }
	   break;

#ifdef ESI
	   case FT_ROS: {
		struct ros_header *ros_hdr = sps_hdr->hd.ros;

		short maxtype = ros_hdr->maxtype;
		short order = MAX (ros_hdr->order_vcd, ros_hdr->order_unvcd);
		short maxpulses = ros_hdr->maxpulses;
		short maxpow = ros_hdr->maxpow;

		(void) fprintf (stdout,
		"frame_len\t\tLONG\t\t1              \t\t1\n");

		(void) fprintf (stdout,
		"frmlen_pos\t\tSHORT\t\t2            \t\t1\n");

		(void) fprintf (stdout,
		"type       \t\tSHORT\t\t3           \t\t%d\n",
		maxtype);

		(void) fprintf (stdout,
		"ref_coeff  \t\tSHORT\t\t%d          \t\t%d\n",
		maxtype + 3, order);

		(void) fprintf (stdout,
		"p_pulse_len\t\tSHORT\t\t%d          \t\t%d\n",
		maxtype + order + 3, maxpulses);

		(void) fprintf (stdout,
		"power      \t\tSHORT\t\t%d          \t\t%d\n",
		maxtype + order + maxpulses + 3, maxpow);

		(void) fprintf (stdout,
		"type_pos   \t\tSHORT\t\t%d          \t\t%d\n",
		maxtype + order + maxpulses + maxpow + 3, maxtype);

		(void) fprintf (stdout,
		"rc_pos     \t\tSHORT\t\t%d          \t\t%d\n",
		2 * maxtype + order + maxpulses + maxpow + 3, order);

		(void) fprintf (stdout,
		"pulse_pos  \t\tSHORT\t\t%d          \t\t%d\n",
		2 * (maxtype + order) + maxpulses + maxpow + 3, maxpulses);

		(void) fprintf (stdout,
		"pow_pos    \t\tSHORT\t\t%d          \t\t%d\n",
		2 * (maxtype + order + maxpulses) + maxpow + 3, maxpow);

	   }
	   break;
#endif

	   case FT_FILT: {
		struct filt_header *filt_hdr = sps_hdr->hd.filt;

		short max_num = filt_hdr->max_num;
		short max_den = filt_hdr->max_den;

		(void) fprintf (stdout,
		"filt_func->zeros\tFLOAT\t\t1          \t\t%d\n",
		max_num);

		(void) fprintf (stdout,
		"filt_func->poles\tFLOAT\t\t%d          \t\t%d\n",
		max_num + 1, max_den);

		(void) fprintf (stdout,
		"filt_func->nsiz\t\tSHORT\t\t%d          \t\t%d\n",
		max_num + max_den + 1, 1);

		(void) fprintf (stdout,
		"filt_func->dsiz\t\tSHORT\t\t%d          \t\t%d\n",
		max_num + max_den + 2, 1);

		(void) fprintf (stdout,
		"spares     \t\tSHORT\t\t%d          \t\t%d\n",
		max_num + max_den + 3, FDSPARES);

	   }
	   break;

	   case FT_SCBK: {
		struct scbk_header *scbk_hdr = sps_hdr->hd.scbk;

		unsigned short num_cdwds = scbk_hdr->num_cdwds;

		(void) fprintf (stdout,
		"final_dist \t\tFLOAT\t\t1           \t\t1\n");

		(void) fprintf (stdout,
		"cdwd_dist  \t\tFLOAT\t\t2           \t\t%d\n",
		num_cdwds);

		(void) fprintf (stdout,
		"qtable->enc\t\tFLOAT\t\t%d           \t\t%d\n",
		num_cdwds + 2, num_cdwds);

		(void) fprintf (stdout,
		"qtable->dec\t\tFLOAT\t\t%d           \t\t%d\n",
		2 * num_cdwds + 2, num_cdwds);

		(void) fprintf (stdout,
		"final_pop \t\tFLOAT\t\t%d           \t\t%d\n",
		3 * num_cdwds + 2, num_cdwds);

		(void) fprintf (stdout,
		"qtable->code\t\tFLOAT\t\t%d           \t\t%d\n",
		4 * num_cdwds + 2, num_cdwds);

	   }
	   break;

	   default:
		(void) fprintf (stderr,
		"%s: Unknown file type.\n", ProgName);
		exit (1);

	}  /* end switch (sps_hdr->common.type) */
	return 0;

}	
