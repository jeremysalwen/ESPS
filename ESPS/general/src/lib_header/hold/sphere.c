/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1996 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Alan Parker
 * Checked by:
 * Revised by:
 *
 * Brief description: Routine to convert NIST Sphere header to FEASD
 *
 */

static char *sccs_id = "@(#)sphere.c	1.15	28 Oct 1999	ERL";

#include <stdio.h>

#include <sp/sphere.h>
#undef BYTE
#undef SHORT
#undef ROUND
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>

#define ESPS_BYTE  8
#define ESPS_SHORT  4

extern int      errno;
extern int      EspsDebug;	/* set in headers.c for header debugging */
extern int      debug_level;	/* standard debug_level */

SP_FILE        *sp_fp_open();
static long     findlim();
static int      copy_gens();

int             sphere_to_sd_ctrl = 0;	/* shared with headers.c */

struct header  *
sphere_to_feasd(fp)
   FILE           *fp;

{
   char            line[10];
   char           *error;
   struct header_t *t_h;
   SP_FILE        *tsp;
   char          **fields;
   struct header  *e_h;
   int             size, type, nfields;
   long            channel_count;
   long            sample_min = 0, sample_max = 0;
   long            n, i;
   double          start_time = 0;
   double          record_freq;
   char           *p, *q;
   char           *name;
   char            byte_order[3];
   char            sample_coding[4];
   time_t          tloc, time();
   char           *ctime();
   char            I[4];
   int             data_type;
   int             samp_size = 0;
   extern int      called_by_waves;

   if (EspsDebug)
      fprintf(stderr, "Inside of sphere_to_feasd()\n");

   spsassert(fp, "sphere_to_fea: fp is NULL");
   called_by_waves = 1;
   strcpy(byte_order, "10");

   /* check for SPHERE magic numbers */
   (void) fgets(line, 8, fp);
   if (strcmp(line, "NIST_1A") != 0)
      return NULL;
   else
      fseek(fp, 0L, 0);

   /* open Sphere header */
#ifdef OLD_WAY
   if ((t_h = sp_open_header(fp, TRUE, &error)) == NULL) {
      if (EspsDebug)
	 fprintf(stderr, "%s\n", error);
      return NULL;
   }
#else
   if ((tsp = sp_fp_open(fp)) == SPNULL) {
      if (EspsDebug)
	 fprintf(stderr, "sp_fp_open failed\n");
      return NULL;
   }
   t_h = tsp->read_spifr->header;
#endif
   if (EspsDebug)
      fprintf(stderr, "Read sphere header\n");

   nfields = sp_get_nfields(t_h);
   if (nfields < 1)
      return NULL;

   if (EspsDebug)
      fprintf(stderr, "nfields: %d\n", nfields);

   fields = (char **) malloc((unsigned) (nfields * sizeof(char *)));
   spsassert(fields, "malloc failed");

   if (sp_get_fieldnames(t_h, nfields, fields) != nfields) {
      fprintf(stderr, "sphere_to_fea: internal error.\n");
      assert(0);
   }

   n = sp_get_field(t_h, "channel_count", &type, &size);
   if (n < 0)
      channel_count = 1;	/* if no channel count default to 1 */
   else {
      n = sp_get_data(t_h, "channel_count", (char *) &channel_count, &size);
      spsassert(n >= 0, "sp_get_data returned error");
   }
   if (EspsDebug)
      fprintf(stderr, "channels: %d\n", channel_count);
   n = sp_get_field(t_h, "sample_byte_format", &type, &size);
   if (n >= 0) {
      if (size > 2) {
	 size = 2;
      }
      n = sp_get_data(t_h, "sample_byte_format", byte_order, &size);
      spsassert(n >= 0, "sp_get_data returned error");
      byte_order[size] = '\0';
   }
   n = sp_get_field(t_h, "sample_n_bytes", &type, &size);
   if (n >= 0) {
      n = sp_get_data(t_h, "sample_n_bytes", (char *) &samp_size, &size);
      spsassert(n >= 0, "sp_get_data returned error");
   }
   n = sp_get_field(t_h, "sample_min", &type, &size);
   if (n >= 0) {
      n = sp_get_data(t_h, "sample_min", (char *) &sample_min, &size);
      spsassert(n >= 0, "sp_get_data returned error");
   }
   n = sp_get_field(t_h, "sample_max", &type, &size);
   if (n >= 0) {
      n = sp_get_data(t_h, "sample_max", (char *) &sample_max, &size);
      spsassert(n >= 0, "sp_get_data returned error");
   }
   n = sp_get_field(t_h, "sample_coding", &type, &size);
   if (n >= 0) {
      n = sp_get_data(t_h, "sample_coding", sample_coding, &size);
      spsassert(n >= 0, "sp_get_data returned error");
   }
   n = sp_get_field(t_h, "sample_rate", &type, &size);
   if (n < 0) {
      record_freq = 10000;
      fprintf(stderr,
	      "Warning: converting from timit to esps and no sample rate is in header.\n");
      fprintf(stderr,
	      "         Using 10000 Hz.\n");
   } else {
      switch (type) {
      case T_INTEGER:
	 {
	    long           p;
	    n = sp_get_data(t_h, "sample_rate", (char *)&p, &size);
	    spsassert(n >= 0, "sp_get_data returned error");
	    record_freq = p;
	    break;
	 }
      case T_REAL:
	 {
	    double         p;
	    n = sp_get_data(t_h, "sample_rate", (char *)&p, &size);
	    spsassert(n >= 0, "sp_get_data returned error");
	    record_freq = p;
	    break;
	 }
      default:
	 assert(0);
      }


   }


   e_h = new_header(FT_FEA);
   if (byte_order[0] == '0' && byte_order[1] == '1') {
      e_h->common.machine_code = DS3100_CODE;
      e_h->common.edr = NO;
      if (EspsDebug)
	 fprintf(stderr, "ds3100 byte order\n");
   } else if (byte_order[0] == '1' && byte_order[1] == '0') {
      e_h->common.machine_code = SUN4_CODE;
      e_h->common.edr = YES;
      if (EspsDebug)
	 fprintf(stderr, "sun4 byte order\n");
   }
   data_type = ESPS_SHORT;

   start_time = 0;
   n = init_feasd_hd(e_h, data_type, channel_count, &start_time, NO, record_freq);
   spsassert(n == 0, "init_feasd_hd returned error");

   if (sample_min || sample_max)
      *add_genhd_d("max_value", NULL, 1, e_h) =
	 MAX(abs(sample_min), sample_max);

   /* set the date */
   tloc = time(0);
   (void) strcpy(e_h->common.date, ctime(&tloc));
   e_h->common.date[24] = ' ';

   /* add header version */


   I[0] = '\0';			/* dirty trick to keep sccs from */
   (void) strcpy(I, "%");	/* killing the % I % , symbol */
   (void) strcat(I, "I%");
   /*
    * if the header version define (from header.h) is equal to
    * percentIpercent then that means that header.h is an sccs edit version.
    * Substitute a large sccs version instead of putting the sccs keyword
    * into the header
    */
   if (strcmp(I, HD_VERSION) != 0)
      (void) strcpy(e_h->common.hdvers, HD_VERSION);
   else
      (void) strcpy(e_h->common.hdvers, "999");

   /*
    * fix up the program and version fields
    */
   strcpy(e_h->common.prog, "sphere_to_esps");

   /* add a comment */
   add_comment(e_h, "Converted from Sphere header");

   /* add pointer to sphere header */
   *add_genhd_l("sphere_hd_ptr", (long *) NULL, 1, e_h) = (long) tsp;

   for (i = 0; i < nfields; i++) {
      name = fields[i];
#ifdef DEBUG
      fprintf(stderr, "timit: name: %s\n", name);
#endif
      n = sp_get_field(t_h, name, &type, &size);
      spsassert(n >= 0, "sp_get_field returned error");

      p = q = malloc((unsigned) size);
      spsassert(p, "malloc failed");
      n = sp_get_data(t_h, name, p, &size);
      spsassert(n >= 0, "sp_get_data returned error");

#ifdef DEBUG
      fprintf(stderr, "timit_to_fea: ");
      fprintf(stderr, "field: %s", name);
      fprintf(stderr, " size: %d", size);
#endif

      switch (type) {
      case T_STRING:
	 {
	    char           *temp_p;
#ifdef DEBUG
	    fprintf(stderr, " string data: %s\n", p);
#endif
	    temp_p = malloc((unsigned) size + 1);
	    spsassert(temp_p, "malloc failed");
	    bcopy(p, temp_p, size);
	    temp_p[size] = 0;
	    (void) add_genhd_c(name, temp_p, size, e_h);
	    break;
	 }
      case T_INTEGER:
#ifdef DEBUG
	 fprintf(stderr, " int data: %ld\n", *(long *) p);
#endif
	 *add_genhd_l(name, NULL, 1, e_h) = *(long *) p;
	 break;
      case T_REAL:
#ifdef DEBUG
	 fprintf(stderr, " real data: %lf\n", *(double *) p);
#endif
	 *add_genhd_d(name, NULL, 1, e_h) = *(double *) p;
	 break;
      }
      free(q);
   }

   if (sp_set_data_mode(tsp, "SE-PCM-2") > 0) {
      if (EspsDebug)
	 sp_print_return_status(stderr);
      return NULL;
   }
   free(fields);
   return e_h;
}

#define BUFSIZE 8192


static struct feasd *sd_rec;	/* record for input data */
static short   *sdata;		/* pointer to data in fea_sd record */

struct header_t *
sd_to_sphere(ifile, ofile, byte_format)
   FILE           *ifile;
   FILE           *ofile;
   char           *byte_format;

{
   struct header  *hd, *ehd;
   struct header_t *shd;
   long            sample_rate;
   long            nrecords = 0;
   int             i;
   double          max_value = -DBL_MAX;
   double          min_value = DBL_MAX;
   long            max;
   long            min;
   long            ival;
   long            hbytes, dbytes;
   struct header
                  *ohd_esps;	/* dummy output ESPS header */
   int             num_read = 0;/* number samples read  */
   int             breverse;
   struct header  *sdtofea();
   int             j;

   if (ifile == NULL) {
      Fprintf(stderr, "sd_to_sphere: NULL input; returning NULL.\n");
      return (NULL);
   }
   if (!((strcmp(byte_format, "10") == 0)
	 || (strcmp(byte_format, "01") == 0))) {

      Fprintf(stderr, "sd_to_sphere: byte_format must be '01' or '10'.\n");
      return (NULL);
   }
   if (strcmp(byte_format, "01") == 0)
      breverse = 1;
   else
      breverse = 0;

   sphere_to_sd_ctrl = 123;
   hd = read_header(ifile);
   if (hd == NULL) {
      Fprintf(stderr,
	   "sd_to_sphere: ESPS header not SD or FEA_SD; returning NULL.\n");

      return (NULL);
   }
   sphere_to_sd_ctrl = 0;

   if (hd->common.type == FT_SD)
      ehd = sdtofea(hd);
   else if ((hd->common.type == FT_FEA) && (hd->hd.fea->fea_type == FEA_SD))
      ehd = hd;
   else {
      Fprintf(stderr,
	   "sd_to_sphere: ESPS header not SD or FEA_SD; returning NULL.\n");
      return (NULL);
   }


   shd = sp_create_header();

   sample_rate = get_genhd_val("record_freq", ehd, 1);
   sp_add_field(shd, "sample_rate", T_INTEGER, (char *) &sample_rate);

   /* copy generics over */

   copy_gens(shd, ehd);

   if (ofile == NULL)
      return (shd);

   /* Allocate buffer */

   sd_rec = allo_feasd_recs(ehd, ESPS_SHORT, BUFSIZE, (char *) NULL, NO);
   sdata = (short *) sd_rec->data;

   if ((nrecords = findlim(&ifile, ehd, &max_value, &min_value)) == 0) {
      Fprintf(stderr, "sd_to_sphere: no input records or error reading it.\n");
      return (shd);
   }
   if (debug_level) {
      Fprintf(stderr, "sd_to_sphere: max_value = %lg\n", max_value);
      Fprintf(stderr, "sd_to_sphere: min_value = %lg\n", min_value);
   }
   max = max_value;
   min = min_value;

   sp_add_field(shd, "sample_min", T_INTEGER, (char *) &min);
   sp_add_field(shd, "sample_max", T_INTEGER, (char *) &max);
   sp_add_field(shd, "sample_count", T_INTEGER, (char *) &nrecords);

   ival = 1;
   sp_add_field(shd, "channel_count", T_INTEGER, (char *) &ival);

   ival = 2;
   sp_add_field(shd, "sample_n_bytes", T_INTEGER, (char *) &ival);

   sp_add_field(shd, "sample_byte_format", T_STRING, byte_format);

   ival = 16;
   sp_add_field(shd, "sample_sig_bits", T_INTEGER, (char *) &ival);

   j = sp_write_header(ofile, shd, &hbytes, &dbytes);

   /* create dummy output ESPS header for use by put_feasd_recs */

   ohd_esps = new_header(FT_FEA);

   init_feasd_hd(ohd_esps, ESPS_SHORT, 1, get_genhd_d("start_time", ehd),
		 0, get_genhd_val("record_freq", ehd, 1));

   do {
      num_read = get_feasd_recs(sd_rec, 0L, BUFSIZE, ehd, ifile);

      if (num_read != 0) {
	 if (breverse) {
	    for (i = 0; i < BUFSIZE; i++)
	       sdata[i] = short_reverse(sdata[i]);
	 }
	 put_feasd_recs(sd_rec, 0L, num_read, ohd_esps, ofile);
      }
   } while (num_read == BUFSIZE);

   return (shd);
}

static int
copy_gens(dest, src)
   struct header_t *dest;
   struct header  *src;
{
   int             i, count = 0, k;
   char           *ptr;
   struct gen_hd  *np, *lookup();
   long            intval;
   double          dblval;

   /* check arguments */

   spsassert(dest && src, "Bad args in copy_gens");

   /* check for no items in src */
   if (src->variable.ngen == 0)
      return 0;

   /* find entries */
   for (i = 0; i < GENTABSIZ; i++) {
      np = src->variable.gentab[i];
      while (np != NULL) {
	 /* create new item and copy data */

	 switch (np->type) {
	 case ESPS_SHORT:
	 case CODED:
	    intval = *(short *) np->d_ptr;
	    sp_add_field(dest, np->name, T_INTEGER, (char *) &intval);
	 case LONG:
	    intval = *(long *) np->d_ptr;
	    sp_add_field(dest, np->name, T_INTEGER, (char *) &intval);
	    break;
	 case FLOAT:
	    dblval = *(float *) np->d_ptr;
	    sp_add_field(dest, np->name, T_REAL, (char *) &dblval);
	    break;
	 case DOUBLE:
	    dblval = *(double *) np->d_ptr;
	    sp_add_field(dest, np->name, T_REAL, (char *) &dblval);
	    break;
	 case CHAR:
	    sp_add_field(dest, np->name, T_STRING, np->d_ptr);
	    break;
	 }
	 np = np->next;
	 count++;
      }
   }
   return count;
}


static long
findlim(file, hd, max, min)
   FILE          **file;
   struct header  *hd;
   double         *max, *min;
/*
 * find max and min of data in file; if input is a pipe, then write data to
 * temporary file and change the corresponding file pointer; this is so the
 * data can be read again in order to copy it from input to output in the
 * main program
 */
{
   FILE           *tmpstrm;	/* temp file; might be return to main */
   int             first = 1;	/* flag for first time read from file */
   long            tot_read = 0;
   int             i;
   int             num_read = 0;

   if (hd->common.ndrec == -1) {
      tmpstrm = tmpfile();
      if (debug_level)
	 Fprintf(stderr, "sd_to_sphere: input is pipe; creating temp file\n");
   }
   do {
      num_read = get_feasd_recs(sd_rec, 0L, BUFSIZE, hd, *file);

      tot_read += num_read;

      if (debug_level > 1)
	 Fprintf(stderr, "sd_to_sphere: num_read = %d\n", num_read);
      if (first) {
	 if (num_read == 0)
	    return 0;
	 first = 0;
      }
      for (i = 0; i < num_read; i++) {
	 if (sdata[i] < *min)
	    *min = sdata[i];
	 if (sdata[i] > *max)
	    *max = sdata[i];
      }
      if (num_read != 0 && hd->common.ndrec == -1) {
	 put_feasd_recs(sd_rec, 0L, num_read, hd, tmpstrm);
	 if (debug_level > 1)
	    Fprintf(stderr, "sd_to_sphere: wrote %d samples to temp file\n",
		    num_read);
      }
   } while (num_read == BUFSIZE);

   if (hd->common.ndrec == -1) {/* Input was a pipe */
      (void) rewind(tmpstrm);
      *file = tmpstrm;
   } else {
      (void) rewind(*file);
      sphere_to_sd_ctrl = 123;
      (void) read_header(*file);
      sphere_to_sd_ctrl = 0;
   }
   return (tot_read);
}

int
sphere_set_file(sp, stream)
   SP_FILE        *sp;
   FILE           *stream;
{
   SPWAVEFORM     *wp;
   SP_FILE        *tsp;

   if (sp && sp->read_spifr->status->read_occured_flag) {
      wp = sp->read_spifr->waveform;

      if (wp->sp_fob->fp && !sp->read_spifr->status->is_temp_file) {
	 wp->sp_fob->fp = stream;
      } else if (wp->sp_fob->fp && sp->read_spifr->status->is_temp_file) {
	 (void) rewind(wp->sp_fob->fp);
      } else if (wp->sp_fob->buf) {
	 return 1;
      }
   }
   return 0;
}

int
sphere_temp(sp)
   SP_FILE        *sp;
{
   return (sp && sp->read_spifr->status->is_temp_file);
}

int
sphere_been_read(sp)
   SP_FILE        *sp;
{
   return (sp && sp->read_spifr->status->read_occured_flag);
}

char           *
get_sphere_hdr(hdr)
   struct header         *hdr;
{
   if (hdr)
      return (char *) get_genhd_val_l("sphere_hd_ptr", hdr, 0L);
   else
      return NULL;
}

/* this is just to force this function to be linked in for later use */
int DUMMY()
{
   SP_FILE *hd;
   sp_rewind(hd);
   return 1;
}
