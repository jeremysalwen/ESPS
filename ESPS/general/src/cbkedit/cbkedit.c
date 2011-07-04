/* cbkedit - allows editing of scaler codebook files
 *
 * This material contains proprietary software of Entropic Speech, Inc.
 * Any reproduction, distribution, or publication without the prior
 * written permission of Entropic Speech, Inc. is strictly prohibited.
 * Any public distribution of copies of this work authorized in writing by
 * Entropic Speech, Inc. must bear the notice
 *
 *    "Copyright (c) 1987 Entropic Speech, Inc.; All rights reserved"
 *
 * Author:  Alan Parker
 * Purpose: edit scaler codebook files
 * Usage:   cbkedit file
 *
 *
 */

#ifndef lint
	static char *sccs_id = "@(#)cbkedit.c	3.6 11/11/94	ESI";
#define VERSION "3.6"
#define DATE "11/11/94"
#endif

#ifndef DATE 
#define DATE "none"
#endif

#ifndef VERSION
#define VERSION "nosccs"
#endif

/* Unix include files */
#include <stdio.h>

/* ESPS include files */
#include <esps/esps.h>
#include <esps/scbk.h>
#include <esps/unix.h>

/* Externals */
extern  getopt(), 
	optind, 
	errno;
extern char *optarg,  
	    *get_cmd_line();

#define BCKSIZ 4096	
int debug_level=0;
char   template[20] = "/tmp/cbXXXXXX";

main (argc, argv)
int     argc;
char  **argv;
{
    int     c;
    char   *editor,
            buf[256],
            buf1[256];
    char   *filename;
    FILE *in, *tstrm, *out;
    struct header  *ih, *oh;
    struct scbk_data   *cbk_rec;
    int     i;
    float   enc[BCKSIZ],
            dec[BCKSIZ],
            final_dist,
            cdwd_dist[BCKSIZ];
    long    final_pop[BCKSIZ];
    unsigned int    code[BCKSIZ];
    float   convergence;
    long    num_items;
    int     distortion,
            codebook_type,
            element_num;
    unsigned int    num_cdwds;


    while ((c = getopt (argc, argv, "x:")) != EOF) {
	switch (c) {
	    case 'x': 
		debug_level = atoi (optarg);
		break;
	}
    }

    if ((argc - optind) != 1) {
	Fprintf (stderr, "cbkedit: needs file name\n");
	exit (1);
    }
    if (debug_level)
	Fprintf (stderr, "cbkedit: file: %s\n", argv[optind]);

    filename = eopen("cbkedit", argv[optind], "r", FT_SCBK, NONE, &ih, &in);
    if(ih->common.ndrec == -1) {
	Fprintf(stderr,"cbkedit: input to this program cannot be a pipe.\n");
	exit(1);
    }

    if ((tstrm = fopen (mktemp (template), "w")) == NULL)
	CANTOPEN ("cbkedit", template);

    if (ih -> common.ndrec != 1)
	Fprintf (stderr, "cbkedit: Warning, ndrec = %d, should be 1.\n",
		ih -> common.ndrec);

    cbk_rec = allo_scbk_rec (ih);

    if (get_scbk_rec (cbk_rec, ih, in) == EOF) {
	Fprintf (stderr, "cbkedit: No data record in file.\n");
	exit (1);
    }

    Fprintf (tstrm, "Header -- Do not delete any lines in the header.\n");
    Fprintf (tstrm, "num_items: %ld\n", ih -> hd.scbk -> num_items);
    Fprintf (tstrm, "distortion: %d\n", ih -> hd.scbk -> distortion);
    Fprintf (tstrm, "num_cdwds: %d (do not edit)\n", ih -> hd.scbk -> num_cdwds);
    Fprintf (tstrm, "convergence: %f\n", ih -> hd.scbk -> convergence);
    Fprintf (tstrm, "codebook_type: %d\n", ih -> hd.scbk -> codebook_type);
    Fprintf (tstrm, "element_num: %d\n", ih -> hd.scbk -> element_num);

    Fprintf (tstrm, "final_dist: %g\n", cbk_rec -> final_dist);
    Fprintf (tstrm, "Data -- you may delete or add lines here.\n");
    Fprintf (tstrm, "/*     enc         dec   code           cdwd_dist   final_pop */\n");
    for (i = 0; i < ih -> hd.scbk -> num_cdwds; i++) {
	Fprintf (tstrm, "%11.5f, %11.5f, 0x%x,    /* %11.5f %9ld */\n",
		cbk_rec -> qtable[i].enc,
		cbk_rec -> qtable[i].dec,
		cbk_rec -> qtable[i].code,
		cbk_rec -> cdwd_dist[i],
		cbk_rec -> final_pop[i]);
    }
    if (fclose (tstrm) == EOF) {
	Fprintf(stderr,"cbkedit: trouble closing temp file\n");
	exit(1);
    }

    if ((editor = getenv ("EDITOR")) == NULL)
	editor = "vi";
    (void)sprintf (buf, "%s %s", editor, template);
    (void)system (buf);

    (void)printf ("cbkedit: Make changes? [return for yes, 'n' for abort] ");
    if (getchar () != 'n') {

	(void)printf ("cbkedit: making changes, ");
	(void)sprintf (buf, "%s.bak", argv[optind]);
	if (rename (argv[optind], buf) != 0) {
	    perror ("cbkedit");
	    exit (1);
	}

	(void)printf ("original %s saved in %s\n", argv[optind], buf);
	if (fclose(in) == EOF) {
	    Fprintf(stderr,"cbkedit: cannot close input file.\n");
	    Fprintf(stderr,"cbkedit: cannot continue.\n");
	    exit(1);
	}
	TRYOPEN ("cbkedit", filename, "w", out);
	TRYOPEN ("cbkedit", template, "r", tstrm);

	oh = copy_header (ih);
	add_source_file (oh, argv[optind], ih);
	add_comment (oh, get_cmd_line (argc, argv));

	if (fgets (buf, 256, tstrm) == NULL)
	    goto trouble;
	if (fgets (buf, 256, tstrm) == NULL)
	    goto trouble;
	if (sscanf (buf, "%s %ld", buf1, &num_items) == 0)
	    goto trouble;
	if (fgets (buf, 256, tstrm) == NULL)
	    goto trouble;
	if (sscanf (buf, "%s %d", buf1, &distortion) == 0)
	    goto trouble;
	if (fgets (buf, 256, tstrm) == NULL)
	    goto trouble;
	if (sscanf (buf, "%s %d", buf1, &num_cdwds) == 0)
	    goto trouble;
	if (fgets (buf, 256, tstrm) == NULL)
	    goto trouble;
	if (sscanf (buf, "%s %f", buf1, &convergence) == 0)
	    goto trouble;
	if (fgets (buf, 256, tstrm) == NULL)
	    goto trouble;
	if (sscanf (buf, "%s %d", buf1, &codebook_type) == 0)
	    goto trouble;
	if (fgets (buf, 256, tstrm) == NULL)
	    goto trouble;
	if (sscanf (buf, "%s %d", buf1, &element_num) == 0)
	    goto trouble;
	if (fgets (buf, 256, tstrm) == NULL)
	    goto trouble;
	if (sscanf (buf, "%s %f", buf1, &final_dist) == 0)
	    goto trouble;

	if (fgets (buf, 256, tstrm) == NULL)
	    goto trouble;
	if (fgets (buf, 256, tstrm) == NULL)
	    goto trouble;
	i = 0;
	while (fgets (buf, 256, tstrm) != NULL) {
	    (void)sscanf (buf, "%f, %f, 0x%x, /* %f %ld */",
		    &enc[i], &dec[i], &code[i], &cdwd_dist[i], &final_pop[i]);
	    i++;
	}
	if (debug_level)
	    Fprintf (stderr, "cbkedit: %d cdwds read back in.\n", i);
	oh -> hd.scbk -> num_cdwds = i;
	cbk_rec = allo_scbk_rec (oh);

	oh -> hd.scbk -> num_items = num_items;
	oh -> hd.scbk -> distortion = distortion;
	oh -> hd.scbk -> convergence = convergence;
	oh -> hd.scbk -> codebook_type = codebook_type;
	oh -> hd.scbk -> element_num = element_num;
	(void)strcpy (oh -> common.prog, "cbkedit");
	(void)strcpy (oh -> common.vers, VERSION);
	(void)strcpy (oh -> common.progdate, DATE);
	write_header (oh, out);


	cbk_rec -> final_dist = final_dist;
	for (i = 0; i < oh -> hd.scbk -> num_cdwds; i++) {
	    cbk_rec -> qtable[i].enc = enc[i];
	    cbk_rec -> qtable[i].dec = dec[i];
	    cbk_rec -> qtable[i].code = code[i];
	    cbk_rec -> cdwd_dist[i] = cdwd_dist[i];
	    cbk_rec -> final_pop[i] = final_pop[i];
	}
	put_scbk_rec (cbk_rec, oh, out);
	if (fclose (out) == EOF) {
	    Fprintf(stderr,"cbkedit: trouble closing output file.\n");
	    Fprintf(stderr,"cannot continue.\n");
	    exit(1);
	}
	(void)fclose (tstrm);	/* I don't care if it fails */
    }
    else {			/* other part of the if abort */
	printf ("cbkedit: aborting without making changes.\n");
    }
    (void)unlink (template);
    return 0;

trouble: 
    Fprintf (stderr, "cbkedit: trouble reading temp file back.\n");
    Fprintf (stderr, "cbkedit: temp file saved in %s\n", template);
    return 1;
}
