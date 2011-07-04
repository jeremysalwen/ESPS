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
 * Data output methods for commonly encountered data types
 */

static char *sccs_id = "@(#)write_data.c	1.19	1/18/97	ATT/ESI/ERL";

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

void            put_sd_recs();
void            free_signal();
void            free_fea_rec();
int             allo_esps_fea();/* defined in read_data.c */

extern int      debug_level;



write_data(s, start, page_size)
   Signal         *s;
   double          start, page_size;
{
   int             fd;
   double          fudge;
   int             typsize;

#ifdef DEMO
   return FALSE;
#endif

   if (s && s->data) {
      if (debug_level >= 1)
	 printf("write_data: s->name = \"%s\".  s->file = %d.\n",
		s->name, s->file);
      if ((fd = s->file) < 0) {
	 if (!((s->name)
	 && ((s->file = fd = open(s->name, O_RDWR | O_CREAT, 0664)) >= 0))) {
	    printf("write_data(): Can't open the output data file or file doesn't exist\n");
	    return (FALSE);
	 }
	 if (s->header && s->header->magic == ESPS_MAGIC) {
	    s->header->strm = fdopen(fd, "r+");

	    /* ! *//* Needed after get_ and put_esps_* fixed up? */
	    if (fseek(s->header->strm, (long) s->bytes_skip, 0)) {
	       printf("write_data: Can't skip header of ESPS file.\n");
	       return FALSE;
	    }
	 }
      }
      if (s->type & SPECIAL_SIGNALS)	/* trap special signals here */
	 switch (s->type & SPECIAL_SIGNALS) {
	 case SIG_SPECTROGRAM:
	    if (s->header && s->header->magic == ESPS_MAGIC)
	       switch (s->header->esps_hdr->common.type) {
	       case FT_FEA:
		  switch (s->header->esps_hdr->hd.fea->fea_type) {
		  case FEA_SPEC:
		     return put_esps_fspec(s, start, page_size);
		     break;
		  default:
		     printf("%s: wrong FEA subtype for spectrogram.\n",
			    "write_data");
		     return FALSE;
		     break;
		  }
		  break;
	       case FT_SPEC:
	       default:
		  printf("%s: wrong ESPS file type for spectrogram.\n",
			 "write_data");
		  return FALSE;
		  break;
	       }
	    break;
	 default:		/* fall through to standard handling */
	    break;
	 }
      fudge = 0.5 / s->freq;
      if ((BUF_START_TIME(s) > (fudge + start)) || (page_size < fudge) ||
	  ((page_size + start - fudge) > BUF_END_TIME(s))) {
	 printf("Bad start time or page size in write_data()\n");
	 printf("wd: START:%f END:%f; start:%f page_size:%f\n", BUF_START_TIME(s),
		BUF_END_TIME(s), start, page_size);
	 return (FALSE);
      }
      if (IS_GENERIC(s->type)) {

	 if ((s->type & SIG_FORMAT) == SIG_ASCII)
	    return (put_ascii(s, start, page_size));

	 switch (s->type & VECTOR_SIGNALS) {
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
	       printf("%s: data type P_MIXED but not ESPS FEA file.\n",
		      "write_data");
	       return FALSE;
	    }
	    break;
	 default:
	    printf("Unknown data type in write_data()\n");
	    return FALSE;
	    break;
	 }

	 if (s->header && s->header->magic == ESPS_MAGIC) {
	    switch (s->header->esps_hdr->common.type) {
	    case FT_SD:
	       return put_esps_sd(s, start, page_size, typsize);
	    case FT_FEA:
	       if (s->header->e_scrsd)
		  /* for FEA_SD, single chan, non-complex, short data */
		  return put_esps_sd(s, start, page_size, typsize);
               else if (s->header->esps_hdr->hd.fea->fea_type == FEA_SD && 
                        s->header->e_short) {
                  /* for FEA_SD, multi-channel, non-complex, short */
                  return put_esps_feasd(s, start, page_size);
               }
	       else
		  /* all other FEA files */
		  return put_esps_fea(s, start, page_size);
	       break;
	    default:
	       printf("write_data: Called on unsupported ESPS file type.\n");
	       return FALSE;
	       break;
	    }
	 }
	 return (put_binary(s, start, page_size, typsize));

      } else
	 printf("Unknown data type in write_data(%d)\n", s->type);
   } else
      printf("Bad Signal structure in write_data()\n");
   return (FALSE);
}
/****************************************************************/
put_esps_feasd(s, start, page_size)
   Signal         *s;
   double          start, page_size;
{
   int             i, j, n, dim;
   int             sam_to_skip, nsamp;
   int             loc;
   struct feasd   *feasd_rec;	/* FEA record */
   caddr_t        *spp = (caddr_t *) s->data;
   FILE           *strm = s->header->strm;
   struct header  *hdr = s->header->esps_hdr;
   struct feasd   *allo_feasd_recs();
   short	  **s_ptr;

   sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
   dim = s->dim;
   loc = sam_to_skip - s->start_samp;
   nsamp = (int) (0.5 + page_size * s->freq);

   if (fseek(strm, (long) s->bytes_skip, 0)) {
      printf("write_data: Can't skip header of ESPS file.\n");
      return FALSE;
   }
   fea_skiprec(strm, (long) sam_to_skip, hdr);

   if (!(feasd_rec = allo_feasd_recs(hdr, SHORT, nsamp, NULL, YES))) {
	fprintf(stderr,"failed to allocate feasd rec in write_data!\n");
	return FALSE;
   }

   s_ptr= (short **)feasd_rec->ptrs;
   for (i = 0; i < nsamp; i++, loc++) {
      for (j = 0; j < dim; j++) {
	    s_ptr[i][j] = ((short *) spp[j])[loc];
      }
   }

   put_feasd_recs(feasd_rec, 0L, nsamp, hdr, strm);
   free(feasd_rec->data);
   free(feasd_rec->ptrs);
   free(feasd_rec);
   return TRUE;
}

/****************************************************************/
put_esps_fea(s, start, page_size)
   Signal         *s;
   double          start, page_size;
{
   int             i, j, n, dim;
   int             sam_to_skip, nsamp;
   int             loc;
   struct fea_data *buf;	/* FEA record */
   caddr_t        *ptr;		/* pointers to field elements in buf */
   caddr_t        *spp = (caddr_t *) s->data;
   FILE           *strm = s->header->strm;
   struct header  *hdr = s->header->esps_hdr;

   sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
   dim = s->dim;
   loc = sam_to_skip - s->start_samp;
   nsamp = (int) (0.5 + page_size * s->freq);

   if (fseek(strm, (long) s->bytes_skip, 0)) {
      printf("write_data: Can't skip header of ESPS file.\n");
      return FALSE;
   }
   fea_skiprec(strm, (long) sam_to_skip, hdr);

   if (!allo_esps_fea(hdr, dim, &buf, &ptr)) {
      printf("%s: trouble allocating FEA record & pointer array.\n",
	     "put_esps_fea");
      return FALSE;
   }
   for (i = 0; i < nsamp; i++, loc++) {
      int             type;
      for (j = 0; j < dim; j++) {
	 if (s->type != P_MIXED)
	    type = s->type;
	 else
	    type = s->types[j];
	 switch (type) {
	 case P_UCHARS:
	 case P_CHARS:
	    *(char *) ptr[j] = ((char *) spp[j])[loc];
	    break;
	 case P_USHORTS:
	 case P_SHORTS:
	    *(short *) ptr[j] = ((short *) spp[j])[loc];
	    break;
	 case P_UINTS:
	 case P_INTS:
	    *(int *) ptr[j] = ((int *) spp[j])[loc];
	    break;
	 case P_FLOATS:
	    *(float *) ptr[j] = ((float *) spp[j])[loc];
	    break;
	 case P_DOUBLES:
	    *(double *) ptr[j] = ((double *) spp[j])[loc];
	    break;
	 default:
	    printf("%s: unrecognized Waves+ data type.\n",
		   "read_esps_hdr");
	    free((char *) ptr);
	    return FALSE;
	 }
      }

      put_fea_rec(buf, hdr, strm);
   }

   free((char *) ptr);
   free_fea_rec(buf);
   return TRUE;
}
/****************************************************************/

put_esps_sd(s, start, page_size, typsize)
   Signal         *s;
   double          start, page_size;
   int             typsize;
{
   int             sam_to_skip, loc, nelem;
   struct header  *hdr = s->header->esps_hdr;

   sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
   loc = sam_to_skip - s->start_samp;
   nelem = (int) (0.5 + page_size * s->freq);

   if (fseek(s->header->strm, (long) s->bytes_skip, 0)) {
      printf("write_data: Can't skip header of ESPS file.\n");
      return FALSE;
   }
   fea_skiprec(s->header->strm, (long) sam_to_skip, hdr);
   switch (s->type) {
   case P_SHORTS:
      {
	 short         **spp;

	 spp = (short **) s->data;
	 put_sd_recs(spp[0] + loc, nelem,
		     s->header->esps_hdr, s->header->strm);
      }
      break;
   default:
      printf("write_data: SD file; Signal type not P_SHORT.\n");
      return FALSE;
      break;
   }

   return TRUE;
}

/****************************************************************/

put_esps_fspec(s, start, page_size)
   Signal         *s;
   double          start, page_size;
{
   int             i, j, dim;
   int             sam_to_skip, nsamp;
   int             loc, istagged = IS_TAGGED_FEA(s);
   struct feaspec *buf;
   char          **spp = (char **) s->data;
   FILE           *strm = s->header->strm;
   struct header  *hdr = s->header->esps_hdr;
   double          ssf, get_genhd_val();

   sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
   if ((ssf = get_genhd_val("src_sf", hdr,
			    get_genhd_val("sf", hdr, -1.0))) <= 0.0) {
      fprintf(stderr,
	      "Warning: src_sf not present in tagged file; guessing 8kHz\n");
      ssf = 8000.0;
   }
   dim = s->dim;
   loc = sam_to_skip - s->start_samp;
   nsamp = (int) (0.5 + page_size * s->freq);

   if (fseek(strm, (long) s->bytes_skip, 0)) {
      printf("write_data: Can't skip header of ESPS file.\n");
      return FALSE;
   }
   fea_skiprec(strm, (long) sam_to_skip, hdr);

   buf = allo_feaspec_rec(hdr, BYTE);

   for (i = 0; i < nsamp; i++, loc++) {
      for (j = 0; j < dim; j++)
	 buf->re_spec_val_b[j] = spp[j][loc];
      if (istagged && s->x_dat)
	 *buf->tag = (int) (1.5 + ((s->x_dat[loc] - s->start_time) * ssf));
      put_feaspec_rec(buf, hdr, strm);
   }

   return TRUE;
}

/****************************************************************/

put_binary(s, start, page_size, sizof)
   Signal         *s;
   double          start, page_size;
   int             sizof;
{
   register int    i, j, dim, nlim, bsize = 500;
   int             sam_to_skip, nbytes, bytes_per_sam, seek, nelem, loc,
                   buf_bytes, nwrit, samples;

   sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
   dim = s->dim;
   bytes_per_sam = dim * sizof;
   seek = s->bytes_skip + (sam_to_skip * sizof * dim);
   if (lseek(s->file, seek, L_SET) == seek) {
      nelem = dim * (samples = (int) (0.5 + (page_size * s->freq)));
      nbytes = sizof * nelem;
      buf_bytes = bytes_per_sam * bsize;

      switch (sizof) {
      case sizeof(char):
	 {
	    char          **spp, *buf, *p, *q;

	    spp = (char **) s->data;
	    loc = sam_to_skip - s->start_samp;
	    if (dim > 1) {
	       if ((buf = (char *) malloc(buf_bytes))) {
		  do {
		     if ((loc + bsize) <= s->buff_size)
			nlim = bsize;
		     else
			nlim = s->buff_size - loc;
		     buf_bytes = nlim * bytes_per_sam;
		     for (i = 0; i < dim; i++)
			for (j = 0, p = spp[i] + loc, q = buf + i; j++ < nlim;
			     q += dim)
			   *q = *p++;
		     loc += nlim;
		     if ((nwrit = write(s->file, buf, buf_bytes)) != buf_bytes) {
			printf("error while writing data in write_data()\n");
			return (FALSE);
		     }
		  } while (loc < s->buff_size);
		  free(buf);
		  return (TRUE);
	       } else
		  printf("Can't allocate scratch buffer in write_data()\n");
	    } else {		/* one dimensional chars */
	       if ((nwrit = write(s->file, spp[0] + loc, nbytes)) == nbytes)
		  return (TRUE);
	       else
		  printf("Error during write (%d) in write_data()\n", nwrit);
	    }
	 }
	 break;
      case sizeof(short):
	 {
	    short         **spp, *buf, *p, *q;

	    spp = (short **) s->data;
	    loc = sam_to_skip - s->start_samp;
	    if (dim > 1) {
	       if ((buf = (short *) malloc(buf_bytes))) {
		  do {
		     if ((loc + bsize) <= s->buff_size)
			nlim = bsize;
		     else
			nlim = s->buff_size - loc;
		     buf_bytes = nlim * bytes_per_sam;
		     for (i = 0; i < dim; i++)
			for (j = 0, p = spp[i] + loc, q = buf + i; j++ < nlim;
			     q += dim)
			   *q = *p++;
		     loc += nlim;
		     if ((nwrit = write(s->file, buf, buf_bytes)) != buf_bytes) {
			printf("error while writing data in write_data()\n");
			return (FALSE);
		     }
		  } while (loc < s->buff_size);
		  free(buf);
		  return (TRUE);
	       } else
		  printf("Can't allocate scratch buffer in write_data()\n");
	    } else {		/* one dimensional shorts */
	       if ((nwrit = write(s->file, spp[0] + loc, nbytes)) == nbytes)
		  return (TRUE);
	       else
		  printf("Error during write (%d) in write_data()\n", nwrit);
	    }
	 }
	 break;
      case sizeof(int):
	 {
	    register int  **spp, *buf, *p, *q;

	    spp = (int **) s->data;
	    loc = sam_to_skip - s->start_samp;
	    if (dim > 1) {
	       if ((buf = (int *) malloc(buf_bytes))) {
		  do {
		     if ((loc + bsize) <= s->buff_size)
			nlim = bsize;
		     else
			nlim = s->buff_size - loc;
		     buf_bytes = nlim * bytes_per_sam;
		     for (i = 0; i < dim; i++)
			for (j = 0, p = spp[i] + loc, q = buf + i; j++ < nlim;
			     q += dim)
			   *q = *p++;
		     loc += nlim;
		     if ((nwrit = write(s->file, buf, buf_bytes)) != buf_bytes) {
			printf("error while writing data in write_data()\n");
			return (FALSE);
		     }
		  } while (loc < s->buff_size);
		  free(buf);
		  return (TRUE);
	       } else
		  printf("Can't allocate scratch buffer in write_data()\n");
	    } else {		/* one dimensional ints */
	       if ((nwrit = write(s->file, spp[0] + loc, nbytes)) == nbytes)
		  return (TRUE);
	       else
		  printf("Error during write (%d) in write_data()\n", nwrit);
	    }
	 }
	 break;
#ifdef FLOATS_AND_INTS_DIFFER
      case sizeof(float):
	 {
	    float         **spp, *buf, *p, *q;

	    spp = (float **) s->data;
	    loc = sam_to_skip - s->start_samp;
	    if (dim > 1) {
	       if ((buf = (float *) malloc(buf_bytes))) {
		  do {
		     if ((loc + bsize) <= s->buff_size)
			nlim = bsize;
		     else
			nlim = s->buff_size - loc;
		     buf_bytes = nlim * bytes_per_sam;
		     for (i = 0; i < dim; i++)
			for (j = 0, p = spp[i] + loc, q = buf + i; j++ < nlim;
			     q += dim)
			   *q = *p++;
		     loc += nlim;
		     if ((nwrit = write(s->file, buf, buf_bytes)) != buf_bytes) {
			printf("error while writing data in write_data()\n");
			return (FALSE);
		     }
		  } while (loc < s->buff_size);
		  free(buf);
		  return (TRUE);
	       } else
		  printf("Can't allocate scratch buffer in write_data()\n");
	    } else {		/* one dimensional floats */
	       if ((nwrit = write(s->file, spp[0] + loc, nbytes)) == nbytes)
		  return (TRUE);
	       else
		  printf("Error during write (%d) in write_data()\n", nwrit);
	    }
	 }
	 break;
#endif
      case sizeof(double):
	 {
	    double        **spp, *buf, *p, *q;

	    spp = (double **) s->data;
	    loc = sam_to_skip - s->start_samp;
	    if (dim > 1) {
	       if ((buf = (double *) malloc(buf_bytes))) {
		  do {
		     if ((loc + bsize) <= s->buff_size)
			nlim = bsize;
		     else
			nlim = s->buff_size - loc;
		     buf_bytes = nlim * bytes_per_sam;
		     for (i = 0; i < dim; i++)
			for (j = 0, p = spp[i] + loc, q = buf + i; j++ < nlim;
			     q += dim)
			   *q = *p++;
		     loc += nlim;
		     if ((nwrit = write(s->file, buf, buf_bytes)) != buf_bytes) {
			printf("error while writing data in write_data()\n");
			return (FALSE);
		     }
		  } while (loc < s->buff_size);
		  free(buf);
		  return (TRUE);
	       } else
		  printf("Can't allocate scratch buffer in write_data()\n");
	    } else {		/* one dimensional doubles */
	       if ((nwrit = write(s->file, spp[0] + loc, nbytes)) == nbytes)
		  return (TRUE);
	       else
		  printf("Error during write (%d) in write_data()\n", nwrit);
	    }
	 }
	 break;
      default:
	 printf("Unknown data type (size) in put_binary(%d)\n", sizof);
	 break;
      }
   } else
      printf("Failure in seek before write in write_data()\n");
   return (FALSE);
}

/****************************************************************/

put_ascii(si, start, page_size)
   Signal         *si;
   double          start, page_size;
{
   Signal         *s, *convert();
   FILE           *fp, *fdopen();
   register int    i, j, dim, nlim;
   int             sam_to_skip, bytes_per_sam, seek, loc, samples, its_ok;
   double        **spp;

   its_ok = FALSE;
   if (si) {
      /*
       * bytes_per_sam can be set to the actual number of bytes required per
       * sample provided that a fixed format is agreed upon.  For now we'll
       * keep seven significant figures and output things in scientific
       * notation.
       */

      bytes_per_sam = (si->dim * 14) + 1;
      /* (assumes "%13.6e " for each element, and one "\n" per sample) */

      if ((s = convert(si, P_DOUBLES))) {
	 s->file = si->file;
	 s->bytes_skip = si->bytes_skip;
	 sam_to_skip = ((start - s->start_time) * s->freq) + 0.5;
	 dim = s->dim;
	 seek = s->bytes_skip + (sam_to_skip * bytes_per_sam);
	 if ((fp = fdopen(s->file, "r+"))) {
	    if (!fseek(fp, seek, 0)) {
	       samples = 0.5 + (page_size * s->freq);

	       spp = (double **) s->data;
	       loc = sam_to_skip - s->start_samp;
	       nlim = samples + loc;
	       its_ok = TRUE;
	       for (j = loc; j < nlim; j++) {
		  for (i = 0; i < dim; i++) {
		     if (fprintf(fp, "%13.6e ", spp[i][j]) == EOF)
			its_ok = FALSE;
		  }
		  if (fprintf(fp, "\n") == EOF)
		     its_ok = FALSE;
	       }
	       if (!its_ok)
		  printf("Problems during write in put_ascii()\n");
	       fclose(fp);
	       close_sig_file(s);
	    } else
	       printf("Can't seek before writing data in put_ascii()\n");
	 } else
	    printf("Can't open file %s for formatted output in put_ascii()\n", s->name);
      } else
	 printf("Can't convert signal to doubles for put_ascii\n");
      if (s && (s != si)) {
	 s->file = SIG_UNKNOWN;
	 free_signal(s);
      }
   } else
      printf("Null signal pointer passed to put_ascii()\n");
   return (its_ok);
}
