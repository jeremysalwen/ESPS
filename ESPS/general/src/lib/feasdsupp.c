/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1989-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  Rodney Johnson
 * Checked by:
 * Revised by:
 *
 * Support routines for FEA file subtype FEA_SD
 */

static char *sccs_id = "@(#)feasdsupp.c	1.16	5/1/98	ESI/ERL";

/*
 * Include files.
 */

#include <stdio.h>
#include <esps/esps.h>
#if !defined(LINUX_OR_MAC)
#include <esps/unix.h>
#endif
#include <esps/fea.h>
#include <esps/feasd.h>
#include <math.h>
#include <esps/esignal_fea.h>
#include <esps/pc_wav.h>

char           *arr_alloc(), *marg_index();
int             typesiz();
char           *zero_fill();
char           *type_convert();

long            miio_get(), miio_put();
static int      numeric();
char           *get_sphere_hdr();

static void
Warning(i, s)
   int             i;
   char           *s;
{
   if (i)
      fprintf(stderr, "Warning (get_feasd_recs): %s\n", s);
   else
      fprintf(stderr, "                          %s\n", s);
}

/*
 * these routines are used to implement a varient of mu_law and bit reverse
 * for Ft. Meade
 */
/***********************************************************************/

static int
rev(i, bits)
   int             i;
   int             bits;
{
   int             rev_i = i;
   switch (bits) {
   case 8:
      rev_i = ((rev_i & 0xf0) >> 4) | ((rev_i & 0x0f) << 4);
   case 4:
      rev_i = ((rev_i & 0xcc) >> 2) | ((rev_i & 0x33) << 2);
   case 2:
      rev_i = ((rev_i & 0xaa) >> 1) | ((rev_i & 0x55) << 1);
      break;
   default:
      spsassert(0, "error in call to rev (bits)");
   }
   return rev_i;
}

static void
bit_reverse(buffer, num_read)
   char           *buffer;
   int             num_read;
{
   int             i;
   for (i = 0; i < num_read; i++) {
      buffer[i] = rev(buffer[i], 8);
   }
}

static void
mu2_to_linear(in, out, n)
   char           *in;
   short          *out;
   long            n;
{
   int             i;
   double          M, N, s, sign, two_pow_N, result;

   for (i = 0; i < n; i++) {
      M = (double) (rev((in[i] & 0xf0) >> 4, 4) ^ 0xf);
      N = (double) (rev((in[i] & 0x0e), 4) ^ 0x7);
      s = (double) ((in[i] & 0x01) ^ 0x1);

      sign = pow((double) -1.0, s);
      two_pow_N = pow((double) 2.0, N);
      result = sign * (((double) 16.5 + M) * two_pow_N - (double) 16.5);
      out[i] = result;
   }
}

/***********************************************************************/


/*
 * This function fills in the header of a FEA file to make it a file of
 * subtype FEA_SD.
 */

#define ADDFLD(name,size,rank,dimen,type,enums) \
        {if (add_fea_fld((name),(long)(size),(rank),(long*)(dimen), \
                type,(char**)(enums),hd) == -1) return 1;}

int
init_feasd_hd(hd, data_type, num_channels, start_time, mult_st_t, record_freq)
   struct header  *hd;
   int             data_type, num_channels;
   double         *start_time;
   int             mult_st_t;
   double          record_freq;
{
   int             i;

   spsassert(hd, "init_feasd_hd: hd is NULL");
   spsassert(start_time, "init_feasd_hd: start_time is NULL");
   spsassert(hd->common.type == FT_FEA, "init_feasd_hd: file not FEA");

   /*
    * First, put in the generic header items.
    */

   if (mult_st_t) {
      double         *ptr =
      add_genhd_d("start_time", (double *) NULL, num_channels, hd);

      for (i = 0; i < num_channels; i++)
	 ptr[i] = start_time[i];
   } else
      *add_genhd_d("start_time", (double *) NULL, 1, hd) = *start_time;

   *add_genhd_d("record_freq", (double *) NULL, 1, hd) = record_freq;

   /*
    * Then define the record fields.
    */

   ADDFLD("samples", num_channels, 1, NULL, data_type, NULL)
      hd->hd.fea->fea_type = FEA_SD;
   return 0;
}

/*
 * This function allocates a record structure for the FEA file subtype
 * FEA_SD.
 */

struct feasd   *
allo_feasd_recs(hd, data_type, num_records, data, make_ptrs)
   struct header  *hd;
   int             data_type;
   long            num_records;
   char           *data;
   int             make_ptrs;
{
   struct feasd   *sd_rec;
   long            num_channels, num_samples;

   spsassert(hd, "allo_feasd_recs: hd is NULL");
   spsassert(hd->common.type == FT_FEA && hd->hd.fea->fea_type == FEA_SD,
	     "allo_feasd_recs: file not FEA or not FEA_SD");
   spsassert(numeric(data_type),
	     "allo_feasd_recs: data type unknown or not numeric.");

   sd_rec = (struct feasd *) malloc(sizeof(struct feasd));
   spsassert(sd_rec, "allo_feasd_recs: malloc failed on sd_rec.");

   num_channels = get_fea_siz("samples", hd, (short *) NULL, (long **) NULL);
   spsassert(num_channels,
	     "allo_feasd_recs: no \"samples\" field defined in header.");

   sd_rec->data_type = data_type;
   sd_rec->num_records = num_records;
   sd_rec->num_channels = num_channels;

   if (!data) {
      num_samples = num_records * num_channels;
      sd_rec->data = arr_alloc(1, &num_samples, data_type, 0);
   } else
      sd_rec->data = data;


   if (make_ptrs) {
      long            dim[2];

      dim[0] = num_records;
      dim[1] = num_channels;
      sd_rec->ptrs = marg_index(sd_rec->data, 2, dim, data_type, 0);
   } else
      sd_rec->ptrs = NULL;

   return sd_rec;
}

/*
 * This routine reads records of the FEA file subtype FEA_SD.
 */

long
get_feasd_recs(data, start, num_records, hd, file)
   struct feasd   *data;
   long            start, num_records;
   struct header  *hd;
   FILE           *file;
{
   int             source_type, dest_type;
   long            num_channels;
   int             rec_size;
   char           *buffer, *buf_ptr;
   char           *fea_ptr;
   long            num_read;
   long            i;
   struct fea_data *fea_rec;
   char           *sd_data;
   int             encoding;
   char           *sphere_hdr_ptr;
   esignal_hd	  *esignal_hdr_ptr;
   pc_wav_hd	  *pc_wav_hdr_ptr;

   spsassert(hd, "get_feasd_recs: hd is NULL");
   spsassert(data, "get_feasd_recs: sd record struct is NULL");
   spsassert(file, "get_feasd_recs: file is NULL");
   spsassert(hd->common.type == FT_FEA
	     && hd->hd.fea->fea_type == FEA_SD,
	     "get_feasd_recs: file not FEA or not FEA_SD");
   spsassert(!hd->common.tag,
	     "get_feasd_recs: tagged FEA_SD files not supported.");
   spsassert(start >= 0,
	     "get_feasd_recs: negative starting record.");
   spsassert(num_records >= 0,
	     "get_feasd_recs: negative number of records requested.");
   spsassert(start + num_records <= data->num_records,
	     "get_feasd_recs: requested samples overrun end of data array.");

   if (num_records == 0)
      return 0;

   num_channels = get_fea_siz("samples", hd, (short *) NULL, (long **) NULL);
   spsassert(num_channels == data->num_channels,
       "get_feasd_recs: data array num_channels inconsistent with header.");

   source_type = get_fea_type("samples", hd);
   dest_type = data->data_type;

   esignal_hdr_ptr = get_esignal_hdr(hd);
   sphere_hdr_ptr = get_sphere_hdr(hd);
   pc_wav_hdr_ptr = get_pc_wav_hdr(hd);
   encoding = (int) get_genhd_val("encoding", hd, (double) 0.0);

   if (sphere_hdr_ptr)
      source_type = SHORT;

   if (encoding)
      source_type = BYTE;

   rec_size = typesiz(source_type) * num_channels;

   sd_data = data->data + start * typesiz(dest_type) * num_channels;

   if (source_type != dest_type) {	/* conversion required */
      buffer = calloc((unsigned) num_records, (unsigned) rec_size);
      spsassert(buffer,
		"get_feasd_recs: couldn't allocate buffer.");
   } else			/* read directly into data array */
      buffer = sd_data;

   if (esignal_hdr_ptr)		/* read with esignal functions */
   {
      num_read =
	 esignal_getsd_recs(buffer, num_records, esignal_hdr_ptr, file);
   }
   else if (sphere_hdr_ptr)	/* read it with the sphere functions */
   {
      num_read =
	 sp_mc_read_data(buffer, num_records, sphere_hdr_ptr);
   }
   else if (pc_wav_hdr_ptr)	/* read it with the PC WAVE functions */
   {
       num_read =
	   pc_wav_getsd_recs(buffer, num_records, pc_wav_hdr_ptr, file)
	   / data->num_channels;
   }
   else if (hd->hd.fea->field_count == 1) /* block read possible */
   {
      num_read =
	 miio_get(source_type, buffer, (int) (num_records * num_channels),
		  hd->common.edr, hd->common.machine_code, file);
      if (num_read % num_channels != 0)
	 Fprintf(stderr, "%s: Warning--partial record read.\n",
		 "get_feasd_recs");
      num_read /= num_channels;
   }
   else				/* read a record at a time */
   {
      fea_rec = allo_fea_rec(hd);
      spsassert(fea_rec,
		"get_feasd_recs: can't allocate FEA record.");
      fea_ptr = get_fea_ptr(fea_rec, "samples", hd);
      spsassert(fea_ptr,
		"get_feasd_recs: can't get pointe to \"samples\" field.");
      buf_ptr = buffer;

      for (num_read = 0;
	   num_read < num_records && get_fea_rec(fea_rec, hd, file) != EOF;
	   num_read++, buf_ptr += rec_size
	 ) {
#define	    CASE(type) \
		{ \
		    type    *buf = (type *) buf_ptr; \
		    type    *fea = (type *) fea_ptr; \
		    \
		    for (i = 0; i < data->num_channels; i++) \
			buf[i] = fea[i]; \
		} \
		break;
	 switch (source_type) {
	 case BYTE:
	    CASE(char)
	 case SHORT:
	    CASE(short)
	 case LONG:
	    CASE(long)
	 case FLOAT:
	    CASE(float)
	 case DOUBLE:
	    CASE(double)
	 case BYTE_CPLX:
	    CASE(byte_cplx)
	 case SHORT_CPLX:
	    CASE(short_cplx)
	 case LONG_CPLX:
	    CASE(long_cplx)
	 case FLOAT_CPLX:
	    CASE(float_cplx)
	 case DOUBLE_CPLX:
	    CASE(double_cplx)
	 }
#undef	    CASE
      }
      free_fea_rec(fea_rec);
   }
   if (encoding) {
      short          *conv_buffer;

      if (0 /* source_type != BYTE */) {
	 Warning(1, "header item encoding indicates mu-law but file is not BYTE");
	 Warning(0, "the data will not be converted.");
      } else if (dest_type == BYTE) {
	 Warning(1, "cannot decode mu-law data into BYTE format");
	 Warning(0, "the data will not be converted.");
      } else {
	 if (dest_type == SHORT)
	    conv_buffer = (short *) sd_data;
	 else {
	    conv_buffer = (short *) calloc((unsigned) num_records, sizeof(short) * num_channels);
	    spsassert(conv_buffer, "calloc failed!");
	 }

	 switch (encoding) {
	 case 1:		/* normal esps mu-law encoded */
	    mu_to_linear(buffer, conv_buffer, (long) num_read*num_channels);
	    source_type = SHORT;
	    rec_size = sizeof(short) * num_channels;
	    free(buffer);
	    buffer = (char *) conv_buffer;
	    break;
	 case 2:		/* bit reverse esps mu-law encoded */
	    bit_reverse(buffer, num_read);
	    mu_to_linear(buffer, conv_buffer, (long) num_read*num_channels);
	    source_type = SHORT;
	    rec_size = sizeof(short) * num_channels;
	    free(buffer);
	    buffer = (char *) conv_buffer;
	    break;
	    /*
	     * the following two methods were provides by Terri Kamm at DoD.
	     * The reason that the no bit reverse case calls the reverse
	     * routine is that the reverse is already in the algorithm
	     */
	 case 3:		/* Terri Kamm mu_law, bit reverse */
	    mu2_to_linear(buffer, conv_buffer, (long) num_read*num_channels);
	    source_type = SHORT;
	    rec_size = sizeof(short) * num_channels;
	    free(buffer);
	    buffer = (char *) conv_buffer;
	    break;
	 case 4:		/* Terri Kamm Sun mu_law, no bit reverse */
	    bit_reverse(buffer, num_read);
	    mu2_to_linear(buffer, conv_buffer, (long) num_read*num_channels);
	    source_type = SHORT;
	    rec_size = sizeof(short) * num_channels;
	    free(buffer);
	    buffer = (char *) conv_buffer;
	    break;
	    /*
	     * these are the two A-law versions that we have
	     */
	 case 5:		/* standard alaw */
	    (void) a_to_linear(buffer, conv_buffer,(long)num_read*num_channels);
	    source_type = SHORT;
	    rec_size = sizeof(short) * num_channels;
	    free(buffer);
	    buffer = (char *) conv_buffer;
	    break;
	 case 6:		/* CCITT */
	    (void) a_to_linear_2(buffer, conv_buffer, (long)num_read*num_channels);
	    source_type = SHORT;
	    rec_size = sizeof(short) * num_channels;
	    free(buffer);
	    buffer = (char *) conv_buffer;
	    break;
	 default:
	    Warning(1, "unknown value for header item encoding ignored");
	    break;
	 }
      }
   }
   (void) zero_fill((num_records - num_read) * num_channels,
		    source_type, buffer + num_read * rec_size);

   if (source_type != dest_type) {
      (void) type_convert(num_records * num_channels, buffer, source_type,
			  sd_data, dest_type, (void (*) ()) NULL);
      free(buffer);
   }
   return num_read;
}


/*
 * This routine reads possibly overlapping or noncontiguous records of the
 * FEA file subtype FEA_SD.
 */

long
get_feasd_orecs(data, start, framelen, step, hd, file)
   struct feasd   *data;
   long            start, framelen, step;
   struct header  *hd;
   FILE           *file;
{
   int             i;
   long            numoldsamples, numnewsamples;

   spsassert(hd, "get_feasd_orecs: hd is NULL");
   spsassert(data, "get_feasd_orecs: sd record struct is NULL");
   spsassert(file, "get_feasd_orecs: file is NULL");
   spsassert(hd->common.type == FT_FEA
	     && hd->hd.fea->fea_type == FEA_SD,
	     "get_feasd_orecs: file not FEA or not FEA_SD");
   spsassert(step >= 0,
	     "get_feasd_orecs: negative step.");
   spsassert(start >= 0,
	     "get_feasd_orecs: negative starting record.");
   spsassert(framelen >= 0,
	     "get_feasd_orecs: negative frame length.");
   spsassert(start + framelen <= data->num_records,
	     "get_feasd_orecs: frame overruns end of data array.");

   if (step == 0)
      return framelen;

   if (step >= framelen) {
      fea_skiprec(file, step - framelen, hd);
      return get_feasd_recs(data, start, framelen, hd, file);
   }
   numoldsamples = (framelen - step) * data->num_channels;
   numnewsamples = step * data->num_channels;

#define CASE(type) \
	{ \
	    type	*buf; \
	    \
	    buf = (type *) data->data + start * data->num_channels; \
	    for (i = 0; i < numoldsamples; i++) \
		buf[i] = buf[i + numnewsamples]; \
	    break; \
	}
   switch (data->data_type) {
   case BYTE:
      CASE(char)
   case SHORT:
      CASE(short)
   case LONG:
      CASE(long)
   case FLOAT:
      CASE(float)
   case DOUBLE:
      CASE(double)
   case BYTE_CPLX:
      CASE(byte_cplx)
   case SHORT_CPLX:
      CASE(short_cplx)
   case LONG_CPLX:
      CASE(long_cplx)
   case FLOAT_CPLX:
      CASE(float_cplx)
   case DOUBLE_CPLX:
      CASE(double_cplx)
   }
#undef CASE

   return framelen - step
      + get_feasd_recs(data, start + framelen - step, step, hd, file);
}

/*
 * This routine writes records of the FEA file subtype FEA_SD.
 */

int
put_feasd_recs(data, start, num_records, hd, file)
   struct feasd   *data;
   long            start, num_records;
   struct header  *hd;
   FILE           *file;
{
   long            num_channels;
   int             source_type, dest_type;
   int             rec_size;
   char           *sd_data, *buffer;
   int             rtn;

   spsassert(hd, "put_feasd_recs: header is NULL.");
   spsassert(data, "put_feasd_recs: sd record struct is NULL.");
   spsassert(file, "put_feasd_recs: file is NULL.");
   spsassert(hd->common.type == FT_FEA
	     && hd->hd.fea->fea_type == FEA_SD,
	     "put_feasd_recs: file not FEA or not FEA_SD");
   spsassert(start >= 0,
	     "put_feasd_recs: negative starting record.");
   spsassert(num_records >= 0,
	     "put_feasd_recs: can't write negative number of records.");
   spsassert(start + num_records <= data->num_records,
	"put_feasd_recs: records to be written overrun end of data array.");

   num_channels = get_fea_siz("samples", hd, (short *) NULL, (long **) NULL);
   spsassert(num_channels == data->num_channels,
       "put_feasd_recs: data array num_channels inconsistent with header.");
   spsassert(hd->hd.fea->field_count == 1,
	     "put_feasd_recs: extraneous fields defined in header.");

   if (num_records == 0)
      return 0;

   dest_type = get_fea_type("samples", hd);
   source_type = data->data_type;
   rec_size = typesiz(dest_type) * num_channels;

   sd_data = data->data + start * typesiz(source_type) * num_channels;

   if (source_type != dest_type) {	/* conversion required */
      buffer = calloc((unsigned) num_records, (unsigned) rec_size);
      spsassert(buffer,
		"put_feasd_recs: couldn't allocate buffer.");
      (void) type_convert(num_records * num_channels, sd_data, source_type,
			  buffer, dest_type, (void (*) ()) NULL);
   } else			/* write directly from data array */
      buffer = sd_data;

   rtn = miio_put(dest_type, buffer,
		  (int) (num_records * num_channels), hd->common.edr, file)
      != num_records * num_channels;

   if (source_type != dest_type)
      free(buffer);

   return rtn;
}

/*
 * Check for numeric data type.
 */

static int
numeric(type)
   int             type;
{
   switch (type) {
   case BYTE:
   case SHORT:
   case LONG:
   case FLOAT:
   case DOUBLE:
   case BYTE_CPLX:
   case SHORT_CPLX:
   case LONG_CPLX:
   case FLOAT_CPLX:
   case DOUBLE_CPLX:
      return YES;
      break;
   default:
      return NO;
      break;
   }

   /* NOTREACHED */
}


long
miio_get(type_code, ptr, n, flag, mach, fp)
   int             type_code;
   char           *ptr;
   int             n;
   int             flag;
   int             mach;
   FILE           *fp;
{
#define	CASE(type, miio_func) \
	return miio_func((type *) ptr, n, flag, mach, fp);
   switch (type_code) {
   case CHAR:
      CASE(char, miio_get_char)
   case BYTE:
      CASE(char, miio_get_byte)
   case SHORT:
      CASE(short, miio_get_short)
   case LONG:
      CASE(long, miio_get_long)
   case FLOAT:
      CASE(float, miio_get_float)
   case DOUBLE:
      CASE(double, miio_get_double)
   case BYTE_CPLX:
      CASE(byte_cplx, miio_get_bcplx)
   case SHORT_CPLX:
      CASE(short_cplx, miio_get_scplx)
   case LONG_CPLX:
      CASE(long_cplx, miio_get_lcplx)
   case FLOAT_CPLX:
      CASE(float_cplx, miio_get_fcplx)
   case DOUBLE_CPLX:
      CASE(double_cplx, miio_get_dcplx)
   default:
      Fprintf(stderr, "%s: unrecognized type code %d.\n",
	      "miio_get", type_code);
      exit(1);
      /* NOTREACHED */
   }
#undef	CASE
}

long
miio_put(type_code, ptr, n, flag, fp)
   int             type_code;
   char           *ptr;
   int             n;
   int             flag;
   FILE           *fp;
{
#define	CASE(type, miio_func) \
	return miio_func((type *) ptr, n, flag, fp);
   switch (type_code) {
   case CHAR:
      CASE(char, miio_put_char)
   case BYTE:
      CASE(char, miio_put_byte)
   case SHORT:
      CASE(short, miio_put_short)
   case LONG:
      CASE(long, miio_put_long)
   case FLOAT:
      CASE(float, miio_put_float)
   case DOUBLE:
      CASE(double, miio_put_double)
   case BYTE_CPLX:
      CASE(byte_cplx, miio_put_bcplx)
   case SHORT_CPLX:
      CASE(short_cplx, miio_put_scplx)
   case LONG_CPLX:
      CASE(long_cplx, miio_put_lcplx)
   case FLOAT_CPLX:
      CASE(float_cplx, miio_put_fcplx)
   case DOUBLE_CPLX:
      CASE(double_cplx, miio_put_dcplx)
   default:
      Fprintf(stderr, "%s: unrecognized type code %d.\n",
	      "miio_put", type_code);
      exit(1);
      /* NOTREACHED */
   }
#undef	CASE
}
