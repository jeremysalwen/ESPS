/*
 * This material contains unpublished, proprietary software of 
 * Entropic Research Laboratory, Inc. Any reproduction, distribution, 
 * or publication of this work must be authorized in writing by Entropic 
 * Research Laboratory, Inc., and must bear the notice: 
 *
 *    "Copyright (c) 1987-1990  AT&T, Inc.
 *    "Copyright (c) 1986-1990  Entropic Speech, Inc. 
 *    "Copyright (c) 1990-1996  Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * The copyright notice above does not evidence any actual or intended 
 * publication of this source code.     
 *
 * Written by:  David Talkin
 * Checked by:
 * Revised by:  David Talkin, Rod Johnson, John Shore, Alan Parker
 *
 * Brief description:
 * Data access methods for commonly encountered data types.
 */

static char *sccs_id = "@(#)read_data.c	1.43	5/1/98	ATT/ESI/ERL";

#include <sys/file.h>
#ifdef OS5
#include <sys/fcntl.h>
#endif
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feasd.h>
#include <esps/feaspec.h>
#include <Objects.h>
#include <spectrogram.h>
#include <esps/esignal_fea.h>
#include <esps/pc_wav.h>

#ifndef NULL
#define NULL 0
#endif

short           get_fea_type();
char           *get_fea_ptr();
void            free_fea_rec();
struct fea_data *allo_fea_rec();
void            free_signal();
int		intern_size_rec();

extern int      debug_level;
extern int      max_buff_bytes;	/* defined in globals.c */

void            check_file_size();

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
isa_homogeneous_vector_signal_etc(s)
   Signal         *s;
{
   if (s && s->header) {
      if (s->header->esps_hdr && (s->header->magic == ESPS_MAGIC)) {
	 struct header  *h = s->header->esps_hdr;
	 int             ok_byte_order =
	    ((h->common.edr || edr_default(h->common.machine_code))
	     && !get_pc_wav_hdr(h));
#if  defined(SUN386i) || defined(DS3100) || defined(VAX) || defined(DEC_ALPHA) || defined(LINUX_OR_MAC)
	 ok_byte_order = !ok_byte_order;
#endif
	 if (get_esignal_hdr(h))
	    return FALSE;
	 if (get_genhd_val("sphere_hd_ptr", h, (double) 0.0))
	    return FALSE;
	 if (get_genhd_val("encoding", h, (double) 0.0))
	    return FALSE;
	 if (h->common.tag)
	    return FALSE;
	 if (s->type == P_MIXED) {
	    if (*s->types) {
	       int            *tp = s->types, i = s->dim - 1, t = *tp++, ish = 1;
	       while (i-- > 0)
		  if (*tp++ != t)
		     ish = 0;
	       return (ish && ok_byte_order &&
		       (h->hd.fea->field_count == 1));
	    } else {
	       fprintf(stderr, "Apparantly bogus header passed to isa_homogeneous_vector_signal_etc(%s)\n", s->name);
	       return (FALSE);
	    }
	 } else
	    return (ok_byte_order && IS_GENERIC(s->type));
      } else if (s->header->magic == SIGNAL_MAGIC)
	 return (IS_GENERIC(s->type));
   }
   return (FALSE);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
type_of_signal(s)
   Signal         *s;
{
   if (isa_homogeneous_vector_signal_etc(s)) {
      if (s->type == P_MIXED)
	 return (*s->types);
   }
   return (s->type);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
size_of_fea_unmixed(s)
   Signal         *s;
{
   if (s && s->header && s->header->esps_hdr && (s->header->magic == ESPS_MAGIC) && s->types) {
      int             typsize;

      switch (*s->types) {
      case P_SHORTS:
      case P_USHORTS:
	 typsize = sizeof(short);
	 break;
      case P_INTS:
      case P_UINTS:
	 typsize = sizeof(int);
	 break;
      case P_FLOATS:
	 typsize = sizeof(float);
	 break;
      case P_DOUBLES:
	 typsize = sizeof(double);
	 break;
      case P_CHARS:
      case P_UCHARS:
	 typsize = sizeof(char);
	 break;
      default:
	 typsize = sizeof(short);
	 fprintf(stderr, "Faking element size to shorts (probably wrong)\n");
	 break;
      }
      return (typsize);
   }
   return (0);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
install_signal_methods(s)
   Signal         *s;
{
   if (IS_TAGGED_FEA(s) &&
       (get_genhd_val("src_sf", s->header->esps_hdr, -1.0) > 0.0))
      install_tag_fea_methods(s);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
read_data(s, start, page_size)
   Signal         *s;
   double          start, page_size;
{
   int             fd;
   int             retrn;
   int             typsize;
   char           *sp;
   char           *name;
   Signal         *get_any_signal();
   char           *get_sphere_hdr();

   if (s) {
      if (debug_level >= 1) {
	 fprintf(stderr, "read_data: s->name = \"%s\".  s->file = %d.\n",
		 s->name, s->file);
	 fprintf(stderr, "read_data: start: %f, page_size: %f\n",
		 start, page_size);
      }
#ifdef DEMO
      if (!(s->header && s->header->magic == ESPS_MAGIC)
	  && (s->type & SPECIAL_SIGNALS) != SIG_FORMANTS
	 )
	 return FALSE;
#endif

      if (is_feasd_sphere(s))
         close_sig_file(s);
         
      if ((fd = s->file) < 0) {
	 if (!((s->name) && ((s->file = fd = open(s->name, O_RDONLY)) >= 0))) {
	    fprintf(stderr,
	    "read_data(): Can't open the data file or file doesn't exist\n");
	    return (FALSE);
	 }
	 if (s->header && s->header->magic == ESPS_MAGIC) {
	    char           *sp;
	    s->header->strm = fdopen(fd, "r");

            if (!is_feasd_sphere(s))
                if (debug_level)
                 fprintf(stderr,"in read_data, not a sphere file, seeking..\n");
	        if (fseek(s->header->strm, (long) s->bytes_skip, 0)) {
	           fprintf(stderr,
		   "read_data: Can't skip header (%d bytes) of ESPS file.\n",
		    s->bytes_skip);
	        return FALSE;
	        }
	 }
      }
      install_signal_methods(s);

      if (s->start_time > start)
	 start = s->start_time;

      if ((!IS_TAGGED_FEA(s)) && (page_size >= 0.0)
	  && (page_size < 1.0 / s->freq)) {	/* check for silly size */
	 /*
	  * fprintf(stderr,"%s: Requested segment too short (%g s); no
	  * records read.\n", "read_data", page_size);
	  */
	 return TRUE;
      }				/* if read length is 0, don't check type */
      /*
       * check for special sigs before anything which assumes vector sigs.
       */
      if (s->type & SPECIAL_SIGNALS) {
	 switch (s->type & SPECIAL_SIGNALS) {
	 case SIG_SPECTROGRAM:
	    if (s->header && s->header->magic == ESPS_MAGIC)
	       switch (s->header->esps_hdr->common.type) {
	       case FT_FEA:
		  switch (s->header->esps_hdr->hd.fea->fea_type) {
		  case FEA_SPEC:
		     return get_esps_fspec(s, start, page_size);
		     break;
		  default:
		     fprintf(stderr, "%s: wrong FEA subtype for spectrogram.\n",
			     "read_data");
		     return FALSE;
		     break;
		  }
		  break;
	       case FT_SPEC:
	       default:
		  fprintf(stderr, "%s: wrong ESPS file type for spectrogram.\n",
			  "read_data");
		  return FALSE;
		  break;
	       }
	    break;
	 default:		/* fall through to standard handling */
	    break;
	 }
      }
      if (IS_GENERIC(s->type)) {
	 /* If negative pg size, want entire file. */
	 if ((page_size < 0.0) && (s->freq > 0.0))
	    /*
	     * this works for periodic signals, but not for other types
	     */
	    page_size = ((double) s->file_size) / s->freq;

	 if ((s->type & SIG_FORMAT) == SIG_ASCII)
	    return (get_ascii(s, start, page_size));

	 switch (s->type & (~SPECIAL_SIGNALS)) {
	 case P_SHORTS:
	 case P_USHORTS:
	    typsize = sizeof(short);
	    break;
	 case P_INTS:
	 case P_UINTS:
	    typsize = sizeof(int);
	    break;
	 case P_FLOATS:
	    typsize = sizeof(float);
	    break;
	 case P_DOUBLES:
	    typsize = sizeof(double);
	    break;
	 case P_CHARS:
	 case P_UCHARS:
	    typsize = sizeof(char);
	    break;
	 case P_MIXED:
	    if (!(s->header
		  && s->header->magic == ESPS_MAGIC
		  && s->header->esps_hdr->common.type == FT_FEA)) {
	       fprintf(stderr, "%s: signal type P_MIXED but not ESPS FEA file.\n",
		       "read_data");
	       return FALSE;
	    }
	    if (!(typsize = size_of_fea_unmixed(s))) {
	       fprintf(stderr, "weird signal type in read_data(%s)\n", s->name);
	       return (FALSE);
	    }
	    break;
	 default:
	    fprintf(stderr, "Unknown data type %d in read_data()\n", s->type);
	    return FALSE;
	    break;
	 }

	 if (s->header && s->header->magic == ESPS_MAGIC) {
	    switch (s->header->esps_hdr->common.type) {
	    case FT_SD:
	       return get_esps_sd(s, start, page_size, typsize);
	    case FT_FEA:
	       if (debug_level)
		  fprintf(stderr, "is FT_FEA,s->type: %d\n", s->type);
	       if (s->header->esps_hdr->hd.fea->fea_type == FEA_SD
		   && s->type == P_SHORTS) {
		  if (debug_level)
		     fprintf(stderr, "calling get_esps_sd\n");
		  return get_esps_sd(s, start, page_size, typsize);
	       } else {
		  if (isa_homogeneous_vector_signal_etc(s)) {
		     if (debug_level)
			fprintf(stderr, "calling get_binary\n");
		     return (get_binary(s, start, page_size, typsize));
		  } else {
		     /*
		      * all other FEA files
		      */
		     if (debug_level)
			fprintf(stderr, "calling get_esps_fea\n");
		     return get_esps_fea(s, start, page_size);
		  }
	       }
	       break;
	    default:
	       fprintf(stderr, "read_data: Called on unsupported ESPS file type.\n");
	       return FALSE;
	       break;
	    }
	 }
	 switch (s->type & (~SPECIAL_SIGNALS)) {
	 case P_SHORTS:
	 case P_USHORTS:
	    {
	       char           *bloc;
	       if (retrn = get_binary(s, start, page_size, typsize)) {
		  bloc = *((char **) s->data);
		  if (s->header && s->header->magic == RHEADR_MAGIC)
		     byte_swap(bloc, s->buff_size * 2);
	       }
	       return (retrn);
	    }
	 default:
	    return (get_binary(s, start, page_size, typsize));
	    break;
	 }
      } else
	 fprintf(stderr, "Unknown data type %d in read_data()\n", s->type);
   } else
      fprintf(stderr, "Null Signal pointer in read_data()\n");

   return (FALSE);
}
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* Read ESPS signals from a sampled-data file.   */

get_esps_sd(s, start, page_size, typsize)
   Signal         *s;
   double          start, page_size;
   int             typsize;
{
   int             dim, sam_to_skip, old_size;
   int             i, j;
   int             max_buff_size = max_buff_bytes / typsize;
   int             loc, bytes_per_sam;
   FILE           *strm = s->header->strm;
   struct header  *hdr = s->header->esps_hdr;
   short         **spp;

   if (s->type != P_SHORTS)
   {
      fprintf(stderr,
	      "get_esps_sd: FEA_SD or SD file; Signal type not P_SHORT.\n");
      return FALSE;
   }

   sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
   dim = s->dim;
   old_size = s->buff_size;
   bytes_per_sam = dim * typsize;
   s->buff_size = (int) (0.5 + (page_size * s->freq));
   spp = (short **) s->data;

   if (s->buff_size == 0)
   {
      if (spp)
      {
	 for (i = 0; i < dim; i++)
	    if (spp[i])
	       free((char *) spp[i]);
	 free(s->data);
	 s->data = NULL;
      }

      return TRUE;
   }

   if (s->buff_size > max_buff_size) {
      s->buff_size = max_buff_size;
      fprintf(stderr,
	      "%s: read request exceeds max_buff_size; limiting to %f sec.\n",
	      "get_esps_sd", ((double) s->buff_size) / s->freq);
   }

   if (s->data && (old_size == s->buff_size) &&
       (sam_to_skip == s->start_samp))
      return TRUE;

/* ugly hack to work around sphere problem
*/
   if ((int)get_genhd_val("sphere_hd_ptr", hdr, (double) 0.0) && sam_to_skip) 
	sam_to_skip -= 1;

   if (s->file_size <= sam_to_skip) {
      fprintf(stderr,
	      "File %s too small to reach requested segment at time %g\n",
	      s->name, start);
      return FALSE;
   }

   s->start_samp = sam_to_skip;
   check_file_size(s, FALSE);

   if (is_feasd_sphere(s)) {
      if(debug_level) 
      fprintf(stderr, "get_esps_sd: Is a sphere file, do not skip hdr\n");
   } else
      if (fseek(strm, (long) s->bytes_skip, 0)) {
        fprintf(stderr, "get_esps_sd: Can't skip header of ESPS SD file.\n");
        return FALSE;
      }

   if (debug_level) 
      fprintf(stderr,
	      "calling fea_skiprec(%x, %d, %x)\n", strm, sam_to_skip, hdr);
   fea_skiprec(strm, (long) sam_to_skip, hdr);

   if (!spp)
   {
      spp = (short **) malloc(sizeof(short *) * dim);
      if (!spp)
      {
	 s->buff_size = 0;
	 fprintf(stderr,
		 "get_esps_sd: Can't allocate data pointer array.\n");
	 return FALSE;
      }

      for (i = 0; i < dim; i++)
	 spp[i] = NULL;
      s->data = (caddr_t) spp;
      old_size = 0;
   }

   if (old_size != s->buff_size)
      for (i = 0; i < dim; i++)	/* allocate an array for each dim. */
      {
	 if (spp[i])
	    free((char *) spp[i]);
	 spp[i] = (short *) malloc(s->buff_size * sizeof(short));
	 if (!spp[i])
	 {
	    fprintf(stderr, "get_esps_sd: Can't allocate data array.\n");
	    break;
	 }
      }

   if (old_size == s->buff_size || i == dim)
   {
      if (dim == 1)
      {
	 if (debug_level >= 1)
	    fprintf(stderr, "get_esps_sd: Calling get_sd_recs().\n");

	 loc = get_sd_recs(spp[0], s->buff_size,
			   s->header->esps_hdr, s->header->strm);
      }
      else
      {
	 short		*buf, *p, *q;
	 long		nread;
	 int		bsize = 500;
	 struct feasd	*feasd_rec;

	 loc = 0;
	 buf = (short *) malloc((size_t)(bytes_per_sam * bsize));

	 if (!buf)
	    fprintf(stderr, "get_esps_sd: Can't allocate data buffer.\n");
	 else
	 {
	    feasd_rec = allo_feasd_recs(s->header->esps_hdr,
					SHORT, (long) bsize, buf, NO);
	    if (!feasd_rec)
	       fprintf(stderr, "get_esps_sd: Can't allocate feasd_rec.\n");
	    else
	    {
	       do {
		  int	sam_left = s->buff_size - loc;

		  if (bsize > sam_left)
		     bsize = sam_left;

		  nread =
		     get_feasd_recs(feasd_rec, 0L, (long) bsize,
				    s->header->esps_hdr, s->header->strm);
		  if (nread >= 1)
		  {
		     for (i = 0; i < dim; i++)
			for (j = 0, p = spp[i] + loc, q = buf + i;
			     j++ < nread; q += dim)
			   *p++ = *q;
		     loc += nread;
		  }

	       } while (nread == bsize && loc < s->buff_size);

	       free((char *) feasd_rec);
	    }
	    free((char *) buf);
	 }
      }

      if (loc == 0)
	 fprintf(stderr, "get_esps_sd: Unable to read any data.\n");
      else
      {
	 if (loc < s->buff_size)
	 {
	    s->buff_size = loc;
	    check_file_size(s, TRUE);

	    for (i = 0; i < dim; i++)
	    {
	       char *sp = realloc((char *) spp[i], loc * sizeof(short));

	       if (sp)
		  spp[i] = (short *) sp;
	       else
		  break;
	    }
	 }
	 else
	    check_file_size(s, FALSE);

	 return TRUE;
      }
   }

   for (i = 0; i < dim; i++)
      if (spp[i])
	 free((char *) spp[i]);
   s->buff_size = 0;
   free(s->data);
   s->data = NULL;

   return FALSE;
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* Read spectrogram data from ESPS FEA_SPEC file. */

get_esps_fspec(s, start, page_size)
   Signal         *s;
   double          start, page_size;
{
   int             i, dim, istagged = IS_TAGGED_FEA(s);
   int             sam_to_skip, old_size, loc;
   struct feaspec *buf;
   char          **spp = (char **) s->data;
   FILE           *strm = s->header->strm;
   struct header  *hdr = s->header->esps_hdr;
   int             max_buff_size = max_buff_bytes / (s->dim * sizeof(char));
   double          ssf, get_genhd_val();
   int             truncate = 0; /* flag for too-short files */

   sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
   ssf = 8000.0;
   if (istagged && (ssf = get_genhd_val("src_sf", hdr,
				  get_genhd_val("sf", hdr, -1.0))) <= 0.0) {
      fprintf(stderr,
	      "Warning: src_sf not present in tagged file; guessing 8kHz\n");
      ssf = 8000.0;
   }
   dim = s->dim;
   old_size = s->buff_size;
   s->buff_size = (int) (0.5 + (page_size * s->freq));
   if (s->buff_size > max_buff_size) {
      s->buff_size = max_buff_size;
      /*
       * fprintf(stderr,"%s: read request exceeds max_buff_size; limiting to
       * %f sec.\n", "get_esps_fspec", ((double)s->buff_size)/s->freq);
       */
   }
   if (s->data && (old_size == s->buff_size)
       && (sam_to_skip == s->start_samp))
      return TRUE;

   if (s->file_size <= sam_to_skip) {
      fprintf(stderr, "File %s too small to reach requested segment at time %g\n",
	      s->name, start);
      return (FALSE);
   }

   s->start_samp = sam_to_skip;
   check_file_size(s, FALSE);

   if (fseek(strm, (long) s->bytes_skip, 0)) {
      fprintf(stderr, "get_esps_fspec: Can't skip header of ESPS FEA_SPEC file.\n");
      return FALSE;
   }
   fea_skiprec(strm, (long) sam_to_skip, hdr);

   if (!spp) {
      spp = (char **) malloc(dim * sizeof(char *));
      if (!spp) {
	 fprintf(stderr, "get_esps_fspec: Can't allocate data pointer array.\n");
	 return FALSE;
      }
      for (i = 0; i < dim; i++)
	 spp[i] = NULL;
      old_size = 0;
   }
   s->data = (caddr_t) spp;
   if (old_size != s->buff_size) {
      if (istagged) {
	 if (s->x_dat)
	 {
	    free((char *) s->x_dat);
	    s->x_dat = NULL;
	 }
	 if (!(s->x_dat = (double *) malloc(sizeof(double) * s->buff_size))) {
	    fprintf(stderr, "Allocation failure for tag array in get_esps_fspec()\n");
	    return (FALSE);
	 }
      }
      for (i = 0; i < dim; i++) {	/* Allocate an array for each freq. */
	 if (spp[i])
	    free((char *) spp[i]);
	 if (!(spp[i] = (char *) malloc(s->buff_size * sizeof(char)))) {
	    fprintf(stderr, "get_esps_fspec: Can't allocate data array.\n");
	    while (--i >= 0)
	       free((char *) spp[i]);
	    free(s->data);
	    s->data = NULL;
	    s->buff_size = 0;
	    return FALSE;
	 }
      }
   }
   buf = allo_feaspec_rec(hdr, BYTE);
   loc = 0;
   while (loc < s->buff_size) {
      if (get_feaspec_rec(buf, hdr, strm) == EOF)
      {
	 s->buff_size = loc;
	 truncate = 1;
	 for (i = 0; i < dim; i++) /* Reallocate array for each freq. */
	 {
	    char *sp = realloc((char *) spp[i], loc * sizeof(char));

	    if (sp)
	       spp[i] = sp;
	    else
	       break;
	 }
	 if (istagged)
	 {
	    char *sp = realloc((char *) s->x_dat, loc * sizeof(double));

	    if (sp)
	       s->x_dat = (double *) sp;
	 }
      }
      else {
	 if (istagged)
	    s->x_dat[loc] = s->start_time + ((double) (*buf->tag - 1)) / ssf;

	 for (i = 0; i < dim; i++)
	    spp[i][loc] = buf->re_spec_val_b[i];

	 loc++;
      }
   }
   /* Should free buf here, but no ESPS free_feaspec_rec(). */

   check_file_size(s, truncate);

   return TRUE;
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Allocate an ESPS FEA record and an array of pointers for accessing each
 * element of each field.
 */
int
allo_esps_fea(hdr, dim, bufp, ptrp)
   struct header  *hdr;
   int             dim;
   struct fea_data **bufp;
   caddr_t       **ptrp;
{
   *bufp = allo_fea_rec(hdr);
   return (setup_fea_pointers(hdr, dim, *bufp, ptrp));
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
setup_fea_pointers(hdr, dim, buf, ptrp)
   struct header  *hdr;
   int             dim;
   struct fea_data *buf;
   caddr_t       **ptrp;
{
   int             n, j;
   int             i;
   caddr_t        *ptr;

   ptr = (caddr_t *) malloc((unsigned) dim * sizeof(caddr_t));
   if (!ptr) {
      fprintf(stderr, "setup_fea_pointers: Can't allocate element pointer array.\n");
      return FALSE;
   }
   n = 0;
   for (i = 0; i < hdr->hd.fea->field_count; i++) {
      struct fea_header *fea = hdr->hd.fea;

#define	ALLO_R(type, member) { \
	    {	type    *fld = buf->member + fea->starts[i]; \
		for (j = 0; j < fea->sizes[i]; j++) \
		    ptr[n++] = (caddr_t) &fld[j]; } \
	    break; }
#define	ALLO_C(type, member) { \
	    {	type    *fld = buf->member + fea->starts[i]; \
		for (j = 0; j < fea->sizes[i]; j++) { \
		    ptr[n++] = (caddr_t) &fld[j].real; \
		    ptr[n++] = (caddr_t) &fld[j].imag; } } \
	    break; }

      switch (fea->types[i]) {
      case BYTE:
      case CHAR:
	 ALLO_R(char, b_data);
      case SHORT:
      case CODED:
	 ALLO_R(short, s_data);
      case LONG:
	 ALLO_R(long, l_data);
      case FLOAT:
	 ALLO_R(float, f_data);
      case DOUBLE:
	 ALLO_R(double, d_data);
      case BYTE_CPLX:
	 ALLO_C(byte_cplx, bc_data);
      case SHORT_CPLX:
	 ALLO_C(short_cplx, sc_data);
      case LONG_CPLX:
	 ALLO_C(long_cplx, lc_data);
      case FLOAT_CPLX:
	 ALLO_C(float_cplx, fc_data);
      case DOUBLE_CPLX:
	 ALLO_C(double_cplx, dc_data);
      default:
	 printf("%s: unrecognized ESPS data type.\n",
		"allo_esps_fea");
	 free((char *) ptr);
	 return FALSE;
      }
   }
   *ptrp = ptr;
   return TRUE;
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* Storage required for each element of a given Signal type. */

static int
sig_type_size(type)
   int     type;
{
   int     typsize;

   switch (type)
   {
   case P_SHORTS:
   case P_USHORTS:
      typsize = sizeof(short);
      break;
   case P_INTS:
   case P_UINTS:
      typsize = sizeof(int);
      break;
   case P_FLOATS:
      typsize = sizeof(float);
      break;
   case P_DOUBLES:
      typsize = sizeof(double);
      break;
   case P_CHARS:
   case P_UCHARS:
      typsize = sizeof(char);
      break;
   default:
      typsize = 0;
      break;
   }

   return typsize;
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* Read data from ESPS FEA file. */

get_esps_fea(s, start, page_size)
   Signal         *s;
   double          start, page_size;
{
   int             i, j, n, dim;
   int             sam_to_skip, old_size, loc;
   struct fea_data *buf;	/* FEA record */
   caddr_t        *ptr;		/* pointers to field elements in buf */
   caddr_t        *spp = (caddr_t *) s->data;
   FILE           *strm = s->header->strm;
   struct header  *hdr = s->header->esps_hdr;
   int             max_buff_size = max_buff_bytes / intern_size_rec(hdr);
   extern char    *fea_file_type[];
   int             truncate = 0; /* flag for too-short files */

   if (hdr->common.tag)
      return (get_esps_fea_tag(s, start, page_size));

   sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
   dim = s->dim;
   old_size = s->buff_size;
   s->buff_size = (int) (0.5 + (page_size * s->freq));
   if (s->buff_size > max_buff_size) {
      s->buff_size = max_buff_size;
      fprintf(stderr, "%s: read request exceeds max_buff_size; limiting to %f sec.\n",
	      "get_esps_fea", ((double) s->buff_size) / s->freq);
   }
   if (s->data && (old_size == s->buff_size)
       && (sam_to_skip == s->start_samp))
      return TRUE;

   if (s->file_size <= sam_to_skip) {
      fprintf(stderr, "File %s too small to reach requested segment at time %g\n",
	      s->name, start);
      return (FALSE);
   }

   s->start_samp = sam_to_skip;
   check_file_size(s, FALSE);

   if (fseek(strm, (long) s->bytes_skip, 0)) {
      fprintf(stderr, "get_esps_fea: Can't skip header of ESPS FEA file.\n");
      return FALSE;
   }

   fea_skiprec(strm, (long) sam_to_skip, hdr);

   if (!allo_esps_fea(hdr, dim, &buf, &ptr)) {
      fprintf(stderr, "%s: trouble allocating FEA record & pointer array.\n",
	      "get_esps_fea");
      return FALSE;
   }
   if (!spp) {
      spp = (caddr_t *) malloc(dim * sizeof(caddr_t));
      if (!spp) {
	 fprintf(stderr,
		 "get_esps_fea: Can't allocate data pointer array.\n");
	 free((char *) ptr);
	 return FALSE;
      }
      for (i = 0; i < dim; i++)
	 spp[i] = NULL;
      old_size = 0;
   }
   s->data = (caddr_t) spp;
   if (old_size != s->buff_size)
      for (i = 0; i < dim; i++) /* Allocate an array for each element. */
      {
	 int     typsize = sig_type_size(s->types[i]);

	 if (typsize == 0)
	 {
	    fprintf(stderr,
		    "get_esps_fea: Unknown data type %d.\n", s->types[i]);
	    return FALSE;
	    break;
	 }
	 if (spp[i])
	    free((char *) spp[i]);
	 if (!(spp[i] = (caddr_t) malloc((size_t)(s->buff_size * typsize))))
	 {
	    fprintf(stderr, "get_esps_fea: Can't allocate data array.\n");
	    while (--i >= 0)
	       free((char *) spp[i]);
	    free(s->data);
	    s->data = NULL;
	    s->buff_size = 0;
	    free((char *) ptr);
	    return FALSE;
	 }
      }

   loc = 0;
   while (loc < s->buff_size) {
      if (get_fea_rec(buf, hdr, strm) == EOF)
      {
	 s->buff_size = loc;
	 truncate = 1;
	 for (i = 0; i < dim; i++) /* Reallocate array for each element. */
	 {
	    int     typsize = sig_type_size(s->types[i]);
	    char    *sp = realloc((char *) spp[i], (size_t)(loc * typsize));

	    if (sp)
	       spp[i] = (caddr_t) sp;
	    else
	       break;
	 }
      }
      else {
	 for (i = 0; i < dim; i++)
	    switch (s->types[i]) {
	    case P_SHORTS:
	    case P_USHORTS:
	       ((short *) spp[i])[loc] = *(short *) ptr[i];
	       break;
	    case P_INTS:
	    case P_UINTS:
	       ((int *) spp[i])[loc] = *(int *) ptr[i];
	       break;
	    case P_FLOATS:
	       ((float *) spp[i])[loc] = *(float *) ptr[i];
	       break;
	    case P_DOUBLES:
	       ((double *) spp[i])[loc] = *(double *) ptr[i];
	       break;
	    case P_CHARS:
	    case P_UCHARS:
	       ((char *) spp[i])[loc] = *(char *) ptr[i];
	       break;
	    }

	 loc += 1;
      }
   }

   free((char *) ptr);
   free_fea_rec(buf);

   check_file_size(s, truncate);

   return TRUE;
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
double
tag_index_to_time(s, ind)
   Signal         *s;
   int             ind;
{
   if (s && s->x_dat) {
      if (ind < 0)
	 ind = 0;
      if (ind >= s->buff_size)
	 ind = s->buff_size - 1;
      return (s->x_dat[ind]);
   }
   return (0.0);
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
int
tag_time_to_index(s, time)
   Signal         *s;
   double          time;
{
   if (s && s->x_dat && (s->start_time < time)) {
      register int    i = s->buff_size - 1;
      while ((i > 0) && (time < s->x_dat[i]))
	 i--;
      if ((i < s->buff_size - 1) &&
	  ((time - s->x_dat[i]) > (s->x_dat[i + 1] - time)))
	 i++;
      return (i);
   }
   return (0);
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
double
tag_buf_end(s)
   Signal         *s;
{
   if (s && s->x_dat)
      return (s->x_dat[s->buff_size - 1]);
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
double
tag_buf_start(s)
   Signal         *s;
{
   if (s && s->x_dat)
      return (s->x_dat[0]);
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
double
tag_sig_dur(s)
   Signal         *s;
{
   if (s) {
      if (s->end_time > s->start_time)
	 return (s->end_time - s->start_time);
      fprintf(stderr, "tag_sig_dur entered with end_time unset (%s)\n", s->name);
      return (100.0);
   }
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
install_tag_fea_methods(s)
   Signal         *s;
{
   if (s && s->utils) {
      extern int      tag_write_data();

      s->utils->index_to_time = tag_index_to_time;
      s->utils->time_to_index = tag_time_to_index;
      s->utils->buf_end = tag_buf_end;
      s->utils->buf_start = tag_buf_start;
      s->utils->sig_dur = tag_sig_dur;
      s->utils->write_data = tag_write_data;
   }
}

#define D_SAMNO ((double) (buf->tag - very_first_tag))
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* Read data from ESPS TAGGED FEA file. */
get_esps_fea_tag(s, start, page_size)
   Signal         *s;
   double          start, page_size;
{
   int             i, j, n, dim;
   int             old_size, loc, sam_to_skip, dstart, first_tag, very_first_tag;
   struct fea_data *buf;	/* FEA record */
   caddr_t        *ptr;		/* pointers to field elements in buf */
   caddr_t        *spp = (caddr_t *) s->data;
   FILE           *strm = s->header->strm;
   struct header  *hdr = s->header->esps_hdr;
   int             max_buff_size = max_buff_bytes / intern_size_rec(hdr);
   double          get_genhd_val(), ssf;	/* "source file" sample
						 * frequency */

   dim = s->dim;
   old_size = s->buff_size;
   if (!allo_esps_fea(hdr, dim, &buf, &ptr)) {
      fprintf(stderr, "%s: trouble allocating FEA record & pointer array.\n",
	      "get_esps_fea_tag");
      return FALSE;
   }
   ssf = get_genhd_val("src_sf", hdr, -1.0);
   if (ssf < 0.0) {
      fprintf(stderr, "No vald source sampling frequency found in %s; using 8k\n",
	      s->name);
      ssf = 8000.0;
   }
   very_first_tag = get_genhd_val("start", hdr, 0.0);

   if (fseek(strm, (long) s->bytes_skip, 0)) {
      fprintf(stderr, "get_esps_fea_tag: Can't skip header of tagged FEA file.\n");
      return FALSE;
   }
   start -= s->start_time;	/* to simplify arithmetic for frame counting */
   if (start < 0.0)
      start = 0.0;

   /* First, seek to the start of interesting data. */
   sam_to_skip = -1;
   do {
      dstart = ftell(strm);	/* so we can return to start of data */
      if (get_fea_rec(buf, hdr, strm) == EOF) {
	 fprintf(stderr, "Problems seeking to start of data\n");
	 return (FALSE);
      }
      if (!very_first_tag && (sam_to_skip < 0)) {
	 very_first_tag = buf->tag;
	 *add_genhd_l("start", (long *) NULL, 1, hdr) = very_first_tag;
      }
      sam_to_skip++;
   } while ((D_SAMNO / ssf) < start);
   first_tag = buf->tag;
   /*
    * For now take the brute-force approach of seeking once to determine the
    * number of records required, then again to actually get the data...
    */
   fseek(strm, (long) dstart, 0);

   loc = 0;
   do {
      if (get_fea_rec(buf, hdr, strm) == EOF) {
	 if (debug_level)
	    fprintf(stderr, "EOF when counting records in get_esps_fea_tag()\n");
	 s->end_time = s->start_time + (D_SAMNO / ssf);
	 s->file_size = sam_to_skip + loc;
	 break;
      }
      loc++;
   } while ((D_SAMNO / ssf) < (start + page_size));

   if (loc <= max_buff_size)
      s->buff_size= loc;
   else
   {
      s->buff_size = max_buff_size;
      fprintf(stderr,
	      "%s: read request exceeds max_buff_size; limiting to %d recs.\n",
	      "get_esps_fea_tag", s->buff_size);
   }
   if (!s->buff_size) {
      fprintf(stderr,
	      "Problems when counting records in get_esps_fea_tag()\n");
      return (FALSE);
   }

   /* If end_time not set, scan the file to determine last-record's tag. */
   if (s->end_time < s->start_time) {
      while (get_fea_rec(buf, hdr, strm) != EOF)
	 loc++;
      s->end_time = s->start_time + D_SAMNO / ssf;
      s->file_size = sam_to_skip + loc;
   }
   fseek(strm, (long) dstart, 0);
   if (s->data && s->x_dat && (old_size == s->buff_size)
       && (sam_to_skip == s->start_samp))
      return TRUE;

   s->start_samp = sam_to_skip;

   /* Generate an average frame rate in case it's needed somewhere. */
   s->freq = (ssf * (double) (s->start_samp + s->buff_size)
	      / (buf->tag - very_first_tag));

   if (!spp) {
      spp = (caddr_t *) malloc(dim * sizeof(caddr_t));
      if (!spp) {
	 fprintf(stderr, "get_esps_fea: Can't allocate data pointer array.\n");
	 free((char *) ptr);
	 return FALSE;
      }
      for (i = 0; i < dim; i++)
	 spp[i] = NULL;
      old_size = 0;
   }
   s->data = (caddr_t) spp;

   if (old_size < s->buff_size) {
      if (s->x_dat)
	 free(s->x_dat);
      if (!(s->x_dat = (double *) malloc(s->buff_size * sizeof(double)))) {
	 fprintf(stderr, "Can't allocate time array in get_esps_fea_tag()\n");
	 return FALSE;
      }
      for (i = 0; i < dim; i++) /* Allocate an array for each element. */
      {
	 int     typsize = sig_type_size(s->types[i]);

	 if (typsize == 0)
	 {
	    fprintf(stderr,
		    "get_esps_fea_tag: Unknown data type %d.\n", s->types[i]);
	    return FALSE;
	    break;
	 }

	 if (spp[i])
	    free((char *) spp[i]);
	 if (!(spp[i] = (caddr_t) malloc(s->buff_size * typsize))) {
	    fprintf(stderr, "get_esps_fea_tag: Can't allocate data array.\n");
	    while (--i >= 0)
	       free((char *) spp[i]);
	    free(s->data);
	    s->data = NULL;
	    s->buff_size = 0;
	    free((char *) ptr);
	    return FALSE;
	 }
      }
   }
   loc = 0;
   while (loc < s->buff_size) {
      if (get_fea_rec(buf, hdr, strm) == EOF) {
	 fprintf(stderr, "Weird error in get_fea_rec in get_esps_fea_tag\n");
	 return (FALSE);
      } else {
	 s->x_dat[loc] = s->start_time + (D_SAMNO / ssf);
	 for (i = 0; i < dim; i++)
	    switch (s->types[i]) {
	    case P_SHORTS:
	    case P_USHORTS:
	       ((short *) spp[i])[loc] = *(short *) ptr[i];
	       break;
	    case P_INTS:
	    case P_UINTS:
	       ((int *) spp[i])[loc] = *(int *) ptr[i];
	       break;
	    case P_FLOATS:
	       ((float *) spp[i])[loc] = *(float *) ptr[i];
	       break;
	    case P_DOUBLES:
	       ((double *) spp[i])[loc] = *(double *) ptr[i];
	       break;
	    case P_CHARS:
	    case P_UCHARS:
	       ((char *) spp[i])[loc] = *(char *) ptr[i];
	       break;
	    }
	 loc++;
      }
   }
   free((char *) ptr);
   free_fea_rec(buf);
   return TRUE;
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
/*
 * Read ascii signals from a file into double arrays.  Convert from double to
 * the desired internal type.  This routine assumes all vector elements are
 * on one line ane all lines are terminated with '\n'.  A maximum of
 * A_BUF_SIZE-1 characters are permitted per line (i.e. per sample).
 */
get_ascii(s, start, page_size)
   Signal         *s;
   double          start, page_size;
{
#define A_BUF_SIZE 8000
   FILE           *fp, *fdopen();
   char            bufin[A_BUF_SIZE], *cp, *get_next_item();
   register int    i, j, dim;
   double        **spp;
   int             sam_to_skip, old_size;
   int             max_buff_size = max_buff_bytes / (s->dim * sizeof(double));

   sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
   dim = s->dim;
   old_size = s->buff_size;
   s->buff_size = (int) (0.5 + (page_size * s->freq));
   if (s->buff_size > max_buff_size) {
      s->buff_size = max_buff_size;
      fprintf(stderr, "%s: read request exceeds max_buff_size; limiting to %f sec.\n",
	      "get_ascii", ((double) s->buff_size) / s->freq);
      /*
       * fprintf(stderr,"get_ascii: large file -- using buff size
       * %d\n",s->buff_size);
       */
   }
   if (s->data && (old_size == s->buff_size) &&
       (sam_to_skip == s->start_samp))
      return (TRUE);
   s->start_samp = sam_to_skip;
   fp = fdopen(s->file, "r");
   if (!fseek(fp, s->bytes_skip, 0)) {
      while (sam_to_skip) {
	 if (!fgets(bufin, A_BUF_SIZE, fp)) {
	    fprintf(stderr, "Error while seeking in ascii file in get_ascii()\n");
	    fclose(fp);
	    close_sig_file(s);
	    return (FALSE);
	 }
	 sam_to_skip--;
      }
      if (!(spp = (double **) s->data)) {
	 spp = (double **) malloc(sizeof(double *) * dim);
	 for (i = 0; i < dim; i++)
	    spp[i] = NULL;
	 old_size = 0;
      }
      if (spp) {
	 s->data = (caddr_t) spp;
	 for (i = 0; i < dim; i++)	/* allocate an array for each dim. */
	    if (old_size != s->buff_size) {
	       if (spp[i])
		  free(spp[i]);
	       if (!(spp[i] = (double *) malloc(s->buff_size * sizeof(double)))) {
		  fprintf(stderr, "Can't allocate data array in get_ascii()\n");
		  while (--i >= 0)
		     free(spp[i]);
		  free(spp);
		  fclose(fp);
		  close_sig_file(s);
		  return (FALSE);
	       }
	    }
	 for (j = 0; j < s->buff_size; j++) {
	    if (fgets(bufin, A_BUF_SIZE, fp)) {
	       cp = bufin;
	       for (i = 0; i < dim; i++) {
		  if (!sscanf(cp, "%lf", &(spp[i][j])))
		     spp[i][j] = 0.0;
		  cp = get_next_item(cp);
	       }
	    } else {
	       if (j) {
		  s->buff_size = j;	/* also forces loop termination */
		  check_file_size(s, TRUE);
	       } else {
		  fprintf(stderr, "Error while reading ascii input\n");
		  fclose(fp);
		  close_sig_file(s);
		  return (FALSE);
	       }
	    }
	 }
	 fclose(fp);
	 close_sig_file(s);
	 if ((s->type & VECTOR_SIGNALS) != P_DOUBLES) {
	    int             type;
	    Signal         *s2, *convert();

	    type = s->type;
	    s->type = P_DOUBLES;
	    if (s2 = (Signal *) convert(s, type)) {
	       spp = (double **) s2->data;
	       s2->data = s->data;
	       s->data = (caddr_t) spp;
	       s->type = type;
	       free_signal(s2);
	    } else {
	       fprintf(stderr, "Couldn't convert from doubles to type %d in get_ascii()\n",
		       type);
	       return (FALSE);
	    }
	 }
	 check_file_size(s, FALSE);
	 return (TRUE);
      } else
	 fprintf(stderr, "Can't allocate buffer pointer array in get_ascii()\n");
   } else
      fprintf(stderr, "Can't fseek to data start in get_ascii()\n");
   fclose(fp);
   close_sig_file(s);
   return (FALSE);
}


/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
void
check_file_size(s, flag)
   Signal         *s;
   int             flag;	/* if non-zero, allow new file_size */
{
   if (!s)
      return;

   if (flag && ((s->start_samp + s->buff_size) < s->file_size)) {
      s->file_size = s->start_samp + s->buff_size;
      fprintf(stderr, "Adjusting file size of %s to %d samples\n",
	      s->name, s->file_size);
   }
   /* adjust end time if necessary */
   if ((int) s->freq &&
       ((s->start_time + ((double) s->file_size) / s->freq) != s->end_time))
      s->end_time = s->start_time + ((double) s->file_size) / s->freq;
}

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#define READIT(type) { \
    type *data, **spp, *buf, *p, *q; \
    if(!(spp = (type**)s->data)) { \
      spp = (type**)malloc(sizeof(type*) * dim); \
      for(i=0; i < dim; i++) spp[i] = NULL; \
      old_size = 0; \
    } \
    if(spp) { \
      s->data = (caddr_t)spp; \
      for(i=0; i < dim; i++)	/* allocate an array for each dim. */ \
	if(old_size != s->buff_size) { \
	  if(spp[i]) free(spp[i]); \
	  if(!(spp[i] = (type*)malloc(s->buff_size * sizof))) { \
	    fprintf(stderr,"Can't allocate data array in get_binary()\n"); \
	    while(--i >= 0) free(spp[i]); \
	    free(spp); \
	    s->data = NULL; \
	    return(FALSE); \
	  } \
	} \
      if(dim > 1) { \
	if(!(buf = (type*)malloc(buf_bytes))) { \
	  free(spp); \
	  fprintf(stderr,"Can't allocate data buffer in get_binary()\n"); \
	  return(FALSE); \
	} \
	loc = 0; \
	do { \
	  int bytes_left = bytes_per_sam * (s->buff_size - loc); \
	  if (buf_bytes > bytes_left) buf_bytes = bytes_left; \
	  if((nread = read(s->file,buf,buf_bytes)) >= bytes_per_sam) { \
	    nsread = nread/bytes_per_sam; \
	    for(i=0; i < dim; i++) \
	      for(j=0, p = spp[i] + loc, q = buf + i; j++ < nsread; \
		  q += dim) *p++ = *q; \
	    loc += nsread; \
	  } \
	  if(nread != buf_bytes) { \
	    s->buff_size = loc; \
	    truncate = 1; \
	    break; \
	  } \
	} while(loc < s->buff_size); \
	free(buf); \
      } else {			/* one dimensional binary data */ \
	if((nread = read(s->file,(char*)spp[0],nbytes)) != nbytes) { \
	  truncate = 1; \
	  if(! (s->buff_size = nread/sizof)) { \
	    fprintf(stderr,"Unable to read any data in get_binary()\n"); \
	    free(spp[0]); \
	    free(spp); \
	    s->data = NULL; \
	    return(FALSE); \
	  } \
	} \
      } \
      s->start_samp = sam_to_skip; \
      check_file_size(s,truncate); \
      return(TRUE); \
    } else \
      fprintf(stderr,"Can't allocate data pointer array in get_binary()\n"); \
  }

/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
get_binary(s, start, page_size, sizof)
   Signal         *s;
   double          start, page_size;
   int             sizof;
{
   register int    i, j, dim, nsread, bsize = 500;
   int             sam_to_skip, nbytes, old_size, bytes_per_sam, seek, nelem,
                   loc, buf_bytes, nread;
   /* max buffer size per channel */
   int             max_buff_size = max_buff_bytes / (s->dim * sizof);
   int             truncate = 0;/* flag for too-short files */

   sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
   dim = s->dim;
   old_size = s->buff_size;
   s->buff_size = (int) (0.5 + (page_size * s->freq));
   if (s->buff_size > max_buff_size) {
      s->buff_size = max_buff_size;
      fprintf(stderr, "%s: read request exceeds max_buff_size; limiting to %f sec.\n",
	      "get_binary", ((double) s->buff_size) / s->freq);
   }
   if (s->data && (old_size == s->buff_size) &&
       (sam_to_skip == s->start_samp))
      return (TRUE);
   bytes_per_sam = dim * sizof;
   seek = s->bytes_skip + (sam_to_skip * bytes_per_sam);
   if (s->file_size < (sam_to_skip + s->buff_size)) {	/* is the file really
							 * that small? */
#ifndef OS5
      s->file_size = (lseek(s->file, 0, L_XTND) - s->bytes_skip) / bytes_per_sam;
#else
      s->file_size = (lseek(s->file, 0, SEEK_END) - s->bytes_skip) / bytes_per_sam;
#endif
      if (s->file_size <= sam_to_skip) {
	 fprintf(stderr, "File %s too small to reach requested segment at time %g\n",
		 s->name, start);
	 return (FALSE);
      }
   }
   /*
    * I hate this end_time kluge;  excellent example of overspecification! dt
    */
   check_file_size(s, FALSE);

#ifndef OS5
   if (lseek(s->file, seek, L_SET) == seek) {
#else
   if (lseek(s->file, seek, SEEK_SET) == seek) {
#endif
      nelem = dim * s->buff_size;
      nbytes = sizof * nelem;
      buf_bytes = bytes_per_sam * bsize;
      if (buf_bytes > nbytes)
	 buf_bytes = nbytes;
      switch (sizof) {
      case sizeof(char):
	 READIT(unsigned char);
	 break;
      case sizeof(short):
	 READIT(short);
	 break;
      case sizeof(int):
	 READIT(int);
	 break;
#ifdef FLOATS_AND_LONGS_DIFFER
      case sizeof(float):
	 READIT(float);
	 break;
#endif
      case sizeof(double):
	 READIT(double);
	 break;
      default:
	 fprintf(stderr, "Unknown data type (size) in get_binary()\n");
	 break;
      }
   } else
      fprintf(stderr, "Can't seek to data segment start in get_binary()\n");
   return (FALSE);
}
