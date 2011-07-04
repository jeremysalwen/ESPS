/* Copyright (c) 1995 Entropic Research Laboratory, Inc. */
/*	Copyright (c) 1993 */
/*	  and Entropic Research Laboratory.	*/
/*	  All Rights Reserved.	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF	*/
/*	    ENTROPIC RESEARCH LABORATORY, INC.				*/
/*	The copyright notice above does not evidence any	*/
/*	actual or intended publication of such source code.	*/

/* tag_write_data.c */
/* Data output methods for ESPS tagged file types */

#ifndef lint
static char *sccs_id = "@(#)tag_writer.c	1.6	9/26/95	ATT/ERL";
#endif

#include <sys/file.h>
#include <esps/esps.h>
#include <esps/fea.h>
#include <esps/feaspec.h>
#include <Objects.h>

void	    free_signal();
void	    free_fea_rec();
int	    allo_esps_fea();	/* defined in read_data.c */

extern int  debug_level;


tag_write_data(s, start, page_size)
    Signal  *s;
    double  start, page_size;
{
  int	    fd;
  double  fudge, ssf;
  int	    typsize;

  if(s && s->data && s->header) {
    struct header *hdr = s->header->esps_hdr;

    if (debug_level >= 1)
      fprintf(stderr,"tag_write_data: s->name = \"%s\".  s->file = %d.\n",
	      s->name, s->file);

    if(!(hdr && (s->header->magic == ESPS_MAGIC) && hdr->hd.fea &&
	 (hdr->common.tag))) {
      fprintf(stderr,"Incorrect header type passed to tag_write_data(%x)\n",
	      s->header->magic);
      return(FALSE);
    }
    if((fd = s->file) < 0) {
      if(!((s->name)
	   && ((s->file = fd = open(s->name,O_RDWR|O_CREAT,0664)) >= 0))) {
	fprintf(stderr,"tag_write_data(): Can't open the output data file or file doesn't exist\n");
	return(FALSE);
      }
      if (s->header && s->header->magic == ESPS_MAGIC) {
	s->header->strm = fdopen(fd, "r+");

	if (fseek(s->header->strm, (long) s->bytes_skip, 0)) {
	  fprintf(stderr,"tag_write_data: Can't skip header of ESPS file.\n");
	  return FALSE;
	}
      }
    }
    
    ssf = get_genhd_val("src_sf",hdr,-1.0);
    if(ssf < 0.0) {
      fprintf(stderr,
	      "No vald source sampling frequency found in %s; using 8k\n",
	      s->name);
      ssf = 8000.0;
    }

    if(s->x_dat && (s->buff_size > 1))
      fudge = s->x_dat[1] - s->x_dat[0];
    else
      fudge = 1.0/ssf;

    if((BUF_START_TIME(s) > (fudge+start)) || (page_size < fudge) ||
       ((page_size+start-fudge) > BUF_END_TIME(s))) {
      fprintf(stderr,"Bad start time or page size in tag_write_data()\n");
      fprintf(stderr,"wd: START:%f END:%f; start:%f page_size:%f\n",BUF_START_TIME(s),
	      BUF_END_TIME(s), start, page_size);
      return(FALSE);
    }
    if(BUF_START_TIME(s) > start)
      start = BUF_START_TIME(s);
    
    return(put_esps_fea_tag(s, start, page_size, ssf));

  } else
    fprintf(stderr,"Bad header or Signal structure in tag_write_data()\n");
  return(FALSE);
}

/****************************************************************/
put_esps_fea_tag(s, start, page_size, ssf)
    Signal  *s;
    double  start, page_size, ssf;
{
  int     i, j, n, dim, tag, dstart, tag_start;
  int     sam_to_skip, nsamp;
  int     loc;
  struct fea_data *buf;		/* FEA record */
  caddr_t 	    *ptr = NULL; /* pointers to field elements in buf */
  caddr_t    	    *spp = (caddr_t *) s->data;
  FILE    	    *strm = s->header->strm;
  struct header   *hdr = s->header->esps_hdr;

  dim = s->dim;
  nsamp = 0;
  i = 0;

  tag_start = get_genhd_val("start",hdr,0.0);

  while((i < s->buff_size) && (s->x_dat[i] < start)) i++;
  if(i < s->buff_size) {
    while((i < s->buff_size) && (s->x_dat[i] <= (start+page_size))) {
      nsamp++;
      i++;
    }
    if(i >= s->buff_size) {
      page_size = s->x_dat[s->buff_size - 1] - *s->x_dat;
      if(debug_level)
	fprintf(stderr,"Limited page size to %f in put_esps_fea_tag()\n",
		page_size);
    }
  } else {
    fprintf(stderr, "Bad start time (%f) passed to put_esps_fea_tag\n",start);
    return(FALSE);
  }
  
  if (fseek(strm, (long) s->bytes_skip, 0)) {
    printf("write_data: Can't skip header of ESPS file.\n");
    return FALSE;
  }

  if (!allo_esps_fea(hdr, dim, &buf, &ptr)) {
    fprintf(stderr,"%s: trouble allocating FEA record & pointer array.\n",
	    "put_esps_fea_tag");
    return FALSE;
  }

  /* NOTE: this brute force approach costs as the file size! */
  sam_to_skip = -1;
  do {		/* Seek from file start to correct pos. in file */
    dstart = ftell(strm);
    sam_to_skip++;
  } while(((i = get_fea_rec(buf,hdr,strm)) != EOF) &&
    ((s->start_time + (((double)((buf->tag - tag_start)))/ssf)) < start));

  if((i == EOF) && debug_level)
    fprintf(stderr,"EOF when seeking to start of output data\n");

  if((loc = sam_to_skip - s->start_samp) < 0) loc = 0;
  
  fseek(strm,dstart,0);

  for (i = 0; i < nsamp; i++, loc++) {
    for (j = 0; j < dim; j++)
      switch (s->types[j])
	{
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
	  printf ("%s: unrecognized Waves+ data type.\n",
		  "read_esps_hdr");
	  free((char *) ptr);
	  return FALSE;
	}
    buf->tag = (int)(0.5 + (ssf * (s->x_dat[loc] - s->start_time))) + tag_start;
    /* .5 is to round */
    put_fea_rec(buf, hdr, strm);
  }

  free((char *) ptr);
  free_fea_rec(buf);

  return TRUE;
}




