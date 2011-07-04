 /*
					
  This material contains proprietary software of Entropic Speech, Inc.   
  Any reproduction, distribution, or publication without the the prior	   
  written permission of Entropic Speech, Inc. is strictly prohibited.
  Any public distribution of copies of this work authorized in writing by
  Entropic Speech, Inc. must bear the notice			
 								
               "Copyright 1986 Entropic Speech, Inc."
 				
 	
  This program converts a LAR codebook file to a RC codebook file.

  Alan Parker, EPI, Washington, DC.

*/
/*
 * System Includes
*/
#include <stdio.h>

/*
 * ESPS Includes
*/
#include <esps/esps.h>
#include <esps/scbk.h>

/*
 * DEfines
*/

#define SYNTAX USAGE ("larcbk2rc [-x debug-level] infile outfile")

/*
 * ESPS Functions
*/
  int getopt();
  char *get_cmd_line();
  float lar_to_rc();
  char *eopen();
/*
 * SCCS stuff
*/
static char *sccs_id = "@(#)larcbk2rc.c	3.2 1/6/93 ESI";
static char *VERSION = "3.2";
static char *DATE = "1/6/93";


extern optind;
extern char *optarg;
int debug_level = 0;

main(argc,argv)
int argc;
char **argv;
{
  int c,i;
  FILE *in=stdin;		/* input file stream ptr*/
  FILE *out=stdout;		/* output file stream ptr*/
  struct header *ih;		/* input file header ptr*/
  struct header *oh;		/* output file header ptr*/
  char *infile="<stdin>";	/*input file name*/
  char *ofile="<stdout>";	/* output file name */
  struct scbk_data *in_rec;	/*codebook data structure*/

  int debug=0;


  while ((c = getopt(argc,argv,"x:")) != EOF) {
    switch(c) {
	  case 'x':
	     debug = atoi(optarg);
		 break;
  	  default:
	     Fprintf(stderr,"larcbk2rc: unknown option.\n");
	     SYNTAX;
    }
  }


  if(argc-optind < 2)
   SYNTAX;

  ofile = eopen("larcbk2rc", argv[argc-1], "w", FT_SCBK, NONE,
		(struct header **)NULL, &out);

  infile = eopen("larcbk2rc", argv[optind], "r", FT_SCBK, NONE,
		&ih, &in);

  if(debug) Fprintf(stderr,"larcbk2rc: infile: %s, outfile: %s\n",
  			infile,ofile);


  if(ih->hd.scbk->codebook_type != LAR_CBK &&
     ih->hd.scbk->codebook_type != LAR_VCD_CBK &&
     ih->hd.scbk->codebook_type != LAR_UNVCD_CBK) {
   Fprintf(stderr,"larcbk2rc: input file not type LAR.\n");
   exit(1);
  }

  oh = copy_header(ih);
  if(ih->hd.scbk->codebook_type == LAR_CBK) 
   oh->hd.scbk->codebook_type = RC_CBK;
  if(ih->hd.scbk->codebook_type == LAR_VCD_CBK) 
   oh->hd.scbk->codebook_type = RC_VCD_CBK;
  if(ih->hd.scbk->codebook_type == LAR_UNVCD_CBK) 
   oh->hd.scbk->codebook_type = RC_UNVCD_CBK;
  add_source_file(oh,infile,ih);
  add_comment(oh,get_cmd_line(argc,argv));
  write_header(oh,out);

  in_rec = allo_scbk_rec(ih);
  if(get_scbk_rec(in_rec,ih,in) == EOF) {
   Fprintf(stderr,"larcbk2rc: error reading record in %s\n",infile);
   exit(1);
  }
  if(debug) Fprintf(stderr,"larcbk2rc: num_cdwds = %d\n",
   ih->hd.scbk->num_cdwds);

  in_rec->final_dist = -1;
  for(i=0; i<ih->hd.scbk->num_cdwds-1;i++) {
   in_rec->cdwd_dist[i] = -1;
   if(debug) Fprintf(stderr,"qtable[%d].enc: %f\n",i,
    in_rec->qtable[i].enc);
   if(debug) Fprintf(stderr,"qtable[%d].dec: %f\n",i,
    in_rec->qtable[i].dec);
   in_rec->qtable[i].enc = lar_to_rc(in_rec->qtable[i].enc);
   in_rec->qtable[i].dec = lar_to_rc(in_rec->qtable[i].dec);
  }
  /* we have to handle the last case as a special case */
  i = ih->hd.scbk->num_cdwds-1;
  if(debug) Fprintf(stderr,"qtable[%d].enc: %f\n",i,
   in_rec->qtable[i].enc);
  if(debug) Fprintf(stderr,"qtable[%d].dec: %f\n",i,
   in_rec->qtable[i].dec);
  in_rec->cdwd_dist[i] = -1;
  in_rec->qtable[i].dec = lar_to_rc(in_rec->qtable[i].dec);

  put_scbk_rec(in_rec,oh,out);
  Fclose(out);
  exit(0);
  return(0);
}



